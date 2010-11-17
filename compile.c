/* compile.c
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
#include "util.h"
#include "sexpr/sexp.h"
#include "parser.h"
#include "trie.h"
#include "varnam.h"
#include "varnam-result-codes.h"

const char compile_usage[] = 
    "compile: varnam compile [--help] <scheme-file> [output_directory]\n";

extern int varnam_generate_symbols(const char *output_file_path, struct trie *result, char **error_message);

struct compile_parameters {
    const char *scheme_file;
    const char *output_directory;
};

static void exp_tostring(sexp_t *exp, struct strbuf *string)
{
    sexp_t *e;
    assert(exp);

    if(exp->ty == SEXP_VALUE)
    {
        strbuf_add(string, exp->val);
        strbuf_add(string, " ");
    }
    else
    {
        strbuf_add(string, " (");
        for(e = exp->list; e != NULL; e = e->next) {
            exp_tostring(e, string);
        }
        strbuf_add(string, ")");
    }
}

static void handle_error(struct parser_result *pr)
{
    unsigned int error_count = 0;
    struct parser_error *err;
    struct exp_list *expression;
    struct strbuf *string;

    string = strbuf_init(100);

    for(err = pr->err; err != NULL; err = err->next) {
        ++error_count;
        varnam_error(":: %d) %s", error_count, err->message);
        for(expression = err->call_stack; expression != NULL; expression = expression->next) {
            strbuf_clear(string);
            exp_tostring( expression->exp, string );
            varnam_error(":: in : %s", string->buffer);
        }  
        varnam_error("::");
    }

    strbuf_destroy(string);
    
    varnam_error(":: %d errors. Compilation aborted.", error_count);
}

static int compile_scheme_file(varnam *handle, const char *scheme_file, const char *output_directory)
{
    char output_file_path[MAX_PATH_LENGTH], *error_message = NULL;
    struct parser_result *pr = NULL;
    int status;

    struct path_info *pinfo = splitpath(scheme_file);
    const char *directory = output_directory ? output_directory :  pinfo->directory;
    const char *extension = ".vst";
    
    if(pinfo->filename == 0 || strcmp(pinfo->filename, "") == 0) {
        varnam_error("compile: invalid scheme file specified. aborting.");
        return VARNAM_ERROR;
    }

    strncpy(output_file_path, directory, MAX_PATH_LENGTH);
    strcat(output_file_path, pinfo->filename);
    strcat(output_file_path, extension);
    
    status = parser_init(scheme_file);
    if(status != VARNAM_SUCCESS) {
        varnam_error(":: Compilation failed for '%s'", scheme_file);
        xfree( pinfo );
        return status;
    }

    pr = parser_parse();
    if(pr->err != NULL) {
        handle_error( pr );
        xfree( pinfo );
        parser_destroy( pr );
        return VARNAM_ERROR;
    }

    status = varnam_generate_symbols(output_file_path, pr->result, &error_message);
    if(status != VARNAM_SUCCESS) {
        varnam_error(":: Compilation failed for %s", scheme_file);
        xfree( pinfo );
        parser_destroy( pr );
        return status;
    }

    varnam_info("::");
    varnam_info(":: Successfully compiled '%s'", scheme_file);
    varnam_info(":: Created %s", output_file_path);

    xfree( pinfo );
    parser_destroy( pr );

    return VARNAM_SUCCESS;
}

int varnam_compile(varnam *handle, const char *scheme_file, const char *output_directory)
{
    if(handle == NULL || scheme_file == NULL) 
        return VARNAM_MISUSE;

    return compile_scheme_file(handle, scheme_file, output_directory);
}


