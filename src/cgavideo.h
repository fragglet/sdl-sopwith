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
// CGA Video Interface
//
//-----------------------------------------------------------------------

#ifndef __CGAVIDEO_H__
#define __CGAVIDEO_H__

#include "sw.h"

extern void CGA_Init();
extern void CGA_Shutdown();
extern void CGA_Reset();
extern void CGA_Update();

extern BOOL CGA_GetCtrlBreak();
extern int CGA_GetKey();
extern int CGA_GetGameKeys();
extern char *CGA_GetVRAM();

extern BOOL cga_fullscreen;         // fullscreen
extern BOOL cga_double_size;        // x2 scale

#endif

//-----------------------------------------------------------------------
//
// $Log: $
//
// sdh 21/10/2001: added cvs tags
//
//-----------------------------------------------------------------------
