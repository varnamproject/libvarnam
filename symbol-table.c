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
        "create table if not exists symbols (id INTEGER PRIMARY KEY AUTOINCREMENT, type INTEGER, pattern TEXT, value1 TEXT, value2 TEXT, value3 TEXT, tag TEXT, match_type INTEGER, priority INTEGER DEFAULT 0, accept_condition INTEGER, flags INTEGER DEFAULT 0);";

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

/*
 * Makes a prefix tree.
 *
 * Iterates over all the symbols, finds if a symbol can have more matches, if yes mark it in the flags and move on
 * flags will be a bit field which represents more matches for values and pattern
 * */
int
vst_make_prefix_tree (varnam *handle)
{
    int rc, symbolId, flags = 0;
    sqlite3 *db;
    sqlite3_stmt *selectSymbolsStmt = NULL, *patternChildrenStmt = NULL, *valueChildrenStmt = NULL, *updateFlagsStmt = NULL;
    strbuf *pattern, *value;

    db = handle->internal->db;

    rc = sqlite3_prepare_v2 (db, "select id, pattern, value1 from symbols;", -1, &selectSymbolsStmt, NULL);
    if (rc != SQLITE_OK) {
        set_last_error (handle, "Failed to prepare statement to get all symbols: %s", sqlite3_errmsg(db));
        sqlite3_finalize (selectSymbolsStmt);
        return VARNAM_ERROR;
    }

    rc = sqlite3_prepare_v2 (db, "select count(id) from symbols where pattern <> ?1 and pattern like ?1 || '%';", -1, &patternChildrenStmt, NULL);
    if (rc != SQLITE_OK) {
        set_last_error (handle, "Failed to prepare statement to get children for pattern: %s", sqlite3_errmsg(db));
        sqlite3_finalize (patternChildrenStmt);
        return VARNAM_ERROR;
    }

    rc = sqlite3_prepare_v2 (db, "select count(id) from symbols where (value1 <> ?1 and value2 <> ?1) and (value1 like ?1 || '%' or value2 like ?1 || '%');", 
            -1, &valueChildrenStmt, NULL);
    if (rc != SQLITE_OK) {
        set_last_error (handle, "Failed to prepare statement to get children for value: %s", sqlite3_errmsg(db));
        sqlite3_finalize (valueChildrenStmt);
        return VARNAM_ERROR;
    }

    rc = sqlite3_prepare_v2 (db, "update symbols set flags = ?1 where id = ?2;", -1, &updateFlagsStmt, NULL);
    if (rc != SQLITE_OK) {
        set_last_error (handle, "Failed to prepare statement to update flags for symbols: %s", sqlite3_errmsg(db));
        sqlite3_finalize (updateFlagsStmt);
        return VARNAM_ERROR;
    }

    pattern = get_pooled_string (handle);
    value = get_pooled_string (handle);
    for (;;) {
        flags = 0;

        rc = sqlite3_step (selectSymbolsStmt);
        if (rc == SQLITE_DONE)
            break;
        else if (rc != SQLITE_ROW) {
            set_last_error (handle, "Failed to select all symbols: %s", sqlite3_errmsg(db));
            sqlite3_finalize (selectSymbolsStmt);
            return VARNAM_ERROR;
        }

        symbolId = sqlite3_column_int (selectSymbolsStmt, 0);
        strbuf_add (pattern, (const char*) sqlite3_column_text (selectSymbolsStmt, 1));
        strbuf_add (value, (const char*) sqlite3_column_text (selectSymbolsStmt, 2));

        sqlite3_bind_text (patternChildrenStmt, 1, strbuf_to_s (pattern), -1, NULL);
        rc = sqlite3_step (patternChildrenStmt);
        if (rc != SQLITE_ROW) {
            set_last_error (handle, "Failed to find children for pattern: %s", sqlite3_errmsg(db));
            sqlite3_finalize (patternChildrenStmt);
            return VARNAM_ERROR;
        }
        if (sqlite3_column_int (patternChildrenStmt, 0) > 0)
            flags = VARNAM_TOKEN_FLAGS_MORE_MATCHES_FOR_PATTERN;
        sqlite3_reset (patternChildrenStmt);

        sqlite3_bind_text (valueChildrenStmt, 1, strbuf_to_s (value), -1, NULL);
        rc = sqlite3_step (valueChildrenStmt);
        if (rc != SQLITE_ROW) {
            set_last_error (handle, "Failed to find children for value: %s", sqlite3_errmsg(db));
            sqlite3_finalize (valueChildrenStmt);
            return VARNAM_ERROR;
        }
        if (sqlite3_column_int (valueChildrenStmt, 0) > 0)
            flags = flags | VARNAM_TOKEN_FLAGS_MORE_MATCHES_FOR_VALUE;
        sqlite3_reset (valueChildrenStmt);

        sqlite3_bind_int (updateFlagsStmt, 1, flags);
        sqlite3_bind_int (updateFlagsStmt, 2, symbolId);
        rc = sqlite3_step (updateFlagsStmt);
        if (rc != SQLITE_DONE) {
            set_last_error (handle, "Failed to make prefix tree: %s", sqlite3_errmsg(db));
            sqlite3_finalize (updateFlagsStmt);
            return VARNAM_ERROR;
        }
        sqlite3_reset (updateFlagsStmt);

        strbuf_clear (pattern);
        strbuf_clear (value);
    }

    sqlite3_finalize (selectSymbolsStmt);
    sqlite3_finalize (patternChildrenStmt);
    sqlite3_finalize (valueChildrenStmt);
    sqlite3_finalize (updateFlagsStmt);

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

int
vst_get_virama(varnam* handle, struct token **output)
{
    int rc;
    sqlite3 *db; sqlite3_stmt *stmt;

    if (v_->virama != NULL)
    {
        *output = v_->virama;
        return VARNAM_SUCCESS;
    }

    db = handle->internal->db;

    rc = sqlite3_prepare_v2( db, "select id, type, match_type, pattern, value1, value2, value3, tag, priority, accept_condition, flags from symbols where type = ?1 and match_type = ?2 limit 1;", -1, &stmt, NULL );
    if(rc != SQLITE_OK)
    {
        set_last_error (handle, "Failed to get virama : %s", sqlite3_errmsg(db));
        sqlite3_finalize( stmt );
        return VARNAM_ERROR;
    }

    sqlite3_bind_int (stmt, 1, VARNAM_TOKEN_VIRAMA);
    sqlite3_bind_int (stmt, 2, VARNAM_MATCH_EXACT);

    rc = sqlite3_step( stmt );
    if( rc == SQLITE_ROW )
    {
        /* Not using pooled token here because we need to cache this in v_->virama */
        *output = Token((int) sqlite3_column_int(stmt, 0),
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

        v_->virama = *output;
    }
    else if ( rc == SQLITE_DONE )
    {
        *output = NULL;
    }
    else
    {
        set_last_error (handle, "Failed to get virama : %s", sqlite3_errmsg(db));
        sqlite3_finalize( stmt );
        return VARNAM_ERROR;
    }

    sqlite3_finalize( stmt );

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
vst_get_metadata (varnam *handle, const char* key, struct strbuf *output)
{
    int rc;
    sqlite3 *db; sqlite3_stmt *stmt;

    assert (handle);
    strbuf_clear (output);

    db = handle->internal->db;

    rc = sqlite3_prepare_v2( db, "select value from metadata where key = ?1;", -1, &stmt, NULL );
    if(rc != SQLITE_OK)
    {
        set_last_error (handle, "Failed to get metadata : %s", sqlite3_errmsg(db));
        sqlite3_finalize( stmt );
        return VARNAM_ERROR;
    }

    sqlite3_bind_text(stmt, 1, key, -1, NULL);

    rc = sqlite3_step( stmt );
    if (rc == SQLITE_ROW)
        strbuf_add (output, (const char*) sqlite3_column_text( stmt, 0 ));
    else if (rc != SQLITE_DONE)
    {
        set_last_error (handle, "Failed to get metadata : %s", sqlite3_errmsg(db));
        sqlite3_finalize( stmt );
        return VARNAM_ERROR;
    }

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

static int
can_find_more_matches(varnam *handle, varray *tokens, struct strbuf *lookup, int tokenize_using, bool *possible)
{
    int rc;
    sqlite3_stmt *stmt = NULL;
    char candidate[500];
    vtoken *token;

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

    snprintf( candidate, 500, "%s%s", strbuf_to_s (lookup), "%");
    sqlite3_bind_text (stmt, 1, candidate, -1, NULL);

    *possible = false;
    rc = sqlite3_step( stmt );
    if( rc == SQLITE_ROW ) {
        if( sqlite3_column_int( stmt, 0 ) > 0 ) {
            *possible = true;
        }
    }

    sqlite3_reset (stmt);

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
}
