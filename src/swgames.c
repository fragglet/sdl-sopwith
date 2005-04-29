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
//        swgames  -      SW definition of games
//
//---------------------------------------------------------------------------


#include "sw.h"
#include "swgames.h"

static int planes[]      = { 0, 7, 1, 6 };
static int mult_planes[] = { 0, 7, 3, 4, 2, 5, 1, 6 };

GAMES swgames[1] = {
	{
		{1270, 588, 1330, 1360, 1630, 1660, 2456, 1720},
		{0, 0, 0, 0, 1, 1, 1, 1},
		NULL,
		7491,
		NULL,
		{
			191, 284, 409, 539, 685,
			807, 934, 1210, 1240, 1440,
			1550, 1750, 1780, 2024, 2159,
			2279, 2390, 2549, 2678, 2763
		},
		{
			1, 3, 1, 1, 3,
			0, 1, 2, 0, 3,
			3, 0, 2, 1, 1,
			3, 3, 0, 0, 1
		},
		planes,
		mult_planes,
	}
};

//---------------------------------------------------------------------------
//
// $Log$
// Revision 1.2  2005/04/29 10:10:12  fraggle
// "Medals" feature
// By Christoph Reichenbach <creichen@gmail.com>
//
// Revision 1.1.1.1  2003/02/14 19:03:10  fraggle
// Initial Sourceforge CVS import
//
//
// sdh 14/2/2003: change license header to GPL
// sdh 21/10/2001: rearranged file headers, added cvs tags
// sdh 21/10/2001: reformatted with indent, adjusted some code by hand
//                 to make more readable
// sdh 20/10/2001: added parentheses to shut up compiler
//
// 87-03-09        Microsoft compiler.
// 84-02-08        Development
//
//---------------------------------------------------------------------------

