/*
* Copyright (C) Navaneeth.K.N
*
* This is part of libvarnam. See LICENSE.txt for the license
*/

#include <check.h>
#include <stdio.h>
#include <string.h>
#include "testcases.h"
#include "../varnam.h"
#include "../symbol-table.h"
#include "../vword.h"

extern int
stem(varnam *handle, const char *word, struct varray_t *stem_results);

void
setup_test_data()
{
	char **msg=NULL;
	char *filename = get_unique_filename();

	reinitialize_varnam_instance(filename);
	ensure_schema_exists(varnam_instance, msg);
}

int
insert_to_vst(int type,
	const char *pattern,
	const char *value1,
	const char *value2,
	const char *value3,
	const char *tag,
	int match_type,
	int priority,
	int accept_condition,
	int flags)
{
	int rc;
	sqlite3 *db=NULL;
	sqlite3_stmt *stmt;
	const char *sql = "insert into symbols (type, pattern, value1, value2, value3, tag, match_type, priority, accept_condition, flags) values (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10)";

	db = varnam_instance->internal->db;

	if(db == NULL)
		return 1;

	rc = sqlite3_prepare_v2( db, sql, -1, &stmt, NULL);

	if(rc != SQLITE_OK)
	{
		set_last_error (varnam_instance, "Failed to add metadata : %s", sqlite3_errmsg(db));
		sqlite3_finalize( stmt );
		ck_abort_msg("Could not prepare sqlite3 statement");
	}

	sqlite3_bind_int(stmt, 1, type);
	sqlite3_bind_text(stmt, 2, pattern, -1, NULL);
	sqlite3_bind_text(stmt, 3, value1, -1, NULL);
	sqlite3_bind_text(stmt, 4, value2, -1, NULL);
	sqlite3_bind_text(stmt, 5, value3, -1, NULL);
	sqlite3_bind_text(stmt, 5, tag, -1, NULL);
	sqlite3_bind_int(stmt, 6, match_type);
	sqlite3_bind_int(stmt, 7, priority);
	sqlite3_bind_int(stmt, 8, accept_condition);
	sqlite3_bind_int(stmt, 9, flags);

	rc = sqlite3_step(stmt);

	if(rc != SQLITE_DONE)
	{
		printf("Error inserting symbols into vst\n");
		return 1;
	}

	sqlite3_finalize(stmt);

	return 0;
}


START_TEST (insert_stemrule)
{
	int rc;
	sqlite3 *db;
	sqlite3_stmt *stmt;
	const char *sql = "select new_ending from stemrules where old_ending = ?1;";
	const char *empty_word=NULL;

	db = varnam_instance->internal->db;

	rc = vst_has_stemrules(varnam_instance);
	ck_assert_int_eq(rc, VARNAM_STEMRULE_MISS);
	
	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	if(rc != SQLITE_OK)
		ck_abort_msg("Sqlite error : %s", sqlite3_errmsg(db));
	
	rc = varnam_create_stemrule(varnam_instance, empty_word, "ല");
	assert_error(rc);
	rc = varnam_create_stemrule(varnam_instance, "ക", "ല");
	assert_success(rc);

	rc = sqlite3_bind_text(stmt, 1, "ക", -1, NULL);
	if(rc != SQLITE_OK)
		ck_abort_msg("Sqlite error : %s", sqlite3_errmsg(db));

	rc = sqlite3_step(stmt);
	if(rc == SQLITE_ROW)
		ck_assert_str_eq((const char*)sqlite3_column_text(stmt, 0), "ല");
	else if(rc != SQLITE_DONE)
		ck_abort_msg("Sqlite error : %s", sqlite3_errmsg(db));

	sqlite3_reset(stmt);

	rc = varnam_create_stemrule(varnam_instance, "ളായ", "ൾ");
	assert_success(rc);

	rc = sqlite3_bind_text(stmt, 1, "ളായ", -1, NULL);
	rc = sqlite3_step(stmt);

	if(rc == SQLITE_ROW)
		ck_assert_str_eq((const char*)sqlite3_column_text(stmt, 0), "ൾ");
	else if(rc != SQLITE_DONE)
		ck_abort_msg("Sqlite error : %s", sqlite3_errmsg(db));	

	/*resetting stemrule count so that vst_has_stemrule will recalculate it*/
	varnam_instance->internal->stemrules_count = -1;
	rc = vst_has_stemrules(varnam_instance);
	ck_assert_int_eq(rc, VARNAM_STEMRULE_HIT);

	sqlite3_finalize(stmt);
}
END_TEST

START_TEST(stemming)
{
	int rc;
	varray *stem_results = varray_init();
	rc = varnam_create_stemrule(varnam_instance, "മാണ്", "ം");
	assert_success(rc);
	rc = varnam_create_stemrule(varnam_instance, "യാണ്", "");
	assert_success(rc);
	rc = varnam_create_stemrule(varnam_instance, "ന്", "ൻ");
	assert_success(rc);
	rc = varnam_create_stemrule(varnam_instance, "ണെന്ന്", "ണ്");
	assert_success(rc);

	rc = stem(varnam_instance, "കാര്യമാണ്", stem_results);
	assert_success(rc);
	ck_assert_str_eq(((vword*)stem_results->memory[stem_results->index])->text, "കാര്യം");
	varray_clear(stem_results);
	
	rc = stem(varnam_instance, "അവശതയാണ്", stem_results);
	assert_success(rc);
	ck_assert_str_eq(((vword*)stem_results->memory[stem_results->index])->text, "അവശത");
	varray_clear(stem_results);


	rc = insert_to_vst(9, "~", "", "", "", "", 1, 0, 0, 0);
	assert_success(rc);
	rc = insert_to_vst(1, "aa", "ാ", "", "", "", 1, 0, 0, 0);
	assert_success(rc);
	rc = insert_to_vst(1, "e", "എ", "െ", "", "", 1, 0, 0, 0);
	assert_success(rc);
	rc = insert_to_vst(1, "i", "ഇ", "ി", "", "", 1, 0, 0, 1);
	assert_success(rc);
	rc = insert_to_vst(4, "Ne", "ണെ", "", "", "", 1, 0, 0, 1);
	assert_success(rc);
	rc = insert_to_vst(3, "n~", "ന്", "്", "", "", 1, 0, 0, 3);
	assert_success(rc);
	rc = insert_to_vst(2, "na", "ന", "", "", "", 1, 0, 0, 3);
	assert_success(rc);
	
	rc = varnam_create_stem_exception(varnam_instance, "ന്", "ന്");
	assert_success(rc);
	rc = stem(varnam_instance, "അവിടെയാണെന്ന്", stem_results);
	assert_success(rc);
	ck_assert_str_eq(((vword*)stem_results->memory[stem_results->index])->text, "അവിടെ");
	varray_clear(stem_results);

	varray_free(stem_results, *destroy_word);
}
END_TEST

TCase* get_stemmer_tests()
{
	TCase* tcase = tcase_create("stemmer");
    tcase_add_checked_fixture(tcase, setup, teardown);
    tcase_add_checked_fixture(tcase, setup_test_data, NULL);
    tcase_add_test(tcase, insert_stemrule);
    tcase_add_test(tcase, stemming);
    return tcase;
}

