//
// Copyright(C) 1984-2000 David L. Clark
// Copyright(C) 2001-2005 Simon Howard
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or (at your
// option) any later version. This program is distributed in the hope that
// it will be useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
// the GNU General Public License for more details. You should have
// received a copy of the GNU General Public License along with this
// program; if not, write to the Free Software Foundation, Inc., 59 Temple
// Place - Suite 330, Boston, MA 02111-1307, USA.
//

#ifndef __SWDISP_H__
#define __SWDISP_H__

#include "sw.h"

extern void dispplyr(OBJECTS *ob);
extern void dispbomb(OBJECTS *obp);
extern void dispmiss(OBJECTS *obp);
extern void dispburst(OBJECTS *obp);
extern void dispexpl(OBJECTS *obp);
extern void dispcomp(OBJECTS *ob);
extern void disptarg(OBJECTS *ob);
extern void dispflck(OBJECTS *ob);
extern void dispbird(OBJECTS *ob);

#endif
