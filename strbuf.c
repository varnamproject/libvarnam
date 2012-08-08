/* Dynamically growing string buffer
 *
 * Copyright (C) Navaneeth.K.N
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

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "util.h"
#include "varray.h"

static int grow_buffer(struct strbuf *string)
{
    char *tmp;
    size_t toallocate;

    assert(string != NULL);

    toallocate = string->allocated + (string->allocated / 2);
    tmp = (char*) realloc(string->buffer, toallocate);
    if(tmp) {
        string->buffer = tmp;
        string->allocated = toallocate;
        return 1;
    }
    return 0;
}

int strbuf_addc(struct strbuf *string, char c)
{
    size_t space_available;

    assert(string != NULL);

    space_available = string->allocated - string->length;
    if(space_available <= 1) {
        if(!grow_buffer(string)) {
            return 0;
        }
    }
    string->buffer[string->length++] = c;
    string->buffer[string->length] = '\0';

    return 1;
}

struct strbuf *strbuf_init(size_t initial_buf_size)
{
    struct strbuf *string = (struct strbuf*) xmalloc(sizeof (struct strbuf));
    string->length = string->allocated = 0;
    string->buffer = (char*) xmalloc(initial_buf_size);
    string->buffer[0] = '\0';
    string->allocated = initial_buf_size;
    return string;
}

int strbuf_add(struct strbuf *string, const char *c)
{
    if(string == NULL || c == NULL) return 0;

    while(*c != '\0') {
        if(!strbuf_addc(string, *c++))
            return 0;
    }

    return 1;
}

int strbuf_add_bytes(struct strbuf *string, const char *c, int bytes_to_read)
{
    int i;

    if (c == NULL || *c == '\0') return 0;
    for (i = 0; i < bytes_to_read; i++)
    {
        strbuf_addc (string, c[i]);
    }

    return 1;
}

int strbuf_addln(struct strbuf *string, const char *c)
{
    if(strbuf_add(string, c))
        return strbuf_add(string, "\n");
    return 0;
}

int strbuf_addf(struct strbuf *string, const char *format, ...)
{
    int rc;
    va_list args;

    va_start(args, format);
    rc = strbuf_addvf(string, format, args);
    va_end(args);

    return rc;
}

int strbuf_addvf(struct strbuf *string, const char *format, va_list args)
{
    const char *p;
    char fmtbuf[256];
    char *s;
    char c;
    int i;
    double f;

    for(p = format; *p != '\0'; p++)
    {
	if(*p != '%')
        {
            if(!strbuf_addc(string, *p))
                return 0;

            continue;
        }

	switch(*++p)
        {
        case 'c':
            c = (char) va_arg(args, int);
            if(!strbuf_addc(string, c))
                return 0;
            break;

        case 'd':
            i = va_arg(args, int);
            snprintf(fmtbuf, 256, "%d", i);
            if(!strbuf_add(string, fmtbuf))
                return 0;
            break;

        case 'f':
            f = va_arg(args, double);
            snprintf(fmtbuf, 256, "%f", f);
            if(!strbuf_add(string, fmtbuf))
                return 0;
            break;

        case 's':
            s = va_arg(args, char *);
            if(!strbuf_add(string, s))
                return 0;
            break;

        case '%':
            if(!strbuf_add(string, "%"))
                return 0;
            break;
        }
    }

    strbuf_addc (string, '\n');
    return 1;

}

void strbuf_destroy(struct strbuf *string)
{
    if(string->buffer != NULL) {
        xfree(string->buffer);
    }
    xfree(string);
}

/*
 * clears the contents in the buffer. this method won't deallocate
 * memory. it will reuse already allocated memory
 */
void strbuf_clear(struct strbuf *string)
{
    if(string->buffer != NULL)
    {
        string->buffer[0] = '\0';
        string->length = 0;
    }
}

/**
 * Checks the string is empty by stripping whitespaces.
 * returns true if it is blank, else false
 */
int strbuf_is_blank(struct strbuf *string)
{
    const char *b = string->buffer;

    if(string->length == 0) return 1;

    while(*b != '\0') {
        switch(*b)
        {
        case '\n':
        case ' ':
        case '\t':
        case '\r':
            ++b;
            break;
        default:
            return 0;
        }
    }

    return 1;
}

int strbuf_endswith(struct strbuf *string, const char *str)
{
    int str_length, buffer_length;
    char substring[100];

    if(!str) return 0;

    buffer_length = utf8_length(string->buffer);
    str_length = utf8_length(str);

    substr(substring, string->buffer, (buffer_length - str_length) + 1, str_length);
    return (strcmp(substring, str) == 0 ? 1 : 0);
}

bool strbuf_remove_from_first(struct strbuf *string, const char *toremove)
{
    size_t to_remove_len, newlen;
    int i;
    bool matching;

    if(!toremove) return false;
    to_remove_len = strlen(toremove);

    if(string->length < to_remove_len) return false;
    newlen = (string->length - to_remove_len);

    matching = true;
    for (i = 0; i < to_remove_len; i++) {
      if (string->buffer[i] != toremove[i]) {
          matching = false;
            break;
      }
    }

    if (matching) {
      memmove (string->buffer, string->buffer + to_remove_len, newlen);
      string->buffer[newlen] = '\0';
      string->length = newlen;
      return true;
    }
    return false;
}

bool strbuf_remove_from_last(struct strbuf *string, const char *toremove)
{
    size_t to_remove_len, newlen;
    const char *buf;

    if(!toremove) return false;
    to_remove_len = strlen(toremove);

    if(string->length < to_remove_len) return false;
    newlen = (string->length - to_remove_len);

    buf = string->buffer + newlen;
    if(strcmp(buf, toremove) == 0) {
        string->buffer[newlen] = '\0';
        string->length = newlen;
        return true;
    }
    return false;
}

char* strbuf_detach(struct strbuf *string)
{
    char *buffer;
    assert(string);
    buffer = string->buffer;
    xfree(string);
    return buffer;
}

const char* strbuf_to_s(struct strbuf *string)
{
    return string->buffer;
}

struct strbuf* get_pooled_string(varnam *handle)
{
    strbuf *string;

    if (v_->strings_pool == NULL)
        v_->strings_pool = vpool_init ();

    string = vpool_get (v_->strings_pool);
    if (string == NULL)
    {
        string = strbuf_init (20);
        vpool_add (v_->strings_pool, string);
    }

    strbuf_clear (string);
    return string;
}
