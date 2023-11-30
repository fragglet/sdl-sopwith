//
// Copyright(C) 1984-2000 David L. Clark
// Copyright(C) 2001-2005 Simon Howard
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or (at your
// option) any later version. This program is distributed in the hope that
// it will be useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//
// CGA Video Interface
//

#ifndef __VIDEO_H__
#define __VIDEO_H__

#include "sw.h"


typedef enum {
	KEY_UNKNOWN,
	KEY_PULLUP,
	KEY_PULLDOWN,
	KEY_FLIP,
	KEY_BOMB,
	KEY_FIRE,
	KEY_HOME,
	KEY_MISSILE,
	KEY_STARBURST,
	KEY_ACCEL,
	KEY_DECEL,
	KEY_SOUND,
	NUM_KEYS,
} sopkey_t;

// which keys are currently down
// this is actually a simple bitfield
// bit 0 is whether the button is currently down
// bit 1 is whether the button has been pressed
//       since the last call of Vid_GetGameKeys
// in this way, every button press will have an effect:
// if it is done based on what is currently down it is
// possible to miss keypresses (if you press and release
// a button fast enough)

extern int keysdown[NUM_KEYS];
extern int keybindings[NUM_KEYS];

extern bool vid_fullscreen;         // fullscreen

// init/shutdown

extern void Vid_Init(void);
extern void Vid_Reset(void);

// update screen

extern void Vid_Update(void);

extern bool Vid_GetCtrlBreak(void);

// video palette

void Vid_SetVideoPalette(int palette);
const char* Vid_GetVideoPaletteName(int palette);
int Vid_GetNumVideoPalettes(void);

// keyboard functions

extern int Vid_GetKey(void);
extern int Vid_GetChar(void);
extern int Vid_GetGameKeys(void);
extern const char *Vid_KeyName(int key);

// In text input mode, characters returned from Vid_GetChar() are "fully
// baked" - with shifting applied etc. Text input mode also pops up the
// on-screen keyboard if appropriate.
void Vid_StartTextInput(void);
void Vid_StopTextInput(void);

// drawing routines

void Vid_Box(int x, int y, int w, int h, int c);

// draw ground

extern void Vid_DispGround(GRNDTYPE *gptr);
extern void Vid_DispGround_Solid(GRNDTYPE *gptr);

// draw a pixel

extern void Vid_PlotPixel(int x, int y, int clr);
extern void Vid_XorPixel(int x, int y, int clr);

// draw a symbol

extern void Vid_DispSymbol(int x, int y, sopsym_t *symbol,
                           ob_owner_t owner);
extern int Vid_FuselageColor(ob_owner_t clr);

// clear screen

void Vid_ClearBuf(void);

char *Vid_GetPrefPath(void);

void error_exit(char *s, ...);

#endif
