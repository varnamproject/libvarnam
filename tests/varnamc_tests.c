/* test-runner.c - driver program that executes the supplied test
 *
 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */

#include "../varnam.h"
#include <check.h>
#include "testcases.h"

START_TEST (learn_failures_file_should_not_be_created_always)
{
    int exitcode;
    exitcode = system ("../varnamc -s ml -t varnam -d output/");
    ck_assert_int_eq (0, exitcode);
    ck_assert_int_eq (0, file_exist ("output/varnamc-learn-failures.txt"));
    ck_assert_int_eq (0, file_exist ("output/varnamc-train-failures.txt"));
}
END_TEST

START_TEST (learn_failures_file_should_be_created_upon_failures)
{
    int exitcode;
    char* filename;
    strbuf* command, *contents;

    filename = create_text_file ("not-valid-indic-word");
    command = strbuf_init (20);
    strbuf_addf (command, "../varnamc -s ml --learn-from %s -d output/", filename);
    exitcode = system (strbuf_to_s (command));
    ck_assert_int_eq (0, exitcode);
    ck_assert_int_eq (1, file_exist ("output/varnamc-learn-failures.txt"));

    contents = read_text_file ("output/varnamc-learn-failures.txt");
    ck_assert_str_eq ("not-valid-indic-word : Can't process 'n'. One or more characters in 'not-valid-indic-word' are not known",
            trimwhitespace (strbuf_to_s (contents)));

    strbuf_destroy (contents);
    strbuf_destroy (command);
    free (filename);
}
END_TEST

START_TEST (training_failures_file_should_be_created_upon_failures)
{
    int exitcode;
    char* filename;
    strbuf* command, *contents;

    filename = create_text_file ("not-valid-indic-word");
    command = strbuf_init (20);
    strbuf_addf (command, "../varnamc -s ml --train-from %s -d output/", filename);
    exitcode = system (strbuf_to_s (command));
    ck_assert_int_eq (0, exitcode);
    ck_assert_int_eq (1, file_exist ("output/varnamc-train-failures.txt"));

    contents = read_text_file ("output/varnamc-train-failures.txt");
    ck_assert_str_eq ("Error at text: not-valid-indic-word. Invalid format. Each line should contain pattern and word separated with a single space",
            trimwhitespace (strbuf_to_s (contents)));

    strbuf_destroy (contents);
    strbuf_destroy (command);
    free (filename);
}
END_TEST

TCase* get_varnamc_tests()
{
    TCase* tcase = tcase_create("varnamc");
    tcase_add_test (tcase, learn_failures_file_should_not_be_created_always);
    tcase_add_test (tcase, learn_failures_file_should_be_created_upon_failures);
    tcase_add_test (tcase, training_failures_file_should_be_created_upon_failures);
    return tcase;
}
