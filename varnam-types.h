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
#define VARNAM_TOKEN_TYPE_MAX       5
#define VARNAM_LIB_TEMP_BUFFER_SIZE 100

/* allowed runtime functions */
#define VARNAM_RULE_FN_INITIALS "if_initials"

/* available type of tokens */
#define VARNAM_TOKEN_VOWEL                "vo"
#define VARNAM_TOKEN_CONSONANT            "co"
#define VARNAM_TOKEN_DEAD_CONSONANT       "dco"
#define VARNAM_TOKEN_CONSONANT_CLUSTER    "cc"
#define VARNAM_TOKEN_NUMBER               "nu"  
#define VARNAM_TOKEN_SYMBOL               "sy"
#define VARNAM_TOKEN_OTHER                "ot"

struct varnam_internal {
    sqlite3 *db;
    char *message;
    char virama[VARNAM_SYMBOL_MAX];
};

typedef struct varnam {
    char *symbols_file;
    struct varnam_internal *internal;
} varnam;

struct token {
    char type[VARNAM_TOKEN_TYPE_MAX];
    char pattern[VARNAM_SYMBOL_MAX];
    char value1[VARNAM_SYMBOL_MAX];
    char value2[VARNAM_SYMBOL_MAX];
    int children;
};

struct varnam_rule {
    char pattern[VARNAM_SYMBOL_MAX];
    char function[VARNAM_SYMBOL_MAX];
    char arg1[VARNAM_SYMBOL_MAX];
    char arg2[VARNAM_SYMBOL_MAX];
    char render_as[VARNAM_SYMBOL_MAX];
    int negate;
    struct varnam_rule *next;
};

#endif
