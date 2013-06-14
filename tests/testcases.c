
#include "testcases.h"
#include <check.h>
#include <time.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "../varnam.h"
#include <string.h>

varnam *varnam_instance = NULL;

static int
file_exist (const char *filename)
{
    struct stat buffer;
    return (stat (filename, &buffer) == 0);
}

strbuf*
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

    return filename;
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
setup()
{
    int rc;
    char *msg;
    varnam *handle;
    strbuf *filename;

    filename = get_unique_filename();

    rc = varnam_init(strbuf_to_s (filename), &handle, &msg);
    if(rc != VARNAM_SUCCESS) {
        printf ("%s\n", msg);
        ck_abort_msg ("Varnam initialization failed");
    }

    printf ("Using %s\n", strbuf_to_s (filename));

   varnam_instance = handle;
   strbuf_destroy (filename);
}

void
teardown()
{
    varnam_instance = NULL;
}
