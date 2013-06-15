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

int main(int argc, char **argv)
{
    Suite *suite, *util;
    SRunner *runner;
    int failed; 
        
    srand(time(NULL));

    suite = suite_create ("core");
    suite_add_tcase (suite, get_initialization_tests());
    suite_add_tcase (suite, get_transliteration_tests());
    suite_add_tcase (suite, get_learning_tests());
    suite_add_tcase (suite, get_export_tests());
    suite_add_tcase (suite, get_token_creation_tests());

    util = suite_create ("util");
    suite_add_tcase (util, get_strbuf_tests());

    runner = srunner_create (suite);
    srunner_add_suite (runner, util);
    srunner_run_all (runner, CK_NORMAL);
    failed = srunner_ntests_failed (runner);
    srunner_free (runner);

    return failed;
}
