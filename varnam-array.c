/* <file-name>
 *
 * Copyright (C) <year> <author>
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


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "varnam-array.h"
#include "varnam-util.h"

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
    if ((array->allocated - array->used) < size) {
        toallocate = array->allocated == 0 ? size : (array->allocated * 2);
        array->memory = realloc(array->memory, toallocate);
        array->allocated = array->allocated + toallocate;
    }

    array->memory[++array->index] = data;
    array->used = array->used + size;
}

int
varray_length(varray *array)
{
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
varray_free(varray *array, bool free_items)
{
    int i;
    if (free_items)
    {
        for(i = 0; i < varray_length(array); i++)
        {
            xfree (array->memory[i]);
            array->memory[i] = NULL;
        }
    }
    array->used = 0;
    array->index = -1;

    free(array->memory);
    free(array);
}

void*
varray_get(varray *array, int index)
{
    if (index < 0 || index > array->index)
        return NULL;

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

vpool*
vpool_init()
{
    vpool *pool = (vpool*) malloc (sizeof(vpool));
    pool->array = varray_init();
    pool->next_slot = -1;
    return pool;
}

void*
vpool_get(vpool *pool)
{
    void *item;
    assert (pool);

    item = varray_get (pool->array, pool->next_slot);
    if (item != NULL)
        ++pool->next_slot;

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
vpool_reset(vpool *pool)
{
    if (pool != NULL)
        pool->next_slot = 0;
}

void
vpool_free(vpool *pool)
{
    assert (pool);
    varray_free (pool->array, true);
}
