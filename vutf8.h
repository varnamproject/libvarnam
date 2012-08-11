/* UTF8 related functions
 *
 * Copyright (c) 2005 JSON.org
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
