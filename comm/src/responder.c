/*
BSD 2-Clause License

Copyright (c) 2017, Subrato Roy (subratoroy@hotmail.com)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "responder.h"
#include "stringex.h"

#include <memory.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

const int INVALID_SOCKET = -1;
const int SOCKET_ERROR = -1;

#pragma pack(1)
typedef struct responder_t
{
    bool			connected;
    socket_t 		socket;
    struct sockaddr_in		server_address;
    char			server_name[33];
    int				server_port;
    size_t			prefetched_buffer_size;
    unsigned char*	prefetched_buffer;
    int             error_code;
}responder_t;

bool responder_internal_is_ip4_address(char* str);
void responder_internal_split_buffer(const char* orig, size_t orig_len, const char* delimeter, size_t delimeter_len, char** left, size_t* left_len, char** right, size_t* right_len);

responder_t* responder_allocate()
{
    responder_t* ptr = (responder_t*)calloc(1, sizeof (responder_t));
    return ptr;
}

void responder_free(responder_t* ptr)
{
    if(!ptr)
    {
        return;
    }

    if(ptr->connected)
    {
        shutdown(ptr->socket, 2);
        close(ptr->socket);
        ptr->connected = false;
    }

    if(ptr->prefetched_buffer)
    {
        free(ptr->prefetched_buffer);
        ptr->prefetched_buffer = NULL;
        ptr->prefetched_buffer_size = 0;
    }

    free(ptr);
}

bool responder_create_socket(responder_t *ptr, const char* servername, int serverport)
{
    if(!ptr)
    {
        return  false;
    }

    strncpy(ptr->server_name, servername, 32);
    ptr->server_port = serverport;

    ptr->server_address.sin_family = AF_INET;
    ptr->server_address.sin_port = htons(serverport);

    char ipbuffer[32]={0};
    strncpy(ipbuffer, servername, 31);

    bool ip = responder_internal_is_ip4_address(ipbuffer);

    if(!ip)
    {
        struct hostent* pHE = gethostbyname(ptr->server_name);
        if (pHE == 0)
        {
            return false;
        }

        memset(&(ptr->server_address), 0, sizeof(struct sockaddr_in));
        ptr->server_address.sin_family = AF_INET;
        bcopy((char*)pHE->h_addr_list[0], (char*)&(ptr->server_address.sin_addr.s_addr), pHE->h_length);
        ptr->server_address.sin_port = htons(serverport);

    }
    else
    {
         inet_pton (AF_INET, ptr->server_name, &ptr->server_address.sin_addr);
    }

    ptr->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(ptr->socket == INVALID_SOCKET)
    {
        return false;
    }

    return true;
}

responder_t *responder_assign_socket(responder_t *ptr, int inSocket)
{
    if(!ptr)
    {
        return  NULL;
    }

    ptr->socket = inSocket;
    ptr->connected = true;
    return ptr;
}

bool responder_connect_socket(responder_t* ptr)
{
    if(!ptr)
    {
        return  false;
    }

    if(ptr->connected == true)
	{
		return true;
	}

    int returncode = -1;

    returncode = connect(ptr->socket,(struct sockaddr*)&ptr->server_address, sizeof(struct sockaddr_in));

    if(returncode == SOCKET_ERROR)
	{
        ptr->error_code = errno;
        shutdown(ptr->socket, 2);
        close(ptr->socket);
        ptr->connected = false;
		return false;
	}

    ptr->connected = true;
	return true;
}

bool responder_close_socket(responder_t* ptr)
{
    if(!ptr)
    {
        return  false;
    }

    if(ptr->connected == false)
    {
        return true;
    }

    shutdown(ptr->socket, 2);
    close(ptr->socket);

    ptr->connected = false;

	return true;
}

bool responder_receive_buffer_by_length(responder_t* ptr, char** iobuffer, size_t len, size_t* out_len, bool alloc_buffer)
{
    if(!ptr)
    {
        return  false;
    }

    size_t	bufferpos = 0;
    size_t	bytesleft = len;

    // If there are pre-fetched bytes left, we have to copy that first and release memory

    if(alloc_buffer)
    {
        *iobuffer = (char*)calloc(1, len + 1);
    }

    if(ptr->prefetched_buffer_size > 0)
    {
        memcpy(*iobuffer, ptr->prefetched_buffer, ptr->prefetched_buffer_size);
        bytesleft = len - ptr->prefetched_buffer_size;
        bufferpos = ptr->prefetched_buffer_size;
        ptr->prefetched_buffer_size = 0;
        free(ptr->prefetched_buffer);
        ptr->prefetched_buffer = NULL;

        if(bytesleft < 1)
        {
            return true;
        }
    }

    while(true)
    {
        char*	buffer = 0;
        ssize_t	bytesread = 0;
        buffer = (char*)calloc(1, bytesleft + 1);

        if (buffer)
        {
            bytesread = (ssize_t)recv(ptr->socket, buffer, (int)bytesleft, 0);

            if (out_len)
            {
                *out_len += (size_t)bytesread;
            }
        }

        if (bytesread == 0)
        {
            // Connection closed gracefully
            if (buffer)
            {
                free(buffer);
            }

            ptr->connected = false;
            return true;
        }

        // Error or link down
        if(bytesread < 0 || buffer == NULL)
        {
            ptr->error_code = SOCKET_ERROR;

            if (buffer)
            {
                free(buffer);
            }

            if(alloc_buffer)
            {
                free(*iobuffer);
            }
            else
            {
                memset(*iobuffer, 0, len);
            }

            len	= 0;
            ptr->connected = false;
            return false;
        }

        memcpy(*iobuffer+bufferpos, buffer, (size_t)bytesread);
        free(buffer);

        bufferpos = bufferpos + (size_t)bytesread;

        bytesleft = bytesleft - (size_t)bytesread;

        if(bufferpos >= len)
        {
            return true;
        }
    }
}

 bool responder_receive_buffer_by_delimeter(responder_t* ptr, char** iobuffer, bool alloc_buffer)
 {
    if(!ptr)
    {
        return  false;
    }

    // If there are pre-fetched bytes left, we have to copy that first and release memory

    if(ptr->prefetched_buffer_size > 0)
    {
        if(alloc_buffer)
        {
            *iobuffer = (char*)calloc(1, ptr->prefetched_buffer_size + 1);
        }

        memcpy(*iobuffer, ptr->prefetched_buffer, ptr->prefetched_buffer_size);
        ptr->prefetched_buffer_size = 0;
        free(ptr->prefetched_buffer);
        ptr->prefetched_buffer = NULL;
    }

    while(true)
    {
        char*	buffer = 0;
        ssize_t	bytesread = 0;
        buffer = (char*)calloc(1, bytesleft + 1);

        if (buffer)
        {
            bytesread = (ssize_t)recv(ptr->socket, buffer, (int)bytesleft, 0);

            if (out_len)
            {
                *out_len += (size_t)bytesread;
            }
        }

        if (bytesread == 0)
        {
            // Connection closed gracefully
            if (buffer)
            {
                free(buffer);
            }

            ptr->connected = false;
            return true;
        }

        // Error or link down
        if(bytesread < 0 || buffer == NULL)
        {
            ptr->error_code = SOCKET_ERROR;

            if (buffer)
            {
                free(buffer);
            }

            if(alloc_buffer)
            {
                free(*iobuffer);
            }
            else
            {
                memset(*iobuffer, 0, len);
            }

            len	= 0;
            ptr->connected = false;
            return false;
        }

        memcpy(*iobuffer+bufferpos, buffer, (size_t)bytesread);
        free(buffer);

        bufferpos = bufferpos + (size_t)bytesread;

        bytesleft = bytesleft - (size_t)bytesread;

        if(bufferpos >= len)
        {
            return true;
        }
    }
 }


bool responder_receive_string(responder_t* ptr, char** iostr, const char* delimeter)
{
    if(!ptr)
    {
        return  false;
    }

    char*   current_line = NULL;
    char*   next_line = NULL;
    size_t delimeter_len = strlen(delimeter);
    size_t current_len = 0;
    size_t next_len = 0;

    if(ptr->prefetched_buffer_size > 0)
	{
        if(strstr((char*)ptr->prefetched_buffer, delimeter) !=0 )
		{
            responder_internal_split_buffer((const char*)ptr->prefetched_buffer, ptr->prefetched_buffer_size, delimeter, delimeter_len, &current_line, &current_len, &next_line, &next_len);

            free(ptr->prefetched_buffer);
            ptr->prefetched_buffer = NULL;
            ptr->prefetched_buffer_size = 0;

            if(current_line != NULL)
            {
                *iostr = (char*)calloc(1, strlen(current_line));
                strcpy(*iostr, current_line);
                free(current_line);
            }

            if(next_line != NULL)
            {
                ptr->prefetched_buffer_size = strlen(next_line);
                ptr->prefetched_buffer = (unsigned char*)calloc(1, (sizeof (unsigned char)*ptr->prefetched_buffer_size) + 1);
                strcpy((char*)ptr->prefetched_buffer, next_line);
                free(next_line);
            }

			return true;
		}
	}

	while(true)
	{
        char* buffer = NULL;

        size_t out_len = 0;

        bool ret = responder_receive_buffer_by_length(ptr, &buffer, 1024, &out_len, true);

        if(!ret || out_len == 0)
        {
            if(*iostr)
            {
                free(*iostr);
            } 

            if(buffer)
            {
                free(buffer);
            }

            ptr->connected = false;
            ptr->error_code = SOCKET_ERROR;
            return false;
        }

        // Append the newly received data to the pre-fetched buffer
        if(ptr->prefetched_buffer == NULL)
        {
            ptr->prefetched_buffer = (unsigned char*)calloc(1, (sizeof (unsigned char)*out_len) + 1);
            memcpy(ptr->prefetched_buffer, buffer, out_len);
            ptr->prefetched_buffer_size = out_len;
        }
        else
        {
            ptr->prefetched_buffer = (unsigned char*)realloc(ptr->prefetched_buffer, (sizeof (unsigned char)*(ptr->prefetched_buffer_size + out_len)) + 1); 
            memcpy(ptr->prefetched_buffer + ptr->prefetched_buffer_size, buffer, out_len);
            ptr->prefetched_buffer_size = ptr->prefetched_buffer_size + out_len;
        }

        free(buffer);

        if(strstr(ptr->prefetched_buffer, delimeter) != 0)
		{
            responder_internal_split_buffer((const char*)ptr->prefetched_buffer, ptr->prefetched_buffer_size, delimeter, delimeter_len, &current_line, &current_len, &next_line, &next_len);

            if(next_line != NULL)
            {
                ptr->prefetched_buffer_size = strlen(next_line);
            }
            
            if(ptr->prefetched_buffer_size > 0)
            {
                ptr->prefetched_buffer = (unsigned char*)calloc(1, ptr->prefetched_buffer_size +1);
                memcpy(ptr->prefetched_buffer, next_line, ptr->prefetched_buffer_size);
                free(next_line);
            }
            
            if(*iostr == NULL)
            {
                *iostr = (char*)calloc(1, strlen(current_line) + 1);
            }
            else
            {
                *iostr = (char*)realloc(*iostr, strlen(*iostr) + strlen(current_line) + 1);
            }

            strcat(*iostr, current_line);
            free(current_line);

            return true;
		}

	}
	return true;
}

bool responder_send_buffer(responder_t* ptr, const char* data, size_t len)
{
    if(!ptr)
    {
        return  false;
    }

	long sentsize =0;

    sentsize = send(ptr->socket, data, (int)len, (int)0);

    if(sentsize == SOCKET_ERROR)
	{
		return false;
	}

	return true;
}

bool responder_send_string(responder_t* ptr, const char* str)
{
    size_t len = strlen(str);

    return responder_send_buffer(ptr, str, len);
}

size_t responder_get_prefetched_buffer_size(responder_t *ptr)
{
    if(!ptr)
    {
        return  false;
    }

    return ptr->prefetched_buffer_size;
}

bool responder_is_connected(responder_t* ptr)
{
    if(!ptr)
    {
        return  false;
    }

    return ptr->connected;
}

socket_t responder_get_socket(responder_t *ptr)
{
    if(!ptr)
    {
        return  false;
    }

    if(ptr->connected)
    {
        return ptr->socket;
    }

    return -1;
}

int  responder_get_error_code(responder_t* ptr)
{
    if(!ptr)
    {
        return  SOCKET_ERROR;
    }

    return ptr->error_code;
}

bool responder_internal_is_ip4_address(char* str)
{
    size_t slen = strlen(str);

    // Check the string length, for the range ...
    // 0.0.0.0 and 255.255.255.255
    if(slen < 7 || slen > 15)
    {
        // Bail out
        return false;
    }

    int ctr;
    bool isdelimeter = false;
    char nibble[4];
    memset((char*)&nibble[0],0,4);
    int nbindex = 0;
    for(ctr = 0 ; str[ctr] != '\0' ; ctr++)
    {
        // Check for permitted characters
        if(str[ctr] != '.' && isdigit(str[ctr]) <= 0)
        {
            // Bail out
            return false;
        }

        // '.' Delimeter case
        if(str[ctr] == '.')
        {
            if(isdelimeter)
            {
                // The flag was set in last iteration
                // This means ".." type of expression was found
                // Bail out
                return false;
            }

            // We have read a complete nibble
            // The characters in the nibble must represent a permissible value
            int numval = atoi(nibble);
            if(numval < 0 || numval > 255)
            {
                return false;
            }

            // Set the flag and continue
            memset((char*)&nibble[0],0,4);
            nbindex = 0;
            isdelimeter = true;
            continue;
        }

        if(isdigit(str[ctr])> 0)
        {
            isdelimeter = false;
            nibble[nbindex] = str[ctr];
            nbindex++;
            continue;
        }

    }

    return true;
}

void responder_internal_split_buffer(const char* orig, size_t orig_len, const char* delimeter, size_t delimeter_len, char** left, size_t* left_len, char** right, size_t* right_len)
{
    // Set output pointers to NULL and 0 initially to handle cases where the delimiter is not found.
    *left = NULL;
    *left_len = 0;
    *right = NULL;
    *right_len = 0;

    if (delimeter_len == 0 || orig_len == 0 || delimeter_len > orig_len)
    {
        // If the delimiter is invalid or longer than the original buffer,
        // treat the whole original buffer as the left part and the right part as empty.
        *left = (char*)calloc(1, orig_len+1);
        if (*left)
        {
            memcpy(*left, orig, orig_len);
            *left_len = orig_len;
        }
        return;
    }

    // Find the first occurrence of the delimiter.
    const char* p = orig;
    const char* end = orig + orig_len - delimeter_len;
    while (p <= end)
    {
        if (memcmp(p, delimeter, delimeter_len) == 0)
        {
            // Delimiter found.
            size_t left_size = p - orig;
            size_t right_size = orig_len - (left_size + delimeter_len);

            // Allocate and copy the left part.
            *left = (char*)calloc(1, left_size+1);
            if (*left)
            {
                memcpy(*left, orig, left_size);
                *left_len = left_size;
            }

            // Allocate and copy the right part.
            *right = (char*)calloc(1, right_size+1);
            if (*right)
            {
                memcpy(*right, p + delimeter_len, right_size);
                *right_len = right_size;
            }
            return;
        }
        p++;
    }

    // If the delimiter is not found, the whole buffer is the "left" part.
    *left = (char*)calloc(1, orig_len+1);
    if (*left)
    {
        memcpy(*left, orig, orig_len);
        *left_len = orig_len;
    }
}

void responder_clear_buffer(responder_t* ptr)
{
    if(ptr == NULL)
    {
        return;
    }

    if(ptr->prefetched_buffer_size > 0)
    {
        printf("Clearing %d bytes from prefetched buffer", ptr->prefetched_buffer_size);
    }

    free(ptr->prefetched_buffer);
    ptr->prefetched_buffer = NULL;
    ptr->prefetched_buffer_size = 0;
}
