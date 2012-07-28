/* ml_unicode.c

Copyright (C) 2010 Navaneeth.K.N

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <string.h>

#include "renderers.h"
#include "../varnam-util.h"
#include "../varnam-types.h"
#include "../varnam-result-codes.h"
#include "../varnam-symbol-table.h"

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

    rc = vst_get_virama (handle, &virama);
    if (rc) return rc;

    /* Not sure about this */
    /* if(strcmp(match->pattern, "r") == 0 || strcmp(match->pattern, "R") == 0) { */
    /*     if(handle->internal->last_token_available && !strbuf_endswith(output, virama->value1)) { */
    /*         strbuf_add(output, "ര്‍"); */
    /*         return VARNAM_SUCCESS; */
    /*     } */
    /* } */

    if (strcmp(current->tag, RENDER_VALUE2_TAG) == 0 && previous != NULL)
    {
        strbuf_add(output, current->value2);
        return VARNAM_SUCCESS;
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
