/* Simple language detector
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
    {0x0D00, 0x0D7F, VARNAM_LANG_CODE_ML}
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
