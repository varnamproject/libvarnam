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

int
vst_persist_stemrule(varnam *handle, const char* old_ending, const char* new_ending);

int 
vst_persist_stem_exception(varnam *handle, const char *rule, const char *exception);

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

int
vst_has_stemrules (varnam *handle);

int 
vst_get_last_syllable (varnam *handle, strbuf *string, strbuf *syllable);

int
vst_check_exception(varnam *handle, strbuf *word_buffer, strbuf *end_buffer);

int
vst_get_stem(varnam* handle, strbuf* old_ending, strbuf *new_ending);

int
vst_stamp_version (varnam *handle);

int
vst_load_scheme_details(varnam *handle, vscheme_details *output);

#endif
