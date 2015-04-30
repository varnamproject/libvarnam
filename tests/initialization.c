/*
 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */


#include <check.h>
#include "testcases.h"
#include "../varnam.h"

START_TEST (get_corpus_details)
{
	int rc;
	char *msg, *filename;
	varnam *handle;
	vcorpus_details *details;

	rc = varnam_init_from_id ("ml", &handle, &msg);
  assert_success (rc);

  filename = get_unique_filename();
  rc = varnam_config (handle, VARNAM_CONFIG_ENABLE_SUGGESTIONS, filename);
  assert_success (rc);

	rc = varnam_get_corpus_details (handle, &details);
  assert_success (rc);
	ck_assert_int_eq (details->wordsCount, 0);

	/* learn some word and corpus details should reflect the change */
	rc = varnam_learn (handle, "മലയാളം");
	assert_success (rc);

	rc = varnam_get_corpus_details (handle, &details);
  assert_success (rc);
	ck_assert_int_gt (details->wordsCount, 0);

	varnam_destroy (handle);
}
END_TEST

START_TEST (set_scheme_details)
{
    int rc;
    char *msg, *filename;
    varnam *handle;
		vscheme_details scheme_details;
		vscheme_details *loadedSchemeDetails;

    filename = get_unique_filename();
    rc = varnam_init (filename, &handle, &msg);
    assert_success (rc);

		scheme_details.langCode = "ml";
		scheme_details.identifier = "ml-unicode";
		scheme_details.displayName = "Malayalam";
		scheme_details.author = "Navaneeth K N";
		scheme_details.compiledDate = "May 3 2012";
		scheme_details.isStable = 1;
    rc = varnam_set_scheme_details(handle, &scheme_details);
    assert_success (rc);

		rc = varnam_get_scheme_details(handle, &loadedSchemeDetails);
    assert_success (rc);

		ck_assert_str_eq (loadedSchemeDetails->langCode, "ml");
		ck_assert_str_eq (loadedSchemeDetails->identifier, "ml-unicode");
		ck_assert_str_eq (loadedSchemeDetails->displayName, "Malayalam");
		ck_assert_str_eq (loadedSchemeDetails->author, "Navaneeth K N");
		ck_assert_str_eq (loadedSchemeDetails->compiledDate, "May 3 2012");
		ck_assert_int_eq (loadedSchemeDetails->isStable, 1);

    varnam_destroy (handle);
    free (filename);
}
END_TEST

START_TEST (enable_suggestions)
{
    int rc;
    char *msg, *filename;
    varnam *handle;

    filename = get_unique_filename();
    rc = varnam_init(filename, &handle, &msg);
    assert_success (rc);

    rc = varnam_config (handle, VARNAM_CONFIG_ENABLE_SUGGESTIONS, "output/00-suggestions");
    assert_success (rc);

    varnam_destroy (handle);
    free (filename);
}
END_TEST

START_TEST (normal_init)
{
    int rc;
    char *msg, *filename;
    varnam *handle;

    filename = get_unique_filename();
    rc = varnam_init(filename, &handle, &msg);
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

    varnam_destroy (handle);
    free (filename);
}
END_TEST

START_TEST (initialize_using_lang_code)
{
  int rc;
  char *errMsg = NULL;
  varnam *handle;
  strbuf *tmp;

  rc = varnam_init_from_id ("ml", &handle, &errMsg);
  if (errMsg != NULL) {
    printf ("init_from_lang failed: %s\n", errMsg);
  }

  assert_success (rc);
  ck_assert_str_eq ("/usr/local/share/varnam/vst/ml.vst", varnam_get_scheme_file (handle));
  tmp = strbuf_init (10);
  strbuf_addf (tmp, "%s/.local/share/varnam/suggestions/ml.vst.learnings", getenv ("HOME"));
  ck_assert_str_eq (strbuf_to_s (tmp), varnam_get_suggestions_file (handle));

  strbuf_destroy (tmp);
  varnam_destroy (handle);
}
END_TEST

START_TEST (initialize_using_invalid_lang_code)
{
  int rc;
  char *errMsg = NULL;
  varnam *handle;
  rc = varnam_init_from_id ("mll", &handle, &errMsg);
  assert_error (rc);
  ck_assert (errMsg != NULL);
  varnam_destroy (handle);
}
END_TEST

START_TEST (initialize_on_writeprotected_location)
{
    int rc;
    char *msg;
    varnam *handle;

    const char *filename = "/etc/varnam-test.vst";
    rc = varnam_init (filename, &handle, &msg);
    if (rc != VARNAM_STORAGE_ERROR)
    {
        ck_abort_msg ("VARNAM_STORAGE_ERROR expected. Never got");
    }

    free (msg);
    varnam_destroy (handle);
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

    free (msg);
    varnam_destroy (handle);
}
END_TEST

START_TEST (initialize_on_already_existing_file)
{
    int rc;
    char *msg = NULL;
    varnam *handle = NULL;

    const char *filename = "initialization.c";
    rc = varnam_init(filename, &handle, &msg);
    if (rc != VARNAM_STORAGE_ERROR)
    {
        ck_abort_msg ("VARNAM_STORAGE_ERROR expected. Never got");
    }

    free (msg);
    varnam_destroy (handle);
}
END_TEST

START_TEST (init_destroy_loop_memory_stress_test)
{
    int rc, i;
    char* msg;
    varnam* handle;
    varray* output;

    for (i = 0; i < 100; i++) {
        rc = varnam_init ("../schemes/ml.vst", &handle, &msg);
        ck_assert_int_eq (rc, VARNAM_SUCCESS);
        rc = varnam_transliterate(handle, "navaneeth", &output);
        ck_assert_int_eq (rc, VARNAM_SUCCESS);
        varnam_destroy (handle);
    }
}
END_TEST

TCase* get_initialization_tests()
{
    TCase* tcase = tcase_create("initialization");
    tcase_add_test (tcase, get_corpus_details);
    tcase_add_test (tcase, set_scheme_details);
    tcase_add_test (tcase, enable_suggestions);
    tcase_add_test (tcase, normal_init);
    tcase_add_test (tcase, initialize_using_lang_code);
    tcase_add_test (tcase, initialize_using_invalid_lang_code);
    tcase_add_test (tcase, initialize_on_writeprotected_location);
    tcase_add_test (tcase, initialize_on_incorrect_location);
    tcase_add_test (tcase, initialize_on_already_existing_file);
    tcase_add_test (tcase, init_destroy_loop_memory_stress_test);
    return tcase;
}
