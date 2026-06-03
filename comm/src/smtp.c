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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

static bool smtp_response_contains(const string_t* response, const char* token)
{
    if (response == NULL || token == NULL)
    {
        return false;
    }

    const char* raw = string_c_str(response);
    return (raw != NULL && strstr(raw, token) != NULL);
}

static void smtp_get_rfc2822_date(char* out, size_t out_size)
{
    if (out == NULL || out_size == 0)
    {
        return;
    }

    time_t now = time(NULL);
    struct tm local_tm;

    if (now == (time_t)-1 || localtime_r(&now, &local_tm) == NULL)
    {
        out[0] = 0;
        return;
    }

    if (strftime(out, out_size, "%a, %d %b %Y %H:%M:%S %z", &local_tm) == 0)
    {
        out[0] = 0;
    }
}

static char* smtp_prepare_message_body(const char* plaintext_message)
{
    if (plaintext_message == NULL)
    {
        return NULL;
    }

    size_t in_len = strlen(plaintext_message);
    size_t max_len = (in_len * 3) + 8;
    char* out = (char*)calloc(1, max_len);

    if (out == NULL)
    {
        return NULL;
    }

    size_t i = 0;
    size_t o = 0;
    bool line_start = true;

    while (i < in_len)
    {
        char ch = plaintext_message[i];

        if (line_start && ch == '.')
        {
            out[o++] = '.';
        }

        if (ch == '\r')
        {
            out[o++] = '\r';

            if ((i + 1) < in_len && plaintext_message[i + 1] == '\n')
            {
                out[o++] = '\n';
                i++;
            }
            else
            {
                out[o++] = '\n';
            }

            line_start = true;
            i++;
            continue;
        }

        if (ch == '\n')
        {
            out[o++] = '\r';
            out[o++] = '\n';
            line_start = true;
            i++;
            continue;
        }

        out[o++] = ch;
        line_start = false;
        i++;
    }

    if (o < 2 || out[o - 2] != '\r' || out[o - 1] != '\n')
    {
        out[o++] = '\r';
        out[o++] = '\n';
    }

    out[o] = 0;
    return out;
}

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
            dictionary_free(ptr->email_header);
            ptr->email_header = NULL;
        }

        if (ptr->bearer)
        {
            tcp_client_free(ptr->bearer);
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

    if (hoststr != NULL)
    {
        strncpy(ptr->host, hoststr, 32);
    }
    else
    {
        ptr->host[0] = 0;
    }
    ptr->host[32] = 0;

    if (usernamestr != NULL)
    {
        strncpy(ptr->username, usernamestr, 32);
    }
    else
    {
        ptr->username[0] = 0;
    }
    ptr->username[32] = 0;

    if (passwordstr != NULL)
    {
        strncpy(ptr->password, passwordstr, 32);
    }
    else
    {
        ptr->password[0] = 0;
    }
    ptr->password[32] = 0;

    if (emailid != NULL)
    {
        strncpy(ptr->emailid, emailid, 32);
    }
    else
    {
        ptr->emailid[0] = 0;
    }
    ptr->emailid[32] = 0;
    ptr->port = (portstr == 0 ? 25 : portstr);
    ptr->securityType = sectype;
}

bool smtp_connect(smtp_t* ptr)
{
    if( ptr == NULL)
    {
        return false;
    }

    if (ptr->host[0] == 0)
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "SMTP host is empty");
        return false;
    }

    if (ptr->bearer)
    {
        tcp_client_close_socket(ptr->bearer);
        tcp_client_free(ptr->bearer);
        ptr->bearer = NULL;
    }

    // Initialize the responder (bearer) here based on securityType
    if (ptr->securityType == Ssl) 
    {
        //ptr->bearer = responder_ssl_allocate();
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "SSL mode is not supported");
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

    if (!smtp_response_contains(rx_buffer, "220"))
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Invalid SMTP greeting");
        string_free(&rx_buffer);
        tcp_client_close_socket(ptr->bearer);
        tcp_client_free(ptr->bearer);
        ptr->bearer = NULL;
        return false;
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

    tcp_client_free(ptr->bearer);
    ptr->bearer = NULL;

    return true;
}

bool smtp_send_helo(smtp_t* ptr)
{
    if (ptr == NULL || ptr->bearer == NULL)
    {
        return false;
    }

    char tx_temp[128] = { 0 };
    string_t* tx_buffer = NULL;

    const char* helo_host = (selfIp[0] == 0 ? "localhost" : selfIp);
    snprintf(tx_temp, sizeof(tx_temp), "EHLO %s\r\n", helo_host);

    tx_buffer = string_allocate(tx_temp);

    if (tx_buffer == NULL)
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to allocate EHLO buffer");
        return false;
    }

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

    if (eof_response == NULL || tls_support == NULL)
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to allocate SMTP response tokens");
        string_free(&eof_response);
        string_free(&tls_support);
        return false;
    }

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
        if(tls_support != NULL && string_index_of_substr(rx_buffer, tls_support) >= 0)
        {
            ptr->start_tls = true;
            string_free(&tls_support);
            tls_support = NULL;
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
    (void)ptr;
    memset(selfIp, 0, sizeof(selfIp));

    if (ip != NULL)
    {
        strncpy(selfIp, ip, 15);
    }
}

const char* smtp_get_account(smtp_t* ptr)
{
    if (ptr == NULL || ptr->username[0] == 0)
    {
        return NULL;
    }

    return ptr->username;
}

const char* smtp_get_error(smtp_t* ptr)
{
    if (ptr == NULL || ptr->errorStr[0] == 0)
    {
        return NULL;
    }

    return ptr->errorStr;
}

bool smtp_sendmail_basic(smtp_t* ptr, const char* recipient, const char* subject, const char* plaintext_message )
{
    if(!ptr || !ptr->bearer || !tcp_client_is_connected(ptr->bearer) || !recipient || !subject || !plaintext_message)
    {
        return false;
    }

    if (ptr->emailid[0] == 0 && ptr->username[0] == 0)
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Missing sender identity");
        return false;
    }

    string_t* tx_buffer = NULL;
    string_t* rx_buffer = NULL;
    string_t* respcode = NULL;
	char tx_temp[128] = { 0 };
    char date_header[80] = { 0 };
    char* prepared_body = NULL;

    // Code for RCPT TO

	memset(tx_temp, 0, sizeof(tx_temp));
    snprintf(tx_temp, sizeof(tx_temp), "MAIL FROM: <%s>\r\n", (ptr->emailid[0] == 0 ? ptr->username : ptr->emailid));

    tx_buffer = string_allocate(tx_temp);

    if (tx_buffer == NULL)
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to allocate MAIL FROM");
        return false;
    }

    if (!tcp_client_send_string(ptr->bearer, tx_buffer)) 
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to send MAIL FROM");
        string_free(&tx_buffer);
        return false;
    }
    string_free(&tx_buffer);
    tx_buffer = NULL;

    rx_buffer = NULL;
    respcode = string_allocate("250");

    if (respcode == NULL)
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to allocate SMTP response code");
        return false;
    }

    rx_buffer = tcp_client_receive_string_chunked(ptr->bearer, "\r\n");

    if (rx_buffer == NULL || string_index_of_substr(rx_buffer, respcode) < 0) 
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "MAIL FROM not accepted");
        if (rx_buffer)
        {
            string_free(&rx_buffer);
        }
        string_free(&respcode);
        return false;
    }

    string_free(&rx_buffer);
    string_free(&respcode);
    rx_buffer = NULL;
    respcode = NULL;

    // Code for RCPT TO

    memset(tx_temp, 0, sizeof(tx_temp));
    snprintf(tx_temp, sizeof(tx_temp), "RCPT TO: <%s>\r\n", recipient);

    tx_buffer = string_allocate(tx_temp);

    if (tx_buffer == NULL)
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to allocate RCPT TO");
        return false;
    }

    if (!tcp_client_send_string(ptr->bearer, tx_buffer)) 
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to send RCPT TO");
        string_free(&tx_buffer);
        return false;
    }
    string_free(&tx_buffer);
    tx_buffer = NULL;

    rx_buffer = NULL;
    respcode = string_allocate("250");

    if (respcode == NULL)
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to allocate SMTP response code");
        return false;
    }

    rx_buffer = tcp_client_receive_string_chunked(ptr->bearer, "\r\n");

    if (rx_buffer == NULL || string_index_of_substr(rx_buffer, respcode) < 0) 
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "RCPT TO not accepted");
        if (rx_buffer)
        {
            string_free(&rx_buffer);
        }
        string_free(&respcode);
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
    snprintf(tx_temp, sizeof(tx_temp), "DATA\r\n");

    tx_buffer = string_allocate(tx_temp);

    if (tx_buffer == NULL)
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to allocate DATA command");
        return false;
    }

    if (!tcp_client_send_string(ptr->bearer, tx_buffer)) 
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to send DATA command");
        string_free(&tx_buffer);
        return false;
    }
    string_free(&tx_buffer);
    tx_buffer = NULL;

    rx_buffer = NULL;
    respcode = string_allocate("354");

    if (respcode == NULL)
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to allocate SMTP response code");
        return false;
    }

    rx_buffer = tcp_client_receive_string_chunked(ptr->bearer, "\r\n");

    if (rx_buffer == NULL || string_index_of_substr(rx_buffer, respcode) < 0) 
    {
        const char* response = (rx_buffer == NULL ? "no response" : string_c_str(rx_buffer));
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "DATA not accepted: %.42s", (response == NULL ? "(null)" : response));
        if (rx_buffer)
        {
            string_free(&rx_buffer);
        }
        string_free(&respcode);
        return false;
    }

    string_free(&rx_buffer);
    string_free(&respcode);
    rx_buffer = NULL;
    respcode = NULL;
    // Code for DATA

    prepared_body = smtp_prepare_message_body(plaintext_message);
    if (prepared_body == NULL)
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to prepare message body");
        return false;
    }

    smtp_get_rfc2822_date(date_header, sizeof(date_header));

    // Code for sending actual message body
    tx_buffer = string_allocate_formatted("From: %s\r\nTo: %s\r\nSubject: %s\r\nDate: %s\r\nMIME-Version: 1.0\r\nContent-Type: text/plain; charset=UTF-8\r\n\r\n%s.\r\n",
                                         (ptr->emailid[0] == 0 ? ptr->username : ptr->emailid),
                                         recipient,
                                         subject,
                                         (date_header[0] == 0 ? "" : date_header),
                                         prepared_body);

    if (tx_buffer == NULL)
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to allocate message body");
        free(prepared_body);
        return false;
    }

    if (!tcp_client_send_string(ptr->bearer, tx_buffer)) 
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to send message body");
        string_free(&tx_buffer);
        free(prepared_body);
        return false;
    }
    free(prepared_body);
    prepared_body = NULL;

    string_free(&tx_buffer);
    tx_buffer = NULL;
    rx_buffer = NULL;

    respcode = string_allocate("250");
    if (respcode == NULL)
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to allocate SMTP response code");
        return false;
    }

    rx_buffer = tcp_client_receive_string_chunked(ptr->bearer, "\r\n");
    if (rx_buffer == NULL || string_index_of_substr(rx_buffer, respcode) < 0)
    {
        const char* response = (rx_buffer == NULL ? "no response" : string_c_str(rx_buffer));
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Message rejected: %.45s", (response == NULL ? "(null)" : response));
        if (rx_buffer)
        {
            string_free(&rx_buffer);
        }
        string_free(&respcode);
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
    if (ptr == NULL || ptr->bearer == NULL)
    {
        return false;
    }

    char tx_temp[32] = { 0 };
    snprintf(tx_temp, sizeof(tx_temp), "STARTTLS\r\n");

    string_t* tx_buffer = string_allocate(tx_temp);
    if (tx_buffer == NULL)
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to allocate STARTTLS command");
        return false;
    }

    if (!tcp_client_send_string(ptr->bearer, tx_buffer))
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to send STARTTLS");
        string_free(&tx_buffer);
        return false;
    }
    string_free(&tx_buffer);

    string_t* rx_buffer = tcp_client_receive_string_chunked(ptr->bearer, "\r\n");
    if (rx_buffer == NULL || !smtp_response_contains(rx_buffer, "220"))
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "STARTTLS rejected");
        string_free(&rx_buffer);
        return false;
    }

    string_free(&rx_buffer);

    if (!tcp_client_switch_to_tls(ptr->bearer))
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to switch to TLS mode");
        return false;
    }

    ptr->start_tls = false;
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

    if (ptr->username[0] == 0 || ptr->password[0] == 0)
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Missing username or password");
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
    unsigned long b64_outlen = 0;

    memset(tx_temp, 0, sizeof(tx_temp));
    b64_ptr = base64_encode((const unsigned char*)ptr->username, strlen(ptr->username), NULL, &b64_outlen);
    if (b64_ptr == NULL)
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to encode username");
        return false;
    }

    snprintf(tx_temp, sizeof(tx_temp), "%s\r\n", b64_ptr);
    tx_buffer = string_allocate(tx_temp);

    if (tx_buffer == NULL)
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to allocate username payload");
        free(b64_ptr);
        return false;
    }

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
    memset(tx_temp, 0, sizeof(tx_temp));
    b64_outlen = 0;

    b64_ptr = base64_encode((const unsigned char*)ptr->password, strlen(ptr->password), NULL, &b64_outlen);
    if (b64_ptr == NULL)
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to encode password");
        return false;
    }

    snprintf(tx_temp, sizeof(tx_temp), "%s\r\n", b64_ptr);
    tx_buffer = string_allocate(tx_temp);

    if (tx_buffer == NULL)
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to allocate password payload");
        free(b64_ptr);
        return false;
    }

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
    if( ptr == NULL || ptr->bearer == NULL)
    {
        return false;
    }   

	char tx_temp[128] = { 0 };
	snprintf(tx_temp, sizeof(tx_temp), "QUIT\r\n");

    string_t* tx_buffer = string_allocate(tx_temp);

    if (tx_buffer == NULL)
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to allocate QUIT command");
        return false;
    }

    if (!tcp_client_send_string(ptr->bearer, tx_buffer))
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to send QUIT");
        string_free(&tx_buffer);
        return false;
    }

    string_t* rx_buffer = NULL; 
    rx_buffer = tcp_client_receive_string_chunked(ptr->bearer, "\r\n");

    if (rx_buffer == NULL)
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "No response to QUIT");
        string_free(&tx_buffer);
        return false;
    }

    string_t *resp = NULL;

    resp = string_allocate("221");

    if (resp == NULL)
    {
        snprintf(ptr->errorStr, sizeof(ptr->errorStr), "Failed to allocate SMTP response code");
        string_free(&tx_buffer);
        string_free(&rx_buffer);
        return false;
    }

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
    size_t copy_len = buffer_get_size(rx_buffer);
    if (copy_len > (sizeof(selfIp) - 1))
    {
        copy_len = sizeof(selfIp) - 1;
    }

    strncpy(selfIp, buffer_get_data(rx_buffer), copy_len);

    tcp_client_close_socket(http_client);
    tcp_client_free(http_client);
    buffer_free(&rx_buffer);

    // TEST CODE
    printf("Public IP Address: %s\n", selfIp);

    return true;
}
