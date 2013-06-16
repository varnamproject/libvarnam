/* UTF8 related functions
 *
 * Copyright (c) 2005 JSON.org
 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */



#ifndef VUTF8_H_INCLUDED_111220
#define VUTF8_H_INCLUDED_111220

#define UTF8_END   -1
#define UTF8_ERROR -2

typedef struct utf8_decoder_t
{
    int the_index;
    int the_length;
    int the_char;
    int the_byte;
    char* the_input;
} utf8_decoder;

extern int  utf8_decode_at_byte(utf8_decoder *decoder);
extern int  utf8_decode_at_character(utf8_decoder *decoder);
extern void utf8_decode_init(char p[], int length, utf8_decoder *decoder);
extern int  utf8_decode_next(utf8_decoder *decoder);


#endif
