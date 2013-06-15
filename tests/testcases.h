/* testcases.h - defines all available test fixtures
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

#ifndef TESTCASES_H_INCLUDED_125727
#define TESTCASES_H_INCLUDED_125727

#include <check.h>
#include "../varnam.h"

extern varnam *varnam_instance;

strbuf*
get_unique_filename();

void 
assert_success (int value);

void 
assert_error (int value);

void
ensure_word_list_contains(varray *words, const char *word);

void
reinitialize_varnam_instance(const char *filename);

int
file_exist (const char *filename);

void
setup();

void 
teardown();

/* All the test cases */
TCase* get_initialization_tests();
TCase* get_transliteration_tests();
TCase* get_learning_tests();
TCase* get_strbuf_tests();
TCase* get_export_tests();

#endif
