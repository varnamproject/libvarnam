/* renderers.h

 * Copyright (C) Navaneeth.K.N
 *
 * This is part of libvarnam. See LICENSE.txt for the license
 */


#ifndef RENDERING_RENDERERS_H_INCLUDED_105305
#define RENDERING_RENDERERS_H_INCLUDED_105305

#include "../vtypes.h"
#include "../util.h"

int
ml_unicode_renderer(varnam *handle,
                    vtoken *previous,
                    vtoken *current,
                    strbuf *output);

int
ml_unicode_rtl_renderer(varnam *handle,
                        vtoken *previous,
                        vtoken *current,
                        strbuf *output);

#endif
