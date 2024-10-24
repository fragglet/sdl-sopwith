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

#ifndef __SWASYNIO_H__
#define __SWASYNIO_H__

enum asyn_mode {
	ASYN_LISTEN,
	ASYN_CONNECT,
};

extern enum asyn_mode asynmode;
extern char asynhost[128];
extern int asynport;

extern void asynput(int movekey);
extern char *asynclos(void);
extern void init1asy(void);
extern void init2asy(void);
extern void asynupdate(void);

#endif

