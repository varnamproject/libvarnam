/* varnam-words-table.h
 *
 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */



#ifndef VARNAM_WORDS_TABLE_H_INCLUDED
#define VARNAM_WORDS_TABLE_H_INCLUDED

#include "vtypes.h"
#include "varray.h"

#define MAXIMUM_PATTERNS_TO_LEARN 32

int
vwt_ensure_schema_exists(varnam *handle);

int
vwt_persist_possibilities(varnam *handle, varray *tokens, const char *word, int confidence);

int
vwt_start_changes(varnam *handle);

int
vwt_end_changes(varnam *handle);

int
vwt_discard_changes(varnam *handle);

int
vwt_get_words_count(varnam *handle, bool onlyLearned, int *wordCount);

int
vwt_optimize_for_huge_transaction(varnam *handle);

int
vwt_turn_off_optimization_for_huge_transaction(varnam *handle);

int
vwt_get_best_match (varnam *handle, const char *input, varray *words);

/**
 * Gets the suggestions for input and store it in words array
 **/
int
vwt_get_suggestions (varnam *handle, const char *input, varray *words);

/**
 * Tokenizes the pattern based on words table. Result will be multidimensional
 * array where each sub array containing vtoken instances. */
int
vwt_tokenize_pattern (varnam *handle, const char *pattern, varray *result);

int
vwt_compact_file (varnam *handle);

int
vwt_get_word_id (varnam *handle, const char *word, sqlite3_int64 *word_id);

int
vwt_try_insert_new_word (varnam* handle, const char* word, int confidence, sqlite3_int64* new_word_id);

int
vwt_persist_pattern(varnam *handle, const char *pattern, sqlite3_int64 word_id, bool is_prefix);

int
vwt_delete_word(varnam *handle, const char *word);

int
vwt_export_words(varnam* handle, int words_per_file, const char* out_dir,
    void (*callback)(int total_words, int processed, const char *current_word));

int
vwt_full_export(varnam* handle, int words_per_file, const char* out_dir,
    void (*callback)(int, int, const char *));

int
vwt_import_words (varnam* handle, const char* filepath);

#endif
