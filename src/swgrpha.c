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
//        swgrph   -      SW screen graphics
//
//---------------------------------------------------------------------------

#include <string.h>

#include "video.h"

#include "sw.h"
#include "swdisp.h"
#include "swground.h"
#include "swgrpha.h"
#include "swinit.h"
#include "swmain.h"
#include "swsplat.h"
#include "swsymbol.h"
#include "swutil.h"

/*---------------------------------------------------------------------------

        Update display of ground.   Delete previous display of ground by
        XOR graphics.

        Different routines are used to display/delete ground on colour
        or monochrome systems.

---------------------------------------------------------------------------*/

static void dispgrnd()
{
	if (conf_solidground)
		Vid_DispGround_Solid(ground + displx);
	else 
		Vid_DispGround(ground + displx);
}



/*---------------------------------------------------------------------------

        External display ground call for title screen processing.

---------------------------------------------------------------------------*/




void swground()
{
	dispgrnd();
}


// sdh 27/7/2002: removed clrcol

/*---------------------------------------------------------------------------

        External calls to display a point of a specified colour at a
        specified position.   The point request may or may not ask for
        collision detection by returning the old colour of the point.

        Different routines are used to display points on colour or
        monochrome systems.

---------------------------------------------------------------------------*/




// sdh 27/03/2002: remove swpntcol and swpntsym

// sdh 14/2/2003: find the color of an object
// always draw bullets white

static inline int ob_color(OBJECTS *ob)
{
	if (ob->ob_type == SHOT)
		return 3;
	else
		return ob->ob_clr;
}


/*---------------------------------------------------------------------------

        Display an object's current symbol at a specified screen location
        Collision detection may or may not be asked for.

        Different routines are used to display symbols on colour or
        monochrome systems.

---------------------------------------------------------------------------*/




void swputsym(int x, int y, OBJECTS * ob)
{
	// sdh 14/2/2003: always draw bullets white

	Vid_DispSymbol(x, y, ob->ob_newsym, ob_color(ob));
}


// sdh 27/7/2002: removed putcol



/*---------------------------------------------------------------------------

        Main display loop.   Delete and display all visible objects.
        Delete any newly deleted objects

---------------------------------------------------------------------------*/


// sdh 14/2/2003: always draw bullets white

void swdisp()
{
	register OBJECTS *ob;

	Vid_ClearBuf();

	// calculate displx from the player position
	// do sanity checks to make sure we never go out of range

	displx = consoleplayer->ob_x - SCR_CENTR;

	if (displx < 0)
		displx = 0;
	else if (displx >= MAX_X - SCR_WDTH)
		displx = MAX_X - SCR_WDTH - 1;

	// display the status bar

	dispmap();
	dispscore(consoleplayer);
	dispguages(consoleplayer);

	// heads up splats
	
	if (conf_hudsplats)
		swdispsplats();

	// "the end"
	
	dispendmessage();

	// draw objects

	for (ob = objtop; ob; ob = ob->ob_next) {
		if (ob->ob_drwflg
		 && ob->ob_x >= displx 
		 && ob->ob_x < displx + SCR_WDTH) {
			swputsym(ob->ob_x - displx, ob->ob_y, ob);

			if (ob->ob_drawf)
				(*(ob->ob_drawf)) (ob);
		}
	}

	dispgrnd();

	dispinit = FALSE;
	forcdisp = TRUE;

	// need to update the screen as we arent writing
	// directly into vram any more

	Vid_Update();
}


// sdh 26/3/02: removed mode setting stuff (done in platform specific
// code now)

// sdh: experiments into fixing splatted ox
// color the screen all one color

void colorscreen(int color)
{
	int x, y;

	for (y=19; y<SCR_HGHT; ++y) {
		for (x=0; x<SCR_WDTH; ++x) {
			Vid_PlotPixel(x, y, color);
		}
	}
}

//---------------------------------------------------------------------------
//
// $Log$
// Revision 1.10  2004/10/15 18:51:24  fraggle
// Fix the map. Rename dispworld to dispmap as this is what it really does.
//
// Revision 1.9  2004/10/15 17:52:32  fraggle
// Clean up compiler warnings. Rename swmisc.c -> swtext.c as this more
// accurately describes what the file does.
//
// Revision 1.8  2004/10/15 17:23:32  fraggle
// Restore HUD splats
//
// Revision 1.7  2003/06/08 18:41:01  fraggle
// Merge changes from 1.7.0 -> 1.7.1 into HEAD
//
// Revision 1.6  2003/06/08 03:41:41  fraggle
// Remove auxdisp buffer totally, and all associated functions
//
// Revision 1.5  2003/06/08 02:48:45  fraggle
// Remove dispdx, always calculated displx from the current player position
// and do proper edge-of-level bounds checking
//
// Revision 1.4  2003/06/08 02:39:25  fraggle
// Initial code to remove XOR based drawing
//
// Revision 1.3.2.1  2003/06/08 18:16:38  fraggle
// Fix networking and some compile bugs
//
// Revision 1.3  2003/06/04 17:13:25  fraggle
// Remove disprx, as it is implied from displx anyway.
//
// Revision 1.2  2003/06/04 15:59:09  fraggle
// Remove broken screenshot function
//
// Revision 1.1.1.1  2003/02/14 19:03:12  fraggle
// Initial Sourceforge CVS import
//
//
// sdh 14/2/2003: change license header to GPL
// 		  fix bullets being colored (should be white)
// sdh 27/7/2002: removed old collision detection code
// sdh 27/6/2002: move to new sopsym_t for symbols
// sdh 26/03/2002: moved all drawing functions into platform specific 
//                 files
//                 change CGA_ to Vid_
// sdh 28/10/2001: get_type/set_type removed
// sdh 28/10/2001: moved auxdisp and auxdisp functions here
// sdh 24/10/2001: fix auxdisp buffer
// sdh 21/10/2001: use new obtype_t and obstate_t
// sdh 21/10/2001: rearranged headers, added cvs tags
// sdh 21/10/2001: added #define for solid ground (sopwith 1 style)
// sdh 21/10/2001: reformatted with indent, adjusted some code by hand
//                 to make more readable
// sdh 19/10/2001: removed extern definitions, these are in headers now
//                 shuffled some functions round to shut up the compiler
// sdh 18/10/2001: converted all functions to ANSI-style arguments
//
// 87-03-09        Microsoft compiler.
// 85-11-05        Atari
// 84-06-13        PCjr Speed-up
// 84-02-21        Development
//
//---------------------------------------------------------------------------

