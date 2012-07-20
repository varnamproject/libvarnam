/* rendering.h - Token rendering related functions
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

#include "varnam-types.h"
#include "varnam-array.h"

#ifndef RENDERING_H_INCLUDED_200624
#define RENDERING_H_INCLUDED_200624

int
resolve_tokens(varnam *handle,
               varray *tokens,
               vword **word);

struct varnam_token_rendering*
get_additional_rendering_rule(varnam *handle);

#endif
