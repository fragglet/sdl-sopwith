// Emacs style mode select -*- C++ -*-
//--------------------------------------------------------------------------
//
// $Id$
//
// Copyright(C) 1984-2000 David L. Clark
// Copyright(C) 2001-2003 Simon Howard
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

// get the location of the configuration file

static char *get_config_file()
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

confoption_t confoptions[] = {
    {"conf_missiles",     CONF_BOOL, {&conf_missiles},     "missiles"},
    {"conf_solidground",  CONF_BOOL, {&conf_solidground},  "solid ground"},
    {"conf_hudsplats",    CONF_BOOL, {&conf_hudsplats},    "HUD splats"},
    {"conf_wounded",      CONF_BOOL, {&conf_wounded},      "wounded planes"},
    {"conf_animals",      CONF_BOOL, {&conf_animals},      "oxen and birds"},
    {"conf_harrykeys",    CONF_BOOL, {&conf_harrykeys},    "harry keys mode"},
    {"vid_fullscreen",    CONF_BOOL, {&vid_fullscreen},    "run fullscreen"},
    {"vid_double_size",   CONF_BOOL, {&vid_double_size},   "scale window by 2x"},
};

int num_confoptions = sizeof(confoptions) / sizeof(*confoptions);

// clean up a string

static void chomp(char *s)
{
	char *p;

	for (p=s; isspace(*p); ++p);
	strcpy(s, p);

	for (p=s+strlen(s)-1; isspace(*p) && p > s; --p)
		*p = '\0';
}

//
// load config file
//
// ugly but it works
//

void swloadconf()
{
	char *config_file = get_config_file();
	FILE *fs;
	int line = 0;

	fs = fopen(config_file, "r");

	// doesnt exist, or we cant open it for
	// some reason
	
	if (!fs) {
		fprintf(stderr,
			"swloadconf: cant open %s: %s\n",
			config_file,
			strerror(errno));
		return;
	}
	
	while(!feof(fs)) {
		char inbuf[128];
		char *p;
		int i;
		
		fgets(inbuf, sizeof(inbuf)-1, fs);
		++line;
		
		// comments

		p = strchr(inbuf, '#');
		if (p)
			*p = '\0';
		
		// clean up string

		chomp(inbuf);

		if (!strlen(inbuf))
			continue;

		for (p=inbuf; *p && !isspace(*p); ++p);

		if (!*p) {
			fprintf(stderr,
				"swloadconf: line %i of %s is malformed\n",
				line, config_file);
			continue;
		}

		*p++ = '\0';
		for (; isspace(*p); ++p);

		// now, inbuf = variable name
		//      p = value

		for (i=0; i<num_confoptions; ++i) {
			if (strcasecmp(inbuf, confoptions[i].name))
				continue;

			// found option

			switch(confoptions[i].type) {
			case CONF_BOOL:
				*confoptions[i].value.b = atoi(p) != 0;
				break;
			default:
				break;
			}

			break;
		}

		if (i >= num_confoptions)
			fprintf(stderr,
				"swloadconf: unknown configuration option "
				"'%s' on "
				"line %i of %s\n",
				inbuf, line, config_file);
	}

	// all done

	fclose(fs);
}

//
// swsaveconf
//
// save config file
//

void swsaveconf()
{
	char *config_file = get_config_file();
	FILE *fs;
	int i;

	fs = fopen(config_file, "w");

	if (!fs) {
		fprintf(stderr,
			"swsaveconf: cant open %s for writing: %s\n",
			config_file,
			strerror(errno));
		return;
	}

	fprintf(fs, "# sopwith config file\n");
	fprintf(fs, "# created by SDL Sopwith " VERSION "\n");
	fprintf(fs, "\n");
	
	for (i=0; i<num_confoptions; ++i) {
		int n;

		fprintf(fs, "%s", confoptions[i].name);

		for (n=3-strlen(confoptions[i].name)/8; n > 0; --n)
			fprintf(fs, "\t");
		
		switch (confoptions[i].type) {
		case CONF_BOOL:
			fprintf(fs, "%i", *confoptions[i].value.b);
			break;
		default:
			fprintf(fs, "xyzzy!");
		}

		fprintf(fs, "\t# %s\n", confoptions[i].description);
	}

	fprintf(fs, "\n\n");

	fclose(fs);
}


// sdh 28/10/2001: options menu
// sdh 29/10/2001: moved here

void setconfig()
{
	for (;;) {
		int i;
		
		Vid_ClearBuf();

		swcolour(2);
		swposcur(15, 2);
		swputs("OPTIONS");
		
		swcolour(3);

		for (i=0; i<num_confoptions; ++i) {
			char buf[40];
			
			sprintf(buf,
				"%s %i - %s:",
				i ? "    " : "Key:",
				i+1, confoptions[i].description);

			swposcur(1, 5+i);
			swputs(buf);

			swposcur(35, 5+i);
			switch (confoptions[i].type) {
			case CONF_BOOL:
				swputs(*confoptions[i].value.b ? "on" : "off");
				break;
			default:
				break;
			}
		}

		swcolour(1);
		
		swposcur(1, 22);
		swputs("   ESC - Exit Menu");

		Vid_Update();

		if (ctlbreak())
			swend(NULL, NO);

		i = toupper(swgetc() & 0xff);

		// check if a number has been pressed for a menu option
		
		if (i >= '1' && i <= '9') {

			i -= '1';

			switch (confoptions[i].type) {
			case CONF_BOOL:
				*confoptions[i].value.b
					= !*confoptions[i].value.b;
				break;
			default:
				break;
			}

			// reset the screen if we need to
			
			if (confoptions[i].value.b == &vid_fullscreen
			    || confoptions[i].value.b == &vid_double_size) {
				Vid_Reset();
			}

			swsaveconf();
			
			continue;
		}

		// other keys
		
		switch(i) {
		case 27:
			return;
		}
	}
		
}

//-------------------------------------------------------------------------
//
// $Log$
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


