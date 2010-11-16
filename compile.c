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
#include "module.h"
#include "sexpr/sexp.h"
#include "parser.h"
#include "trie.h"

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

static int compile(struct compile_parameters *params)
{
    char output_file_path[MAX_PATH_LENGTH], *error_message = NULL;
    struct parser_result *pr = NULL;
    int status;

    struct path_info *pinfo = splitpath(params->scheme_file);
    const char *directory = params->output_directory ? params->output_directory :  pinfo->directory;
    const char *extension = ".vst";
    
    if(pinfo->filename == 0 || strcmp(pinfo->filename, "") == 0) {
        varnam_error("compile: invalid scheme file specified. aborting.");
        varnam_info(compile_usage);
        return VARNAM_ERROR;
    }

    strncpy(output_file_path, directory, MAX_PATH_LENGTH);
    strcat(output_file_path, pinfo->filename);
    strcat(output_file_path, extension);
    
    varnam_info(":: Varnam Scheme File Compiler");
    varnam_info(":: Copyright (C) Navaneeth.K.N");
    varnam_info("::");
    varnam_info(":: Compiling scheme file :  %s", params->scheme_file);
    varnam_info(":: Output will be written to : %s", output_file_path);
    varnam_info("::");

    if(parser_init(params->scheme_file) != VARNAM_OK) {
        varnam_error(":: Compilation failed for '%s'", params->scheme_file);
        xfree( pinfo );
        return VARNAM_ERROR;
    }

    pr = parser_parse();
    if(pr->err != NULL) {
        handle_error( pr );
        xfree( pinfo );
        parser_destroy( pr );
        return VARNAM_ERROR;
    }

    status = varnam_generate_symbols(output_file_path, pr->result, &error_message);
    if(status != VARNAM_OK) {
        varnam_error(":: Compilation failed for %s", params->scheme_file);
        xfree( pinfo );
        parser_destroy( pr );
        return VARNAM_ERROR;
    }

    varnam_info("::");
    varnam_info(":: Successfully compiled '%s'", params->scheme_file);
    varnam_info(":: Created %s", output_file_path);

    xfree( pinfo );
    parser_destroy( pr );

    return VARNAM_OK;
}

int compile_scheme_file(int argc, char **argv)
{
    struct compile_parameters params;
    if(argc == 0) {
        varnam_error(":: compile: expected scheme file but found none.\n%s", compile_usage);
        return 1;
    }
    else if(strcmp(argv[0], "--help") == 0) {
        varnam_info(compile_usage);
        return 1;
    }
    
    params.scheme_file = argv[0];
    params.output_directory = 0;
    if(argc >= 2) {
        params.output_directory = argv[1];
    }
    return compile(&params) == VARNAM_OK ? 0 : 1;
}
