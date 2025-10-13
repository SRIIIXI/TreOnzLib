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

#ifndef IMAP4_C
#define IMAP4_C

#include "defines.h"
#include "mail.h"
#include "securitytypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct imap4_t imap4_t;

extern LIBRARY_EXPORT imap4_t* imap4_allocate(void);
extern LIBRARY_EXPORT void imap4_free(imap4_t* ptr);
extern LIBRARY_EXPORT void imap4_set_account_information(imap4_t* ptr, const char* hoststr, uint16_t portstr, const char* usernamestr, const char* passwordstr, security_type_t sectype);
extern LIBRARY_EXPORT bool imap4_connect(imap4_t* ptr);
extern LIBRARY_EXPORT bool imap4_login(imap4_t* ptr);
extern LIBRARY_EXPORT bool imap4_disconnect(imap4_t* ptr);
extern LIBRARY_EXPORT bool imap4_logout(imap4_t* ptr);
extern LIBRARY_EXPORT bool imap4_get_directory_list(imap4_t* ptr, const char** dirList);
extern LIBRARY_EXPORT bool imap4_get_capabilities(imap4_t* ptr);
extern LIBRARY_EXPORT bool imap4_get_directory(imap4_t* ptr, const char* dirname, unsigned long emailCount, unsigned long uidNext);
extern LIBRARY_EXPORT bool imap4_select_directory(imap4_t* ptr, const char* dirname);
extern LIBRARY_EXPORT bool imap4_get_emails_since(imap4_t* ptr, const char* dirname, const char* fromdate, const char* uidlist);
extern LIBRARY_EXPORT bool imap4_get_emails_prior(imap4_t* ptr, const char* dirname, const char* fromdate, const char* uidlist);
extern LIBRARY_EXPORT bool imap4_get_emails_recent(imap4_t* ptr, const char* dirname, const char* uidlist);
extern LIBRARY_EXPORT bool imap4_get_emails_all(imap4_t* ptr, const char* dirname, const char* uidlist);
extern LIBRARY_EXPORT bool imap4_get_message_header(imap4_t* ptr, const char* uid, mail_t* mail);
extern LIBRARY_EXPORT bool imap4_get_message_body(imap4_t* ptr, const char* uid, mail_t* mail);
extern LIBRARY_EXPORT bool imap4_delete_message(imap4_t* ptr, const char* uid);
extern LIBRARY_EXPORT bool imap4_flag_message(imap4_t* ptr, const char* uid, const char* flag);
extern LIBRARY_EXPORT bool imap4_mark_as_seen(imap4_t* ptr, const char* uid);
extern LIBRARY_EXPORT bool imap4_expunge(imap4_t* ptr, const char* dir);
extern LIBRARY_EXPORT const char* imap4_get_error(imap4_t* ptr);
extern LIBRARY_EXPORT const char* imap4_get_account(imap4_t* ptr);

#ifdef __cplusplus
}
#endif

#endif
