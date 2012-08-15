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

int strbuf_test(int argc, char **argv)
{
    int rc;
    rc = replace_string ();
    if (rc) return rc;

    rc = replace_string_should_be_case_sensitive ();
    if (rc) return rc;

    rc = replace_should_word_for_utf8_strings ();
    if (rc) return rc;

    return VARNAM_SUCCESS;
}
