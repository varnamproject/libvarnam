/* 03-vst-compilation.c
 *
 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */


/* Contains test cases for initialization */

#include <stdio.h>
#include <string.h>
#include "../varnam.h"
#include <check.h>
#include "testcases.h"


START_TEST (create_without_buffering)
{
    int rc;

    rc = varnam_create_token(varnam_instance, "pattern", "value1", "value2", "value3", "tag", VARNAM_TOKEN_VOWEL, VARNAM_MATCH_EXACT, 0, 0, 0);
    assert_success (rc);
}
END_TEST

START_TEST (create_with_buffering)
{
    int rc;

    rc = varnam_create_token(varnam_instance, "pattern", "value1", "value2", "", "value3", VARNAM_TOKEN_VOWEL, VARNAM_MATCH_EXACT, 1, 0, 0);
    assert_success (rc);

    rc = varnam_flush_buffer(varnam_instance);
    assert_success (rc);
}
END_TEST

START_TEST (get_all_tokens)
{
    int rc;
    varray *tokens;

    rc = varnam_create_token(varnam_instance, "pattern", "value1", "value2","","", VARNAM_TOKEN_VOWEL, VARNAM_MATCH_EXACT, 0, 0, 0);
    assert_success (rc);

    rc = varnam_create_token(varnam_instance, "pattern1", "value11", "value21", "","", VARNAM_TOKEN_VOWEL, VARNAM_MATCH_EXACT, 0, 0, 0);
    assert_success (rc);

    rc = varnam_create_token(varnam_instance, "pattern2", "value12", "value22", "", "",VARNAM_TOKEN_VOWEL, VARNAM_MATCH_EXACT, 0, 0, 0);
    assert_success (rc);

    varnam_config(varnam_instance, VARNAM_CONFIG_USE_DEAD_CONSONANTS, 0);

    rc = varnam_create_token(varnam_instance, "p", "v", "v", "", "",VARNAM_TOKEN_CONSONANT, VARNAM_MATCH_EXACT, 0, 0, 0);
    assert_success (rc);

    rc = varnam_create_token(varnam_instance, "p", "v12", "v", "", "", VARNAM_TOKEN_CONSONANT, VARNAM_MATCH_POSSIBILITY, 0, 0, 0);
    assert_success (rc);

    rc = varnam_get_all_tokens(varnam_instance, VARNAM_TOKEN_VOWEL, &tokens);
    assert_success (rc);
    ck_assert_int_eq (3, varray_length (tokens));

    rc = varnam_get_all_tokens(varnam_instance, VARNAM_TOKEN_CONSONANT, &tokens);
    assert_success (rc);
    ck_assert_int_eq (2, varray_length (tokens));
}
END_TEST

START_TEST (prefix_tree)
{
    int rc, i;
    struct token *current;
    varray *tokens;

    rc = varnam_create_token(varnam_instance, "a", "value1", "value2","","", VARNAM_TOKEN_VOWEL, VARNAM_MATCH_EXACT, 0, 0, 0);
    assert_success (rc);

    rc = varnam_create_token(varnam_instance, "aa", "value11", "value21", "","", VARNAM_TOKEN_VOWEL, VARNAM_MATCH_EXACT, 0, 0, 0);
    assert_success (rc);

    rc = varnam_create_token(varnam_instance, "b", "value12", "value22", "", "",VARNAM_TOKEN_VOWEL, VARNAM_MATCH_EXACT, 0, 0, 0);
    assert_success (rc);

    rc = varnam_create_token(varnam_instance, "ba", "value12", "value22", "", "",VARNAM_TOKEN_VOWEL, VARNAM_MATCH_EXACT, 0, 0, 0);
    assert_success (rc);

    rc = varnam_get_all_tokens(varnam_instance, VARNAM_TOKEN_VOWEL, &tokens);
    assert_success (rc);

    for (i = 0; i < varray_length (tokens); i++) {
        current = (vtoken*) varray_get (tokens, i);
        if (strcmp ("a", current->pattern) == 0) {
            ck_assert (current->flags & VARNAM_TOKEN_FLAGS_MORE_MATCHES_FOR_PATTERN);
            ck_assert (current->flags & VARNAM_TOKEN_FLAGS_MORE_MATCHES_FOR_VALUE);
        }
        if (strcmp ("aa", current->pattern) == 0) {
            ck_assert (current->flags == 0);
        }
        if (strcmp ("b", current->pattern) == 0) {
            ck_assert (current->flags & VARNAM_TOKEN_FLAGS_MORE_MATCHES_FOR_PATTERN);
        }
        if (strcmp ("ba", current->pattern) == 0) {
            ck_assert (current->flags == 0);
        }
    }
}
END_TEST

START_TEST (ignore_duplicates)
{
    int rc;

    varnam_config (varnam_instance, VARNAM_CONFIG_IGNORE_DUPLICATE_TOKEN, 0);

    rc = varnam_create_token(varnam_instance, "pattern", "value1", "value2", "", "", VARNAM_TOKEN_VOWEL, VARNAM_MATCH_EXACT, 0, 0, 0);
    assert_success (rc);

    /* Creating again. This should fail */
    rc = varnam_create_token(varnam_instance, "pattern", "value1", "value2", "", "", VARNAM_TOKEN_VOWEL, VARNAM_MATCH_EXACT, 0, 0, 0);
    assert_error (rc);

    /* Turning off duplicate failure */
    rc = varnam_config (varnam_instance, VARNAM_CONFIG_IGNORE_DUPLICATE_TOKEN, 1);
    assert_success (rc);

    /* Creating again. This should ignore the duplicates */
    rc = varnam_create_token(varnam_instance, "pattern", "value1", "value2", "", "", VARNAM_TOKEN_VOWEL, VARNAM_MATCH_EXACT, 0, 0, 0);
    assert_success (rc);
}
END_TEST

START_TEST (auto_create_dead_consonants)
{
    int rc, i;
    varray *tokens;
    vtoken *token;

    varnam_config (varnam_instance, VARNAM_CONFIG_USE_DEAD_CONSONANTS, 1);

    rc = varnam_create_token(varnam_instance, "~", "്", NULL, "tag", "",VARNAM_TOKEN_VIRAMA, VARNAM_MATCH_EXACT, 1, 0, 0);
    assert_success (rc);

    rc = varnam_create_token(varnam_instance, "ka", "ക", NULL, "tag", "", VARNAM_TOKEN_CONSONANT, VARNAM_MATCH_EXACT, 1, 0, 0);
    assert_success (rc);

    rc = varnam_create_token(varnam_instance, "p", "പ്", NULL, "tag", "value3", VARNAM_TOKEN_CONSONANT, VARNAM_MATCH_EXACT, 1, 0, 0);
    assert_success (rc);

    varnam_flush_buffer(varnam_instance);

    rc = varnam_get_all_tokens (varnam_instance, VARNAM_TOKEN_DEAD_CONSONANT, &tokens);
    assert_success (rc);

    for (i = 0; i < varray_length (tokens); i++) {
        token = varray_get (tokens, i);
        if (strcmp(token->pattern, "k") == 0) {
            return;
        }
    }

    ck_abort_msg ("Expected dead consonants, found none\n");
}
END_TEST

START_TEST (create_exact_match_duplicates)
{
    int rc;

    rc = varnam_create_token(varnam_instance, "pattern", "value1", "value2", "", "value3", VARNAM_TOKEN_VOWEL, VARNAM_MATCH_EXACT, 0, 0, 0);
    assert_success (rc);

    varnam_config(varnam_instance, VARNAM_CONFIG_IGNORE_DUPLICATE_TOKEN, 0);
    rc = varnam_create_token(varnam_instance, "pattern ", "value1", "value2", "", "value3", VARNAM_TOKEN_VOWEL, VARNAM_MATCH_EXACT, 0, 0, 0);
    if (rc != VARNAM_ERROR)
    {
        ck_abort_msg ("VARNAM_ERROR expected. Never got. Looks like duplicate exact match token are allowed");
    }
}
END_TEST

START_TEST (create_possibility_match_duplicates)
{
    int rc;

    varnam_config (varnam_instance, VARNAM_CONFIG_IGNORE_DUPLICATE_TOKEN, 0);

    rc = varnam_create_token(varnam_instance, "pattern", "value1", "value2", "", "value3", VARNAM_TOKEN_VOWEL, VARNAM_MATCH_POSSIBILITY, 0, 0, 0);
    assert_success (rc);

    /* This should be allowed as it is a different value */
    rc = varnam_create_token(varnam_instance, "pattern ", "value11", "value22", "", "value3", VARNAM_TOKEN_VOWEL, VARNAM_MATCH_POSSIBILITY, 0, 0, 0);
    assert_success (rc);

    rc = varnam_create_token(varnam_instance, "pattern ", "value1", "value2", "", "value3", VARNAM_TOKEN_VOWEL, VARNAM_MATCH_POSSIBILITY, 0, 0, 0);
    if (rc != VARNAM_ERROR)
    {
        ck_abort_msg ("Creating possible matches for same pattern and same value again is allowed.");
    }
}
END_TEST

START_TEST (only_valid_matchtypes)
{
    int rc;

    rc = varnam_create_token(varnam_instance, "pattern", "value1", "value2", "", "value3", VARNAM_TOKEN_VOWEL, 10, 0, 0, 0);
    if (rc != VARNAM_ARGS_ERROR)
    {
        ck_abort_msg ("Looks like invalid match type value is working without error!!");
    }
}
END_TEST

START_TEST (maxlength_check)
{
    int rc, i;
    char pattern[VARNAM_SYMBOL_MAX + 2]; char value1[VARNAM_SYMBOL_MAX + 2]; char value2[VARNAM_SYMBOL_MAX + 2];

    for (i = 0; i < VARNAM_SYMBOL_MAX + 1; i++)
    {
        pattern[i] = 'a';
        value1[i] = 'a';
        value2[i] = 'a';
    }

    pattern[VARNAM_SYMBOL_MAX + 1] = '\0';
    value1[VARNAM_SYMBOL_MAX + 1] = '\0';
    value2[VARNAM_SYMBOL_MAX + 1] = '\0';

    rc = varnam_create_token(varnam_instance, pattern, value1, value2, "", "value3", VARNAM_TOKEN_VOWEL, VARNAM_MATCH_EXACT, 0, 0, 0);
    if (rc != VARNAM_ARGS_ERROR)
    {
        ck_abort_msg ("VARNAM_ARGS_ERROR expected. Never got. Looks like max length validation is off");
    }
}
END_TEST

TCase* get_token_creation_tests()
{
    TCase* tcase = tcase_create("transliteration");
    tcase_add_checked_fixture (tcase, setup, teardown);
    tcase_add_test (tcase, create_without_buffering);
    tcase_add_test (tcase, create_with_buffering);
    tcase_add_test (tcase, get_all_tokens);
    tcase_add_test (tcase, ignore_duplicates);
    tcase_add_test (tcase, auto_create_dead_consonants);
    tcase_add_test (tcase, create_exact_match_duplicates);
    tcase_add_test (tcase, create_possibility_match_duplicates);
    tcase_add_test (tcase, only_valid_matchtypes);
    tcase_add_test (tcase, maxlength_check);
    tcase_add_test (tcase, prefix_tree);
    return tcase;
}
