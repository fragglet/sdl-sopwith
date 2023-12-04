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
//
// Configuration code
//
// Save game settings to a configuration file
//

#ifndef __SWCONF_H__
#define __SWCONF_H__

typedef struct
{
	char *name;
	enum {
		CONF_BOOL,
		CONF_INT,
		CONF_KEY,
		CONF_BTN,
	} type;
	union {
		void *v;
		bool *b;
		int *i;
		int *btn;
	} value;
} confoption_t;

extern void swloadconf(void);
extern void swsaveconf(void);
extern void setconfig(void);          // config menu

#endif
