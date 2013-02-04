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

#include "deps/sqlite3.h"

#define VARNAM_SYMBOL_MAX           30
#define VARNAM_LIB_TEMP_BUFFER_SIZE 100
#define VARNAM_WORD_MAX             25

/* logging */
#define VARNAM_LOG_DEFAULT 1
#define VARNAM_LOG_DEBUG   2

/* pattern matching */
#define VARNAM_MATCH_EXACT       1
#define VARNAM_MATCH_POSSIBILITY 2
#define VARNAM_MATCH_ALL         3

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
#define VARNAM_TOKEN_NON_JOINER        11
#define VARNAM_TOKEN_JOINER            12

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
struct vpool_t;

struct varnam_internal
{
    /* file handles */
    sqlite3 *db;
    sqlite3 *known_words;
    char *message;

    struct varray_t *renderers;
    struct token *virama;
    struct strbuf *last_error;

    /* Logging related */
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

    /* instance pools */
    struct vpool_t *tokens_pool;
    struct vpool_t *arrays_pool;
    struct vpool_t *strings_pool;
    struct vpool_t *words_pool;

    struct varray_t *tokens;

    /* Prepared statements */
    sqlite3_stmt *tokenize_using_pattern;
    sqlite3_stmt *tokenize_using_value;
    sqlite3_stmt *tokenize_using_value_and_match_type;
    sqlite3_stmt *can_find_more_matches_using_pattern;
    sqlite3_stmt *can_find_more_matches_using_value;
    sqlite3_stmt *learn_word;
    sqlite3_stmt *learn_pattern;
    sqlite3_stmt *get_word;
    sqlite3_stmt *get_suggestions;
    sqlite3_stmt *get_best_match;
    sqlite3_stmt *get_matches_for_word;
    sqlite3_stmt *possible_to_find_matches;
    sqlite3_stmt *update_confidence;
    sqlite3_stmt *update_learned_flag;
    sqlite3_stmt *delete_pattern;
    sqlite3_stmt *delete_word;
};

typedef struct varnam {
    char *scheme_file;
    struct varnam_internal *internal;
} varnam;

typedef struct token {
    int id, type, match_type;
    char tag[VARNAM_SYMBOL_MAX];
    char pattern[VARNAM_SYMBOL_MAX];
    char value1[VARNAM_SYMBOL_MAX];
    char value2[VARNAM_SYMBOL_MAX];
    char value3[VARNAM_SYMBOL_MAX];
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

typedef struct varnam_token_rendering {
    const char *scheme_id;
    int (*tl)(varnam *handle, vtoken *previous, vtoken *current,  struct strbuf *output);
    int (*rtl)(varnam *handle, vtoken *previous, vtoken *current,  struct strbuf *output);
} vtoken_renderer;

typedef struct varnam_info_t {
    const char *scheme_file;
    int symbols;
    int words;

    int tokens_in_memory;
    int arrays_in_memory;
} vinfo;

typedef struct varnam_learn_status_t {
    int total_words;
    int failed;
} vlearn_status;

typedef struct varnam_word_t {
    const char *text;
    int confidence;
} vword;

#endif
