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

#include "video.h"

#include "sw.h"
#include "swdisp.h"
#include "swground.h"
#include "swgrpha.h"
#include "swmain.h"
#include "swsymbol.h"
#include "swutil.h"

static GRNDTYPE grndsave[SCR_WDTH];   // saved ground buffer


/*---------------------------------------------------------------------------

        Update display of ground.   Delete previous display of ground by
        XOR graphics.

        Different routines are used to display/delete ground on colour
        or monochrome systems.

---------------------------------------------------------------------------*/

static void dispgrnd()
{
	{
		static int clrgrndsave = 0;
		if (!clrgrndsave) {
			memset(grndsave, 0, sizeof(grndsave));
			clrgrndsave = 1;
		}
	}

	if (!dispinit) {
		if (!(dispdx || forcdisp))
			return;
		if (conf_solidground)
			Vid_DispGround_Solid(grndsave);
		else 
			Vid_DispGround(grndsave);
	}

	// sdh 16/10/2001: removed movmem

	memcpy(grndsave, ground+displx, SCR_WDTH * sizeof(GRNDTYPE));

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
	setvdisp();
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

	setvdisp();

	for (ob = objtop; ob; ob = ob->ob_next) {
		if (!(ob->ob_delflg && ob->ob_drwflg)
		    || ob->ob_newsym->h == 1
		    || ob->ob_oldsym != ob->ob_newsym
		    || ob->ob_y != ob->ob_oldy
		    || (ob->ob_oldx + displx) != ob->ob_x) {
			if (ob->ob_delflg)
				Vid_DispSymbol(ob->ob_oldx, ob->ob_oldy,
					       ob->ob_oldsym, 
					       ob_color(ob));
			if (!ob->ob_drwflg)
				continue;
			if (ob->ob_x < displx || ob->ob_x > disprx) {
				ob->ob_drwflg = 0;
				continue;
			}
			ob->ob_oldx = ob->ob_x - displx;
			ob->ob_oldy = ob->ob_y;
			Vid_DispSymbol(ob->ob_oldx,
				       ob->ob_oldy,
				       ob->ob_newsym, 
				       ob_color(ob));
		}

		if (ob->ob_drawf)
			(*(ob->ob_drawf)) (ob);
	}

	for (ob = deltop; ob; ob = ob->ob_next)
		if (ob->ob_delflg)
			Vid_DispSymbol(ob->ob_oldx, ob->ob_oldy,
				       ob->ob_oldsym, ob_color(ob));

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


/*---------------------------------------------------------------------------

        External calls to specify current video ram as screen ram or
        auxiliary screen area.

---------------------------------------------------------------------------*/

void setvdisp()
{
	Vid_SetBuf();
}

void setadisp()
{
	Vid_SetBuf_Aux();
}

void movedisp()
{
	Vid_CopyBuf();
}


void clrdispv()
{
	Vid_ClearBuf();
}

void clrdispa()
{
	Vid_ClearBuf_Aux();
}

//---------------------------------------------------------------------------
//
// $Log$
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

