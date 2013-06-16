/* print-tokens.c - Print the tokens generated. This is mostly used for debugging
 *
 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */


#include <stdio.h>
#include <string.h>
#include "../symbol-table.h"
#include "../varnam.h"

int main(int argc, char **argv)
{
    int rc = 0;
    varnam *handle;
    char *msg;
    int tokenize_using = 0;
    varray *tokens = varray_init();

    if (argc != 4) {
        printf ("Usage : %s symbols-file-path text-to-tokenize tokenization-type(pattern|value)\n", argv[0]);
        return 1;
    }

    rc = varnam_init (argv[1], &handle, &msg);
    if (rc != VARNAM_SUCCESS) {
        printf ("Initialization failed. %s\n", msg);
        return 1;
    }

    if (strcmp (argv[3], "pattern") == 0) {
        tokenize_using = VARNAM_TOKENIZER_PATTERN;
    }
    else {
        tokenize_using = VARNAM_TOKENIZER_VALUE;
    }

    rc = vst_tokenize (handle, argv[2], tokenize_using, VARNAM_MATCH_ALL, tokens);
    if (rc != VARNAM_SUCCESS) {
        printf ("Tokenization failed. %s\n", varnam_get_last_error(handle));
        return 1;
    }

    print_tokens_array (tokens);

    return 0;
}

