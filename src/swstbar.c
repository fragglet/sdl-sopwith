// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------
//
// Status bar code
//
//---------------------------------------------------------------------------

#include "video.h"

#include "swground.h"
#include "swmain.h"

static void dispgge(int x, int cury, int maxy, int clr)
{
	int y;

	cury = cury * 10 / maxy - 1;
	if (cury > 9)
		cury = 9;
	for (y = 0; y <= cury; ++y)
		Vid_PlotPixel(x, y, clr);
	for (; y <= 9; ++y)
		Vid_PlotPixel(x, y, 0);
}

// sdh 26/10/2001: merged gauge functions into a single function

static void dispgauges(OBJECTS *ob)
{
	int x = GAUGEX;
	int sep = conf_missiles ? 3 : 5;

	// crashes/lives

	dispgge(x += sep, maxcrash - ob->ob_crashcnt, maxcrash, ob->ob_clr);

	// fuel

	dispgge(x += sep, ob->ob_life >> 4, MAXFUEL >> 4, ob->ob_clr);

	// bombs

	dispgge(x += sep, ob->ob_bombs, MAXBOMBS, 3 - ob->ob_clr);

	// bullets

 	dispgge(x += sep, ob->ob_rounds, MAXROUNDS, 3);

	if (conf_missiles) {

		// missiles
		
		dispgge(x += sep, ob->ob_missiles, MAXMISSILES, ob->ob_clr);

// starburst (flares)

		dispgge(x += sep, ob->ob_bursts, MAXBURSTS, 3 - ob->ob_clr);
	}
}

static void dispmapobjects()
{
	OBJECTS *ob;

	for (ob=objtop; ob; ob=ob->ob_next) {
		if (ob->ob_onmap) {
			int x, y;

			x = SCR_CENTR 
			  + ((ob->ob_x + (ob->ob_newsym->w / 2)) / WRLD_RSX);
			y = ((ob->ob_y - (ob->ob_newsym->h / 2)) / WRLD_RSY); 

			Vid_PlotPixel(x, y, ob->ob_clr);
		}
	}
}

static void dispmap()
{
	int x, y, dx, maxh, sx;

	dx = 0;
	sx = SCR_CENTR;

	maxh = 0;
	y = 0;

	// draw ground

	for (x = 0; x < MAX_X; ++x) {

		if (ground[x] > maxh)
			maxh = ground[x];

		++dx;

		if (dx == WRLD_RSX) {
			maxh /= WRLD_RSY;
			if (maxh == y)
				Vid_PlotPixel(sx, maxh, 7);
			else if (maxh > y)
				for (++y; y <= maxh; ++y)
					Vid_PlotPixel(sx, y, 7);
			else
				for (--y; y >= maxh; --y)
					Vid_PlotPixel(sx, y, 7);
			y = maxh;
			Vid_PlotPixel(sx, 0, 11);
			++sx;
			dx = maxh = 0;
		}
	}

	// map border

	maxh = MAX_Y / WRLD_RSY;
	for (y = 0; y <= maxh; ++y) {
		Vid_PlotPixel(SCR_CENTR, y, 11);
		Vid_PlotPixel(sx, y, 11);
	}

	dispmapobjects();

	// border of status bar

	for (x = 0; x < SCR_WDTH; ++x)
		Vid_PlotPixel(x, (SCR_MNSH + 2), 7);
}


void dispstatusbar(void)
{
	dispmap();
	dispscore(consoleplayer);
	dispgauges(consoleplayer);
}



