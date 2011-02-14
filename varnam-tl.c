/* tl.c - implementation of transliterator
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

#include "varnam-api.h"
#include "foreign/sqlite3.h"
#include "varnam-util.h"
#include "varnam-types.h"
#include "varnam-result-codes.h"
#include "varnam-symbol-table.h"

struct varnam_token_rendering*
get_additional_rendering_rule(varnam *handle)
{
    struct varnam_token_rendering *tr;
    int i;

    if(handle->internal->renderers == NULL) return NULL;

    for(i = 0; i < 1; i++)
    {
        tr = &(handle->internal->renderers[i]);
        if(strcmp(tr->scheme_identifier, handle->internal->scheme_identifier) == 0)
            return tr;
    }
    return NULL;
}

static void 
resolve_token(varnam *handle,
              struct token *match,
              struct strbuf *string)
{   
    const char *virama = NULL;
    struct varnam_token_rendering *rule;
    int rc;

    assert(handle);
    assert(match);
    assert(string);

    if(handle->internal->virama[0] == '\0') {
        fill_general_values(handle, handle->internal->virama, "virama");
    }

    if(handle->internal->scheme_identifier[0] == '\0') {
        fill_general_values(handle, handle->internal->scheme_identifier, "scheme_identifier");
    }

    virama = handle->internal->virama;

    rule = get_additional_rendering_rule(handle);
    if(rule != NULL) {
        rc = rule->render(handle, match, string);
        if(rc == VARNAM_SUCCESS) {
            return;
        }
    }

    if(strcmp(match->type, VARNAM_TOKEN_VOWEL) == 0) 
    {
        if(strbuf_endswith(string, virama)) {
            /* removing the virama and adding dependent vowel value */
            strbuf_remove_from_last(string, virama);
            if(match->value2[0] != '\0') {
                strbuf_add(string, match->value2);
            }
        }
        else if(handle->internal->last_token_available) {
            strbuf_add(string, match->value2);
        }
        else {
            strbuf_add(string, match->value1);
        }
    }
    else {
        strbuf_add(string, match->value1);
    }
}

static void
set_last_token(varnam *handle, struct token *tok)
{
    struct varnam_internal *vi;
    vi = handle->internal;

    if(tok == NULL) {
        vi->last_token_available = 0;
        return;
    }

    if(vi->last_token == NULL) {
        vi->last_token = (struct token *) xmalloc(sizeof (struct token));
        assert(vi->last_token);
    }

    strncpy (vi->last_token->type, tok->type, VARNAM_TOKEN_TYPE_MAX);
    strncpy (vi->last_token->pattern, tok->pattern, VARNAM_SYMBOL_MAX);
    strncpy (vi->last_token->value1, tok->value1, VARNAM_SYMBOL_MAX);
    strncpy (vi->last_token->value2, tok->value2, VARNAM_SYMBOL_MAX);
    vi->last_token->children = tok->children;
    vi->last_token_available = 1;
}

static int 
tokenize(varnam *handle, 
         const char *input, 
         struct strbuf *string)
{
    const char *text,  *remaining;
    int matchpos = 0, counter = 0;
    struct varnam_internal *vi;       
    struct strbuf *lookup;
    struct token *temp = NULL, *last = NULL;

    vi = handle->internal;
    lookup = vi->lookup;

    text = input;
    while( *text != '\0' ) 
    {
        strbuf_addc( lookup, *text );
        ++counter;

        temp = find_token( handle, lookup->buffer );
        if (temp) {
            last = temp;
            matchpos = counter;
            if( last->children <= 0 ) break;
        }
        else if( !can_find_token( handle, last, lookup->buffer )) { 
            break;
        }
        ++text;
    }

    if (last) 
    {
        resolve_token(handle, last, string);
        remaining = input + matchpos;
        set_last_token (handle, last);
    }
    else {
        if(lookup->buffer[0] != '_')
            strbuf_addc( string, lookup->buffer[0] );
        remaining = input + 1;
        set_last_token (handle, NULL);
    }

    strbuf_clear (lookup);
    if( strlen( remaining ) > 0 )
        return tokenize( handle, remaining, string );

    return VARNAM_SUCCESS;
}

static void 
cleanup(varnam *handle)
{
    strbuf_clear(handle->internal->output);
    handle->internal->last_token_available = 0;
}

int 
varnam_transliterate(varnam *handle, 
                         const char *input, 
                         char **output)
{
    int rc;
    struct strbuf *result;
    
    if(handle == NULL || input == NULL)
        return VARNAM_MISUSE;

    cleanup(handle);
    result = handle->internal->output;
    rc = tokenize( handle, input, result );
    *output = result->buffer;

    return rc;
}

