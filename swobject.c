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
//        swobject -      SW object allocation and deallocation
//
//---------------------------------------------------------------------------

#include "sw.h"
#include "swmain.h"
#include "swobject.h"

OBJECTS *allocobj()
{
	register OBJECTS *ob;

	if (!objfree)
		return NULL;

	ob = objfree;
	objfree = ob->ob_next;

	ob->ob_next = NULL;
	ob->ob_prev = objbot;

	if (objbot)
		objbot->ob_next = ob;
	else
		objtop = ob;

	ob->ob_sound = NULL;
	ob->ob_drwflg = ob->ob_delflg = 0;
	if (ob > objsmax)
		objsmax = ob;
	return (objbot = ob);
}



void deallobj(OBJECTS * obp)
{
	register OBJECTS *ob=obp;
	register OBJECTS *obb = ob->ob_prev;

	if (obb)
		obb->ob_next = ob->ob_next;
	else
		objtop = ob->ob_next;

	obb = ob->ob_next;

	if (obb)
		obb->ob_prev = ob->ob_prev;
	else
		objbot = ob->ob_prev;

	ob->ob_next = 0;
	if (delbot)
		delbot->ob_next = ob;
	else
		deltop = ob;

	delbot = ob;


}

//---------------------------------------------------------------------------
//
// $Log: $
//
// sdh 21/10/2001: rearranged headers, added cvs tags
// sdh 21/10/2001: reformatted with indent. adjusted some code by hand
//                 to make more readable
// sdh 19/10/2001: removed externs (now in headers)
// sdh 18/10/2001: converted all functions to ANSI-style arguments
//
//
// 87-03-09        Microsoft compiler.
// 84-10-31        Atari
// 84-06-12        PCjr Speed-up
// 84-02-07        Development
//
//---------------------------------------------------------------------------

