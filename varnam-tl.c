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

#include "foreign/sqlite3.h"
#include "varnam-api.h"
#include "varnam-util.h"
#include "varnam-types.h"
#include "varnam-result-codes.h"
#include "varnam-symbol-table.h"
#include "varnam-array.h"
#include "varnam-token.h"
#include "vword.h"
#include "rendering.h"

static void
set_last_rtl_token(varnam *handle, struct token *tok)
{
    struct varnam_internal *vi;
    vi = handle->internal;

    if(tok == NULL) {
        vi->last_rtl_token_available = 0;
        return;
    }

    if(vi->last_rtl_token == NULL) {
        vi->last_rtl_token = (struct token *) xmalloc(sizeof (struct token));
        assert(vi->last_rtl_token);
    }

    vi->last_rtl_token->type = tok->type;
    strncpy (vi->last_rtl_token->pattern, tok->pattern, VARNAM_SYMBOL_MAX);
    strncpy (vi->last_rtl_token->value1, tok->value1, VARNAM_SYMBOL_MAX);
    strncpy (vi->last_rtl_token->value2, tok->value2, VARNAM_SYMBOL_MAX);
    vi->last_rtl_token->children = tok->children;
    vi->last_rtl_token_available = 1;
}

static void 
cleanup(varnam *handle)
{
    strbuf_clear(handle->internal->output);
    strbuf_clear(handle->internal->rtl_output);
    handle->internal->last_token_available = 0;
    handle->internal->last_rtl_token_available = 0;
}

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
    int rc;
    varray *words = 0, *tokens = 0;
    varray *all_tokens = 0; /* This will be multidimensional array */
    vword *word;
    
    if(handle == NULL || input == NULL)
        return VARNAM_ARGS_ERROR;

    reset_pool(handle);

    all_tokens = get_pooled_array (handle);
    rc = vst_tokenize (handle, input, VARNAM_TOKENIZER_PATTERN, all_tokens);
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
    varray_push (words, word);
    *output = words;
    return VARNAM_SUCCESS;
}

static void 
resolve_rtl_token(varnam *handle,
                  const char *lookup,
                  struct token *match,
                  struct strbuf *string)
{   
    struct varnam_token_rendering *rule;
    int rc;

    assert(handle);
    assert(match);
    assert(string);

    rule = get_additional_rendering_rule (handle);
    if (rule != NULL) {
        rc = rule->render_rtl (handle, match, string);
        if(rc == VARNAM_SUCCESS) {
            return;
        }
    }

    if (match->type == VARNAM_TOKEN_VOWEL)
    {
        if (strcmp (match->value1, lookup) == 0 && handle->internal->last_rtl_token_available) {
            /* vowel is standing in it's full form in between a word. need to prefix _
               to avoid unnecessary conjunctions */
            strbuf_add(string, "_");
        }
    }

    strbuf_add (string, match->pattern);
}


static int 
tokenize_indic_text(varnam *handle,
                    const char *input,
                    struct strbuf *string)
{
    const char *remaining;
    int counter = 0, input_len = 0;
    size_t matchpos = 0;
    /* struct varnam_internal *vi;        */
    char lookup[100], match[100];
    struct token *temp = NULL, *last = NULL;

    /* vi = handle->internal; */
    match[0] = '\0';

    input_len = utf8_length (input);
    while (counter < input_len) 
    {
        substr (lookup, input, 1, ++counter);
        temp = find_rtl_token (handle, lookup);
        if (temp) {
            last = temp;
            matchpos = strlen (lookup);
            strncpy(match, lookup, 100);
        }
        else if( !can_find_rtl_token( handle, last, lookup )) { 
            break;
        }
    }

    if (last) 
    {
        resolve_rtl_token (handle, match, last, string);
        remaining = input + matchpos;
        set_last_rtl_token (handle, last);
    }
    else {
        strbuf_add (string, lookup);
        remaining = input + 1;
        set_last_rtl_token (handle, NULL);
    }

    if (strlen (remaining) > 0)
        return tokenize_indic_text (handle, remaining, string);

    return VARNAM_SUCCESS;
}
                    
int 
varnam_reverse_transliterate(varnam *handle,
                             const char *input,
                             char **output)
{
    int rc;
    struct strbuf *result;
    
    if(handle == NULL || input == NULL)
        return VARNAM_MISUSE;

    cleanup (handle);
    result = handle->internal->rtl_output;
    rc = tokenize_indic_text (handle, input, result);
    *output = result->buffer;

    return rc;
}

