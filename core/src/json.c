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

#include "json.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>

/* ------------------------------------------------------------------------- */
/* Structures                                                                */
/* ------------------------------------------------------------------------- */

struct json_node_t
{
    json_node_type_t type;
    char *name;              /* Key for object members, NULL otherwise */
    char *text;              /* For scalars: string, number, boolean, null */
    json_node_t *parent;
    json_node_t *first_child;
    json_node_t *next_sibling;
};

struct json_document_t
{
    json_node_t *root;
};

/* ------------------------------------------------------------------------- */
/* Memory Helpers                                                            */
/* ------------------------------------------------------------------------- */

static void *xmalloc(size_t size)
{
    void *p = malloc(size);
    return p;
}

static void xfree_ptr(void *p)
{
    if (p)
        free(p);
}

static char *xstrdup(const char *s)
{
    if (!s)
        return NULL;

    size_t n = strlen(s) + 1;
    char *p = xmalloc(n);
    if (!p)
        return NULL;

    memcpy(p, s, n);
    return p;
}

/* ------------------------------------------------------------------------- */
/* Node Helpers                                                              */
/* ------------------------------------------------------------------------- */

static void skip_ws(const char **p)
{
    while (**p && (unsigned char)**p <= ' ')
        (*p)++;
}

static json_node_t *node_new(json_node_type_t type)
{
    json_node_t *n = (json_node_t *)xmalloc(sizeof(json_node_t));
    if (!n)
        return NULL;

    n->type = type;
    n->name = NULL;
    n->text = NULL;
    n->parent = NULL;
    n->first_child = NULL;
    n->next_sibling = NULL;

    return n;
}

static void node_append_child(json_node_t *parent, json_node_t *child)
{
    if (!parent || !child)
        return;

    child->parent = parent;

    if (!parent->first_child)
    {
        parent->first_child = child;
    }
    else
    {
        json_node_t *cur = parent->first_child;
        while (cur->next_sibling)
            cur = cur->next_sibling;
        cur->next_sibling = child;
    }
}

static void node_free_recursive(json_node_t *n)
{
    if (!n)
        return;

    json_node_t *c = n->first_child;
    while (c)
    {
        json_node_t *next = c->next_sibling;
        node_free_recursive(c);
        c = next;
    }

    xfree_ptr(n->name);
    xfree_ptr(n->text);
    xfree_ptr(n);
}

/* ------------------------------------------------------------------------- */
/* String Utilities                                                          */
/* ------------------------------------------------------------------------- */

static char *unescape_json_string(const char *start, size_t len)
{
    char *out = (char *)xmalloc(len + 1);
    if (!out)
        return NULL;

    size_t oi = 0;

    for (size_t i = 0; i < len; ++i)
    {
        char c = start[i];

        if (c == '\\' && i + 1 < len)
        {
            char e = start[i + 1];
            switch (e)
            {
                case '"':  out[oi++] = '"';  i++; break;
                case '\\': out[oi++] = '\\'; i++; break;
                case '/':  out[oi++] = '/';  i++; break;
                case 'b':  out[oi++] = '\b'; i++; break;
                case 'f':  out[oi++] = '\f'; i++; break;
                case 'n':  out[oi++] = '\n'; i++; break;
                case 'r':  out[oi++] = '\r'; i++; break;
                case 't':  out[oi++] = '\t'; i++; break;

                case 'u':
                {
                    if (i + 5 < len)
                    {
                        unsigned int code = 0;
                        int ok = 1;

                        for (int k = 0; k < 4; ++k)
                        {
                            char ch = start[i + 2 + k];
                            code <<= 4;

                            if (ch >= '0' && ch <= '9') code |= ch - '0';
                            else if (ch >= 'a' && ch <= 'f') code |= 10 + (ch - 'a');
                            else if (ch >= 'A' && ch <= 'F') code |= 10 + (ch - 'A');
                            else { ok = 0; break; }
                        }

                        if (ok)
                        {
                            if (code <= 0x7F)
                            {
                                out[oi++] = (char)code;
                            }
                            else if (code <= 0x7FF)
                            {
                                out[oi++] = (char)(0xC0 | ((code >> 6) & 0x1F));
                                out[oi++] = (char)(0x80 | (code & 0x3F));
                            }
                            else
                            {
                                out[oi++] = (char)(0xE0 | ((code >> 12) & 0x0F));
                                out[oi++] = (char)(0x80 | ((code >> 6) & 0x3F));
                                out[oi++] = (char)(0x80 | (code & 0x3F));
                            }

                            i += 5;
                        }
                    }
                    break;
                }

                default:
                    out[oi++] = e;
                    i++;
                    break;
            }
        }
        else
        {
            out[oi++] = c;
        }
    }

    out[oi] = '\0';
    return out;
}

/* ------------------------------------------------------------------------- */
/* Parsing Primitives                                                        */
/* ------------------------------------------------------------------------- */

static char *parse_string_literal(const char **p)
{
    const char *s = *p;

    if (*s != '"')
        return NULL;

    s++;
    const char *start = s;

    while (*s && *s != '"')
    {
        if (*s == '\\' && *(s + 1))
            s++;
        s++;
    }

    if (*s != '"')
        return NULL;

    size_t len = (size_t)(s - start);
    char *res = unescape_json_string(start, len);

    *p = s + 1;
    return res;
}

static char *parse_number_literal(const char **p)
{
    const char *s = *p;
    const char *start = s;

    if (*s == '-') s++;
    if (*s == '0') s++;
    else if (isdigit((unsigned char)*s))
        while (isdigit((unsigned char)*s)) s++;
    else
        return NULL;

    if (*s == '.')
    {
        s++;
        if (!isdigit((unsigned char)*s)) return NULL;
        while (isdigit((unsigned char)*s)) s++;
    }

    if (*s == 'e' || *s == 'E')
    {
        s++;
        if (*s == '+' || *s == '-') s++;
        if (!isdigit((unsigned char)*s)) return NULL;
        while (isdigit((unsigned char)*s)) s++;
    }

    size_t len = (size_t)(s - start);
    char *out = (char *)xmalloc(len + 1);
    if (!out)
        return NULL;

    memcpy(out, start, len);
    out[len] = '\0';

    *p = s;
    return out;
}

static char *parse_literal_text(const char **p, const char *lit)
{
    size_t L = strlen(lit);
    if (strncmp(*p, lit, L) == 0)
    {
        *p += L;
        return xstrdup(lit);
    }
    return NULL;
}

/* ------------------------------------------------------------------------- */
/* Recursive Descent Parser                                                  */
/* ------------------------------------------------------------------------- */

static json_node_t *parse_value(const char **p);

static json_node_t *parse_array(const char **p)
{
    const char *s = *p;

    if (*s != '[')
        return NULL;

    s++;
    skip_ws(&s);

    json_node_t *arr = node_new(JSON_NODE_ARRAY);
    if (!arr)
        return NULL;

    if (*s == ']')
    {
        *p = s + 1;
        return arr;
    }

    while (1)
    {
        skip_ws(&s);
        json_node_t *elem = parse_value(&s);
        if (!elem)
        {
            node_free_recursive(arr);
            return NULL;
        }

        node_append_child(arr, elem);
        skip_ws(&s);

        if (*s == ',')
        {
            s++;
            continue;
        }
        else if (*s == ']')
        {
            *p = s + 1;
            return arr;
        }
        else
        {
            node_free_recursive(arr);
            return NULL;
        }
    }
}

static json_node_t *parse_object(const char **p)
{
    const char *s = *p;

    if (*s != '{')
        return NULL;

    s++;
    skip_ws(&s);

    json_node_t *obj = node_new(JSON_NODE_OBJECT);
    if (!obj)
        return NULL;

    if (*s == '}')
    {
        *p = s + 1;
        return obj;
    }

    while (1)
    {
        skip_ws(&s);

        if (*s != '"')
        {
            node_free_recursive(obj);
            return NULL;
        }

        char *key = parse_string_literal(&s);
        skip_ws(&s);

        if (*s != ':')
        {
            xfree_ptr(key);
            node_free_recursive(obj);
            return NULL;
        }

        s++;
        skip_ws(&s);

        json_node_t *val = parse_value(&s);
        if (!val)
        {
            xfree_ptr(key);
            node_free_recursive(obj);
            return NULL;
        }

        val->name = key;
        node_append_child(obj, val);
        skip_ws(&s);

        if (*s == ',')
        {
            s++;
            continue;
        }
        else if (*s == '}')
        {
            *p = s + 1;
            return obj;
        }
        else
        {
            node_free_recursive(obj);
            return NULL;
        }
    }
}

static json_node_t *parse_value(const char **p)
{
    skip_ws(p);
    const char *s = *p;

    if (!*s)
        return NULL;

    if (*s == '{') return parse_object(p);
    if (*s == '[') return parse_array(p);

    if (*s == '"')
    {
        char *str = parse_string_literal(p);
        if (!str) return NULL;

        json_node_t *n = node_new(JSON_NODE_STRING);
        if (!n)
        {
            xfree_ptr(str);
            return NULL;
        }

        n->text = str;
        return n;
    }

    if (*s == '-' || isdigit((unsigned char)*s))
    {
        char *num = parse_number_literal(p);
        if (!num) return NULL;

        json_node_t *n = node_new(JSON_NODE_NUMBER);
        if (!n)
        {
            xfree_ptr(num);
            return NULL;
        }

        n->text = num;
        return n;
    }

    if (strncmp(s, "true", 4) == 0)
    {
        *p = s + 4;
        json_node_t *n = node_new(JSON_NODE_BOOLEAN);
        n->text = xstrdup("true");
        return n;
    }

    if (strncmp(s, "false", 5) == 0)
    {
        *p = s + 5;
        json_node_t *n = node_new(JSON_NODE_BOOLEAN);
        n->text = xstrdup("false");
        return n;
    }

    if (strncmp(s, "null", 4) == 0)
    {
        *p = s + 4;
        json_node_t *n = node_new(JSON_NODE_NULL);
        n->text = xstrdup("null");
        return n;
    }

    return NULL;
}

/* ------------------------------------------------------------------------- */
/* Public API                                                                */
/* ------------------------------------------------------------------------- */

json_document_t *json_parse_string(const char *input)
{
    if (!input)
        return NULL;

    const char *p = input;
    skip_ws(&p);

    json_node_t *rootval = parse_value(&p);
    if (!rootval)
        return NULL;

    skip_ws(&p);
    if (*p != '\0')
    {
        node_free_recursive(rootval);
        return NULL;
    }

    json_document_t *doc = (json_document_t *)xmalloc(sizeof(json_document_t));
    if (!doc)
    {
        node_free_recursive(rootval);
        return NULL;
    }

    json_node_t *docroot = node_new(JSON_NODE_DOCUMENT);
    node_append_child(docroot, rootval);
    doc->root = docroot;

    return doc;
}

json_document_t *json_load_file(const char *path)
{
    if (!path)
        return NULL;

    FILE *f = fopen(path, "rb");
    if (!f)
        return NULL;

    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    rewind(f);

    char *buf = (char *)xmalloc((size_t)sz + 1);
    if (!buf)
    {
        fclose(f);
        return NULL;
    }

    fread(buf, 1, (size_t)sz, f);
    buf[sz] = '\0';
    fclose(f);

    json_document_t *doc = json_parse_string(buf);
    xfree_ptr(buf);
    return doc;
}

void json_free_document(json_document_t *doc)
{
    if (!doc)
        return;

    if (doc->root)
        node_free_recursive(doc->root);

    xfree_ptr(doc);
}

/* ------------------------------------------------------------------------- */
/* Node Navigation and Access                                                */
/* ------------------------------------------------------------------------- */

json_node_t *json_node_first_child_element(json_node_t *node, const char *name)
{
    if (!node)
        return NULL;

    json_node_t *c = node->first_child;

    if (!name)
        return c;

    while (c)
    {
        if (c->name && strcmp(c->name, name) == 0)
            return c;
        c = c->next_sibling;
    }

    return NULL;
}

json_node_t *json_node_next_sibling_element(json_node_t *node, const char *name)
{
    if (!node)
        return NULL;

    json_node_t *s = node->next_sibling;

    if (!name)
        return s;

    while (s)
    {
        if (s->name && strcmp(s->name, name) == 0)
            return s;
        s = s->next_sibling;
    }

    return NULL;
}

json_node_t *json_node_parent(json_node_t *node)
{
    return node ? node->parent : NULL;
}

json_node_type_t json_node_type(json_node_t *node)
{
    return node ? node->type : JSON_NODE_NULL;
}

const char *json_node_name(json_node_t *node)
{
    return node ? node->name : NULL;
}

const char *json_node_get_attr(json_node_t *node, const char *name)
{
    if (!node)
        return NULL;

    if (node->type == JSON_NODE_OBJECT)
    {
        if (!name)
            return NULL;

        json_node_t *c = node->first_child;
        while (c)
        {
            if (c->name && strcmp(c->name, name) == 0)
                return c->text;
            c = c->next_sibling;
        }

        return NULL;
    }

    if (name == NULL)
        return node->text;

    return NULL;
}

const char *json_node_get_text(json_node_t *node)
{
    if (!node)
        return NULL;

    switch (node->type)
    {
        case JSON_NODE_STRING:
        case JSON_NODE_NUMBER:
        case JSON_NODE_BOOLEAN:
        case JSON_NODE_NULL:
            return node->text;

        default:
            return NULL;
    }
}

/* ------------------------------------------------------------------------- */
/* Debug Print                                                               */
/* ------------------------------------------------------------------------- */

static void print_indent(int n)
{
    for (int i = 0; i < n; ++i)
        putchar(' ');
}

void json_print_node(json_node_t *node, int indent)
{
    if (!node)
        return;

    print_indent(indent);

    switch (node->type)
    {
        case JSON_NODE_DOCUMENT:
            printf("DOCUMENT\n");
            if (node->first_child)
                json_print_node(node->first_child, indent + 2);
            break;

        case JSON_NODE_OBJECT:
            printf("%sOBJECT\n", node->name ? node->name : "");
            for (json_node_t *c = node->first_child; c; c = c->next_sibling)
                json_print_node(c, indent + 2);
            break;

        case JSON_NODE_ARRAY:
            printf("%sARRAY\n", node->name ? node->name : "");
            for (json_node_t *c = node->first_child; c; c = c->next_sibling)
                json_print_node(c, indent + 2);
            break;

        case JSON_NODE_STRING:
            printf("%sSTRING = \"%s\"\n", node->name ? node->name : "", node->text ? node->text : "");
            break;

        case JSON_NODE_NUMBER:
            printf("%sNUMBER = %s\n", node->name ? node->name : "", node->text ? node->text : "");
            break;

        case JSON_NODE_BOOLEAN:
            printf("%sBOOLEAN = %s\n", node->name ? node->name : "", node->text ? node->text : "");
            break;

        case JSON_NODE_NULL:
            printf("%sNULL\n", node->name ? node->name : "");
            break;

        default:
            printf("UNKNOWN\n");
            break;
    }
}
