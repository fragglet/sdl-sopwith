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

#ifndef __SWINIT_H__
#define __SWINIT_H__

#include "sw.h"
extern void swinit(int argc, char *argv[]);
extern void swinitlevel(void);
extern void swrestart(void);
extern void initdisp(bool reset);
extern void initgrnd(void);
extern void initcomp(OBJECTS *obp);
extern void initplyr(OBJECTS *obp);
extern OBJECTS *initpln(OBJECTS *obp);
extern void initshot(OBJECTS *obop, OBJECTS *targ);
extern void initbomb(OBJECTS *obop);
extern void initmiss(OBJECTS *obop);
extern void initburst(OBJECTS *obop);
extern void initexpl(OBJECTS *obop, int small);
extern void initsmok(OBJECTS *obop);
extern void initbird(OBJECTS *obop, int i);

extern GRNDTYPE *ground;
extern int starting_level;

#endif
