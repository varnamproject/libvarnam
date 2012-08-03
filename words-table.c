/* Functions to handle the words store
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
#include <string.h>

#include "symbol-table.h"
#include "util.h"
#include "vtypes.h"
#include "result-codes.h"
#include "api.h"
#include "token.h"
#include "rendering.h"
#include "varray.h"
#include "vword.h"

#define MAXIMUM_PATTERNS_TO_LEARN 20
#define MINIMUM_CHARACTER_LENGTH_FOR_SUGGESTION 3

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
        "create virtual table if not exists patterns using fts4(content='patterns_content', pattern text, word_id integer, prefix=\"3,4,5,6,7,8,9,10\");";

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
        set_last_error (handle, "Failed to initialize file for storing known words. Pragma setting failed. : %s", zErrMsg);
        sqlite3_free(zErrMsg);
        return VARNAM_ERROR;
    }

    rc = sqlite3_exec(v_->known_words, tables, NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
        set_last_error (handle, "Failed to initialize file for storing known words. Schema creation failed. : %s", zErrMsg);
        sqlite3_free(zErrMsg);
        return VARNAM_ERROR;
    }

    rc = sqlite3_exec(v_->known_words, triggers1, NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
        set_last_error (handle, "Failed to initialize file for storing known words. First set of triggers failed. : %s", zErrMsg);
        sqlite3_free(zErrMsg);
        return VARNAM_ERROR;
    }

    rc = sqlite3_exec(v_->known_words, triggers2, NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
        set_last_error (handle, "Failed to initialize file for storing known words. Second set of triggers failed. : %s", zErrMsg);
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

int
vwt_start_changes(varnam *handle)
{
    assert (v_->known_words);
    return execute_sql(handle, v_->known_words, "BEGIN;");
}

int
vwt_end_changes(varnam *handle)
{
    assert (v_->known_words);
    return execute_sql(handle, v_->known_words, "COMMIT;");
}

int
vwt_discard_changes(varnam *handle)
{
    assert (v_->known_words);
    return execute_sql(handle, v_->known_words, "ROLLBACK;");
}

int
vwt_optimize_for_huge_transaction(varnam *handle)
{
    const char *sql =
        "pragma journal_mode=delete;";

    assert (handle);
    assert (v_->known_words);

    return execute_sql (handle, v_->known_words, sql);
}

int
vwt_turn_off_optimization_for_huge_transaction(varnam *handle)
{
    const char *sql =
        "pragma journal_mode=wal;";

    assert (handle);
    assert (v_->known_words);

    return execute_sql (handle, v_->known_words, sql);
}

static int
persist_pattern(varnam *handle, const char *pattern, sqlite3_int64 word_id)
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

/* Learns the pattern. strbuf* is passed in because of memory optimizations.
 * See comments in the learn_suffixes() function */
static int
learn_pattern (varnam *handle, varray *tokens, const char *word, strbuf *pattern)
{
    int rc, i;
    sqlite3_int64 word_id;
    vtoken *token;

    rc = get_word_id (handle, word, &word_id);
    if (rc) return rc;

    strbuf_clear (pattern);
    for (i = 0; i < varray_length (tokens); i++)
    {
        token = varray_get (tokens, i);
        strbuf_add (pattern, token->pattern);
    }

    rc = persist_pattern (handle, strbuf_to_s (pattern), word_id);
    if (rc)
        return rc;

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

/* Learns all the suffixes. This won't learn single tokens and the word itself
 * tokens_tmp - Is passed is for memory usage optimization. This function gets
 * called inside a cartesion product finder which means there will be a lot of
 * instances of array required. To optimize this, we pass in this array which
 * will be allocated from cartesian product finder */
static int
learn_suffixes(varnam *handle, varray *tokens, strbuf *pattern, bool word_already_learned)
{
    int i, rc, tokens_len = 0;
    vword *word;
    vtoken *token;

    varray *tokens_tmp = get_pooled_array (handle);
    for (i = 0; i < varray_length (tokens); i++)
    {
        token = varray_get (tokens, i);
        assert (token != NULL);

        varray_push (tokens_tmp, token);

        tokens_len = varray_length (tokens_tmp);
        /* We don't learn if it is only one token.
         * We don't learn the full word here because it would have already learned before this method is called */
        if (tokens_len > 1 && tokens_len != varray_length(tokens))
        {
            rc = resolve_tokens (handle, tokens_tmp, &word);
            if (rc) {
                return_array_to_pool (handle, tokens_tmp);
                return rc;
            }

            if (!word_already_learned)
            {
                rc = learn_word (handle, word->text);
                if (rc) {
                    return_array_to_pool (handle, tokens_tmp);
                    return rc;
                }
            }

            rc = learn_pattern (handle, tokens_tmp, word->text, pattern);
            if (rc) {
                return_array_to_pool (handle, tokens_tmp);
                return rc;
            }
        }
    }

    return_array_to_pool (handle, tokens_tmp);
    return VARNAM_SUCCESS;
}

/* This function learns all possibilities of writing the word and it's suffixes.
 * It finds cartesian product of the tokens passed in and process each product.
 * tokens will be a multidimensional array */
static int
learn_all_possibilities(varnam *handle, varray *tokens, const char *word)
{
    int rc, array_cnt, *offsets, i, last_array_offset, total = 0;
    varray *array, *tmp;
    strbuf *pattern;
    bool word_already_learned = false;

    array_cnt = varray_length (tokens);
    offsets = xmalloc(sizeof(int) * (size_t) array_cnt);

    for (i = 0; i < array_cnt; i++) offsets[i] = 0;

    array = get_pooled_array (handle);
    pattern = get_pooled_string (handle);

    for (;;)
    {
        varray_clear (array);
        for (i = 0; i < array_cnt; i++)
        {
            tmp = varray_get (tokens, i);
            assert (tmp);
            varray_push (array, varray_get (tmp, offsets[i]));
        }

        rc = learn_pattern (handle, array, word, pattern);
        if (rc)
            goto finished;

        rc = learn_suffixes (handle, array, pattern, word_already_learned);
        if (rc)
            goto finished;

        word_already_learned = true;
        if (++total == MAXIMUM_PATTERNS_TO_LEARN) {
            goto finished;
        }

        last_array_offset = array_cnt - 1;
        offsets[last_array_offset]++;

        while (offsets[last_array_offset] == varray_length ((varray*) varray_get (tokens, last_array_offset)))
        {
            offsets[last_array_offset] = 0;

            if (--last_array_offset < 0) goto finished;

            offsets[last_array_offset]++;
        }
    }

finished:
    xfree (offsets);
    return rc;
}

int
vwt_persist_possibilities(varnam *handle, varray *tokens, const char *word)
{
    int rc;

    rc = learn_word (handle, word);
    if (rc) return rc;

    rc = learn_all_possibilities (handle, tokens, word);
    if (rc) return rc;

    return VARNAM_SUCCESS;
}

int
vwt_get_suggestions (varnam *handle, const char *input, varray *words)
{
    int rc;
    vword *word;
    const char *sql = "select word, confidence from words where rowid in (SELECT distinct(word_id) FROM patterns_content as pc where pc.pattern >= ?1 and pc.pattern <= ?1 || 'z' limit 10) order by confidence desc";

    assert (handle);
    assert (words);

    if (v_->known_words == NULL)
        return VARNAM_SUCCESS;

    if (strlen(input) < MINIMUM_CHARACTER_LENGTH_FOR_SUGGESTION)
        return VARNAM_SUCCESS;

    if (v_->get_suggestions == NULL)
    {
        rc = sqlite3_prepare_v2( v_->known_words, sql, -1, &v_->get_suggestions, NULL );
        if (rc != SQLITE_OK) {
            set_last_error (handle, "Failed to get suggestions : %s", sqlite3_errmsg(v_->known_words));
            sqlite3_reset (v_->get_suggestions);
            return VARNAM_ERROR;
        }
    }

    sqlite3_bind_text (v_->get_suggestions, 1, input, -1, NULL);

    for (;;)
    {
        rc = sqlite3_step (v_->get_suggestions);
        if (rc == SQLITE_ROW)
        {
            word = get_pooled_word (handle,
                                    (const char*) sqlite3_column_text(v_->get_suggestions, 0),
                                    (int) sqlite3_column_int(v_->get_suggestions, 1));
            varray_push (words, word);
        }
        else if (rc == SQLITE_DONE)
        {
            break;
        }
        else
        {
            set_last_error (handle, "Failed to get suggestions : %s", sqlite3_errmsg(v_->known_words));
            sqlite3_reset (v_->get_suggestions);
            return VARNAM_ERROR;
        }
    }

    sqlite3_clear_bindings (v_->get_suggestions);
    sqlite3_reset (v_->get_suggestions);
    return VARNAM_SUCCESS;
}

/**
 * Tokenize supplied pattern based on words available.
 *
 * This tokenization is done against patterns table. Like symbols tokenization,
 * this also does the tokenization based on longest prefix match. But this method does
 * it in a different way. Patterns is a prefix array of all available patterns.
 *
 * This method tries to find a match until it gets the first match. It will accept this match
 * if the next match attempt fails. This is safe since patterns table is prefix array and when a
 * prefix lookup fails, there won't be anymore matches.
 *
 * This function works in the above way because patterns table doesn't keep all the prefixes. It discards
 * small prefixes to save space. So this alogorithm doesn't know about the smallest prefix available
 *
 * This will populate vtoken instances into result array. 
 **/
/* int */
/* vwt_tokenize_pattern (varnam *handle, const char *pattern, varray *result) */
/* { */
/*     int rc, matchpos = 0, pos = 0; */
/*     struct strbuf *lookup, *match; */
/*     vtoken *token; */
/*     bool found = false; */
/*     const char *pc;     */
    
/*     if (pattern == NULL || *pattern == '\0') */
/*         return VARNAM_SUCCESS; */

/*     varray_clear (result); */
/*     pc = pattern; */
/*     lookup = get_pooled_string (handle); */
/*     match = get_pooled_string (handle); */

/*     while (*pc != '\0') */
/*     { */
/*         ++pos; */
/*         strbuf_add_c (lookup, *pc++); */
/*         rc = get_match (handle, lookup, match, &found); */
/*         if (rc != VARNAM_SUCCESS) */
/*             return rc; */

/*         if (found) */
/*         { */
/*             matchpos = pos; */
/*             continue; */
/*         } */


/*         if (rc) return rc; */

/*         if (tokens_available) */
/*             matchpos = bytes_read; */

/*         if (varray_is_empty (tokens)) */
/*         { */
/*             /\* We couldn't find any tokens. So adding lookup as the match *\/ */
/*             token = get_pooled_token (handle, -99, */
/*                                       VARNAM_TOKEN_OTHER, */
/*                                       VARNAM_MATCH_EXACT, */
/*                                       strbuf_to_s (lookup), "", "", ""); */
/*             assert (token); */
/*             varray_push (tokens, token); */
/*             matchpos = (int) lookup->length; */
/*         } */
/*         rc = can_find_more_matches (handle, lookup, tokenize_using, &possibility); */
/*         if (rc) return rc; */
/*         if (possibility && *inputcopy != '\0') continue; */

/*         varray_push (result, tokens); */
/*         bytes_read = 0; */
/*         tokens = NULL; */

/*         input = input + matchpos; */
/*         inputcopy = input; */
/*     } */

/*     return VARNAM_SUCCESS; */
/* } */
