/* ml_unicode.c

 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */


#include <string.h>

#include "renderers.h"
#include "../util.h"
#include "../vtypes.h"
#include "../result-codes.h"
#include "../symbol-table.h"

#define RENDER_VALUE2_TAG "render_value2"
#define CHIL_TAG "chill"

int
ml_unicode_renderer(varnam *handle,
                    vtoken *previous,
                    vtoken *current,
                    strbuf *output)
{
    int rc;
    vtoken *virama;
    bool removed;

    rc = vst_get_virama (handle, &virama);
    if (rc) return rc;

    if (previous != NULL && current->type == VARNAM_TOKEN_VOWEL && strcmp(current->pattern, "r") == 0)
    {
        strbuf_add (output, current->value3);
        return VARNAM_SUCCESS;
    }

    if (strcmp(current->tag, RENDER_VALUE2_TAG) == 0 && previous != NULL)
    {
#ifdef _VARNAM_VERBOSE
        varnam_debug (handle, "ml-unicode-renderer - Found %s tag", RENDER_VALUE2_TAG);
#endif
        strbuf_add(output, current->value2);
        return VARNAM_SUCCESS;
    }

    if (current->type == VARNAM_TOKEN_VOWEL && previous != NULL && strcmp(previous->tag, CHIL_TAG) == 0)
    {
        removed = strbuf_remove_from_last (output, previous->value1);
        if (!removed) {
            removed = strbuf_remove_from_last (output, previous->value2);
        }

        if (removed)
        {
            strbuf_add (output, previous->value3);
            strbuf_add (output, current->value2);
            return VARNAM_SUCCESS;
        }
    }

    return VARNAM_PARTIAL_RENDERING;
}

int
ml_unicode_rtl_renderer(varnam *handle,
                        vtoken *previous,
                        vtoken *current,
                        strbuf *output)
{
    if (strcmp(current->tag, CHIL_TAG) == 0) {
        strbuf_add (output, current->pattern);
        strbuf_add (output, "_");
        return VARNAM_SUCCESS;
    }

    return VARNAM_PARTIAL_RENDERING;
}
