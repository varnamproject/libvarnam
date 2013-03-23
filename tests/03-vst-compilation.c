/* 03-vst-compilation.c
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

/* Contains test cases for initialization */

#include <stdio.h>
#include <string.h>
#include "../varnam.h"

#define return_on_error(rc)      \
    if (rc != VARNAM_SUCCESS)\
    {\
        printf("VARNAM_SUCCESS expected. Never got. %s", varnam_get_last_error(handle));\
        return 1;\
    }\


static void log_fun(const char* msg)
{
    printf("%s\n", msg);
}

int create_without_buffering()
{
    int rc;
    char *msg;
    varnam *handle;

    const char *filename = "output/03-without-buffering.vst";
    rc = varnam_init(filename, &handle, &msg);

    if (rc != VARNAM_SUCCESS)
    {
        printf("VARNAM_SUCCESS expected. Never got. %s", msg);
        return 1;
    }

    rc = varnam_create_token(handle, "pattern", "value1", "value2", "value3", "tag", VARNAM_TOKEN_VOWEL, VARNAM_MATCH_EXACT, 0, 0, 0);
    if (rc != VARNAM_SUCCESS)
    {
        printf("VARNAM_SUCCESS expected. Never got. %s", varnam_get_last_error(handle));
        return 1;
    }

    return 0;
}

int get_all_tokens()
{
    int rc, count = 0, i;
    char *msg;
    varnam *handle;
    struct token *current;
    varray *tokens;

    const char *filename = "output/03-get-all-tokens.vst";
    rc = varnam_init(filename, &handle, &msg);

    if (rc != VARNAM_SUCCESS)
    {
        printf("VARNAM_SUCCESS expected. Never got. %s", msg);
        return 1;
    }

    rc = varnam_create_token(handle, "pattern", "value1", "value2","","", VARNAM_TOKEN_VOWEL, VARNAM_MATCH_EXACT, 0, 0, 0);
    if (rc != VARNAM_SUCCESS)
    {
        printf("VARNAM_SUCCESS expected. Never got. %s", varnam_get_last_error(handle));
        return 1;
    }

    rc = varnam_create_token(handle, "pattern1", "value11", "value21", "","", VARNAM_TOKEN_VOWEL, VARNAM_MATCH_EXACT, 0, 0, 0);
    if (rc != VARNAM_SUCCESS)
    {
        printf("VARNAM_SUCCESS expected. Never got. %s", varnam_get_last_error(handle));
        return 1;
    }

    rc = varnam_create_token(handle, "pattern2", "value12", "value22", "", "",VARNAM_TOKEN_VOWEL, VARNAM_MATCH_EXACT, 0, 0, 0);
    if (rc != VARNAM_SUCCESS)
    {
        printf("VARNAM_SUCCESS expected. Never got. %s", varnam_get_last_error(handle));
        return 1;
    }

    varnam_config(handle, VARNAM_CONFIG_USE_DEAD_CONSONANTS, 0);

    rc = varnam_create_token(handle, "p", "v", "v", "", "",VARNAM_TOKEN_CONSONANT, VARNAM_MATCH_EXACT, 0, 0, 0);
    if (rc != VARNAM_SUCCESS)
    {
        printf("VARNAM_SUCCESS expected. Never got. %s", varnam_get_last_error(handle));
        return 1;
    }

    rc = varnam_create_token(handle, "p", "v12", "v", "", "", VARNAM_TOKEN_CONSONANT, VARNAM_MATCH_POSSIBILITY, 0, 0, 0);
    if (rc != VARNAM_SUCCESS)
    {
        printf("VARNAM_SUCCESS expected. Never got. %s", varnam_get_last_error(handle));
        return 1;
    }

    rc = varnam_get_all_tokens(handle, VARNAM_TOKEN_VOWEL, &tokens);
    if (rc != VARNAM_SUCCESS)
    {
        printf("VARNAM_SUCCESS expected. Never got. %s", varnam_get_last_error(handle));
        return 1;
    }

    for (i = 0; i < varray_length (tokens); i++)
    {
        current = (struct token*) varray_get (tokens, i);
        printf ("%s => [%s, %s]\n", current->pattern, current->value1, current->value2);
        ++count;
    }

    if (count != 3)
    {
        printf("3 vowels expected, but got %d", count);
        return 1;
    }

    rc = varnam_get_all_tokens(handle, VARNAM_TOKEN_CONSONANT, &tokens);
    if (rc != VARNAM_SUCCESS)
    {
        printf("VARNAM_SUCCESS expected. Never got. %s", varnam_get_last_error(handle));
        return 1;
    }

    count = 0;

    for (i = 0; i < varray_length (tokens); i++)
    {
        current = (struct token*) varray_get (tokens, i);
        printf ("%s => [%s, %s]\n", current->pattern, current->value1, current->value2);
        ++count;
    }

    if (count != 2)
    {
        printf("2 consonants expected, but got %d", count);
        return 1;
    }

    return 0;
}

int ignore_duplicates()
{
    int rc;
    char *msg;
    varnam *handle;

    const char *filename = "output/03-ignore-duplicates.vst";
    rc = varnam_init(filename, &handle, &msg);

    if (rc != VARNAM_SUCCESS)
    {
        printf("VARNAM_SUCCESS expected. Never got. %s", msg);
        return 1;
    }

    varnam_enable_logging (handle, VARNAM_LOG_DEFAULT, &log_fun);
    varnam_config (handle, VARNAM_CONFIG_IGNORE_DUPLICATE_TOKEN, 0);

    rc = varnam_create_token(handle, "pattern", "value1", "value2", "", "", VARNAM_TOKEN_VOWEL, VARNAM_MATCH_EXACT, 0, 0, 0);
    if (rc != VARNAM_SUCCESS)
    {
        printf("VARNAM_SUCCESS expected. Never got. %s", varnam_get_last_error(handle));
        return 1;
    }

    /* Creating again. This should fail */
    rc = varnam_create_token(handle, "pattern", "value1", "value2", "", "", VARNAM_TOKEN_VOWEL, VARNAM_MATCH_EXACT, 0, 0, 0);
    if (rc == VARNAM_SUCCESS)
    {
        printf("Duplicate check is not working. %s", varnam_get_last_error(handle));
        return 1;
    }

    /* Turning off duplicate failure */
    rc = varnam_config (handle, VARNAM_CONFIG_IGNORE_DUPLICATE_TOKEN, 1);
    if (rc != VARNAM_SUCCESS)
    {
        printf("VARNAM_SUCCESS expected. Never got. %s", varnam_get_last_error(handle));
        return 1;
    }

    /* Creating again. This should ignore the duplicates */
    rc = varnam_create_token(handle, "pattern", "value1", "value2", "", "", VARNAM_TOKEN_VOWEL, VARNAM_MATCH_EXACT, 0, 0, 0);
    if (rc != VARNAM_SUCCESS)
    {
        printf("Duplicate ignore is not working. %s", varnam_get_last_error(handle));
        return 1;
    }

    return 0;
}

int auto_create_dead_consonants()
{
    int rc, i;
    char *msg;
    varnam *handle;
    varray *tokens;
    vtoken *token;

    const char *filename = "output/03-auto_create_dead_co.vst";
    rc = varnam_init(filename, &handle, &msg);

    varnam_config (handle, VARNAM_CONFIG_USE_DEAD_CONSONANTS, 1);

    if (rc != VARNAM_SUCCESS)
    {
        printf("VARNAM_SUCCESS expected. Never got. %s", msg);
        return 1;
    }

    rc = varnam_create_token(handle, "~", "്", NULL, "tag", "",VARNAM_TOKEN_VIRAMA, VARNAM_MATCH_EXACT, 1, 0, 0);
    if (rc != VARNAM_SUCCESS)
    {
        printf("VARNAM_SUCCESS expected. Never got. %s", varnam_get_last_error(handle));
        return 1;
    }

    rc = varnam_create_token(handle, "ka", "ക", NULL, "tag", "", VARNAM_TOKEN_CONSONANT, VARNAM_MATCH_EXACT, 1, 0, 0);
    if (rc != VARNAM_SUCCESS)
    {
        printf("VARNAM_SUCCESS expected. Never got. %s", varnam_get_last_error(handle));
        return 1;
    }

    rc = varnam_create_token(handle, "p", "പ്", NULL, "tag", "value3", VARNAM_TOKEN_CONSONANT, VARNAM_MATCH_EXACT, 1, 0, 0);
    if (rc != VARNAM_SUCCESS)
    {
        printf("VARNAM_SUCCESS expected. Never got. %s", varnam_get_last_error(handle));
        return 1;
    }

    varnam_flush_buffer(handle);

    rc = varnam_get_all_tokens (handle, VARNAM_TOKEN_DEAD_CONSONANT, &tokens);
    if (rc != VARNAM_SUCCESS)
    {
        printf("VARNAM_SUCCESS expected. Never got. %s", varnam_get_last_error(handle));
        return 1;
    }

    for (i = 0; i < varray_length (tokens); i++) {
        token = varray_get (tokens, i);
        if (strcmp(token->pattern, "k") == 0) {
            return 0;
        }
    }

    printf ("Expected dead consonants, found none\n");
    return 1;
}

int create_with_buffering()
{
    int rc;
    char *msg;
    varnam *handle;

    const char *filename = "output/03-with-buffering.vst";
    rc = varnam_init(filename, &handle, &msg);

    if (rc != VARNAM_SUCCESS)
    {
        printf("VARNAM_SUCCESS expected. Never got. %s", msg);
        return 1;
    }

    rc = varnam_create_token(handle, "pattern", "value1", "value2", "", "value3", VARNAM_TOKEN_VOWEL, VARNAM_MATCH_EXACT, 1, 0, 0);
    if (rc != VARNAM_SUCCESS)
    {
        printf("VARNAM_SUCCESS expected. Never got. %s", varnam_get_last_error(handle));
        return 1;
    }

    rc = varnam_flush_buffer(handle);
    if (rc != VARNAM_SUCCESS)
    {
        printf("VARNAM_SUCCESS expected. Never got. %s", varnam_get_last_error(handle));
        return 1;
    }

    return 0;
}

int create_exact_match_duplicates()
{
    int rc;
    char *msg;
    varnam *handle;

    const char *filename = "output/03-duplicates-exact.vst";
    rc = varnam_init(filename, &handle, &msg);

    if (rc != VARNAM_SUCCESS)
    {
        printf("VARNAM_SUCCESS expected. Never got. %s", msg);
        return 1;
    }

    rc = varnam_create_token(handle, "pattern", "value1", "value2", "", "value3", VARNAM_TOKEN_VOWEL, VARNAM_MATCH_EXACT, 0, 0, 0);
    if (rc != VARNAM_SUCCESS)
    {
        printf("VARNAM_SUCCESS expected. Never got. %s", varnam_get_last_error(handle));
        return 1;
    }

    varnam_config(handle, VARNAM_CONFIG_IGNORE_DUPLICATE_TOKEN, 0);
    rc = varnam_create_token(handle, "pattern ", "value1", "value2", "", "value3", VARNAM_TOKEN_VOWEL, VARNAM_MATCH_EXACT, 0, 0, 0);
    if (rc != VARNAM_ERROR)
    {
        printf("VARNAM_ERROR expected. Never got. Looks like duplicate exact match token are allowed %s", varnam_get_last_error(handle));
        return 1;
    }

    return 0;
}

int create_possibility_match_duplicates()
{
    int rc;
    char *msg;
    varnam *handle;

    const char *filename = "output/03-duplicates-possibility.vst";
    rc = varnam_init(filename, &handle, &msg);

    varnam_config (handle, VARNAM_CONFIG_IGNORE_DUPLICATE_TOKEN, 0);

    if (rc != VARNAM_SUCCESS)
    {
        printf("VARNAM_SUCCESS expected. Never got. %s", msg);
        return 1;
    }

    rc = varnam_create_token(handle, "pattern", "value1", "value2", "", "value3", VARNAM_TOKEN_VOWEL, VARNAM_MATCH_POSSIBILITY, 0, 0, 0);
    if (rc != VARNAM_SUCCESS)
    {
        printf("VARNAM_SUCCESS expected. Never got. %s", varnam_get_last_error(handle));
        return 1;
    }

    /* This should be allowed as it is a different value */
    rc = varnam_create_token(handle, "pattern ", "value11", "value22", "", "value3", VARNAM_TOKEN_VOWEL, VARNAM_MATCH_POSSIBILITY, 0, 0, 0);
    if (rc != VARNAM_SUCCESS)
    {
        printf("Creating possible matches for same pattern with different value failing. %s", varnam_get_last_error(handle));
        return 1;
    }

    rc = varnam_create_token(handle, "pattern ", "value1", "value2", "", "value3", VARNAM_TOKEN_VOWEL, VARNAM_MATCH_POSSIBILITY, 0, 0, 0);
    if (rc != VARNAM_ERROR)
    {
        printf("Creating possible matches for same pattern and same value again is allowed. %s", varnam_get_last_error(handle));
        return 1;
    }

    return 0;
}

int only_valid_matchtypes()
{
    int rc;
    char *msg;
    varnam *handle;

    const char *filename = "output/03-valid-match-types.vst";
    rc = varnam_init(filename, &handle, &msg);

    if (rc != VARNAM_SUCCESS)
    {
        printf("VARNAM_SUCCESS expected. Never got. %s", msg);
        return 1;
    }

    rc = varnam_create_token(handle, "pattern", "value1", "value2", "", "value3", VARNAM_TOKEN_VOWEL, 10, 0, 0, 0);
    if (rc != VARNAM_ERROR)
    {
        printf("VARNAM_ERROR expected. Never got. %s", varnam_get_last_error(handle));
        return 1;
    }

    return 0;
}

int maxlength_check()
{
    int rc, i;
    char *msg; char pattern[VARNAM_SYMBOL_MAX + 2]; char value1[VARNAM_SYMBOL_MAX + 2]; char value2[VARNAM_SYMBOL_MAX + 2];
    varnam *handle;

    const char *filename = "output/03-max-length.vst";
    rc = varnam_init(filename, &handle, &msg);

    for (i = 0; i < VARNAM_SYMBOL_MAX + 1; i++)
    {
        pattern[i] = 'a';
        value1[i] = 'a';
        value2[i] = 'a';
    }

    pattern[VARNAM_SYMBOL_MAX + 2] = '\0';
    value1[VARNAM_SYMBOL_MAX + 2] = '\0';
    value2[VARNAM_SYMBOL_MAX + 2] = '\0';

    if (rc != VARNAM_SUCCESS)
    {
        printf("VARNAM_SUCCESS expected. Never got. %s", msg);
        return 1;
    }

    rc = varnam_create_token(handle, pattern, value1, value2, "", "value3", VARNAM_TOKEN_VOWEL, VARNAM_MATCH_EXACT, 0, 0, 0);
    if (rc != VARNAM_ARGS_ERROR)
    {
        printf("VARNAM_ARGS_ERROR expected. Never got. %s", varnam_get_last_error(handle));
        return 1;
    }

    return 0;
}

int test_vst_file_creation(int argc, char **argv)
{
    int rc;

    rc = create_without_buffering();
    if (rc)
        return 1;

    rc = create_with_buffering();
    if (rc)
        return 1;

    rc = create_exact_match_duplicates();
    if (rc)
        return 1;

    rc = create_possibility_match_duplicates();
    if (rc)
        return 1;

    rc = maxlength_check();
    if (rc)
        return 1;

    rc = auto_create_dead_consonants();
    if (rc)
        return 1;

    rc = ignore_duplicates();
    if (rc)
        return 1;

    rc = get_all_tokens();
    if (rc)
        return 1;

    return 0;
}

