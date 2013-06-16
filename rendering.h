/* rendering.h - Token rendering related functions
 *
 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */


#include "vtypes.h"
#include "varray.h"

#ifndef RENDERING_H_INCLUDED_200624
#define RENDERING_H_INCLUDED_200624

int
resolve_tokens(varnam *handle,
               varray *tokens,
               vword **word);

int
resolve_rtl_tokens(varnam *handle,
                  varray *tokens,
                  char **output);

#endif
