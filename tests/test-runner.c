/* test-runner.c - driver program that executes the supplied test
 *
 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */


#include <stdio.h>
#include <string.h>
#include "tests.h"
#include <check.h>
#include "testcases.h"
#include <time.h>

int main(int argc, char **argv)
{
    Suite *suite, *util, *commandline;
    SRunner *runner;
    int failed, exit_code;

    /* Cleaning the output directory */
    exit_code = system ("ruby test_output_cleanup.rb");
    if (exit_code != 0) {
        fprintf (stderr, "Failed to cleanup test output directory. Process returned %d", exit_code);
    }

    suite = suite_create ("core");
    suite_add_tcase (suite, get_initialization_tests());
    suite_add_tcase (suite, get_transliteration_tests());
    suite_add_tcase (suite, get_learning_tests());
    suite_add_tcase (suite, get_export_tests());
    suite_add_tcase (suite, get_token_creation_tests());
    suite_add_tcase (suite, get_stemmer_tests());

    util = suite_create ("util");
    suite_add_tcase (util, get_strbuf_tests());

    commandline = suite_create ("commandline");
    suite_add_tcase (commandline, get_varnamc_tests());

    runner = srunner_create (suite);
    srunner_add_suite (runner, util);
    srunner_add_suite (runner, commandline);
    srunner_set_log (runner, "testrun.log");
    srunner_set_xml (runner, "testrun.xml");
    srunner_set_fork_status (runner, CK_NOFORK);
    srunner_run_all (runner, CK_NORMAL);
    failed = srunner_ntests_failed (runner);
    srunner_free (runner);

    return failed;
}
