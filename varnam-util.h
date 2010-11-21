/* util.h
 *
 * Copyright (C) 2010 Navaneeth.K.N
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA */


#ifndef VARNAM_LIB_UTIL_H_INCLUDED_095439
#define VARNAM_LIB_UTIL_H_INCLUDED_095439

#include "snprintf.h"

char *substr(char *dst, unsigned int start, unsigned int length, const char *src);
int startswith(const char *string1, const char *string2);

struct strbuf {
    char *buffer;            /* null terminated buffer */
    size_t length;           /* length of the string excluding null terminator */
    size_t allocated;        /* total memory allocated */
};

struct strbuf *strbuf_init(size_t initial_buf_size);
int strbuf_addc(struct strbuf *string, char c);
int strbuf_add(struct strbuf *string, const char *c);
int strbuf_addln(struct strbuf *string, const char *c);
void strbuf_destroy(struct strbuf *string);
void strbuf_clear(struct strbuf *string);
int strbuf_is_blank_string(struct strbuf *string);

void *xmalloc(size_t size);
void xfree (void *ptr);

/* Constants */

#define MAX_PATH_LENGTH 4096
#define MAX_PATTERN_LENGTH 20
 
#endif
