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
// SDL Video Code
//
// By Simon Howard
//
//-----------------------------------------------------------------------

#include <SDL.h>

#include "video.h"
#include "sw.h"

#include "vid_vga.c"

// lcd mode to emulate my old laptop i used to play sopwith on :)

//#define LCD

BOOL vid_fullscreen = FALSE;
BOOL vid_double_size = TRUE;

static int ctrlbreak = 0;
static BOOL initted = 0;
static SDL_Surface *screen;
static SDL_Surface *screenbuf = NULL;        // draw into buffer in 2x mode
static int colors[16];

// which keys are currently down
// this is actually a simple bitfield
// bit 0 is whether the button is currently down
// bit 1 is whether the button has been pressed
//       since the last call of Vid_GetGameKeys
// in this way, every button press will have an effect:
// if it is done based on what is currently down it is
// possible to miss keypresses (if you press and release
// a button fast enough)

static int keysdown[SDLK_LAST];

static int getcolor(int r, int g, int b)
{
	SDL_Palette *pal = screen->format->palette;
	int i;
	int nearest = 0xffff, n = 0;

	for (i = 0; i < pal->ncolors; ++i) {
		int diff =
		    (r - pal->colors[i].r) * (r - pal->colors[i].r) +
		    (g - pal->colors[i].g) * (g - pal->colors[i].g) +
		    (b - pal->colors[i].b) * (b - pal->colors[i].b);

//              printf("%i, %i, %i\n",
//                     pal->colors[i].r, pal->colors[i].g,
//                     pal->colors[i].b);

		if (!diff)
			return i;

		if (diff < nearest) {
			nearest = diff;
			n = i;
		}
	}

	return n;
}

// 2x scale

static void Vid_UpdateScaled()
{
	register char *pixels = (char *) screen->pixels;
	register char *pixels2 = (char *) screenbuf->pixels;
	register int pitch = screen->pitch;
	register int pitch2 = screenbuf->pitch;
	register int x, y;

	SDL_LockSurface(screen);
	SDL_LockSurface(screenbuf);

	for (y = 0; y < SCR_HGHT; ++y) {
		register char *p = pixels;
		register char *p2 = pixels2;

		for (x=0; x<SCR_WDTH; ++x) {
			p[0] = p[1] =  p[pitch] = p[pitch + 1] = *p2++;
			p += 2;
		}

		pixels += pitch * 2;
		pixels2 += pitch2;
	}

	SDL_UnlockSurface(screenbuf);
	SDL_UnlockSurface(screen);
}

void Vid_Update()
{
	if (!initted)
		Vid_Init();

	SDL_UnlockSurface(screenbuf);

	if (vid_double_size)
		Vid_UpdateScaled();
	else 
		SDL_BlitSurface(screenbuf, NULL, screen, NULL);

	SDL_UpdateRect(screen, 0, 0, screen->w, screen->h);

	SDL_LockSurface(screenbuf);
}

static void set_icon(char *icon_file)
{
	unsigned char *pixels;
	unsigned char *mask;
	SDL_Surface *icon = SDL_LoadBMP(icon_file);
	int mask_size;
	int i;
	int x, y;

	if (!icon) {
		fprintf(stderr,
			"set_icon: cant load %s\n", icon_file);
		return;
	}

	// generate mask from icon

	mask_size = (icon->w * icon->h) / 8 + 1;

	mask = (unsigned char *)malloc(mask_size);

	pixels = (unsigned char *)icon->pixels;

	i = 0;

	for (y=0; y<icon->h; y++) {
		for (x=0; x<icon->w; x++) {
			if (i % 8) {
				mask[i / 8] <<= 1;
			} else {
				mask[i / 8] = 0;
			}

			if (pixels[i]) 
				mask[i / 8] |= 0x01;

			++i;
		}
	}

	// set icon

	SDL_WM_SetIcon(icon, mask);
}


static void Vid_UnsetMode()
{
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

static void Vid_SetMode()
{
	int n;
	int w, h;
	int flags = 0;
	SDL_Color pal[5] = {
		{0, 0, 0}, {0, 255, 255},
      		{255, 0, 255}, {255, 255, 255},
	};

	printf("CGA Screen Emulation\n");
	printf("init screen: ");

	SDL_Init(SDL_INIT_VIDEO);

	set_icon("icon.bmp");

	w = SCR_WDTH;
	h = SCR_HGHT;

	if (vid_double_size) {
		w *= 2;
		h *= 2;
	}

	flags = SDL_HWPALETTE;
	if (vid_fullscreen)
		flags |= SDL_FULLSCREEN;

	screen = SDL_SetVideoMode(w, h, 8, flags);

	if (screen) {
		printf("initialised\n");
	} else {
		printf("failed\n");
		fprintf(stderr, "cant init SDL\n");
		exit(-1);
	}

	SDL_EnableUNICODE(1);

	for (n = 0; n < SDLK_LAST; ++n)
		keysdown[n] = 0;

#ifdef LCD
	pal[0].r = 213; pal[0].g = 226; pal[0].b = 138;
	pal[1].r = 150; pal[1].g = 160; pal[1].b = 150;
	pal[2].r = 120; pal[2].g = 120; pal[2].b = 160;
	pal[3].r = 0;   pal[3].g = 20;  pal[3].b = 200;
#endif

	SDL_SetColors(screen, pal, 0, 4);

	SDL_WM_SetCaption("SDL Sopwith", NULL);

	SDL_SetColors(screenbuf, pal, 0, 4);		

	SDL_LockSurface(screenbuf);
}

void Vid_Shutdown()
{
	if (!initted)
		return;

	Vid_UnsetMode();

	SDL_FreeSurface(screenbuf);

	initted = 0;
}

void Vid_Init()
{
	if (initted)
		return;

	fflush(stdout);

	screenbuf = SDL_CreateRGBSurface(0, SCR_WDTH, SCR_HGHT, 8,
					 0, 0, 0, 0);
	
	vid_vram = screenbuf->pixels;
	vid_pitch = screenbuf->pitch;


	Vid_SetMode();

	initted = 1;

	atexit(Vid_Shutdown);
}

void Vid_Reset()
{
	if (!initted)
		return;

	Vid_UnsetMode();
	Vid_SetMode();

	// need to redraw buffer to screen

	Vid_Update();
}

static int input_buffer[128];
static int input_buffer_head=0, input_buffer_tail=0;

static void input_buffer_push(int c)
{
	input_buffer[input_buffer_tail++] = c;
	input_buffer_tail %= sizeof(input_buffer) / sizeof(*input_buffer);
}

static int input_buffer_pop()
{
	int c;

	if (input_buffer_head == input_buffer_tail)
		return 0;

	c = input_buffer[input_buffer_head++];

	input_buffer_head %= sizeof(input_buffer) / sizeof(*input_buffer);

	return c;
}

static void getevents()
{
	SDL_Event event;
	static BOOL ctrldown = 0, altdown = 0;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_LALT)
				altdown = 1;
			else if (event.key.keysym.sym == SDLK_LCTRL)
				ctrldown = 1;
			else if (ctrldown &&
				 (event.key.keysym.sym == SDLK_c ||
				  event.key.keysym.sym == SDLK_BREAK)) {
				++ctrlbreak;
				if (ctrlbreak >= 3) {
					fprintf(stderr,
						"user aborted with 3 ^C's\n");
					exit(-1);
				}
			} else if (event.key.keysym.sym == SDLK_ESCAPE) {
				input_buffer_push(27);
			} else if (event.key.keysym.sym == SDLK_RETURN) {
				if(altdown) {
					vid_fullscreen = !vid_fullscreen;
					Vid_Reset();
				} else {
					input_buffer_push('\n');
				}
 			} else {
				input_buffer_push(event.key.keysym.unicode & 0x7f);
			}
			keysdown[event.key.keysym.sym] |= 3;
			break;
		case SDL_KEYUP:
			if (event.key.keysym.sym == SDLK_LALT)
				altdown = 0;
			else if (event.key.keysym.sym == SDLK_LCTRL)
				ctrldown = 0;
			else
				keysdown[event.key.keysym.sym] &= ~1;
			break;
		}
	}
}

int Vid_GetKey()
{
	int l;

	getevents();
	
	return input_buffer_pop();
}

int Vid_GetGameKeys()
{
	int i, c = 0;

	getevents();

	while (input_buffer_pop());

	if (keysdown[SDLK_PERIOD]) {
		keysdown[SDLK_PERIOD] = 0;
		c |= K_FLIP;
	}
	if (keysdown[SDLK_COMMA])
		c |= K_FLAPU;
	if (keysdown[SDLK_SLASH])
		c |= K_FLAPD;
	if (keysdown[SDLK_x]) {
		keysdown[SDLK_x] = 0;
		c |= K_ACCEL;
	}
	if (keysdown[SDLK_z]) {
		keysdown[SDLK_z] = 0;
		c |= K_DEACC;
	}
	if (keysdown[SDLK_s]) {
		keysdown[SDLK_s] = 0;
		c |= K_SOUND;
	}
	if (keysdown[SDLK_b])
		c |= K_BOMB;
	if (keysdown[SDLK_SPACE])
		c |= K_SHOT;
	if (keysdown[SDLK_h])
		c |= K_HOME;
	if (keysdown[SDLK_v]) {
		keysdown[SDLK_v] = 0;
		c |= K_MISSILE;
	}
	if (keysdown[SDLK_c]) {
		keysdown[SDLK_c] = 0;
		c |= K_STARBURST;
	}
	if (ctrlbreak) {
		c |= K_BREAK;
	}
	
	for (i=0; i<SDLK_LAST; ++i) {
		keysdown[i] &= ~2;
//		if (keysdown[i] & 2 && !(keysdown[i] & 1))
//			keysdown[i] = 0;
	}

	return c;
}

BOOL Vid_GetCtrlBreak()
{
	getevents();
	return ctrlbreak;
}

//-----------------------------------------------------------------------
// 
// $Log: $
//
// sdh 25/04/2002: rename vga_{pitch,vram} to vid_{pitch,vram}
// sdh 26/03/2002: now using platform specific vga code for drawing stuff
//                 (#include "vid_vga.c")
//                 rename CGA_ to Vid_
// sdh 17/11/2001: buffered input for keypresses, 
//                 CGA_GetLastKey->CGA_GetKey
// sdh 07/11/2001: add CGA_Reset
// sdh 21/10/2001: added cvs tags
//
//-----------------------------------------------------------------------
