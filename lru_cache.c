/*

 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */

#include <string.h>
#include "util.h"
#include "vtypes.h"

/* this is an example of how to do a LRU cache in C using uthash - https://gist.github.com/jehiah/900846
 http://uthash.sourceforge.net/
 by Jehiah Czebotar 2011 - jehiah@gmail.com
 this code is in the public domain http://unlicense.org/ */

#define MAX_CACHE_SIZE 100000

void*
lru_find_in_cache (vcache_entry **cache, char *key)
{
    vcache_entry *entry;
    HASH_FIND_STR(*cache, key, entry);
    if (entry) {
        /* remove it (so the subsequent add will throw it on the front of the list) */
        HASH_DELETE(hh, *cache, entry);
        HASH_ADD_KEYPTR(hh, *cache, entry->key, strlen(entry->key), entry);
        return entry->value;
    }
    return NULL;
}

int
lru_key_exists (vcache_entry **cache, char *key)
{
    vcache_entry *entry;
    HASH_FIND_STR(*cache, key, entry);
    if (entry) {
        return 1;
    }
    return 0;
}

void
lru_add_to_cache (vcache_entry **cache, char *key, void *value, vcache_value_free_cb cb)
{
    vcache_entry *entry, *tmp_entry;
    strbuf *keyCopy;
    entry = malloc(sizeof(vcache_entry));
    keyCopy = strbuf_init (16);
    strbuf_add (keyCopy, key);
    entry->key = strbuf_detach (keyCopy);
    entry->value = value;
    entry->cb = cb;
    HASH_ADD_KEYPTR(hh, *cache, entry->key, strlen(entry->key), entry);

    /* prune the cache to MAX_CACHE_SIZE */
    if (HASH_COUNT(*cache) >= MAX_CACHE_SIZE) {
        HASH_ITER(hh, *cache, entry, tmp_entry) {
            /* prune the first entry (loop is based on insertion order so this deletes the oldest item) */
            HASH_DELETE(hh, *cache, entry);
            free(entry->key);
            if (entry->cb != NULL) {
                entry->cb (entry->value);
            }
            free(entry);
            break;
        }
    }
}

