/* varnam-array.h
 *
 * Copyright (C) Navaneeth KN
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

#ifndef VARNAM_ARRAY_H_INCLUDED_0924
#define VARNAM_ARRAY_H_INCLUDED_0924

#include "varnam-util.h"

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
} vpool;

varray*
varray_init();

void
varray_push(varray *array, void *data);

int
varray_length(varray *array);

void
varray_clear(varray *array);

void*
varray_get(varray *array, int index);

void
varray_insert(varray *array, int index, void *data);

void
varray_free(varray *array, int free_items);

vpool*
vpool_init();

/**
 * Returns next item from the pool. NULL otherwise
 **/
void*
vpool_get(vpool *pool);

void
vpool_add(vpool *pool, void *item);

void
vpool_reset(vpool *pool);

/**
 * Free the items contained in the pool and finally the pool itself
 *
 **/
void
vpool_free(vpool *pool);

#endif
