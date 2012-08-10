/* This example shows how to use varnam's learning APIs
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
	rc = varnam_init ("../schemes/ml-unicode.vst", &handle, &msg);
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

	/* To learn, we need to enable suggestions using varnam_config */
	rc = varnam_config(handle, VARNAM_CONFIG_ENABLE_SUGGESTIONS, "learning-example-suggestions.varnam");
	if (rc != VARNAM_SUCCESS)
	{
		printf("Error in configuring. Reason %s\n", varnam_get_last_error(handle));
		varnam_destroy (handle);
		return 1;
	}

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

	printf("transliterating the text 'malayalam' after learning again. You can see it is more confident about the word now!\n");
	rc = varnam_transliterate (handle, "malayalam", &words);
	if (rc != VARNAM_SUCCESS)
	{
		printf("Transliteration failed. Reason - %s", varnam_get_last_error(handle));
		varnam_destroy(handle);
		return 1;
	}
	print_transliteration_output ("malayalam", words);

    return 0;
}
