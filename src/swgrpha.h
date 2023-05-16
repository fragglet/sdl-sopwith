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

#ifndef __SWGRPHA_H__
#define __SWGRPHA_H__

#include "sw.h"

extern char *vidram;

extern void swdisp(void);
extern void swground(void);
extern void swputsym(int x, int y, OBJECTS *ob);
//extern char    fill[];

extern void swinitgrph(void);

extern void colorscreen(int color);

#endif
