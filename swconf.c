// Emacs style mode select -*- C++ -*-
//--------------------------------------------------------------------------
//
// $Id: $
//
// Copyright(C) 2001 Simon Howard
//
// This file is dual-licensed under version 2 of the GNU General Public
// License as published by the Free Software Foundation, and the Sopwith
// License as published by David L. Clark. See the files GPL and license.txt
// respectively for more information.
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

#include "cgavideo.h"

#include "sw.h"
#include "swconf.h"
#include "swend.h"
#include "swgrpha.h"
#include "swmisc.h"
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

typedef struct
{
	char *name;
	enum {
		CONF_BOOL,
		CONF_INT
	} type;
	union {
		void *v;
		BOOL *b;
		int *i;
	} value;
	char *description;
} confoption_t;

static confoption_t confoptions[] = {
    {"conf_missiles",     CONF_BOOL, {&conf_missiles},     "missiles"},
    {"conf_solidground",  CONF_BOOL, {&conf_solidground},  "solid ground"},
    {"conf_hudsplats",    CONF_BOOL, {&conf_hudsplats},    "HUD splats"},
    {"conf_wounded",      CONF_BOOL, {&conf_wounded},      "wounded planes"},
    {"conf_animals",      CONF_BOOL, {&conf_animals},      "oxen and birds"},
    {"conf_harrykeys",    CONF_BOOL, {&conf_harrykeys},    "harry keys mode"},
    {"cga_fullscreen",    CONF_BOOL, {&cga_fullscreen},    "run fullscreen"},
    {"cga_double_size",   CONF_BOOL, {&cga_double_size},   "scale window by 2x"},
};

static int num_confoptions = sizeof(confoptions) / sizeof(*confoptions);

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

		if ((p = strchr(inbuf, '#')))
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
	FOREVER {
		int i;
		
		clrdispv();

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
		
		swposcur(1, 21);
		swputs("     S - save configuration file");

		swposcur(1, 22);
		swputs("   ESC - Exit Menu");

		CGA_Update();

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
			
			if (confoptions[i].value.b == &cga_fullscreen
			    || confoptions[i].value.b == &cga_double_size) {
				swshutdowngrph();
				swinitgrph();				
			}
			
			continue;
		}

		// other keys
		
		switch(i) {
		case 'S':
			swsaveconf();
			break;
		case 27:
			return;
		}
	}
		
}

//-------------------------------------------------------------------------
//
// $Log: $
//
//-------------------------------------------------------------------------
