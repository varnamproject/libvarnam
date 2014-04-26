/* vword.h - Functions that operates on a word
 *
 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */



#ifndef VWORD_H_INCLUDED_0631
#define VWORD_H_INCLUDED_0631

#include "vtypes.h"
#include "util.h"

/*
 * Constructor for vword
 */
vword*
Word(varnam *handle, const char *text, int confidence);

/*
 * Gets a pooled instance of vword. If no free instances are available, one will be created
 */
vword*
get_pooled_word(varnam *handle, const char *text, int confidence);

bool
word_equals (void *left, void *right);

void
destroy_word(void *word);

#endif
