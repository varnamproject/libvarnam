/* varnam-symbol-table.c
 *
 * Copyright (C) <year> <author>
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

#include "varnam-symbol-table.h"
#include "varnam-util.h"
#include "varnam-types.h"

struct token* 
find_token(varnam *handle, 
           const char *lookup)
{
    struct varnam_internal *internal;
    struct token *tok = NULL;
    char sql[500]; 
    const char *pattern, *value1, *value2, *type, *tag;
    sqlite3_stmt *stmt; sqlite3 *db;
    int rc;
    int has_children;
    
    assert( handle ); assert( lookup );

    internal = handle->internal;
    db = internal->db;

    snprintf( sql, 500, "select type, pattern, value1, value2, children, tag from symbols where pattern = ?1;");
    rc = sqlite3_prepare_v2( db, sql, 500, &stmt, NULL );
    if( rc == SQLITE_OK ) 
    {
        sqlite3_bind_text (stmt, 1, lookup, (int) strlen(lookup), NULL);
        rc = sqlite3_step (stmt);
        if( rc == SQLITE_ROW ) 
        {
            type = (const char*) sqlite3_column_text( stmt, 0 );
            pattern = (const char*) sqlite3_column_text( stmt, 1 );
            value1 = (const char*) sqlite3_column_text( stmt, 2 );
            value2 = (const char*) sqlite3_column_text( stmt, 3 );
            has_children = sqlite3_column_int( stmt, 4 );
            tag = (const char*) sqlite3_column_text( stmt, 5 );

            if(internal->current_token == NULL) {
                internal->current_token = (struct token *) xmalloc(sizeof (struct token));
                assert( internal->current_token );
            }

            tok = internal->current_token;
            strncpy( tok->type, type, VARNAM_TOKEN_TYPE_MAX);
            strncpy( tok->pattern, pattern, VARNAM_SYMBOL_MAX);
            strncpy( tok->value1, value1, VARNAM_SYMBOL_MAX);
            strncpy( tok->value2, value2, VARNAM_SYMBOL_MAX);
            strncpy( tok->tag, tag, VARNAM_TOKEN_TAG_MAX);
            tok->children = has_children;
        }
    }

    sqlite3_finalize( stmt );
    return tok;
}

struct token* 
find_rtl_token(varnam *handle, 
               const char *lookup)
{
    struct varnam_internal *internal;
    struct token *tok = NULL;
    char sql[500]; 
    const char *pattern, *value1, *value2, *type, *tag;
    sqlite3_stmt *stmt; sqlite3 *db;
    int rc;
    int has_children;
    
    assert( handle ); assert( lookup );

    internal = handle->internal;
    db = internal->db;

    snprintf( sql, 500, "select type, pattern, value1, value2, children, tag from symbols where value1 = ?1 or value2 = ?1 limit 1;");
    rc = sqlite3_prepare_v2( db, sql, 500, &stmt, NULL );
    if( rc == SQLITE_OK ) 
    {
        sqlite3_bind_text (stmt, 1, lookup, (int) strlen(lookup), NULL);
        rc = sqlite3_step (stmt);
        if( rc == SQLITE_ROW ) 
        {
            type = (const char*) sqlite3_column_text( stmt, 0 );
            pattern = (const char*) sqlite3_column_text( stmt, 1 );
            value1 = (const char*) sqlite3_column_text( stmt, 2 );
            value2 = (const char*) sqlite3_column_text( stmt, 3 );
            has_children = sqlite3_column_int( stmt, 4 );
            tag = (const char*) sqlite3_column_text( stmt, 5 );

            if(internal->current_rtl_token == NULL) {
                internal->current_rtl_token = (struct token *) xmalloc(sizeof (struct token));
                assert( internal->current_rtl_token );
            }

            tok = internal->current_rtl_token;
            strncpy( tok->type, type, VARNAM_TOKEN_TYPE_MAX);
            strncpy( tok->pattern, pattern, VARNAM_SYMBOL_MAX);
            strncpy( tok->value1, value1, VARNAM_SYMBOL_MAX);
            strncpy( tok->value2, value2, VARNAM_SYMBOL_MAX);
            strncpy( tok->tag, tag, VARNAM_TOKEN_TAG_MAX);
            tok->children = has_children;
        }
    }

    sqlite3_finalize( stmt );
    return tok;
}

int can_find_token(varnam *handle, 
                   struct token *last, 
                   const char *lookup)
{
    char sql[500];
    sqlite3_stmt *stmt;
    sqlite3 *db;
    int rc; int result = 0;

    assert( lookup );
    assert( handle );

    db = handle->internal->db;

    snprintf( sql, 500, "select count(pattern) as cnt from symbols where pattern like '%s%%';", lookup );
    rc = sqlite3_prepare_v2( db, sql, 500, &stmt, NULL );
    if( rc == SQLITE_OK ) {
        rc = sqlite3_step( stmt );
        if( rc == SQLITE_ROW ) {
            if( sqlite3_column_int( stmt, 0 ) > 0 ) {
                result = 1;
            }
        }
    }

    sqlite3_finalize( stmt );
    return result;
}

int can_find_rtl_token(varnam *handle, 
                       struct token *last, 
                       const char *lookup)
{
    char sql[500];
    sqlite3_stmt *stmt;
    sqlite3 *db;
    int rc; int result = 0;

    assert( lookup );
    assert( handle );

    db = handle->internal->db;

    snprintf( sql, 500, "select count(pattern) as cnt from symbols where value1 like '%s%%' or value2 like '%s%%';", lookup, lookup );
    rc = sqlite3_prepare_v2( db, sql, 500, &stmt, NULL );
    if( rc == SQLITE_OK ) {
        rc = sqlite3_step( stmt );
        if( rc == SQLITE_ROW ) {
            if( sqlite3_column_int( stmt, 0 ) > 0 ) {
                result = 1;
            }
        }
    }

    sqlite3_finalize( stmt );
    return result;
}

void fill_general_values(varnam *handle, 
                         char *output, 
                         const char *name)
{
    char sql[500];
    const char *result;
    sqlite3_stmt *stmt;
    sqlite3 *db;
    int rc; 

    assert( name );
    assert( handle );

    db = handle->internal->db;

    snprintf(sql, 500, "select value from general where key = ?1;");

    rc = sqlite3_prepare_v2( db, sql, 500, &stmt, NULL );
    if( rc == SQLITE_OK ) {
        sqlite3_bind_text(stmt, 1, name, (int) strlen(name), NULL);
        rc = sqlite3_step( stmt );
        if( rc == SQLITE_ROW ) 
        {
            result = (const char*) sqlite3_column_text(stmt, 0);
            if(result) {
                strncpy(output, result, VARNAM_SYMBOL_MAX);
            }
        }
    }

    sqlite3_finalize( stmt );
}
