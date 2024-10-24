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
// Configuration code
//
// Save game settings to a configuration file
//

#ifndef __SWCONF_H__
#define __SWCONF_H__

struct conf_option {
	char *name;
	enum {
		CONF_BOOL,
		CONF_INT,
		CONF_KEY,
	} type;
	union {
		void *v;
		bool *b;
		int *i;
	} value;
};

extern void swloadconf(void);
extern void swsaveconf(void);
extern void setconfig(void);          // config menu

#endif
