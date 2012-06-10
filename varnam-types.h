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
#define VARNAM_TOKEN_TAG_MAX        15
#define VARNAM_LIB_TEMP_BUFFER_SIZE 100
#define VARNAM_WORD_MAX             25

/* logging */
#define VARNAM_LOG_DEFAULT 1
#define VARNAM_LOG_DEBUG   2

/* pattern matching */
#define VARNAM_MATCH_EXACT       1
#define VARNAM_MATCH_POSSIBILITY 2

/* allowed runtime functions */
#define VARNAM_RULE_FN_INITIALS "if_initials"
#define VARNAM_RULE_FN_BEGINS_WITH "begins_with"

/* available type of tokens */
#define VARNAM_TOKEN_VOWEL             1
#define VARNAM_TOKEN_CONSONANT         2
#define VARNAM_TOKEN_DEAD_CONSONANT    3
#define VARNAM_TOKEN_CONSONANT_VOWEL   4
#define VARNAM_TOKEN_NUMBER            5
#define VARNAM_TOKEN_SYMBOL            6
#define VARNAM_TOKEN_ANUSVARA          7
#define VARNAM_TOKEN_VISARGA           8
#define VARNAM_TOKEN_VIRAMA            9
#define VARNAM_TOKEN_OTHER             10

/* configuration options */
#define VARNAM_CONFIG_USE_DEAD_CONSONANTS      100
#define VARNAM_CONFIG_IGNORE_DUPLICATE_TOKEN   101
#define VARNAM_CONFIG_ENABLE_SUGGESTIONS       102

/* Keys used in metadata*/
#define VARNAM_METADATA_SCHEME_LANGUAGE_CODE     "lang-code"
#define VARNAM_METADATA_SCHEME_IDENTIFIER        "scheme-id"
#define VARNAM_METADATA_SCHEME_DISPLAY_NAME      "scheme-display-name"
#define VARNAM_METADATA_SCHEME_AUTHOR            "scheme-author"
#define VARNAM_METADATA_SCHEME_COMPILED_DATE     "scheme-compiled-date"

#define VARNAM_TOKENIZER_PATTERN 200
#define VARNAM_TOKENIZER_VALUE   201

struct varnam_rule;
struct varnam_token_rendering;
struct strbuf;
struct token;
struct word;
struct vpool_t;

struct varnam_internal
{
    sqlite3 *db, *known_words;
    char *message;
    struct varnam_token_rendering *renderers;

    struct token *virama;

    struct strbuf *output;
    struct strbuf *rtl_output;
    struct strbuf *lookup;
    struct strbuf *last_error;

    struct token *current_rtl_token;

    struct token *last_token;
    struct token *last_rtl_token;

    int last_token_available;
    int last_rtl_token_available;

    int log_level;
    void (*log_callback)(const char*);
    struct strbuf *log_message;

    int vst_buffering;

    /* Buffers to cache scheme details */
    struct strbuf *scheme_language_code;
    struct strbuf *scheme_identifier;
    struct strbuf *scheme_display_name;
    struct strbuf *scheme_author;
    struct strbuf *scheme_compiled_date;

    /* configuration options */
    int config_use_dead_consonants;
    int config_ignore_duplicate_tokens;

    /* token pool */
    struct vpool_t *tokens_pool;
    struct vpool_t *tokens_array_pool;

    struct varray_t *tokens;

    sqlite3_stmt *tokenize_using_pattern;
    sqlite3_stmt *tokenize_using_value;
    sqlite3_stmt *can_find_more_matches;
};

typedef struct varnam {
    char *scheme_file;
    struct varnam_internal *internal;
} varnam;

typedef struct token {
    int type, match_type;
    char tag[VARNAM_TOKEN_TAG_MAX];
    char pattern[VARNAM_SYMBOL_MAX];
    char value1[VARNAM_SYMBOL_MAX];
    char value2[VARNAM_SYMBOL_MAX];
    int children;
    struct token* next;
} vtoken;

struct varnam_rule {
    char scheme_name[VARNAM_SYMBOL_MAX];
    char pattern[VARNAM_SYMBOL_MAX];
    char function[VARNAM_SYMBOL_MAX];
    char arg1[VARNAM_SYMBOL_MAX];
    char arg2[VARNAM_SYMBOL_MAX];
    char render_as[VARNAM_SYMBOL_MAX];
    int negate;
    struct varnam_rule *next;
};

struct varnam_token_rendering {
    const char *scheme_identifier;
    int (*render)(varnam *handle, struct token *match,  struct strbuf *output);
    int (*render_rtl)(varnam *handle, struct token *match,  struct strbuf *output);
};

#endif
