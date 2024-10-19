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

#ifndef __SWOBJECT_H__
#define __SWOBJECT_H__

#include "sw.h"

extern bool insertx(OBJECTS *ob, OBJECTS *obp);
extern OBJECTS *deletex(OBJECTS *obp);
extern void updateobjpos(OBJECTS *ob);
extern void copyobj(OBJECTS *to, OBJECTS *from);
extern OBJECTS *allocobj(void);
extern void deallobj(OBJECTS *obp);
extern void movexy(OBJECTS *ob, int *x, int *y);
extern void setdxdy(OBJECTS *obj, int dx, int dy);

extern bool plane_is_killed(obstate_t state);
extern bool plane_is_stalled(obstate_t state);
extern bool plane_is_wounded(obstate_t state);

#endif
