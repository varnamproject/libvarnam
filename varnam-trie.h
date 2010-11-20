
/* trie.h - A simple implementation of a suffic tree
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

#ifndef TRIE_H_INCLUDED_095519
#define TRIE_H_INCLUDED_095519

#include "varnam-util.h"

struct trie {
    int root; /* if 1, then this is root */
    char label[MAX_PATTERN_LENGTH];
    void *value;
    struct trie *child;
    struct trie *next;
};

/* Creates a new trie and return the root node */
struct trie *trie_create();

/* Creates a new trie structure and add it to the children list of parent. */
/* Newly created trie will be returned. This should be freed using "free_trie" function.  */
struct trie *trie_add_child(struct trie *parent, const char *label, void  *value);

typedef int (*itfunction)(struct trie* t, unsigned int depth, void *userdata);
int trie_iterate(struct trie *t, itfunction function, void *userdata);

/* iterates children and returns the count */
unsigned int trie_children_count(struct trie *root);

/* free the memory allocated for the supplied trie and  */
/* returns the number of elements freed */
typedef void (*freefunction)(void *userdata);
unsigned int trie_free(struct trie *root, freefunction callback);

/* looks for the supplied label in the trie. returns the value
 * associated if found. else returns NULL
 */
void *trie_lookup(struct trie *root, const char *label);

#endif
