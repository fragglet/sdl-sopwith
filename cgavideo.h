// Emacs style mode select -*- C++ -*-
//-----------------------------------------------------------------------
//
// CGA Display
//
//-----------------------------------------------------------------------

#ifndef __CGAVIDEO_H__
#define __CGAVIDEO_H__

#include "sw.h"

extern void CGA_Init();
extern void CGA_Shutdown();
extern void CGA_Update();
extern char *CGA_GetVRAM();

extern BOOL cga_fullscreen;         // fullscreen
extern BOOL cga_double_size;        // x2 scale

#endif
