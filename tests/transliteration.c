/*
 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */


#include <check.h>
#include "testcases.h"

static void 
setup_data()
{
    int rc;

    rc = varnam_create_token (varnam_instance, "a", "a-value1", "a-value2", 
            "", "", VARNAM_TOKEN_VOWEL, VARNAM_MATCH_EXACT, 0, 0, 0);
    assert_success (rc);

    rc = varnam_create_token (varnam_instance, "aa", "aa-value1", "aa-value2", 
            "", "", VARNAM_TOKEN_VOWEL, VARNAM_MATCH_EXACT, 0, 0, 0);
    assert_success (rc);

    rc = varnam_create_token (varnam_instance, "e", "e-value1", "e-value2", 
            "", "", VARNAM_TOKEN_VOWEL, VARNAM_MATCH_EXACT, 0, 0, 0);
    assert_success (rc);

    rc = varnam_create_token (varnam_instance, "k", "k-value1", "k-value2", 
            "", "", VARNAM_TOKEN_CONSONANT, VARNAM_MATCH_EXACT, 0, 0, 0);
    assert_success (rc);

    rc = varnam_create_token (varnam_instance, "kh", "kh-value1", "kh-value2", 
            "", "", VARNAM_TOKEN_CONSONANT, VARNAM_MATCH_EXACT, 0, 0, 0);
    assert_success (rc);

    rc = varnam_create_token (varnam_instance, "0", "०", "", 
            "", "", VARNAM_TOKEN_NUMBER, VARNAM_MATCH_EXACT, 0, 0, 0);
    assert_success (rc);

    rc = varnam_create_token (varnam_instance, "1", "१", "", 
            "", "", VARNAM_TOKEN_NUMBER, VARNAM_MATCH_EXACT, 0, 0, 0);
    assert_success (rc);

    rc = varnam_create_token (varnam_instance, "_", "", "", 
            "", "", VARNAM_TOKEN_NON_JOINER, VARNAM_MATCH_EXACT, 0, 0, 0);
    assert_success (rc);
}

START_TEST (basic_transliteration)
{
    vword* word;
    int rc;
    varray *words;

    rc = varnam_transliterate (varnam_instance, "aek", &words);
    assert_success (rc);
    ck_assert_int_eq (varray_length(words), 1);

    word = varray_get(words, 0);
    ck_assert_str_eq (word->text, "a-value1e-value2k-value1");
}
END_TEST

START_TEST (dependent_vowel_rendering)
{
    int rc;
    vword* word;
    varray *words;

    rc = varnam_transliterate(varnam_instance, "aaa", &words);
    assert_success (rc);
    ck_assert_int_eq (varray_length(words), 1);

    word = varray_get(words, 0);
    ck_assert_str_eq (word->text, "aa-value1a-value2");
}
END_TEST

START_TEST (cancellation_character_should_force_independent_vowel_form)
{
    int rc;
    vword* word;
    varray *words;

    rc = varnam_transliterate(varnam_instance, "aa_a", &words);
    assert_success (rc);
    ck_assert_int_eq (varray_length(words), 1);

    word = varray_get(words, 0);
    ck_assert_str_eq (word->text, "aa-value1a-value1");
}
END_TEST

START_TEST (indic_digit_rendering)
{
    int rc;
    vword* word;
    varray *words;

    rc = varnam_transliterate (varnam_instance, "01", &words);
    assert_success (rc);
    ck_assert_int_eq (varray_length (words), 1);
    word = varray_get (words, 0);
    ck_assert_str_eq (word->text, "01");

    rc = varnam_config (varnam_instance, VARNAM_CONFIG_USE_INDIC_DIGITS, 1);
    assert_success (rc);

    rc = varnam_transliterate (varnam_instance, "01", &words);
    assert_success (rc);
    ck_assert_int_eq (varray_length (words), 1);
    word = varray_get (words, 0);
    ck_assert_str_eq (word->text, "०१");
}
END_TEST

TCase* get_transliteration_tests()
{
    TCase* tcase = tcase_create("transliteration");
    tcase_add_checked_fixture (tcase, setup, teardown);
    tcase_add_checked_fixture (tcase, setup_data, NULL);
    tcase_add_test (tcase, basic_transliteration);
    tcase_add_test (tcase, dependent_vowel_rendering);
    tcase_add_test (tcase, cancellation_character_should_force_independent_vowel_form);
    tcase_add_test (tcase, indic_digit_rendering);
    return tcase;
}
