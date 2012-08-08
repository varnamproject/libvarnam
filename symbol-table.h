/* varnam-symbol-table.h
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


#ifndef VARNAM_SYMBOL_TABLE_H_INCLUDED_120215
#define VARNAM_SYMBOL_TABLE_H_INCLUDED_120215

#include "vtypes.h"
#include "varray.h"

/**
 * checks the schema availability. this function will create it when necessary
 **/
int
ensure_schema_exists(varnam *handle, char **msg);

/**
 * Starts buffering
 **/
int
vst_start_buffering(varnam *handle);

/**
 * Persist the token
 **/
int
vst_persist_token(
    varnam *handle,
    const char *pattern,
    const char *value1,
    const char *value2,
    const char *value3,
    const char *tag,
    int token_type,
    int match_type);

/**
 * Flushes changes to disk
 **/
int
vst_flush_changes(
    varnam *handle);

/**
 * Rollback changes
 **/
int
vst_discard_changes(varnam *handle);

/**
 * Reads VARNAM_TOKEN_VIRAMA and assigns that to output
 **/
int
vst_get_virama(varnam* handle, struct token **output);

/**
 * Loops through all the dead consonants and make vowel combinations for all of them
 **/
int
vst_generate_cv_combinations(varnam* handle);

int
vst_get_all_tokens (varnam* handle, int token_type, varray *tokens);

/**
 * Adds supplied metadata
 **/
int
vst_add_metadata (varnam *handle, const char* key, const char* value);

int
vst_get_metadata (varnam *handle, const char* key, struct strbuf *output);

/* Tokenizes the input and add the tokens into result. Result will point to a multidimensional array
 * where each element will be an array of vtoken* */
int
vst_tokenize (varnam *handle, const char *input, int tokenize_using, int match_type, varray *result);

#endif
