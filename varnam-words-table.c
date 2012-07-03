/* <file-name>
 *
 * Copyright (C) Navaneeth K N
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

#include <assert.h>

#include "varnam-symbol-table.h"
#include "varnam-util.h"
#include "varnam-types.h"
#include "varnam-result-codes.h"
#include "varnam-api.h"

int
vwt_ensure_schema_exists(varnam *handle)
{
    const char *pragmas =
        "pragma page_size=4096;"
        "pragma journal_mode=wal;";

    const char *tables =

        "create         table if not exists metadata (key TEXT UNIQUE, value TEXT);"
        "create         table if not exists words (id integer primary key, word text unique, confidence integer, learned_on date);"
        "create         table if not exists patterns_content (pattern text, word_id integer, primary key(pattern, word_id));"
        "create virtual table if not exists patterns using fts4(content='patterns_content', pattern text, word_id integer);"
        "create virtual table if not exists words_substrings using fts4(word text unique);";

    const char *triggers1 =
        "create trigger if not exists pc_bu before update on patterns_content begin delete from patterns where docid = old.rowid; end;"
        "create trigger if not exists pc_bd before delete on patterns_content begin delete from patterns where docid = old.rowid; end;";

    const char *triggers2 =
        "create trigger if not exists pc_au after  update on patterns_content begin insert into patterns (docid, pattern, word_id) values (new.rowid, new.pattern, new.word_id); end;"
        "create trigger if not exists pc_ai after  insert on patterns_content begin insert into patterns (docid, pattern, word_id) values (new.rowid, new.pattern, new.word_id); end;";

    char *zErrMsg = 0;
    int rc;

    rc = sqlite3_exec(v_->known_words, pragmas, NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
        set_last_error (handle, "Failed to initialize file for storing known words : %s", zErrMsg);
        sqlite3_free(zErrMsg);
        return VARNAM_ERROR;
    }

    rc = sqlite3_exec(v_->known_words, tables, NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
        set_last_error (handle, "Failed to initialize file for storing known words : %s", zErrMsg);
        sqlite3_free(zErrMsg);
        return VARNAM_ERROR;
    }

    rc = sqlite3_exec(v_->known_words, triggers1, NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
        set_last_error (handle, "Failed to initialize file for storing known words : %s", zErrMsg);
        sqlite3_free(zErrMsg);
        return VARNAM_ERROR;
    }

    rc = sqlite3_exec(v_->known_words, triggers2, NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
        set_last_error (handle, "Failed to initialize file for storing known words : %s", zErrMsg);
        sqlite3_free(zErrMsg);
        return VARNAM_ERROR;
    }

    return VARNAM_SUCCESS;
}

static int
execute_sql(varnam *handle, sqlite3 *db, const char *sql)
{
    char *zErrMsg = 0;
    int rc;

    rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
        set_last_error (handle, "Failed to write : %s", zErrMsg);
        sqlite3_free(zErrMsg);
        return VARNAM_ERROR;
    }

    return VARNAM_SUCCESS;
}

static int
learn_pattern (varnam *handle, const char *pattern, sqlite3_int64 word_id)
{
    int rc;
    const char *sql = "insert or ignore into patterns_content (pattern, word_id) values (trim(lower(?1)), ?2)";

    assert (v_->known_words);

    if (v_->learn_pattern == NULL)
    {
        rc = sqlite3_prepare_v2( v_->known_words, sql, -1, &v_->learn_pattern, NULL );
        if (rc != SQLITE_OK) {
            set_last_error (handle, "Failed to learn word : %s", sqlite3_errmsg(v_->known_words));
            sqlite3_reset (v_->learn_pattern);
            return VARNAM_ERROR;
        }
    }

    sqlite3_bind_text  (v_->learn_pattern, 1, pattern, -1, NULL);
    sqlite3_bind_int64 (v_->learn_pattern, 2, word_id);

    rc = sqlite3_step (v_->learn_pattern);
    if (rc != SQLITE_DONE) {
        set_last_error (handle, "Failed to learn pattern : %s", sqlite3_errmsg(v_->known_words));
        sqlite3_reset (v_->learn_pattern);
        return VARNAM_ERROR;
    }

    sqlite3_reset (v_->learn_pattern);
    return VARNAM_SUCCESS;
}

static int
learn_word (varnam *handle, const char *word)
{
    int rc;
    const char *sql = "insert or replace into words (id, word, confidence, learned_on) "
                      "select (select id from words where word = trim(?1)), trim(?1), coalesce((select confidence + 1 from words where word = trim(?1)), 1), date();;";

    assert (v_->known_words);

    if (v_->learn_word == NULL)
    {
        rc = sqlite3_prepare_v2( v_->known_words, sql, -1, &v_->learn_word, NULL );
        if (rc != SQLITE_OK) {
            set_last_error (handle, "Failed to learn word : %s", sqlite3_errmsg(v_->known_words));
            sqlite3_reset (v_->learn_word);
            return VARNAM_ERROR;
        }
    }

    sqlite3_bind_text (v_->learn_word, 1, word, -1, NULL);

    rc = sqlite3_step (v_->learn_word);
    if (rc != SQLITE_DONE) {
        set_last_error (handle, "Failed to learn word : %s", sqlite3_errmsg(v_->known_words));
        sqlite3_reset (v_->learn_word);
        return VARNAM_ERROR;
    }

    sqlite3_reset (v_->learn_word);
    varnam_log (handle, "Learned word %s", word);

    return VARNAM_SUCCESS;
}

static int
get_word_id (varnam *handle, const char *word, sqlite3_int64 *word_id)
{
    int rc;

    assert (v_->known_words);

    if (v_->get_word == NULL)
    {
        rc = sqlite3_prepare_v2( v_->known_words, "select id, word, confidence, learned_on from words where word = ?1 limit 1", -1, &v_->get_word, NULL );
        if (rc != SQLITE_OK) {
            set_last_error (handle, "Failed to get word : %s", sqlite3_errmsg(v_->known_words));
            sqlite3_reset (v_->get_word);
            return VARNAM_ERROR;
        }
    }

    *word_id = -1;
    sqlite3_bind_text (v_->get_word, 1, word, -1, NULL);

    rc = sqlite3_step (v_->get_word);
    if (rc == SQLITE_ROW) {
        *word_id = sqlite3_column_int64 (v_->get_word, 0);
    }
    else if (rc != SQLITE_DONE) {
        set_last_error (handle, "Failed to get word : %s", sqlite3_errmsg(v_->known_words));
        sqlite3_reset (v_->get_word);
        return VARNAM_ERROR;
    }

    sqlite3_reset (v_->get_word);
    return VARNAM_SUCCESS;
}

int
vwt_persist_possibilities(varnam *handle, varray *tokens, const char *word)
{
    int i, j, rc;
    varray *array;
    vtoken *token;
    sqlite3_int64 word_id;
    strbuf *pattern = get_pooled_string (handle);

    rc = execute_sql (handle, v_->known_words, "begin;");
    if (rc) return rc;

    rc = learn_word (handle, word);
    if (rc) return rc;

    rc = get_word_id (handle, word, &word_id);
    if (rc) return rc;

    for (i = 0; i < varray_length (tokens); i++)
    {
        array = varray_get (tokens, i);
        strbuf_clear (pattern);
        for (j = 0; j < varray_length (array); j++)
        {
            token = varray_get (array, j);
            strbuf_add (pattern, token->pattern);
        }

        rc = learn_pattern (handle, strbuf_to_s (pattern), word_id);
        if (rc) {
            execute_sql (handle, v_->known_words, "rollback;");
            return rc;
        }
    }

    rc = execute_sql (handle, v_->known_words, "commit;");
    if (rc) return rc;

    return VARNAM_SUCCESS;
}
