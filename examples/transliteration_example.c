/* Explains the user of varnam C API
 *
 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */


#include <stdio.h>
#include <stdlib.h>

#include "../varnam.h"

static void
print_transliteration_output(const char *pattern, varray *words)
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
    rc = varnam_init ("../schemes/ml.vst", &handle, &msg);
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
