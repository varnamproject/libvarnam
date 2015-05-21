/* Dynamically growing string buffer
 *
 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */


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

	if (string->allocated < 2) {
		toallocate = 2;
	}
	else {
		toallocate = string->allocated + (string->allocated / 2);
	}

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

struct strbuf* strbuf_create_from(const char* value)
{
	size_t len = strlen(value);
	strbuf *buf = strbuf_init(len);
	strbuf_add(buf, value);
	return buf;
}

int strbuf_is_eq(struct strbuf *string, const char *value)
{
	if (string == NULL) return 0;
	return strcmp(strbuf_to_s(string), value) == 0;
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

int strbuf_addfln(struct strbuf *string, const char *format, ...)
{
	int rc;
	va_list args;

	va_start(args, format);
	rc = strbuf_addvf(string, format, args);
	va_end(args);

	if (rc)
		return strbuf_add (string, "\n");

	return rc;
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
				portable_snprintf(fmtbuf, 256, "%d", i);
				if(!strbuf_add(string, fmtbuf))
					return 0;
				break;

			case 'f':
				f = va_arg(args, double);
				portable_snprintf(fmtbuf, 256, "%f", f);
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

	return 1;

}

/*
 * Gets each unicode character in this string
 * returned result should be destroyed
 * */
	varray*
strbuf_chars(strbuf *b)
{
	const unsigned char *ustring; const char *inputcopy;
	int bytes_read = 0;
	varray *chars;
	strbuf *tmp;

	inputcopy = b->buffer;
	chars = varray_init();
	while (*inputcopy != '\0') {
		READ_A_UTF8_CHAR (ustring, inputcopy, bytes_read);
		tmp = strbuf_init(8);
		strbuf_add_bytes (tmp, inputcopy - bytes_read, bytes_read);
		varray_push (chars, strbuf_detach(tmp));
		bytes_read = 0;
	}
	return chars;
}

/* Returns the last unicode character of the word
	 Returned result should be destroyed
	 */
/* Returns the last unicode character of the word
	 Returned result should be destroyed
	 */
char*
strbuf_get_last_unicode_char(strbuf *word)
{
	varray *characters = NULL;
	char *lastUnicodeChar = NULL;
	characters = strbuf_chars(word);

	if (varray_is_empty (characters)) {
		varray_free (characters, NULL);
		return NULL;
	}

	lastUnicodeChar = portable_strdup ((const char*) varray_get(characters, varray_length(characters) - 1));
	varray_free(characters, &free);
	/*ending should be freed in the calling function*/
	return lastUnicodeChar;
}

void strbuf_destroy(void *s)
{
	strbuf *string;

	if (s == NULL)
		return;

	string = (strbuf*) s;
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
	char substring[100] = "";

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

	bool
strbuf_replace(strbuf *string, const char *rep, const char *with)
{
	/* Credits goes to jmucchiello (http://stackoverflow.com/questions/779875/what-is-the-function-to-replace-string-in-c) */

	char *result; /* the return string */
	char *ins;		/* the next insert point */
	char *tmp;		/* varies */
	size_t len_rep;	/* length of rep */
	size_t len_with; /* length of with */
	size_t len_front; /* distance between rep and end of last rep */
	size_t count;		/* number of replacements */
	char *orig;

	orig = string->buffer;

	if (!orig)
		return false;
	if (!rep || !(len_rep = strlen(rep)))
		return false;
	if (!(ins = strstr(orig, rep)))
		return false;
	if (!with)
		with = "";
	len_with = strlen(with);

	for (count = 0; (tmp = strstr(ins, rep)); ++count) {
		ins = tmp + len_rep;
	}

	/* first time through the loop, all the variable are set correctly
	// from here on,
	//		tmp points to the end of the result string
	//		ins points to the next occurrence of rep in orig
	//		orig points to the remainder of orig after "end of rep"*/
	tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

	if (!result)
		return false;

	while (count--) {
		ins = strstr(orig, rep);
		len_front = ins - orig;
		tmp = strncpy(tmp, orig, len_front) + len_front;
		tmp = strcpy(tmp, with) + len_with;
		orig += len_front + len_rep; /* move to next "end of rep" */
	}
	strcpy(tmp, orig);

	strbuf_clear (string);
	strbuf_add (string, result);
	xfree (result);

	return true;
}

	varray*
strbuf_split(strbuf *string, varnam *handle, char delim)
{
	char *orig, *copy;
	int bytes = 0;
	varray *result = get_pooled_array(handle);
	strbuf *string_part;

	if (string == NULL || string->length <= 0)
		return NULL;

	orig = copy = string->buffer;
	while(*orig)
	{
		if (*orig == delim) {
			if (bytes > 0) {
				string_part = get_pooled_string(handle);
				strbuf_add_bytes (string_part, copy, bytes);
				varray_push (result, string_part);
				bytes = 0;
			}
			copy = ++orig;
		}
		else {
			++bytes;
			++orig;
		}
	}

	if (bytes > 0) {
		string_part = get_pooled_string(handle);
		strbuf_add_bytes (string_part, copy, bytes);
		varray_push (result, string_part);
	}

	return result;
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

	void
return_string_to_pool (varnam *handle, strbuf* string)
{
	if (v_->strings_pool == NULL)
		return;

	vpool_return (v_->strings_pool, string);
}


