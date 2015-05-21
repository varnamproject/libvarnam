/* util.h
 *
 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */


#ifndef VARNAM_LIB_UTIL_H_INCLUDED_095439
#define VARNAM_LIB_UTIL_H_INCLUDED_095439

#include <stddef.h>
#include <stdarg.h>

#ifndef __cplusplus
  typedef int bool;
  #define false 0
  #define true  1
#endif

#include "vtypes.h"
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

#define v_ \
    (handle->internal)\

/**
 * Iterate over token collection and points `current' variable to the current element
 *
 * current       - Variable to hold current element
 * start_from    - Where to start the iteration. Usually this will be the head element
 *
 **/
#define varnam_tokens_for_each(current, start_from)  \
    for(current = start_from; current != NULL; current = current->next) \

/**
 * Iterate over token collection and deleted each entry in the list
 *
 * current       - Variable to hold current element
 * head          - Head of the list. This is where the iteration starts
 *
 **/
#define varnam_tokens_free(current, head)                          \
    while(head != NULL) {                                          \
        current = head->next; free(head); head = current;}         \

#ifdef _RECORD_EXEC_TIME
    #include <time.h>
#endif

#define V_BEGIN_TIMING \
    clock_t start; double diff;                   \
    start = clock();                              \

#define V_REPORT_TIME_TAKEN(s) \
    diff = (double)(clock() - start) / (double) CLOCKS_PER_SEC;         \
    printf ("%s - %f secs\n", s, diff);                                 \

/* Cmake will define varnam_EXPORTS on Windows when it
configures to build a shared library. If you are going to use
another build system on windows or create the visual studio
projects by hand you need to define varnam_EXPORTS when
building a DLL on windows.
*/

#if defined (_WIN32)
  #if defined(varnam_EXPORTS)
    #define VARNAM_EXPORT __declspec(dllexport)
  #else
    #define VARNAM_EXPORT __declspec(dllimport)
  #endif /* varnam_EXPORTS */
#else /* defined (_WIN32) */
 #define VARNAM_EXPORT
#endif

#define PORTABLE_SNPRINTF_VERSION_MAJOR 2
#define PORTABLE_SNPRINTF_VERSION_MINOR 2

#ifdef HAVE_SNPRINTF
#include <stdio.h>
#else
extern int snprintf(char *, size_t, const char *, /*args*/ ...);
extern int vsnprintf(char *, size_t, const char *, va_list);
#endif

#if defined(HAVE_SNPRINTF) && defined(PREFER_PORTABLE_SNPRINTF)
VARNAM_EXPORT extern int portable_snprintf(char *str, size_t str_m, const char *fmt, /*args*/ ...);
VARNAM_EXPORT extern int portable_vsnprintf(char *str, size_t str_m, const char *fmt, va_list ap);
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
 * Advances input to next valid character in a UTF8 sequence
 **/
#define READ_A_UTF8_CHAR(ustring, input, bytes) {        \
    if (*input != '\0') { \
        ustring = (const unsigned char*) input; \
        bytes++; input++;                       \
        if( (*(ustring++)) >= 0xc0 ) {                                  \
            while( (*ustring & 0xc0) == 0x80 ){ ustring++;input++; bytes++; } \
        }}\
else  {bytes = 0;}                              \
}\

/**
* substr(output,str,start,length) writes length characters of str beginning with start to substring.
* start is is 1-indexed and string should be valid UTF8.
**/
VARNAM_EXPORT void substr(char *substring,
                          const char *string,
                          int start,
                          int len);

VARNAM_EXPORT int startswith(const char *string1, const char *string2);

/**
 * calculates length of the UTF8 encoded string.
 * length will be the total number of characters and not the bytes
 **/
VARNAM_EXPORT int utf8_length(const char *string);

int
utf8_ends_with(const char *buffer, const char *tocheck);

typedef struct strbuf {
    char *buffer;          /* null terminated buffer */
    size_t length;         /* length of the string in bytes excluding null terminator */
    size_t allocated;      /* total memory allocated */
} strbuf;

void varnam_debug(varnam *handle, const char *format, ...);
void varnam_log(varnam *handle, const char *format, ...);

VARNAM_EXPORT struct strbuf* strbuf_create_from(const char* value);
VARNAM_EXPORT struct strbuf *strbuf_init(size_t initial_buf_size);
VARNAM_EXPORT int strbuf_addc(struct strbuf *string, char c);
VARNAM_EXPORT int strbuf_add(struct strbuf *string, const char *c);
VARNAM_EXPORT int strbuf_add_bytes(struct strbuf *string, const char *c, int bytes_to_read);
VARNAM_EXPORT int strbuf_addln(struct strbuf *string, const char *c);
VARNAM_EXPORT int strbuf_addf(struct strbuf *string, const char *format, ...);
VARNAM_EXPORT int strbuf_addfln(struct strbuf *string, const char *format, ...);
VARNAM_EXPORT int strbuf_addvf(struct strbuf *string, const char *format, va_list args);
VARNAM_EXPORT void strbuf_destroy(void *s);
VARNAM_EXPORT char* strbuf_detach(struct strbuf *string);
VARNAM_EXPORT const char* strbuf_to_s(struct strbuf *string);
VARNAM_EXPORT struct varray_t* strbuf_chars(strbuf *b);
VARNAM_EXPORT void strbuf_clear(struct strbuf *string);
VARNAM_EXPORT int strbuf_is_blank(struct strbuf *string);
VARNAM_EXPORT int strbuf_is_eq(struct strbuf *string, const char *value);
VARNAM_EXPORT int strbuf_endswith(struct strbuf *string, const char *str);
VARNAM_EXPORT bool strbuf_remove_from_first(struct strbuf *string, const char *toremove);
VARNAM_EXPORT bool strbuf_remove_from_last(struct strbuf *string, const char *toremove);
VARNAM_EXPORT bool strbuf_replace(strbuf *string, const char *rep, const char *with);
VARNAM_EXPORT char* strbuf_get_last_unicode_char(strbuf *word);
VARNAM_EXPORT char* portable_strdup(const char*);
VARNAM_EXPORT struct varray_t* strbuf_split(strbuf *string, varnam *handle, char delim);
VARNAM_EXPORT struct strbuf* get_pooled_string(varnam *handle);
VARNAM_EXPORT void return_string_to_pool (varnam *handle, strbuf* string);

VARNAM_EXPORT void *xmalloc(size_t size);
VARNAM_EXPORT void xfree (void *ptr);

void set_last_error(varnam *handle, const char *format, ...);
bool is_utf8(const char *string);
const char *ZWNJ();
const char *ZWJ();
char *trimwhitespace(char *str);
bool is_special_character(char c);

int get_stat(const char *pathname);
bool is_directory(const char *pathname);
bool is_path_exists(const char *pathname);

void*
lru_find_in_cache (vcache_entry **cache, char *key);

void
lru_add_to_cache (vcache_entry **cache, char *key, void *value, vcache_value_free_cb cb);

int
lru_key_exists (vcache_entry **cache, char *key);

/* Constants */
#define MAX_PATH_LENGTH 4096
#define MAX_PATTERN_LENGTH 20
#define V_PATHNAME_INVALID 0
#define V_PATHNAME_DIRECTORY 1
#define V_PATHNAME_FILE 2

#endif
