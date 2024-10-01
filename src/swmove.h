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

#ifndef __SWMOVE_H__
#define __SWMOVE_H__

#include "sw.h"

extern bool successful_flight;

extern void swmove(void);
extern bool moveplyr(OBJECTS *obp);
extern bool movecomp(OBJECTS *obp);
extern bool moveshot(OBJECTS *obp);
extern bool movebomb(OBJECTS *obp);
extern bool movemiss(OBJECTS *obp);
extern bool moveburst(OBJECTS *obp);
extern bool movetarg(OBJECTS *obt);
extern bool moveexpl(OBJECTS *obp);
extern bool movesmok(OBJECTS *obp);
extern bool moveflck(OBJECTS *obp);
extern bool movebird(OBJECTS *obp);
extern bool moveox(OBJECTS *ob);
extern bool crashpln(OBJECTS *obp);
extern bool hitpln(OBJECTS *obp);

#endif
