//
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
// SDL Video Code
//

#include <string.h>
#include <time.h>
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "video.h"
#include "sw.h"
#include "swinit.h"

// lcd mode to emulate my old laptop i used to play sopwith on :)

//#define LCD

static SDL_Color cga_pal[] = {{}, {}, {}, {}};

typedef struct {
	char name[13]; // Up to 12 characters will display correctly on the menu
	SDL_Color color[4];
} VideoPalette;

VideoPalette VideoPalettes[] = {
	{"CGA 1", 		// CGA black, cyan, magenta, white (Sopwith's default color scheme)
		{{0, 0, 0}, {0, 255, 255}, {255, 0, 255}, {255, 255, 255}}},
	{"CGA 2", 		// CGA black, red, green, yellow
		{{0, 0, 0}, {0, 255, 0}, {255, 0, 0}, {255, 255, 0}}},
	{"CGA 3", 		// CGA black, cyan, red, white (aka CGA mode 5)
		{{0, 0, 0}, {0, 255, 255}, {255, 0, 0}, {255, 255, 255}}},
	{"Mono Amber",   // Shades of amber from a monochrome CGA display
		{{0, 0, 0}, {255, 170, 16}, {242, 125, 0}, {255, 226, 52}}},
	{"Mono Green", 	// Shades of green from a monochrome CGA display
		{{0, 0, 0}, {12, 238, 56}, {8, 202, 48}, {49, 253, 90}}},
	{"Mono Grey", 	// Shades of grey from a monochrome CGA display
		{{0, 0, 0}, {222, 222, 210}, {182, 186, 182}, {255, 255, 255}}},
	{"Tosh LCD 1",		// Toshiba laptop with STN panel
		{{213, 226, 138}, {150, 160, 150}, {120, 120, 160}, {0, 20, 200}}},
	{"Tosh LCD 2",		// Toshiba laptop with STN panel, reversed
		{{0, 20, 200}, {120, 120, 160}, {150, 160, 150}, {213, 226, 138}}},
	{"Tosh LCD 3",		// Toshiba T1000 with no backlight
		{{0x72, 0x88, 0x79}, {0x4b, 0x6e, 0x75},
		 {0x42, 0x5a, 0x75}, {0x27, 0x46, 0x6d}}},
	{"IBM LCD",  // IBM PC Convertible
		{{0x6b, 0x85, 0x88}, {0x56, 0x6b, 0x6e},
		 {0x42, 0x52, 0x54}, {0x2e, 0x39, 0x3b}}},
	{"Tandy LCD", // Tandy 1100FD
		{{0x48, 0xad, 0x68}, {0x36, 0x8c, 0x61},
		 {0x24, 0x6c, 0x5a}, {0x13, 0x4a, 0x54}}},
	{"Gas Plasma",
		{{0x7d, 0x1b, 0x02}, {0xd3, 0x41, 0x00},
		 {0xa8, 0x2e, 0x01}, {0xfe, 0x54, 0x00}}},
};

bool vid_fullscreen = false;

extern unsigned char *vid_vram;
extern unsigned int vid_pitch;
extern int gamenum;
extern bool isNetworkGame(void);

int keybindings[NUM_KEYS] = {
	0,                    // KEY_UNKNOWN
	SDL_SCANCODE_COMMA,   // KEY_PULLUP
	SDL_SCANCODE_SLASH,   // KEY_PULLDOWN
	SDL_SCANCODE_PERIOD,  // KEY_FLIP
	SDL_SCANCODE_B,       // KEY_BOMB
	SDL_SCANCODE_SPACE,   // KEY_FIRE
	SDL_SCANCODE_H,       // KEY_HOME
	SDL_SCANCODE_V,       // KEY_MISSILE
	SDL_SCANCODE_C,       // KEY_STARBURST
	SDL_SCANCODE_X,       // KEY_ACCEL
	SDL_SCANCODE_Z,       // KEY_DECEL
	SDL_SCANCODE_S,       // KEY_SOUND
};

static int ctrlbreak = 0;
static bool initted = 0;
static SDL_Window *window = NULL;
static uint32_t pixel_format;
static SDL_Renderer *renderer;

// Maximum number of pixels to use for intermediate scale buffer.
static int max_scaling_buffer_pixels = 16000000;

// These are (1) the 320x200x8 paletted buffer that we draw to (i.e. the one
// that holds vid_vram), (2) the 320x200x32 RGBA intermediate buffer that
// we blit the former buffer to, (3) the intermediate 320x200 texture that we
// load the RGBA buffer to and that we render into another texture (4) which
// is upscaled by an integer factor UPSCALE using "nearest" scaling and which
// in turn is finally rendered to screen using "linear" scaling.
static SDL_Surface *screenbuf = NULL;
static SDL_Surface *argbbuffer = NULL;
static SDL_Texture *texture = NULL;
static SDL_Texture *texture_upscaled = NULL;


// convert a sopsym_t into a surface
#define ICON_SCALE 4
static SDL_Surface *surface_from_sopsym(sopsym_t *sym)
{
	SDL_Surface *surface = SDL_CreateRGBSurface(
		0, sym->w * ICON_SCALE, sym->h * ICON_SCALE, 32,
		0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);

	unsigned char *src;
	uint32_t *dst;
	int x, y, sx, sy;

	if (surface == NULL) {
		return NULL;
	}

	// set palette
	SDL_LockSurface(surface);

	// copy data from symbol into surface
	for (y = 0; y < surface->h; ++y) {
		sy = y / ICON_SCALE;
		src = sym->data + sy * sym->w;
		dst = (uint32_t *) ((uint8_t *) surface->pixels +
		                    y * surface->pitch);
		for (x = 0; x < surface->w; ++x) {
			SDL_Color *p;
			sx = x / ICON_SCALE;
			if (src[sx] == 0) {
				dst[x] = 0;
				continue;
			}
			p = &cga_pal[src[sx]];
			dst[x] = (p->r << 24) | (p->g << 16)
			       | (p->b << 8) | 0xff;
		}
	}

	SDL_UnlockSurface(surface);

	return surface;
}

// 2x scale

void Vid_Update(void)
{
	static SDL_Rect blit_rect = { 0, 0, SCR_WDTH, SCR_HGHT };

	if (!initted) {
		Vid_Init();
	}

	SDL_UnlockSurface(screenbuf);

	// Blit from the paletted 8-bit screen buffer to the intermediate
	// 32-bit RGBA buffer that we can load into the texture.
	SDL_LowerBlit(screenbuf, &blit_rect, argbbuffer, &blit_rect);

	// Update intermediate texture with the contents of the RGBA buffer.
	SDL_UpdateTexture(texture, NULL, argbbuffer->pixels, argbbuffer->pitch);

	// Make sure the pillarboxes are kept clear each frame.
	SDL_RenderClear(renderer);

	// Render this intermediate texture into the upscaled texture
	// using "nearest" integer scaling.
	SDL_SetRenderTarget(renderer, texture_upscaled);
	SDL_RenderCopy(renderer, texture, NULL, NULL);

	// Finally, render this upscaled texture to screen using linear scaling.
	SDL_SetRenderTarget(renderer, NULL);
	SDL_RenderCopy(renderer, texture_upscaled, NULL, NULL);

	// Draw!
	SDL_RenderPresent(renderer);

	SDL_LockSurface(screenbuf);
}

static bool is_special_day(void)
{
	time_t now = time(NULL);
	struct tm *t = localtime(&now);

	return
	    // 18 January 1888, birth date of Thomas Sopwith:
	    (t->tm_mon == 0 && t->tm_mday == 18)
	    // 15 December 1913, founding of the Sopwith Aviation Company:
	 || (t->tm_mon == 11 && t->tm_mday == 15)
	    // 22 December 1916, first flight of the Sopwith Camel:
	 || (t->tm_mon == 11 && t->tm_mday == 22)
	    // 11 November 1918, Armistice Day:
	 || (t->tm_mon == 10 && t->tm_mday == 11)
	    // 24 April 1984, date from the original Sopwith documentation
	    // and assumed to be the original Sopwith release date?
	 || (t->tm_mon == 3 && t->tm_mday == 24);
}

static void set_icon(void)
{
	SDL_Surface *icon;
	sopsym_t *sym;

	if (is_special_day()) {
		sym = symbol_plane[1][0];
	} else {
		sym = symbol_plane[0][0];
	}

	icon = surface_from_sopsym(sym);
	if (icon == NULL) {
		return;
	}

	// set icon
	SDL_SetWindowIcon(window, icon);
	SDL_FreeSurface(icon);
}

static void LimitTextureSize(int *w_upscale, int *h_upscale)
{
	SDL_RendererInfo rinfo;
	int orig_w, orig_h;

	orig_w = *w_upscale;
	orig_h = *h_upscale;

	// Query renderer and limit to maximum texture dimensions of hardware:
	if (SDL_GetRendererInfo(renderer, &rinfo) != 0) {
		error_exit("CreateUpscaledTexture: SDL_GetRendererInfo() "
		           "call failed: %s", SDL_GetError());
	}

	while (*w_upscale * SCR_WDTH > rinfo.max_texture_width) {
		--*w_upscale;
	}
	while (*h_upscale * SCR_HGHT > rinfo.max_texture_height) {
		--*h_upscale;
	}

	if ((*w_upscale < 1 && rinfo.max_texture_width > 0)
	 || (*h_upscale < 1 && rinfo.max_texture_height > 0)) {
		error_exit("CreateUpscaledTexture: Can't create a "
		           "texture big enough for the whole screen! "
		           "Maximum texture size %dx%d",
		           rinfo.max_texture_width, rinfo.max_texture_height);
	}

	// We limit the amount of texture memory used for the intermediate buffer,
	// since beyond a certain point there are diminishing returns. Also,
	// depending on the hardware there may be performance problems with very
	// huge textures, so the user can use this to reduce the maximum texture
	// size if desired.
	if (max_scaling_buffer_pixels < SCR_WDTH * SCR_HGHT) {
		error_exit("CreateUpscaledTexture: max_scaling_buffer_"
		           "pixels too small to create a texture buffer:"
		           " %d < %d", max_scaling_buffer_pixels,
		           SCR_WDTH * SCR_HGHT);
	}

	while (*w_upscale * *h_upscale * SCR_WDTH * SCR_HGHT
	       > max_scaling_buffer_pixels) {
		if (*w_upscale > *h_upscale) {
			--*w_upscale;
		} else {
			--*h_upscale;
		}
	}

	if (*w_upscale != orig_w || *h_upscale != orig_h) {
		printf("CreateUpscaledTexture: Limited texture size to %dx%d "
		       "(max %d pixels, max texture size %dx%d)",
		       *w_upscale * SCR_WDTH, *h_upscale * SCR_HGHT,
		       max_scaling_buffer_pixels, rinfo.max_texture_width,
		       rinfo.max_texture_height);
	}
}

static void CreateUpscaledTexture(int force)
{
	static int h_upscale_old, w_upscale_old;
	int w, h;
	int h_upscale, w_upscale;
	SDL_Texture *new_texture, *old_texture;

	// Get the size of the renderer output. The units this gives us will be
	// real world pixels, which are not necessarily equivalent to the
	// screen's window size (because of highdpi).
	if (SDL_GetRendererOutputSize(renderer, &w, &h) != 0) {
		error_exit("failed to get renderer size: %s", SDL_GetError());
	}

	// When the screen or window dimensions do not match the aspect ratio
	// of the texture, the rendered area is scaled down to fit. Calculate
	// the actual dimensions of the rendered area.
	if (w * SCR_HGHT < h * SCR_WDTH) {
		// Tall window.
		h = w * SCR_HGHT / SCR_WDTH;
	} else {
		// Wide window.
		w = h * SCR_WDTH / SCR_HGHT;
	}

	// Pick texture size the next integer multiple of the screen dimensions.
	// If one screen dimension matches an integer multiple of the original
	// resolution, there is no need to overscale in this direction.
	w_upscale = (w + SCR_WDTH - 1) / SCR_WDTH;
	h_upscale = (h + SCR_HGHT - 1) / SCR_HGHT;

	// Minimum texture dimensions of 320x200.
	if (w_upscale < 1) {
		w_upscale = 1;
	}
	if (h_upscale < 1) {
		h_upscale = 1;
	}

	LimitTextureSize(&w_upscale, &h_upscale);

	// Create a new texture only if the upscale factors have actually changed.
	if (h_upscale == h_upscale_old && w_upscale == w_upscale_old && !force) {
		return;
	}

	h_upscale_old = h_upscale;
	w_upscale_old = w_upscale;

	// Set the scaling quality for rendering the upscaled texture to "linear",
	// which looks much softer and smoother than "nearest" but does a better
	// job at downscaling from the upscaled texture to screen.
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

	new_texture = SDL_CreateTexture(
		renderer, pixel_format, SDL_TEXTUREACCESS_TARGET,
		w_upscale*SCR_WDTH, h_upscale*SCR_HGHT);

	old_texture = texture_upscaled;
	texture_upscaled = new_texture;

	if (old_texture != NULL) {
		SDL_DestroyTexture(old_texture);
	}
}

static void Vid_UnsetMode(void)
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

static void GetWindowSize(int *w, int *h)
{
	SDL_DisplayMode mode;
	int factor;

	*w = SCR_WDTH;
	*h = SCR_HGHT;

	if (SDL_GetDesktopDisplayMode(0, &mode) != 0) {
		return;
	}

	for (factor = 1; *w * factor < (mode.w / 2)
	              && *h * factor < (mode.h / 2); ++factor);

	*w *= factor;
	*h *= factor;
}

static void Vid_SetMode(void)
{
	int n;
	int w, h;
	int flags = 0, renderer_flags = 0;
	unsigned int rmask, gmask, bmask, amask;
	int bpp;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		error_exit("Unable to initialize video subsystem: %s",
		           SDL_GetError());
	}
	srand(time(NULL));

	GetWindowSize(&w, &h);

	flags = SDL_WINDOW_RESIZABLE;
	flags |= SDL_WINDOW_ALLOW_HIGHDPI;
	if (vid_fullscreen) {
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}

	window = SDL_CreateWindow(PACKAGE_STRING, SDL_WINDOWPOS_CENTERED,
	                          SDL_WINDOWPOS_CENTERED, w, h, flags);

	if (window == NULL) {
		error_exit("Failed to open SDL window: %s", SDL_GetError());
	}

        SDL_StopTextInput();

        pixel_format = SDL_GetWindowPixelFormat(window);

	for (n = 0; n < NUM_KEYS; ++n) {
		keysdown[n] = 0;
	}

	set_icon();
	SDL_ShowCursor(0);

	renderer_flags = SDL_RENDERER_PRESENTVSYNC;
	renderer = SDL_CreateRenderer(window, -1, renderer_flags);

	// Important: Set the "logical size" of the rendering context. At the
	// same time this also defines the aspect ratio that is preserved while
	// scaling and stretching the texture into the window.
	SDL_RenderSetLogicalSize(renderer, SCR_WDTH, SCR_HGHT);

	// Blank out the full screen area in case there is any junk in
	// the borders that won't otherwise be overwritten.
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);

	// Format of argbbuffer must match the screen pixel format because we
	// import the surface data into the texture.
	if (argbbuffer != NULL) {
		SDL_FreeSurface(argbbuffer);
		argbbuffer = NULL;
	}

	if (argbbuffer == NULL) {
		SDL_PixelFormatEnumToMasks(
			pixel_format, &bpp, &rmask, &gmask,
			&bmask, &amask);
		argbbuffer = SDL_CreateRGBSurface(
			0, SCR_WDTH, SCR_HGHT, bpp,
			rmask, gmask, bmask, amask);
		SDL_FillRect(argbbuffer, NULL, 0);
	}

	if (texture != NULL) {
		SDL_DestroyTexture(texture);
	}

	// Set the scaling quality for rendering the intermediate texture into
	// the upscaled texture to "nearest", which is gritty and pixelated and
	// resembles software scaling pretty well.
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

	// Create the intermediate texture that the RGBA surface gets loaded
	// into. The SDL_TEXTUREACCESS_STREAMING flag means that this
	// texture's content is going to change frequently.
	texture = SDL_CreateTexture(renderer, pixel_format,
	                            SDL_TEXTUREACCESS_STREAMING,
	                            SCR_WDTH, SCR_HGHT);

	// Initially create the upscaled texture for rendering to screen
	CreateUpscaledTexture(1);
}

void Vid_Shutdown(void)
{
	if (!initted) {
		return;
	}

	Vid_UnsetMode();

	SDL_FreeSurface(screenbuf);

	initted = 0;
}

void Vid_Init(void)
{
	if (initted) {
		return;
	}

	fflush(stdout);

	Vid_SetMode();

	screenbuf = SDL_CreateRGBSurface(0, SCR_WDTH, SCR_HGHT, 8,
	                                 0, 0, 0, 0);
	vid_vram = screenbuf->pixels;
	vid_pitch = screenbuf->pitch;
	SDL_SetPaletteColors(screenbuf->format->palette, cga_pal, 0,
	                     sizeof(cga_pal) / sizeof(*cga_pal));

	initted = 1;

	atexit(Vid_Shutdown);

	SDL_LockSurface(screenbuf);
}

void Vid_Reset(void)
{
	if (!initted) {
		return;
	}

	Vid_UnsetMode();
	Vid_SetMode();

	// need to redraw buffer to screen

	Vid_Update();
}

void Vid_SetVideoPalette(int palette)
{
	cga_pal[0] = VideoPalettes[palette].color[0];
	cga_pal[1] = VideoPalettes[palette].color[1];
	cga_pal[2] = VideoPalettes[palette].color[2];
	cga_pal[3] = VideoPalettes[palette].color[3];
	SDL_SetPaletteColors(screenbuf->format->palette, cga_pal, 0, sizeof(cga_pal) / sizeof(*cga_pal));
	Vid_Update();
}

const char* Vid_GetVideoPaletteName(int palette)
{
        return VideoPalettes[palette].name;
}

int Vid_GetNumVideoPalettes(void)
{
    int numPalettes = sizeof(VideoPalettes) / sizeof(VideoPalettes[0]);
	return numPalettes;
}

#define INPUT_BUFFER_LEN 32
static SDL_Keysym input_buffer[INPUT_BUFFER_LEN];
static int input_buffer_head = 0, input_buffer_tail = 0;

static void input_buffer_push(SDL_Keysym c)
{
	int tail_next = (input_buffer_tail + 1) % INPUT_BUFFER_LEN;
	if (tail_next == input_buffer_head) {
		return;
	}
	input_buffer[input_buffer_tail] = c;
	input_buffer_tail = tail_next;
}

static SDL_Keysym input_buffer_pop(void)
{
	SDL_Keysym result;

	if (input_buffer_head == input_buffer_tail) {
		result.sym = SDLK_UNKNOWN;
		result.scancode = SDL_SCANCODE_UNKNOWN;
		return result;
	}

	result = input_buffer[input_buffer_head];
	input_buffer_head = (input_buffer_head + 1) % INPUT_BUFFER_LEN;
	return result;
}

static sopkey_t translate_scancode(int sdl_scancode)
{
	int i;

	for (i = 1; i < NUM_KEYS; ++i) {
		if (keybindings[i] != 0 && sdl_scancode == keybindings[i]) {
			return i;
		}
	}

	return KEY_UNKNOWN;
}

// Special keys get passed through as input events even when text input mode
// is activated.
static bool IsSpecialKey(SDL_Keysym *k) {
	switch (k->sym) {
		case SDLK_ESCAPE:
		case SDLK_RETURN:
		case SDLK_BACKSPACE:
			return true;
		default:
			return false;
	}
}

static bool CtrlDown(void)
{
	return (SDL_GetModState() & KMOD_CTRL) != 0;
}

static bool AltDown(void)
{
	return (SDL_GetModState() & KMOD_ALT) != 0;
}

static void CtrlKeyPress(SDL_KeyCode k)
{
	switch (k) {
#ifndef NO_EXIT
	case SDLK_c:
	case SDLK_PAUSE:
		++ctrlbreak;
		if (ctrlbreak >= 3) {
			fprintf(stderr,
				"user aborted with 3 ^C's\n");
			exit(-1);
		}
		break;
#endif
	case SDLK_r:
		if (!isNetworkGame()) {
			gamenum = starting_level;
			swinitlevel();
		}
		break;
	case SDLK_q:
		if (!isNetworkGame()) {
			swrestart();
		}
		break;
	default:
		break;
	}
}

static void getevents(void)
{
	SDL_Event event;
	int need_redraw = 0;
	int i;
	sopkey_t translated;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_KEYDOWN:
			if (CtrlDown()) {
				CtrlKeyPress(event.key.keysym.sym);
			} else if (AltDown() && event.key.keysym.sym == SDLK_RETURN) {
#ifndef NO_FULLSCREEN
				vid_fullscreen = !vid_fullscreen;
				Vid_Reset();
				continue;
#endif
			}
			if (!SDL_IsTextInputActive()
			 || IsSpecialKey(&event.key.keysym)) {
				input_buffer_push(event.key.keysym);
			}
			translated = translate_scancode(
				event.key.keysym.scancode);
			if (translated != KEY_UNKNOWN) {
				keysdown[translated] |= 3;
			}
			break;

		case SDL_KEYUP:
			translated = translate_scancode(
				event.key.keysym.scancode);
			if (translated != KEY_UNKNOWN) {
				keysdown[translated] &= ~1;
			}
			break;

		case SDL_TEXTINPUT:
			for (i = 0; event.text.text[i] != '\0'; ++i) {
				char c = event.text.text[i];
				SDL_Keysym fake;
				if (c >= 0x80) {
					continue;
				}
				fake.sym = c;
				fake.scancode = SDL_SCANCODE_UNKNOWN;
				input_buffer_push(fake);
			}
			break;

		case SDL_WINDOWEVENT:
			switch (event.window.event) {
			case SDL_WINDOWEVENT_CLOSE:
				exit(0);
				break;

			default:
				// When we get one of these events, we redraw
				// the screen immediately, as we may be in a
				// menu waiting for a keypress.
				need_redraw = 1;
				break;
			}
		}
	}

	if (need_redraw) {
		Vid_Update();
	}
}

void Vid_StartTextInput(void)
{
	SDL_StartTextInput();
}

void Vid_StopTextInput(void)
{
	SDL_StopTextInput();
}

int Vid_GetKey(void)
{
	SDL_Keysym k;
	getevents();
	k = input_buffer_pop();
	return k.scancode;
}

int Vid_GetChar(void)
{
	int result;
	getevents();
	result = input_buffer_pop().sym;
	if (result == '\r') {
		result = '\n';
	}
	return result;
}

bool Vid_GetCtrlBreak(void)
{
	getevents();
	return ctrlbreak;
}

const char *Vid_KeyName(int key)
{
	return SDL_GetScancodeName(key);
}

// Not really video related code, but it had to go somewhere.
char *Vid_GetPrefPath(void)
{
	char *result = SDL_GetPrefPath("", PACKAGE_NAME);

	// If SDL_GetPrefPath() fails, we can't load or save a config file,
	// but at least we let the user play the game...
	if (result == NULL) {
		fprintf(stderr, "Vid_GetPrefPath: Failed to make preferences "
		                "directory: %s\n", SDL_GetError());
	}

	return result;
}

#ifdef HAVE_ISATTY
#include <unistd.h>
#else
int isatty(int fd)
{
	return 0;
}
#endif

void error_exit(char *s, ...)
{
	static char buf[128];
	va_list args;

	va_start(args, s);
	vsnprintf(buf, sizeof(buf), s, args);
	va_end(args);

	if (!isatty(1)) {
		if (SDL_ShowSimpleMessageBox(
			SDL_MESSAGEBOX_ERROR, "Error", buf, window) == 0) {
			exit(1);
		}
	}

	fprintf(stderr, "%s\n", buf);
	exit(1);
}
