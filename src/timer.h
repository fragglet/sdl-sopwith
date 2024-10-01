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
//
// Timer code
//
// Abstraction layer for access to the timer
// Basically this is to keep SDL out of the main code, so we can if we
// want drop in other code to run without SDL.
//

#ifndef __TIMER_H__
#define __TIMER_H__

extern void Timer_Init(void);
extern int Timer_GetMS(void);
extern void Timer_Sleep(int usec);

#endif
