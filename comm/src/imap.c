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

#include "imap.h"
#include "stringex.h"
#include "responder.h"
#include "responderssl.h"

#include <malloc.h>

typedef struct imap_t
{
    char host[33];
    char username[33];
    char password[33];
    uint16_t port;

    security_type_t securityType;
    char currentDirectory[65];
    char errorStr[65];

    //responder_ssl_t* bearer;
    responder_t* bearer;
}imap_t;

unsigned long imap_internal_get_number(const char* str);

imap_t* imap_allocate(void)
{
    imap_t* ptr = (imap_t*)calloc(1, sizeof (imap_t));
    ptr->bearer = responder_allocate();
    return ptr;
}

void imap_free(imap_t* ptr)
{
    responder_free(ptr->bearer);
    free(ptr);
}

void imap_set_account_information(imap_t* ptr, const char* hoststr, uint16_t portstr, const char* usernamestr, const char* passwordstr, security_type_t sectype)
{
	if (sectype == Tls)
	{
		sectype = Ssl;
	}

//    host = hoststr;
//    port = portstr;
//    username = usernamestr;
//    password = passwordstr;
//    securityType = sectype;
//	currentDirectory = "";

    return;
}

bool imap_connect(imap_t* ptr)
{
//	bool need_ssl = false;

//	if (securityType == None)
//	{
//		need_ssl = false;
//	}
//	else
//	{
//		need_ssl = true;
//	}
	
//	int retcode;

//	if (bearer.CreateSocket(host.c_str(), port, need_ssl))
//	{
//		if (bearer.ConnectSocket(retcode))
//		{
//			return true;
//		}
//	}

	return false;
}

bool imap_disconnect(imap_t* ptr)
{
//	if (bearer.IsConnected())
//	{
//		return bearer.CloseSocket();
//	}

	return false;
}

bool imap_get_capabilities(imap_t* ptr)
{
//	std::string resp;
//	std::string capability = "CY CAPABILITY\r\n";
//	bearer.SendString(capability);

//	while (true)
//	{
//		if (!bearer.ReceiveString(resp, "\r\n"))
//		{
//			return false;
//		}

//		if (strcontains(resp.c_str(), "CY OK"))
//		{
//			return true;
//		}
//	}

	return false;
}

bool imap_login(imap_t* ptr)
{
//	std::string resp;

//	std::string login = "LG LOGIN ";
//	login += username + " ";
//	login += password + "\r\n";

//	bearer.SendString(login);

//	while (true)
//	{
//		if (!bearer.ReceiveString(resp, "\r\n"))
//		{
//			return false;
//		}

//		if (strcontains(resp.c_str(), "LG OK"))
//		{
//			return true;
//		}
//	}

	return false;
}

bool imap_logout(imap_t* ptr)
{
//	std::string resp;

//	std::string login = "LG LOGOUT\r\n ";

//	bearer.SendString(login);

//	while (true)
//	{
//		if (!bearer.ReceiveString(resp, "\r\n"))
//		{
//			return false;
//		}

//		if (strcontains(resp.c_str(), "LG OK"))
//		{
//			return true;
//		}
//	}

	return false;
}

const char* imap_get_error(imap_t* ptr)
{
    //return errorStr;
    return NULL;
}

const char* imap_get_account(imap_t* ptr)
{
    //return username;
    return NULL;
}

bool imap_expunge(imap_t* ptr, const char* dir)
{
//	std::string resp;
//	std::string capability = "EX EXPUNGE\r\n";
//	bearer.SendString(capability);

//	while (true)
//	{
//		if (!bearer.ReceiveString(resp, "\r\n"))
//		{
//			return false;
//		}

//		if (strcontains(resp.c_str(), "EX OK"))
//		{
//			return true;
//		}
//	}

	return false;
}

bool imap_mark_as_seen(imap_t* ptr, const char* uid)
{
//	std::string resp;
//	char command[128] = { 0 };
//	memset(command, 0, 128);
//	sprintf(command, "UID STORE %s +FLAGS \\Seen\r\n", uid.c_str());
//	bearer.SendString(command);

//	bool result = false;

//	while (true)
//	{
//		if (!bearer.ReceiveString(resp, "\r\n"))
//		{
//			result = false;
//			break;
//		}

//		if (strcontains(resp.c_str(), "OK STORE") || strcontains(resp.c_str(), "OK Store"))
//		{
//			result = true;
//			break;
//		}
//	}

//	return result;
    return false;
}

bool imap_delete_message(imap_t* ptr, const char* uid)
{
//	std::string resp;
//	char command[128] = { 0 };
//	memset(command, 0, 128);
//	sprintf(command, "UID STORE %s +FLAGS \\Deleted\r\n", uid.c_str());
//	bearer.SendString(command);

//	bool result = false;

//	while (true)
//	{
//		if (!bearer.ReceiveString(resp, "\r\n"))
//		{
//			result = false;
//			break;
//		}

//		if (strcontains(resp.c_str(), "OK STORE") || strcontains(resp.c_str(), "OK Store"))
//		{
//			result = true;
//			break;
//		}
//	}

//	return result;
    return false;
}

bool imap_flag_message(imap_t* ptr, const char* uid, const char* flag)
{
//	std::string resp;
//	char command[128] = { 0 };
//	memset(command, 0, 128);
//	sprintf(command, "UID STORE %s +FLAGS \\%s\r\n", uid.c_str(), flag.c_str());
//	bearer.SendString(command);

//	bool result = false;

//	while (true)
//	{
//		if (!bearer.ReceiveString(resp, "\r\n"))
//		{
//			result = false;
//			break;
//		}

//		if (strcontains(resp.c_str(), "OK STORE") || strcontains(resp.c_str(), "OK Store"))
//		{
//			result = true;
//			break;
//		}
//	}

//	return result;
    return false;
}

bool imap_get_directory_list(imap_t* ptr, const char** dirList)
{
//	std::string resp;

//	std::string temp = std::string("LS LIST ") + std::string("\"\"") + std::string(" \"*\"") + std::string("\r\n");
//	char command[128] = { 0 };
//	sprintf(command, "%s", temp.c_str());

//	bearer.SendString(command);

//	bool result = false;

//	while (true)
//	{
//		if (!bearer.ReceiveString(resp, "\r\n"))
//		{
//			return false;
//		}

//		if (strcontains(resp.c_str(), "HasChildren") || strcontains(resp.c_str(), "Sync Issue"))
//		{
//			continue;
//		}

//		std::vector<std::string> templist;

//		if (strcontains(resp.c_str(), "\".\""))
//		{
//			strsplit(resp, templist, "\".\"", true);
//			resp = templist[templist.size() -1];
//		}

//		if (strcontains(resp.c_str(), "\"/\""))
//		{
//			strsplit(resp, templist, "\"/\"", true);
//			resp = templist[templist.size() - 1];
//		}

//		if (strcontains(resp.c_str(), "LS OK"))
//		{
//			result = true;
//			break;
//		}
		
//		dirList.push_back(resp);
//	}

//	return result;
    return false;
}

bool imap_select_directory(imap_t* ptr, const char* dirname)
{
//	currentDirectory = dirname;

//	std::string resp;
//	std::vector<std::string> buffer;
//	char command[128] = { 0 };
//	sprintf(command, "IN SELECT \"%s\"\r\n", currentDirectory.c_str());

//	bearer.SendString(command);

//	bool result = false;

//	while (true)
//	{
//		if (!bearer.ReceiveString(resp, "\r\n"))
//		{
//			result = false;
//			break;
//		}

//		if (strcontains(resp.c_str(), "IN OK"))
//		{
//			result = true;
//			break;
//		}

//		if (strcontains(resp.c_str(), "IN BAD"))
//		{
//			result = false;
//			break;
//		}

//		buffer.push_back(resp);
//	}

//	return result;
    return false;
}

bool imap_get_directory(imap_t* ptr, const char* dirname, unsigned long emailCount, unsigned long uidNext)
{
//	currentDirectory = dirname;
	
//	std::string resp;
//	std::vector<std::string> buffer;
//	char command[128] = { 0 };
//	sprintf(command, "IN SELECT \"%s\"\r\n", currentDirectory.c_str());

//	bearer.SendString(command);

//	bool result = false;

//	while (true)
//	{
//		if (!bearer.ReceiveString(resp, "\r\n"))
//		{
//			result = false;
//			break;
//		}

//		if (strcontains(resp.c_str(), "IN OK"))
//		{
//			result = true;
//			break;
//		}

//		if (strcontains(resp.c_str(), "IN BAD"))
//		{
//			result = false;
//			break;
//		}
		
//		buffer.push_back(resp);
//	}

//	if (result)
//	{
//		for (auto str : buffer)
//		{
//			if (strcontains(str.c_str(), "EXISTS") || (strcontains(str.c_str(), "exists")))
//			{
//				emailCount = getNumber(str);
//			}

//			if (strcontains(str.c_str(), "UIDNEXT") || (strcontains(str.c_str(), "uidnext")))
//			{
//				uidNext = getNumber(str);
//			}
//		}
//	}

//	return result;
    return false;
}

bool imap_get_emails_all(imap_t* ptr, const char* dirname, const char* uidlist)
{
//	std::string fromdate = "01-JAN-1980";
//	std::string resp;
//	std::vector<std::string> buffer;
//	char command[128] = { 0 };
//	memset(command, 0, 128);
//	sprintf(command, "UID SEARCH SINCE \"%s\"\r\n", fromdate.c_str());
//	bearer.SendString(command);

//	bool result = false;

//	while (true)
//	{
//		if (!bearer.ReceiveString(resp, "\r\n"))
//		{
//			result = false;
//			break;
//		}

//		if (strcontains(resp.c_str(), "UID OK"))
//		{
//			result = true;
//			break;
//		}

//		if (strcontains(resp.c_str(), "UID NO") || strcontains(resp.c_str(), "UID BAD"))
//		{
//			result = false;
//			break;
//		}

//		uidlist = resp;
//		strreplace(uidlist, "SEARCH", "*");
//		strremove(uidlist, '*');
//		stralltrim(uidlist);
//	}

//	return result;
    return false;
}

bool imap_get_emails_since(imap_t* ptr, const char* dirname, const char* fromdate, const char* uidlist)
{
//	std::string resp;
//	std::vector<std::string> buffer;
//	char command[128] = { 0 };
//	memset(command, 0, 128);
//	sprintf(command, "UID SEARCH SINCE \"%s\"\r\n", fromdate.c_str());
//	bearer.SendString(command);

//	bool result = false;

//	while (true)
//	{
//		if (!bearer.ReceiveString(resp, "\r\n"))
//		{
//			result = false;
//			break;
//		}

//		if (strcontains(resp.c_str(), "UID OK"))
//		{
//			result = true;
//			break;
//		}

//		if (strcontains(resp.c_str(), "UID NO") || strcontains(resp.c_str(), "UID BAD"))
//		{
//			result = false;
//			break;
//		}

//		uidlist = resp;
//		strreplace(uidlist, "SEARCH", "*");
//		strremove(uidlist, '*');
//		stralltrim(uidlist);
//	}

//	return result;
    return false;
}

bool imap_get_emails_prior(imap_t* ptr, const char* dirname, const char* fromdate, const char* uidlist)
{
//	std::string resp;
//	std::vector<std::string> buffer;
//	char command[128] = { 0 };
//	memset(command, 0, 128);
//	sprintf(command, "UID SEARCH BEFORE \"%s\"\r\n", fromdate.c_str());
//	bearer.SendString(command);

//	bool result = false;

//	while (true)
//	{
//		if (!bearer.ReceiveString(resp, "\r\n"))
//		{
//			result = false;
//			break;
//		}

//		if (strcontains(resp.c_str(), "UID OK"))
//		{
//			result = true;
//			break;
//		}

//		if (strcontains(resp.c_str(), "UID NO") || strcontains(resp.c_str(), "UID BAD"))
//		{
//			result = false;
//			break;
//		}

//		uidlist = resp;
//		strreplace(uidlist, "SEARCH", "*");
//		strremove(uidlist, '*');
//		stralltrim(uidlist);
//	}

//	return result;
    return false;
}

bool imap_get_emails_recent(imap_t* ptr, const char* dirname, const char* uidlist)
{
//	std::string resp;
//	std::vector<std::string> buffer;
//	char command[128] = { 0 };
//	memset(command, 0, 128);
//	sprintf(command, "UID SEARCH RECENT\r\n");
//	bearer.SendString(command);

//	bool result = false;

//	while (true)
//	{
//		if (!bearer.ReceiveString(resp, "\r\n"))
//		{
//			result = false;
//			break;
//		}

//		if (strcontains(resp.c_str(), "UID OK"))
//		{
//			result = true;
//			break;
//		}

//		if (strcontains(resp.c_str(), "UID NO") || strcontains(resp.c_str(), "UID BAD"))
//		{
//			result = false;
//			break;
//		}

//		uidlist = resp;
//		strreplace(uidlist, "SEARCH", "*");
//		strremove(uidlist, '*');
//		stralltrim(uidlist);
//	}

//	return result;
    return false;
}

bool imap_get_message_header(imap_t* ptr, const char* uid, mail_t* mail)
{
//	char command[128] = { 0 };
//	memset(command, 0, 128);
//	sprintf(command, "UID FETCH %s (BODY[HEADER.FIELDS (DATE FROM SUBJECT TO CC BCC MESSAGE-ID)])\r\n", uid.c_str());

//	std::string resp;
//	std::vector<std::string> buffer;
//	bearer.SendString(command);

//	bool result = false;

//	while (true)
//	{
//		if (!bearer.ReceiveString(resp, "\r\n"))
//		{
//			return false;
//		}

//		if (strcontains(resp.c_str(), "UID OK"))
//		{
//			result = true;
//			break;
//		}

//		if (strcontains(resp.c_str(), "UID NO") || strcontains(resp.c_str(), "UID BAD"))
//		{
//			result = false;
//			break;
//		}

//		buffer.push_back(resp);
//	}

//	if (result)
//	{
//		buffer.pop_back();
//		buffer.pop_back();
//		buffer.erase(buffer.begin());

//		for (auto str : buffer)
//		{
//			std::string field_name, field_value;
//			strsplit(str, ':', field_name, field_value, true);

//			if (strcontains(field_name.c_str(), "Message-ID") || strcontains(field_name.c_str(), "Message-Id"))
//			{
//				stralltrim(field_value);
//				mail.Header.SetMessageId(field_value);
//			}

//			if (strcontains(field_name.c_str(), "Subject"))
//			{
//				stralltrim(field_value);
//				strreplace(field_value, '|', ',');
//				mail.Header.SetSubject(field_value);
//			}

//			if (strcontains(field_name.c_str(), "Date"))
//			{
//				stralltrim(field_value);
//				mail.Header.SetTimeStamp(field_value);
//			}

//			if (strcontains(field_name.c_str(), "From"))
//			{
//				stralltrim(field_value);
//				mail.Header.SetFrom(field_value);
//			}

//			if (strcontains(field_name.c_str(), "To"))
//			{
//				stralltrim(field_value);
//				mail.Header.AddtoToList(field_value);
//			}

//			if (strcontains(field_name.c_str(), "CC"))
//			{
//				stralltrim(field_value);
//				mail.Header.AddtoCcList(field_value);
//			}

//			if (strcontains(field_name.c_str(), "BCC"))
//			{
//				stralltrim(field_value);
//				mail.Header.AddtoBccList(field_value);
//			}
//		}
//	}

//	return result;
    return false;
}

bool imap_get_message_body(imap_t* ptr, const char* uid, mail_t* mail)
{
//	char command[128] = { 0 };
//	memset(command, 0, 128);
//	sprintf(command, "UID FETCH %s (BODY[TEXT])\r\n", uid.c_str());

//	std::string resp;
//	std::string buffer;
//	bearer.SendString(command);

//	bool result = false;

//	long linectr = 0;

//	while (true)
//	{
//		if (!bearer.ReceiveString(resp, "\r\n"))
//		{
//			return false;
//		}

//		if (strcontains(resp.c_str(), "UID OK"))
//		{
//			result = true;
//			break;
//		}

//		if (strcontains(resp.c_str(), "UID NO") || strcontains(resp.c_str(), "UID BAD"))
//		{
//			result = false;
//			break;
//		}

//		if (linectr == 0)
//		{
//			linectr++;
//			continue;
//		}

//		buffer += resp + "\r\n";
//	}

//	if (result)
//	{
//		mail.Serialize();
//		mail.SerializedData += buffer;
//	}

//	return result;
    return false;
}

///////////////////////////////////////////////////////////////////////////

unsigned long imap_internal_get_number(const char* str)
{
    unsigned long num = -1;
//	std::string str_count;

//	for (auto c : str)
//	{
//		if (isdigit(c))
//		{
//			str_count.push_back(c);
//		}
//	}
	
//	num = atol(str_count.c_str());
	return num;
}
