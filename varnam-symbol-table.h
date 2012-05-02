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

#include "varnam-types.h"

/**
* This function will try to get a token for the lookup text provided. 
* search will be done directly on the symbol table.
* A valid instance of token will returned upon success. 
* NULL value indicates a failure to get the token
**/
struct token*
find_token(varnam *handle, const char *lookup);

/**
* This function will try to get a token for the lookup text provided. 
* search will be done directly on the symbol table on value1 and value2 fields.
* A valid instance of token will returned upon success. 
* NULL value indicates a failure to get the token
**/
struct token*
find_rtl_token(varnam *handle, const char *lookup);

/**
* Does a search in the symbol table and retrns a boolean value indicating
* the possibility of finding a token for the lookup text.
**/
int 
can_find_token(varnam *handle, struct token *last, const char *lookup);

/**
* Does a search in the symbol table and retrns a boolean value indicating
* the possibility of finding a rtl token for the lookup text.
**/
int 
can_find_rtl_token(varnam *handle, struct token *last, const char *lookup);

/**
 * fetches the value for the supplied name and writes that to the output
 **/
void 
fill_general_values(varnam *handle, char *output, const char *name);

/**
 * checks the schema availability. this function will create it when necessary
 **/
int
ensure_schema_exist(varnam *handle, char **msg);

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
vst_get_virama(varnam* handle, char* output);

/**
 * Loops through all the dead consonants and make vowel combinations for all of them
 **/
int
vst_generate_cv_combinations(varnam* handle);

int
vst_get_all_tokens (varnam* handle, int token_type, struct token **tokens);

#endif
