/* varnam-symbol-table.c
 *
 * Copyright (C) <year> <author>
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

#include "varnam-symbol-table.h"
#include "varnam-util.h"
#include "varnam-types.h"
#include "varnam-result-codes.h"
#include "varnam-api.h"

static struct token*
Token(int type, int match_type, const char* pattern, const char* value1, const char* value2, const char* tag)
{
    struct token* tok = (struct token*) xmalloc(sizeof(struct token));
    tok->type = type;
    tok->match_type = match_type;
    strncpy( tok->pattern, pattern, VARNAM_SYMBOL_MAX);
    strncpy( tok->value1, value1, VARNAM_SYMBOL_MAX);
    strncpy( tok->value2, value2, VARNAM_SYMBOL_MAX);
    strncpy( tok->tag, tag, VARNAM_TOKEN_TAG_MAX);
    tok->next = NULL;

    return tok;
}

struct token*
find_token(varnam *handle, const char *lookup)
{
    struct varnam_internal *internal;
    struct token *tok = NULL;
    char sql[500];
    const char *pattern, *value1, *value2, *tag;
    sqlite3_stmt *stmt; sqlite3 *db;
    int rc, type, has_children;

    assert( handle ); assert( lookup );

    internal = handle->internal;
    db = internal->db;

    snprintf( sql, 500, "select type, pattern, value1, value2, children, tag from symbols where pattern = ?1;");
    rc = sqlite3_prepare_v2( db, sql, 500, &stmt, NULL );
    if( rc == SQLITE_OK )
    {
        sqlite3_bind_text (stmt, 1, lookup, (int) strlen(lookup), NULL);
        rc = sqlite3_step (stmt);
        if( rc == SQLITE_ROW )
        {
            type = sqlite3_column_int( stmt, 0 );
            pattern = (const char*) sqlite3_column_text( stmt, 1 );
            value1 = (const char*) sqlite3_column_text( stmt, 2 );
            value2 = (const char*) sqlite3_column_text( stmt, 3 );
            has_children = sqlite3_column_int( stmt, 4 );
            tag = (const char*) sqlite3_column_text( stmt, 5 );

            if(internal->current_token == NULL) {
                internal->current_token = (struct token *) xmalloc(sizeof (struct token));
                assert( internal->current_token );
            }

            tok = internal->current_token;
            tok->type = type;
            strncpy( tok->pattern, pattern, VARNAM_SYMBOL_MAX);
            strncpy( tok->value1, value1, VARNAM_SYMBOL_MAX);
            strncpy( tok->value2, value2, VARNAM_SYMBOL_MAX);
            strncpy( tok->tag, tag, VARNAM_TOKEN_TAG_MAX);
            tok->children = has_children;
        }
    }

    sqlite3_finalize( stmt );
    return tok;
}

struct token*
find_rtl_token(varnam *handle, const char *lookup)
{
    struct varnam_internal *internal;
    struct token *tok = NULL;
    char sql[500];
    const char *pattern, *value1, *value2, *tag;
    sqlite3_stmt *stmt; sqlite3 *db;
    int rc, type, has_children;

    assert( handle ); assert( lookup );

    internal = handle->internal;
    db = internal->db;

    snprintf( sql, 500, "select type, pattern, value1, value2, children, tag from symbols where value1 = ?1 or value2 = ?1 limit 1;");
    rc = sqlite3_prepare_v2( db, sql, 500, &stmt, NULL );
    if( rc == SQLITE_OK )
    {
        sqlite3_bind_text (stmt, 1, lookup, (int) strlen(lookup), NULL);
        rc = sqlite3_step (stmt);
        if( rc == SQLITE_ROW )
        {
            type = sqlite3_column_int( stmt, 0 );
            pattern = (const char*) sqlite3_column_text( stmt, 1 );
            value1 = (const char*) sqlite3_column_text( stmt, 2 );
            value2 = (const char*) sqlite3_column_text( stmt, 3 );
            has_children = sqlite3_column_int( stmt, 4 );
            tag = (const char*) sqlite3_column_text( stmt, 5 );

            if(internal->current_rtl_token == NULL) {
                internal->current_rtl_token = (struct token *) xmalloc(sizeof (struct token));
                assert( internal->current_rtl_token );
            }

            tok = internal->current_rtl_token;
            tok->type = type;
            strncpy( tok->pattern, pattern, VARNAM_SYMBOL_MAX);
            strncpy( tok->value1, value1, VARNAM_SYMBOL_MAX);
            strncpy( tok->value2, value2, VARNAM_SYMBOL_MAX);
            strncpy( tok->tag, tag, VARNAM_TOKEN_TAG_MAX);
            tok->children = has_children;
        }
    }

    sqlite3_finalize( stmt );
    return tok;
}

int
can_find_token(varnam *handle, struct token *last, const char *lookup)
{
    char sql[500];
    sqlite3_stmt *stmt;
    sqlite3 *db;
    int rc; int result = 0;

    assert( lookup );
    assert( handle );

    db = handle->internal->db;

    snprintf( sql, 500, "select count(pattern) as cnt from symbols where pattern like '%s%%';", lookup );
    rc = sqlite3_prepare_v2( db, sql, 500, &stmt, NULL );
    if( rc == SQLITE_OK ) {
        rc = sqlite3_step( stmt );
        if( rc == SQLITE_ROW ) {
            if( sqlite3_column_int( stmt, 0 ) > 0 ) {
                result = 1;
            }
        }
    }

    sqlite3_finalize( stmt );
    return result;
}

int
can_find_rtl_token(varnam *handle, struct token *last, const char *lookup)
{
    char sql[500];
    sqlite3_stmt *stmt;
    sqlite3 *db;
    int rc; int result = 0;

    assert( lookup );
    assert( handle );

    db = handle->internal->db;

    snprintf( sql, 500, "select count(pattern) as cnt from symbols where value1 like '%s%%' or value2 like '%s%%';", lookup, lookup );
    rc = sqlite3_prepare_v2( db, sql, 500, &stmt, NULL );
    if( rc == SQLITE_OK ) {
        rc = sqlite3_step( stmt );
        if( rc == SQLITE_ROW ) {
            if( sqlite3_column_int( stmt, 0 ) > 0 ) {
                result = 1;
            }
        }
    }

    sqlite3_finalize( stmt );
    return result;
}

void
fill_general_values(varnam *handle, char *output, const char *name)
{
    char sql[500];
    const char *result;
    sqlite3_stmt *stmt;
    sqlite3 *db;
    int rc;

    assert( name );
    assert( handle );

    db = handle->internal->db;

    snprintf(sql, 500, "select value from general where key = ?1;");

    rc = sqlite3_prepare_v2( db, sql, 500, &stmt, NULL );
    if( rc == SQLITE_OK ) {
        sqlite3_bind_text(stmt, 1, name, (int) strlen(name), NULL);
        rc = sqlite3_step( stmt );
        if( rc == SQLITE_ROW )
        {
            result = (const char*) sqlite3_column_text(stmt, 0);
            if(result) {
                strncpy(output, result, VARNAM_SYMBOL_MAX);
            }
        }
    }

    sqlite3_finalize( stmt );
}

int
ensure_schema_exist(varnam *handle, char **msg)
{
    const char *sql =
        "create table if not exists general (key TEXT, value TEXT);"
        "create table if not exists symbols (type INTEGER, pattern TEXT, value1 TEXT, value2 TEXT, tag TEXT, match_type INTEGER);"
        "create index if not exists index_general on general (key);"
        "create index if not exists index_pattern on symbols (pattern);"
        "create index if not exists index_value1  on symbols (value1);"
        "create index if not exists index_value2  on symbols (value2);";

   char *zErrMsg = 0;
   int rc;

    assert(handle);
    assert(handle->internal->db);

    rc = sqlite3_exec(handle->internal->db, sql, NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
        asprintf(msg, "Failed to initialize output file : %s", zErrMsg);
        sqlite3_free(zErrMsg);
        return VARNAM_STORAGE_ERROR;
    }

    return VARNAM_SUCCESS;
}

int
vst_start_buffering(varnam *handle)
{
    char *zErrMsg, *msg;
    int rc;
    const char *sql = "BEGIN;";

    assert(handle);

    if (handle->internal->vst_buffering)
        return VARNAM_SUCCESS;

    rc = sqlite3_exec(handle->internal->db, sql, NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        asprintf(&msg, "Failed to start buffering : %s", zErrMsg);
        set_last_error (handle, msg);

        sqlite3_free(zErrMsg);
        xfree (msg);

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
    char *msg;
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
        asprintf(&msg, "Failed to check already persisted : %s", sqlite3_errmsg(db));
        set_last_error (handle, msg);
        xfree (msg);
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
        asprintf(&msg, "Failed to check already persisted : %s", sqlite3_errmsg(db));
        set_last_error (handle, msg);
        xfree (msg);
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
    int token_type,
    int match_type)
{
    int rc, persisted;
    char *msg;
    sqlite3 *db; sqlite3_stmt *stmt;
    const char *sql = "insert into symbols values (?1, trim(?2), trim(?3), trim(?4), trim(?5), ?6);";

    assert(handle); assert(pattern); assert(value1); assert(token_type);

    rc = already_persisted (handle, pattern, value1, match_type, &persisted);
    if (rc != VARNAM_SUCCESS)
        return rc;

    if (persisted)
    {
        if (handle->internal->config_ignore_duplicate_tokens)
        {
            asprintf(&msg, "%s => %s is already available. Ignoring duplicate tokens", pattern, value1);
            log (msg);
            xfree (msg);
            return VARNAM_SUCCESS;
        }

        asprintf(&msg, "%s => %s is already available. Duplicate entries are not allowed", pattern, value1);
        set_last_error (handle, msg);
        xfree (msg);
        return VARNAM_ERROR;
    }

    db = handle->internal->db;

    rc = sqlite3_prepare_v2( db, sql, -1, &stmt, NULL );
    if(rc != SQLITE_OK)
    {
        asprintf(&msg, "Failed to initialize statement : %s", sqlite3_errmsg(db));
        set_last_error (handle, msg);
        xfree (msg);
        sqlite3_finalize( stmt );
        return VARNAM_ERROR;
    }

    sqlite3_bind_int (stmt, 1, token_type);
    sqlite3_bind_text(stmt, 2, pattern,    -1, NULL);
    sqlite3_bind_text(stmt, 3, value1,     -1, NULL);
    sqlite3_bind_text(stmt, 4, value2 == NULL ? "" : value2,     -1, NULL);
    sqlite3_bind_text(stmt, 5, "",         -1, NULL);
    sqlite3_bind_int (stmt, 6, match_type);

    rc = sqlite3_step( stmt );
    if( rc != SQLITE_DONE )
    {
        asprintf(&msg, "Failed to persist token : %s", sqlite3_errmsg(db));
        set_last_error (handle, msg);
        xfree (msg);
        sqlite3_finalize( stmt );
        return VARNAM_ERROR;
    }

    sqlite3_finalize( stmt );

    return VARNAM_SUCCESS;
}

int
vst_flush_changes(varnam *handle)
{
    char *zErrMsg, *msg;
    int rc;
    const char *sql = "COMMIT;";

    assert(handle);

    if (!handle->internal->vst_buffering)
        return VARNAM_SUCCESS;

    rc = sqlite3_exec(handle->internal->db, sql, NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        asprintf(&msg, "Failed to flush changes : %s", zErrMsg);
        set_last_error (handle, msg);

        sqlite3_free(zErrMsg);
        xfree (msg);

        return VARNAM_STORAGE_ERROR;
    }

    handle->internal->vst_buffering = 0;
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
vst_get_virama(varnam* handle, char *output)
{
    int rc;
    char *msg;
    sqlite3 *db; sqlite3_stmt *stmt;
    const char* result;

    db = handle->internal->db;

    rc = sqlite3_prepare_v2( db, "select value1 from symbols where type = ?1 and match_type = ?2 limit 1;", -1, &stmt, NULL );
    if(rc != SQLITE_OK)
    {
        asprintf(&msg, "Failed to get virama : %s", sqlite3_errmsg(db));
        set_last_error (handle, msg);
        xfree (msg);
        sqlite3_finalize( stmt );
        return VARNAM_ERROR;
    }

    sqlite3_bind_int (stmt, 1, VARNAM_TOKEN_VIRAMA);
    sqlite3_bind_int (stmt, 2, VARNAM_MATCH_EXACT);

    rc = sqlite3_step( stmt );
    if( rc == SQLITE_ROW )
    {
        result = (const char*) sqlite3_column_text(stmt, 0);
        if(result) {
            strncpy(output, result, VARNAM_SYMBOL_MAX);
        }
    }
    else if ( rc == SQLITE_DONE )
    {
        output[0] = '\0';
    }
    else
    {
        asprintf(&msg, "Failed to check already persisted : %s", sqlite3_errmsg(db));
        set_last_error (handle, msg);
        xfree (msg);
        sqlite3_finalize( stmt );
        return VARNAM_ERROR;
    }

    sqlite3_finalize( stmt );

    return VARNAM_SUCCESS;
}

int
vst_get_all_tokens (varnam* handle, int token_type, struct token **tokens)
{
    struct token *head = NULL, *tail = NULL, *tok = NULL;
    int rc;
    char *msg;
    sqlite3 *db; sqlite3_stmt *stmt;

    db = handle->internal->db;

    rc = sqlite3_prepare_v2( db, "select type, match_type, pattern, value1, value2, tag from symbols where type = ?1;", -1, &stmt, NULL );
    if(rc != SQLITE_OK)
    {
        asprintf(&msg, "Failed to get all tokens : %s", sqlite3_errmsg(db));
        set_last_error (handle, msg);
        xfree (msg);
        sqlite3_finalize( stmt );
        return VARNAM_ERROR;
    }

    sqlite3_bind_int (stmt, 1, token_type);

    while(1)
    {
        rc = sqlite3_step( stmt );
        if( rc == SQLITE_ROW )
        {
            tok = Token( (int) sqlite3_column_int(stmt, 0),
                         (int) sqlite3_column_int(stmt, 1),
                         (const char*) sqlite3_column_text(stmt, 2),
                         (const char*) sqlite3_column_text(stmt, 3),
                         (const char*) sqlite3_column_text(stmt, 4),
                         (const char*) sqlite3_column_text(stmt, 5));

            if (head == NULL)
            {
                head = tok;
            }
            else if (head->next == NULL)
            {
                head->next = tok;
                tail = tok;
            }
            else
            {
                tail->next = tok;
                tail = tok;
            }
        }
        else if ( rc == SQLITE_DONE )
            break;
        else
        {
            asprintf(&msg, "Failed to get all tokens : %s", sqlite3_errmsg(db));
            set_last_error (handle, msg);
            xfree (msg);
            sqlite3_finalize( stmt );
            return VARNAM_ERROR;
        }
    }

    sqlite3_finalize( stmt );

    *tokens = head;
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
    int rc, config_ignore_duplicate_tokens_old_val, new_match_type;
    struct token *vowels = NULL,
                 *vowel = NULL,
                 *consonants = NULL,
                 *consonant = NULL;
    char cons_value1[VARNAM_SYMBOL_MAX], cons_value2[VARNAM_SYMBOL_MAX], virama[VARNAM_SYMBOL_MAX];
    char newpattern[VARNAM_SYMBOL_MAX], newvalue1[VARNAM_SYMBOL_MAX], newvalue2[VARNAM_SYMBOL_MAX];

    set_last_error (handle, NULL);

    rc = vst_get_virama(handle, virama);
    if (rc != VARNAM_SUCCESS)
        return rc;
    else if (virama[0] == '\0')
    {
        set_last_error (handle, "Virama needs to be set before generating consonant vowel combinations");
        return VARNAM_ERROR;
    }

    rc = vst_get_all_tokens(handle, VARNAM_TOKEN_VOWEL, &vowels);
    if (rc != VARNAM_SUCCESS)
        return rc;

    rc = vst_get_all_tokens(handle, VARNAM_TOKEN_DEAD_CONSONANT, &consonants);
    if (rc != VARNAM_SUCCESS)
        return rc;

    /* When generating CV combinations, we ignore duplicate token check. duplicate token
       won't generate an error, instead it won't be persisted. */
    config_ignore_duplicate_tokens_old_val = handle->internal->config_ignore_duplicate_tokens;
    handle->internal->config_ignore_duplicate_tokens = 1;

    varnam_tokens_for_each(consonant, consonants)
    {
        /* Since we are iterating over dead consonants, value1 and value2 will have a virama at the end.
           This needs to removed before appending vowel */
        strncpy(cons_value1, consonant->value1, VARNAM_SYMBOL_MAX);
        cons_value2[0] = '\0';
        if(consonant->value2 && consonant->value2[0] != '\0') {
            strncpy(cons_value2, consonant->value2, VARNAM_SYMBOL_MAX);
        }

        remove_from_last(cons_value1, virama);
        remove_from_last(cons_value2, virama);

        varnam_tokens_for_each(vowel, vowels)
        {
            newpattern[0] = '\0';
            newvalue1[0]  = '\0';
            newvalue2[0]  = '\0';

            snprintf(newpattern, VARNAM_SYMBOL_MAX, "%s%s", consonant->pattern, vowel->pattern);
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
                                     newvalue2, VARNAM_TOKEN_CONSONANT_VOWEL, new_match_type, 1);

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

    varnam_tokens_free(vowel, vowels);
    varnam_tokens_free(consonant, consonants);

    return VARNAM_SUCCESS;
}
