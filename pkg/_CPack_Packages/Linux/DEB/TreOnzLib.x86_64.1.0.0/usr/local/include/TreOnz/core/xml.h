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

#ifndef TINYXML_C_H
#define TINYXML_C_H

#ifdef __cplusplus
extern "C" {
#endif

/* Node types */
typedef enum
{
    XML_NODE_DOCUMENT,
    XML_NODE_ELEMENT,
    XML_NODE_TEXT,
    XML_NODE_CDATA,
    XML_NODE_COMMENT
} xml_node_type_t;

/* Forward declarations */
typedef struct xml_node_t xml_node_t;
typedef struct xml_document_t xml_document_t;

/* Document management */
xml_document_t *xml_parse_string(const char *input);
xml_document_t *xml_load_file(const char *path);
void xml_free_document(xml_document_t *doc);

/* Node navigation */
xml_node_t *xml_node_first_child_element(xml_node_t *node, const char *name);
xml_node_t *xml_node_next_sibling_element(xml_node_t *node, const char *name);
xml_node_t *xml_node_parent(xml_node_t *node);

/* Node info */
xml_node_type_t xml_node_type(xml_node_t *node);
const char *xml_node_name(xml_node_t *node);
const char *xml_node_get_attr(xml_node_t *node, const char *name);
const char *xml_node_get_text(xml_node_t *node);

/* Debugging / output */
void xml_print_node(xml_node_t *node, int indent);

#ifdef __cplusplus
}
#endif

#endif /* TINYXML_C_H */

