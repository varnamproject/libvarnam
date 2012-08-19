/* Functions to handle the symbols table
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
        "create table if not exists symbols (id INTEGER PRIMARY KEY AUTOINCREMENT, type INTEGER, pattern TEXT, value1 TEXT, value2 TEXT, value3 TEXT, tag TEXT, match_type INTEGER);";

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

int
vst_persist_token(
    varnam *handle,
    const char *pattern,
    const char *value1,
    const char *value2,
    const char *value3,
    const char *tag,
    int token_type,
    int match_type)
{
    int rc, persisted;
    sqlite3 *db; sqlite3_stmt *stmt;
    const char *sql = "insert into symbols (type, pattern, value1, value2, value3, tag, match_type) values (?1, trim(?2), trim(?3), trim(?4), trim(?5), trim(?6), ?7);";

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

    rc = sqlite3_prepare_v2( db, "select id, type, match_type, pattern, value1, value2, value3, tag from symbols where type = ?1 and match_type = ?2 limit 1;", -1, &stmt, NULL );
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
                        (const char*) sqlite3_column_text(stmt, 7));

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

    rc = sqlite3_prepare_v2( db, "select id, type, match_type, pattern, value1, value2, value3, tag from symbols where type = ?1;", -1, &stmt, NULL );
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
                                    (const char*) sqlite3_column_text(stmt, 7));

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
vst_generate_cv_combinations(varnam* handle)
{
    int rc, config_ignore_duplicate_tokens_old_val, new_match_type, i , j;
    struct token *vowel = NULL,
                 *consonant = NULL,
                 *virama = NULL;
    varray *vowels, *consonants;
    char cons_pattern[VARNAM_SYMBOL_MAX], cons_value1[VARNAM_SYMBOL_MAX], cons_value2[VARNAM_SYMBOL_MAX];
    char newpattern[VARNAM_SYMBOL_MAX], newvalue1[VARNAM_SYMBOL_MAX], newvalue2[VARNAM_SYMBOL_MAX];

    set_last_error (handle, NULL);

    rc = vst_get_virama(handle, &virama);
    if (rc != VARNAM_SUCCESS)
        return rc;
    else if (virama == NULL)
    {
        set_last_error (handle, "Virama needs to be set before generating consonant vowel combinations");
        return VARNAM_ERROR;
    }

    vowels = varray_init ();
    rc = vst_get_all_tokens(handle, VARNAM_TOKEN_VOWEL, vowels);
    if (rc != VARNAM_SUCCESS)
        return rc;

    consonants = varray_init ();
    rc = vst_get_all_tokens(handle, VARNAM_TOKEN_DEAD_CONSONANT, consonants);
    if (rc != VARNAM_SUCCESS)
        return rc;

    /* When generating CV combinations, we ignore duplicate token check. duplicate token
       won't generate an error, instead it won't be persisted. */
    config_ignore_duplicate_tokens_old_val = handle->internal->config_ignore_duplicate_tokens;
    handle->internal->config_ignore_duplicate_tokens = 1;

    for (i = 0; i < varray_length (consonants); i++)
    {
        consonant = (struct token*) varray_get (consonants, i);
        /* Since we are iterating over dead consonants, pattern, value1 and value2 will have a virama at the end.
           This needs to removed before appending vowel */
        strncpy(cons_pattern, consonant->pattern, VARNAM_SYMBOL_MAX);
        strncpy(cons_value1, consonant->value1, VARNAM_SYMBOL_MAX);
        cons_value2[0] = '\0';
        if(consonant->value2 && consonant->value2[0] != '\0') {
            strncpy(cons_value2, consonant->value2, VARNAM_SYMBOL_MAX);
        }

        remove_from_last(cons_pattern, virama->pattern);
        remove_from_last(cons_value1, virama->value1);
        remove_from_last(cons_value2, virama->value1);

        for (j = 0; j < varray_length (vowels); j++)
        {
            vowel = (struct token*) varray_get (vowels, j);
            newpattern[0] = '\0';
            newvalue1[0]  = '\0';
            newvalue2[0]  = '\0';

            snprintf(newpattern, VARNAM_SYMBOL_MAX, "%s%s", cons_pattern, vowel->pattern);
            if(vowel->value2 && strlen(vowel->value2) != 0)
            {
                snprintf(newvalue1, VARNAM_SYMBOL_MAX, "%s%s", cons_value1, vowel->value2);
                if(cons_value2[0] != '\0')
                    snprintf(newvalue2, VARNAM_SYMBOL_MAX, "%s%s", cons_value2, vowel->value2);
            }
            else {
                snprintf(newvalue1, VARNAM_SYMBOL_MAX, "%s", cons_value1);
                if(cons_value2[0] != '\0')
                    snprintf(newvalue2, VARNAM_SYMBOL_MAX, "%s", cons_value2);
            }

            new_match_type = VARNAM_MATCH_EXACT;
            if (consonant->match_type == VARNAM_MATCH_POSSIBILITY ||
                vowel->match_type == VARNAM_MATCH_POSSIBILITY)
            {
                new_match_type = VARNAM_MATCH_POSSIBILITY;
            }
            rc = varnam_create_token(handle,
                                     newpattern,
                                     newvalue1,
                                     newvalue2, "", "", VARNAM_TOKEN_CONSONANT_VOWEL, new_match_type, 1);

            if (rc != VARNAM_SUCCESS)
            {
                handle->internal->config_ignore_duplicate_tokens = config_ignore_duplicate_tokens_old_val;
                return rc;
            }
        }
    }

    handle->internal->config_ignore_duplicate_tokens = config_ignore_duplicate_tokens_old_val;

    rc = varnam_flush_buffer(handle);
    if (rc != VARNAM_SUCCESS)
        return rc;

    varray_free (consonants, &destroy_token);
    varray_free (vowels, &destroy_token);

    return VARNAM_SUCCESS;
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
            rc = sqlite3_prepare_v2( v_->db, "select id, type, match_type, pattern, value1, value2, value3, tag from symbols where pattern = ?1 and match_type = 1;",
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
                rc = sqlite3_prepare_v2( v_->db, "select id, type, match_type, pattern, value1, value2, value3, tag from symbols where value1 = ?1 or value2 = ?1;",
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
                rc = sqlite3_prepare_v2( v_->db, "select id, type, match_type, pattern, value1, value2, value3, tag from symbols where (value1 = ?1 or value2 = ?1) and match_type = ?2;",
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

static int
read_all_tokens_and_add_to_array (varnam *handle, const char *lookup, int tokenize_using, int match_type, varray *tokens, bool *tokens_available)
{
    vtoken *tok = 0;
    bool cleared = false;
    int rc;
    sqlite3_stmt *stmt = 0;

    rc = prepare_tokenization_stmt (handle, tokenize_using, match_type, &stmt);
    if (rc) return rc;

    *tokens_available = false;
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
            tok = get_pooled_token (handle,
                                    sqlite3_column_int( stmt, 0 ),
                                    sqlite3_column_int( stmt, 1 ),
                                    sqlite3_column_int( stmt, 2 ),
                                    (const char*) sqlite3_column_text( stmt, 3 ),
                                    (const char*) sqlite3_column_text( stmt, 4 ),
                                    (const char*) sqlite3_column_text( stmt, 5 ),
                                    (const char*) sqlite3_column_text( stmt, 6 ),
                                    (const char*) sqlite3_column_text( stmt, 7 ));
            assert (tok);

            if (!cleared) {
                /* We have new set of tokens and we don't care about previous match */
                varray_clear (tokens);
                cleared = true;
            }

            varray_push (tokens, tok);
            *tokens_available = true;
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

static bool
can_find_more_matches(varnam *handle, struct strbuf *lookup, int tokenize_using, bool *possible)
{
    int rc;
    sqlite3_stmt *stmt;
    char candidate[500];

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

int
vst_tokenize (varnam *handle, const char *input, int tokenize_using, int match_type, varray *result)
{
    int rc, bytes_read = 0, matchpos = 0;
    const unsigned char *ustring; const char *inputcopy;
    struct strbuf *lookup;
    vtoken *token;
    varray *tokens = NULL;
    bool possibility, tokens_available = false;

    if (input == NULL || *input == '\0') return VARNAM_SUCCESS;

    varray_clear (result);
    inputcopy = input;
    lookup = get_pooled_string (handle);

    while (*inputcopy != '\0')
    {
        READ_A_UTF8_CHAR (ustring, inputcopy, bytes_read);

        strbuf_clear (lookup);
        strbuf_add_bytes (lookup, input, bytes_read);

        if (tokens == NULL)
            tokens = get_pooled_array (handle);

        rc = read_all_tokens_and_add_to_array (handle,
                                               strbuf_to_s (lookup),
                                               tokenize_using,
                                               match_type,
                                               tokens,
                                               &tokens_available);
        if (rc) return rc;

        if (tokens_available)
            matchpos = bytes_read;

        if (varray_is_empty (tokens))
        {
            /* We couldn't find any tokens. So adding lookup as the match */
            token = get_pooled_token (handle, -99,
                                      VARNAM_TOKEN_OTHER,
                                      VARNAM_MATCH_EXACT,
                                      strbuf_to_s (lookup), strbuf_to_s (lookup), "", "", "");
            assert (token);
            varray_push (tokens, token);
            matchpos = (int) lookup->length;
        }
        rc = can_find_more_matches (handle, lookup, tokenize_using, &possibility);
        if (rc) return rc;
        if (possibility && *inputcopy != '\0') continue;

        varray_push (result, tokens);
        bytes_read = 0;
        tokens = NULL;

        input = input + matchpos;
        inputcopy = input;
    }

    return VARNAM_SUCCESS;
}

void
destroy_all_statements(varnam *handle)
{
    sqlite3_finalize (v_->tokenize_using_pattern);
    sqlite3_finalize (v_->tokenize_using_value);
    sqlite3_finalize (v_->tokenize_using_value_and_match_type);
    sqlite3_finalize (v_->can_find_more_matches_using_pattern);
    sqlite3_finalize (v_->can_find_more_matches_using_value);
    sqlite3_finalize (v_->learn_word);
    sqlite3_finalize (v_->learn_pattern);
    sqlite3_finalize (v_->get_word);
    sqlite3_finalize (v_->get_suggestions);
    sqlite3_finalize (v_->get_matches_for_word);
    sqlite3_finalize (v_->possible_to_find_matches);
    sqlite3_finalize (v_->update_confidence);
}
