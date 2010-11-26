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

#include "varnam-util.h"
#include "varnam-types.h"

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
