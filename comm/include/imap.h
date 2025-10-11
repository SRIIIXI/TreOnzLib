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

#ifndef IMAP_C
#define IMAP_C

#include "defines.h"
#include "mail.h"
#include "securitytypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct imap_t imap_t;

extern LIBRARY_EXPORT imap_t* imap_allocate(void);
extern LIBRARY_EXPORT void imap_free(imap_t* ptr);
extern LIBRARY_EXPORT void imap_set_account_information(imap_t* ptr, const char* hoststr, uint16_t portstr, const char* usernamestr, const char* passwordstr, security_type_t sectype);
extern LIBRARY_EXPORT bool imap_connect(imap_t* ptr);
extern LIBRARY_EXPORT bool imap_login(imap_t* ptr);
extern LIBRARY_EXPORT bool imap_disconnect(imap_t* ptr);
extern LIBRARY_EXPORT bool imap_logout(imap_t* ptr);
extern LIBRARY_EXPORT bool imap_get_directory_list(imap_t* ptr, const char** dirList);
extern LIBRARY_EXPORT bool imap_get_capabilities(imap_t* ptr);
extern LIBRARY_EXPORT bool imap_get_directory(imap_t* ptr, const char* dirname, unsigned long emailCount, unsigned long uidNext);
extern LIBRARY_EXPORT bool imap_select_directory(imap_t* ptr, const char* dirname);
extern LIBRARY_EXPORT bool imap_get_emails_since(imap_t* ptr, const char* dirname, const char* fromdate, const char* uidlist);
extern LIBRARY_EXPORT bool imap_get_emails_prior(imap_t* ptr, const char* dirname, const char* fromdate, const char* uidlist);
extern LIBRARY_EXPORT bool imap_get_emails_recent(imap_t* ptr, const char* dirname, const char* uidlist);
extern LIBRARY_EXPORT bool imap_get_emails_all(imap_t* ptr, const char* dirname, const char* uidlist);
extern LIBRARY_EXPORT bool imap_get_message_header(imap_t* ptr, const char* uid, mail_t* mail);
extern LIBRARY_EXPORT bool imap_get_message_body(imap_t* ptr, const char* uid, mail_t* mail);
extern LIBRARY_EXPORT bool imap_delete_message(imap_t* ptr, const char* uid);
extern LIBRARY_EXPORT bool imap_flag_message(imap_t* ptr, const char* uid, const char* flag);
extern LIBRARY_EXPORT bool imap_mark_as_seen(imap_t* ptr, const char* uid);
extern LIBRARY_EXPORT bool imap_expunge(imap_t* ptr, const char* dir);
extern LIBRARY_EXPORT const char* imap_get_error(imap_t* ptr);
extern LIBRARY_EXPORT const char* imap_get_account(imap_t* ptr);

#ifdef __cplusplus
}
#endif

#endif
