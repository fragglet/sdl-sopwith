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

#ifndef __SWMISC_H__
#define __SWMISC_H__

#include "sw.h"

extern void swputc(char c);
extern void swputs(const char *sp );
extern void swgets(char *s, int max);
extern void swcolor(int a);
extern void swposcur(int a, int b);
extern int swgetc(void);
extern void swflush(void);

#endif
