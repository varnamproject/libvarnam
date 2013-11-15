/* varnam-symbol-table.h
 *
 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */



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
    int match_type,
    int priority,
    int accept_condition);

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

void
print_tokens_array(varray *tokens);

void
destroy_all_statements(struct varnam_internal* vi);

int
vst_make_prefix_tree (varnam *handle);

#endif
