/* vstgen.c - Implementation of Varnam Symbol Table generator
 *
 * Copyright (C) 2010 Navaneeth.K.N
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

#include "trie.h"
#include "util.h"
#include "parser.h"
#include "varnam-result-codes.h"
#include "foreign/sqlite3.h"

static const char *token_type_tostring(enum token_type type)
{
    switch(type)
    {
    case PARSER_TOKEN_VOWEL:
        return "vo";
    case PARSER_TOKEN_CONSONANT:
        return "co";
    case PARSER_TOKEN_CONSONANT_CLUSTER:
        return "cc";
    case PARSER_TOKEN_NUMBER:
        return "nu";
    case PARSER_TOKEN_SYMBOL:
        return "sy";
    case PARSER_TOKEN_OTHER:
        return "ot";
    }
    return "";
}

#define VSTGEN_SYMBOLS_STORE "symbols"

static unsigned int totalitems;
static unsigned int current;

static int callback(struct trie* t, unsigned int depth, void *userdata)
{
    sqlite3 *db; const struct token *tok;
    char *zErrMsg = NULL;
    int rc;
    char sql[500];

    if(t->root) return VARNAM_SUCCESS;

    assert(userdata);
    assert(t->value);

    db = (sqlite3 *) userdata;
    tok = (const struct token *) t->value;

    snprintf(sql, 500, "insert into %s values ('%s', '%s', '%s', '%s', %d);", VSTGEN_SYMBOLS_STORE, token_type_tostring(tok->type),
             tok->pattern, tok->value1, tok->value2, t->child == NULL ? 0 : 1);

    rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK ) {
        varnam_error("Error processing %s : %s", t->label, zErrMsg);
        sqlite3_free(zErrMsg);
        return VARNAM_ERROR;
    }

    varnam_info(":: (%d/%d) Processing : %s", ++current, totalitems, t->label);
    return VARNAM_SUCCESS;
}

extern int varnam_generate_symbols(const char *output_file_path, struct trie *result, char **error_message)
{
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    char sql[200];

    assert(output_file_path);
    assert(result);

    rc = sqlite3_open(output_file_path, &db);
    if( rc ) {      
        varnam_error("Can't open %s: %s\n", output_file_path, sqlite3_errmsg(db));
        sqlite3_close(db);
        return VARNAM_ERROR;
    }

    snprintf(sql, 200, "drop table if exists %s;", VSTGEN_SYMBOLS_STORE);
    rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
        varnam_error("Failed to initialize output file : %s", zErrMsg);
        sqlite3_free(zErrMsg);
        return VARNAM_ERROR;
    }

    snprintf(sql, 200, "create table %s (type TEXT, pattern TEXT, value1 TEXT, value2 TEXT, children INTEGER);", VSTGEN_SYMBOLS_STORE);
    rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
        varnam_error("Failed to initialize output file : %s", zErrMsg);
        sqlite3_free(zErrMsg);
        return VARNAM_ERROR;
    }

    snprintf(sql, 200, "BEGIN;", VSTGEN_SYMBOLS_STORE);
    rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
        varnam_error("Failed to start a transaction : %s", zErrMsg);
        sqlite3_free(zErrMsg);
        return VARNAM_ERROR;
    }

    varnam_info(":: Generating symbols");

    totalitems = trie_children_count( result );
    current = 0;
    trie_iterate(result, callback, db);

    snprintf(sql, 200, "COMMIT;", VSTGEN_SYMBOLS_STORE);
    rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
        varnam_error("Failed to commit transaction : %s", zErrMsg);
        sqlite3_free(zErrMsg);
        return VARNAM_ERROR;
    }

    varnam_info(":: Symbols generated");
 
    sqlite3_close( db );
    return VARNAM_SUCCESS;
}
