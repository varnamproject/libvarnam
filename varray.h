/* varnam-array.h
 *
 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */


#ifndef VARNAM_ARRAY_H_INCLUDED_0924
#define VARNAM_ARRAY_H_INCLUDED_0924

#include "vtypes.h"
#include "util.h"

/**
 * Array to hold pointers. This expands automatically.
 *
 **/
typedef struct varray_t
{
    void **memory;
    size_t allocated;
    size_t used;
    int index;
} varray;

typedef struct vpool_t
{
    varray *array;
    int next_slot;
    varray *free_pool;
} vpool;

varray*
varray_init();

void
varray_push(varray *array, void *data);

void
varray_remove_at(varray *array, int index);

int
varray_length(varray *array);

bool
varray_is_empty (varray *array);

bool
varray_exists (varray *array, void *item, bool (*equals)(void *left, void *right));

void
varray_clear(varray *array);

void*
varray_get(varray *array, int index);

void*
varray_get_last_item(varray *array);

void*
varray_pop_last_item(varray *array);

void
varray_insert(varray *array, int index, void *data);

void
varray_free(varray *array, void (*destructor)(void*));

vpool*
vpool_init();

/**
 * Returns next item from the pool. NULL otherwise
 **/
void*
vpool_get(vpool *pool);

void
vpool_add(vpool *pool, void *item);

/**
 * Returns the element back to the pool
 */
void
vpool_return(vpool *pool, void *item);

void
vpool_reset(vpool *pool);

/**
 * Free the items contained in the pool and finally the pool itself
 *
 **/
void
vpool_free(vpool *pool, void (*destructor)(void*));

varray*
get_pooled_array (varnam *handle);

void
return_array_to_pool (varnam *handle, varray *array);

void
reset_pool(varnam *handle);

#endif
