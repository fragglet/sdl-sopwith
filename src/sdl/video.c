// Emacs style mode select -*- C++ -*-
//--------------------------------------------------------------------------
//
// $Id$
//
// Copyright(C) 2001-2003 Simon Howard
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

static SDL_Color cga_pal[] = {
#ifdef LCD
	{213, 226, 138}, {150, 160, 150}, 
	{120, 120, 160}, {0, 20, 200},
#else
	{0, 0, 0}, {0, 255, 255},
	{255, 0, 255}, {255, 255, 255},
#endif
};

BOOL vid_fullscreen = FALSE;
BOOL vid_double_size = TRUE;

static int ctrlbreak = 0;
static BOOL initted = 0;
static SDL_Surface *screen;
static SDL_Surface *screenbuf = NULL;        // draw into buffer in 2x mode
static int colors[16];

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

// convert a sopsym_t into a surface

SDL_Surface *surface_from_sopsym(sopsym_t *sym)
{
	SDL_Surface *surface = SDL_CreateRGBSurface(0, sym->w, sym->h, 8,
						    0, 0, 0, 0);
	char *p1, *p2;
	int y;

	if (!surface)
		return NULL;

	// set palette
	
	SDL_SetColors(surface, cga_pal, 0, sizeof(cga_pal)/sizeof(*cga_pal));	

	SDL_LockSurface(surface);

	p1 = sym->data;
	p2 = (unsigned char *) surface->pixels;

	// copy data from symbol into surface

	for (y=0; y<sym->h; ++y, p1 += sym->w, p2 += surface->pitch)
		memcpy(p2, p1, sym->w);

	SDL_UnlockSurface(surface);

	return surface;
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

static void set_icon(sopsym_t *sym)
{
	unsigned char *pixels;
	unsigned char *mask;
	SDL_Surface *icon = surface_from_sopsym(sym);
	int mask_size;
	int i;
	int x, y;

	if (!icon)
		return;

	// generate mask from icon

	mask_size = (icon->w * icon->h) / 8 + 1;

	mask = (unsigned char *)malloc(mask_size);

	SDL_LockSurface(icon);

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

	SDL_UnlockSurface(icon);

	// set icon

	SDL_WM_SetIcon(icon, mask);

	SDL_FreeSurface(icon);
	free(mask);
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

	printf("CGA Screen Emulation\n");
	printf("init screen: ");

	SDL_Init(SDL_INIT_VIDEO);

	srand(time(NULL));
	set_icon(symbol_plane[rand() % 2][rand() % 16]);

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

	for (n = 0; n < NUM_KEYS; ++n)
		keysdown[n] = 0;

	SDL_WM_SetCaption("SDL Sopwith", NULL);

	SDL_SetColors(screen, cga_pal, 0, sizeof(cga_pal)/sizeof(*cga_pal));
	SDL_SetColors(screenbuf, cga_pal, 0, sizeof(cga_pal)/sizeof(*cga_pal));
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

	SDL_LockSurface(screenbuf);
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

static sopkey_t translate_key(int sdl_key)
{
	switch (sdl_key) {
	case SDLK_LEFT:
	case SDLK_COMMA:
		return KEY_PULLUP;
	case SDLK_RIGHT:
	case SDLK_SLASH:
		return KEY_PULLDOWN;
	case SDLK_DOWN:
	case SDLK_PERIOD:
		return KEY_FLIP;
	case SDLK_x:
		return KEY_ACCEL;
	case SDLK_z:
		return KEY_DECEL;
	case SDLK_b:
		return KEY_BOMB;
	case SDLK_SPACE:
		return KEY_FIRE;
	case SDLK_h:
		return KEY_HOME;
	case SDLK_v:
		return KEY_MISSILE;
	case SDLK_c:
		return KEY_STARBURST;
	case SDLK_s:
		return KEY_SOUND;
	default:
		return KEY_UNKNOWN;
	}
}

static void getevents()
{
	SDL_Event event;
	static BOOL ctrldown = 0, altdown = 0;
	sopkey_t translated;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_LALT)
				altdown = 1;
			else if (event.key.keysym.sym == SDLK_LCTRL || event.key.keysym.sym == SDLK_RCTRL)
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
			translated = translate_key(event.key.keysym.sym);
			if (translated)
				keysdown[translated] |= 3;
			break;
		case SDL_KEYUP:
			if (event.key.keysym.sym == SDLK_LALT)
				altdown = 0;
			else if (event.key.keysym.sym == SDLK_LCTRL || event.key.keysym.sym == SDLK_RCTRL)
				ctrldown = 0;
			else {
				translated = translate_key(event.key.keysym.sym);
				if (translated)
					keysdown[translated] &= ~1;
			}
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

BOOL Vid_GetCtrlBreak()
{
	getevents();
	return ctrlbreak;
}

//-----------------------------------------------------------------------
// 
// $Log$
// Revision 1.3  2003/03/26 13:53:29  fraggle
// Allow control via arrow keys
// Some code restructuring, system-independent video.c added
//
// Revision 1.2  2003/03/26 12:02:38  fraggle
// Apply patch from David B. Harris (ElectricElf) for right ctrl key and
// documentation
//
// Revision 1.1.1.1  2003/02/14 19:03:37  fraggle
// Initial Sourceforge CVS import
//
//
// sdh 14/2/2003: change license header to GPL
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
