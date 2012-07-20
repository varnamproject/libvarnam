/* vword.c - Functions operating on a word
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

#include "vword.h"
#include "varnam-types.h"
#include "varnam-array.h"

static void
initialize_word(vword *word, const char *text, int confidence)
{
    assert (word);
    
    word->text       = text;
    word->confidence = confidence;
}

vword*
Word(const char *text, int confidence)
{
    vword *word = xmalloc (sizeof (vword));
    initialize_word (word, text, confidence);
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
        word = Word (text, confidence);
        vpool_add (v_->words_pool, word);
    }
    else
        initialize_word (word, text, confidence);

    return word;
}
