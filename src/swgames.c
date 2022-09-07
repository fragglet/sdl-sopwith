// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------
//
//        swgames  -      SW definition of games
//
//---------------------------------------------------------------------------


#include "sw.h"
#include "swground.h"
#include "swgames.h"

static original_ob_t original_targets[] = {
	{TARGET,  191, 1, 0, 0, OWNER_PLAYER1_IN_MULT},
	{TARGET,  284, 3, 0, 0, OWNER_PLAYER1_IN_MULT},
	{TARGET,  409, 1, 0, 0, OWNER_PLAYER1_IN_MULT},
	{TARGET,  539, 1, 0, 0, OWNER_PLAYER1_IN_MULT},
	{TARGET,  685, 3, 0, 0, OWNER_PLAYER1_IN_MULT},
	{TARGET,  807, 0, 0, 0, OWNER_PLAYER1_IN_MULT},
	{TARGET,  934, 1, 0, 0, OWNER_PLAYER1_IN_MULT},

	{TARGET, 1210, 2, 0, 0, OWNER_PLAYER1},
	{TARGET, 1240, 0, 0, 0, OWNER_PLAYER1},
	{OX,     1376, 0, 0, 0, OWNER_PLAYER1},
	{TARGET, 1440, 3, 0, 0, OWNER_PLAYER1},

	{TARGET, 1550, 3, 0, 0, OWNER_PLAYER2},
	{OX,     1608, 0, 0, 0, OWNER_PLAYER1},
	{TARGET, 1750, 0, 0, 0, OWNER_PLAYER2},
	{TARGET, 1780, 2, 0, 0, OWNER_PLAYER2},
	{TARGET, 2024, 1, 0, 0, OWNER_PLAYER2},
	{TARGET, 2159, 1, 0, 0, OWNER_PLAYER2},
	{TARGET, 2279, 3, 0, 0, OWNER_PLAYER2},
	{TARGET, 2390, 3, 0, 0, OWNER_PLAYER2},
	{TARGET, 2549, 0, 0, 0, OWNER_PLAYER2},
	{TARGET, 2678, 0, 0, 0, OWNER_PLAYER2},
	{TARGET, 2763, 1, 0, 0, OWNER_PLAYER2},

	{FLOCK,   370, 0, 370, 2630},
	{FLOCK,  1000, 0, 370, 2630},
	{FLOCK,  1630, 0, 370, 2630},
	{FLOCK,  2630, 0, 370, 2630},
};

GAMES swgames[1] = {
	{
		{
			{PLANE, 1270, 0,  901, 1835},
			{PLANE, 1720, 1, 1155, 2089},
			{PLANE,  588, 0,    0, 1154},
			{PLANE, 2456, 1, 2089, 9999},
			// Extra planes for multiplayer:
			{PLANE, 1330, 0,  901, 1835},
			{PLANE, 1360, 0,  901, 1835},
			{PLANE, 1630, 1, 1155, 2089},
			{PLANE, 1660, 1, 1155, 2089},
		},
		7491,
		original_targets,
		arrlen(original_targets),
		orground,
		MAX_X,
	},
};

//---------------------------------------------------------------------------
//
// $Log$
// Revision 1.3  2005/04/29 19:25:28  fraggle
// Update copyright to 2005
//
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

