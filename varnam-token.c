/* Methods related to token creation and pooling
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

#include <string.h>
#include "varnam-token.h"
#include "varnam-types.h"
#include "varnam-util.h"
#include "varnam-array.h"
#include "varnam-result-codes.h"

static void
initialize_token (vtoken *tok,
                  int id,
                  int type,
                  int match_type,
                  const char* pattern,
                  const char* value1,
                  const char* value2,
                  const char* tag)
{
    tok->id = id;
    tok->type = type;
    tok->match_type = match_type;
    strncpy( tok->pattern, pattern, VARNAM_SYMBOL_MAX);
    strncpy( tok->value1, value1, VARNAM_SYMBOL_MAX);

    if (value2 != NULL)
        strncpy( tok->value2, value2, VARNAM_SYMBOL_MAX);
    else
        tok->value2[0] = '\0';

    if (tag != NULL)
        strncpy( tok->tag, tag, VARNAM_TOKEN_TAG_MAX);
    else
        tok->tag[0] = '\0';
}

struct token*
Token(int id, int type, int match_type, const char* pattern, const char* value1, const char* value2, const char* tag)
{
    struct token* tok = (struct token*) xmalloc(sizeof(struct token));
    initialize_token (tok, id, type, match_type, pattern, value1, value2, tag);
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
    const char* tag)
{
    vtoken *tok;
    
    if (v_->tokens_pool == NULL)
        v_->tokens_pool = vpool_init ();

    tok = vpool_get (v_->tokens_pool);
    if (tok == NULL)
    {
        tok = Token (id, type, match_type, pattern, value1, value2, tag);
        vpool_add (v_->tokens_pool, tok);
    }
    else
        initialize_token (tok, id, type, match_type, pattern, value1, value2, tag);

    return tok;
}

varray*
get_pooled_tokens (varnam *handle)
{
    varray *array;

    if (v_->tokens_array_pool == NULL)
        v_->tokens_array_pool = vpool_init ();

    array = vpool_get (v_->tokens_array_pool);
    if (array == NULL)
    {
        array = varray_init ();
        vpool_add (v_->tokens_array_pool, array);
    }

    varray_clear (array);
    return array;
}

void
reset_tokens_pool (varnam *handle)
{
    vpool_reset (v_->tokens_pool);
    vpool_reset (v_->tokens_array_pool);
}

varray* 
product_tokens(varnam *handle, varray *tokens)
{
    int array_cnt, *offsets, i, last_array_offset;
    varray *product, *array, *tmp;

    array_cnt = varray_length (tokens);
    offsets = xmalloc(sizeof(int) * (size_t) array_cnt);
    product = get_pooled_tokens (handle);

    varray_clear (product);
    for (i = 0; i < array_cnt; i++) offsets[i] = 0;

    for (;;)
    {
        array = get_pooled_tokens (handle);
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

