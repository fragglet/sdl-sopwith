// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------------
//
// $Id: $
//
// Copyright(C) 1984-2000 David L. Clark
// Copyright(C) 2001 Simon Howard
//
// All rights reserved except as specified in the file license.txt.
// Distribution of this file without the license.txt file accompanying
// is prohibited.
//
//---------------------------------------------------------------------------
//
//        swgames  -      SW definition of games
//
//---------------------------------------------------------------------------


#include "sw.h"
#include "swgames.h"


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
	}
};

//---------------------------------------------------------------------------
//
// $Log: $
//
// sdh 21/10/2001: rearranged file headers, added cvs tags
// sdh 21/10/2001: reformatted with indent, adjusted some code by hand
//                 to make more readable
// sdh 20/10/2001: added parentheses to shut up compiler
//
// 87-03-09        Microsoft compiler.
// 84-02-08        Development
//
//---------------------------------------------------------------------------

