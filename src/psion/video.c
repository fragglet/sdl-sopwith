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
// Psion Framebuffer Video code
//
// Some parts of this are from picogui. Thanks scanline :P
//
//-----------------------------------------------------------------------

#include <fcntl.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <sys/mman.h>
#include <signal.h>
#include <termios.h>

#include "video.h"

#include "sw.h"
#include "swconf.h"
#include "swmain.h"

#define SIGVT SIGUSR1

#define KB_DEV "/dev/tty"
#define FB_DEV "/dev/fb0"
#define FB_DEV_DEVFS "/dev/fb/0"

// use the vga (8 bit) drawing routines

#include "vid_4bit.c"

BOOL vid_fullscreen = FALSE;
BOOL vid_double_size = TRUE;

static unsigned char *screenbuf;

static int fb_fd; 			// framebuffer file descriptor
static int fb_w, fb_h;
static BOOL fb_enabled = TRUE;
static unsigned char *framebuffer;	// mmaped framebuffer
static unsigned char *scrbuf;		// screen buffer

static int kb_fd;                       // keyboard file descriptor
static struct termios kb_oldtio;

static int ctrlbreak = 0;
static int colors[16];

static BOOL in_vid_mode = 0;

//============================================================================
//
// Graphics Code
//
//============================================================================


static unsigned long getcolor(int r, int g, int b)
{
	return ((r+g+b) / 3) >> 4;
}


void Vid_Update()
{
	if (!fb_enabled)
		return;

	if (scrbuf) {
		// revo screen squishing

		char *p1, *p2;
		int y, x;

		p1 = scrbuf;
		p2 = (framebuffer+32) + (fb_w - SCR_WDTH) / 4;

		for (y=0; y<200; y += 5) {
			memcpy(p2, p1, vid_pitch * 4);
			p1 += vid_pitch * 4;
			p2 += vid_pitch * 3;
			for (x=0; x<vid_pitch; ++x) {
				*p2++ &= *p1++;
			}
		}
	}
}

static int fbdev_getvt(void) {
	struct vt_stat stat;
	ioctl(kb_fd, VT_GETSTATE, &stat);
	return stat.v_active;
}

static void toggle_fb_enabled()
{
	fb_enabled = !fb_enabled;

	if (fb_enabled) {
		ioctl(kb_fd, VT_RELDISP, VT_ACKACQ);
		if (in_vid_mode)
			Vid_Update();

	} else {
		ioctl(kb_fd, VT_RELDISP, 1);
	}
}

static void Vid_UnsetMode()
{
	if (!in_vid_mode)
		return;

	in_vid_mode = 0;

	// text mode

	ioctl(kb_fd, KDSETMODE, KD_TEXT);

	// this is uberlame.
	// switch to the next console and back again, this will 
	// redraw the console
	{
		int n = fbdev_getvt();
		ioctl(kb_fd, VT_ACTIVATE, n+1);
		ioctl(kb_fd, VT_ACTIVATE, n);
	}

	// shutdown screen

	munmap(framebuffer, fb_fd);
	close(fb_fd);

	// shutdown keyboard

	tcsetattr(kb_fd, TCSAFLUSH, &kb_oldtio);
	close(kb_fd);
}

static void Vid_SetMode()
{
	struct fb_fix_screeninfo fixinfo;
	struct fb_var_screeninfo varinfo;
	char *device;

	device = FB_DEV;
        fb_fd = open(device, O_RDWR);

	if (fb_fd < 0) {
		device = FB_DEV_DEVFS;
		fb_fd = open(device, O_RDWR);

		if (fb_fd < 0) {
			fprintf(stderr, "cant open framebuffer\n");
			exit(-1);
		}
	}
        
	ioctl(fb_fd, KDSETMODE, KD_GRAPHICS);
        ioctl(fb_fd, FBIOGET_FSCREENINFO, &fixinfo);
        ioctl(fb_fd, FBIOGET_VSCREENINFO, &varinfo);

	if (varinfo.bits_per_pixel != 4) {
		fprintf(stderr,
			"%s: this is not a 4 bit framebuffer!\n", device);
		exit(-1);
	}

	fb_w = varinfo.xres;
	fb_h = varinfo.yres;

        printf("%s: %ix%ix%i\n", device, fb_w, fb_h, varinfo.bits_per_pixel);

	framebuffer = mmap(NULL, fixinfo.smem_len,
			   PROT_READ|PROT_WRITE, MAP_SHARED,
			   fb_fd, 0);

	if (framebuffer == MAP_FAILED) {
		fprintf(stderr, "%s: cant mmap framebuffer\n", device);
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

	// open keyboard

	kb_fd = open(KB_DEV, O_NONBLOCK);

	if (kb_fd < 0) {
		fprintf(stderr, "%s: cant open\n", KB_DEV);
		exit(-1);
	}

	// save attributes, turn off console echo

	if (tcgetattr(kb_fd, &kb_oldtio) < 0) {
		printf("%s: cant get attributes\n", KB_DEV);
		exit(-1);
	} else {
		struct termios new_tio;

		new_tio = kb_oldtio;
		new_tio.c_lflag &= ~(ECHO|ICANON|IEXTEN|ISIG);
		new_tio.c_iflag &= ~(ICRNL|INPCK|ISTRIP|IXON|BRKINT);
		new_tio.c_cflag &= ~(CSIZE|PARENB);

		tcsetattr(kb_fd, TCSAFLUSH, &new_tio);
	}

	ioctl(kb_fd, KDSETMODE, KD_GRAPHICS);

	{
		struct vt_mode mode;
		
		// notify us of vt changes
		ioctl(kb_fd, VT_GETMODE, &mode);
		mode.mode = VT_PROCESS;
		mode.relsig = SIGVT;
		mode.acqsig = SIGVT;
		ioctl(kb_fd, VT_SETMODE, &mode);

		fb_enabled = TRUE;
	}

	in_vid_mode = TRUE;
}

void Vid_Shutdown()
{
	if (in_vid_mode)
		Vid_UnsetMode();
}

void Vid_Init()
{
	Vid_SetMode();

	signal(SIGVT, toggle_fb_enabled);
	atexit(Vid_Shutdown);
}

void Vid_Reset()
{
	Vid_UnsetMode();
	Vid_SetMode();

	// need to redraw buffer to screen

	Vid_Update();
}

static inline sopkey_t translate_key(int c)
{
	switch (tolower(c)) {
		// use j,k and l because of the keyboard layout
	case 'k': 
		return KEY_FLIP;
	case 'j':
		return KEY_PULLUP;
	case 'l':
		return KEY_PULLDOWN;
	case 'x':
		return KEY_ACCEL;
	case 'z':
		return KEY_DECEL;
	case 'b':
		return KEY_BOMB;
	case ' ':
		return KEY_FIRE;
	case 'h':
		return KEY_HOME;
	case 'v':
		return KEY_MISSILE;
	case 'c':
		return KEY_STARBURST;
	case 's':
		return KEY_SOUND;
	default: 
		return KEY_UNKNOWN;
	}
}

static inline int getkey()
{
	unsigned char c;
	sopkey_t translated;

	if (read(kb_fd, &c, 1) <= 0)
		return -1;

	translated = translate_key(c);

	// we dont actually know if a key is down or not,
	// only if it has been pressed since last run of 
	// Vid_GetGameKeys, so set the second bit only
	
	if (translated)
		keysdown[translated] |= 2;

	if (c == 0x3) {            // ctrl c
		ctrlbreak++;
		if (ctrlbreak >= 3) {
			printf("User aborted with 3 ^C's\n");
			exit(-1);
		}
		return -1;
	}

	return c;
}

int Vid_GetKey()
{
	int c = getkey();

	return c < 0 ? 0 : c;
}

BOOL Vid_GetCtrlBreak()
{
	return ctrlbreak > 0;
}


//-----------------------------------------------------------------------
// 
// $Log$
// Revision 1.3  2003/03/26 14:58:34  fraggle
// devfs support
// improved screen squishing for revo
//
// Revision 1.2  2003/03/26 14:29:50  fraggle
// Move psion code to new key architecture
//
// Revision 1.1.1.1  2003/02/14 19:03:38  fraggle
// Initial Sourceforge CVS import
//
//
// sdh 14/2/2003: change license header to GPL
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

