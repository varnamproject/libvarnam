
/* 02-ml.c
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

#define LINE_MAX 1000

int ml_unicode_transliteration(int argc, char **argv)
{
    varnam *handle;
    int rc;
    char *msg;
    varray *words;
    vword *word;
    FILE *fp;   
    char line[LINE_MAX];
    char *part1, *part2;

    if(argc == 0) {
        printf("no scheme file specified\n");
        return 1;
    }
    else if(argc == 1) {
        printf("no input file specified\n");
        return 1;
    }
    printf("%s\n", argv[0]);
    printf("%s\n", argv[1]);

    rc = varnam_init(argv[0], &handle, &msg);
    if(rc != VARNAM_SUCCESS) {
        printf("initialization failed - %s\n", msg);
        return 1;
    }

    /* reads input from the supplied file and matches it with the expected string */
    fp = fopen(argv[1], "r");
    if(fp == NULL) {
        printf("can't open input file\n");
        return 1;
    }

    while(fgets(line, LINE_MAX, fp) != NULL) 
    {
        part1 = strtok(line, " ");
        part2 = strtok(NULL, "\n");
                   
        rc = varnam_transliterate(handle, part1, &words);
        if(rc != VARNAM_SUCCESS) {
            printf("transliteration of %s failed  - \n", part1);
            printf("%s", varnam_get_last_error(handle));
            return 1;
        }            

        word = varray_get (words, 0);
        if(strcmp(word->text, part2) != 0) {
            printf("transliterating %s - expected %s, but was %s\n", part1, part2, word->text);
            return 1;
        }        
    }

    fclose(fp);
    rc = varnam_destroy(handle);
    if(rc != VARNAM_SUCCESS) {
        printf("destruction failed\n");
        return 1;
    }

    return 0;
}

int ml_unicode_reverse_transliteration(int argc, char **argv)
{
    varnam *handle;
    int rc;
    char *msg;
    char *output;
    FILE *fp;
    char line[LINE_MAX];
    char *part1, *part2;

    if(argc == 0) {
        printf("no scheme file specified\n");
        return 1;
    }
    else if(argc == 1) {
        printf("no input file specified\n");
        return 1;
    }
    printf("%s\n", argv[0]);
    printf("%s\n", argv[1]);

    rc = varnam_init(argv[0], &handle, &msg);
    if(rc != VARNAM_SUCCESS) {
        printf("initialization failed - %s\n", msg);
        return 1;
    }

    /* reads input from the supplied file and matches it with the expected string */
    fp = fopen(argv[1], "r");
    if(fp == NULL) {
        printf("can't open input file\n");
        return 1;
    }

    while(fgets(line, LINE_MAX, fp) != NULL)
    {
        part1 = strtok(line, " ");
        part2 = strtok(NULL, "\n");
                   
        rc = varnam_reverse_transliterate(handle, part1, &output);
        if(rc != VARNAM_SUCCESS) {
            printf("reverse transliteration of %s failed  - \n", part1);
            return 1;
        }

        if(strcmp(output, part2) != 0) {
            printf("reverse transliterating %s - expected %s, but was %s\n", part1, part2, output);
            return 1;
        }
    }

    fclose(fp);
    rc = varnam_destroy(handle);
    if(rc != VARNAM_SUCCESS) {
        printf("destruction failed\n");
        return 1;
    }

    return 0;
}

