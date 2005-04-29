// Emacs style mode select -*- C++ -*-
//--------------------------------------------------------------------------
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
//--------------------------------------------------------------------------
//
// Timer code
//
// Abstraction layer for access to the timer
// this just uses the standard C time routines
//
//--------------------------------------------------------------------------

#include <sys/time.h>
#include <unistd.h>
#include "sw.h"

static struct timeval start;

// get time in milliseconds

int Timer_GetMS()
{
        struct timeval nowtime;
	int ms;

        gettimeofday(&nowtime, NULL);
        ms = (nowtime.tv_sec - start.tv_sec) * 1000
		+ (nowtime.tv_usec-start.tv_usec)/1000;
        return ms;
}

void Timer_Sleep(int usec)
{
	usleep(usec);
}

void Timer_Init()
{
	gettimeofday(&start, NULL);
}

//--------------------------------------------------------------------------
//
// $Log$
// Revision 1.2  2005/04/29 19:25:29  fraggle
// Update copyright to 2005
//
// Revision 1.1.1.1  2003/02/14 19:03:35  fraggle
// Initial Sourceforge CVS import
//
//
// sdh 14/2/2003: change license header to GPL
// sdh 16/11/2001: work properly when usleep is used (I stole the SDL
//                 timer code)
// sdh 10/11/2001: Gtk+ port
// sdh 21/10/2001: added cvs tags
//
//--------------------------------------------------------------------------
