/* lex.h - a lexical tokenizer
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

#ifndef LEX_H_INCLUDED_080329
#define LEX_H_INCLUDED_080329

#include "sexpr/sexp.h"

typedef enum {

    LEX_OK,
    LEX_EOF,
    LEX_DONE,
    LEX_ERRORED

} lex_statuscodes;

int lex_init(const char *filename);
sexp_t *lex_nextexp();
void lex_destroy_expression(sexp_t *expression);
void lex_destroy();
lex_statuscodes lex_status();
const char* lex_message();
const char* lex_partial_expression();

#endif
