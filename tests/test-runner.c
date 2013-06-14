/* test-runner.c - driver program that executes the supplied test
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

#include <stdio.h>
#include <string.h>
#include "tests.h"
#include <check.h>
#include "testcases.h"
#include <time.h>

struct tests_t {
    const char *name;
    int(*function)(int argc, char **argv);
};

static struct tests_t tests[] = {
    { "strbuf-test", strbuf_test },
    { "test-vst-file-creation", test_vst_file_creation },
    { "test-export", test_varnam_export}
};

#define NO_OF_TESTS (sizeof(tests)/sizeof(tests[0]))

static int execute_test(int argc, char **argv)
{
    const char *test_to_execute = argv[0];
    struct tests_t *test = 0;
    unsigned i;
    for (i = 0; i < NO_OF_TESTS; i++) {
        struct tests_t *t  = tests + i;
        if (strcmp(t->name, test_to_execute) == 0) {
            test = t;
            break;
        }
    }

    if(test == NULL) {
        printf("invalid test '%s'.\n", test_to_execute);
        return 1;
    }
    argv++;
    argc--;
    return test->function(argc, argv);
}

static void print_all_test_names()
{
    int i;
    for (i = 0; i < NO_OF_TESTS; i++) {
        struct tests_t *t  = tests + i;
        printf ("%s\n", t->name);
    }
}


int main(int argc, char **argv)
{
    Suite *suite;
    SRunner *runner;
    int failed; 
        
    srand(time(NULL));

    suite = suite_create("varnam");
    suite_add_tcase (suite, get_initialization_tests());
    suite_add_tcase (suite, get_transliteration_tests());
    suite_add_tcase (suite, get_learning_tests());

    runner = srunner_create (suite);
    srunner_run_all (runner, CK_NORMAL);
    failed = srunner_ntests_failed (runner);
    srunner_free (runner);

    return failed;
}
