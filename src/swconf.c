// Emacs style mode select -*- C++ -*-
//--------------------------------------------------------------------------
//
// $Id$
//
// Copyright(C) 1984-2000 David L. Clark
// Copyright(C) 2001-2005 Simon Howard
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or (at your
// option) any later version. This program is distributed in the hope that
// it will be useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
// the GNU General Public License for more details. You should have
// received a copy of the GNU General Public License along with this
// program; if not, write to the Free Software Foundation, Inc., 59 Temple
// Place - Suite 330, Boston, MA 02111-1307, USA.
//
//--------------------------------------------------------------------------
//
// Configuration code
//
// Save game settings to a configuration file
//
//-------------------------------------------------------------------------

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "video.h"

#include "sw.h"
#include "swconf.h"
#include "swend.h"
#include "swgrpha.h"
#include "swtext.h"
#include "swtitle.h"
#include "swmain.h"

#ifdef _WIN32
static char config_file[] = "sopwith.ini";
#else
static char config_file[] = ".sopwithrc";
#endif

static char *get_config_filename()
{
#ifdef _WIN32
	// this should probably be saved in the registry,
	// but pfft, whatever
	return config_file;
#else
	if (getenv("HOME")) {
		static char *namebuf = NULL;

		if (!namebuf) {
			namebuf = malloc(strlen(config_file)
					 + strlen(getenv("HOME")) + 5);

			sprintf(namebuf, "%s/%s", getenv("HOME"), config_file);
		}

		return namebuf;
	} else {
		return config_file;
	}
#endif
}

static confoption_t confoptions[] = {
	{"conf_missiles",    CONF_BOOL, {&conf_missiles}},
	{"conf_solidground", CONF_BOOL, {&conf_solidground}},
	{"conf_hudsplats",   CONF_BOOL, {&conf_hudsplats}},
	{"conf_wounded",     CONF_BOOL, {&conf_wounded}},
	{"conf_animals",     CONF_BOOL, {&conf_animals}},
	{"conf_harrykeys",   CONF_BOOL, {&conf_harrykeys}},
	{"conf_medals",	     CONF_BOOL, {&conf_medals}},
	{"vid_fullscreen",   CONF_BOOL, {&vid_fullscreen}},
	{"vid_double_size",  CONF_BOOL, {&vid_double_size}},

	{"key_accelerate", CONF_KEY, {&keybindings[KEY_ACCEL]}},
	{"key_decelerate", CONF_KEY, {&keybindings[KEY_DECEL]}},
	{"key_pullup",     CONF_KEY, {&keybindings[KEY_PULLUP]}},
	{"key_pulldown",   CONF_KEY, {&keybindings[KEY_PULLDOWN]}},
	{"key_flip",       CONF_KEY, {&keybindings[KEY_FLIP]}},
	{"key_fire",       CONF_KEY, {&keybindings[KEY_FIRE]}},
	{"key_dropbomb",   CONF_KEY, {&keybindings[KEY_BOMB]}},
	{"key_home",       CONF_KEY, {&keybindings[KEY_HOME]}},
	{"key_missile",    CONF_KEY, {&keybindings[KEY_MISSILE]}},
	{"key_starburst",  CONF_KEY, {&keybindings[KEY_STARBURST]}},
};

int num_confoptions = sizeof(confoptions) / sizeof(*confoptions);

static void chomp(char *s)
{
	char *p;
	for (p=s+strlen(s)-1; isspace(*p) && p > s; --p) {
		*p = '\0';
	}
}

static confoption_t *confoption_by_name(char *name)
{
	int i;

	for (i=0; i<num_confoptions; ++i) {
		if (!strcasecmp(name, confoptions[i].name)) {
			return &confoptions[i];
		}
	}

	return NULL;
}

static void parse_config_line(char *config_file, int lineno, char *line)
{
	char *name, *value, *p;
	int key;
	confoption_t *opt;

	p = line;
	chomp(p);

	// skip whitespace and discard comments.
	while (*p != '\0' && isspace(*p)) {
		++p;
	}
	if (*p == '#') {
		return;
	}
	if (*p == '\0') {
		return;
	}

	name = p;
	for (; *p != '\0' && !isspace(*p); ++p);

	if (*p == '\0') {
		fprintf(stderr, "swloadconf: %s:%d: malformed line: no value\n",
		        config_file, lineno);
		return;
	}

	*p++ = '\0';
	for (; isspace(*p); ++p);
	value = p;

	opt = confoption_by_name(name);
	if (opt == NULL) {
		fprintf(stderr,
		        "swloadconf: %s:%d: unknown config option '%s'\n",
		        config_file, lineno, name);
		return;
	}

	switch (opt->type) {
		case CONF_BOOL:
			*opt->value.b = atoi(value) != 0;
			break;
		case CONF_KEY:
			key = Vid_KeyFromName(value);
			if (key != 0) {
				*opt->value.i = key;
			}
			break;
		default:
			break;
	}
}

//
// load config file
//
// ugly but it works
//

void swloadconf()
{
	char *config_file = get_config_filename();
	FILE *fs;
	char inbuf[128];
	int lineno = 0;

	fs = fopen(config_file, "r");

	if (fs == NULL) {
		fprintf(stderr, "swloadconf: failed to open %s: %s\n",
		        config_file, strerror(errno));
		return;
	}

	while (!feof(fs)) {
		fgets(inbuf, sizeof(inbuf), fs);
		++lineno;
		parse_config_line(config_file, lineno, inbuf);
	}

	fclose(fs);
}

//
// swsaveconf
//
// save config file
//

void swsaveconf()
{
	char *config_file = get_config_filename();
	FILE *fs;
	int i;

	fs = fopen(config_file, "w");

	if (fs == NULL) {
		fprintf(stderr, "swsaveconf: failed to open %s: %s\n",
		        config_file, strerror(errno));
		return;
	}

	fprintf(fs, "# sopwith config file\n"
	            "# created by " PACKAGE_STRING "\n\n");

	for (i=0; i<num_confoptions; ++i) {
		fprintf(fs, "%-20s", confoptions[i].name);
		switch (confoptions[i].type) {
		case CONF_BOOL:
			fprintf(fs, "%d", *confoptions[i].value.b);
			break;
		case CONF_KEY:
			fprintf(fs, "%s", Vid_KeyName(*confoptions[i].value.i));
			break;
		default:
			fprintf(fs, "?");
			break;
		}
		fprintf(fs, "\n");
	}

	fprintf(fs, "\n\n");

	fclose(fs);
}

struct menuitem {
	char *config_name;
	char *description;
};

static const char menukeys[] = "1234567890ABCDEFGHIJKL";

static void drawmenu(char *title, struct menuitem *menu)
{
	int i, keynum, said_key = 0;
	Vid_ClearBuf();

	swcolour(2);
	swposcur(19 - strlen(title) / 2, 2);
	swputs(title);

	swcolour(3);

	for (i=0, keynum=0; menu[i].config_name != NULL; ++i) {
		confoption_t *opt;
		char *suffix;
		char buf[40];
		int key;

		if (strlen(menu[i].config_name) == 0) {
			continue;
		}

		if (menu[i].config_name[0] == '>') {
			key = menu[i].config_name[1];
			swcolour(1);
			suffix = " >>>";
		} else {
			key = menukeys[keynum];
			++keynum;
			suffix = ":";

			if (!said_key) {
				swposcur(1, 5+i);
				swputs("Key:");
				said_key = 1;
			}
		}
		sprintf(buf, "%c - %s%s",
			key, menu[i].description, suffix);

		swposcur(6, 5+i);
		swputs(buf);
		swcolour(3);

		swposcur(32, 5+i);
		opt = confoption_by_name(menu[i].config_name);
		if (opt == NULL) {
			continue;
		}
		switch (opt->type) {
		case CONF_BOOL:
			swputs(*opt->value.b ? "on" : "off");
			break;
		case CONF_KEY:
			swputs(Vid_KeyName(*opt->value.i));
			break;
		default:
			break;
		}
	}

	swcolour(1);

	swposcur(1, 22);
	swputs("   ESC - Exit Menu");

	Vid_Update();
}

static struct menuitem *menuitem_for_key(struct menuitem *menu, int key)
{
	int i, keynum;

	for (i=0, keynum=0; menu[i].config_name != NULL; ++i) {
		int itemkey;
		if (strlen(menu[i].config_name) == 0) {
			continue;
		} else if (menu[i].config_name[0] == '>') {
			itemkey = menu[i].config_name[1];
		} else {
			itemkey = menukeys[keynum];
			++keynum;
		}
		if (key == itemkey) {
			return &menu[i];
		}
	}

	return NULL;
}

// Present the given menu to the user. Returns zero if escape was pushed
// to exit the menu, or if a >jump item was chosen, it returns the key
// binding associated with it.
static int runmenu(char *title, struct menuitem *menu)
{
	struct menuitem *pressed;
	confoption_t *opt;
	int key;

	for (;;) {
		drawmenu(title, menu);

		if (ctlbreak()) {
			swend(NULL, NO);
		}

		key = toupper(swgetc() & 0xff);
		if (key == 27) {
			return 0;
		}

		// check if a number has been pressed for a menu option
		pressed = menuitem_for_key(menu, key);
		if (pressed == NULL) {
			continue;
		}

		if (pressed->config_name[0] == '>') {
			return pressed->config_name[1];
		}

		opt = confoption_by_name(pressed->config_name);
		if (opt == NULL) {
			continue;
		}
		switch (opt->type) {
		case CONF_BOOL:
			*opt->value.b = !*opt->value.b;
			break;
		default:
			break;
		}

		// reset the screen if we need to
		if (opt->value.b == &vid_fullscreen
		 || opt->value.b == &vid_double_size) {
			Vid_Reset();
		}

		swsaveconf();
	}
}

struct menuitem keys_menu[] = {
	{"key_accelerate", "Accelerate"},
	{"key_decelerate", "Decelerate"},
	{"key_pullup",     "Pull up"},
	{"key_pulldown",   "Pull down"},
	{"key_flip",       "Flip"},
	{"key_fire",       "Fire machine gun"},
	{"key_dropbomb",   "Drop bomb"},
	{"key_home",       "Navigate home"},
	{"key_missile",    "Fire missile"},
	{"key_starburst",  "Starburst"},
	{NULL},
};

struct menuitem options_menu[] = {
	{"vid_fullscreen",    "Run fullscreen"},
	{"vid_double_size",   "Scale window by 2x"},
	{"conf_missiles",     "Missiles"},
	{"conf_solidground",  "Solid ground"},
	{"conf_hudsplats",    "HUD splats"},
	{"conf_wounded",      "Wounded planes"},
	{"conf_animals",      "Oxen and birds"},
	{"conf_harrykeys",    "Harry keys mode"},
	{"conf_medals",	      "Medals"},
	{"",                  ""},
	{">K",                "Key bindings"},
	{NULL},
};

void setconfig()
{
	runmenu("OPTIONS", options_menu);
}

//-------------------------------------------------------------------------
//
// $Log$
// Revision 1.9  2005/05/29 19:46:10  fraggle
// Fix up autotools build. Fix "make dist".
//
// Revision 1.8  2005/04/29 19:25:28  fraggle
// Update copyright to 2005
//
// Revision 1.7  2005/04/29 19:00:48  fraggle
// Capitalise first letter of config descriptions
//
// Revision 1.6  2005/04/29 10:10:12  fraggle
// "Medals" feature
// By Christoph Reichenbach <creichen@gmail.com>
//
// Revision 1.5  2004/10/15 17:52:31  fraggle
// Clean up compiler warnings. Rename swmisc.c -> swtext.c as this more
// accurately describes what the file does.
//
// Revision 1.4  2003/06/08 03:41:41  fraggle
// Remove auxdisp buffer totally, and all associated functions
//
// Revision 1.3  2003/06/04 17:22:11  fraggle
// Remove "save settings" option in settings menus. Just save it anyway.
//
// Revision 1.2  2003/04/05 22:55:11  fraggle
// Remove the FOREVER macro and some unused stuff from std.h
//
// Revision 1.1.1.1  2003/02/14 19:03:10  fraggle
// Initial Sourceforge CVS import
//
//
// sdh 14/2/2003: change license header to GPL
// sdh 26/03/2002: change CGA_ to Vid_
// sdh 10/11/2001: make confoptions globally available for gtk code to use
//
//-------------------------------------------------------------------------


