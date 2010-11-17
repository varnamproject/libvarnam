/* varnam.h
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


#ifndef VARNAM_H_INCLUDED_091620
#define VARNAM_H_INCLUDED_091620

#include "foreign/sqlite3.h"
#include "util.h"
#include <stdlib.h>

struct varnam_internal {
    sqlite3 *db;
};

typedef struct varnam {
    char *symbols_file;
    struct varnam_internal *internal;
} varnam;

int varnam_init(const char *symbols_file, size_t file_length, varnam **handle, char **msg);

int varnam_compile(varnam *handle, const char *scheme_file, const char *output_directory);

int varnam_transliterate(varnam *handle, const char *input, struct strbuf *output);

#endif
