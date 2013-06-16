
/* trie.c - simple implementation of a suffix tree
 *
 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */



#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "util.h"
#include "trie.h"
#include "result-codes.h"

static struct trie *trie_new(const char *label, void *value)
{
    struct trie *t = xmalloc(sizeof(struct trie));
    if(t) {
        strncpy(t->label, label, MAX_PATTERN_LENGTH);
        t->root = 0;
        t->value = value;
        t->child = NULL;
        t->next = NULL;
    }
    return t;
}

struct trie *trie_create()
{
    struct trie *t;
    t = trie_new( "", NULL );
    assert( t );
    t->root = 1;
    return t;
}

/* inserts the supplied child at the end of children collection */
static void insert_child(struct trie *parent, struct trie *child)
{
    assert(parent != NULL);
    assert(child != NULL);
    assert(child->next == NULL);

    if(parent->child == NULL) {
        parent->child = child;
    }
    else {
        /* inserting child at the end of the list */
        struct trie *t = parent->child;
        while(t->next != NULL) {
            t = t->next;
        }
        t->next = child;
    }
}

static void rearrange_siblings(struct trie *parent, struct trie *new)
{
    unsigned int first_item = 0;
    struct trie *item = NULL, *prev = NULL, *next = NULL;

    assert(parent != NULL);
    assert(parent->child != NULL);
    assert(new != NULL);

    item = parent->child;
    while(item != NULL) 
    {
        next = item->next; 
        first_item = (strcmp(item->label, parent->child->label) == 0);

        if(strcmp(item->label, new->label) != 0 && startswith(item->label, new->label)) 
        {
            /* making this item orphan and moving the orphaned node */
            item->next = NULL;
            if(first_item) {
                /* this is the first item. So the only item refering this is the parent.  */
                /* cutting this reference and repointing the parent to current item's next */
                parent->child = next;
            }
            else {
                /* to make a non first item orphan, repoint the previous item to next item of this */
                prev->next = next;
            }
            insert_child(new, item);
        }
        else {
            prev = item;
        }

        item = next;
    }
}

/* Search inside the immediate children of parent and returns */
/* the matching item. If no match found, parent will be returned. */
/**/
static struct trie *find_match(struct trie *parent, const char *lookup)
{
    struct trie *item = NULL;

    if(parent->child == NULL) {
        return parent;
    }
    
    item = parent->child;
    do {
        if(strcmp(lookup, item->label) == 0) {
            return item;
        }
        item = item->next;
    } while(item != NULL);

    return NULL;
}

/* iterate the tree begining from root and tries to find a node matches the label. */
/* matched item will be returned. If no match found, root will be returned. */
/* if the label already exist, NULL will be returned */
static struct trie *find_parent(struct trie *root, const char *label)
{
    struct trie *item = NULL, *temp = NULL;
    int charcount = 1;
    char buffer[MAX_PATTERN_LENGTH];
    const char *lbl;

    item = root;
    lbl = label;

    /* iterating each character in the label and incrementally looking the children */
    /* collection for the match */
    /**/
    while(*lbl != '\0') 
    {
        substr(buffer, label, 1, charcount++);
        temp = find_match(item, buffer);
        if(temp) {
            item = temp;
        }
        ++lbl;
    }

    if(strcmp(item->label, label) == 0) {
        /* this means item already exist */
        return NULL;
    }
    return item;
}

/* add a new item to the children collection of parent. newly created node will be returned */
/* if parent is NULL, NULL will be returned */
/**/
struct trie *trie_add_child(struct trie *parent, const char *label, void *value)
{
    struct trie *new = NULL, *p = NULL;
    int should_rearrange = 0;

    if(parent == NULL) return NULL;

    /* Adding a new element into the trie
     *
     * (1) If parent has no children, add the new trie as a child and return.
     * (2) when parent has children, iterate each character in the label and look for it in the children collection
     * (3) if not found, add the new trie into the children list. else continue until we get a place to insert.
     * (4) if item already exit without doing anything.
     */
    p = find_parent(parent, label);
    if(p == NULL) {
        /* supplied label already exist. ignoring duplicates */
        return parent;
    }

    if(p->child != NULL) {
        /* this trie has another children. After inserting we may neeed to rearrange the siblings */
        should_rearrange = 1;
    }
    
    new = trie_new(label, value);
    if(new) 
    {       
        insert_child(p, new);

        if(should_rearrange) {
            rearrange_siblings(p, new);
        }
    }
    return new;
}

static int iterate_trie_recursive(struct trie *t, itfunction function, unsigned int depth, void *userdata)
{    
    int rc;
    while(t != NULL)
    {
        if(function(t, depth, userdata) != VARNAM_SUCCESS) {
            return VARNAM_ERROR;
        }
        rc = iterate_trie_recursive(t->child, function, depth + 1, userdata); 
        if(rc != VARNAM_SUCCESS) {
            return rc;
        }
        t = t->next;
    }
    return VARNAM_SUCCESS;
}

int trie_iterate(struct trie *t, itfunction function, void *userdata)
{    
    if(function == NULL) return VARNAM_ERROR;

    return iterate_trie_recursive(t, function, 0, userdata);
}

unsigned int trie_free(struct trie *root, freefunction callback)
{
    struct trie *current = NULL, *next = NULL;
    unsigned int freecnt = 0;

    current = root;
    while(current != NULL)
    {
        freecnt += trie_free(current->child, callback) + 1;
        next = current->next;
        if( !current->root && callback != NULL ) {
            callback( current->value );
        }
        xfree(current);
        current = next;
    }
    return freecnt;
}

static unsigned int get_count(struct trie *t, unsigned int count)
{
    while(t != NULL)
    {
        count = get_count(t->child, ++count);
        t = t->next;
    }
    return count;
}

unsigned int trie_children_count(struct trie *root)
{
    unsigned int count = get_count(root, 0);
    if(count != 0)
        count = count - 1;  /* not considering root */
    return count;
}

static struct trie *lookup(struct trie *root, const char *label)
{
    struct trie *item = NULL, *temp = NULL;
    int charcount = 1;
    char buffer[MAX_PATTERN_LENGTH];
    const char *lbl;

    item = root;
    lbl = label;

    /* iterating each character in the label and incrementally looking the children */
    /* collection for the match */
    /**/
    while(*lbl != '\0') 
    {
        substr(buffer, label, 1, charcount++);
        temp = find_match(item, buffer);
        if(temp) {
            item = temp;
        }
        ++lbl;
    }

    if(strcmp(item->label, label) == 0) {
        /* this means item already exist */
        return item;
    }
    return NULL;
}

void *trie_lookup(struct trie *root, const char *label)
{
    struct trie *item = NULL;
    if(root == NULL) return NULL;

    if((item = lookup(root, label)) != NULL) {
        return item->value;
    }
    return NULL;
}

