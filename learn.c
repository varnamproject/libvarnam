/* Functions that implements the learning module

   Copyright (C) 2010 Navaneeth.K.N

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "api.h"
#include "vtypes.h"
#include "varray.h"
#include "util.h"
#include "result-codes.h"
#include "symbol-table.h"
#include "words-table.h"

static void
language_specific_sanitization(strbuf *string)
{
    /* Malayalam has got two ways to write chil letters. Converting the old style to new one */
    strbuf_replace (string, "ന്‍", "ൻ");
    strbuf_replace (string, "ണ്‍", "ൺ");
    strbuf_replace (string, "ല്‍","ൽ");
    strbuf_replace (string, "ള്‍", "ൾ");
    strbuf_replace (string, "ര്‍", "ർ");
}

static strbuf*
sanitize_word (varnam *handle, const char *word)
{
    size_t i;
    bool is_special = false;
    strbuf *string, *to_remove;

    string = get_pooled_string (handle);
    to_remove = get_pooled_string (handle);

    strbuf_add (string, word);

    for (i = 0; i < string->length; i++)
    {
        is_special = is_special_character (string->buffer[i]);
        if (is_special)
            strbuf_addc (to_remove, string->buffer[i]);
        else
            break;
    }

    strbuf_remove_from_first (string, strbuf_to_s (to_remove));

    strbuf_clear (to_remove);
    for (i = string->length - 1; i >= 0; i--)
    {
        is_special = is_special_character (string->buffer[i]);
        if (is_special)
            strbuf_addc (to_remove, string->buffer[i]);
        else
            break;
    }

    strbuf_remove_from_last (string, strbuf_to_s (to_remove));
    language_specific_sanitization (string);

    /* Remove trailing ZWNJ and leading ZWJ */
    strbuf_remove_from_last (string, ZWNJ());
    strbuf_remove_from_first (string, ZWJ());

    return string;
}

static bool
can_learn_from_tokens (varnam *handle, varray *tokens, const char *word)
{
    bool all_vowels = true, unknown_tokens = false;
    int i, j, repeating_tokens = 0, last_token_id = 0;
    vtoken *t, *unknown_token;
    varray *array;

    if (varray_length (tokens) < 2) {
        set_last_error (handle, "Nothing to learn from '%s'", word);
        return false;
    }

    for (i = 0; i < varray_length (tokens); i++)
    {
        array = varray_get (tokens, i);
        for (j = 0; j < varray_length (array); j++)
        {
            t = varray_get (array, j);
            if (t->match_type == VARNAM_MATCH_POSSIBILITY) continue;

            if (t->type != VARNAM_TOKEN_VOWEL) all_vowels = false;

            if (t->type == VARNAM_TOKEN_OTHER) {
                unknown_tokens = true;
                unknown_token = t;
                goto done;
            }

            if (last_token_id == t->id) {
                ++repeating_tokens;
            }
            else {
                repeating_tokens = 0;
                last_token_id = t->id;
            }
        }
    }

done:
    if (all_vowels) {
        set_last_error (handle, "Word contains only vowels. Nothing to learn from '%s'", word);
        return false;
    }
    else if (unknown_tokens) {
        set_last_error (handle, "Can't process '%s'. One or more characters in '%s' are not known", unknown_token->pattern, word);
        return false;
    }
    else if (repeating_tokens >= 3) {
        set_last_error (handle, "'%s' looks incorrect. Not learning anything", word);
        return false;
    }

    return true;
}

static int
varnam_learn_internal(varnam *handle, const char *word, int confidence)
{
    int rc;
    varray *tokens;
    strbuf *sanitized_word;

    if (handle == NULL || word == NULL)
        return VARNAM_ARGS_ERROR;

    if (v_->known_words == NULL) {
        set_last_error (handle, "'words' store is not enabled.");
        return VARNAM_ERROR;
    }

    if (!is_utf8 (word)) {
        set_last_error (handle, "Incorrect encoding. Expected UTF-8 string");
        return VARNAM_ERROR;
    }

    tokens = get_pooled_array (handle);

    /* This removes all starting and trailing special characters from the word */
    sanitized_word = sanitize_word (handle, word);

    rc = vst_tokenize (handle, strbuf_to_s (sanitized_word), VARNAM_TOKENIZER_VALUE, VARNAM_MATCH_ALL, tokens);
    if (rc) return rc;

    if (!can_learn_from_tokens (handle, tokens, strbuf_to_s (sanitized_word)))
        return VARNAM_ERROR;

    return vwt_persist_possibilities (handle,
                                      tokens,
                                      strbuf_to_s (sanitized_word),
                                      confidence);
}

int
varnam_learn(varnam *handle, const char *word)
{
    int rc;
#ifdef _RECORD_EXEC_TIME
    V_BEGIN_TIMING
#endif

    reset_pool (handle);

    rc = vwt_start_changes (handle);
    if (rc != VARNAM_SUCCESS) return rc;

    rc = varnam_learn_internal(handle, word, 1);
    if (rc != VARNAM_SUCCESS) {
        vwt_discard_changes (handle);
        return rc;
    }

    vwt_end_changes (handle);

#ifdef _RECORD_EXEC_TIME
    V_REPORT_TIME_TAKEN("varnam_learn")
#endif
    return VARNAM_SUCCESS;
}

int
varnam_learn_from_file(varnam *handle,
                       const char *filepath,
                       vlearn_status *status,
                       void (*callback)(varnam *handle, const char *word, int status_code, void *object),
                       void *object)
{
    int rc;
    FILE *infile;
    char line_buffer[10000];
    strbuf *word;
    varray *word_parts;
    int confidence;
    int parts;

    infile = fopen(filepath, "r");
    if (!infile) {
        set_last_error (handle, "Couldn't open file '%s' for reading.\n", filepath);
        return VARNAM_ERROR;
    }

    if (status != NULL)
    {
        status->total_words = 0;
        status->failed = 0;
    }

    rc = vwt_optimize_for_huge_transaction(handle);
    if (rc) {
        fclose (infile);
        return rc;
    }

    varnam_log (handle, "Starting to learn from %s", filepath);
    rc = vwt_start_changes (handle);
    if (rc) {
        vwt_turn_off_optimization_for_huge_transaction(handle);
        fclose (infile);
        return rc;
    }

    while (fgets(line_buffer, sizeof(line_buffer), infile))
    {
        reset_pool (handle);

        word = get_pooled_string (handle);
        strbuf_add (word, trimwhitespace (line_buffer));
        word_parts = strbuf_split (word, handle, ' ');
        parts = varray_length (word_parts);
        if (parts > 0 && parts <= 2)
        {
            confidence = 1;
            if (parts == 2) {
                word = varray_get (word_parts, 1);
                confidence = atoi (strbuf_to_s (word));
            }

            word = varray_get (word_parts, 0);
            rc = varnam_learn_internal (handle, strbuf_to_s (word), confidence);
            if (rc) {
                if (status != NULL) status->failed++;
            }
        }
        else {
            rc = VARNAM_ERROR;
            if (status != NULL) status->failed++;
        }

        if (status   != NULL) status->total_words++;
        if (callback != NULL) callback (handle, strbuf_to_s (word), rc, object);
    }

    varnam_log (handle, "Writing changes to disk");
    rc = vwt_end_changes (handle);
    if (rc) {
        varnam_log (handle, "Writing changes to disk failed");
    }

    varnam_log (handle, "Ensuring file integrity");
    rc = vwt_turn_off_optimization_for_huge_transaction(handle);
    if (rc) {
        varnam_log (handle, "Failed to check file integrity");
    }

    varnam_log (handle, "Compacting file");
    rc = vwt_compact_file (handle);
    if (rc) return rc;

    fclose (infile);
    return rc;
}

int
varnam_train(varnam *handle, const char *pattern, const char *word)
{
    int rc;
    sqlite3_int64 word_id;

    reset_pool (handle);

    rc = vwt_start_changes (handle);
    if (rc != VARNAM_SUCCESS) return rc;

    rc = varnam_learn_internal(handle, word, 1);
    if (rc != VARNAM_SUCCESS) {
        vwt_discard_changes (handle);
        return rc;
    }

    rc = vwt_get_word_id (handle, word, &word_id);
    if (rc) return rc;

    rc = vwt_persist_pattern (handle, pattern, word_id, true);
    if (rc)
        return rc;

    vwt_end_changes (handle);
    return VARNAM_SUCCESS;
}
