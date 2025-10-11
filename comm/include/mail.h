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

#ifndef MAIL_C
#define MAIL_C

#include "defines.h"
#include "dictionary.h"
#include "stringex.h"
#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum MailTextEncoding
{
    QuotedPrintable,
    Base64,
    XToken,
    UnKnownEncoding
}MailTextEncoding;

typedef enum MimeType
{
    PlainText,
    Html,
    InlineImage,
    Attachment,
    Multipart,
    UnKnownType
}MimeType;

typedef struct mime_node_t mime_node_t;
typedef struct mail_body_t mail_body_t;
typedef struct mail_t mail_t;

extern LIBRARY_EXPORT void mail_body_set_message_id(mail_body_t* mailbody, const char* msgid);
extern LIBRARY_EXPORT const char* nail_body_get_message_id(mail_body_t* mailbody);
extern LIBRARY_EXPORT void mail_body_add_node(mail_body_t* mailbody, mime_node_t* node);
extern LIBRARY_EXPORT list_t* mail_body_get_data_nodes(mail_body_t* mailbody, mime_node_t* node);
extern LIBRARY_EXPORT void mail_body_serialize(mail_body_t* mailbody, const char* str);
extern LIBRARY_EXPORT void mail_body_deserialize(mail_body_t* mailbody);
extern LIBRARY_EXPORT bool mail_body_has_multi_mimedata(mail_body_t* mailbody);
extern LIBRARY_EXPORT const char* mail_body_encode_string(mail_body_t* mailbody, const char* str);
extern LIBRARY_EXPORT const char* mail_body_encode_buffer(mail_body_t* mailbody, const char* buffer, unsigned long len);
extern LIBRARY_EXPORT const char* mail_body_decode(mail_body_t* mailbody, const char* str);
extern LIBRARY_EXPORT void mail_generate_messageid(mail_t* mail);
extern LIBRARY_EXPORT void mail_generate_timestamp(mail_t* mail);
extern LIBRARY_EXPORT string_list_t* mail_get_headers(mail_t* mail);
extern LIBRARY_EXPORT const char* mail_get_header_value(mail_t* mail);
extern LIBRARY_EXPORT void mail_set_subject(const char* sub);
extern LIBRARY_EXPORT void mail_set_from(const char* from);
extern LIBRARY_EXPORT void mail_set_messageid(const char* msgid);
extern LIBRARY_EXPORT void mail_set_timestamp(const char* timestamp);
extern LIBRARY_EXPORT void mail_add_to_tolist(const char* receipient, bool overwrite);
extern LIBRARY_EXPORT void mail_add_to_cclist(const char* receipient, bool overwrite);
extern LIBRARY_EXPORT void mail_add_to_bcclist(const char* receipient, bool overwrite);
extern LIBRARY_EXPORT void mail_add_header(const char* key, const char* value);
extern LIBRARY_EXPORT void mail_add_header_from_string(mail_t* mail, const char* strr);
extern LIBRARY_EXPORT const char* mail_get_subject(mail_t* mail);
extern LIBRARY_EXPORT const char* mail_get_from(mail_t* mail);
extern LIBRARY_EXPORT const char* mail_get_messageid(mail_t* mail);
extern LIBRARY_EXPORT const char* mail_get_timestamp(mail_t* mail);
extern LIBRARY_EXPORT string_list_t* mail_get_to_list(mail_t* mail);
extern LIBRARY_EXPORT string_list_t* mail_get_cc_list(mail_t* mail);
extern LIBRARY_EXPORT string_list_t* mail_get_bcc_list(mail_t* mail);
extern LIBRARY_EXPORT const char* mail_get_to_as_string(mail_t* mail);
extern LIBRARY_EXPORT const char* mail_get_cc_as_string(mail_t* mail);
extern LIBRARY_EXPORT const char* mail_get_bcc_as_string(mail_t* mail);
extern LIBRARY_EXPORT void mail_serialize_headers(const char* str);
extern LIBRARY_EXPORT void mail_deserialize_headers(mail_t* mail);
extern LIBRARY_EXPORT const char* mail_encode_headers(mail_t* mail, const char* str);
extern LIBRARY_EXPORT const char* mail_encode_headers_buffer(mail_t* mail, const char *buffer, unsigned long len);
extern LIBRARY_EXPORT const char* mail_decode_headers(mail_t* mail, const char* str);
extern LIBRARY_EXPORT void mail_serialize(mail_t* mail);
extern LIBRARY_EXPORT void mail_deserialize(mail_t* mail);
extern LIBRARY_EXPORT void mail_add_node(mail_t* mail, const char* substring, mime_node_t* node, const char* boundary);
extern LIBRARY_EXPORT void mail_parse_node(mail_t* mail, const char* substring, mime_node_t* node);

#ifdef __cplusplus
}
#endif

#endif
