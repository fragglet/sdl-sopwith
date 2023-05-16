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

#ifndef __SWEND_H__
#define __SWEND_H__

#include "sw.h"

extern void swend(char *msg, bool update);
extern void endgame(int targclr);
extern void winner(OBJECTS *obp);
extern void loser(OBJECTS *ob);
extern void dispendmessage(void);

#endif
