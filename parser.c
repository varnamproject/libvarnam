/* parser.c - implements parser to parse the s-expression
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

#include "util.h"
#include "lex.h"
#include "trie.h"
#include "sexpr/sexp.h"
#include "parser.h"
#include <stdlib.h>
#include <string.h>

#define PARSER_HASH_SIZE         101
#define PARSER_STACK_SIZE        1024

#define PARSER_SET_FN "set"
#define PARSER_CO_FN "co"
#define PARSER_CC_FN "cc"
#define PARSER_NU_FN "nu"
#define PARSER_VO_FN "vo"
#define PARSER_SY_FN "sy"
#define PARSER_OT_FN "ot"

enum symbol_type {
    SYMBOL_FUNCTION,
    SYMBOL_VARIABLE
};

struct execution_context {
    /* argument stack which is shared across functions. */
    /* const char *arg_stack[PARSER_STACK_SIZE]; */
    const char **arg_stack;

    /* 0 indexed stack pointer */
    int sp;

    /** 
     * eval_exp() adds each expression it evaluates to this list. If the evaluation is successful,
     * it will remove the entry from here. This is used to report the expression trace when 
     * evaluation fails.
     **/
    struct exp_list *call_stack;

    /* contains the error message written by the evaluation routines */
    char *error_message;

    struct trie *result;
};

/**
 * Represents a symbol. This will be the value in 
 * symbol table
 **/
struct symbol {
    enum symbol_type type;

    /* symbol name. this is used to identify the symbol */
    char name[PARSER_SYMBOL_MAX];

    /* for variables, this field holds the value */
    char value[PARSER_SYMBOL_MAX];

    /* true if this symbol is in reserved category */
    unsigned int reserved;

    /* for SYMBOL_FUNCTION, this holds the pointer to the function to be executed */
    /* it takes a context and number of arguments found. this function will be responsible for argument count matching */
    const char* (*function)(struct execution_context*, unsigned int);

    /* for reserved symbols, this field contains the routine that can process the command */
    int (*handler)(struct execution_context *context, sexp_t *sexp, struct symbol *s);

    struct symbol *next;
};

/* symbol table */
struct symbol *st[PARSER_HASH_SIZE];

static void push_arg(struct execution_context *context, const char *item)
{
    assert(context != NULL);
    if(context->sp == PARSER_STACK_SIZE - 1) {
        varnam_error("Error: Stack overflow!");
        exit(100); /* TODO: standarize this error code */
        return;
    }
    context->arg_stack[++context->sp] = item;
}

static const char *pop_arg(struct execution_context *context)
{
    assert(context != NULL);
    assert(context->sp >= 0);
    return context->arg_stack[context->sp--];
}

static const char *top_arg(struct execution_context *context)
{
    assert(context != NULL);
    if(context->sp < 0) return NULL;
    return context->arg_stack[context->sp];
}

static void free_token(void *tok)
{
    assert( tok );
    xfree( tok );
}

static void push_exp(struct execution_context *context, sexp_t *exp)
{
    struct exp_list *item = NULL;

    item = (struct exp_list *) xmalloc (sizeof (struct exp_list));
    assert(item);

    item->exp = exp;
    item->next = context->call_stack;
    context->call_stack = item;
}

static void pop_exp(struct execution_context *context)
{
    struct exp_list *head = NULL;
    assert(context->call_stack != NULL);

    if(context->call_stack->next == NULL) {
        xfree( context->call_stack );
        context->call_stack = NULL;
    }
    else {
        head = context->call_stack;
        context->call_stack = context->call_stack->next;
        xfree( head );
    }
}

/**
 * given a null terminated string, returns the hash value of it.
 * this will be used as the key for table lookup.
 **/
static unsigned int hash(const char *s)
{
    unsigned int hashval;
    assert(s);
    for (hashval = 0; *s != '\0'; s++)
        hashval = (unsigned char) *s + 31 * hashval;
    return hashval % PARSER_HASH_SIZE;
}

static struct symbol *lookup(const char *s)
{
    struct symbol *sym;
    assert(s);
    for (sym = st[hash(s)]; sym != NULL; sym = sym->next)
        if (strcmp(s, sym->name) == 0)
            return sym;
    return NULL;
}

static int install_symbol(struct symbol *sym)
{
    const char *name;
    struct symbol *temp;
    unsigned int hashval;

    assert(sym != NULL);
    assert(sym->name != NULL);

    name = sym->name;

    if ((temp = lookup(name)) == NULL) { 
        hashval = hash(name);
        sym->next = st[hashval];
        st[hashval] = sym;
    } 
    else {
        return VARNAM_ERROR;
    }
    return VARNAM_OK;
}

static struct symbol *make_symbol(const char *name, 
                                  const char *value, 
                                  enum symbol_type type, 
                                  unsigned int reserved)
{
    struct symbol *s;
    s = (struct symbol *) xmalloc(sizeof (struct symbol));
    assert(s);

    strncpy(s->name, name, PARSER_SYMBOL_MAX);
    strncpy(s->value, value, PARSER_SYMBOL_MAX);
    s->type = type;
    s->reserved = reserved;
    s->function = NULL;
    s->handler = NULL;
    s->next = NULL;
    return s;
}

static int argcount_ok(struct execution_context *context, 
                       unsigned int expected, 
                       unsigned int actual, 
                       const char *function_name)
{
    if(expected != actual) {
        snprintf(context->error_message,
                 PARSER_ERROR_MESSAGE_MAX,
                 "'%s' expects %d arguments but found %d", function_name, expected, actual);
        return VARNAM_ERROR;
    }
    return VARNAM_OK;
}

static struct token *create_token(const char *pattern, 
                                  const char *value1, 
                                  const char *value2, 
                                  enum token_type type)

{   
    struct token *tok = NULL;
    assert(pattern); assert(value1); assert(value2);

    tok = (struct token *) xmalloc(sizeof (struct token));
    assert(tok);

    tok->type = type;
    strncpy(tok->pattern, pattern, PARSER_SYMBOL_MAX);
    strncpy(tok->value1, value1, PARSER_SYMBOL_MAX);
    strncpy(tok->value2, value2, PARSER_SYMBOL_MAX);

    return tok;
}

static int pattern_valid(struct execution_context *context, 
                         const char *pattern, 
                         const char *function_name)
{
   if(strlen (pattern) <= 0) {
        snprintf(context->error_message,
                 PARSER_ERROR_MESSAGE_MAX,
                 "Empty patterns are not allowed in function '%s", function_name);
        return VARNAM_ERROR;
   }
   return VARNAM_OK;
}

static int value_valid(struct execution_context *context, 
                         const char *value, 
                         const char *function_name)
{
   if(strlen (value) <= 0) {
        snprintf(context->error_message,
                 PARSER_ERROR_MESSAGE_MAX,
                 "Empty values are not allowed in function '%s", function_name);
        return VARNAM_ERROR;
   }
   return VARNAM_OK;
}

static int symbol_length_ok(struct execution_context *context, const char *sym)
{
    if(sym == NULL) return VARNAM_OK;

    if(strlen(sym) >= PARSER_SYMBOL_MAX) {
        snprintf(context->error_message,
                 PARSER_ERROR_MESSAGE_MAX,
                 "'%s' exceeds allowed symbol size %d", sym, PARSER_SYMBOL_MAX);
        return VARNAM_ERROR;
    }
    return VARNAM_OK;
}

static const char *builtin_set(struct execution_context *context, unsigned int argcount)
{
    const char *name;
    const char *value;
    struct symbol *s;

    if(!argcount_ok (context, 2, argcount, PARSER_SET_FN)) {
        return NULL;
    }

    assert(context->sp >= 1);
    value = pop_arg (context);
    name = pop_arg (context);

    if(!symbol_length_ok( context, name ) || !symbol_length_ok( context, value )) {
        return NULL;
    }

    s = lookup(name);
    if(s == NULL) { 
        s = make_symbol(name, value, SYMBOL_VARIABLE, 0);
        install_symbol(s);
    }
    else if(s->reserved) {
        snprintf(context->error_message,
                 PARSER_ERROR_MESSAGE_MAX,
                 "'%s' is a reserved word and can't be used as a variable name", name);
        return NULL;
    }
    else {
        strncpy(s->value, value, PARSER_SYMBOL_MAX);
    }

    return value;
}

/* creates a vowel */
static const char *builtin_vo(struct execution_context *context, unsigned int argcount)
{
    const char *pattern = NULL, *value1 = NULL, *value2 = NULL;
    struct token *tok = NULL;

    /* this function allows 2 or 3 arguments */
    if(!argcount_ok (context, 3, argcount, PARSER_VO_FN) && 
       !argcount_ok (context, 2, argcount, PARSER_VO_FN)) {

       snprintf(context->error_message,
                 PARSER_ERROR_MESSAGE_MAX,
                 "'vo' expects 2 or 3 arguments but found %d", argcount);
        return NULL;
    }

    if(argcount == 3) {
        assert(context->sp >= 2);
        value2 = pop_arg (context);
    }
    assert(context->sp >= 1);
    value1 = pop_arg (context);
    pattern = pop_arg (context);

    if(!symbol_length_ok( context, pattern ) || !symbol_length_ok( context, value1 ) || !symbol_length_ok( context, value2 )) {
        return NULL;
    }

    if(!pattern_valid(context, pattern, PARSER_VO_FN)) {
        return NULL;
    }
    else if(!value_valid(context, value1, PARSER_VO_FN)) {
        return NULL;
    }
    else if(argcount == 3 && !value_valid(context, value2, PARSER_VO_FN)) {
        return NULL;
    }

    tok = create_token(pattern, value1, value2 == NULL ? "" : value2, PARSER_TOKEN_VOWEL);
    trie_add_child(context->result, pattern, tok);

    return value1;
}

/* create a consonant */
static const char *builtin_co(struct execution_context *context, unsigned int argcount)
{
    const char *pattern = NULL, *value = NULL;
    struct token *tok = NULL;

    if(!argcount_ok (context, 2, argcount, PARSER_CO_FN)) {
        return NULL;
    }

    assert(context->sp >= 1);
    value = pop_arg (context);
    pattern = pop_arg (context);

    if(!symbol_length_ok( context, pattern ) || !symbol_length_ok( context, value )) {
        return NULL;
    }

    if(!pattern_valid(context, pattern, PARSER_CO_FN)) {
        return NULL;
    }
    else if(!value_valid(context, value, PARSER_CO_FN)) {
        return NULL;
    }

    tok = create_token(pattern, value, "", PARSER_TOKEN_CONSONANT);
    trie_add_child(context->result, pattern, tok);

    return value;
}

/* create a consonant cluster */
static const char *builtin_cc(struct execution_context *context, unsigned int argcount)
{
    const char *pattern = NULL, *value = NULL;
    struct token *tok = NULL;

    if(!argcount_ok (context, 2, argcount, PARSER_CC_FN)) {
        return NULL;
    }

    assert(context->sp >= 1);
    value = pop_arg (context);
    pattern = pop_arg (context);

    if(!symbol_length_ok( context, pattern ) || !symbol_length_ok( context, value )) {
        return NULL;
    }

    if(!pattern_valid(context, pattern, PARSER_CC_FN)) {
        return NULL;
    }
    else if(!value_valid(context, value, PARSER_CC_FN)) {
        return NULL;
    }

    tok = create_token(pattern, value, "", PARSER_TOKEN_CONSONANT_CLUSTER);
    trie_add_child(context->result, pattern, tok);

    return value;
}

/* create a number */
static const char *builtin_nu(struct execution_context *context, unsigned int argcount)
{
    const char *pattern = NULL, *value = NULL;
    struct token *tok = NULL;

    if(!argcount_ok (context, 2, argcount, PARSER_NU_FN)) {
        return NULL;
    }

    assert(context->sp >= 1);
    value = pop_arg (context);
    pattern = pop_arg (context);

    if(!symbol_length_ok( context, pattern ) || !symbol_length_ok( context, value )) {
        return NULL;
    }

    if(!pattern_valid(context, pattern, PARSER_NU_FN)) {
        return NULL;
    }
    else if(!value_valid(context, value, PARSER_NU_FN)) {
        return NULL;
    }

    tok = create_token(pattern, value, "", PARSER_TOKEN_NUMBER);
    trie_add_child(context->result, pattern, tok);

    return value;
}

/* create a symbol */
static const char *builtin_sy(struct execution_context *context, unsigned int argcount)
{
    const char *pattern = NULL, *value = NULL;
    struct token *tok = NULL;

    if(!argcount_ok (context, 2, argcount, PARSER_SY_FN)) {
        return NULL;
    }

    assert(context->sp >= 1);
    value = pop_arg (context);
    pattern = pop_arg (context);

    if(!symbol_length_ok( context, pattern ) || !symbol_length_ok( context, value )) {
        return NULL;
    }

    if(!pattern_valid(context, pattern, PARSER_SY_FN)) {
        return NULL;
    }
    else if(!value_valid(context, value, PARSER_SY_FN)) {
        return NULL;
    }

    tok = create_token(pattern, value, "", PARSER_TOKEN_SYMBOL);
    trie_add_child(context->result, pattern, tok);

    return value;
}

/* create a symbol imn 'other' category*/
static const char *builtin_ot(struct execution_context *context, unsigned int argcount)
{
    const char *pattern = NULL, *value = NULL;
    struct token *tok = NULL;

    if(!argcount_ok (context, 2, argcount, PARSER_OT_FN)) {
        return NULL;
    }

    assert(context->sp >= 1);
    value = pop_arg (context);
    pattern = pop_arg (context);

    if(!symbol_length_ok( context, pattern ) || !symbol_length_ok( context, value )) {
        return NULL;
    }

    if(!pattern_valid(context, pattern, PARSER_OT_FN)) {
        return NULL;
    }
    else if(!value_valid(context, value, PARSER_OT_FN)) {
        return NULL;
    }

    tok = create_token(pattern, value, "", PARSER_TOKEN_OTHER);
    trie_add_child(context->result, pattern, tok);

    return value;
}

static int eval_exp_recursive(struct execution_context *context, sexp_t *exp)
{
    struct symbol *s;
    int result = 0;

    assert(exp != NULL); assert(context != NULL);
    
    push_exp(context, exp);

    if(exp->ty == SEXP_VALUE) {
        snprintf(context->error_message,
                 PARSER_ERROR_MESSAGE_MAX,
                 "'%s' is not a valid expression. A valid expression will look like (%s)", exp->val, exp->val);
        return VARNAM_ERROR;
    }

    assert(exp->list != NULL);

    /**
     * expressions that are incorrectly nested like ((exp))
     */
    if(exp->list->ty != SEXP_VALUE) {
        snprintf(context->error_message,
                 PARSER_ERROR_MESSAGE_MAX,
                 "Incorrect nested expression detected. Can't infer the executable command");
        return VARNAM_ERROR;
    }

    assert(exp->list->val != NULL);

    s = lookup(exp->list->val);
    if(s == NULL) {
        snprintf(context->error_message,
                 PARSER_ERROR_MESSAGE_MAX,
                 "'%s' is not a valid command", exp->list->val);
        return VARNAM_ERROR;
    }

    result = s->handler(context, exp, s);
    if(result != VARNAM_OK) {
        return VARNAM_ERROR;
    }

   /**
    * that was a successful execution! removing the expression in the stack as it doesn't 
    * make sense to keep it there
    **/
    pop_exp( context );
    return VARNAM_OK;
}
                         
static int eval_function(struct execution_context *context, 
                         sexp_t *sexp, 
                         struct symbol *s)
{
    sexp_t *exp;
    struct symbol *sym;
    unsigned int argcount = 0;    
    const char *result;

    assert(s != NULL);
    assert(s->type == SYMBOL_FUNCTION);
    assert(s->name != NULL);
    assert(sexp->ty == SEXP_LIST);
    assert(sexp->list->ty == SEXP_VALUE);

    argcount = 0;

    /**
     * reads each atom from the expression and pushes it to the stack. 
     * if it finds nested expression, ask eval_exp() to process it.
     **/
    for(exp = sexp->list->next; exp != NULL; exp = exp->next) 
    {
        if(exp->ty == SEXP_VALUE) 
        {
            switch(exp->aty) 
            {
            case SEXP_BASIC:
                /* we saw an identifier. need to validate this against the known symbols */
                if(( sym = lookup(exp->val) ) == NULL) {
                    snprintf(context->error_message,
                             PARSER_ERROR_MESSAGE_MAX,
                             "'%s' is an unknown identifier", exp->val);
                    return VARNAM_ERROR;
                }
                else if(sym->type != SYMBOL_VARIABLE) {
                    snprintf(context->error_message,
                             PARSER_ERROR_MESSAGE_MAX,
                             "'%s' is invalid in this context. Expected a variable or literal", exp->val);
                    return VARNAM_ERROR;

                }
                else {
                    push_arg(context, sym->value);
                    ++argcount;
                }                
                break;
            case SEXP_DQUOTE:
                /* this is a double quoted string literal. pushing it as it is */
                push_arg(context, exp->val);
                ++argcount;
                break;
            case SEXP_SQUOTE:
                /* single quoted values are invalid */
                snprintf(context->error_message,
                         PARSER_ERROR_MESSAGE_MAX,
                         "'%s' is an invalid literal. String literals has to be specified within double quotes", exp->val);
                return VARNAM_ERROR;
            case SEXP_BINARY:                
                snprintf(context->error_message,
                         PARSER_ERROR_MESSAGE_MAX,
                         "Binary value found which is invalid");
                return VARNAM_ERROR;
            }
        }
        else
        {
            /* seeing a nested expression. evaluate it and push the result into the stack */
            ++argcount;
            if(eval_exp_recursive(context, exp) == VARNAM_ERROR) {
                /* a nested expression failed. so this function evaluation also failed */
                return VARNAM_ERROR;
            }
        }
    }

    /**
     * if we reach here, it means that all the nested expressions are evaluated and we are good to execute the actual
     * command. argument values are available in the stack. 
     **/
    result = s->function(context, argcount);

    if(result != NULL) {
        if(top_arg(context) != NULL) {
            /**
             * stack has more data. so this will be a nested function call. need to push the result of last
             * function call so that the caller can use it.
             **/
            push_arg(context, result);
        }
        return VARNAM_OK;
    }
    else {
        return VARNAM_ERROR;
    }
}

/**
 * evaluates the supplied expression. Errors are writtern to the error list 
 */
static void eval_exp(struct parser_result *res, sexp_t *exp)
{
    struct execution_context context;
    struct parser_error *err = NULL, *temp = NULL;
    const char *arg_stack[PARSER_STACK_SIZE];
    char error_message[PARSER_ERROR_MESSAGE_MAX];
    int status = 0;

    assert(exp != NULL);
    assert(res != NULL);

    arg_stack[0] = "";
    error_message[0] = 0;

    context.arg_stack = arg_stack;
    context.sp = 0;
    context.call_stack = NULL;
    context.error_message = error_message;
    context.result = res->result;

    status = eval_exp_recursive(&context, exp);
    if(status != VARNAM_OK) {
        err = (struct parser_error *) xmalloc(sizeof (struct parser_error));
        assert(err);

        strcpy(err->message, context.error_message);
        err->call_stack = context.call_stack;
        err->next = NULL;
        if(res->err == NULL) {
            res->err = err;
        }
        else {
            temp = res->err;
            while (temp->next != NULL) temp = temp->next;
            temp->next = err;
        }
    }
    else {
        destroy_sexp( exp );
    }
}

struct parser_result *parser_parse()
{
    sexp_t *exp = NULL;
    struct parser_result *result;
    lex_statuscodes lexstatus;

    result = (struct parser_result *) xmalloc(sizeof (struct parser_result));
    assert(result);

    result->err = NULL;
    result->result = trie_create();
    assert(result->result);

    for(; ;) {
        exp = lex_nextexp();
        if(exp == NULL) {
            lexstatus = lex_status();
            if(lex_status() == LEX_DONE) {
                break;
            }
            else if(lex_status() == LEX_ERRORED) {
                /* lex failed. this will stop the compilation as lexical errors here are non-recoverable */
                varnam_error("Lex error : %s", lex_message());
                break; /* TODO: print the message properly */
            }
            break;
        }
        eval_exp(result, exp);
    }

    return result;
}

unsigned int parser_init(const char *filename)
{
    struct symbol *set, *vo, *co, *cc, *nu, *sy, *ot;
    int ls;

    ls = lex_init(filename);
    if(ls != VARNAM_OK) {
        varnam_error("%s\n", lex_message());
        return VARNAM_ERROR;
    }

    memset(&st[0], 0, sizeof(st));

    /**
     * default entries into the symbol table
     **/
    set = make_symbol("set", "", SYMBOL_FUNCTION, 1);
    set->function = &builtin_set;
    set->handler = &eval_function;
    install_symbol(set);

    vo = make_symbol("vo", "", SYMBOL_FUNCTION, 1);
    vo->function = &builtin_vo;
    vo->handler = &eval_function;
    install_symbol(vo);

    co = make_symbol("co", "", SYMBOL_FUNCTION, 1);
    co->function = &builtin_co;
    co->handler = &eval_function;
    install_symbol(co);

    cc = make_symbol("cc", "", SYMBOL_FUNCTION, 1);
    cc->function = &builtin_cc;
    cc->handler = &eval_function;
    install_symbol(cc);

    nu = make_symbol("nu", "", SYMBOL_FUNCTION, 1);
    nu->function = &builtin_nu;
    nu->handler = &eval_function;
    install_symbol(nu);

    sy = make_symbol("sy", "", SYMBOL_FUNCTION, 1);
    sy->function = &builtin_sy;
    sy->handler = &eval_function;
    install_symbol(sy);

    ot = make_symbol("ot", "", SYMBOL_FUNCTION, 1);
    ot->function = &builtin_ot;
    ot->handler = &eval_function;
    install_symbol(ot);

    return VARNAM_OK;
}

void parser_destroy(struct parser_result *pr)
{
    int i;
    struct symbol *sym = NULL, *next_sym = NULL;
    struct parser_error *err = NULL, *next_err = NULL;
    struct exp_list *exps, *next_exps = NULL;

    /* cleaning the symbol table entries */
    for(i = 0; i < PARSER_HASH_SIZE; i++) {
        for(sym = st[i]; sym != NULL;) {
            next_sym = sym->next;
            xfree(sym);
            sym = next_sym;
        }
    }

    /* cleaning the error list */
    for(err = pr->err; err != NULL;) {
        next_err = err->next;
        for( exps = err->call_stack; exps != NULL; ) {
            next_exps = exps->next;
            if( exps->exp ) {
                destroy_sexp( exps->exp );
            }
            xfree( exps );
            exps = next_exps;
        }
        xfree( err );
        err = next_err;
    }

    trie_free( pr->result, free_token );
    lex_destroy();
    xfree( pr );
}
