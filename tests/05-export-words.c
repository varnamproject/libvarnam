/* Tests word export
 *
 * Copyright (C) Navaneeth K N
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
#include <string.h>
#include "../varnam.h"

int
test_varnam_export(int argc, char** argv)
{
    varnam* handle;
    char* msg;
    int rc, confidence;
    FILE* fp;
    char buffer[100];

    const char* word1 =  "മലയാളം";
    const char* word2 = "വർണം";
    const char* word3 = "തമിഴ്";

    rc = varnam_init("../schemes/ml-unicode.vst", &handle, &msg);
    if (rc != VARNAM_SUCCESS) {
        printf ("Something failed in init\n");
        printf ("%s\n", msg);
        return 1;
    }

    rc = varnam_config (handle, VARNAM_CONFIG_ENABLE_SUGGESTIONS, "output/05-learnings-from-file");
    if (rc != VARNAM_SUCCESS)
    {
        printf ("Failed to enable suggestions - %s", varnam_get_last_error (handle));
        return 1;
    }

    rc = varnam_learn (handle, word1);
    if (rc != VARNAM_SUCCESS) {
        printf ("Error - %s", varnam_get_last_error (handle));
        return 1;
    }

    rc = varnam_learn (handle, word2);
    if (rc != VARNAM_SUCCESS) {
        printf ("Error - %s", varnam_get_last_error (handle));
        return 1;
    }

    rc = varnam_learn (handle, word3);
    if (rc != VARNAM_SUCCESS) {
        printf ("Error - %s", varnam_get_last_error (handle));
        return 1;
    }

    rc = varnam_export_words (handle, 2, "output/");
    if (rc != VARNAM_SUCCESS) {
        printf ("Error - %s", varnam_get_last_error (handle));
        return 1;
    }

    fp = fopen ("output/0.txt", "r");
    if (fp == NULL) {
        printf ("Error opening 0.txt\n");
        return 1;
    }

    fscanf (fp, "%s %d", buffer, &confidence);
    if (strcmp(buffer, word1) != 0) {
        printf ("Expected %s, but got %s\n", word1, buffer);
        return 1;
    }

    fscanf (fp, "%s %d", buffer, &confidence);
    if (strcmp(buffer, word2) != 0) {
        printf ("Expected %s, but got %s\n", word2, buffer);
        return 1;
    }

    if (fscanf (fp, "%s %d", buffer, &confidence) == 2) {
        printf ("0.txt has more than 2 lines");
        return 1;
    }

    fclose (fp);

    fp = fopen ("output/1.txt", "r");
    if (fp == NULL) {
        printf ("Error opening 0.txt\n");
        return 1;
    }

    fscanf (fp, "%s %d", buffer, &confidence);
    if (strcmp(buffer, word3) != 0) {
        printf ("Expected %s, but got %s\n", word3, buffer);
        return 1;
    }

    if (fscanf (fp, "%s %d", buffer, &confidence) == 2) {
        printf ("1.txt has more than 1 lines");
        return 1;
    }

    fclose (fp);

    return 0;
}
