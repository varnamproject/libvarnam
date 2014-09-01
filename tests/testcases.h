/* testcases.h - defines all available test fixtures
 *
 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */


#ifndef TESTCASES_H_INCLUDED_125727
#define TESTCASES_H_INCLUDED_125727

#include <check.h>
#include "../varnam.h"

extern varnam *varnam_instance;

char*
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
execute_query (sqlite3* db, const char* sql);

int
execute_query_int (sqlite3* db, const char* sql);

char*
create_text_file (const char* contents);

strbuf*
read_text_file (const char* filename);

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
TCase* get_token_creation_tests();
TCase* get_varnamc_tests();
TCase* get_stemmer_tests();

#endif
