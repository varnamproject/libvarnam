/*
 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */


#include "testcases.h"
#include <check.h>
#include <time.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "../varnam.h"
#include <string.h>

varnam *varnam_instance = NULL;

int
file_exist (const char *filename)
{
    struct stat buffer;
    return (stat (filename, &buffer) == 0);
}

char*
get_unique_filename()
{
  static int fileUniqueId = 1;
  strbuf *filename = strbuf_init (25);
  strbuf_addf (filename, "output/%d.test.vst", fileUniqueId);
  fileUniqueId = fileUniqueId + 1;
  return strbuf_detach (filename);
}

void
assert_success (int value)
{
    strbuf *string = NULL;
    if (value != VARNAM_SUCCESS) {
        string = strbuf_init (50);
        strbuf_addf (string, "Expected VARNAM_SUCCESS, but got %d. %s", value, 
                varnam_get_last_error (varnam_instance));
        ck_abort_msg (strbuf_to_s (string));
    }
}

void 
assert_error (int value)
{
    strbuf *string = NULL;
    if (value != VARNAM_ERROR) {
        string = strbuf_init (50);
        strbuf_addf (string, "Expected VARNAM_ERROR, but got %d. %s", value, 
                varnam_get_last_error (varnam_instance));
        ck_abort_msg (strbuf_to_s (string));
    }
}

void
ensure_word_list_contains(varray *words, const char *word)
{
    int i = 0, found = 0;
    vword *w;
    strbuf *error;

    for (i = 0; i < varray_length (words); i++) {
        w = varray_get (words, i);
        if (strcmp (w->text, word) == 0) {
            found = 1;
            break;
        }
    }

    if (!found) {
        error = strbuf_init (50);
        strbuf_addf (error, "Expected word list to contain '%s'", word);
        ck_abort_msg (strbuf_to_s (error));
    }
}

void
reinitialize_varnam_instance(const char *filename)
{
    int rc;
    char *msg;
    varnam *handle;
    strbuf *error;

    if (varnam_instance != NULL) {
        varnam_destroy (varnam_instance);
        varnam_instance = NULL;
    }

    rc = varnam_init (filename, &handle, &msg);
    if(rc != VARNAM_SUCCESS) {
        error = strbuf_init (50);
        strbuf_addf (error, "Varnam initialization failed. %s. %s", filename, msg);
        ck_abort_msg (strbuf_to_s (error));
    }

    varnam_instance = handle;
}

void
execute_query (sqlite3* db, const char* sql)
{
    int rc;
    sqlite3_stmt* stmt;
    strbuf* error;

    rc = sqlite3_prepare_v2 (db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        error = strbuf_init (50);
        strbuf_addf (error, "Failed to prepare query: %s. Return code was: %d\n", sql, rc);
        ck_abort_msg (strbuf_to_s (error));
    }

    rc = sqlite3_step (stmt);
    if (rc != SQLITE_DONE) {
        error = strbuf_init (50);
        strbuf_addf (error, "Failed to execute query: %s. Return code was: %d\n", sql, rc);
        ck_abort_msg (strbuf_to_s (error));
    }

    sqlite3_finalize (stmt);
}

int
execute_query_int (sqlite3* db, const char* sql)
{
    int rc, result;
    sqlite3_stmt* stmt;
    strbuf* error;

    rc = sqlite3_prepare_v2 (db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        error = strbuf_init (50);
        strbuf_addf (error, "Failed to prepare query: %s. Return code was: %d\n", sql, rc);
        ck_abort_msg (strbuf_to_s (error));
    }

    rc = sqlite3_step (stmt);
    if (rc != SQLITE_ROW) {
        error = strbuf_init (50);
        strbuf_addf (error, "Failed to execute query: %s. Return code was: %d\n", sql, rc);
        ck_abort_msg (strbuf_to_s (error));
    }

    result = sqlite3_column_int (stmt, 0);
    sqlite3_finalize (stmt);
    return result;
}

char*
create_text_file (const char* contents)
{
    FILE* fp;
    char* filename;

    filename = get_unique_filename ();
    fp = fopen (filename, "w");
    if (fp == NULL)
        ck_abort_msg ("Failed to create a text file");
    
    fprintf (fp, "%s\n", contents);
    fclose (fp);

    return filename;
}

strbuf*
read_text_file (const char* filename)
{
    FILE* fp;
    char buffer[1000];
    strbuf* contents;

    fp = fopen (filename, "r");
    if (fp == NULL)
        ck_abort_msg ("Failed to open file for reading");

    contents = strbuf_init (100);
    while (fgets (buffer, 1000, fp) != NULL) {
        strbuf_add (contents, buffer);
    }

    fclose (fp);
    return contents;
}

void
setup()
{
    char *filename =  get_unique_filename();
    reinitialize_varnam_instance (filename);
    xfree (filename);
}

void
teardown()
{
    if (varnam_instance != NULL)
        varnam_destroy (varnam_instance);
    varnam_instance = NULL;
}
