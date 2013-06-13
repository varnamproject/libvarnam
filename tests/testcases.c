
#include "testcases.h"
#include <check.h>
#include <time.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "../varnam.h"

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
    varnam_destroy (varnam_instance);
    varnam_instance = NULL;
}
