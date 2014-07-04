/* varnam-array.h
 *
 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */


#ifndef VARNAM_ARRAY_H_INCLUDED_0924
#define VARNAM_ARRAY_H_INCLUDED_0924

#include "util.h"
#include "vtypes.h"

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

VARNAM_EXPORT extern varray* 
varray_init();

VARNAM_EXPORT extern void
varray_push(varray *array, void *data);

VARNAM_EXPORT extern void
varray_copy(varray *source, varray *destination);

VARNAM_EXPORT extern void
varray_remove_at(varray *array, int index);

VARNAM_EXPORT extern int
varray_length(varray *array);

VARNAM_EXPORT extern bool
varray_is_empty (varray *array);

VARNAM_EXPORT extern bool
varray_exists (varray *array, void *item, bool (*equals)(void *left, void *right));

VARNAM_EXPORT extern void
varray_clear(varray *array);

VARNAM_EXPORT extern void*
varray_get(varray *array, int index);

VARNAM_EXPORT extern void*
varray_get_last_item(varray *array);

VARNAM_EXPORT extern void*
varray_pop_last_item(varray *array);

VARNAM_EXPORT extern void
varray_insert(varray *array, int index, void *data);

VARNAM_EXPORT extern void
varray_free(varray *array, void (*destructor)(void*));

VARNAM_EXPORT extern vpool*
vpool_init();

/**
 * Returns next item from the pool. NULL otherwise
 **/
VARNAM_EXPORT extern void*
vpool_get(vpool *pool);

VARNAM_EXPORT extern void
vpool_add(vpool *pool, void *item);

/**
 * Returns the element back to the pool
 */
VARNAM_EXPORT extern void
vpool_return(vpool *pool, void *item);

VARNAM_EXPORT extern void
vpool_reset(vpool *pool);

/**
 * Free the items contained in the pool and finally the pool itself
 *
 **/
VARNAM_EXPORT extern void
vpool_free(vpool *pool, void (*destructor)(void*));

VARNAM_EXPORT extern varray*
get_pooled_array (varnam *handle);

VARNAM_EXPORT extern void
return_array_to_pool (varnam *handle, varray *array);

VARNAM_EXPORT extern void
reset_pool(varnam *handle);

#endif
