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

#include "varnam-api.h"
#include "foreign/sqlite3.h"
#include "varnam-util.h"
#include "varnam-types.h"
#include "varnam-result-codes.h"
#include "varnam-symbol-table.h"
#include "varnam-array.h"
#include "varnam-token.h"

static void
initialize_word(vword *word, const char *text, int confidence)
{
    assert (word);
    
    word->text       = text;
    word->confidence = confidence;
}

static vword*
Word(const char *text, int confidence)
{
    vword *word = xmalloc (sizeof (vword));
    initialize_word (word, text, confidence);
    return word;
}

static vword*
get_pooled_word(varnam *handle, const char *text, int confidence)
{
    vword *word;
    
    if (v_->words_pool == NULL)
        v_->words_pool = vpool_init ();

    word = vpool_get (v_->words_pool);
    if (word == NULL)
    {
        word = Word (text, confidence);
        vpool_add (v_->words_pool, word);
    }
    else
        initialize_word (word, text, confidence);

    return word;
}

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
static int 
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

/* static void */
/* set_last_token(varnam *handle, struct token *tok) */
/* { */
/*     struct varnam_internal *vi; */
/*     vi = handle->internal; */

/*     if(tok == NULL) { */
/*         vi->last_token_available = 0; */
/*         return; */
/*     } */

/*     if(vi->last_token == NULL) { */
/*         vi->last_token = (struct token *) xmalloc(sizeof (struct token)); */
/*         assert(vi->last_token); */
/*     } */

/*     vi->last_token->type = tok->type; */
/*     strncpy (vi->last_token->pattern, tok->pattern, VARNAM_SYMBOL_MAX); */
/*     strncpy (vi->last_token->value1, tok->value1, VARNAM_SYMBOL_MAX); */
/*     strncpy (vi->last_token->value2, tok->value2, VARNAM_SYMBOL_MAX); */
/*     vi->last_token->children = tok->children; */
/*     vi->last_token_available = 1; */
/* } */


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


/* static int  */
/* tokenize(varnam *handle,  */
/*          const char *input,  */
/*          struct strbuf *string) */
/* { */
/*     const char *text,  *remaining; */
/*     int matchpos = 0, counter = 0; */
/*     struct varnam_internal *vi;        */
/*     struct strbuf *lookup; */
/*     struct token *temp = NULL, *last = NULL; */

/*     vi = handle->internal; */
/*     lookup = vi->lookup; */

/*     text = input; */
/*     while( *text != '\0' )  */
/*     { */
/*         strbuf_addc( lookup, *text ); */
/*         ++counter; */

/*         temp = find_token( handle, lookup->buffer ); */
/*         if (temp) { */
/*             last = temp; */
/*             matchpos = counter; */
/*             if( last->children <= 0 ) break; */
/*         } */
/*         else if( !can_find_token( handle, last, lookup->buffer )) {  */
/*             break; */
/*         } */
/*         ++text; */
/*     } */

/*     if (last)  */
/*     { */
/*         resolve_token(handle, last, string); */
/*         remaining = input + matchpos; */
/*         set_last_token (handle, last); */
/*     } */
/*     else { */
/*         if(lookup->buffer[0] != '_') */
/*             strbuf_addc( string, lookup->buffer[0] ); */
/*         remaining = input + 1; */
/*         set_last_token (handle, NULL); */
/*     } */

/*     strbuf_clear (lookup); */
/*     if( strlen( remaining ) > 0 ) */
/*         return tokenize( handle, remaining, string ); */

/*     return VARNAM_SUCCESS; */
/* } */

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

