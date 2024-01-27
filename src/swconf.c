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

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "timer.h"
#include "pcsound.h"
#include "video.h"

#include "sw.h"
#include "swconf.h"
#include "swend.h"
#include "swsound.h"
#include "swtext.h"
#include "swtitle.h"
#include "swmain.h"

#define CONFIG_FILE_NAME "sopwith.cfg"

static char *GetConfigFilename(void)
{
	static char *result = NULL;
	char *pref_path;
	size_t buflen;

	if (result != NULL) {
		return result;
	}

	pref_path = Vid_GetPrefPath();
	if (pref_path == NULL) {
		return NULL;
	}

	buflen = strlen(pref_path) + strlen(CONFIG_FILE_NAME) + 1;
	result = malloc(buflen);
	if (result == NULL) {
		return NULL;
	}

	snprintf(result, buflen, "%s%s", pref_path, CONFIG_FILE_NAME);
	return result;
}

static confoption_t confoptions[] = {
	{"conf_missiles",       CONF_BOOL, {&conf_missiles}},
	{"conf_solidground",    CONF_BOOL, {&conf_solidground}},
	{"conf_hudsplats",      CONF_BOOL, {&conf_hudsplats}},
	{"conf_wounded",        CONF_BOOL, {&conf_wounded}},
	{"conf_animals",        CONF_BOOL, {&conf_animals}},
	{"conf_harrykeys",      CONF_BOOL, {&conf_harrykeys}},
	{"conf_big_explosions", CONF_BOOL, {&conf_big_explosions}},
	{"conf_medals",         CONF_BOOL, {&conf_medals}},
	{"vid_fullscreen",      CONF_BOOL, {&vid_fullscreen}},
	{"snd_tinnyfilter",     CONF_BOOL, {&snd_tinnyfilter}},

	{"conf_video_palette",	CONF_INT, {&conf_video_palette}},

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

static int num_confoptions = sizeof(confoptions) / sizeof(*confoptions);

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
		case CONF_INT:
			if(!strcasecmp(opt->name, "conf_video_palette")) {
				// If an invalid video palette number was loaded, use palette 0
				if(atoi(value) >= (Vid_GetNumVideoPalettes()) ){
					*opt->value.i = 0;
				} else {
					*opt->value.i = atoi(value);
				}
			} else {
				*opt->value.i = atoi(value);
			}
			break;
		case CONF_KEY:
			if (sscanf(value, "%d", &key) == 1) {
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

void swloadconf(void)
{
	char *config_file = GetConfigFilename();
	FILE *fs;
	char inbuf[128];
	int lineno = 0;

	if (config_file == NULL) {
		return;
	}

	fs = fopen(config_file, "r");

	if (fs == NULL) {
		// It isn't an error if the config file doesn't exist yet.
		if (errno != ENOENT) {
			fprintf(stderr, "swloadconf: failed to open %s: %s\n",
			        config_file, strerror(errno));
		}
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

void swsaveconf(void)
{
	char *config_file = GetConfigFilename();
	FILE *fs;
	int i;

	if (config_file == NULL) {
		return;
	}

	fs = fopen(config_file, "w");

	if (fs == NULL) {
		fprintf(stderr, "swsaveconf: failed to open %s: %s\n",
		        config_file, strerror(errno));
		return;
	}

	fprintf(fs, "# Configuration file for " PACKAGE_NAME "\n"
	            "# Created by " PACKAGE_STRING "\n\n");

	for (i=0; i<num_confoptions; ++i) {
		fprintf(fs, "%-20s", confoptions[i].name);
		switch (confoptions[i].type) {
		case CONF_BOOL:
			fprintf(fs, "%d", *confoptions[i].value.b);
			break;
		case CONF_INT:
		case CONF_KEY:
			fprintf(fs, "%d", *confoptions[i].value.i);
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

static void change_key_binding(struct menuitem *item)
{
	confoption_t *opt;
	int key;

	Vid_ClearBuf();

	swcolor(3);
	swposcur(10, 5);
	swputs("Press the new key for: ");

	swcolor(2);
	swposcur(14, 7);
	swputs(item->description);

	swcolor(1);
	swposcur(1, 22);
	swputs("   ESC - Cancel");

	Vid_Update();

	while ((key = Vid_GetKey()) == 0) {
		Timer_Sleep(50);

		swsndupdate();
		if (ctlbreak()) {
			return;
		}
	}

	if (!strcasecmp(Vid_KeyName(key), "Escape")) {
		return;
	}

	opt = confoption_by_name(item->config_name);
	if (opt == NULL) {
		return;
	}
	*opt->value.i = key;
}

static void drawmenu(char *title, struct menuitem *menu)
{
	int i, y, keynum, said_key = 0;
	Vid_ClearBuf();

	swcolor(2);
	swposcur(19 - strlen(title) / 2, 2);
	swputs(title);

	swcolor(3);

	for (i=0, y=0, keynum=0; menu[i].config_name != NULL; ++i, ++y) {
		confoption_t *opt;
		char *suffix;
		char buf[40];
		int key;

		if (strlen(menu[i].config_name) == 0) {
			continue;
		}

		if (menu[i].config_name[0] == '>') {
			key = menu[i].config_name[1];
			swcolor(1);
			suffix = " >>>";
		} else {
			key = menukeys[keynum];
			++keynum;
			suffix = ":";

			if (!said_key) {
				swposcur(0, 5+y);
				swputs("Key:");
				said_key = 1;
			}
		}
		snprintf(buf, sizeof(buf), "%c - %s%s",
		         key, menu[i].description, suffix);

		swposcur(5, 5+y);
		swputs(buf);
		swcolor(3);

		if (strlen(buf) > 22) {
			++y;
		}

		swposcur(28, 5+y);
		opt = confoption_by_name(menu[i].config_name);
		if (opt == NULL) {
			continue;
		}
		switch (opt->type) {
		case CONF_BOOL:
			swputs(*opt->value.b ? "on" : "off");
			break;
		case CONF_INT:
			if(!strcasecmp(opt->name, "conf_video_palette")) {
				swputs(Vid_GetVideoPaletteName(*opt->value.i));
			}
			break;
		case CONF_KEY:
			swputs(Vid_KeyName(*opt->value.i));
			break;
		default:
			break;
		}
	}

	swcolor(1);

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
			swend(NULL, false);
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
		case CONF_INT:
			if(!strcasecmp(opt->name, "conf_video_palette")) {
				*opt->value.i = (*opt->value.i + 1) % Vid_GetNumVideoPalettes();
				Vid_SetVideoPalette(*opt->value.i);
			}
			break;
		case CONF_KEY:
			change_key_binding(pressed);
			break;
		default:
			break;
		}

		// reset the screen if we need to
		if (opt->value.b == &vid_fullscreen) {
			Vid_Reset();
		}

		swsaveconf();
	}
}

static struct menuitem keys_menu[] = {
	{"key_accelerate", "Accelerate"},
	{"key_decelerate", "Decelerate"},
	{"key_pullup",     "Pull up"},
	{"key_pulldown",   "Pull down"},
	{"key_flip",       "Flip"},
	{"key_fire",       "Fire machine gun"},
	{"key_dropbomb",   "Drop bomb"},
	{"key_home",       "Navigate home"},
	{NULL},
};

static struct menuitem options_menu[] = {
#ifndef NO_FULLSCREEN
	{"vid_fullscreen",      "Run fullscreen"},
#endif
	{"conf_video_palette",  "Video palette"},
	{"conf_solidground",    "Solid ground"},
	{"conf_hudsplats",      "HUD splats"},
	{"conf_wounded",        "Wounded planes"},
	{"conf_animals",        "Oxen and birds"},
	{"conf_big_explosions", "Big oil tank explosions"},
	{"conf_medals",         "Medals"},
	{"conf_harrykeys",      "Harry keys mode"},
	{"",                    ""},
	{">K",                  "Key bindings"},
	{NULL},
};

void setconfig(void)
{
	for (;;) {
		switch (runmenu("OPTIONS", options_menu)) {
			case 0:
				return;
			case 'K':
				runmenu("OPTIONS > KEY BINDINGS", keys_menu);
				break;
		}
	}
}
