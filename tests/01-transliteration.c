/* 01-transliteration.c
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

/* Contains test cases for basic transliteration. this test uses some test data rather than 
actual characters in the supported languages */

#include <stdio.h>
#include <string.h>
#include "../varnam.h"

static int rendering_vowels(varnam *handle)
{
    char *output;

    varnam_transliterate(handle, "oof", &output);
    printf("%s\n", output);
    if(strcmp(output, "v-oof~") == 0) 
        return 0;
    return 1;
}

static int rendering_dependent_vowels(varnam *handle)
{
    char *output;

    varnam_transliterate(handle, "foo", &output);
    printf("%s\n", output);
    if(strcmp(output, "fdv-oo") == 0) 
        return 0;
    return 1;
}

static int scheme_details(varnam *handle)
{
    const char *author = varnam_scheme_author (handle);
    const char *identifier = varnam_scheme_identifier (handle);
    const char *display_name = varnam_scheme_display_name (handle);

    if(strcmp(author, "navaneeth") != 0) 
    {
        printf("author : expected %s but was %s\n", "navaneeth", author);
        return 1;
    }

    if(strcmp(identifier, "ml") != 0) 
    {
        printf("scheme_identifier : expected %s but was %s\n", "ml", identifier);
        return 1;
    }

    if(strcmp(display_name, "language name") != 0) 
    {
        printf("scheme_display_name : expected %s but was %s\n", "language name", display_name);
        return 1;
    }

    return 0;
}

int basic_transliteration(int argc, char **argv)
{
    varnam *handle;
    int rc;
    char *msg;

    if(argc == 0) {
        printf("no scheme file specified\n");
        return 1;
    }
    printf("%s\n", argv[0]);
    rc = varnam_init(argv[0], strlen(argv[0]), &handle, &msg);
    if(rc != VARNAM_SUCCESS) {
        printf("initialization failed - %s\n", msg);
        return 1;
    }

    rc = scheme_details (handle);
    if(rc != VARNAM_SUCCESS) {
        printf("reading scheme details failed - \n");
        return 1;
    }    

    rc = rendering_vowels(handle);
    if(rc != VARNAM_SUCCESS) {
        printf("rendering dependent vowels is not proper - \n");
        return 1;
    }

    rc = rendering_dependent_vowels(handle);
    if(rc != VARNAM_SUCCESS) {
        printf("rendering dependent vowels is not proper - \n");
        return 1;
    }

    return 0;
}
