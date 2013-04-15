/* 01-transliteration.c
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

/* Contains test cases for basic transliteration. this test uses some test data rather than 
actual characters in the supported languages */

#include <stdio.h>
#include <string.h>
#include "../varnam.h"

static int rendering_vowels(varnam *handle)
{
    vword* word;
    varray *words = varray_init();

    varnam_transliterate(handle, "a", &words);

    if (varray_length(words) != 1) {
        printf("Expected transliterate to return 1 word but it returned %d words\n", varray_length(words));
        return 0;
    }

    word = varray_get(words, 0);
    if(strcmp(word->text, "a-value1") == 0) {
        return 0;
    }

    return 1;
}

static int rendering_dependent_vowels(varnam *handle)
{
    vword* word;
    varray *words = varray_init();

    varnam_transliterate(handle, "aaa", &words);

    if (varray_length(words) != 1) {
        printf("Expected transliterate to return 1 word but it returned %d words\n", varray_length(words));
        return 0;
    }

    word = varray_get(words, 0);
    if(strcmp(word->text, "aa-value1a-value2") == 0) {
        return 0;
    }

    return 1;
}

static int rendering_cancelation_character(varnam *handle)
{
    vword* word;
    varray *words = varray_init();

    varnam_transliterate(handle, "aa_a", &words);

    if (varray_length(words) != 1) {
        printf("Expected transliterate to return 1 word but it returned %d words\n", varray_length(words));
        return 0;
    }

    word = varray_get(words, 0);
    if(strcmp(word->text, "aa-value1a-value1") == 0) {
        return 0;
    }

    return 1;
}

static void log_fun(const char* msg)
{
    printf("%s\n", msg);
}

int basic_transliteration(int argc, char **argv)
{
    varnam *handle;
    int rc;
    char *msg;

    rc = varnam_init("output/01-t.vst", &handle, &msg);
    if(rc != VARNAM_SUCCESS) {
        printf("initialization failed - %s\n", msg);
        return 1;
    }

    rc = varnam_enable_logging(handle, VARNAM_LOG_DEBUG, &log_fun);

    rc = varnam_create_token (handle, "a", "a-value1", "a-value2", "", "", VARNAM_TOKEN_VOWEL, VARNAM_MATCH_EXACT, 0, 0, 0);
    if(rc != VARNAM_SUCCESS) {
        printf("Token creation failed - %s\n", varnam_get_last_error (handle));
        return 1;
    }

    rc = varnam_create_token (handle, "aa", "aa-value1", "aa-value2", "", "", VARNAM_TOKEN_VOWEL, VARNAM_MATCH_EXACT, 0, 0, 0);
    if(rc != VARNAM_SUCCESS) {
        printf("Token creation failed - %s\n", varnam_get_last_error (handle));
        return 1;
    }

    rc = varnam_create_token (handle, "_", "", "", "", "", VARNAM_TOKEN_NON_JOINER, VARNAM_MATCH_EXACT, 0, 0, 0);
    if(rc != VARNAM_SUCCESS) {
        printf("Token creation failed - %s\n", varnam_get_last_error (handle));
        return 1;
    }

    rc = rendering_vowels(handle);
    if(rc != VARNAM_SUCCESS) {
        printf("rendering dependent vowels is not proper - \n");
        return 1;
    }

    rc = rendering_dependent_vowels(handle);
    if(rc != VARNAM_SUCCESS) {
        printf("rendering dependent vowels is not proper - \n");
        return 1;
    }

    return 0;
}
