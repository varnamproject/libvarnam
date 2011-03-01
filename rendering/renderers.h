/* renderers.h

Copyright (C) 2010 Navaneeth.K.N

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "../varnam-types.h"

#ifndef RENDERING_RENDERERS_H_INCLUDED_105305
#define RENDERING_RENDERERS_H_INCLUDED_105305

int ml_unicode_renderer(varnam*, struct token*, struct strbuf*);
int ml_unicode_rtl_renderer(varnam*, struct token*, struct strbuf*);

#endif
