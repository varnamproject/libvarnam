/* tl.c - implementation of transliterator
 *
 * Copyright (C) 2010 Navaneeth.K.N
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

#include <string.h>
#include <assert.h>

#include "varnam-api.h"
#include "foreign/sqlite3.h"
#include "varnam-util.h"
#include "varnam-types.h"
#include "varnam-result-codes.h"
#include "varnam-symbol-table.h"


static int tokenize(varnam *handle, 
                    const char *input, 
                    struct strbuf *string)
{
    const char *text,  *remaining;
    struct token *last = NULL, *temp = NULL;
    int matchpos = 0, counter = 0;
   
    struct strbuf *lookup;
    lookup = strbuf_init( 100 );

    text = input;
    while( *text != '\0' ) 
    {
        strbuf_addc( lookup, *text );
        ++counter;

        temp = get_token( handle, lookup->buffer );
        if( temp ) {
            last = temp;
            matchpos = counter;
            if( last->children <= 0 ) break;
        }
        else if( !can_find_token( handle, last, lookup->buffer )) { 
            break;
        }
        ++text;
    }

    if( last != NULL ) {
        strbuf_add( string, last->value1 );
        remaining = input + matchpos;
    }
    else {
        strbuf_addc( string, lookup->buffer[0] );
        remaining = input + 1;
    }

    strbuf_destroy( lookup );

    if( strlen( remaining ) > 0 )
        return tokenize( handle, remaining, string );

    return VARNAM_SUCCESS;
}

int varnam_transliterate(varnam *handle, 
                         const char *input, 
                         char **output)
{
    int rc;
    struct strbuf *result;
    
    if(handle == NULL || input == NULL || output == NULL)
        return VARNAM_MISUSE;

    result = strbuf_init(20);
    if(!result) {
        return VARNAM_MEMORY_ERROR;
    }
    rc = tokenize( handle, input, result );
    *output = strbuf_detach(result);

    return rc;
}

