/* varnam.c

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

#include "varnam-api.h"
#include "varnam-types.h"
#include "varnam-util.h"
#include "varnam-result-codes.h"
#include "varnam-symbol-table.h"
#include "rendering/renderers.h"

static struct varnam_token_rendering renderers[] = {
    { "ml-unicode", ml_unicode_renderer, ml_unicode_rtl_renderer }
};

static struct varnam_internal*
initialize_internal() 
{
    struct varnam_internal *vi;
    vi = (struct varnam_internal *) xmalloc(sizeof (struct varnam_internal));
    if(vi) {
        vi->virama[0] = '\0';
        vi->scheme_identifier[0] = '\0';
        vi->scheme_display_name[0] = '\0';
        vi->scheme_author[0] = '\0';
        vi->last_token_available = 0;
        vi->last_rtl_token_available = 0;
        vi->last_token = NULL;
        vi->last_rtl_token = NULL;
        vi->current_token = NULL;
        vi->current_rtl_token = NULL;
        vi->output = strbuf_init(100);
        vi->rtl_output = strbuf_init(100);
        vi->lookup = strbuf_init(10);
    }
    return vi;
}

int 
varnam_init(const char *symbols_file, size_t file_length, varnam **handle, char **msg)
{
    int rc;
    varnam *c;
    struct varnam_internal *vi;

    if(symbols_file == NULL || file_length <= 0)
        return VARNAM_MISUSE;

    c = (varnam *) xmalloc(sizeof (varnam));
    if(!c) 
        return VARNAM_MEMORY_ERROR;

    vi = initialize_internal();
    if(!vi)
        return VARNAM_MEMORY_ERROR;

    vi->message = (char *) xmalloc(sizeof (char) * VARNAM_LIB_TEMP_BUFFER_SIZE);
    if(!vi->message)
        return VARNAM_MEMORY_ERROR;
   
    rc = sqlite3_open(symbols_file, &vi->db);
    if( rc ) {
        asprintf(msg, "Can't open %s: %s\n", symbols_file, sqlite3_errmsg(vi->db));
        sqlite3_close(vi->db);
        return VARNAM_ERROR;
    }

    c->symbols_file = (char *) xmalloc(file_length + 1);
    if(!c->symbols_file)
        return VARNAM_MEMORY_ERROR;

    strncpy(c->symbols_file, symbols_file, file_length + 1);
    vi->renderers = renderers;
    c->internal = vi;
    
    *handle = c;
    return VARNAM_SUCCESS;
}

const char*
varnam_scheme_identifier(varnam *handle)
{
    if(handle->internal->scheme_identifier[0] == '\0') {
        fill_general_values(handle, handle->internal->scheme_identifier, "scheme_identifier");
    }

    return handle->internal->scheme_identifier;
}

const char*
varnam_scheme_display_name(varnam *handle)
{
    if(handle->internal->scheme_display_name[0] == '\0') {
        fill_general_values(handle, handle->internal->scheme_display_name, "scheme_display_name");
    }

    return handle->internal->scheme_display_name;
}

const char*
varnam_scheme_author(varnam *handle)
{
    if(handle->internal->scheme_author[0] == '\0') {
        fill_general_values(handle, handle->internal->scheme_author, "scheme_author");
    }

    return handle->internal->scheme_author;
}

int 
varnam_destroy(varnam *handle)
{
    struct varnam_internal *vi;
    int rc;

    if(handle == NULL) return VARNAM_SUCCESS;

    vi = handle->internal;

    xfree(vi->message);
    strbuf_destroy(vi->output);
    strbuf_destroy(vi->rtl_output);
    strbuf_destroy(vi->lookup);
    xfree(vi->last_token);
    xfree(vi->current_token);
    xfree(vi->last_rtl_token);
    xfree(vi->current_rtl_token);
    rc = sqlite3_close(handle->internal->db);
    if (rc != SQLITE_OK) {
        return VARNAM_ERROR;
    }
    xfree(handle->internal);
    xfree(handle->symbols_file);
    xfree(handle);
    return VARNAM_SUCCESS;
}

