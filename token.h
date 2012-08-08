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


#ifndef VARNAM_TOKEN_H_INCLUDED_090112
#define VARNAM_TOKEN_H_INCLUDED_090112

#include "vtypes.h"
#include "varray.h"

struct token*
Token(int id, int type, int match_type, const char* pattern, const char* value1, const char* value2, const char* value3, const char* tag);

struct token*
get_pooled_token (
    varnam *handle,
    int id,
    int type,
    int match_type,
    const char* pattern,
    const char* value1,
    const char* value2,
    const char* value3,
    const char* tag);

varray* 
product_tokens(varnam *handle, varray *tokens);

#endif
