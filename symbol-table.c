/* Functions to handle the symbols table
 *
 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */


#include <string.h>
#include <assert.h>

#include "symbol-table.h"
#include "util.h"
#include "vtypes.h"
#include "result-codes.h"
#include "api.h"
#include "token.h"

int
ensure_schema_exists(varnam *handle, char **msg)
{
    const char *sql =
        "pragma page_size=4096;"
        "create table if not exists metadata (key TEXT UNIQUE, value TEXT);"
        "create table if not exists symbols (id INTEGER PRIMARY KEY AUTOINCREMENT, type INTEGER, pattern TEXT, value1 TEXT, value2 TEXT, value3 TEXT, tag TEXT, match_type INTEGER, priority INTEGER DEFAULT 0, accept_condition INTEGER, flags INTEGER DEFAULT 0);"
        "create table if not exists stemrules (id INTEGER PRIMARY KEY AUTOINCREMENT, old_ending TEXT, new_ending TEXT);"
        "create table if not exists stem_exceptions (id INTEGER PRIMARY KEY AUTOINCREMENT, stem TEXT, exception TEXT)";

    const char *indexes =
        "create index if not exists index_metadata on metadata (key);"
        "create index if not exists index_pattern on symbols (pattern);"
        "create index if not exists index_value1 on symbols (value1);"
        "create index if not exists index_value2 on symbols (value2);"
        "create index if not exists index_value3 on symbols (value3);";

    char *zErrMsg = 0;
    int rc;

    assert(handle);
    assert(handle->internal->db);

    rc = sqlite3_exec(v_->db, sql, NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
        set_last_error (handle, "Failed to initialize output file : %s", zErrMsg);
        sqlite3_free(zErrMsg);
        return VARNAM_STORAGE_ERROR;
    }

    rc = sqlite3_exec(v_->db, indexes, NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
        set_last_error (handle, "Failed to generate indexes : %s", zErrMsg);
        sqlite3_free(zErrMsg);
        return VARNAM_STORAGE_ERROR;
    }

    return VARNAM_SUCCESS;
}

int
vst_start_buffering(varnam *handle)
{
    char *zErrMsg;
    int rc;
    const char *sql = "BEGIN;";

    assert(handle);

    if (handle->internal->vst_buffering)
        return VARNAM_SUCCESS;

    rc = sqlite3_exec(handle->internal->db, sql, NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        set_last_error (handle, "Failed to start buffering : %s", zErrMsg);
        sqlite3_free(zErrMsg);
        return VARNAM_STORAGE_ERROR;
    }

    handle->internal->vst_buffering = 1;

    return VARNAM_SUCCESS;
}

static int
already_persisted(
    varnam *handle,
    const char *pattern,
    const char *value1,
    int match_type,
    int *result)
{
    int rc;
    sqlite3 *db; sqlite3_stmt *stmt;

    assert (result);

    *result = 0;
    db = handle->internal->db;

    if (match_type == VARNAM_MATCH_EXACT)
        rc = sqlite3_prepare_v2( db, "select count(1) from symbols where pattern = trim(?1) and match_type = ?2", -1, &stmt, NULL );
    else
        rc = sqlite3_prepare_v2( db, "select count(1) from symbols where pattern = trim(?1) and value1 = trim(?2)", -1, &stmt, NULL );

    if(rc != SQLITE_OK)
    {
        set_last_error (handle, "Failed to check already persisted : %s", sqlite3_errmsg(db));
        sqlite3_finalize( stmt );
        return VARNAM_ERROR;
    }

    sqlite3_bind_text(stmt, 1, pattern, -1, NULL);
    if (match_type == VARNAM_MATCH_EXACT)
        sqlite3_bind_int (stmt, 2, match_type);
    else
        sqlite3_bind_text(stmt, 2, value1, -1, NULL);

    rc = sqlite3_step( stmt );
    if( rc == SQLITE_ROW )
    {
        if( sqlite3_column_int( stmt, 0 ) > 0 )
        {
            *result = 1;
        }
    }
    else if ( rc == SQLITE_DONE )
    {
        *result = 0;
    }
    else
    {
        set_last_error (handle, "Failed to check already persisted : %s", sqlite3_errmsg(db));
        sqlite3_finalize( stmt );
        return VARNAM_ERROR;
    }

    sqlite3_finalize( stmt );

    return VARNAM_SUCCESS;
}

static int
find_prefixes_and_update_flags (varnam *handle, sqlite3_stmt *stmtToExecute, sqlite3_stmt *stmtToUpdate)
{
    /* map which keeps (pattern OR value OR value1) -> id */
    typedef struct {
        const char *symbol;
        int id;
        UT_hash_handle hh;
    } symbol_id_map;

    int rc, symbolId;
    const char *tmp; const char *symbol;
    strbuf *symbolPrefix, *symbolCopy;
    symbol_id_map *symbolIdMap = NULL, *symbolIdItem = NULL, *tmpSymbolIdItem;
    sqlite3 *db;
    sqlite3_stmt *stmt = stmtToExecute;

    db = handle->internal->db;

    symbolPrefix = get_pooled_string (handle);
    for (;;) {
        rc = sqlite3_step (stmt);
        if (rc == SQLITE_DONE)
            break;
        else if (rc != SQLITE_ROW) {
            set_last_error (handle, "Failed to execute statement: %s", sqlite3_errmsg(db));
            return VARNAM_ERROR;
        }

        symbolId = sqlite3_column_int (stmt, 0);
        symbol = tmp = (const char*) sqlite3_column_text (stmt, 1);
        strbuf_clear (symbolPrefix);
        while (*tmp != '\0') {
            strbuf_addc (symbolPrefix, *tmp);
            HASH_FIND_STR (symbolIdMap, strbuf_to_s (symbolPrefix), symbolIdItem);
            if (symbolIdItem) {
                /* we got a symbol id. This means this prefix will have children */
                sqlite3_bind_text(stmtToUpdate, 1, strbuf_to_s (symbolPrefix),  -1, NULL);
                rc = sqlite3_step (stmtToUpdate);
                if (rc != SQLITE_DONE) {
                    set_last_error (handle, "Failed to update flags: %d, %s", rc, sqlite3_errmsg(db));
                    return VARNAM_ERROR;
                }
                sqlite3_clear_bindings (stmtToUpdate);
                sqlite3_reset (stmtToUpdate);
            }
            tmp++;
        }

        symbolIdItem = xmalloc (sizeof (symbol_id_map));
        symbolCopy = strbuf_init (16);
        strbuf_add (symbolCopy, symbol);
        symbolIdItem->symbol = strbuf_detach (symbolCopy);
        symbolIdItem->id = symbolId;
        HASH_ADD_KEYPTR (hh, symbolIdMap, symbolIdItem->symbol, strlen(symbolIdItem->symbol), symbolIdItem);
    }

    HASH_ITER (hh, symbolIdMap, symbolIdItem, tmpSymbolIdItem) {
        xfree (symbolIdItem->symbol);
        xfree (symbolIdItem);
    }

    return VARNAM_SUCCESS;
}

/*
 * Makes a prefix tree.
 *
 * Iterates over all the symbols, finds if a symbol can have more matches, if yes mark it in the flags and move on
 * flags will be a bit field which represents more matches for values and pattern
 * */
int
vst_make_prefix_tree (varnam *handle)
{
    int rc, i, mask;
    sqlite3 *db;
    sqlite3_stmt *stmt, *updateStmt;
    strbuf *query;
    const char *columnNames[] = {"pattern", "value1", "value2"};
    
    db = handle->internal->db;

    query = get_pooled_string (handle);
    for (i = 0; i < ARRAY_SIZE (columnNames); i++) {
        strbuf_clear (query);
        strbuf_addf (query, "select id, %s from symbols group by %s order by length(%s) asc;", columnNames[i], columnNames[i], columnNames[i]);
        rc = sqlite3_prepare_v2 (db, strbuf_to_s (query), -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
            set_last_error (handle, "Failed to prepare statement to get all %s: %s", columnNames[i], sqlite3_errmsg(db));
            sqlite3_finalize (stmt);
            return VARNAM_ERROR;
        }

        if (strcmp ("pattern", columnNames[i]) == 0)
            mask = VARNAM_TOKEN_FLAGS_MORE_MATCHES_FOR_PATTERN;
        else
            mask = VARNAM_TOKEN_FLAGS_MORE_MATCHES_FOR_VALUE;

        strbuf_clear (query);
        strbuf_addf (query, "update symbols set flags = flags | %d where %s = ?1;", mask, columnNames[i]);
        rc = sqlite3_prepare_v2 (db, strbuf_to_s (query), -1, &updateStmt, NULL);
        if (rc != SQLITE_OK) {
            set_last_error (handle, "Failed to prepare statement update flags for %s: %s", columnNames[i], sqlite3_errmsg(db));
            sqlite3_finalize (updateStmt);
            return VARNAM_ERROR;
        }

        rc = find_prefixes_and_update_flags (handle, stmt, updateStmt);
        sqlite3_finalize (stmt);
        sqlite3_finalize (updateStmt);
        if (rc != VARNAM_SUCCESS)
            return rc;
    }
    
    return VARNAM_SUCCESS;
}

int
vst_persist_token(
    varnam *handle,
    const char *pattern,
    const char *value1,
    const char *value2,
    const char *value3,
    const char *tag,
    int token_type,
    int match_type,
    int priority,
    int accept_condition)
{
    int rc, persisted;
    sqlite3 *db; sqlite3_stmt *stmt;
    const char *sql = "insert into symbols (type, pattern, value1, value2, value3, tag, match_type, priority, accept_condition) values (?1, trim(?2), trim(?3), trim(?4), trim(?5), trim(?6), ?7, ?8, ?9);";

    assert(handle); assert(pattern); assert(value1); assert(token_type);

    rc = already_persisted (handle, pattern, value1, match_type, &persisted);
    if (rc != VARNAM_SUCCESS)
        return rc;

    if (persisted)
    {
        if (handle->internal->config_ignore_duplicate_tokens)
        {
            varnam_log (handle, "%s => %s is already available. Ignoring duplicate tokens", pattern, value1);
            return VARNAM_SUCCESS;
        }

        set_last_error (handle, "There is already a match available for '%s => %s'. Duplicate entries are not allowed", pattern, value1);
        return VARNAM_ERROR;
    }

    db = handle->internal->db;

    rc = sqlite3_prepare_v2( db, sql, -1, &stmt, NULL );
    if(rc != SQLITE_OK)
    {
        set_last_error (handle, "Failed to initialize statement : %s", sqlite3_errmsg(db));
        sqlite3_finalize( stmt );
        return VARNAM_ERROR;
    }

    sqlite3_bind_int (stmt, 1, token_type);
    sqlite3_bind_text(stmt, 2, pattern,    -1, NULL);
    sqlite3_bind_text(stmt, 3, value1,     -1, NULL);
    sqlite3_bind_text(stmt, 4, value2 == NULL ? "" : value2, -1, NULL);
    sqlite3_bind_text(stmt, 5, value3 == NULL ? "" : value3, -1, NULL);
    sqlite3_bind_text(stmt, 6, tag == NULL ? "" : tag, -1, NULL);
    sqlite3_bind_int (stmt, 7, match_type);
    sqlite3_bind_int (stmt, 8, priority);
    sqlite3_bind_int (stmt, 9, accept_condition);

    rc = sqlite3_step( stmt );
    if( rc != SQLITE_DONE )
    {
        set_last_error (handle, "Failed to persist token : %s", sqlite3_errmsg(db));
        sqlite3_finalize( stmt );
        return VARNAM_ERROR;
    }

    sqlite3_finalize( stmt );

    return VARNAM_SUCCESS;
}

int
vst_persist_stemrule(varnam *handle, const char* old_ending, const char* new_ending)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;
    const char *sql = "insert into stemrules (old_ending,new_ending) values (?1, ?2);";

    db = handle->internal->db;

    if(v_->persist_stemrule == NULL)
    {
        rc = sqlite3_prepare_v2(db, sql, -1, &v_->persist_stemrule, NULL);
        if(rc != SQLITE_OK)
        {
            set_last_error(handle, "Failed to prepare statement : %s", sqlite3_errmsg(db));
            return VARNAM_ERROR;
        }
    }

    stmt = v_->persist_stemrule;
    rc = sqlite3_bind_text(stmt, 1, old_ending, -1, NULL);
    if(rc != SQLITE_OK)
    {
        sqlite3_reset(stmt);
        set_last_error(handle, "Failed binding : %s", sqlite3_errmsg(db));
        return VARNAM_ERROR;
    }
    
    rc = sqlite3_bind_text(stmt, 2, new_ending, -1, NULL);
    if(rc != SQLITE_OK)
    {
        sqlite3_reset(stmt);
        set_last_error(handle, "Failed binding : %s", sqlite3_errmsg(db));
        return VARNAM_ERROR;
    }
    
    rc = sqlite3_step(stmt);

    if(rc != SQLITE_DONE)
    {
        set_last_error (handle, "Failed to persist stemrule : %s", sqlite3_errmsg(db));
        sqlite3_reset( stmt );
        return VARNAM_ERROR;
    }

    sqlite3_reset( stmt );
    return VARNAM_SUCCESS;
}

int 
vst_persist_stem_exception(varnam *handle, const char *rule, const char *exception)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;
    const char *sql = "insert into stem_exceptions(stem, exception) values (?1, ?2)";

    db = handle->internal->db;

    if(v_->persist_stem_exception == NULL)
    {
        rc = sqlite3_prepare_v2(db, sql, -1, &v_->persist_stem_exception, NULL);
        
        if(rc != SQLITE_OK)
        {
            set_last_error(handle, "Failed to initialize statement : %s", sqlite3_errmsg(db));
            return VARNAM_ERROR;
        }
    }
    
    stmt = v_->persist_stem_exception;
    sqlite3_bind_text(stmt, 1, rule, -1, NULL);
    sqlite3_bind_text(stmt, 2, exception, -1, NULL);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE)
    {
        set_last_error (handle, "Failed to persist stemrule : %s", sqlite3_errmsg(db));
        sqlite3_reset( stmt );
        return VARNAM_ERROR;
    }

    sqlite3_reset( stmt );
    return VARNAM_SUCCESS;
}

int
vst_flush_changes(varnam *handle)
{
    char *zErrMsg;
    int rc;
    const char *sql = "COMMIT;";

    assert(handle);

    if (!handle->internal->vst_buffering)
        return VARNAM_SUCCESS;

    varnam_log (handle, "Writing changes to file...");
    rc = sqlite3_exec(handle->internal->db, sql, NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        set_last_error (handle, "Failed to flush changes : %s", zErrMsg);
        sqlite3_free(zErrMsg);
        return VARNAM_STORAGE_ERROR;
    }
    handle->internal->vst_buffering = 0;

    varnam_log (handle, "Compacting file...");
    rc = sqlite3_exec(handle->internal->db, "VACUUM;", NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        set_last_error (handle, "Failed to compact file : %s", zErrMsg);
        sqlite3_free(zErrMsg);
        return VARNAM_STORAGE_ERROR;
    }

    return VARNAM_SUCCESS;
}

int
vst_discard_changes(varnam *handle)
{
    char *zErrMsg;

    assert(handle);

    if (!handle->internal->vst_buffering)
        return VARNAM_SUCCESS;

    /* vst_discard_changes() is usually called when something wrong happened.
     * at this time, most probably last error will have some value. so just executing
     * rollback without any error check as this function don't want to overwrite the
     * last error set by previous functions */
    sqlite3_exec(handle->internal->db, "ROLLBACK;", NULL, 0, &zErrMsg);
    handle->internal->vst_buffering = 0;
    return VARNAM_SUCCESS;
}

static int
vst_get_token_by_type (varnam* handle, int tokenType, struct token *output, bool *tokenAvailable)
{
  int rc;
  sqlite3 *db; sqlite3_stmt *stmt;

  db = handle->internal->db;
	*tokenAvailable = false;

  rc = sqlite3_prepare_v2( db, "select id, type, match_type, pattern, value1, value2, value3, tag, priority, accept_condition, flags from symbols where type = ?1 and match_type = ?2 limit 1;", -1, &stmt, NULL );
  if(rc != SQLITE_OK) {
    set_last_error (handle, "Failed to get token by type. Error preparing : %s", sqlite3_errmsg(db));
    sqlite3_finalize( stmt );
    return VARNAM_ERROR;
  }

  sqlite3_bind_int (stmt, 1, tokenType);
  sqlite3_bind_int (stmt, 2, VARNAM_MATCH_EXACT);

  rc = sqlite3_step( stmt );
  if( rc == SQLITE_ROW ) {
    initialize_token (output, (int) sqlite3_column_int(stmt, 0),
        (int) sqlite3_column_int(stmt, 1),
        (int) sqlite3_column_int(stmt, 2),
        (const char*) sqlite3_column_text(stmt, 3),
        (const char*) sqlite3_column_text(stmt, 4),
        (const char*) sqlite3_column_text(stmt, 5),
        (const char*) sqlite3_column_text(stmt, 6),
        (const char*) sqlite3_column_text(stmt, 7),
        (int) sqlite3_column_int(stmt, 8),
        (int) sqlite3_column_int(stmt, 9),
        (int) sqlite3_column_int(stmt, 10));
		*tokenAvailable = true;
  }
  else if (rc != SQLITE_DONE) {
    set_last_error (handle, "Failed to get token by type : %s", sqlite3_errmsg(db));
    sqlite3_finalize( stmt );
    return VARNAM_ERROR;
  }

  sqlite3_finalize( stmt );
  return VARNAM_SUCCESS;
}

int
vst_get_virama(varnam* handle, struct token **output)
{
    int rc;
		bool tokenAvailable = false;

    if (v_->virama != NULL) {
        *output = v_->virama;
        return VARNAM_SUCCESS;
    }

		*output = token_new();
		rc = vst_get_token_by_type (handle, VARNAM_TOKEN_VIRAMA, *output, &tokenAvailable);
		if (rc != VARNAM_SUCCESS || !tokenAvailable) {
			destroy_token (*output);
			*output = NULL;
			return rc;
		}
    v_->virama = *output;
    return VARNAM_SUCCESS;
}

int
vst_get_all_tokens (varnam* handle, int token_type, varray *tokens)
{
    struct token *tok = NULL;
    int rc;
    sqlite3 *db; sqlite3_stmt *stmt;

    db = handle->internal->db;

    varray_clear (tokens);

    rc = sqlite3_prepare_v2( db, "select id, type, match_type, pattern, value1, value2, value3, tag, priority, accept_condition, flags from symbols where type = ?1;", -1, &stmt, NULL );
    if(rc != SQLITE_OK)
    {
        set_last_error (handle, "Failed to get all tokens : %s", sqlite3_errmsg(db));
        sqlite3_finalize( stmt );
        return VARNAM_ERROR;
    }

    sqlite3_bind_int (stmt, 1, token_type);

    while(1)
    {
        rc = sqlite3_step( stmt );
        if( rc == SQLITE_ROW )
        {
            tok = get_pooled_token (handle, (int) sqlite3_column_int(stmt, 0),
                                    (int) sqlite3_column_int(stmt, 1),
                                    (int) sqlite3_column_int(stmt, 2),
                                    (const char*) sqlite3_column_text(stmt, 3),
                                    (const char*) sqlite3_column_text(stmt, 4),
                                    (const char*) sqlite3_column_text(stmt, 5),
                                    (const char*) sqlite3_column_text(stmt, 6),
                                    (const char*) sqlite3_column_text(stmt, 7),
                                    (int) sqlite3_column_int(stmt, 8),
                                    (int) sqlite3_column_int(stmt, 9),
                                    (int) sqlite3_column_int(stmt, 10));

            varray_push (tokens, tok);
        }
        else if ( rc == SQLITE_DONE )
            break;
        else
        {
            set_last_error (handle, "Failed to get all tokens : %s", sqlite3_errmsg(db));
            sqlite3_finalize( stmt );
            return VARNAM_ERROR;
        }
    }

    sqlite3_finalize( stmt );
    return VARNAM_SUCCESS;
}

static void
remove_from_last(char *buffer, const char *toremove)
{
    size_t to_remove_len, buffer_len, newlen;
    const char *buf;

    if(!toremove) return;
    to_remove_len = strlen(toremove);
    buffer_len = strlen(buffer);

    if(buffer_len < to_remove_len) return;

    newlen = (buffer_len - to_remove_len);

    buf = buffer + newlen;
    if(strcmp(buf, toremove) == 0) {
        buffer[newlen] = '\0';
    }
}

int
vst_add_metadata (varnam *handle, const char* key, const char* value)
{
    int rc;
    sqlite3 *db; sqlite3_stmt *stmt;

    db = handle->internal->db;

    rc = sqlite3_prepare_v2( db, "insert or replace into metadata (key, value) values (?1, ?2);", -1, &stmt, NULL );
    if(rc != SQLITE_OK)
    {
        set_last_error (handle, "Failed to add metadata : %s", sqlite3_errmsg(db));
        sqlite3_finalize( stmt );
        return VARNAM_ERROR;
    }

    sqlite3_bind_text(stmt, 1, key, -1, NULL);
    sqlite3_bind_text(stmt, 2, value, -1, NULL);

    rc = sqlite3_step( stmt );
    if (rc != SQLITE_DONE)
    {
        set_last_error (handle, "Failed to add metadata : %s", sqlite3_errmsg(db));
        sqlite3_finalize( stmt );
        return VARNAM_ERROR;
    }

    sqlite3_finalize( stmt );
    return VARNAM_SUCCESS;
}

int
vst_load_scheme_details(varnam *handle, vscheme_details *output)
{
	int rc;
	sqlite3_stmt *stmt;
	strbuf *key, *value;
	sqlite3 *db;

	db = handle->internal->db;

  rc = sqlite3_prepare_v2(db, "select key, value from metadata;", -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    set_last_error (handle, "Failed to load scheme details: %s", sqlite3_errmsg(db));
    sqlite3_finalize(stmt);
    return VARNAM_ERROR;
  }

	key = strbuf_init (32);
	while (1) {
  	rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
			strbuf_add (key, (const char*) sqlite3_column_text(stmt, 0));
			value = strbuf_create_from((const char*) sqlite3_column_text(stmt, 1));
			output->identifier = strbuf_is_eq(key, VARNAM_METADATA_SCHEME_IDENTIFIER) ? strbuf_detach(value) : output->identifier;
			output->langCode = strbuf_is_eq(key, VARNAM_METADATA_SCHEME_LANGUAGE_CODE) ? strbuf_detach(value) : output->langCode;
			output->displayName = strbuf_is_eq(key, VARNAM_METADATA_SCHEME_DISPLAY_NAME) ? strbuf_detach(value) : output->displayName;
			output->author = strbuf_is_eq(key, VARNAM_METADATA_SCHEME_AUTHOR) ? strbuf_detach(value) : output->author;
			output->compiledDate = strbuf_is_eq(key, VARNAM_METADATA_SCHEME_COMPILED_DATE) ? strbuf_detach(value) : output->compiledDate;
			if (strbuf_is_eq(key, VARNAM_METADATA_SCHEME_STABLE)) {
				output->isStable = strcmp(strbuf_to_s(value), "1") == 0 ? 1 : 0;
				strbuf_destroy(value);
			}
		} else if (rc == SQLITE_DONE) {
			break;
		}
		else {
        set_last_error (handle, "Failed to get metadata : %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
				strbuf_destroy (key);
        return VARNAM_ERROR;
    }
		strbuf_clear (key);
	}

	strbuf_destroy (key);
  sqlite3_finalize( stmt );
	return VARNAM_SUCCESS;
}

static int
prepare_tokenization_stmt (varnam *handle, int tokenize_using, int match_type, sqlite3_stmt **stmt)
{
    int rc;

    switch (tokenize_using)
    {
    case VARNAM_TOKENIZER_PATTERN:
        if (v_->tokenize_using_pattern == NULL)
        {
            rc = sqlite3_prepare_v2( v_->db, "select id, type, match_type, pattern, value1, value2, value3, tag, priority, accept_condition, flags from symbols where pattern = ?1 and match_type = 1;",
                                     -1, &v_->tokenize_using_pattern, NULL );
            if (rc != SQLITE_OK) {
                set_last_error (handle, "Failed to tokenize : %s", sqlite3_errmsg(v_->db));
                return VARNAM_ERROR;
            }
        }
        *stmt = v_->tokenize_using_pattern;
        break;
    case VARNAM_TOKENIZER_VALUE:
        if (match_type == VARNAM_MATCH_ALL)
        {
            if (v_->tokenize_using_value == NULL)
            {
                rc = sqlite3_prepare_v2( v_->db, "select min(id) as id, type, match_type, lower(pattern) as pattern, value1, value2, value3, tag, priority, accept_condition, flags from symbols where value1 = ?1 or value2 = ?1 group by lower(pattern) order by priority desc, id asc;",
                                         -1, &v_->tokenize_using_value, NULL );
                if (rc != SQLITE_OK) {
                    set_last_error (handle, "Failed to tokenize : %s", sqlite3_errmsg(v_->db));
                    return VARNAM_ERROR;
                }
            }
            *stmt = v_->tokenize_using_value;
        }
        else
        {
            if (v_->tokenize_using_value_and_match_type == NULL)
            {
                rc = sqlite3_prepare_v2( v_->db, "select min(id) as id, type, match_type, lower(pattern) as pattern, value1, value2, value3, tag, priority, accept_condition, flags from symbols where (value1 = ?1 or value2 = ?1) and match_type = ?2 group by lower(pattern) order by priority desc, id asc;",
                                         -1, &v_->tokenize_using_value_and_match_type, NULL );
                if (rc != SQLITE_OK) {
                    set_last_error (handle, "Failed to tokenize : %s", sqlite3_errmsg(v_->db));
                    return VARNAM_ERROR;
                }
            }
            *stmt = v_->tokenize_using_value_and_match_type;
        }
        break;
    }

    return VARNAM_SUCCESS;
}

/* Gets the tokens from file and initialize tokens array with the result. tokens array will not be touched if there are no matches */
/* tokensAvailable will be set to TRUE when there are tokens available */
static int
read_all_tokens_and_add_to_array (varnam *handle, const char *lookup, int tokenize_using, int match_type, varray **tokens, bool *tokensAvailable)
{
    vtoken *tok = 0;
    bool initialized = false;
    int rc;
    sqlite3_stmt *stmt = 0;

    *tokensAvailable = false;
    rc = prepare_tokenization_stmt (handle, tokenize_using, match_type, &stmt);
    if (rc) return rc;

    sqlite3_bind_text (stmt, 1, lookup, -1, NULL);
    if (match_type != VARNAM_MATCH_ALL)
    {
        sqlite3_bind_int (stmt, 2, match_type);
    }
    while (true)
    {
        rc = sqlite3_step (stmt);
        if (rc == SQLITE_ROW)
        {
            tok = Token (
                    sqlite3_column_int( stmt, 0 ),
                    sqlite3_column_int( stmt, 1 ),
                    sqlite3_column_int( stmt, 2 ),
                    (const char*) sqlite3_column_text( stmt, 3 ),
                    (const char*) sqlite3_column_text( stmt, 4 ),
                    (const char*) sqlite3_column_text( stmt, 5 ),
                    (const char*) sqlite3_column_text( stmt, 6 ),
                    (const char*) sqlite3_column_text( stmt, 7 ),
                    sqlite3_column_int( stmt, 8 ),
                    sqlite3_column_int( stmt, 9 ),
                    sqlite3_column_int( stmt, 10 ));
            assert (tok);
            if (!initialized) {
                *tokens = varray_init ();
                initialized = true;
            }
            varray_push (*tokens, tok);
            *tokensAvailable = true;
        }
        else if (rc == SQLITE_DONE)
            break;
    }

    sqlite3_clear_bindings (stmt);
    rc = sqlite3_reset (stmt);
    if (rc != SQLITE_OK) {
        set_last_error (handle, "Failed to read tokens : %s", sqlite3_errmsg(v_->db));
        return VARNAM_ERROR;
    }

    return VARNAM_SUCCESS;
}

static int possibleToFindMatches = 1;
static int notPossibleToFindMatches = 0;

static int
can_find_more_matches(varnam *handle, varray *tokens, struct strbuf *lookup, int tokenize_using, bool *possible)
{
    int rc;
    int *cachedEntry;
    sqlite3_stmt *stmt = NULL;
    char candidate[500];
    vtoken *token;
    strbuf *cacheKey;

    assert (tokenize_using == VARNAM_TOKENIZER_PATTERN
        || tokenize_using == VARNAM_TOKENIZER_VALUE);

    if (tokens != NULL && !varray_is_empty (tokens)) {
        token = varray_get (tokens, 0);
        if (tokenize_using == VARNAM_TOKENIZER_PATTERN) {
            if (token->flags & VARNAM_TOKEN_FLAGS_MORE_MATCHES_FOR_PATTERN)
                *possible = true;
            else
                *possible = false;
        }
        else {
            if (token->flags & VARNAM_TOKEN_FLAGS_MORE_MATCHES_FOR_VALUE)
                *possible = true;
            else
                *possible = false;
        }

        return VARNAM_SUCCESS;
    }

    cacheKey = get_pooled_string (handle);
    strbuf_addf (cacheKey, "%s%d", strbuf_to_s (lookup), tokenize_using);
    cachedEntry = lru_find_in_cache (&v_->tokenizationPossibility, strbuf_to_s (cacheKey));
    if (cachedEntry) {
        *possible = *cachedEntry;
        return VARNAM_SUCCESS;
    }

    switch (tokenize_using)
    {
    case VARNAM_TOKENIZER_PATTERN:
        if (v_->can_find_more_matches_using_pattern == NULL)
        {
            rc = sqlite3_prepare_v2( v_->db, "select count(pattern) as cnt from symbols where pattern like ?1;",
                                     -1, &v_->can_find_more_matches_using_pattern, NULL );
            if (rc != SQLITE_OK) {
                set_last_error (handle, "Failed to prepare query for possible tokens detection : %s", sqlite3_errmsg(v_->db));
                return VARNAM_ERROR;
            }
        }
        stmt = v_->can_find_more_matches_using_pattern;
        break;
    case VARNAM_TOKENIZER_VALUE:
        if (v_->can_find_more_matches_using_value == NULL)
        {
            rc = sqlite3_prepare_v2( v_->db, "select count(pattern) as cnt from symbols where value1 like ?1 or value2 like ?1;",
                                     -1, &v_->can_find_more_matches_using_value, NULL );
            if (rc != SQLITE_OK) {
                set_last_error (handle, "Failed to prepare query for possible tokens detection : %s", sqlite3_errmsg(v_->db));
                return VARNAM_ERROR;
            }
        }
        stmt = v_->can_find_more_matches_using_value;
        break;
    }

    portable_snprintf( candidate, 500, "%s%s", strbuf_to_s (lookup), "%");
    sqlite3_bind_text (stmt, 1, candidate, -1, NULL);

    *possible = false;
    rc = sqlite3_step( stmt );
    if( rc == SQLITE_ROW ) {
        if( sqlite3_column_int( stmt, 0 ) > 0 ) {
            *possible = true;
        }
    }

    sqlite3_reset (stmt);

    /* caching for future use */
    if (*possible)
        lru_add_to_cache (&v_->tokenizationPossibility, strbuf_to_s (cacheKey), &possibleToFindMatches, NULL);
    else
        lru_add_to_cache (&v_->tokenizationPossibility, strbuf_to_s (cacheKey), &notPossibleToFindMatches, NULL);

    return VARNAM_SUCCESS;;
}

static void
destroy_tokens_cb (void *value)
{
    varray *array = value;
    varray_free (array, &destroy_token);
}

int
vst_tokenize (varnam *handle, const char *input, int tokenize_using, int match_type, varray *result)
{
    int rc, bytes_read = 0, matchpos = 0;
    const unsigned char *ustring; const char *inputcopy;
    struct strbuf *lookup, *cacheKey;
    vtoken *token;
    varray *tokens = NULL, *cachedEntry = NULL, *tmpTokens = NULL;
    bool possibility, tokensAvailable = false;

    if (input == NULL || *input == '\0') return VARNAM_SUCCESS;

    varray_clear (result);
    inputcopy = input;
    lookup = get_pooled_string (handle);
    cacheKey = get_pooled_string (handle);

    while (*inputcopy != '\0')
    {
        READ_A_UTF8_CHAR (ustring, inputcopy, bytes_read);

        strbuf_clear (lookup);
        strbuf_clear (cacheKey);
        strbuf_add_bytes (lookup, input, bytes_read);
        strbuf_addf (cacheKey, "%s%d%d", strbuf_to_s (lookup), tokenize_using, match_type);

        cachedEntry = lru_find_in_cache (&v_->tokens_cache, strbuf_to_s (cacheKey)); 
        if (cachedEntry != NULL) {
            tokens = get_pooled_array (handle);
            varray_copy (cachedEntry, tokens);
            assert (varray_length (tokens) > 0);
            tokensAvailable = true;
        }
        else if (lru_key_exists (&v_->noMatchesCache, strbuf_to_s (cacheKey))){
            tokensAvailable = false;
        }
        else {
            rc = read_all_tokens_and_add_to_array (handle,
                    strbuf_to_s (lookup),
                    tokenize_using,
                    match_type,
                    &tmpTokens, &tokensAvailable);
            if (rc) return rc;
            if (tokensAvailable) {
                assert (varray_length (tmpTokens) > 0);
                lru_add_to_cache (&v_->tokens_cache, strbuf_to_s (cacheKey), tmpTokens, destroy_tokens_cb);
                tokens = get_pooled_array (handle);
                varray_copy (tmpTokens, tokens);
            }
            else {
                /* this caching speeds up lookup which are not exists */
                lru_add_to_cache (&v_->noMatchesCache, strbuf_to_s (cacheKey), NULL, NULL);
            }
        }

        if (tokensAvailable) {
            matchpos = bytes_read;
            rc = can_find_more_matches (handle, tokens, lookup, tokenize_using, &possibility);
            if (rc) return rc;
        }
        else {
            rc = can_find_more_matches (handle, NULL, lookup, tokenize_using, &possibility);
            if (rc) return rc;
        }

        if (tokens == NULL || varray_is_empty (tokens))
        {
            /* We couldn't find any tokens. So adding lookup as the match */
            token = get_pooled_token (handle, -99,
                                      VARNAM_TOKEN_OTHER,
                                      VARNAM_MATCH_EXACT,
                                      strbuf_to_s (lookup), strbuf_to_s (lookup), "", "", "", 0, VARNAM_TOKEN_ACCEPT_ALL, 0);
            assert (token);
            if (tokens == NULL)
                tokens = get_pooled_array (handle);

            varray_push (tokens, token);
            matchpos = (int) lookup->length;
        }
        if (possibility && *inputcopy != '\0') continue;

        varray_push (result, tokens);
        bytes_read = 0;
        tokens = NULL;
        tokensAvailable = false;

        input = input + matchpos;
        inputcopy = input;
    }

    return VARNAM_SUCCESS;
}

int
vst_has_stemrules (varnam *handle)
{
    int rc;
    sqlite3 *db;
    sqlite3_stmt *stmt;
    const char *sql = "select count(*) from stemrules;";
    
    db = handle->internal->db;

    if(v_->stemrules_count == -1)
    {
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        if(rc != SQLITE_OK) {
            set_last_error(handle, "Failed to prepare :", sqlite3_errmsg(db));
            return VARNAM_ERROR;
        }

        rc = sqlite3_step(stmt);

        if(rc == SQLITE_ROW) {
            v_->stemrules_count = sqlite3_column_int(stmt, 0);
        }
        else {
            sqlite3_finalize(stmt);
            set_last_error(handle,"Error : ", sqlite3_errmsg(db));
            return VARNAM_ERROR;
        }
        sqlite3_finalize(stmt);
    }

    return v_->stemrules_count > 0 ? VARNAM_STEMRULE_HIT : VARNAM_STEMRULE_MISS;
}

int
vst_get_last_syllable (varnam *handle, strbuf *string, strbuf *syllable)
{
    int rc, flag=0, type;
    char *ending;
    sqlite3 *db;
    sqlite3_stmt *stmt;
    const char *sql = "select type from symbols where value1 = ?1";
    strbuf *temp;

    db = handle->internal->db;

    if(v_->get_last_syllable == NULL)
    {
        rc = sqlite3_prepare_v2(db, sql, -1, &v_->get_last_syllable, NULL);
        
        if(rc != SQLITE_OK)
        {
            set_last_error(handle, "Failed to prepare : ", sqlite3_errmsg(db));
            return VARNAM_ERROR;
        }
    }
    
    stmt = v_->get_last_syllable;
    temp = get_pooled_string(handle);
    strbuf_clear(syllable);

    while(!flag)
    {
        ending = strbuf_get_last_unicode_char(string);
        if(ending == NULL)
        {
            /*Restoring the string*/
            strbuf_clear(string);
            strbuf_add(string, strbuf_to_s(syllable));
            set_last_error(handle, "ending is null");
            return VARNAM_ERROR;
        }

        sqlite3_bind_text(stmt, 1, ending, -1, NULL);

        rc = sqlite3_step(stmt);
        if(rc == SQLITE_ROW)
        {
            /*See if our ending is a consonant (type 2)
            if it is not type 2, then it is a swara.
            If it is a swara, keep adding it to syllable buffer.
            Stop when a consonant is encountered*/
            type = sqlite3_column_int(stmt, 0);
            if(type == 2)
            {
                flag = 1;
            }
        }
        else if (rc != SQLITE_DONE)
        {
            set_last_error (handle, "Failed : %s", sqlite3_errmsg(db));
            sqlite3_reset (stmt);
            free(ending);
            return VARNAM_ERROR;
        }

        strbuf_clear(temp);
        strbuf_add(temp, ending);
        strbuf_add(temp, strbuf_to_s(syllable));
        strbuf_clear(syllable);
        strbuf_add(syllable, strbuf_to_s(temp));

        if(strbuf_remove_from_last(string, ending) == false)
        {
            free(ending);
            set_last_error(handle, "vst_get_last_syllable : could not remove last character");
            return VARNAM_ERROR;
        }

        sqlite3_reset(stmt);
        free(ending);
    }

    /*Restoring the string*/
    strbuf_add(string, strbuf_to_s(syllable));
   
    return VARNAM_SUCCESS;
}

int
vst_get_stem(varnam* handle, strbuf* old_ending, strbuf *new_ending)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    strbuf *cachedEntry=NULL;
    strbuf *cacheKey=NULL;
    strbuf *val_buf=NULL;
    int rc;
    const char *sql="select new_ending from stemrules where old_ending = ?1;";

    db = handle->internal->db;

    cacheKey = get_pooled_string (handle);
    strbuf_addf (cacheKey, "%s", strbuf_to_s (old_ending));
    cachedEntry = lru_find_in_cache(&v_->cached_stems, strbuf_to_s(cacheKey));
    if(cachedEntry != NULL)
    {
        strbuf_clear(new_ending);
        strbuf_add(new_ending, strbuf_to_s(cachedEntry));
        return VARNAM_STEMRULE_HIT;
    }

    if(v_->get_stemrule == NULL)
    {
        rc = sqlite3_prepare_v2(db, sql, -1, &v_->get_stemrule, NULL);
        if(rc != SQLITE_OK)
        {
            set_last_error(handle, "Failed to prepare statement : %s", sqlite3_errmsg(db));
            return VARNAM_ERROR;
        }
    }

    stmt = v_->get_stemrule;
    sqlite3_bind_text(stmt, 1, strbuf_to_s(old_ending), -1, NULL);
    rc = sqlite3_step(stmt);

    if(rc == SQLITE_ROW)
    {
        strbuf_clear(new_ending);
        strbuf_add(new_ending, (const char*)sqlite3_column_text(stmt, 0));
        sqlite3_reset(stmt);

        /*Will be freed by the callback in lru_add_cache*/
        /*can't use get_pooled_string() here. Unpredictable results*/
        /*possible leak if lru_add_cache never callbacks*/
        val_buf = strbuf_init(8);
        strbuf_add(val_buf, strbuf_to_s(new_ending));

        lru_add_to_cache(&v_->cached_stems, (const char*)strbuf_to_s(old_ending), val_buf, strbuf_destroy);
        return VARNAM_STEMRULE_HIT;
    }
    else if(rc == SQLITE_DONE)
    {
        sqlite3_reset(stmt);/*sqlite3_reset()*/
        return VARNAM_STEMRULE_MISS;
    }
    else
    {
        sqlite3_reset(stmt);
        set_last_error(handle, "Sqlite error : %s", sqlite3_errmsg(db));
        return VARNAM_ERROR;
    }

}

int
vst_check_exception(varnam *handle, strbuf *word_buffer, strbuf *end_buffer)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    strbuf *syllable = get_pooled_string(handle);
    int rc;
    const char *sql = "select exception from stem_exceptions where stem = ?1";

    db = handle->internal->db;

    if(v_->check_exception == NULL)
    {
        rc = sqlite3_prepare_v2(db, sql, -1, &v_->check_exception, NULL);
        if(rc != SQLITE_OK)
        {
            set_last_error(handle, "Failed to initialize statement : %s", sqlite3_errmsg(db));
            return VARNAM_ERROR;
        }
    }
    
    stmt = v_->check_exception;
    rc = sqlite3_bind_text(stmt, 1, strbuf_to_s(end_buffer), -1, NULL);
    if(rc != SQLITE_OK)
    {   
        set_last_error(handle, "Failed to initialize statement : %s", sqlite3_errmsg(db));
        sqlite3_reset( stmt );
        return VARNAM_ERROR;
    }

    rc = vst_get_last_syllable(handle, word_buffer, syllable);
    if(rc != VARNAM_SUCCESS)
    {
        sqlite3_reset(stmt);
        set_last_error(handle, "Could not obtain last syllable");
        return VARNAM_SUCCESS;
    }

    rc = sqlite3_step(stmt);
    if(rc == SQLITE_ROW)
    {
        if(sqlite3_column_bytes(stmt,0) != 0)
        {
            if(strcmp(strbuf_to_s(syllable), (const char*)sqlite3_column_blob(stmt, 0)) == 0)
            {
                sqlite3_reset(stmt);
                return VARNAM_STEMRULE_HIT;
            }
            else
            {
                sqlite3_reset(stmt);
                return VARNAM_STEMRULE_MISS;
            }
        }
    }
    else if(rc == SQLITE_DONE)
    {
        sqlite3_reset(stmt);
        return VARNAM_SUCCESS;
    }

    sqlite3_reset(stmt);
    return VARNAM_ERROR;
}


int
vst_stamp_version (varnam *handle)
{
    char *zErrMsg = 0;
    int rc;
    strbuf *sql;

    assert(handle);
    assert(handle->internal->db);

    sql = strbuf_init (30);
    strbuf_addf (sql, "PRAGMA user_version=%d;", VARNAM_SCHEMA_SYMBOLS_VERSION);
    rc = sqlite3_exec(v_->db, strbuf_to_s (sql), NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
        set_last_error (handle, "Failed to stamp schema version : %s", zErrMsg);
        sqlite3_free(zErrMsg);
        return VARNAM_STORAGE_ERROR;
    }

    strbuf_destroy (sql);
    return VARNAM_SUCCESS;
}

void
destroy_all_statements(struct varnam_internal* v)
{
    if (v == NULL)
        return;

    sqlite3_finalize (v->tokenize_using_pattern);
    sqlite3_finalize (v->tokenize_using_value);
    sqlite3_finalize (v->tokenize_using_value_and_match_type);
    sqlite3_finalize (v->can_find_more_matches_using_pattern);
    sqlite3_finalize (v->can_find_more_matches_using_value);
    sqlite3_finalize (v->learn_word);
    sqlite3_finalize (v->learn_pattern);
    sqlite3_finalize (v->get_word);
    sqlite3_finalize (v->get_suggestions);
    sqlite3_finalize (v->get_best_match);
    sqlite3_finalize (v->get_matches_for_word);
    sqlite3_finalize (v->possible_to_find_matches);
    sqlite3_finalize (v->update_confidence);
    sqlite3_finalize (v->update_learned_flag);
    sqlite3_finalize (v->delete_pattern);
    sqlite3_finalize (v->delete_word);
    sqlite3_finalize (v->export_words);
    sqlite3_finalize (v->learned_words_count);
    sqlite3_finalize (v->all_words_count);
    sqlite3_finalize (v->check_exception);
    sqlite3_finalize (v->get_last_syllable);
    sqlite3_finalize (v->get_stemrule);
    sqlite3_finalize (v->persist_stemrule);
    sqlite3_finalize (v->persist_stem_exception);
}
