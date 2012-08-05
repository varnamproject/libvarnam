/* tl.c - implementation of transliterator and reverese transliterator
 *
 * Copyright (C) 2010 Navaneeth.K.N
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
#include <assert.h>

#include "deps/sqlite3.h"
#include "api.h"
#include "util.h"
#include "vtypes.h"
#include "result-codes.h"
#include "symbol-table.h"
#include "words-table.h"
#include "varray.h"
#include "token.h"
#include "vword.h"
#include "rendering.h"

/* Flattens the multi dimensional array all_tokens */
static varray*
flatten(varnam *handle, varray *all_tokens)
{
    int i, j;
    varray *tmp;
    vtoken *token;
    varray *tokens = get_pooled_array (handle);

    for (i = 0; i < varray_length (all_tokens); i++)
    {
        tmp = varray_get (all_tokens, i);
        for (j = 0; j < varray_length (tmp); j++)
        {
            token = varray_get (tmp, j);
            varray_push (tokens, token);
        }
    }

    return tokens;
}

int
varnam_transliterate(varnam *handle, const char *input, varray **output)
{
    int rc, i;
    varray *words = 0, *tokens = 0;
    varray *all_tokens = 0; /* This will be multidimensional array */
    vword *word, *word1;

    if(handle == NULL || input == NULL)
        return VARNAM_ARGS_ERROR;

    reset_pool(handle);
    varnam_debug (handle, "Transliterating %s", input);

    all_tokens = get_pooled_array (handle);
    rc = vst_tokenize (handle, input, VARNAM_TOKENIZER_PATTERN, VARNAM_MATCH_EXACT, all_tokens);
    if (rc)
        return rc;

    /* all_tokens will be a multidimensional array. Flattening it before resolving */
    tokens = flatten (handle, all_tokens);
    rc = resolve_tokens (handle, tokens, &word);
    if (rc)
        return rc;

    if (words == NULL) {
        words = get_pooled_array (handle);
    }

    rc = vwt_get_suggestions (handle, input, words);
    if (rc)
        return rc;

    if (varray_is_empty (words) && strlen(input) > 2)
    {
        /* We don't have any suggestions for the input. In this case, varnam does
         * it's best to provide suggestions by doing a tokenization on words table */
        rc = vwt_tokenize_pattern (handle, input, all_tokens);
        if (rc) return rc;

        for (i = 0; i < varray_length (all_tokens); i++)
        {
            tokens = varray_get (all_tokens, i);
            rc = resolve_tokens (handle, tokens, &word1);
            if (rc) return rc;
            varray_push (words, word1);
        }
    }

    varray_push (words, word);

    *output = words;
    return VARNAM_SUCCESS;
}

int
varnam_reverse_transliterate(varnam *handle,
                             const char *input,
                             char **output)
{
    int rc;
    varray *result;

    if(handle == NULL || input == NULL)
        return VARNAM_ARGS_ERROR;

    reset_pool (handle);

    result = get_pooled_array (handle);
    rc = vst_tokenize (handle, input, VARNAM_TOKENIZER_VALUE, VARNAM_MATCH_EXACT, result);
    if (rc)
        return rc;

    rc = resolve_rtl_tokens (handle, result, output);
    if (rc)
        return rc;

    varnam_debug (handle, "Reverse transliterating %s = %s", input, *output);

    return VARNAM_SUCCESS;
}
