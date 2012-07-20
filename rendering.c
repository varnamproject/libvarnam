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

#include "varnam-symbol-table.h"
#include "varnam-result-codes.h"
#include "varnam-array.h"
#include "varnam-types.h"
#include "rendering.h"
#include "vword.h"

struct varnam_token_rendering*
get_additional_rendering_rule(varnam *handle)
{
    /* struct varnam_token_rendering *tr; */
    /* int i; */

    if(handle->internal->renderers == NULL) return NULL;

    /* Will be fixed when this module is touched */
    /* if(handle->internal->scheme_identifier[0] == '\0') { */
    /*     fill_general_values(handle, handle->internal->scheme_identifier, "scheme_identifier"); */
    /* } */

    /* Will be fixed when this module is touched */
    /* for(i = 0; i < 1; i++) */
    /* { */
    /*     tr = &(handle->internal->renderers[i]); */
    /*     if(strcmp(tr->scheme_identifier, handle->internal->scheme_identifier) == 0) */
    /*         return tr; */
    /* } */
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
    vtoken *virama, *token, *previous;
    strbuf *string;
    struct varnam_token_rendering *rule;
    int rc, i;

    assert(handle);

    rc = vst_get_virama (handle, &virama);
    if (rc)
        return rc;

    /* will be fixed after implementing register_renderer() */
    /* rule = get_additional_rendering_rule(handle); */
    /* if(rule != NULL) { */
    /*     rc = rule->render(handle, match, string); */
    /*     if(rc == VARNAM_SUCCESS) { */
    /*         return; */
    /*     } */
    /* } */

    string = get_pooled_string (handle);
    for(i = 0; i < varray_length(tokens); i++)
    {
        token = varray_get (tokens, i);
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
            if(strbuf_endswith(string, virama->value1)) {
                /* removing the virama and adding dependent vowel value */
                strbuf_remove_from_last(string, virama->value1);
                if(token->value2[0] != '\0') {
                    strbuf_add(string, token->value2);
                }
            }
            else if(previous != NULL) {
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
