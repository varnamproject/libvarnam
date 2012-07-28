/* vword.h - Functions that operates on a word
 *
 * Copyright (C) Navaneeth K.N
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


#ifndef VWORD_H_INCLUDED_0631
#define VWORD_H_INCLUDED_0631

#include "vtypes.h"

/*
 * Constructor for vword
 */
vword*
Word(const char *text, int confidence);

/*
 * Gets a pooled instance of vword. If no free instances are available, one will be created
 */
vword*
get_pooled_word(varnam *handle, const char *text, int confidence);


#endif
