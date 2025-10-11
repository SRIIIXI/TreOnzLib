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

#include "tcpclient.h"
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

#define MAX_TCP_CLIENT_BUFFER_SIZE 512

#pragma pack(1)
typedef struct tcp_client_t
{
    bool			    connected;
    socket_t 		    socket;
    struct sockaddr_in  server_address;
    char			    server_name[33];
    int				    server_port;
    int                 error_code;
}tcp_client_t;

bool tcp_client_internal_is_ip4_address(char* str);
void tcp_client_internal_split_buffer(const char* orig, size_t orig_len, const char* delimeter, size_t delimeter_len, char** left, size_t* left_len, char** right, size_t* right_len);

tcp_client_t* tcp_client_allocate()
{
    tcp_client_t* ptr = (tcp_client_t*)calloc(1, sizeof (tcp_client_t));
    return ptr;
}

void tcp_client_free(tcp_client_t* ptr)
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

    free(ptr);
}

bool tcp_client_create_socket(tcp_client_t *ptr, const char* servername, int serverport)
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

    bool ip = tcp_client_internal_is_ip4_address(ipbuffer);

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

bool tcp_client_connect_socket(tcp_client_t* ptr)
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

bool tcp_client_close_socket(tcp_client_t* ptr)
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

bool tcp_client_send_buffer(tcp_client_t* ptr, const buffer_t* data)
{
    if(!ptr)
    {
        return  false;
    }

    if(!data)
    {
        return false;
    }

    long len = buffer_get_size(data);
	long sentsize =0;

    sentsize = send(ptr->socket, buffer_get_data(data), (int)len, (int)0);

    if(sentsize == SOCKET_ERROR)
	{
		return false;
	}

	return true;
}

bool tcp_client_send_string(tcp_client_t* ptr, const string_t* str)
{
    if(!ptr)
    {
        return  false;
    }

    if(!str)
    {
        return  false;
    }

    long len = string_get_length(str);
	long sentsize =0;

    sentsize = send(ptr->socket, string_c_str(str), (int)len, (int)0);

    if(sentsize == SOCKET_ERROR)
	{
		return false;
	}

	return true;
}

buffer_t* tcp_client_receive_buffer_by_length(tcp_client_t* ptr, size_t len)
{
    buffer_t* iobuffer = NULL;

    if(!ptr)
    {
        return  NULL;
    }

    size_t	bufferpos = 0;
    size_t  bytesleft = len;
    ssize_t  bytesread = 0;

    char* buffer = (char*)calloc(1, bytesleft+1);

    while(true)
    {
        bytesread = (ssize_t)recv(ptr->socket, &buffer[bufferpos], (int)bytesleft, 0);

        // Connection closed gracefully or error or link down
        // Both cases we have to return
        if (bytesread == 0 || bytesread < 0)
        {
            if (buffer)
            {
                free(buffer);
            }

            ptr->error_code = SOCKET_ERROR;
            ptr->connected = false;
            return NULL;
        }

        bufferpos = bufferpos + (size_t)bytesread;
        bytesleft = bytesleft - (size_t)bytesread;

        if(bufferpos >= len)
        {
            iobuffer = buffer_allocate(buffer, len);
            free(buffer);
            return iobuffer;
        }
    }
}

 buffer_t* tcp_client_receive_buffer_by_delimeter(tcp_client_t* ptr, const char* delimeter, size_t delimeterlen)
 {
    // TBD - Not complete yet
    buffer_t* iobuffer = NULL;

    if(!ptr)
    {
        return  NULL;
    }

    iobuffer = buffer_allocate_default();
    size_t totalbytes = 0;

    while(true)
    {
        char bytes[MAX_TCP_CLIENT_BUFFER_SIZE] = {0};
        ssize_t	bytesread = 0;

        bytesread = (ssize_t)recv(ptr->socket, &bytes[0], MAX_TCP_CLIENT_BUFFER_SIZE, 0);
        totalbytes += bytesread;
      
        // Connection closed gracefully or error or link down
        // Both cases we have to return
        if (bytesread == 0 || bytesread < 0)
        {
            ptr->error_code = SOCKET_ERROR;
            ptr->connected = false;
            buffer_free(&iobuffer);
            return NULL;
        }

        buffer_append(iobuffer, &bytes[0], (size_t)bytesread);

        //ATleast we have read so much data that the delimeter can be present
        if(totalbytes >= delimeterlen)
        {
            const char* read_buffer = buffer_get_data(iobuffer);
            const char* comparison_segment = &read_buffer[totalbytes - delimeterlen];

            if(memcmp(comparison_segment, delimeter, delimeterlen) == 0)
            {
                // We have found the delimeter
                // Caller will call with explict knowledge that delimeter will be at the end, thererore no nmore bytes after this in the socket
                // We just need to trim the delimeter from the end
                size_t new_size = totalbytes - delimeterlen;
                buffer_remove_end(iobuffer, delimeterlen);
                break;
            }
        }
    }

    return iobuffer;
 }

string_t* tcp_client_receive_string(tcp_client_t* ptr, const char* delimeter)
{
    string_t* iostr = NULL;

    if(!ptr)
    {
        return  NULL;
    }

    iostr = string_allocate_default();

    size_t delimeterlen = strlen(delimeter);
    size_t totalbytes = 0;

    while(true)
    {
        char bytes[MAX_TCP_CLIENT_BUFFER_SIZE+1] = {0};
        ssize_t	bytesread = 0;

        bytesread = (ssize_t)recv(ptr->socket, &bytes[0], MAX_TCP_CLIENT_BUFFER_SIZE, 0);
        totalbytes += bytesread;
        
        // Connection closed gracefully or error or link down
        // Both cases we have to return
        if (bytesread == 0 || bytesread < 0)
        {
            ptr->error_code = SOCKET_ERROR;
            ptr->connected = false;
            string_free(&iostr);
            return NULL;
        }

        string_append(iostr, &bytes[0]);

        //ATleast we have read so much data that the delimeter can be present
        if(totalbytes >= delimeterlen)
        {
            const char* read_buffer = string_c_str(iostr);
            const char* comparison_segment = &read_buffer[totalbytes - delimeterlen];

            if(memcmp(comparison_segment, delimeter, delimeterlen) == 0)
            {
                // We have found the delimeter
                // Caller will call with explict knowledge that delimeter will be at the end, thererore no nmore bytes after this in the socket
                // As this is a string, we can trim the delimeter from the end
                size_t new_size = totalbytes - delimeterlen;
                // Just remove the delimeter from the end   
                string_remove_end(iostr, delimeterlen);
                // Ensure null termination  
                break;
            }
        }
    }

    return iostr;
}

bool tcp_client_is_connected(tcp_client_t* ptr)
{
    if(!ptr)
    {
        return  false;
    }

    return ptr->connected;
}

socket_t tcp_client_get_socket(tcp_client_t *ptr)
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

int tcp_client_get_error_code(tcp_client_t* ptr)
{
    if(!ptr)
    {
        return  SOCKET_ERROR;
    }

    return ptr->error_code;
}

bool tcp_client_internal_is_ip4_address(char* str)
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

void tcp_client_internal_split_buffer(const char* orig, size_t orig_len, const char* delimeter, size_t delimeter_len, char** left, size_t* left_len, char** right, size_t* right_len)
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

