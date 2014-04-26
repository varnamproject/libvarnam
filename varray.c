/* Dynamically growing array implementation
 *
 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */



#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "varray.h"
#include "util.h"

varray*
varray_init()
{
    varray *array = (varray*) malloc (sizeof(varray));
    array->memory = NULL;
    array->allocated = 0;
    array->used = 0;
    array->index = -1;

    return array;
}

void
varray_push(varray *array, void *data)
{
    size_t toallocate;
    size_t size = sizeof(void*);

    if (data == NULL) return;

    if ((array->allocated - array->used) < size) {
        toallocate = array->allocated == 0 ? size : (array->allocated * 2);
        array->memory = realloc(array->memory, toallocate);
        array->allocated = toallocate;
    }

    array->memory[++array->index] = data;
    array->used = array->used + size;
}

void
varray_copy(varray *source, varray *destination)
{
    int i;
    void *item;

    if (source == NULL) return;
    if (destination == NULL) return;

    for (i = 0; i < varray_length (source); i++) {
        item = varray_get (source, i);
        varray_push (destination, item);
    }
}

void*
varray_get_last_item(varray *array)
{
    assert (array);

    return varray_get (array, array->index);
}

void
varray_remove_at(varray *array, int index)
{
    int i, len;

    if (index < 0 || index > array->index)
        return;

    len = varray_length(array);
    for(i = index + 1; i < len; i++)
    {
        array->memory[index++] = array->memory[i];
    }

    array->used = array->used - sizeof(void*);
    array->index--;
}

void*
varray_pop_last_item(varray *array)
{
    void *item;
    assert (array);

    item = varray_get_last_item (array);
    if (item != NULL)
        varray_remove_at (array, array->index);

    return item;
}

int
varray_length(varray *array)
{
    if (array == NULL)
        return 0;

    return array->index + 1;
}

void
varray_clear(varray *array)
{
    int i;
    for(i = 0; i < varray_length(array); i++)
    {
        array->memory[i] = NULL;
    }
    array->used = 0;
    array->index = -1;
}

void
varray_free(varray *array, void (*destructor)(void*))
{
    int i;
    void *item;

    if (array == NULL)
        return;

    if (destructor != NULL)
    {
        for(i = 0; i < varray_length(array); i++)
        {
            item = varray_get (array, i);
            if (item != NULL) destructor(item);
        }
    }

    if (array->memory != NULL)
        free(array->memory);
    free(array);
}

void*
varray_get(varray *array, int index)
{
    if (index < 0 || index > array->index)
        return NULL;

    assert(array->memory);

    return array->memory[index];
}

void
varray_insert(varray *array, int index, void *data)
{
    if (index < 0 || index > array->index)
        return;

    array->memory[index] = data;
}

bool
varray_is_empty (varray *array)
{
    return (varray_length (array) == 0);
}

bool
varray_exists (varray *array, void *item, bool (*equals)(void *left, void *right))
{
    int i;

    for (i = 0; i < varray_length (array); i++)
    {
        if (equals(varray_get (array, i), item))
            return true;
    }

    return false;
}

vpool*
vpool_init()
{
    vpool *pool = (vpool*) malloc (sizeof(vpool));

    pool->array = varray_init();
    pool->free_pool = varray_init();
    pool->next_slot = 0;

    return pool;
}

void*
vpool_get(vpool *pool)
{
    void *item;
    assert (pool);

    /* First try to get from the free pool if there are any */
    item = varray_pop_last_item (pool->free_pool);
    if (item == NULL)
    {
        item = varray_get (pool->array, pool->next_slot);
        if (item != NULL)
            ++pool->next_slot;
    }

    return item;
}

void
vpool_add(vpool *pool, void *item)
{
    assert (pool);
    assert (item);
    varray_push (pool->array, item);
    ++pool->next_slot;
}

void
vpool_return(vpool *pool, void *item)
{
    assert (pool);
    assert (item);
    varray_push (pool->free_pool, item);
}

void
vpool_reset(vpool *pool)
{
    if (pool != NULL)
    {
        pool->next_slot = 0;
        varray_clear (pool->free_pool);
    }
}

void
vpool_free(vpool *pool, void (*destructor)(void*))
{
    if (pool == NULL)
        return;

    varray_free (pool->array, destructor);
    varray_free (pool->free_pool, NULL);
    pool->next_slot = -1;
    pool->array = NULL;
    pool->free_pool = NULL;
    xfree (pool);
}

varray*
get_pooled_array (varnam *handle)
{
    varray *array;

    if (v_->arrays_pool == NULL)
        v_->arrays_pool = vpool_init ();

    array = vpool_get (v_->arrays_pool);
    if (array == NULL)
    {
        array = varray_init ();
        vpool_add (v_->arrays_pool, array);
    }

    varray_clear (array);
    return array;
}

void
return_array_to_pool (varnam *handle, varray *array)
{
    if (v_->arrays_pool == NULL)
        return;

    vpool_return (v_->arrays_pool, array);
}

void
reset_pool(varnam *handle)
{
    assert(handle);
    assert(handle->internal);
    vpool_reset (v_->tokens_pool);
    vpool_reset (v_->arrays_pool);
    vpool_reset (v_->strings_pool);
}
