/* 00-init.c
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

/* Contains test cases for initialization */

#include <stdio.h>
#include <string.h>
#include "../varnam.h"

int strbuf_formatted_strings()
{
    struct strbuf *buffer = strbuf_init(100);
    strbuf_addf(buffer, "Character %c, Integer %d, String %s\n", 'a', 10, "Navaneth");
    printf("%s", buffer->buffer);

    if (strcmp("Character a, Integer 10, String Navaneth\n", buffer->buffer) != 0)
    {
        printf ("Formatting is incorrect");
        return 1;
    }

    return VARNAM_SUCCESS;
}

int set_scheme_details()
{
    int rc;
    char *msg;
    varnam *handle;

    rc = varnam_init("output/00-set-scheme-details.vst", &handle, &msg);
    if(rc != VARNAM_SUCCESS) {
        printf("initialization failed - %s\n", msg);
        return 1;
    }

    rc = varnam_set_scheme_details(handle, "ml", "ml-unicode", "Malayalam", "Navaneeth K N", "May 3 2012");
    if (rc != VARNAM_SUCCESS)
    {
        printf ("set scheme details failed - %s\n", varnam_get_last_error(handle));
        return 1;
    }

    if (strcmp(varnam_get_scheme_language_code(handle), "ml") != 0)
    {
        printf("Failed to read lang code\n");
        return 1;
    }

    if (strcmp(varnam_get_scheme_identifier(handle), "ml-unicode") != 0)
    {
        printf("Failed to read lang identifier\n");
        return 1;
    }

    if (strcmp(varnam_get_scheme_display_name(handle), "Malayalam") != 0)
    {
        printf("Failed to read display name\n");
        return 1;
    }

    if (strcmp(varnam_get_scheme_author(handle), "Navaneeth K N") != 0)
    {
        printf("Failed to read author\n");
        return 1;
    }

    if (strcmp(varnam_get_scheme_compiled_date(handle), "May 3 2012") != 0)
    {
        printf("Failed to read compiled date\n");
        return 1;
    }


    return VARNAM_SUCCESS;
}

int normal_init(char **argv)
{
    int rc;
    char *msg;
    varnam *handle;

    rc = varnam_init(argv[0], &handle, &msg);
    if(rc != VARNAM_SUCCESS) {
        printf("initialization failed - %s\n", msg);
        return 1;
    }

    if (handle->internal->config_use_dead_consonants != 1)
    {
        printf("varnam_init() should have turned on use dead consonant option");
        return 1;
    }

    rc = varnam_config(handle, VARNAM_CONFIG_USE_DEAD_CONSONANTS, 0);

    if (handle->internal->config_use_dead_consonants != 0)
    {
        printf("varnam_config() is not changing value of use_dead_consonant option");
        return 1;
    }

    return VARNAM_SUCCESS;
}

int initialize_on_writeprotected_location()
{
    int rc;
    char *msg;
    varnam *handle;

    const char *filename = "/etc/varnam-test.vst";
    rc = varnam_init(filename, &handle, &msg);

    if (rc != VARNAM_STORAGE_ERROR)
    {
        printf("VARNAM_STORAGE_ERROR expected. Never got");
        return 1;
    }

    return VARNAM_SUCCESS;
}

int initialize_on_incorrect_location()
{
    int rc;
    char *msg;
    varnam *handle;

    const char *filename = "invalid-dir/varnam-test.vst";
    rc = varnam_init(filename, &handle, &msg);

    if (rc != VARNAM_STORAGE_ERROR)
    {
        printf("VARNAM_STORAGE_ERROR expected. Never got");
        return 1;
    }

    return VARNAM_SUCCESS;
}

int initialize_on_already_existing_file()
{
    int rc;
    char *msg;
    varnam *handle;

    const char *filename = "00-init.c";
    rc = varnam_init(filename, &handle, &msg);

    if (rc != VARNAM_STORAGE_ERROR)
    {
        printf("VARNAM_STORAGE_ERROR expected. Never got");
        return 1;
    }

    return VARNAM_SUCCESS;
}

int test_varnam_init(int argc, char **argv)
{
    int rc;

    if(argc == 0) {
        printf("no scheme file specified\n");
        return 1;
    }
    printf("%s\n", argv[0]);

    rc = strbuf_formatted_strings();
    if ( rc != VARNAM_SUCCESS )
    {
        printf("Formatted strings on strbuf is not working");
        return 1;
    }
 
    rc = normal_init(argv);
    if ( rc != VARNAM_SUCCESS )
    {
        printf("Normal initialization check failed");
        return 1;
    }

    rc = initialize_on_writeprotected_location();
    if ( rc != VARNAM_SUCCESS )
    {
        printf("Initialization on write protected location is allowed! This should not have happened");
        return 1;
    }

    rc = initialize_on_incorrect_location();
    if ( rc != VARNAM_SUCCESS )
    {
        printf("Initialization on incorrect location is allowed! This should not have happened");
        return 1;
    }

    rc = initialize_on_already_existing_file();
    if ( rc != VARNAM_SUCCESS )
    {
        printf("Initialization on already existing file should not have allowed");
        return 1;
    }

    rc = set_scheme_details();
    if (rc)
        return rc;

    return 0;
}
