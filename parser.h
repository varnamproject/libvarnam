/* parser.h - implements parser to parse the s-expression
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


#ifndef PARSER_H_INCLUDED_052443
#define PARSER_H_INCLUDED_052443

#define PARSER_ERROR_MESSAGE_MAX 4000
#define PARSER_SYMBOL_MAX        30

#include "sexpr/sexp.h"

struct exp_list {
    sexp_t *exp;
    struct exp_list *next;
};

struct parser_error {
    char message[PARSER_ERROR_MESSAGE_MAX];
    struct exp_list *call_stack;
    struct parser_error *next;
};

enum token_type {
    PARSER_TOKEN_VOWEL,
    PARSER_TOKEN_CONSONANT,
    PARSER_TOKEN_CONSONANT_CLUSTER,
    PARSER_TOKEN_NUMBER,
    PARSER_TOKEN_SYMBOL,
    PARSER_TOKEN_OTHER
};

/* this will be the item in each trie node */
struct token {
    enum token_type type;
    char pattern[PARSER_SYMBOL_MAX];
    char value1[PARSER_SYMBOL_MAX];
    char value2[PARSER_SYMBOL_MAX];
    int children;
};

struct parser_result {
    struct parser_error *err;
    struct trie *result;
};

unsigned int parser_init(const char *filename);
struct parser_result *parser_parse();
void parser_destroy(struct parser_result *result);

#endif
