/* varnam-words-table.h
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


#ifndef VARNAM_WORDS_TABLE_H_INCLUDED
#define VARNAM_WORDS_TABLE_H_INCLUDED

#include "vtypes.h"
#include "varray.h"

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
vwt_persist_pattern(varnam *handle, const char *pattern, sqlite3_int64 word_id);

int
vwt_mark_as_learned (varnam *handle, sqlite3_int64 word_id);

#endif
