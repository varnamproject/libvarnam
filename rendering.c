/* rendering.c - Token rendering related functions
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

#include <assert.h>
#include <string.h>

#include "symbol-table.h"
#include "result-codes.h"
#include "varray.h"
#include "vtypes.h"
#include "rendering.h"
#include "vword.h"
#include "api.h"

static vtoken_renderer*
get_renderer(varnam *handle)
{
    vtoken_renderer *r;
    int i;

    const char *scheme_id = varnam_get_scheme_identifier (handle);
    if (scheme_id == NULL) {
        varnam_log (handle, "Scheme id is not set. Custom rendering will not be processed");
        return NULL;
    }

    for (i = 0; i < varray_length (v_->renderers); i++)
    {
        r = varray_get (v_->renderers, i);
        assert (r);
        if (strcmp(r->scheme_id, scheme_id) == 0)
            return r;
    }

    return NULL;
}

/* Resolves the tokens.
 * tokens will be a single dimensional array where each item is vtoken instances
 */
int
resolve_tokens(varnam *handle,
               varray *tokens,
               vword **word)
{
    vtoken *virama, *token = NULL, *previous = NULL;
    strbuf *string;
    vtoken_renderer *r;
    int rc, i;

    assert(handle);

    rc = vst_get_virama (handle, &virama);
    if (rc)
        return rc;

    string = get_pooled_string (handle);
    for(i = 0; i < varray_length(tokens); i++)
    {
        token = varray_get (tokens, i);

#ifdef _VARNAM_VERBOSE
        varnam_debug (handle, "Token - %d, %s, %s", token->id, token->pattern, token->value1);
#endif

        r = get_renderer (handle);
        if (r != NULL)
        {
            rc = r->tl (handle, previous, token, string);
            if (rc == VARNAM_ERROR)
                return rc;
            if (rc == VARNAM_SUCCESS)
                continue;
        }

        if (token->type == VARNAM_TOKEN_VIRAMA)
        {
            /* we are resolving a virama. If the output ends with a virama already, add a
               ZWNJ to it, so that following character will not be combined.
               if output not ends with virama, add a virama and ZWNJ */
            if(strbuf_endswith (string, virama->value1)) {
                strbuf_add (string, ZWNJ());
            }
            else {
                strbuf_add (string, virama->value1);
                strbuf_add (string, ZWNJ());
            }
        }
        else if(token->type == VARNAM_TOKEN_VOWEL)
        {
            if(virama && strbuf_endswith(string, virama->value1)) {
                /* removing the virama and adding dependent vowel value */
                strbuf_remove_from_last(string, virama->value1);
                if(token->value2[0] != '\0') {
                    strbuf_add(string, token->value2);
                }
            }
            else if(previous != NULL && previous->type != VARNAM_TOKEN_OTHER) {
                strbuf_add(string, token->value2);
            }
            else {
                strbuf_add(string, token->value1);
            }
        }
        else {
            strbuf_add(string, token->value1);
        }

        previous = token;
    }

    *word = get_pooled_word (handle, strbuf_to_s (string), 1);
    return VARNAM_SUCCESS;
}

/*
 * Resolve tokens for reverse transliteration. tokens will be multidimensional array
 */
int
resolve_rtl_tokens(varnam *handle,
                  varray *all_tokens,
                  char **output)
{
    int rc, i, j;
    vtoken_renderer *r;
    strbuf *rtl;
    vtoken *token = NULL, *previous = NULL;
    varray *tokens;

    assert (handle);
    assert (all_tokens);

    rtl = get_pooled_string (handle);
    assert (rtl);
    for (i = 0; i < varray_length (all_tokens); i++)
    {
        tokens = varray_get (all_tokens, i);
        assert (tokens);
        for (j = 0; j < varray_length (tokens); j++)
        {
            token = varray_get (tokens, j);
            assert (token);

            r = get_renderer (handle);
            if (r != NULL)
            {
                rc = r->rtl (handle, previous, token, rtl);
                if (rc == VARNAM_ERROR)
                    return rc;
                if (rc == VARNAM_SUCCESS) {
                    previous = token;
                    break;
                }
            }

            strbuf_add (rtl, token->pattern);

            /* vowel is standing in it's full form in between a word. need to prefix _
               to avoid unnecessary conjunctions */
            if (token->type == VARNAM_TOKEN_VOWEL && previous != NULL)
            {
                if (strcmp(token->value1, previous->value1) == 0)
                    strbuf_add (rtl, "_");
            }

            previous = token;
            break; /* We only care about first element in each array */
        }
    }

    strbuf_remove_from_last (rtl, "_");

    *output = rtl->buffer;
    return VARNAM_SUCCESS;
}
