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
// Psion Framebuffer Video code
//
//-----------------------------------------------------------------------

#include <fcntl.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <sys/mman.h>

#include "video.h"

#include "sw.h"
#include "swconf.h"
#include "swmain.h"

#define FB_DEV "/dev/fb0"

// use the vga (8 bit) drawing routines

#include "vid_4bit.c"

BOOL vid_fullscreen = FALSE;
BOOL vid_double_size = TRUE;

static unsigned char *screenbuf;

static int fb_fd; 			// framebuffer filedescriptor
static int fb_w, fb_h;
static unsigned char *framebuffer;	// mmaped framebuffer
static unsigned char *scrbuf;		// screen buffer

static int ctrlbreak = 0;
static BOOL initted = 0;
static int colors[16];
static int keysdown[0xff];

//============================================================================
//
// input buffer
//
//============================================================================

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


//============================================================================
//
// Graphics Code
//
//============================================================================


// which keys are currently down
// this is actually a simple bitfield
// bit 0 is whether the button is currently down
// bit 1 is whether the button has been pressed
//       since the last call of Vid_GetGameKeys
// in this way, every button press will have an effect:
// if it is done based on what is currently down it is
// possible to miss keypresses (if you press and release
// a button fast enough)

//static int keysdown[SDLK_LAST];

static unsigned long getcolor(int r, int g, int b)
{
	return ((r+g+b) / 3) >> 4;
}


void Vid_Update()
{
	if (scrbuf) {
		// revo screen squishing

		char *p1, *p2;
		int y;

		for (p1=scrbuf, p2=framebuffer+32 + ((fb_w - SCR_WDTH) / 4), 
		     y=0;
		     y < 200;
		     y += 5, p1 += vid_pitch*5, p2 += vid_pitch << 2) 
			memcpy(p2, p1, vid_pitch << 2);
	}
}

static void set_icon(char *icon_file)
{
}

static void Vid_UnsetMode()
{
	munmap(framebuffer, fb_fd);
	close(fb_fd);
}

static void Vid_SetMode()
{
	struct fb_fix_screeninfo fixinfo;
	struct fb_var_screeninfo varinfo;

        fb_fd = open(FB_DEV, O_RDWR);

        if (fb_fd < 0) {
                printf("%s: cant open framebuffer\n", FB_DEV);
                return;
        }
        
	ioctl(fb_fd, KDSETMODE, KD_GRAPHICS);

        ioctl(fb_fd, FBIOGET_FSCREENINFO, &fixinfo);
        ioctl(fb_fd, FBIOGET_VSCREENINFO, &varinfo);

	if (varinfo.bits_per_pixel != 4) {
		printf("%s: this is not a 4 bit framebuffer!\n", FB_DEV);
		exit(-1);
	}

	fb_w = varinfo.xres;
	fb_h = varinfo.yres;

        printf("%s: %ix%ix%i\n", 
		FB_DEV, fb_w, fb_h,
                varinfo.bits_per_pixel);
	printf("pitch: %i\n", fixinfo.line_length);
	printf("smem_start: %i\n", fixinfo.smem_start);

	framebuffer = mmap(NULL, fixinfo.smem_len,
			   PROT_READ|PROT_WRITE, MAP_SHARED,
			   fb_fd, 0);

	if (framebuffer == MAP_FAILED) {
		printf("%s: cant mmap framebuffer\n", FB_DEV);
		exit(-1);
	}

	// we need to add 32 to the start of the framebuffer on the psion
	// (the first 32 bytes are the palette)

	// on the revo, we have to write to a seperate screen buffer
	// and then squash it down (160 pixel high screen)

	if (varinfo.yres == 160) {
		printf("revo screen squishing enabled\n");
		vid_pitch = fixinfo.line_length;
		scrbuf = malloc(vid_pitch * (SCR_HGHT + 1));
		vid_vram = scrbuf;
	} else if (varinfo.yres >= 200) {
		vid_pitch = fixinfo.line_length;
		scrbuf = NULL;
		vid_vram = framebuffer + 32;
	} else {
		printf("screen is not big enough\n");
	}
}

void Vid_Shutdown()
{
	Vid_UnsetMode();
}

void Vid_Init()
{
	Vid_SetMode();
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

void getevents()
{
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
/*
	if (keysdown[GDK_period]) {
		keysdown[GDK_period] = 0;
		c |= K_FLIP;
	}
	if (keysdown[GDK_comma])
		c |= K_FLAPU;
	if (keysdown[GDK_slash])
		c |= K_FLAPD;
	if (keysdown[GDK_x]) {
		keysdown[GDK_x] = 0;
		c |= K_ACCEL;
	}
	if (keysdown[GDK_z]) {
		keysdown[GDK_z] = 0;
		c |= K_DEACC;
	}
	if (keysdown[GDK_b])
		c |= K_BOMB;
	if (keysdown[GDK_space])
		c |= K_SHOT;
	if (keysdown[GDK_h])
		c |= K_HOME;
	if (keysdown[GDK_v]) {
		keysdown[GDK_v] = 0;
		c |= K_MISSILE;
	}
	if (keysdown[GDK_c]) {
		keysdown[GDK_c] = 0;
		c |= K_STARBURST;
	}
*/
	if (ctrlbreak) {
		c |= K_BREAK;
	}
	
	for (i=0; i<0xff; ++i) {
		keysdown[i] &= ~2;
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
// sdh 25/04/2002: rename vid4_{pitch,vram} to vid_{pitch,vram}
// sdh 20/04/2002: psion framebuffer port
// sdh 26/03/2002: now using platform specific code for drawing stuff
//                 (include "vid_vga.c")
//                 faster blitting to screen
//                 rename CGA_ to Vid_
//                 rename file to video.c
// sdh 17/11/2001: buffered input for keypresses
//                 CGA_GetLastKey->CGA_GetKey
// sdh 10/11/2001: Gtk+ Port
// sdh 07/11/2001: add CGA_Reset
// sdh 21/10/2001: added cvs tags
//
//-----------------------------------------------------------------------
