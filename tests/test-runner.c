/* test-runner.c - driver program that executes the supplied test
 *
 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */

/*

USAGE:

./runtests [-f] [y|n] [-s] [suite-name] [testcase 1] [testcase 2] ...

examples :

./runtests -f n     :   Run all the tests in no fork mode
./runtests -f y     :   Run all the tests in fork mode
./runtests -f n -s core     :   Run the suite "core" in no fork mode
./runtests -f n -s core transliteration     :   Run the testcase "transliteration" from suite "core"
./runtests -s core transliteration initializaiton   :   Run testcases "transliteration" and "initialization" from suite core
*/

#include <stdio.h>
#include <string.h>
#include "tests.h"
#include <check.h>
#include "testcases.h"
#include <time.h>


void resolve_arguments(int argc, char **argv, int *selective, int *fork, int *test_arg_start)
{
    char *ptr;
    int i;

    for (i=1;i<argc;i++)
    {
        if (strcmp(argv[i], "-f") == 0)
        {
            i++;
            if(strcmp(argv[i], "n") == 0)
                *fork = 0;
            else
                *fork = 1;
        }

        else if(strcmp(argv[i], "-s") == 0)
        {
            int j = 0;
            *selective = 1;
            *test_arg_start = ++i;
        }
    }
}

void run_selected_tests(SRunner *runner, char **argv, int argc, int test_arg_start)
{
    int i;
    char *suite;

    suite = argv[test_arg_start++];
    
    /*if suite name is the last argument*/
    if(test_arg_start == argc)
    {    
        srunner_run (runner, suite, NULL, CK_NORMAL);
        return;
    }

    for (i=test_arg_start; i<argc; i++)
        srunner_run (runner, suite, argv[i], CK_NORMAL);        
}


int main(int argc, char **argv)
{
    Suite *suite, *util, *commandline;
    SRunner *runner;
    int failed, exit_code, i, test_arg_start=0;
    int selective=0, fork=0;

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

    util = suite_create ("util");
    suite_add_tcase (util, get_strbuf_tests());

    commandline = suite_create ("commandline");
    suite_add_tcase (commandline, get_varnamc_tests());

    runner = srunner_create (suite);
    srunner_add_suite (runner, util);
    srunner_add_suite (runner, commandline);
    srunner_set_log (runner, "testrun.log");
    srunner_set_xml (runner, "testrun.xml");

    if (argc > 1)
        resolve_arguments(argc, argv, &selective, &fork, &test_arg_start);

    if (fork)
    {    
        srunner_set_fork_status (runner, CK_FORK);
        printf("Fork mode on \n");
    }
    else
    {
        srunner_set_fork_status (runner, CK_NOFORK);
        printf("Fork mode off\n");
    }

    if (selective)
        run_selected_tests(runner, argv, argc, test_arg_start);
    else    
        srunner_run_all (runner, CK_NORMAL);
    
    failed = srunner_ntests_failed (runner);
    srunner_free (runner);

    return failed;
}
