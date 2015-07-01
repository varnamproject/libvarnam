/*
 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */

#include <assert.h>
#include <string.h>

#include "api.h"
#include "vtypes.h"
#include "varray.h"
#include "vword.h"
#include "util.h"
#include "result-codes.h"
#include "symbol-table.h"
#include "words-table.h"
#include "deps/parson.h"

static bool
is_words_store_available(varnam* handle)
{
    assert (handle);

    if (v_->known_words == NULL) {
        set_last_error (handle, "'words' store is not enabled.");
        return false;
    }

    return true;
}

static void
language_specific_sanitization(strbuf *string)
{
    /* Malayalam has got two ways to write chil letters. Converting the old style to new one */
    strbuf_replace (string, "ന്‍", "ൻ");
    strbuf_replace (string, "ണ്‍", "ൺ");
    strbuf_replace (string, "ല്‍","ൽ");
    strbuf_replace (string, "ള്‍", "ൾ");
    strbuf_replace (string, "ര്‍", "ർ");

    /* Hindi's DANDA (Purna viram) */
    strbuf_replace (string, "।", "");
}

static strbuf*
sanitize_word (varnam *handle, const char *word)
{
    int i;
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

    for (i = (int)(string->length - 1); i >= 0; i--)
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
    strbuf_remove_from_first (string, ZWNJ());
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
get_total_possible_patterns(varray *tokens)
{
    int total = 1, i = 0;
    varray *item;

    for (i = 0; i < varray_length (tokens); i++)
    {
        item = varray_get (tokens, i);
        total *= varray_length (item);
    }

    return total;
}

static varray*
get_largest_array(varray *tokens)
{
    int i;
    varray *item = NULL, *largest = NULL;

    for (i = 0; i < varray_length (tokens); i++)
    {
        item = varray_get (tokens, i);
        if (largest == NULL || varray_length (item) > varray_length (largest)) {
            largest = item;
        }
    }

    return largest;
}

static void
apply_acceptance_condition(varray *tokens)
{
    int i, j, to_remove[100], total_to_remove = 0, state, empty_arrays[100], empty_arrays_index = 0;
    varray *item;
    vtoken *t;

    for (i = 0; i < varray_length (tokens); i++)
    {
        if (i == 0)
            state = VARNAM_TOKEN_ACCEPT_IF_STARTS_WITH;
        else if ((i + 1) == varray_length (tokens))
            state = VARNAM_TOKEN_ACCEPT_IF_ENDS_WITH;
        else
            state = VARNAM_TOKEN_ACCEPT_IF_IN_BETWEEN;

        item = varray_get (tokens, i);
        for (j = 0; j < varray_length (item); j++)
        {
            t = varray_get (item, j);
            switch (t->type)
            {
                case VARNAM_TOKEN_VIRAMA:
                case VARNAM_TOKEN_VISARGA:
                case VARNAM_TOKEN_ANUSVARA:
                case VARNAM_TOKEN_NON_JOINER:
                case VARNAM_TOKEN_NUMBER:
                    to_remove[total_to_remove++] = j;
                    break;
                default:
                    if (varray_length (item) != 1 && t->accept_condition != VARNAM_TOKEN_ACCEPT_ALL && t->accept_condition != state) {
                        to_remove[total_to_remove++] = j;
                    }
                    break;
            }
        }

        for (j = 0; j < total_to_remove; j++)
        {
            /* to_remove[j] - j is required to calculate the new index as deleting each item changes index */
            varray_remove_at (item, to_remove[j] - j);
        }

        if (varray_length (item) == 0) {
            /* This happens when all the items in this list is VIRAMA, VISARGA etc */
            empty_arrays[empty_arrays_index++] = i;
        }

        total_to_remove = 0;
    }

    for (i = 0; i < empty_arrays_index; i++)
    {
        varray_remove_at (tokens, empty_arrays[i] - i);
    }
}

static bool
remove_low_priority_tokens(varray *tokens)
{
    int i, j, to_remove[100], total_to_remove = 0;
    varray *item;
    vtoken *t;

    for (i = 0; i < varray_length (tokens); i++)
    {
        item = varray_get (tokens, i);
        if (varray_length (item) == 1)
            continue;

        for (j = 0; j < varray_length (item); j++)
        {
            t = varray_get (item, j);
            if (t->priority <= VARNAM_TOKEN_PRIORITY_LOW) {
                to_remove[total_to_remove++] = j;
            }
        }

        for (j = 0; j < total_to_remove; j++)
        {
            varray_remove_at (item, to_remove[j] - j);
        }
        total_to_remove = 0;

        if (get_total_possible_patterns (tokens) <= MAXIMUM_PATTERNS_TO_LEARN) {
            return true;
        }
    }

    return false;
}

static varray*
get_next_array(varray *tokens)
{
    varray *item, *candidate = NULL;
    int i;
    bool same_priority = true;
    vtoken *last, *lowest = NULL;

    for (i = 0; i < varray_length (tokens); i++)
    {
        item = varray_get (tokens, i);

        if (varray_length (item) == 1)
            continue;

        last = varray_get (item, varray_length (item) - 1);
        if (lowest == NULL || last->priority < lowest->priority) {
            lowest = last;
            candidate = item;
        }

        if (last->priority != lowest->priority) {
            same_priority = false;
        }
    }

    if (same_priority)
    {
        candidate = get_largest_array (tokens);
        if (varray_length (candidate) == 1)
            return NULL;
    }

    return candidate;
}

/*
 * This function iterates over tokens and remove unimportant tokens
 * from the array so that only important tokens remain in tokens.
 * This improves learn quality
 */
static void
reduce_noise_in_tokens(varray *tokens)
{
    varray *next = NULL;

    if (varray_is_empty (tokens)) {
        return;
    }

    /* Removing all unacceptable tokens */
    apply_acceptance_condition (tokens);

    if (get_total_possible_patterns (tokens) <= MAXIMUM_PATTERNS_TO_LEARN) {
        return;
    }

    /* Removing low priority tokens first */
    if (remove_low_priority_tokens (tokens)) {
        return;
    }

    while (get_total_possible_patterns (tokens) > MAXIMUM_PATTERNS_TO_LEARN)
    {
        next = get_next_array (tokens);
        if (next == NULL) {
            return;
        }
        varray_pop_last_item (next);
    }
}

int
stem(varnam *handle, const char *word, varray *stem_results)
{
    int rc;
    strbuf *word_copy, *suffix, *new_ending, *temp;
    char *end_char;

    rc = vst_has_stemrules(handle);
    if(rc == VARNAM_STEMRULE_MISS)
        return VARNAM_SUCCESS;

    else if(rc == VARNAM_ERROR)
        return VARNAM_ERROR;
    
    word_copy = get_pooled_string(handle);
    suffix = get_pooled_string(handle);
    temp = get_pooled_string(handle);
    new_ending = get_pooled_string(handle);
    strbuf_add(word_copy, word);

    while(word_copy->length > 0)
    {
        /*the next character of word_buffer should go 
         to the beginning of the end_bufer. For this 
         we copy end_buffer to temp, clear end_buffer,
         add new ending to end_buffer and append the 
         contents of temp back to end_buffer*/
        strbuf_clear(temp);
        strbuf_add(temp, strbuf_to_s(suffix));
        strbuf_clear(suffix);
        end_char = strbuf_get_last_unicode_char(word_copy);
        strbuf_add(suffix, end_char);
        strbuf_add(suffix, strbuf_to_s(temp));
        strbuf_remove_from_last(word_copy, end_char);

        rc = vst_get_stem(handle, suffix, new_ending);
        if(rc == VARNAM_STEMRULE_HIT)
        {
            rc = vst_check_exception(handle, word_copy, suffix);
            if(rc == VARNAM_STEMRULE_HIT)
            {
                free(end_char);
                continue;
            }

            else if(rc != VARNAM_STEMRULE_MISS && rc != VARNAM_SUCCESS)
                return VARNAM_ERROR;

            strbuf_add(word_copy, strbuf_to_s(new_ending));
            /*Creating a vword using Word()
             word_buffer will change in subsequent iterations of the loop
             So pushing a pointer to word_buffer->buffer to varray is of
             no use. So we create a vword for each word that is to be learned
             and push it to the varray*/
            varray_push(stem_results, Word(handle, (const char*)strbuf_to_s(word_copy), 0));
            strbuf_clear(suffix);
        }
        else if(rc != VARNAM_STEMRULE_MISS)
        {
            free(end_char);
            set_last_error(handle, "stemrule query failed");
            return VARNAM_ERROR;
        }

        free(end_char);
    }

    return VARNAM_SUCCESS;
}

static int
varnam_learn_internal(varnam *handle, const char *word, int confidence)
{
    int rc;
    varray *tokens;
    strbuf *sanitized_word;

    if(strlen(word) == 0)
        return VARNAM_ARGS_ERROR;

    if (handle == NULL || word == NULL)
        return VARNAM_ARGS_ERROR;

    if (!is_words_store_available(handle)) {
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

#ifdef _VARNAM_VERBOSE
    printf ("%s\n", "Tokens before reducing noice");
    print_tokens_array (tokens);
#endif

    /* Tokens may contain more data that we can handle. Reducing noice so that we learn most relevant combinations */
    reduce_noise_in_tokens (tokens);

#ifdef _VARNAM_VERBOSE
    printf ("%s\n", "Tokens after reducing noice");
    print_tokens_array (tokens);
#endif

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
    int rc,i;
    varray *stem_results;
    #ifdef _RECORD_EXEC_TIME
        V_BEGIN_TIMING
    #endif

    reset_pool (handle);

    if (!is_words_store_available (handle)) {
        return VARNAM_ERROR;
    }

    rc = vwt_start_changes (handle);
    if (rc != VARNAM_SUCCESS) return rc;

    rc = varnam_learn_internal(handle, word, 1);
    if (rc != VARNAM_SUCCESS) {
        vwt_discard_changes (handle);
        return rc;
    }


    stem_results = varray_init();
    rc = stem(handle, word, stem_results);
    if(rc != VARNAM_SUCCESS)
        return rc;
    
    for(i=0;i<=stem_results->index;i++)
    {
        varnam_learn_internal(handle, ((vword*)varray_get(stem_results, i))->text, 0);
    }

    varray_free(stem_results, &destroy_word);

    rc = vwt_end_changes (handle);
    if (rc != VARNAM_SUCCESS)
        return rc;

#ifdef _RECORD_EXEC_TIME
    V_REPORT_TIME_TAKEN("varnam_learn")
#endif
    return VARNAM_SUCCESS;
}

int
varnam_delete_word(varnam *handle, const char *word)
{
    if (handle == NULL || word == NULL) {
        return VARNAM_ARGS_ERROR;
    }

    reset_pool (handle);

    return vwt_delete_word (handle, word);
}

int
varnam_learn_from_file(varnam *handle,
                       const char *filepath,
                       vlearn_status *status,
                       void (*callback)(varnam *handle, const char *word, int status_code, void *object),
                       void *object)
{
    int rc;
    int rc2;            
    FILE *infile;
    char line_buffer[10000];
    strbuf *word;
    varray *word_parts;
    varray *stem_results;
    int confidence;
    int parts;
    int i;

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

    /* Learning from file will be mostly new words. Optimizing for that */
    v_->_config_mostly_learning_new_words = 1;

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

            /*hack. if rc is used here then varnam won't create text file upon failure. To fix*/
            
            stem_results = varray_init();
            rc2 = stem(handle, strbuf_to_s(word), stem_results);
            if(rc2 != VARNAM_SUCCESS)
                return rc;
            
            for(i=0;i<=stem_results->index;i++)
            {
                varnam_learn_internal(handle, ((vword*)varray_get(stem_results, i))->text, 0);
            }

            varray_free(stem_results, &destroy_word);
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


    fclose (infile);
    return rc;
}


int
varnam_compact_learnings_file(varnam *handle)
{
  varnam_log (handle, "Compacting file");
  return vwt_compact_file (handle);
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

    rc = vwt_persist_pattern (handle, pattern, word_id, false);
    if (rc)
        return rc;

    vwt_end_changes (handle);
    return VARNAM_SUCCESS;
}

int
varnam_export_words(varnam* handle, int words_per_file, const char* out_dir, int export_type,
    void (*callback)(int total_words, int processed, const char *current_word))
{
    if (handle == NULL || out_dir == NULL || words_per_file <= 0) {
        return VARNAM_ARGS_ERROR;
    }

    reset_pool (handle);

    if (export_type == VARNAM_EXPORT_FULL)
        return vwt_full_export (handle, words_per_file, out_dir, callback);
    else
        return vwt_export_words (handle, words_per_file, out_dir, callback);
}

static int
import_words (varnam* handle, const char* filepath)
{
    int rc;
		size_t i, j;
		bool isPrefix;
		sqlite3_int64 wordId;
		JSON_Value *root = NULL;
		JSON_Array *words = NULL, *patterns = NULL;
		JSON_Object *word = NULL, *pattern = NULL;
		const char* wordString = NULL;
		strbuf *sanitized_word;
		varray* tokens;

		root = json_parse_file_with_comments (filepath);
		if (root == NULL) {
      set_last_error (handle, "Couldn't open file '%s' for reading", filepath);
      return VARNAM_ERROR;
		}

		if (json_value_get_type(root) != JSONArray) {
      set_last_error (handle, "'%s': Unknown file format", filepath);
      return VARNAM_ERROR;
		}

		rc = VARNAM_SUCCESS;

		words = json_value_get_array (root);
		tokens = get_pooled_array (handle);
		for (i = 0; i < json_array_get_count(words); i++) {
			word = json_array_get_object (words, i);
			wordString = json_object_get_string(word, "word");

			/* Making sure word contains only allowed token for the current scheme */
			sanitized_word = sanitize_word (handle, wordString);
			rc = vst_tokenize (handle, strbuf_to_s (sanitized_word), VARNAM_TOKENIZER_VALUE, VARNAM_MATCH_ALL, tokens);
			if (rc) {
				goto cleanup;
			}
			if (!can_learn_from_tokens (handle, tokens, strbuf_to_s (sanitized_word))) {
				continue;
			}

			rc = vwt_try_insert_new_word (handle, wordString, (int)json_object_get_number (word, "confidence"), &wordId);
			if (rc != VARNAM_SUCCESS) {
				goto cleanup;
			}

			patterns = json_object_get_array (word, "patterns");
			if (patterns != NULL) {
				for (j = 0; j < json_array_get_count(patterns); j++) {
					pattern = json_array_get_object (patterns, j);
					isPrefix = json_object_get_number (pattern, "learned") == 0 ? true : false;
					rc = vwt_persist_pattern (handle, json_object_get_string (pattern, "pattern"), wordId, isPrefix);
					if (rc != VARNAM_SUCCESS) {
						goto cleanup;
					}
				}
			}
		}

cleanup:
		if (root != NULL)
			json_value_free (root);

    return rc;
}



int
varnam_import_learnings_from_file(varnam *handle, const char *filepath)
{
    int rc;

    if (handle == NULL || filepath == NULL)
        return VARNAM_ARGS_ERROR;

    reset_pool (handle);

    rc = vwt_optimize_for_huge_transaction(handle);
    if (rc) {
        return rc;
    }

    varnam_log (handle, "Starting to import from %s", filepath);
    rc = vwt_start_changes (handle);
    if (rc) {
        vwt_turn_off_optimization_for_huge_transaction(handle);
        return rc;
    }

    rc = import_words (handle, filepath);
    if (rc != VARNAM_SUCCESS) {
      vwt_turn_off_optimization_for_huge_transaction (handle);
      return rc;
    }

    varnam_log (handle, "Writing changes to disk");
    rc = vwt_end_changes (handle);
    if (rc != VARNAM_SUCCESS) {
        varnam_log (handle, "Writing changes to disk failed");
        return rc;
    }

    varnam_log (handle, "Ensuring file integrity");
    rc = vwt_turn_off_optimization_for_huge_transaction(handle);
    if (rc) {
        varnam_log (handle, "Failed to check file integrity");
        return rc;
    }

    return VARNAM_SUCCESS;
}

int
varnam_is_known_word(varnam* handle, const char* word)
{
    int rc;
    sqlite3_int64 word_id;

    if (handle == NULL || word == NULL)
        return 0;

    if (!is_words_store_available (handle)) {
        return 0;
    }

    reset_pool (handle);

    rc = vwt_get_word_id (handle, word, &word_id);
    if (rc != VARNAM_SUCCESS) {
        return 0;
    }

    if (word_id > 0)
        return 1;
    else
        return 0;
}

