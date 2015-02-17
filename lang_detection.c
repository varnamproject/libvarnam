/* Simple language detector
 *
 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */


#include "langcodes.h"
#include "vutf8.h"
#include "vtypes.h"
#include "util.h"

#define NON_JOINER 0x200C
#define JOINER     0x200D

typedef struct codepoint_range {
    int start;
    int end;
    int language_code;
} cpr;

static cpr ranges[] = {
    {0x0900, 0x097F, VARNAM_LANG_CODE_HI},
    {0x0980, 0x09FF, VARNAM_LANG_CODE_BN},
    {0x0A80, 0x0AFF, VARNAM_LANG_CODE_GU},
    {0x0B00, 0x0B7F, VARNAM_LANG_CODE_OR},
    {0x0B80, 0x0BFF, VARNAM_LANG_CODE_TA},
    {0x0C00, 0x0C7F, VARNAM_LANG_CODE_TE},
    {0x0C80, 0x0CFF, VARNAM_LANG_CODE_KN},
    {0x0D00, 0x0D7F, VARNAM_LANG_CODE_ML},
    {0x0A00, 0x0A7F, VARNAM_LANG_CODE_PA},
    {0x0900, 0x097F, VARNAM_LANG_CODE_MR},
};

static int
get_language(int codepoint)
{
    int i;
    cpr range;
    
    for (i = 0; i < ARRAY_SIZE(ranges); i++)
    {
        range = ranges[i];
        if (codepoint >= range.start && codepoint <= range.end)
            return range.language_code;
    }

    return VARNAM_LANG_CODE_UNKNOWN;
}

static bool
should_skip (int codepoint)
{
    switch (codepoint) {
    case JOINER:
    case NON_JOINER:
        return true;
    };
    return false;
}

int
varnam_detect_lang(varnam *handle, const char *input)
{
    strbuf *word;
    utf8_decoder decoder;
    int codepoint, language = VARNAM_LANG_CODE_UNKNOWN, prev_language = 0;
    
    if (handle == NULL || input == NULL) {
        return VARNAM_LANG_CODE_UNKNOWN;
    }

    word = get_pooled_string (handle);
    strbuf_add (word, input);

    if (strbuf_is_blank (word)) {
        return VARNAM_LANG_CODE_UNKNOWN;
    }

    utf8_decode_init (word->buffer, (int) word->length, &decoder);

    for (;;)
    {
        codepoint = utf8_decode_next (&decoder);
        if (codepoint == UTF8_END || codepoint == UTF8_ERROR)
            break;

        if (should_skip(codepoint))
            continue;

        language = get_language (codepoint);

        if (language == VARNAM_LANG_CODE_UNKNOWN)
            return VARNAM_LANG_CODE_UNKNOWN;
        
        if (prev_language != 0 && language != prev_language) {
            /* Looks like characters from multiple languages are mixed */
            return VARNAM_LANG_CODE_UNKNOWN;
        }
        prev_language = language;
    }

    return language;
}
