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
                  int type,
                  int match_type,
                  const char* pattern,
                  const char* value1,
                  const char* value2,
                  const char* tag)
{
    tok->type = type;
    tok->match_type = match_type;
    strncpy( tok->pattern, pattern, VARNAM_SYMBOL_MAX);
    strncpy( tok->value1, value1, VARNAM_SYMBOL_MAX);
    if (value2 != NULL)
        strncpy( tok->value2, value2, VARNAM_SYMBOL_MAX);
    if (tag != NULL)
        strncpy( tok->tag, tag, VARNAM_TOKEN_TAG_MAX);
}

struct token*
Token(int type, int match_type, const char* pattern, const char* value1, const char* value2, const char* tag)
{
    struct token* tok = (struct token*) xmalloc(sizeof(struct token));
    initialize_token (tok, type, match_type, pattern, value1, value2, tag);
    return tok;
}

struct token*
get_pooled_token (
    varnam *handle,
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
        tok = Token (type, match_type, pattern, value1, value2, tag);
        vpool_add (v_->tokens_pool, tok);
    }
    else
        initialize_token (tok, type, match_type, pattern, value1, value2, tag);

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
