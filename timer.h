// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------------
//
// $Id: $
//
// Copyright(C) 2001 Simon Howard
//
// License as published by the Free Software Foundation, and the Sopwith
// respectively for more information.
//
//--------------------------------------------------------------------------
//
// Timer code
//
// Abstraction layer for access to the timer
// Basically this is to keep SDL out of the main code, so we can if we
// want drop in other code to run without SDL.
//
//--------------------------------------------------------------------------

#ifndef __TIMER_H__
#define __TIMER_H__

extern void Timer_Init();
extern int Timer_GetMS();

#endif

//---------------------------------------------------------------------------
//
// $Log: $
//
// sdh 21/10/2001: added cvs tags
//
//---------------------------------------------------------------------------

