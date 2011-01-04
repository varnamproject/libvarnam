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

#include <stddef.h>
#include <stdarg.h> 

/* Cmake will define varnam_EXPORTS on Windows when it
configures to build a shared library. If you are going to use
another build system on windows or create the visual studio
projects by hand you need to define varnam_EXPORTS when
building a DLL on windows.
*/

#if defined (_WIN32) 
  #if defined(varnam_EXPORTS)
    #define  VARNAM_EXPORT __declspec(dllexport)
  #else
    #define  VARNAM_EXPORT __declspec(dllimport)
  #endif /* varnam_EXPORTS */
#else /* defined (_WIN32) */
 #define VARNAM_EXPORT
#endif

#define PORTABLE_SNPRINTF_VERSION_MAJOR 2
#define PORTABLE_SNPRINTF_VERSION_MINOR 2

#ifdef HAVE_SNPRINTF
#include <stdio.h>
#else
VARNAM_EXPORT extern int snprintf(char *, size_t, const char *, /*args*/ ...);
VARNAM_EXPORT extern int vsnprintf(char *, size_t, const char *, va_list);
#endif

#if defined(HAVE_SNPRINTF) && defined(PREFER_PORTABLE_SNPRINTF)
VARNAM_EXPORT extern int portable_snprintf(char *str, size_t str_m, const char *fmt, /*args*/ ...);
VARNAM_EXPORT extern int portable_vsnprintf(char *str, size_t str_m, const char *fmt, va_list ap);
#define snprintf  portable_snprintf
#define vsnprintf portable_vsnprintf
#endif

VARNAM_EXPORT extern int asprintf  (char **ptr, const char *fmt, /*args*/ ...);
VARNAM_EXPORT extern int vasprintf (char **ptr, const char *fmt, va_list ap);
VARNAM_EXPORT extern int asnprintf (char **ptr, size_t str_m, const char *fmt, /*args*/ ...);
VARNAM_EXPORT extern int vasnprintf(char **ptr, size_t str_m, const char *fmt, va_list ap);

/**
 * this macro can skip the multibyte sequences on a UTF8 encoded string.
 * input will be pointed to the next head byte of the UTF8 string
 * input should be unsigned char* to get it working correctly
 **/

#define SKIP_MULTI_BYTE_SEQUENCE(input) {              \
    if( (*(input++)) >= 0xc0 ) {                       \
    while( (*input & 0xc0) == 0x80 ){ input++; }       \
  }                                                    \
}

/**
* substr(output,str,start,length) writes length characters of str beginning with start to substring.
* start is is 1-indexed and string should be valid UTF8.
**/
VARNAM_EXPORT void substr(char *substring, 
                          const char *string,
                          unsigned int start, 
                          unsigned int len);

VARNAM_EXPORT int startswith(const char *string1, const char *string2);

struct strbuf {
    char *buffer;          /* null terminated buffer */
    size_t length;         /* length of the string in bytes excluding null terminator */
    size_t allocated;      /* total memory allocated */
};

VARNAM_EXPORT struct strbuf *strbuf_init(size_t initial_buf_size);
VARNAM_EXPORT int strbuf_addc(struct strbuf *string, char c);
VARNAM_EXPORT int strbuf_add(struct strbuf *string, const char *c);
VARNAM_EXPORT int strbuf_addln(struct strbuf *string, const char *c);
VARNAM_EXPORT void strbuf_destroy(struct strbuf *string);
VARNAM_EXPORT char* strbuf_detach(struct strbuf *string);
VARNAM_EXPORT void strbuf_clear(struct strbuf *string);
VARNAM_EXPORT int strbuf_is_blank_string(struct strbuf *string);

VARNAM_EXPORT void *xmalloc(size_t size);
VARNAM_EXPORT void xfree (void *ptr);

/* Constants */

#define MAX_PATH_LENGTH 4096
#define MAX_PATTERN_LENGTH 20
 
#endif
