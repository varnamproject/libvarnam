/* varnam-types.h
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

#ifndef VARNAMLIB_H_INCLUDED_103830
#define VARNAMLIB_H_INCLUDED_103830

#include "foreign/sqlite3.h"

#define VARNAM_SYMBOL_MAX           30
#define VARNAM_LIB_TEMP_BUFFER_SIZE 100

struct varnam_internal {
    sqlite3 *db;
    char *message;
};

typedef struct varnam {
    char *symbols_file;
    struct varnam_internal *internal;
} varnam;

enum token_type {
    VARNAM_TOKEN_VOWEL,
    VARNAM_TOKEN_CONSONANT,
    VARNAM_TOKEN_CONSONANT_CLUSTER,
    VARNAM_TOKEN_NUMBER,
    VARNAM_TOKEN_SYMBOL,
    VARNAM_TOKEN_OTHER
};

struct token {
    enum token_type type;
    char pattern[VARNAM_SYMBOL_MAX];
    char value1[VARNAM_SYMBOL_MAX];
    char value2[VARNAM_SYMBOL_MAX];
    int children;
};

#endif
