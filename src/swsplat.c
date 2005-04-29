// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------------
//
// $Id$
//
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
//---------------------------------------------------------------------------
// 
// Keep track of and display heads-up splats.
//
//---------------------------------------------------------------------------

#include <string.h>

#include "swgrpha.h"
#include "swmain.h"
#include "swsplat.h"
#include "swsymbol.h"

typedef struct {
	int x, y;
	int clr;
	sopsym_t *sym;
} splat_t;

#define MAX_SPLATS 64

static int oxsplatted = 0;
static splat_t splats[MAX_SPLATS];
static int num_splats = 0;

void swclearsplats(void)
{
	num_splats = 0;
	oxsplatted = 0;
}

void swdispsplats(void)
{
	int i;
	
	if (oxsplatted)
		colorscreen(2);

	for (i=0; i<num_splats; ++i) {
		OBJECTS ob;

		ob.ob_type = DUMMYTYPE;
		ob.ob_clr = splats[i].clr;
		ob.ob_newsym = splats[i].sym;

		swputsym(splats[i].x, splats[i].y, &ob);
	}
}

#define SEED_START 74917777

static unsigned long seed = SEED_START;

static unsigned long randsd()
{
	seed *= countmove;
	seed += 7491;

	if (!seed)
		seed = SEED_START;

	return 0;
}

static void add_splat(splat_t *splat)
{
	if (num_splats < MAX_SPLATS) {
		memcpy(&splats[num_splats], splat, sizeof(splat_t));
		++num_splats;
	}
}


void swsplatbird(void)
{
	splat_t splat;
	
	randsd();

	splat.x = (unsigned) (seed % (SCR_WDTH - 32));
	splat.y = (unsigned) (seed % (SCR_HGHT - 60)) + 60;
	splat.sym = symbol_birdsplat;
	splat.clr = 2;

	add_splat(&splat);
}

void swwindshot(void)
{
	splat_t splat;

	randsd();

	splat.x = (unsigned) (seed % (SCR_WDTH - 16));
	splat.y = (unsigned) (seed % (SCR_HGHT - 50)) + 50;
	splat.sym = symbol_shotwin;
	splat.clr = 0;

	add_splat(&splat);
}

void swsplatox(void)
{
	oxsplatted = 1;
}


//---------------------------------------------------------------------------
//
// $Log$
// Revision 1.3  2005/04/29 19:25:28  fraggle
// Update copyright to 2005
//
// Revision 1.2  2004/10/15 17:52:32  fraggle
// Clean up compiler warnings. Rename swmisc.c -> swtext.c as this more
// accurately describes what the file does.
//
// Revision 1.1  2004/10/15 17:23:32  fraggle
// Restore HUD splats
//
//
//---------------------------------------------------------------------------


