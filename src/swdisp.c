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
//        swdispc  -      Display all players and objects
//
//---------------------------------------------------------------------------

#include "video.h"

#include "sw.h"
#include "swdisp.h"
#include "swgrpha.h"
#include "swmain.h"
#include "swtext.h"
#include "swsound.h"
#include "swsymbol.h"
#include "swutil.h"

static void plnsound(OBJECTS *obp)
{
	register OBJECTS *ob = obp;

	if (ob->ob_firing)
		sound(S_SHOT, 0, ob);
	else
		switch (ob->ob_state) {
		case FALLING:
			if (ob->ob_dy >= 0)
				sound(S_HIT, 0, ob);
			else
				sound(S_FALLING, ob->ob_y, ob);
			break;

		case FLYING:
			sound(S_PLANE, -ob->ob_speed, ob);
			break;

		case STALLED:
		case WOUNDED:
		case WOUNDSTALL:
			sound(S_HIT, 0, ob);
			break;

		default:
			break;
		}

}



void dispbomb(OBJECTS * obp)
{
	register OBJECTS *ob = obp;

	if (ob->ob_dy <= 0)
		sound(S_BOMB, -ob->ob_y, ob);
}





void dispmiss(OBJECTS * obp)
{
}





void dispburst(OBJECTS * obp)
{
}





void dispexpl(OBJECTS * obp)
{
	register OBJECTS *ob = obp;

	if (ob->ob_orient)
		sound(S_EXPLOSION, ob->ob_hitcount, ob);
}





void dispcomp(OBJECTS * ob)
{
	plnsound(ob);
}

void disptarg(OBJECTS * ob)
{
	if (ob->ob_firing)
		sound(S_SHOT, 0, ob);
}

void dispflck(OBJECTS * ob)
{
}

void dispbird(OBJECTS * ob)
{
}

void dispplyr(OBJECTS * ob)
{
	plnsound(ob);
}

void dispribbonrow (int *ribbonid, int ribbons_nr, int y)
{
	int i;
	int offset = 50 + ((3 - ribbons_nr) * 4);

	for (i = 0; i < ribbons_nr; i++) {
		int ribbon_id = ribbonid[i];

		Vid_DispSymbol(offset, y, symbol_ribbon[ribbon_id], 0);
		offset += 8;
	}
}

void dispscore(OBJECTS * ob)
{
	static const int medal_widths[3] = {10, 8, 8};
	static const int medal_offsets[3] = {0, -1, -1};
	int medal_offset = 50;
	int i;

	Vid_Box(0, 16, 48 + 32, 16, 0);
	
	swposcur((ob->ob_clr - 1) * 7 + 2, 24);
	swcolour(ob->ob_clr);
	swdispd(ob->ob_score.score, 6);

	if (conf_medals) {
		for (i = 0; i < ob->ob_score.medals_nr; i++) {
			int medal_id = ob->ob_score.medalslist[i];

			Vid_DispSymbol(medal_offset + medal_offsets[medal_id], 11, symbol_medal[medal_id], 0);
			medal_offset += medal_widths[medal_id];
		}

		if (ob->ob_score.ribbons_nr <= 3)
			dispribbonrow(ob->ob_score.ribbons, ob->ob_score.ribbons_nr, 15);
		else {
			dispribbonrow(ob->ob_score.ribbons, 3, 16);
			dispribbonrow(ob->ob_score.ribbons + 3, ob->ob_score.ribbons_nr - 3, 14);
		}
	}
}
 
//---------------------------------------------------------------------------
//
// $Log$
// Revision 1.10  2005/04/29 10:10:12  fraggle
// "Medals" feature
// By Christoph Reichenbach <creichen@gmail.com>
//
// Revision 1.9  2004/10/15 21:30:58  fraggle
// Improve multiplayer
//
// Revision 1.8  2004/10/15 18:51:24  fraggle
// Fix the map. Rename dispworld to dispmap as this is what it really does.
//
// Revision 1.7  2004/10/15 17:52:31  fraggle
// Clean up compiler warnings. Rename swmisc.c -> swtext.c as this more
// accurately describes what the file does.
//
// Revision 1.6  2004/10/15 17:23:32  fraggle
// Restore HUD splats
//
// Revision 1.5  2004/10/15 16:39:32  fraggle
// Unobfuscate some parts
//
// Revision 1.4  2003/06/08 18:41:01  fraggle
// Merge changes from 1.7.0 -> 1.7.1 into HEAD
//
// Revision 1.3  2003/06/08 02:39:25  fraggle
// Initial code to remove XOR based drawing
//
// Revision 1.2.2.1  2003/06/08 18:16:38  fraggle
// Fix networking and some compile bugs
//
// Revision 1.2  2003/04/05 22:44:04  fraggle
// Remove some useless functions from headers, make them static if they
// are not used by other files
//
// Revision 1.1.1.1  2003/02/14 19:03:10  fraggle
// Initial Sourceforge CVS import
//
//
// sdh 14/2/2003: change license header to GPL
// sdh 27/06/2002: move to new sopsym_t for symbols, 
//                 remove symwdt and symhgt
// sdh 28/10/2001: option to disable hud splats
// sdh 21/10/2001: rearranged file headers, added cvs tags
// sdh 21/10/2001: reformatted with indent, adjusted some code by hand
//                 to make more readable
// sdh 19/10/2001: removed extern definitions: these are now in headers
//                 shuffled some functions around to shut up compiler
// sdh 18/10/2001: converted all functions to ANSI-style arguments
//
// 87-04-05        Missile and starburst support
// 87-03-13        Splatted bird symbol.
// 87-03-12        Wounded airplanes.
// 87-03-09        Microsoft compiler.
// 84-06-12        PCjr Speed-up
// 84-02-21        Development
//
//---------------------------------------------------------------------------

