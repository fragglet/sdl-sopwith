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
// $Log: $
//
// sdh 16/11/2001: work properly when usleep is used (I stole the SDL
//                 timer code)
// sdh 10/11/2001: Gtk+ port
// sdh 21/10/2001: added cvs tags
//
//--------------------------------------------------------------------------
