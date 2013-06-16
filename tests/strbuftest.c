/* Tests strbuf
 *
 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */


#include "../varnam.h"
#include <stdio.h>
#include <string.h>
#include <check.h>
#include "testcases.h"

START_TEST (formatted_strings)
{
    const char *expected = "Character a, Integer 10, String Navaneeth\n";
    struct strbuf *buffer = strbuf_init(100);
    strbuf_addf(buffer, "Character %c, Integer %d, String %s", 'a', 10, "Navaneeth");
    ck_assert_str_eq (expected, strbuf_to_s (buffer));
}
END_TEST

START_TEST (replace_string)
{
    strbuf *string = strbuf_init (10);
    strbuf_add (string, "Varnam program");
    strbuf_replace(string, "program", "software");
    ck_assert_str_eq ("Varnam software", strbuf_to_s (string));
    strbuf_destroy (string);
}
END_TEST

START_TEST (replace_string_should_be_case_sensitive)
{
    strbuf *string = strbuf_init (10);
    strbuf_add (string, "Varnam program");
    strbuf_replace(string, "varnam", "libvarnam");
    ck_assert_str_eq ("Varnam program", strbuf_to_s (string));
    strbuf_destroy (string);
}
END_TEST

START_TEST (replace_should_work_for_utf8_strings)
{
    strbuf *string = strbuf_init (10);
    strbuf_add (string, "അവന്‍");
    strbuf_replace(string, "ന്‍", "ൻ");
    ck_assert_str_eq (strbuf_to_s(string), "അവൻ");
    strbuf_destroy (string);
}
END_TEST

START_TEST (split_string)
{
    int i = 0;
    varray *tokens;
    strbuf *token;

    strbuf *string = strbuf_init (10);
    strbuf_add (string, "test\ttest\ttest\t\t\t\t\ttest");
    tokens = strbuf_split(string, varnam_instance, '\t');
    ck_assert_int_eq (varray_length (tokens), 4);

    for(i = 0; i < varray_length(tokens); i++) {
        token = varray_get(tokens, i);
        ck_assert_str_eq ("test", strbuf_to_s(token));
    }

    strbuf_clear (string);
    strbuf_add (string, "test test test    ");
    tokens = strbuf_split(string, varnam_instance, ' ');
    ck_assert_int_eq (varray_length (tokens), 3);

    for(i = 0; i < varray_length(tokens); i++) {
        token = varray_get(tokens, i);
        ck_assert_str_eq ("test", strbuf_to_s(token));
    }

    strbuf_clear (string);
    strbuf_add (string, "testtesttest");
    tokens = strbuf_split(string, varnam_instance, 't');
    ck_assert_int_eq (varray_length (tokens), 3);

    for(i = 0; i < varray_length(tokens); i++) {
        token = varray_get(tokens, i);
        ck_assert_str_eq ("es", strbuf_to_s(token));
    }

    strbuf_destroy (string);
}
END_TEST

TCase* get_strbuf_tests()
{
    TCase* tcase = tcase_create("strbuf");
    tcase_add_checked_fixture (tcase, setup, teardown);
    tcase_add_test (tcase, formatted_strings);
    tcase_add_test (tcase, replace_string);
    tcase_add_test (tcase, replace_string_should_be_case_sensitive);
    tcase_add_test (tcase, replace_should_work_for_utf8_strings);
    tcase_add_test (tcase, split_string);
    return tcase;
}
