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

#include "cgavideo.h"
#include "sw.h"

BOOL cga_fullscreen = FALSE;
BOOL cga_double_size = TRUE;

static char *vram;
static int ctrlbreak = 0;
static BOOL initted = 0;
static SDL_Surface *screen;
static int colors[16];

// which keys are currently down
// this is actually a simple bitfield
// bit 0 is whether the button is currently down
// bit 1 is whether the button has been pressed
//       since the last call of CGA_GetGameKeys
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

inline int getpixel(int x, int y)
{
	register int c, mask;
	register char *sptr;

	sptr = vram + ((y) * 160)
	    + ((x & 0xFFF0) >> 1)
	    + ((x & 0x0008) >> 3);
	mask = 0x80 >> (x &= 0x0007);

	c = (*sptr & mask)
	    | ((*(sptr + 2) & mask) << 1)
	    | ((*(sptr + 4) & mask) << 2)
	    | ((*(sptr + 6) & mask) << 3);

	return (c >> (7 - x)) & 0xff;
}

// 2x scale

static void CGA_UpdateScaled()
{
	register char *pixels = (char *) screen->pixels;
	register int pitch = screen->pitch * 2;
	register int x, y;

	SDL_LockSurface(screen);

	for (y = 0; y < 200; ++y) {
		register char *p = pixels;

		for (x = 0; x < 320; ) {
			p[0] = p[1] = p[640] = p[641]
			    = colors[getpixel(x, y)];
			p += 2; ++x;
		}

		pixels += pitch;
	}

	SDL_UnlockSurface(screen);
}

static void CGA_UpdateUnscaled()
{
	register char *pixels = (char *) screen->pixels;
	register int pitch = screen->pitch;
	register int x, y;

	SDL_LockSurface(screen);

	for (y = 0; y < 200; ++y) {
		
		register char *p = pixels;

		for (x = 0; x < 320; ++x) {

			*p++ = colors[getpixel(x, y)];
		}

		pixels += pitch;
	}

	SDL_UnlockSurface(screen);
}

void CGA_Update()
{
	if (!initted)
		CGA_Init();
	if (cga_double_size)
		CGA_UpdateScaled();
	else
 		CGA_UpdateUnscaled();

	SDL_UpdateRect(screen, 0, 0, screen->w, screen->h);
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

char *CGA_GetVRAM()
{
        return vram;
}


static void CGA_UnsetMode()
{
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

static void CGA_SetMode()
{
	int n;
	int w, h;
	int flags = 0;

	printf("CGA Screen Emulation\n");
	printf("init screen: ");

	SDL_Init(SDL_INIT_VIDEO);

	set_icon("icon.bmp");

	w = SCR_WDTH;
	h = SCR_HGHT;

	if (cga_double_size) {
		w *= 2;
		h *= 2;
	}

	flags = SDL_HWPALETTE;
	if (cga_fullscreen)
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

	for (n = 0; n < 4; n++) {
		colors[n * 4 + 0] = getcolor(0, 0, 0);
		colors[n * 4 + 1] = getcolor(0, 255, 255);
		colors[n * 4 + 2] = getcolor(255, 0, 255);
		colors[n * 4 + 3] = getcolor(255, 255, 255);
	}

        for (n = 0; n < 4; n++) {
                colors[n * 4 + 0] = getcolor(0, 0, 0);
                colors[n * 4 + 1] = getcolor(0, 255, 255);
                colors[n * 4 + 2] = getcolor(255, 0, 255);
                colors[n * 4 + 3] = getcolor(255, 255, 255);
        }

/*
  was in swgrpha.c:

  0x000,                  //   0 = black    background
  0x037,                  //   1 = blue     planes,targets,explosions
  0x700,                  //   2 = red      planes,targets,explosions
  0x777,                  //   3 = white    bullets
  0x000,                  //   4
  0x000,                  //   5
  0x000,                  //   6
  0x070,                  //   7 = green    ground 
  0x000,                  //   8
  0x433,                  //   9 = tan      oxen, birds 
  0x420,                  //  10 = brown    oxen
  0x320,                  //  11 = brown    bottom of ground display 
  0x000,                  //  12
  0x000,                  //  13
  0x000,                  //  14
  0x000                   //  15
*/
#if 0
	colors[0] = getcolor(0, 0, 0);
	colors[1] = getcolor(0, 0, 255);
	colors[2] = getcolor(255, 0, 0);
	colors[3] = getcolor(255, 255, 255);
	colors[7] = getcolor(0, 255, 0);
	colors[9] = getcolor(168, 0, 0);
	colors[10] = getcolor(168, 84, 0);
	colors[11] = getcolor(84, 42, 0);
#endif
	SDL_WM_SetCaption("SDL Sopwith", NULL);
}

void CGA_Shutdown()
{
	if (!initted)
		return;

	CGA_UnsetMode();
	free(vram);

	initted = 0;
}

void CGA_Init()
{
	if (initted)
		return;

	vram = (char *) malloc(SCR_WDTH * SCR_HGHT);

	fflush(stdout);

	CGA_SetMode();

	initted = 1;

	atexit(CGA_Shutdown);
}

void CGA_Reset()
{
	if (!initted)
		return;

	CGA_UnsetMode();
	CGA_SetMode();

	// need to redraw buffer to screen

	CGA_Update();
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
					cga_fullscreen = !cga_fullscreen;
					CGA_Reset();
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

int CGA_GetKey()
{
	int l;

	getevents();
	
	return input_buffer_pop();
}

int CGA_GetGameKeys()
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

BOOL CGA_GetCtrlBreak()
{
	getevents();
	return ctrlbreak;
}

void CGA_ClearScreen()
{
	//memset(vram, 0, 320 * 200 * 5);
//      CGA_Update();
}

//-----------------------------------------------------------------------
// 
// $Log: $
//
// sdh 17/11/2001: buffered input for keypresses, 
//                 CGA_GetLastKey->CGA_GetKey
// sdh 07/11/2001: add CGA_Reset
// sdh 21/10/2001: added cvs tags
//
//-----------------------------------------------------------------------
