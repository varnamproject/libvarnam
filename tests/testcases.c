
#include "testcases.h"
#include <check.h>
#include <time.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "../varnam.h"
#include <string.h>

varnam *varnam_instance = NULL;

int
file_exist (const char *filename)
{
    struct stat buffer;
    return (stat (filename, &buffer) == 0);
}

char*
get_unique_filename()
{
    strbuf *filename = strbuf_init (25);

    for (;;)
    {
        strbuf_addf (filename, "output/%d.test.vst", rand());
        if (!file_exist (strbuf_to_s (filename))) {
            break;
        }
        else {
            strbuf_clear (filename);
        }
    }

    return strbuf_detach (filename);
}

void
assert_success (int value)
{
    strbuf *string = NULL;
    if (value != VARNAM_SUCCESS) {
        string = strbuf_init (50);
        strbuf_addf (string, "Expected VARNAM_SUCCESS, but got %d. %s", value, 
                varnam_get_last_error (varnam_instance));
        ck_abort_msg (strbuf_to_s (string));
    }
}

void 
assert_error (int value)
{
    strbuf *string = NULL;
    if (value != VARNAM_ERROR) {
        string = strbuf_init (50);
        strbuf_addf (string, "Expected VARNAM_ERROR, but got %d. %s", value, 
                varnam_get_last_error (varnam_instance));
        ck_abort_msg (strbuf_to_s (string));
    }
}

void
ensure_word_list_contains(varray *words, const char *word)
{
    int i = 0, found = 0;
    vword *w;
    strbuf *error;

    for (i = 0; i < varray_length (words); i++) {
        w = varray_get (words, i);
        if (strcmp (w->text, word) == 0) {
            found = 1;
            break;
        }
    }

    if (!found) {
        error = strbuf_init (50);
        strbuf_addf (error, "Expected word list to contain '%s'", word);
        ck_abort_msg (strbuf_to_s (error));
    }
}

void
reinitialize_varnam_instance(const char *filename)
{
    int rc;
    char *msg;
    varnam *handle;
    strbuf *error;

    if (varnam_instance != NULL) {
        varnam_destroy (varnam_instance);
        varnam_instance = NULL;
    }

    rc = varnam_init (filename, &handle, &msg);
    if(rc != VARNAM_SUCCESS) {
        error = strbuf_init (50);
        strbuf_addf (error, "Varnam initialization failed. %s. %s", filename, msg);
        ck_abort_msg (strbuf_to_s (error));
    }

    printf ("Using %s\n", filename);
    varnam_instance = handle;
}

void
setup()
{
    char *filename =  get_unique_filename();
    reinitialize_varnam_instance (filename);
    xfree (filename);
}

void
teardown()
{
/*    if (varnam_instance != NULL)
        varnam_destroy (varnam_instance);*/
    varnam_instance = NULL;
}
