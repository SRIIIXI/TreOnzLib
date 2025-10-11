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

/*
  tinyjson_c.h - Minimal DOM JSON parser in pure C
  Suitable for embedded systems / telemetry (flat JSON objects/arrays)
*/

#ifndef JSON_C
#define JSON_C

#ifdef __cplusplus
extern "C" {
#endif

/* Node types */
typedef enum
{
    JSON_NODE_DOCUMENT,
    JSON_NODE_OBJECT,
    JSON_NODE_ARRAY,
    JSON_NODE_STRING,
    JSON_NODE_NUMBER,
    JSON_NODE_BOOLEAN,
    JSON_NODE_NULL
} json_node_type_t;

/* Forward declarations */
typedef struct json_node_t json_node_t;
typedef struct json_document_t json_document_t;

/* Document management */
json_document_t *json_parse_string(const char *input);
json_document_t *json_load_file(const char *path);
void json_free_document(json_document_t *doc);

/* Node navigation
   - For objects: name matches the member key
   - For arrays: pass name == NULL to get elements (or pass specific name ignored)
*/
json_node_t *json_node_first_child_element(json_node_t *node, const char *name);
json_node_t *json_node_next_sibling_element(json_node_t *node, const char *name);
json_node_t *json_node_parent(json_node_t *node);

/* Node info */
json_node_type_t json_node_type(json_node_t *node);
/* For object members returns the key name (NULL for array elements/scalars without a key) */
const char *json_node_name(json_node_t *node);
/* For object nodes: returns textual value of member named 'name' (string representation), NULL if not found.
   For scalar nodes: if name == NULL returns its textual value. */
const char *json_node_get_attr(json_node_t *node, const char *name);
/* For scalar nodes returns the textual content (strings are unquoted), otherwise NULL. */
const char *json_node_get_text(json_node_t *node);

/* Debugging / output */
void json_print_node(json_node_t *node, int indent);

#ifdef __cplusplus
}
#endif

#endif 
