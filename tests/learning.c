/*
 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */


#include <stdio.h>
#include "testcases.h"
#include "../varnam.h"

static void
enable_suggestions()
{
    char* filename = get_unique_filename ();
    varnam_config (varnam_instance, VARNAM_CONFIG_ENABLE_SUGGESTIONS, filename);
    free (filename);
}

static void
setup_test_data()
{
    int rc;

    rc = varnam_create_token(varnam_instance, "a", "അ", "", "", "",
            VARNAM_TOKEN_VOWEL, VARNAM_MATCH_EXACT, 0, 0, 0);
    assert_success (rc);

    rc = varnam_create_token(varnam_instance, "aa", "ആ", "ാ", "", "",
            VARNAM_TOKEN_VOWEL, VARNAM_MATCH_EXACT, 0, 0, 0);
    assert_success (rc);

    rc = varnam_create_token(varnam_instance, "a", "ആ", "ാ", "", "",
            VARNAM_TOKEN_VOWEL, VARNAM_MATCH_POSSIBILITY, 0, 0, 0);
    assert_success (rc);

    rc = varnam_create_token(varnam_instance, "ka", "ക", "", "", "",
            VARNAM_TOKEN_CONSONANT, VARNAM_MATCH_EXACT, 0, 0, 0);
    assert_success (rc);

    rc = varnam_create_token(varnam_instance, "kha", "ഖ", "", "", "",
            VARNAM_TOKEN_CONSONANT, VARNAM_MATCH_EXACT, 0, 0, 0);
    assert_success (rc);

    rc = varnam_create_token(varnam_instance, "gha", "ഖ", "", "", "",
            VARNAM_TOKEN_CONSONANT, VARNAM_MATCH_POSSIBILITY, 0, 0, 0);
    assert_success (rc);

    rc = varnam_create_token (varnam_instance, "0", "०", "",
            "", "", VARNAM_TOKEN_NUMBER, VARNAM_MATCH_EXACT, 0, 0, 0);
    assert_success (rc);

    rc = varnam_create_token (varnam_instance, "1", "१", "",
            "", "", VARNAM_TOKEN_NUMBER, VARNAM_MATCH_EXACT, 0, 0, 0);
    assert_success (rc);
}

START_TEST (starting_and_trailing_special_chars_should_be_removed)
{
    int rc;
    rc = varnam_learn (varnam_instance, "''@'കഖഖ@");
    assert_success (rc);
}
END_TEST

START_TEST (words_with_unknown_letters_should_be_rejected)
{
    int rc = varnam_learn (varnam_instance, "test");
    if (rc != VARNAM_ERROR) {
        ck_abort_msg ("Expected return code to be VARNAM_ERROR");
    }
    ck_assert_str_eq (varnam_get_last_error (varnam_instance),
            "Can't process 't'. One or more characters in 'test' are not known");
}
END_TEST

START_TEST (basic_learning)
{
    int rc;
    varray *words;
    const char *word_to_learn = "കഖ";

    rc = varnam_learn (varnam_instance, word_to_learn);
    assert_success (rc);

    /* Here gha is a possibility. But since it is learned, it will be suggested back */
    rc = varnam_transliterate (varnam_instance, "kagha", &words);
    assert_success (rc);
    ck_assert_int_eq (varray_length (words), 2);

    ensure_word_list_contains (words, word_to_learn);
}
END_TEST

START_TEST (confidence_should_get_updated_for_existing_words)
{
    int rc;
    varray* words;
    vword* word;
    const char *word_to_learn = "കഖ";

    rc = varnam_learn (varnam_instance, word_to_learn);
    assert_success (rc);
    rc = varnam_learn (varnam_instance, word_to_learn);
    assert_success (rc);

    rc = varnam_transliterate (varnam_instance, "kagha", &words);
    assert_success (rc);
    ck_assert_int_eq (varray_length (words), 2);

    word = varray_get (words, 0);
    ck_assert_int_eq (word->confidence, 2);
}
END_TEST

START_TEST (words_with_repeating_characters_will_not_be_learned)
{
    int rc;
    strbuf *string;
    const char *word_to_learn = "കകകകകകക";

    rc = varnam_learn (varnam_instance, word_to_learn);
    assert_error (rc);
    string = strbuf_init (50);
    strbuf_addf (string, "'%s' looks incorrect. Not learning anything", word_to_learn);
    ck_assert_str_eq (varnam_get_last_error (varnam_instance), strbuf_to_s (string));
    strbuf_destroy (string);
}
END_TEST

START_TEST (numbers_will_be_ignored_while_learning)
{
    int rc;
    strbuf *string;

    rc = varnam_learn (varnam_instance, "01");
    assert_error (rc);
    string = strbuf_init (50);
    strbuf_add (string, "Can't process '0'. One or more characters in '01' are not known");
    ck_assert_str_eq (varnam_get_last_error (varnam_instance), strbuf_to_s (string));

    rc = varnam_learn (varnam_instance, "१०१");
    assert_error (rc);
    strbuf_clear (string);
    strbuf_add (string, "Nothing to learn from '१०१'");
    ck_assert_str_eq (varnam_get_last_error (varnam_instance), strbuf_to_s (string));
    strbuf_destroy (string);
}
END_TEST

START_TEST (is_known_word)
{
    int rc;
    const char *word_to_learn = "കഖ";
    const char *invalid_word = "ഖക";

    rc = varnam_learn (varnam_instance, word_to_learn);
    assert_success (rc);

    ck_assert_int_eq (varnam_is_known_word(varnam_instance, word_to_learn), 1);
    ck_assert_int_eq (varnam_is_known_word(varnam_instance, invalid_word), 0);
}
END_TEST

START_TEST (learn_from_multiple_open_handles)
{
    int rc;
    const char *word_to_learn = "കഖ";
    varnam *handle1, *handle2;
    char *msg;

    rc = varnam_init_from_id ("ml", &handle1, &msg);
    assert_success (rc);
    rc = varnam_init_from_id ("ml", &handle2, &msg);
    assert_success (rc);

    rc = varnam_learn (handle1, word_to_learn);
    assert_success (rc);

    rc = varnam_learn (handle2, word_to_learn);
    assert_success (rc);
}
END_TEST

TCase* get_learning_tests()
{
    TCase* tcase = tcase_create("learning");
    tcase_add_checked_fixture (tcase, setup, teardown);
    tcase_add_checked_fixture (tcase, setup_test_data, NULL);
    tcase_add_checked_fixture (tcase, enable_suggestions, NULL);
    tcase_add_test (tcase, starting_and_trailing_special_chars_should_be_removed);
    tcase_add_test (tcase, words_with_unknown_letters_should_be_rejected);
    tcase_add_test (tcase, basic_learning);
    tcase_add_test (tcase, words_with_repeating_characters_will_not_be_learned);
    tcase_add_test (tcase, numbers_will_be_ignored_while_learning);
    tcase_add_test (tcase, confidence_should_get_updated_for_existing_words);
    tcase_add_test (tcase, is_known_word);
    tcase_add_test (tcase, learn_from_multiple_open_handles);
    return tcase;
}
