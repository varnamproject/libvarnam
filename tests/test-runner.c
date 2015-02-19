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
#include <argtable2.h>

/*struct arg_file *suite;
 struct arg_file *testcases;
 struct arg_end *end;*/

int main(int argc, char **argv)
{
    Suite *suite, *util, *commandline;
    SRunner *runner;
    int failed, exit_code, nerrors;
    struct arg_file *suite_arg;
    struct arg_lit *help;
    struct arg_lit *fork;
    struct arg_file *test_arg;
    struct arg_end *end;

    void *argtable[] = {
        suite_arg = arg_filen(NULL, NULL, "<suite_names>", 0, 10, "The test suite to run"),
        test_arg = arg_file0("t", NULL, "<test_name>", "A test in the suite to run"),
        help = arg_lit0("h", "help", "Display help text"),
        fork = arg_lit0("f", NULL, "Run in forked mode"),
        end = arg_end(20)
    };

    /*test_arg->hdr.flag |= ARG_HASOPTVALUE;*/
    nerrors = arg_parse(argc, argv, argtable);

    if(help->count > 0)
    {
        printf("usage : \n");
        arg_print_syntax(stdout, argtable, "\n");
        printf("Glossary : \n");
        arg_print_glossary(stdout, argtable, " %-25s %s\n");
        exit(0);
    }

    if(nerrors > 0)
    {       
        arg_print_errors(stdout, end, "runtest");
        exit(1);
    }

    
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

    if(fork->count == 0)
    {
        printf("Running in no fork mode\n");
        srunner_set_fork_status (runner, CK_NOFORK);
    }
    else
        printf("Running in forked mode\n");
    

    if(suite_arg->count == 0)
        srunner_run_all (runner, CK_NORMAL);
    else
    {
        int i;

        if(test_arg->count > 0 && suite_arg->count > 1)
        {
            printf("Only one suite and one test can be specified if a test argument is provided\nSee --help for more\n");
            exit(1);
        }
        else if(test_arg->count > 0)
            srunner_run (runner, suite_arg->filename[0], test_arg->filename[0], CK_NOFORK);
        else
        {
            /*If only suite names are specified*/
            for(i=0; i<suite_arg->count; i++)
            {
                srunner_run (runner, suite_arg->filename[i], NULL, CK_NOFORK);
            }
        }
    }

    failed = srunner_ntests_failed (runner);
    srunner_free (runner);

    return failed;
}
