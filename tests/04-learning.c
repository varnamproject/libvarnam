/* Tests learning module
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

int test_varnam_learn(int argc, char **argv)
{
    char *msg;
    varnam *handle;
    int rc;
    vinfo *info;

    rc = varnam_init("output/04-learn.vst", &handle, &msg);
    if (rc != VARNAM_SUCCESS) {
        printf ("Something failed in init\n");
        return 1;
    }

    /* rc = varnam_create_token(handle, "a", "അ", NULL, VARNAM_TOKEN_VOWEL, VARNAM_MATCH_EXACT, 0); */
    /* rc = varnam_create_token(handle, "aa", "ആ", "ാ", VARNAM_TOKEN_VOWEL, VARNAM_MATCH_EXACT, 0); */
    /* rc = varnam_create_token(handle, "*n*", "n-t", "ാ", VARNAM_TOKEN_VOWEL, VARNAM_MATCH_EXACT, 0); */
    /* rc = varnam_create_token(handle, "*a*", "abb", "ാ", VARNAM_TOKEN_VOWEL, VARNAM_MATCH_EXACT, 0); */

    /* rc = varnam_learn (handle, "ആnav"); */

    /*rc = varnam_learn (handle, "abcdefഅghijklmn-topqrstuvwxyz");*/
    rc = varnam_learn (handle, "നവനീത്");

    /* rc = varnam_learn (handle, "ആആപ"); */

    varnam_get_info (handle, true, &info);

    printf ("No of in memory tokens - %d\n", info->tokens_in_memory);
    printf ("No of in memory arrays - %d\n", info->arrays_in_memory);

    return 0;


}
