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
#include "securitytypes.h"
#include "dictionary.h"
#include "base64.h"
#include "mail.h"

#include <memory.h>
#include <netdb.h>

typedef struct smtp_t
{
    char host[33];
    char username[33];
    char password[33];
    char emailid[33];
    uint16_t port;
    bool start_tls;
    const char* public_ip_address;
    security_type_t securityType;
    char errorStr[65];
    dictionary_t* email_header;
    mail_body_t* email_body;
    
    //responder_ssl_t* bearer;
    tcp_client_t* bearer;
}smtp_t;

char selfIp[16] = {0};

smtp_t* smtp_allocate(void)
{
    smtp_t* ptr = (smtp_t*)malloc(sizeof(smtp_t));

    if (ptr)
    {
        memset(ptr, 0, sizeof(smtp_t));
        ptr->port = 25;
        ptr->securityType = None;
        ptr->bearer = NULL;
        ptr->email_body = NULL;
        ptr->email_header = NULL;
        ptr->public_ip_address = NULL;
        ptr->start_tls = false;
        ptr->errorStr[0] = 0;
        return ptr;
    }   

    return NULL;
}

void smtp_free(smtp_t* ptr)
{
    if (ptr)
    {
        if (ptr->email_body)
        {
            free(ptr->email_body);
            ptr->email_body = NULL;
        }

        if (ptr->email_header)
        {
            free(ptr->email_header);
            ptr->email_header = NULL;
        }

        if (ptr->bearer)
        {
            free(ptr->bearer);
            ptr->bearer = NULL;
        }

        free(ptr);
        ptr = NULL;
    }
}

void smtp_set_account_information(smtp_t* ptr, const char* hoststr, uint16_t portstr, const char* usernamestr, const char* passwordstr, const char* emailid, security_type_t sectype)
{
    if (ptr == NULL)
    {
        return;
    }   

    strncpy(ptr->host, hoststr, 32);
    ptr->host[32] = 0;
    strncpy(ptr->username, usernamestr, 32);
    ptr->username[32] = 0;
    strncpy(ptr->password, passwordstr, 32);
    ptr->password[32] = 0;
    strncpy(ptr->emailid, emailid, 32);
    ptr->emailid[32] = 0;
    ptr->port = portstr;
    ptr->securityType = sectype;
}

bool smtp_connect(smtp_t* ptr)
{
    if( ptr == NULL)
    {
        return false;
    }

    // Initialize the responder (bearer) here based on securityType
    if (ptr->securityType == Ssl) 
    {
        //ptr->bearer = responder_ssl_allocate();
        return false;
    } 
    else
    {
        ptr->bearer = tcp_client_allocate();
    }

    if (ptr->bearer == NULL) 
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to allocate responder");
        return false;
    }

    bool created = tcp_client_create_socket(ptr->bearer, ptr->host, ptr->port);
    if (!created) 
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to create socket");
        tcp_client_free(ptr->bearer);
        ptr->bearer = NULL;
        return false;
    }

    bool connected = tcp_client_connect_socket(ptr->bearer);

    if (!connected) 
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to connect to server");
        tcp_client_free(ptr->bearer);
        ptr->bearer = NULL;
        return false;
    }

    string_t* rx_buffer = NULL;

    rx_buffer = tcp_client_receive_string_chunked(ptr->bearer, "\r\n");

    if(!rx_buffer)
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to receive greeting from server");
        tcp_client_close_socket(ptr->bearer);
        tcp_client_free(ptr->bearer);
        ptr->bearer = NULL;
        return false;
    }
    else
    {
        printf("%s\n", string_c_str(rx_buffer)); // For debugging
    }
    
    string_free(&rx_buffer);

    return true;
}

bool smtp_disconnect(smtp_t* ptr)
{
    if (ptr == NULL || ptr->bearer == NULL) 
    {
        return false;
    }   

    bool closed = tcp_client_close_socket(ptr->bearer);

    if (!closed) 
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to close connection");
        return false;
    }

    return true;
}

bool smtp_send_helo(smtp_t* ptr)
{
    char tx_temp[128] = { 0 };
    string_t* tx_buffer = NULL;

    sprintf(tx_temp, "EHLO %s\r\n", selfIp);

    tx_buffer = string_allocate(tx_temp);

    if (!tcp_client_send_string(ptr->bearer, tx_buffer))
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to send EHLO");
        string_free(&tx_buffer);
        return false;
    }

    string_free(&tx_buffer);

    string_t* rx_buffer = NULL;
    string_t* eof_response = string_allocate("250 ");
    string_t* tls_support = string_allocate("STARTTLS");

    while(true)
    {
        rx_buffer = tcp_client_receive_string_precise(ptr->bearer, "\r\n");

        if(!rx_buffer)
        {
            snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to receive EHLO response");
            string_free(&eof_response);
            string_free(&tls_support);
            return false;
        }
        else
        {
            printf("%s\n", string_c_str(rx_buffer)); // For debugging
        }

        if(string_index_of_substr(rx_buffer, tls_support) >= 0)
        {
            ptr->start_tls = true;
            string_free(&tls_support);
        }

        if(string_index_of_substr(rx_buffer, eof_response) >= 0)
        {
            string_free(&rx_buffer);
            string_free(&eof_response);
            break;
        }

        string_free(&rx_buffer);
        rx_buffer = NULL;
    }

    return true;
}

void smtp_set_public_ip_address(smtp_t* ptr, const char* ip)
{
    memset(selfIp, 0, sizeof(selfIp));
    strncpy(selfIp, ip, 15);
}

const char* smtp_get_account(smtp_t* ptr)
{
    return NULL;
}

const char* smtp_get_error(smtp_t* ptr)
{
    return NULL;
}

bool smtp_sendmail_basic(smtp_t* ptr, const char* recipient, const char* subject, const char* plaintext_message )
{
    if(!ptr || !recipient || !subject || !plaintext_message)
    {
        return false;
    }

    string_t* tx_buffer = NULL;
    string_t* rx_buffer = NULL;
    string_t* respcode = NULL;
	char tx_temp[128] = { 0 };

    // Code for RCPT TO

	memset(tx_temp, 0, sizeof(tx_temp));
	sprintf(tx_temp, "MAIL FROM: <%s>\r\n", ptr->username);

    tx_buffer = string_allocate(tx_temp);

    if (!tcp_client_send_string(ptr->bearer, tx_buffer)) 
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to send MAIL FROM");
        string_free(&tx_buffer);
        return false;
    }
    string_free(&tx_buffer);
    tx_buffer = NULL;

    rx_buffer = NULL;
    respcode = string_allocate("334");

    rx_buffer = tcp_client_receive_string_chunked(ptr->bearer, "\r\n");

    if (string_index_of_substr(rx_buffer, respcode) < 0) 
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "MAIL FROM not accepted");
        if (rx_buffer)
        {
            string_free(&rx_buffer);
        }
        return false;
    }

    string_free(&rx_buffer);
    string_free(&respcode);
    rx_buffer = NULL;
    respcode = NULL;

    // Code for RCPT TO

    memset(tx_temp, 0, sizeof(tx_temp));
	sprintf(tx_temp, "RCPT TO: <%s>\r\n", recipient);

    tx_buffer = string_allocate(tx_temp);

    if (!tcp_client_send_string(ptr->bearer, tx_buffer)) 
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to send RCPT TO");
        string_free(&tx_buffer);
        return false;
    }
    string_free(&tx_buffer);
    tx_buffer = NULL;

    rx_buffer = NULL;
    respcode = string_allocate("334");

    rx_buffer = tcp_client_receive_string_chunked(ptr->bearer, "\r\n");

    if (string_index_of_substr(rx_buffer, respcode) < 0) 
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "RCPT TO not accepted");
        if (rx_buffer)
        {
            string_free(&rx_buffer);
        }
        return false;
    }

    string_free(&rx_buffer);
    string_free(&respcode);
    rx_buffer = NULL;
    respcode = NULL;

    // Code for VRFY --- FOR FUTURE
    // Code for VRFY --- FOR FUTURE

    // Code for DATA
    memset(tx_temp, 0, sizeof(tx_temp));
	sprintf(tx_temp, "DATA");

    tx_buffer = string_allocate(tx_temp);

    if (!tcp_client_send_string(ptr->bearer, tx_buffer)) 
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to send RCPT TO");
        string_free(&tx_buffer);
        return false;
    }
    string_free(&tx_buffer);
    tx_buffer = NULL;

    rx_buffer = NULL;
    respcode = string_allocate("354");

    rx_buffer = tcp_client_receive_string_chunked(ptr->bearer, "\r\n");

    if (string_index_of_substr(rx_buffer, respcode) < 0) 
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "DATA not accepted");
        if (rx_buffer)
        {
            string_free(&rx_buffer);
        }
        return false;
    }

    string_free(&rx_buffer);
    string_free(&respcode);
    rx_buffer = NULL;
    respcode = NULL;
    // Code for DATA

    // Code for sending actual message body
    memset(tx_temp, 0, sizeof(tx_temp));
    sprintf(tx_temp, "From: %s\r\nTo: %s\r\nSubject: %s\r\n\r\n%s\r\n.\r\n", ptr->emailid, recipient, subject, plaintext_message);


    tx_buffer = string_allocate(tx_temp);   
    if (!tcp_client_send_string(ptr->bearer, tx_buffer)) 
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to send message body");
        string_free(&tx_buffer);
        return false;
    }
    string_free(&tx_buffer);
    tx_buffer = NULL;
    rx_buffer = NULL;

    respcode = string_allocate("250");      
    rx_buffer = tcp_client_receive_string_chunked(ptr->bearer, "\r\n");
    if (string_index_of_substr(rx_buffer, respcode) < 0)
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Message body not accepted");
        if (rx_buffer)
        {
            string_free(&rx_buffer);
        }
        return false;
    }   
    string_free(&rx_buffer);
    string_free(&respcode);
    rx_buffer = NULL;
    respcode = NULL;

    return true;
}

bool smtp_sendmail(smtp_t* ptr, const mail_t* mail)
{
    return false;
//	std::vector<std::string> all_rcpt;

//	for (auto s : mail.Header.GetToList())
//	{
//		all_rcpt.push_back(s);
//	}

//	for (auto s : mail.Header.GetCcList())
//	{
//		all_rcpt.push_back(s);
//	}

//	for (auto s : mail.Header.GetBccList())
//	{
//		all_rcpt.push_back(s);
//	}

//	long pending_rcpt = all_rcpt.size();
//	long sent = 0;

//    bool ret = false;
//	std::string resp;
//	char buff[128] = { 0 };

//	memset(buff, 0, 128);
//	sprintf(buff, "MAIL FROM: <%s>\r\n", username.c_str());

//	bearer.SendString(buff);

//	while (true)
//	{
//		if (!bearer.ReceiveString(resp))
//		{
//			return false;
//		}

//		// FROM is verified
//		if (strcontains(resp.c_str(), "250 2.1.0"))
//		{
//			memset(buff, 0, 128);
//			sprintf(buff, "RCPT TO: <%s>\r\n", all_rcpt[sent].c_str());
//			bearer.SendString(buff);
//			sent++;
//			continue;
//		}

//		// RCPT TO is verified
//		if (strcontains(resp.c_str(), "250 2.1.5"))
//		{
//			if (pending_rcpt == sent)
//			{
//				memset(buff, 0, 128);
//				sprintf(buff, "DATA\r\n");
//				bearer.SendString(buff);
//			}
//			else
//			{
//				memset(buff, 0, 128);
//				sprintf(buff, "RCPT TO: <%s>\r\n", all_rcpt[sent].c_str());
//				bearer.SendString(buff);
//				sent++;
//			}
//			continue;
//		}

//		if (strcontains(resp.c_str(), "354"))
//		{
//			if (mail.SerializedData.length() < 1)
//			{
//				mail.Serialize();
//			}

//			bearer.SendString(mail.SerializedData);
//			bearer.SendString("\r\n.\r\n");
//			continue;
//		}

//		if (strcontains(resp.c_str(), "250 2.0.0"))
//		{
//			ret = true;
//			break;
//		}
//	}

//	return ret;
}

bool smtp_start_tls(smtp_t* ptr)
{
    return true;

//	std::string resp;
//	char buff[128] = { 0 };
//	sprintf(buff, "STARTTLS\r\n");

//	bearer.SendString(buff);

//	bool handshake_ok = false;

//	while (true)
//	{
//		if (!bearer.ReceiveString(resp))
//		{
//			return false;
//		}

//		if (strcontains(resp.c_str(), "220"))
//		{
//			handshake_ok = true;
//		}

//		if (bearer.PendingPreFetchedBufferSize() < 1)
//		{
//			break;
//		}
//	}

//	return bearer.SwitchToSecureMode();
}

bool smtp_need_tls(smtp_t* ptr)
{
    if(ptr == NULL)
    {
        return false;
    }   

    return ptr->start_tls;
}

bool smtp_login(smtp_t* ptr)
{
    if (ptr == NULL || ptr->bearer == NULL) 
    {
        return false;
    }

    string_t* rx_buffer = NULL;
    string_t* respcode = NULL;
    string_t* tx_buffer = NULL;
    char* b64_ptr = NULL;
    
    char tx_temp[128] = {0};
    sprintf(tx_temp, "AUTH LOGIN\r\n");

    tx_buffer = string_allocate(tx_temp);

    if (!tcp_client_send_string(ptr->bearer, tx_buffer)) 
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to send AUTH LOGIN");
        string_free(&tx_buffer);
        return false;
    }

    string_free(&tx_buffer);
    tx_buffer = NULL;

    rx_buffer = tcp_client_receive_string_chunked(ptr->bearer, "\r\n");

    if (!rx_buffer) 
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "No response after AUTH LOGIN");
        return false;
    }

    respcode = string_allocate("334");
    if (string_index_of_substr(rx_buffer, respcode) < 0) 
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "AUTH LOGIN not accepted");
        if (rx_buffer)
        {
            string_free(&rx_buffer);
            string_free(&respcode);
        }
        return false;
    }

    string_free(&rx_buffer);
    string_free(&respcode);
    rx_buffer = NULL;

    // Send base64(username)
    char b64_buffer[64] = {0};
    unsigned long b64_outlen = 0;

    memset(tx_temp, 0, sizeof(tx_temp));
    b64_ptr = base64_encode((const unsigned char*)ptr->username, strlen(ptr->username), b64_buffer, &b64_outlen);
    strcpy(b64_buffer, b64_ptr);
    snprintf(tx_temp, sizeof(tx_temp), "%s\r\n", b64_buffer);
    tx_buffer = string_allocate(tx_temp);
    free(b64_ptr);

    if (!tcp_client_send_string(ptr->bearer, tx_buffer)) 
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to send username");
        string_free(&tx_buffer);
        return false;
    }
    string_free(&tx_buffer);
    tx_buffer = NULL;

    rx_buffer = tcp_client_receive_string_chunked(ptr->bearer, "\r\n");

    if (!rx_buffer) 
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "No response after username");
        return false;
    }

    respcode = string_allocate("334");
    if (string_index_of_substr(rx_buffer, respcode) < 0) 
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Username not accepted");
        string_free(&rx_buffer);
        string_free(&respcode);
        return false;
    }

    string_free(&rx_buffer);
    string_free(&respcode);
    rx_buffer = NULL;

    // Send base64(password)
    memset(b64_buffer, 0, sizeof(b64_buffer));
    memset(tx_temp, 0, sizeof(tx_temp));
    b64_outlen = 0;

    b64_ptr = base64_encode((const unsigned char*)ptr->password, strlen(ptr->password), b64_buffer, &b64_outlen);
    strcpy(b64_buffer, b64_ptr);
    snprintf(tx_temp, sizeof(tx_temp), "%s\r\n", b64_buffer);
    tx_buffer = string_allocate(tx_temp);
    free(b64_ptr);

    if (!tcp_client_send_string(ptr->bearer, tx_buffer)) 
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to send password");
        string_free(&tx_buffer);
        return false;
    }

    string_free(&tx_buffer);
    tx_buffer = NULL;

    rx_buffer = tcp_client_receive_string_chunked(ptr->bearer, "\r\n");

    if (!rx_buffer) 
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "No response after password");
        return false;
    }

    // Success if response contains "235"
    respcode = string_allocate("235");
    if (string_index_of_substr(rx_buffer, respcode) < 0) 
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Authentication failed");
        string_free(&respcode);
        string_free(&rx_buffer);

        return false;
    }

    string_free(&rx_buffer);
    string_free(&respcode);

    return true;
}

bool smtp_logout(smtp_t* ptr)
{
    if( ptr == NULL)
    {
        return false;
    }   

	char tx_temp[128] = { 0 };
	sprintf(tx_temp, "QUIT\r\n");

    string_t* tx_buffer = string_allocate(tx_temp);

    tcp_client_send_string(ptr->bearer, tx_buffer);

    string_t* rx_buffer = NULL; 
    rx_buffer = tcp_client_receive_string_chunked(ptr->bearer, "\r\n");

    string_t *resp = NULL;

    resp = string_allocate("221");

    //Check if the response contains "221"
    if(string_index_of_substr(rx_buffer, resp) < 0)
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to logout");
        return false;   
    }

    string_free(&tx_buffer);
    string_free(&rx_buffer);
    string_free(&resp);

	return true;
}

bool smtp_is_connected(smtp_t* ptr)
{
    if( ptr == NULL || ptr->bearer == NULL)
    {
        return false;
    }   

    return tcp_client_is_connected(ptr->bearer); 
}

bool smtp_resolve_public_ip_address()
{
    tcp_client_t* http_client = tcp_client_allocate();

    if (http_client == NULL) 
    {
        return false;
    }

    bool created = tcp_client_create_socket(http_client, "whatismyip.akamai.com", 80);

    if (!created) 
    {
        printf("Failed to create socket");
        tcp_client_free(http_client);
        return false;
    }

    bool connected = tcp_client_connect_socket(http_client);

    if (!connected) 
    {
        printf("Failed to connect to whatismyip.akamai.com");
        tcp_client_free(http_client);
        return false;
    }

    char tx_temp[128] = { 0 };
    sprintf(tx_temp, "GET / HTTP/1.0\r\nHost: whatismyip.akamai.com\r\n\r\n");
    string_t* tx_buffer = string_allocate(tx_temp);

    if (!tcp_client_send_string(http_client, tx_buffer))
    {
        printf("Failed to send HTTP GET");
        string_free(&tx_buffer);
        tcp_client_close_socket(http_client); // Optional, depending on API
        tcp_client_free(http_client); 
        return false;
    }

    string_free(&tx_buffer);

    string_t* rx_string = NULL;

    // This call reads the HTTP headers
    rx_string = tcp_client_receive_string_precise(http_client, "\r\n\r\n");

    if(rx_string == NULL)
    {
        string_free(&rx_string);
        tcp_client_close_socket(http_client);
        tcp_client_free(http_client);
        return false;
    }
    
    string_list_t* headerlist = NULL;
    headerlist = string_split_by_substr(rx_string, "\r\n");

    string_t* header = string_get_first_from_list(headerlist);
    string_t* contentlenheader = string_allocate("Content-Length");
    string_t* tag = NULL;
    string_t* value = NULL;

    while(header)
    {
        if(string_index_of_substr(header, contentlenheader) >= 0)
        {
            string_split_key_value_by_char(header, ':', &tag, &value);
        }
        header = string_get_next_from_list(headerlist);
    }

    string_free_list(headerlist);
    string_free(&contentlenheader);
    string_free(&rx_string);
    rx_string = NULL;

    size_t bodylen = 0;

    if(value)
    {
        bodylen = atol(string_c_str(value));
    }

    string_free(&tag);
    string_free(&value);

    buffer_t* rx_buffer = NULL;

    // This call reads the body which contains the public IP address
    rx_buffer = tcp_client_receive_buffer_by_length(http_client, bodylen);

    if(rx_buffer == NULL)
    {
        buffer_free(&rx_buffer);
        tcp_client_close_socket(http_client);
        tcp_client_free(http_client);
        return false;
    }

    memset(selfIp, 0, 16);
    strncpy(selfIp, buffer_get_data(rx_buffer), buffer_get_size(rx_buffer));

    tcp_client_close_socket(http_client);
    tcp_client_free(http_client);
    buffer_free(&rx_buffer);

    // TEST CODE
    printf("Public IP Address: %s\n", selfIp);

    return true;
}
