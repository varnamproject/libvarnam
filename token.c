/* Methods related to token creation and pooling
 *
 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */

#include <string.h>
#include "token.h"
#include "vtypes.h"
#include "util.h"
#include "varray.h"
#include "result-codes.h"

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
                  int flags)
{
    tok->id = id;
    tok->type = type;
    tok->match_type = match_type;
    tok->priority = priority;
    tok->accept_condition = accept_condition;
    tok->flags = flags;
    strncpy( tok->pattern, pattern, VARNAM_SYMBOL_MAX);
    strncpy( tok->value1, value1, VARNAM_SYMBOL_MAX);

    if (value2 != NULL)
        strncpy( tok->value2, value2, VARNAM_SYMBOL_MAX);
    else
        tok->value2[0] = '\0';

    if (value3 != NULL)
        strncpy( tok->value3, value3, VARNAM_SYMBOL_MAX);
    else
        tok->value3[0] = '\0';

    if (tag != NULL)
        strncpy( tok->tag, tag, VARNAM_SYMBOL_MAX);
    else
        tok->tag[0] = '\0';
}

struct token*
Token(int id, int type, int match_type, const char* pattern, const char* value1,
      const char* value2, const char* value3, const char* tag, int priority, int accept_condition, int flags)
{
    struct token* tok = (struct token*) xmalloc(sizeof(struct token));
    initialize_token (tok, id, type, match_type, pattern, value1, value2, value3, tag, priority, accept_condition, flags);
    return tok;
}

struct token*
token_new()
{
  struct token* tok = (struct token*) xmalloc(sizeof(struct token));
	return tok;
}

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
    int accept_condition,
    int flags)
{
    vtoken *tok;

    if (v_->tokens_pool == NULL)
        v_->tokens_pool = vpool_init ();

    tok = vpool_get (v_->tokens_pool);
    if (tok == NULL)
    {
        tok = Token (id, type, match_type, pattern, value1, value2, value3, tag, priority, accept_condition, flags);
        vpool_add (v_->tokens_pool, tok);
    }
    else
        initialize_token (tok, id, type, match_type, pattern, value1, value2, value3, tag, priority, accept_condition, flags);

    return tok;
}

varray*
product_tokens(varnam *handle, varray *tokens)
{
    int array_cnt, *offsets, i, last_array_offset;
    varray *product, *array, *tmp;

    array_cnt = varray_length (tokens);
    offsets = xmalloc(sizeof(int) * (size_t) array_cnt);
    product = get_pooled_array (handle);

    varray_clear (product);
    for (i = 0; i < array_cnt; i++) offsets[i] = 0;

    for (;;)
    {
        array = get_pooled_array (handle);
        for (i = 0; i < array_cnt; i++) {
            tmp = varray_get (tokens, i);
            varray_push (array, varray_get (tmp, offsets[i]));
        }

        varray_push (product, array);

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
    return product;
}

void
destroy_token(void *token)
{
    if (token != NULL)
    {
        xfree((vtoken*) token);
    }
}

