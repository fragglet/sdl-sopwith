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
// Timer code
//
// Abstraction layer for access to the timer
// this just uses the standard C time routines
//
//--------------------------------------------------------------------------

#include <time.h>
#include "sw.h"

static int start;

// get time in milliseconds

int Timer_GetMS()
{

	return ((clock() - start) * 1000) / CLOCKS_PER_SEC;
}

void Timer_Init()
{
	start = clock();
}

//--------------------------------------------------------------------------
//
// $Log: $
//
// sdh 21/10/2001: added cvs tags
//
//--------------------------------------------------------------------------
