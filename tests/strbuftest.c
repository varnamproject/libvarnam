/* Tests strbuf
 *
 * Copyright (C) <year> <author>
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

#include "../varnam.h"
#include <stdio.h>
#include <string.h>

static varnam *handle;

int strbuf_formatted_strings()
{
    const char *expected = "Character a, Integer 10, String Navaneeth\n";
    struct strbuf *buffer = strbuf_init(100);
    strbuf_addf(buffer, "Character %c, Integer %d, String %s", 'a', 10, "Navaneeth");

    if (strcmp(expected, strbuf_to_s (buffer)) != 0)
    {
        printf ("Incorrect formatting. Expected '%s', but was '%s'\n", expected, strbuf_to_s(buffer));
        return 1;
    }

    return VARNAM_SUCCESS;
}

static int replace_string()
{
    strbuf *string = strbuf_init (10);
    strbuf_add (string, "Varnam program");
    strbuf_replace(string, "program", "software");
    if (strcmp(strbuf_to_s(string), "Varnam software") != 0) {
        printf ("Replace failed\n");
        return VARNAM_ERROR;
    }

    strbuf_destroy (string);
    return VARNAM_SUCCESS;
}

static int replace_string_should_be_case_sensitive()
{
    strbuf *string = strbuf_init (10);
    strbuf_add (string, "Varnam program");
    strbuf_replace(string, "varnam", "libvarnam");
    if (strcmp(strbuf_to_s(string), "libvarnam program") == 0) {
        printf ("Replace is not case sensitive\n");
        return VARNAM_ERROR;
    }

    strbuf_destroy (string);
    return VARNAM_SUCCESS;
}

static int replace_should_word_for_utf8_strings()
{
    strbuf *string = strbuf_init (10);
    strbuf_add (string, "അവന്‍");
    strbuf_replace(string, "ന്‍", "ൻ");
    if (strcmp(strbuf_to_s(string), "അവൻ") != 0) {
        printf ("Replace failed for utf8\n");
        return VARNAM_ERROR;
    }

    strbuf_destroy (string);
    return VARNAM_SUCCESS;
}

static int split_string()
{
    int i = 0;
    varray *tokens;
    strbuf *token;

    strbuf *string = strbuf_init (10);
    strbuf_add (string, "test\ttest\ttest\t\t\t\t\ttest");
    tokens = strbuf_split(string, handle, '\t');

    if (tokens == NULL || varray_length(tokens) != 4) {
        printf("4 elements expected.\n");
        return VARNAM_ERROR;
    }

    for(i = 0; i < varray_length(tokens); i++) {
        token = varray_get(tokens, i);
        if (strcmp("test", strbuf_to_s(token)) != 0) {
            printf("Expected test. but was %s\n", strbuf_to_s(token));
            return VARNAM_ERROR;
        }
    }

    strbuf_clear (string);
    strbuf_add (string, "test test test    ");
    tokens = strbuf_split(string, handle, ' ');

    if (tokens == NULL || varray_length(tokens) != 3) {
        printf("3 elements expected.\n");
        return VARNAM_ERROR;
    }

    for(i = 0; i < varray_length(tokens); i++) {
        token = varray_get(tokens, i);
        if (strcmp("test", strbuf_to_s(token)) != 0) {
            printf("Expected test. but was %s\n", strbuf_to_s(token));
            return VARNAM_ERROR;
        }
    }

    strbuf_clear (string);
    strbuf_add (string, "testtesttest");
    tokens = strbuf_split(string, handle, 't');

    if (tokens == NULL || varray_length(tokens) != 3) {
        printf("3 elements expected.\n");
        return VARNAM_ERROR;
    }

    for(i = 0; i < varray_length(tokens); i++) {
        token = varray_get(tokens, i);
        if (strcmp("es", strbuf_to_s(token)) != 0) {
            printf("Expected es. but was %s\n", strbuf_to_s(token));
            return VARNAM_ERROR;
        }
    }

    strbuf_destroy (string);
    return VARNAM_SUCCESS;
}

int strbuf_test(int argc, char **argv)
{
    int rc;
    char *msg;

    const char *filename = "output/strbuftest.vst";
    rc = varnam_init(filename, &handle, &msg);

    if (rc != VARNAM_SUCCESS)
    {
        printf("VARNAM_SUCCESS expected. Never got. %s", msg);
        return 1;
    }

    rc = split_string();
    if(rc) return rc;

    rc = replace_string ();
    if (rc) return rc;

    rc = replace_string_should_be_case_sensitive ();
    if (rc) return rc;

    rc = replace_should_word_for_utf8_strings ();
    if (rc) return rc;

    return VARNAM_SUCCESS;
}
