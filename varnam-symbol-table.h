/* varnam-symbol-table.h
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


#ifndef VARNAM_SYMBOL_TABLE_H_INCLUDED_120215
#define VARNAM_SYMBOL_TABLE_H_INCLUDED_120215

#include "varnam-types.h"

/*
** This function will try to get a token for the lookup text provided. 
** search will be done directly on the symbol table.
** A valid instance of token is returned upon successful execution. 
** NULL value indicates a failure to get the token
*/
struct token *get_token(varnam *handle, const char *lookup);

/*
** Does a search in the symbol table and retrns a boolean value indicating
** the possibility of finding a token for the lookup text and the last token found.
*/
int can_find_token(varnam *handle, struct token *last, const char *lookup);

#endif
