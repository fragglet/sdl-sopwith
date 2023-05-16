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

#ifndef __SWAUTO_H__
#define __SWAUTO_H__

#include "sw.h"

extern void swauto(OBJECTS *ob);
extern int gohome(OBJECTS *obpt);
extern int aim(OBJECTS *obo, int ax, int ay, OBJECTS *obt, bool longway);
extern int range(int x, int y, int ax, int ay);

#endif
