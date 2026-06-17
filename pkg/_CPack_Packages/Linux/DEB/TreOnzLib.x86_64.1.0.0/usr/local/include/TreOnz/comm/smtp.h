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

#ifndef SMTP_C
#define SMTP_C

#include "defines.h"
#include "mail.h"
#include "securitytypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct smtp_t smtp_t;

extern LIBRARY_EXPORT smtp_t* smtp_allocate(void);
extern LIBRARY_EXPORT void smtp_free(smtp_t* ptr);
extern LIBRARY_EXPORT void smtp_set_account_information(smtp_t* ptr, const char* hoststr, uint16_t portstr, const char* usernamestr, const char* passwordstr, const char* emailid, security_type_t sectype);
extern LIBRARY_EXPORT void smtp_set_public_ip_address(smtp_t* ptr, const char* ip);
extern LIBRARY_EXPORT bool smtp_disconnect(smtp_t* ptr);
extern LIBRARY_EXPORT bool smtp_connect(smtp_t* ptr);
extern LIBRARY_EXPORT bool smtp_send_helo(smtp_t* ptr);
extern LIBRARY_EXPORT bool smtp_start_tls(smtp_t* ptr);
extern LIBRARY_EXPORT bool smtp_need_tls(smtp_t* ptr);
extern LIBRARY_EXPORT bool smtp_login(smtp_t* ptr);
extern LIBRARY_EXPORT bool smtp_logout(smtp_t* ptr);
extern LIBRARY_EXPORT bool smtp_sendmail_basic(smtp_t* ptr, const char* recipient, const char* subject, const char* plaintext_message );
extern LIBRARY_EXPORT bool smtp_sendmail(smtp_t* ptr, const mail_t* mail);
extern LIBRARY_EXPORT const char* smtp_get_account(smtp_t* ptr);
extern LIBRARY_EXPORT const char* smtp_get_error(smtp_t* ptr);
extern LIBRARY_EXPORT bool smtp_is_connected(smtp_t* ptr);
extern LIBRARY_EXPORT bool smtp_resolve_public_ip_address();

#ifdef __cplusplus
}
#endif

#endif
