/* 
 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */

#include <stdio.h>
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

int main(int argc, char **argv)
{
	int rc;
	char *msg;
	varnam *handle;
	varray *words;

	/* Initialization */
	rc = varnam_init_from_id ("ml", &handle, &msg);
	if (rc != VARNAM_SUCCESS)
	{
		printf("Initialization failed. Reason - %s", msg);
		free (msg);
		return 1;
	}

	printf("transliterating the text 'malayalam' before learning\n");
	rc = varnam_transliterate (handle, "malayalam", &words);
	if (rc != VARNAM_SUCCESS)
	{
		printf("Transliteration failed. Reason - %s", varnam_get_last_error(handle));
		varnam_destroy(handle);
		return 1;
	}
	print_transliteration_output ("malayalam", words);

	printf("Learning the word മലയാളം\n");
	rc = varnam_learn (handle, "മലയാളം");
	if (rc != VARNAM_SUCCESS)
	{
		printf("Failed to learn. Reason %s\n", varnam_get_last_error(handle));
		varnam_destroy (handle);
		return 1;
	}

	printf("transliterating the text 'malayalam' after learning. This should give two suggestions. Learned word and literal transliteration\n");
	rc = varnam_transliterate (handle, "malayalam", &words);
	if (rc != VARNAM_SUCCESS)
	{
		printf("Transliteration failed. Reason - %s", varnam_get_last_error(handle));
		varnam_destroy(handle);
		return 1;
	}
	print_transliteration_output ("malayalam", words);

	printf("The more you learn, the more confident you are\n");
	printf("Learning the word മലയാളം again\n");
	rc = varnam_learn (handle, "മലയാളം");
	if (rc != VARNAM_SUCCESS)
	{
		printf("Failed to learn. Reason %s\n", varnam_get_last_error(handle));
		varnam_destroy (handle);
		return 1;
	}

	printf("transliterating the text 'malayalam' after learning. You can see it is more confident about the word now!\n");
	rc = varnam_transliterate (handle, "malayalam", &words);
	if (rc != VARNAM_SUCCESS)
	{
		printf("Transliteration failed. Reason - %s", varnam_get_last_error(handle));
		varnam_destroy(handle);
		return 1;
	}
	print_transliteration_output ("malayalam", words);

    printf("Deleting the word 'malayalam' from the known words.\n");
    rc = varnam_delete_word (handle, "മലയാളം");
    if (rc != VARNAM_SUCCESS)
    {
        printf("Deletion failed. Reason - %s", varnam_get_last_error(handle));
        varnam_destroy (handle);
        return 1;
    }
    
    varnam_destroy (handle);

    return 0;
}
