/* vword.c - Functions operating on a word
 *
 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */


#include <assert.h>
#include <string.h>

#include "vword.h"
#include "vtypes.h"
#include "varray.h"

static void
initialize_word(varnam *handle, vword *word, const char *text, int confidence)
{
    strbuf *w;

    assert (word);

    w = get_pooled_string (handle);
    strbuf_add (w, text);

    word->text       = strbuf_to_s (w);
    word->confidence = confidence;
}

vword*
Word(varnam *handle, const char *text, int confidence)
{
    vword *word = xmalloc (sizeof (vword));
    initialize_word (handle, word, text, confidence);
    return word;
}

vword*
get_pooled_word(varnam *handle, const char *text, int confidence)
{
    vword *word;

    if (v_->words_pool == NULL)
        v_->words_pool = vpool_init ();

    word = vpool_get (v_->words_pool);
    if (word == NULL)
    {
        word = Word (handle, text, confidence);
        vpool_add (v_->words_pool, word);
    }
    else
        initialize_word (handle, word, text, confidence);

    return word;
}

bool
word_equals (void *left, void *right)
{
    vword *lhs, *rhs;

    if (left == NULL)
        return false;

    if (right == NULL)
        return false;

    lhs = (vword*) left;
    rhs = (vword*) right;

    return strcmp (lhs->text, rhs->text) == 0;
}

void
destroy_word(void *word)
{
    if (word != NULL)
    {
        xfree((vword*) word);
    }
}
