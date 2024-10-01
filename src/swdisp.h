//
// Copyright(C) 1984-2000 David L. Clark
// Copyright(C) 2001-2005 Simon Howard
//
// You can redistribute and/or modify this program under the terms of the
// GNU General Public License version 2 as published by the Free Software
// Foundation, or any later version. This program is distributed WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.
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
