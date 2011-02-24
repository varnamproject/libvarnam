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


int 
ml_unicode_renderer(varnam *handle, 
                    struct token *match, 
                    struct strbuf *output)
{
    if(strcmp(match->pattern, "r") == 0 || strcmp(match->pattern, "R") == 0) {
        if(handle->internal->last_token_available && !strbuf_endswith(output, handle->internal->virama)) {
            strbuf_add(output, "ര്‍");
            return VARNAM_SUCCESS;
        }
    }

    if(strcmp(match->tag, "nj") == 0) {
        if(handle->internal->last_token_available) {
            strbuf_add(output, match->value2);
            return VARNAM_SUCCESS;
        }
    }

    if(strcmp(match->tag, "ng") == 0) {
        if(handle->internal->last_token_available) {
            strbuf_add(output, match->value2);
            return VARNAM_SUCCESS;
        }
    }
   
    return VARNAM_PARTIAL_RENDERING;
}
