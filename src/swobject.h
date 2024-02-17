//
// Copyright(C) 1984-2000 David L. Clark
// Copyright(C) 2001-2005 Simon Howard
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or (at your
// option) any later version. This program is distributed in the hope that
// it will be useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//

#ifndef __SWOBJECT_H__
#define __SWOBJECT_H__

#include "sw.h"

extern bool insertx(OBJECTS *ob, OBJECTS *obp);
extern OBJECTS *deletex(OBJECTS *obp);
extern OBJECTS *allocobj(void);
extern void deallobj(OBJECTS *obp);
extern void movexy(OBJECTS *ob, int *x, int *y);
extern void setdxdy(OBJECTS *obj, int dx, int dy);

#endif
