/* strbuf.c - dynamically growing string buffer
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

#include "varnam-util.h"

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
    if(string == NULL) return 0;

    while(*c != '\0') {
        if(!strbuf_addc(string, *c++))
            return 0;
    }

    return 1;
}

int strbuf_addln(struct strbuf *string, const char *c)
{
    if(strbuf_add(string, c))
        return strbuf_add(string, "\n");
    return 0;
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
int strbuf_is_blank_string(struct strbuf *string)
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
    unsigned int str_length, buffer_length;
    char substring[100];

    if(!str) return 0;

    buffer_length = utf8_length(string->buffer);
    str_length = utf8_length(str);

    substr(substring, string->buffer, (buffer_length - str_length) + 1, str_length);
    return (strcmp(substring, str) == 0 ? 1 : 0);
}

void strbuf_remove_from_last(struct strbuf *string, const char *toremove)
{
    size_t to_remove_len, newlen;
    const char *buf;

    if(!toremove) return;
    to_remove_len = strlen(toremove);

    if(string->length < to_remove_len) return;
    newlen = (string->length - to_remove_len);

    buf = string->buffer + newlen;
    if(strcmp(buf, toremove) == 0) {
        string->buffer[newlen] = '\0';
        string->length = newlen;
    }
}

char* strbuf_detach(struct strbuf *string)
{
    char *buffer;
    assert(string);
    buffer = string->buffer;
    xfree(string);
    return buffer;
}
