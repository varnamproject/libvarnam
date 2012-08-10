/* Explains the user of varnam C API
 *
 * Copyright (C) Navaneeth
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

#include <stdio.h>
#include <stdlib.h>

#include "../varnam.h"

void print_transliteration_output(const char *pattern, varray *words)
{
    int i;
    vword *word;
    printf  ("Transliterated %s\n", pattern);
    for (i = 0; i < varray_length (words); i++)
    {
        word = varray_get (words, i);
        printf ("  %s. Confidence %d\n", word->text, word->confidence);
    }
}

int main()
{
    varnam *handle;
    char *msg;
    const char *pattern;
    int rc;
    varray *words; /* Used to store transliterated words */
    
    /* Initialize varnam handle */
    rc = varnam_init ("../schemes/ml-unicode.vst", &handle, &msg);
    if (rc != VARNAM_SUCCESS)
    {
        printf("Initialization failed. Reason - %s\n", msg);
        free (msg);
        return 1;
    }

    /* Transliterating text. Varnam manages memory initialization for words. You don't have to explicitly release it */
    pattern = "navaneeth";
    rc = varnam_transliterate (handle, pattern, &words);
    if (rc != VARNAM_SUCCESS)
    {
        printf("Failed to transliterate. Reason - %s\n", varnam_get_last_error (handle));
        varnam_destroy (handle);
        return 1;
    }
    /* Words will have the transliterated text */
    print_transliteration_output (pattern, words);

    pattern = "malayaaLam";
    rc = varnam_transliterate (handle, pattern, &words);
    if (rc != VARNAM_SUCCESS)
    {
        printf("Failed to transliterate. Reason - %s\n", varnam_get_last_error (handle));
        varnam_destroy (handle);
        return 1;
    }
    /* Words will have the transliterated text */
    print_transliteration_output (pattern, words);

    /* Destroy the handle */
    varnam_destroy (handle);
    return 0;
}
