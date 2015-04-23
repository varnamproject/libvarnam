/* rendering.c - Token rendering related functions
 *
 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */


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
    int i, rc;
		vscheme_details *details;

		rc = varnam_get_scheme_details(handle, &details);
		if (rc != VARNAM_SUCCESS) {
      varnam_log (handle, "Scheme details is not set. Custom rendering will be unavailable");
			return NULL;
		}

    if (details->identifier == NULL) {
        varnam_log (handle, "Scheme id is not set. Custom rendering will not be processed");
        return NULL;
    }

    for (i = 0; i < varray_length (v_->renderers); i++)
    {
        r = varray_get (v_->renderers, i);
        assert (r);
        if (strcmp(r->scheme_id, details->identifier) == 0) {
          return r;
        }
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

        if (token->type == VARNAM_TOKEN_NON_JOINER) {
            previous = NULL;
            continue;
        }

#ifdef _VARNAM_VERBOSE
        printf ("Token %s, %d\n", token->pattern, token->type);
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
        else if (token->type == VARNAM_TOKEN_NUMBER)
        {
            if (v_->config_use_indic_digits)
                strbuf_add (string, token->value1);
            else
                strbuf_add (string, token->pattern);
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
