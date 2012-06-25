/* <file-name>
 *
 * Copyright (C) Navaneeth K N
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

#include "varnam-symbol-table.h"
#include "varnam-util.h"
#include "varnam-types.h"
#include "varnam-result-codes.h"
#include "varnam-api.h"

int
vwt_ensure_schema_exists(varnam *handle)
{
    const char *sql =
        "create         table if not exists metadata (key TEXT UNIQUE, value TEXT);"
        "create virtual table if not exists words using fts4(pattern text, word text, confidence integer, learned_on text);"
        "create virtual table if not exists words_substrings using fts4(word text);";

    char *zErrMsg = 0;
    int rc;

    rc = sqlite3_exec(v_->known_words, sql, NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
        set_last_error (handle, "Failed to initialize file for storing known words : %s", zErrMsg);
        sqlite3_free(zErrMsg);
        return VARNAM_ERROR;
    }

    return VARNAM_SUCCESS;
}

int
vwt_persist_possibilities(varnam *handle, varray *tokens)
{
    int i, j;
    varray *array;
    vtoken *token;
    
    for (i = 0; i < varray_length (tokens); i++)
    {
        array = varray_get (tokens, i);
        printf ("Persisting - ");
        for (j = 0; j < varray_length (array); j++)
        {
            token = varray_get (array, j);
            printf ("%s", token->pattern);
        }
        printf ("\n");
    }

    return VARNAM_SUCCESS;
}
