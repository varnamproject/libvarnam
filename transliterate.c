/* tl.c - implementation of transliterator and reverese transliterator
 *
 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */


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

#ifdef _RECORD_EXEC_TIME
    V_BEGIN_TIMING
#endif

    if(handle == NULL || input == NULL)
        return VARNAM_ARGS_ERROR;

    reset_pool(handle);

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

    rc = vwt_get_best_match (handle, input, words);
    if (rc)
        return rc;

    if (varray_length (words) == 0 && strlen(input) > 2)
    {
        /* We don't have any best match for the input. In this case, varnam does
         * it's best to provide suggestions by doing a tokenization on words table */
        rc = vwt_tokenize_pattern (handle, input, all_tokens);
        if (rc) return rc;

        for (i = 0; i < varray_length (all_tokens); i++)
        {
            tokens = varray_get (all_tokens, i);
            rc = resolve_tokens (handle, tokens, &word1);
            if (rc) return rc;

            if (!varray_exists (words, word1, &word_equals))
                varray_push (words, word1);
        }
    }

    if (!varray_exists (words, word, &word_equals))
        varray_push (words, word);

    rc = vwt_get_suggestions (handle, input, words);
    if (rc)
        return rc;

    *output = words;

#ifdef _RECORD_EXEC_TIME
    V_REPORT_TIME_TAKEN("varnam_transliterate")
#endif
#ifdef _VARNAM_VERBOSE
    printf("Transliterating %s\n", input);
#endif
    return VARNAM_SUCCESS;
}

int
varnam_reverse_transliterate(varnam *handle,
                             const char *input,
                             char **output)
{
    int rc;
    varray *result;

#ifdef _RECORD_EXEC_TIME
    V_BEGIN_TIMING
#endif

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

#ifdef _RECORD_EXEC_TIME
    V_REPORT_TIME_TAKEN("varnam_reverse_transliterate")
#endif

    return VARNAM_SUCCESS;
}
