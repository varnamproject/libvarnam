
#include <check.h>
#include "testcases.h"
#include "../varnam.h"

START_TEST (set_scheme_details)
{
    int rc;
    char *msg;
    varnam *handle;

    rc = varnam_init (strbuf_to_s (get_unique_filename()), &handle, &msg);
    assert_success (rc);

    rc = varnam_set_scheme_details(handle, "ml", "ml-unicode", "Malayalam", "Navaneeth K N", "May 3 2012");
    assert_success (rc);

    ck_assert_str_eq (varnam_get_scheme_language_code (handle), "ml");
    ck_assert_str_eq (varnam_get_scheme_identifier (handle), "ml-unicode");
    ck_assert_str_eq (varnam_get_scheme_display_name (handle), "Malayalam");
    ck_assert_str_eq (varnam_get_scheme_author (handle), "Navaneeth K N");
    ck_assert_str_eq (varnam_get_scheme_compiled_date (handle), "May 3 2012");
}
END_TEST

START_TEST (enable_suggestions)
{
    int rc;
    char *msg;
    varnam *handle;

    rc = varnam_init(strbuf_to_s (get_unique_filename()), &handle, &msg);
    assert_success (rc);

    rc = varnam_config (handle, VARNAM_CONFIG_ENABLE_SUGGESTIONS, "output/00-suggestions");
    assert_success (rc);
}
END_TEST

START_TEST (normal_init)
{
    int rc;
    char *msg;
    varnam *handle;

    rc = varnam_init (strbuf_to_s (get_unique_filename()), &handle, &msg);
    assert_success (rc);

    if (handle->internal->config_use_dead_consonants != 0)
    {
        ck_abort_msg ("varnam_init() should have turned off use dead consonant option");
    }

    rc = varnam_config (handle, VARNAM_CONFIG_USE_DEAD_CONSONANTS, 0);
    assert_success (rc);

    if (handle->internal->config_use_dead_consonants != 0)
    {
        ck_abort_msg ("varnam_config() is not changing value of use_dead_consonant option");
    }
}
END_TEST

START_TEST (initialize_on_writeprotected_location)
{
    int rc;
    char *msg;
    varnam *handle;

    const char *filename = "/etc/varnam-test.vst";
    rc = varnam_init(filename, &handle, &msg);
    if (rc != VARNAM_STORAGE_ERROR)
    {
        ck_abort_msg ("VARNAM_STORAGE_ERROR expected. Never got");
    }
}
END_TEST

START_TEST (initialize_on_incorrect_location)
{
    int rc;
    char *msg;
    varnam *handle;

    const char *filename = "invalid-dir/varnam-test.vst";
    rc = varnam_init(filename, &handle, &msg);
    if (rc != VARNAM_STORAGE_ERROR)
    {
        ck_abort_msg ("VARNAM_STORAGE_ERROR expected. Never got");
    }
}
END_TEST

START_TEST (initialize_on_already_existing_file)
{
    int rc;
    char *msg;
    varnam *handle;

    const char *filename = "initialization.c";
    rc = varnam_init(filename, &handle, &msg);
    if (rc != VARNAM_STORAGE_ERROR)
    {
        ck_abort_msg ("VARNAM_STORAGE_ERROR expected. Never got");
    }
}
END_TEST

TCase* get_initialization_tests()
{
    TCase* tcase = tcase_create("initialization");
    tcase_add_test (tcase, set_scheme_details);
    tcase_add_test (tcase, enable_suggestions);
    tcase_add_test (tcase, normal_init);
    tcase_add_test (tcase, initialize_on_writeprotected_location);
    tcase_add_test (tcase, initialize_on_incorrect_location);
    tcase_add_test (tcase, initialize_on_already_existing_file);
    return tcase;
}
