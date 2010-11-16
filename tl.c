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

#include "util.h"
#include "parser.h"
#include "foreign/sqlite3.h"
#include <string.h>

const char tl_usage[] = 
    "transliterate: varnam tl <scheme-file> <input-text>\n";

static struct token *get_token(sqlite3 *db, const char *lookup)
{
    struct token *tok = NULL;
    char sql[500]; const char *pattern, *value1;
    sqlite3_stmt *stmt;
    int rc; 
    int has_children;

    assert( db ); assert( lookup );

    snprintf( sql, 500, "select * from symbols where pattern = '%s';", lookup );
    rc = sqlite3_prepare_v2( db, sql, 500, &stmt, NULL );
    if( rc == SQLITE_OK ) 
    {
        rc = sqlite3_step( stmt );
        if( rc == SQLITE_ROW ) 
        {
            pattern = (const char *) sqlite3_column_text( stmt, 1 );
            value1 = (const char *) sqlite3_column_text( stmt, 2 );
            has_children = sqlite3_column_int( stmt, 4 );

            tok = (struct token *) xmalloc(sizeof (struct token));
            assert( tok );
            strncpy( tok->pattern, pattern, PARSER_SYMBOL_MAX);
            strncpy( tok->value1, value1, PARSER_SYMBOL_MAX);
            tok->children = has_children;
        }
    }

    sqlite3_finalize( stmt );
    return tok;
}

static int can_find_solution(sqlite3 *db, struct token *last, const char *lookup)
{
    char sql[500];
    sqlite3_stmt *stmt;
    int rc; int result = VARNAM_ERROR;

    assert( lookup );

    if( last == NULL ) {
        snprintf( sql, 500, "select count(pattern) as cnt from symbols where pattern like '%s%%';", lookup );
    }
    else {
        snprintf( sql, 500, "select count(pattern) as cnt from symbols where pattern = '%s' and pattern like '%s%%';", last->pattern, lookup );
    }

    rc = sqlite3_prepare_v2( db, sql, 500, &stmt, NULL );
    if( rc == SQLITE_OK ) {
        rc = sqlite3_step( stmt );
        if( rc == SQLITE_ROW ) {
            if( sqlite3_column_int( stmt, 0 ) > 0 ) {
                result = VARNAM_OK;
            }
        }
    }

    sqlite3_finalize( stmt );
    return result;
}

static int tokenize(sqlite3 *db, 
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

        temp = get_token( db, lookup->buffer );
        if( temp ) {
            last = temp;
            matchpos = counter;
            if( last->children <= 0 ) break;
        }
        else if( can_find_solution( db, last, lookup->buffer ) == VARNAM_ERROR ) { 
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
        return tokenize( db, remaining, string );

    return VARNAM_OK;
}

struct strbuf *transliterate(const char *scheme_file, const char *input) 
{
    sqlite3 *db;
    int rc;
    struct strbuf *string;

    rc = sqlite3_open( scheme_file, &db );
    if( rc ) {
        varnam_error("Can't open %s: %s\n", scheme_file, sqlite3_errmsg(db));
        sqlite3_close(db);
        return VARNAM_ERROR;
    }

    string = strbuf_init( 20 );    
    if( tokenize( db, input, string ) != VARNAM_OK ) {
        strbuf_destroy( string );
        string = NULL;
    }

    sqlite3_close( db );
    return string;
}

int transliterate_input(int argc, char **argv)
{
    char *scheme_file;
    char *input;
    struct strbuf *output;

    if(argc == 0) {
        varnam_info( "transliterate: invalid usage" );
        varnam_info( tl_usage );
        return 1;
    }
    else if(argc == 1) {
        varnam_info( "transliterate: no input text found" );
        varnam_info( tl_usage );
        return 1;
    }

    scheme_file = argv[0];
    input = argv[1];
    output = transliterate( scheme_file, input );
    if( output == NULL ) {
        varnam_error( "transliteration failed! ");
        return 1;
    }
    varnam_info("%s", output->buffer);
    strbuf_destroy(output);
    return 0;
}
