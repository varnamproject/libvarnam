/* Utility functions
 *
 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "util.h"
#include "vtypes.h"

/**
 * substr(str,start,length,output) writes length characters of str beginning with start to substring.
 * start is is 1-indexed and string should be valid UTF8.
 **/
void
substr(char *substring, const char *string, int start, int len)
{
  unsigned int bytes, i;
  const unsigned char *str2, *input;
  unsigned char *sub;

  if(start <= 0) return;
  if(len <= 0) return;

  input = (const unsigned char*) string;
  sub = (unsigned char*) substring;
  --start;

  while( *input && start ) {
    SKIP_MULTI_BYTE_SEQUENCE(input);
    --start;
  }

  for(str2 = input; *str2 && len; len--) {
    SKIP_MULTI_BYTE_SEQUENCE(str2);
  }

  bytes = (unsigned int) (str2 - input);
  for(i = 0; i < bytes; i++) {
    *sub++ = *input++;
  }
  *sub = '\0';
}

/**
 * calculates length of the UTF8 encoded string.
 * length will be the total number of characters and not the bytes
 **/
int
utf8_length(const char *string)
{
  const unsigned char *ustring;
  int len;

  len = 0;
  ustring = (const unsigned char*) string;

  while( *ustring ) {
    ++len;
    SKIP_MULTI_BYTE_SEQUENCE(ustring);
  }

  return len;
}

int
utf8_ends_with(const char *buffer, const char *tocheck)
{
  size_t to_check_len, buffer_len, newlen;
  const char *buf;

  if(!tocheck) return 0;

  to_check_len = strlen(tocheck);
  buffer_len = strlen(buffer);

  if(buffer_len < to_check_len) return 0;

  newlen = (buffer_len - to_check_len);

  buf = buffer + newlen;

  return (strcmp(buf, tocheck) == 0);
}

/* return true if string1 starts with string2 */
int
startswith(const char *string1, const char *string2)
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

void*
xmalloc(size_t size)
{
  void *ret = malloc(size);
  return ret;
}

void
xfree (void *ptr)
{
  if(ptr)
    free(ptr);
}

void
set_last_error(varnam *handle, const char *format, ...)
{
  struct strbuf *last_error;
  va_list args;

  last_error = handle->internal->last_error;

  strbuf_clear (last_error);
  if (format != NULL)
  {
    va_start (args, format);
    strbuf_addvf (last_error, format, args);
    va_end (args);
  }
}

void varnam_debug(varnam* handle, const char *format, ...)
{
  va_list args;
  struct strbuf *log_message = handle->internal->log_message;

  if (handle->internal->log_level == VARNAM_LOG_DEBUG && handle->internal->log_callback != NULL)
  {
    strbuf_clear (log_message);

    va_start (args, format);
    strbuf_addvf(log_message, format, args);
    va_end(args);

    handle->internal->log_callback(log_message->buffer);
  }
}

void varnam_log(varnam* handle, const char *format, ...)
{
  va_list args;
  struct strbuf *log_message = handle->internal->log_message;

  if (handle->internal->log_callback != NULL)
  {
    strbuf_clear (log_message);

    va_start (args, format);
    strbuf_addvf(log_message, format, args);
    va_end(args);

    handle->internal->log_callback(log_message->buffer);
  }
}

bool is_utf8(const char *string)
{
  const unsigned char * bytes;
  if(!string)
    return 0;

  bytes = (const unsigned char*) string;
  while(*bytes)
  {
    if(     (/* ASCII */
          bytes[0] == 0x09 ||
          bytes[0] == 0x0A ||
          bytes[0] == 0x0D ||
          (0x20 <= bytes[0] && bytes[0] <= 0x7E)
          )
      ) {
      bytes += 1;
      continue;
    }

    if(     (/* non-overlong 2-byte */
          (0xC2 <= bytes[0] && bytes[0] <= 0xDF) &&
          (0x80 <= bytes[1] && bytes[1] <= 0xBF)
          )
      ) {
      bytes += 2;
      continue;
    }

    if(     (/* excluding overlongs */
          bytes[0] == 0xE0 &&
          (0xA0 <= bytes[1] && bytes[1] <= 0xBF) &&
          (0x80 <= bytes[2] && bytes[2] <= 0xBF)
          ) ||
        (/* straight 3-byte */
         ((0xE1 <= bytes[0] && bytes[0] <= 0xEC) ||
          bytes[0] == 0xEE ||
          bytes[0] == 0xEF) &&
         (0x80 <= bytes[1] && bytes[1] <= 0xBF) &&
         (0x80 <= bytes[2] && bytes[2] <= 0xBF)
        ) ||
        (/* excluding surrogates */
         bytes[0] == 0xED &&
         (0x80 <= bytes[1] && bytes[1] <= 0x9F) &&
         (0x80 <= bytes[2] && bytes[2] <= 0xBF)
        )
      ) {
      bytes += 3;
      continue;
    }

    if(     (/* planes 1-3 */
          bytes[0] == 0xF0 &&
          (0x90 <= bytes[1] && bytes[1] <= 0xBF) &&
          (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
          (0x80 <= bytes[3] && bytes[3] <= 0xBF)
          ) ||
        (/* planes 4-15 */
         (0xF1 <= bytes[0] && bytes[0] <= 0xF3) &&
         (0x80 <= bytes[1] && bytes[1] <= 0xBF) &&
         (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
         (0x80 <= bytes[3] && bytes[3] <= 0xBF)
        ) ||
        (/* plane 16 */
         bytes[0] == 0xF4 &&
         (0x80 <= bytes[1] && bytes[1] <= 0x8F) &&
         (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
         (0x80 <= bytes[3] && bytes[3] <= 0xBF)
        )
      ) {
      bytes += 4;
      continue;
    }

    return 0;
  }

  return 1;
}

const char zwnj[] = {'\xe2', '\x80', '\x8c', '\0'};
const char *ZWNJ()
{
  return zwnj;
}

const char zwj[] = {'\xe2', '\x80', '\x8d', '\0'};
const char *ZWJ()
{
  return zwj;
}

char *trimwhitespace(char *str)
{
  char *end;

  /* Trim leading space */
  while(isspace(*str)) str++;

  if(*str == 0)  /* All spaces? */
    return str;

  /* Trim trailing space */
  end = str + strlen(str) - 1;
  while(end > str && isspace(*end)) end--;

  /* Write new null terminator */
  *(end+1) = 0;

  return str;
}

static char special_chars[] = {'\n', '\t', '\r', ',', '.', '/', '<', '>', '?', ';', '\'', ':',
  '"', '[', ']', '{', '}', '~', '`', '!', '@', '#', '$', '%', '^',
  '&', '*', '(', ')', '-', '_', '+', '=', '\\', '|', ' '};

bool is_special_character(char c)
{
  int i;
  for (i = 0; i < ARRAY_SIZE(special_chars); i++)
  {
    if (c == special_chars[i])
      return true;
  }

  return false;
}

int get_stat(const char *pathname)
{
  struct stat info;

  if( stat( pathname, &info ) != 0 )
    return V_PATHNAME_INVALID;
#ifdef S_IFDIR
  else if( info.st_mode & S_IFDIR )
#else
  else if( S_ISDIR (info.st_mode) )
#endif
    return V_PATHNAME_DIRECTORY;

  return V_PATHNAME_FILE;
}

bool is_directory(const char *pathname)
{
  return get_stat (pathname) == V_PATHNAME_DIRECTORY; 
}

bool is_path_exists(const char *pathname)
{
  return get_stat (pathname) != V_PATHNAME_INVALID; 
}

