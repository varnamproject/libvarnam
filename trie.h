
/* trie.h - A simple implementation of a suffic tree
 *
 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */


#ifndef TRIE_H_INCLUDED_095519
#define TRIE_H_INCLUDED_095519

#include "util.h"

struct trie {
    int root; /* if 1, then this is root */
    char label[MAX_PATTERN_LENGTH];
    void *value;
    struct trie *child;
    struct trie *next;
};

/* Creates a new trie and return the root node */
VARNAM_EXPORT struct trie *trie_create();

/* Creates a new trie structure and add it to the children list of parent. */
/* Newly created trie will be returned. This should be freed using "free_trie" function.  */
VARNAM_EXPORT struct trie *trie_add_child(struct trie *parent, const char *label, void  *value);

typedef int (*itfunction)(struct trie* t, unsigned int depth, void *userdata);
VARNAM_EXPORT int trie_iterate(struct trie *t, itfunction function, void *userdata);

/* iterates children and returns the count */
VARNAM_EXPORT unsigned int trie_children_count(struct trie *root);

/* free the memory allocated for the supplied trie and  */
/* returns the number of elements freed */
typedef void (*freefunction)(void *userdata);
VARNAM_EXPORT unsigned int trie_free(struct trie *root, freefunction callback);

/* looks for the supplied label in the trie. returns the value
 * associated if found. else returns NULL
 */
VARNAM_EXPORT void *trie_lookup(struct trie *root, const char *label);

#endif
