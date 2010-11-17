
/* lex.c - a lexical tokenizer
 *
 * Copyright (C) Navaneeth.K.N
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


#include <stdlib.h>
#include "util.h"
#include "sexpr/sexp.h"
#include "lex.h"
#include "varnam-result-codes.h"

#define PAGE_SIZE 4096
#define READ_ONLY_MODE "r"
#define SIZEOF_ELEMENT_TO_READ 1

struct lexmetadata {

    /* file descriptor */
    FILE *fd;            

    /* buffer that will hold data read from file */                     
    char buffer[PAGE_SIZE + 1];

    /* current status of the lexer */
    lex_statuscodes lex_status;   

    /* the continuation used by the s-expr library */
    pcont_t *continuation;

    /* event handlers used to form the partial expression */
    parser_event_handlers_t event_handlers;

    /* error message set by the lexer */
    const char *message;

    /* this will have the text seen by the lex. 
     * this will be cleared when we find a valid expression 
     */
    struct strbuf *partial_exp;               
};

static struct lexmetadata *data = NULL;

static void start_sexpr()
{
    strbuf_add(data->partial_exp, "(");
}

static void end_sexpr()
{
    strbuf_add(data->partial_exp, ")");
}

static void atom_found(const char *atom_data, size_t length, atom_t atom)
{
    switch(atom)
    {
    case SEXP_BASIC:
        strbuf_add(data->partial_exp, atom_data);
        break;
    case SEXP_SQUOTE:
        strbuf_add(data->partial_exp, "'");
        strbuf_add(data->partial_exp, atom_data);
        break;
    case SEXP_DQUOTE:
        strbuf_add(data->partial_exp, "\"");
        strbuf_add(data->partial_exp, atom_data);
        strbuf_add(data->partial_exp, "\"");
        break;
    default:
        /* should not have got here */
        strbuf_add(data->partial_exp, atom_data);
        break;
    }

    /** 
     * this may induce extra spaces, but we have no way to find out space was there on the 
     * real expression. 
     */
    strbuf_add(data->partial_exp, " ");
}

static void change_status(lex_statuscodes newstatus, const char *message)
{
    data->lex_status = newstatus;
    data->message = message;
}

static struct lexmetadata *create_metadata()
{
    data = (struct lexmetadata*) xmalloc(sizeof (struct lexmetadata));
    data->fd = NULL;
    data->buffer[0] = '\0';
    data->lex_status = LEX_OK;
    data->continuation = NULL;
    data->message = NULL;
    data->partial_exp = strbuf_init(4096);

    data->event_handlers.start_sexpr = start_sexpr;
    data->event_handlers.end_sexpr = end_sexpr;
    data->event_handlers.characters = atom_found;
    
    return data;
}

static size_t fill_buffer_from_disk()
{
    size_t elements_read = 0;

    assert(data->fd != NULL);
    
    elements_read = fread(data->buffer, SIZEOF_ELEMENT_TO_READ, PAGE_SIZE, data->fd);
    if(elements_read != PAGE_SIZE) {
        if(feof(data->fd)) {
            change_status(LEX_EOF, "");
        }
        else {
            change_status(LEX_ERRORED, "unable to read the input file");
            return VARNAM_ERROR;
        }
        fclose(data->fd);
        data->fd = NULL;
    }
    data->buffer[elements_read + 1] = '\0';
    return elements_read;
}

/*
 * will be called when iparse_sexp returns NULL. 
 * this will return boolean value indicating whether we can continue parsing
 */
static int handle_null_expression()
{    
    if(sexp_errno == SEXP_ERR_INCOMPLETE)
    {
        /**
         * if lexer is at EOF and we have nothing in partial expression string we consider it as a smooth run and
         * lexer is done with it's job
         */
        if(data->lex_status == LEX_EOF && strbuf_is_blank_string(data->partial_exp)) {
            change_status(LEX_DONE, "");
        }
        else if(data->lex_status == LEX_EOF) {
            /*
             * found an incomplete expression and we are at EOF! 
             * this is an error condition and not attempting for panic mode recovery.
             */
            change_status(LEX_ERRORED, "Invalid expression found!");
            return VARNAM_ERROR;
        }
        else {
            if(!fill_buffer_from_disk()) {
                return VARNAM_ERROR;    /* fill_buffer_from_disk sets the error parameter */
            }
        }
    }
    else if(sexp_errno == SEXP_ERR_BADFORM || sexp_errno == SEXP_ERR_BADCONTENT)
    {
        change_status(LEX_ERRORED, 
                      "Badly formed expression found. This can happen when you have misplaced paranthesis.");
        return VARNAM_ERROR;
    }    
    else
    {
        change_status(LEX_ERRORED, "Unknown error! unable to continue");
        return VARNAM_ERROR;
    }

    return VARNAM_SUCCESS;
}

lex_statuscodes lex_status()
{
    if(data == NULL) return LEX_OK;
    return data->lex_status;
}

const char* lex_message()
{
    if(data == NULL) return NULL;
    return data->message;
}

const char* lex_partial_expression()
{
    if(data == NULL) return NULL;
    return data->partial_exp->buffer;
}

int lex_init(const char *filename)
{
    data = create_metadata();
    data->fd = fopen(filename, READ_ONLY_MODE);
    if(data->fd == NULL) {
        change_status(LEX_ERRORED, "error opening input file");
        return VARNAM_ERROR;
    }
    return VARNAM_SUCCESS;
}

/* 
 * seek to next s-expression. Return NULL if operation is not success 
 * and sets the error variable. This function reads data from the file and keeps it in a buffer.
 * So there is no guarantee that each call to this function will lead into a disk read. If all data 
 * in the buffer is processed, there will be a disk read.
 *
 * Currently all lexical errors are considered as fatal. no attempt for panic mode recovery is performed.
 * on error, lexer enters into errored state and won't perform any operations further.
 *
 * TODO : use the event handlers provided by the s-exp library to keep track of expression that we saw. 
 * this will help in better error reporting
 */

sexp_t *lex_nextexp()
{
    sexp_t *sx = NULL;

    assert(data != NULL);

    if(data->lex_status == LEX_DONE || data->lex_status == LEX_ERRORED) return NULL;

    if(data->continuation == NULL) 
    {
        if(!fill_buffer_from_disk()) {
            return NULL;
        }
        data->continuation = init_continuation(data->buffer);
        data->continuation->event_handlers = &data->event_handlers;
    }

    for(; ;)
    {
        sx = iparse_sexp(data->buffer, PAGE_SIZE, data->continuation);
        if(sx == NULL)
        {
            if(!handle_null_expression() || data->lex_status == LEX_DONE) {
                break;
            }
        }
        else {
            strbuf_clear(data->partial_exp);
            break;
        }
    }
    
    return sx;
}

void lex_destroy_expression(sexp_t *expression)
{
    destroy_sexp(expression);
}

void lex_destroy()
{
    if(data != NULL) {
        strbuf_destroy(data->partial_exp);
        destroy_continuation(data->continuation);
        xfree(data);
    }
    sexp_cleanup();
}
