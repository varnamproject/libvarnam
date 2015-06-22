/* <file-name>
 *
 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */



#ifndef VARNAM_TOKEN_H_INCLUDED_090112
#define VARNAM_TOKEN_H_INCLUDED_090112

#include "vtypes.h"
#include "varray.h"

struct token*
token_new();

void
initialize_token (vtoken *tok,
                  int id,
                  int type,
                  int match_type,
                  const char* pattern,
                  const char* value1,
                  const char* value2,
                  const char* value3,
                  const char* tag,
                  int priority,
                  int accept_condition,
                  int flags);

struct token*
Token(int id, int type, int match_type, const char* pattern, const char* value1, const char* value2, const char* value3, const char* tag, int priority, int accept_condition, int flags);

struct token*
get_pooled_token (
    varnam *handle,
    int id,
    int type,
    int match_type,
    const char* pattern,
    const char* value1,
    const char* value2,
    const char* value3,
    const char* tag,
    int priority,
    int accept_condition, int flags);

varray*
product_tokens(varnam *handle, varray *tokens);

void
destroy_token(void *token);

#endif
