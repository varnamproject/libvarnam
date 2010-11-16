/* util.c
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


#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "util.h"

void varnam_info(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
    printf("\n");
}

void varnam_error(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    printf("\n");
}

#ifdef WIN32
const char directory_separator[] = "\\";
#else
const char directory_separator[] = "/";
#endif

struct path_info* splitpath(const char *full_path)
{
    size_t length = strlen(full_path);
    struct path_info *p = (struct path_info*) xmalloc(sizeof(struct path_info) + length + 3);  /* Extra space for padding and shifting */
    if(p)
    {
        char *path = (char *) &p[1];    /* copy of the path */
        char *end = &path[length + 1];  
        char *extension;
        char *last_separator;

        /* copy the path */
        strcpy(path, full_path);
        *end = '\0';
        p->directory = end;
        p->extension = end;
        p->filename  = path;

        last_separator = strrchr(path, directory_separator[0]);    /* Finding the last directory separator */
        if(last_separator) {
            memmove(last_separator + 1, last_separator, strlen(last_separator) + 1);  /* inserting a directory separator where null terminator will be inserted */
            p->directory = path;
            *(++last_separator) = '\0';      /* Truncate the directory path */
            p->filename = ++last_separator;  /* Taking the remaining as file name */
        }

        /* Finding the extension starts from second character. This allows handling filenames 
           starts with '.' like '.emacs'.*/
        extension = strrchr(&p->filename[1], '.');
        if(extension) {

            /* shifting the bytes to preserve the extension */
            memmove(extension + 1, extension, strlen(extension) + 1);
            p->extension = extension + 1;

            *extension = '\0';  /* Truncates the file name */
        }
    }
    return p;
}

char *substr(char *dst, unsigned int start, unsigned int length, const char *src)
{
   sprintf(dst, "%.*s", length, src + start);
   return dst;
}

/* return true if string1 starts with string2 */
int startswith(const char *string1, const char *string2)
{
    for(; ; string1++, string2++) {
        if(!*string2) {
            break;
        }
        else if(*string1 != *string2) {
            return 0;
        }
    }
    return 1;
}

void *xmalloc(size_t size)
{
    void *ret = malloc(size);
    return ret;
}

void xfree (void *ptr)
{
    if(ptr)
        free(ptr);
}
