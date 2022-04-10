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
// Gtk Video Code
//
// By Simon Howard
//
//-----------------------------------------------------------------------

#include "video.h"

#include "sw.h"
#include "swconf.h"
#include "swmain.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

// use the vga (8 bit) drawing routines

#include "vid_vga.c"

BOOL vid_fullscreen = FALSE;
BOOL vid_double_size = TRUE;

static unsigned char *screenbuf;

static int ctrlbreak = 0;
static BOOL initted = 0;
static GtkWidget *window;
static GdkImage *screen;
static GtkWidget *screen_widget = NULL;
static GtkWidget *menubar;
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
// GUI
//
//============================================================================

static void hide_callback(GtkWidget *widget, gpointer data)
{
	GtkWidget *data_widget = (GtkWidget *)data;
	gtk_widget_hide(data_widget);
//puts("hide_callback");
}

static void delete_event(GtkWidget *widget, GdkEvent *event, gpointer data) {
	gtk_widget_hide(widget);
}

static void settings_bool_toggle(GtkWidget *widget, BOOL *b)
{
	*b = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)) != 0;
	if (b == &vid_fullscreen || b == &vid_double_size)
		Vid_Reset();
}

static void settings_dialog()
{
	static GtkWidget *window = NULL;

	if (!window) {
		GtkWidget *hbox, *vbox, *vbox2, *button;
		int i;

		window = gtk_dialog_new();
		vbox = GTK_DIALOG(window)->vbox;

		gtk_window_set_title(GTK_WINDOW(window), "Settings");

		gtk_signal_connect(GTK_OBJECT(window), "destroy",
				   GTK_SIGNAL_FUNC(delete_event), 
				   window);
		gtk_signal_connect(GTK_OBJECT(window), "delete_event",
				   GTK_SIGNAL_FUNC(delete_event), 
				   window);

		vbox = gtk_vbox_new(FALSE, 5);
		vbox2 = gtk_vbox_new(FALSE, 5);

		hbox = gtk_hbox_new(FALSE, 5);

		gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 10);
		gtk_box_pack_start(GTK_BOX(hbox), vbox2, TRUE, TRUE, 10);

		gtk_box_pack_start(GTK_BOX(GTK_DIALOG(window)->vbox),
				   hbox,
				   TRUE, TRUE, 10);

		for (i=0; i<num_confoptions; ++i) {
			GtkWidget *widget;
			
			switch (confoptions[i].type) {
			case CONF_BOOL:
				widget = gtk_check_button_new_with_label
					(confoptions[i].description);

				gtk_box_pack_start(GTK_BOX(i % 2 ? vbox2 : vbox),
						   widget, 
						   TRUE, TRUE, 0);
				gtk_signal_connect(GTK_OBJECT(widget),
						   "toggled",
						   GTK_SIGNAL_FUNC(settings_bool_toggle),
						   confoptions[i].value.b);
				gtk_toggle_button_set_active
					(GTK_TOGGLE_BUTTON(widget),
					 *confoptions[i].value.b);
				gtk_widget_show(widget);
			default:
				break;
			}
		}
		
		gtk_widget_show(vbox);
		gtk_widget_show(vbox2);
		gtk_widget_show(hbox);

		vbox = GTK_DIALOG(window)->action_area;

		// 29/6/2002: save button

		button = gtk_button_new_with_label("Save Settings");
		gtk_box_pack_start(GTK_BOX(vbox), button, TRUE, TRUE, 0);
		gtk_signal_connect(GTK_OBJECT(button), "clicked",
				   GTK_SIGNAL_FUNC(swsaveconf), NULL);
		gtk_widget_show(button);
		
		// close button

		button = gtk_button_new_with_label("Close");
		gtk_box_pack_start(GTK_BOX(vbox), button, TRUE, TRUE, 0);
		gtk_signal_connect(GTK_OBJECT(button), "clicked",
				   GTK_SIGNAL_FUNC(hide_callback), 
				   window);
		gtk_widget_show(button);
	}

	gtk_widget_show(window);
}

static void new_game(gpointer callback_data,
		     guint callback_action,
		     GtkWidget *widget)
{
	playmode = callback_action;
	longjmp(envrestart, 0);
}

static void about_window(gpointer callback_data, 
			 guint callback_action,
			 GtkWidget *widget)
{
	static GtkWidget *window = NULL;

	if (!window) {
		GtkWidget *label = gtk_label_new(
			"Gtk+ Sopwith\n"
			"Version " VERSION "\n"
			"Copyright(C) 1984, 1985, 1987 "
			"BMB Compuscience Canada Ltd.\n"
			"Copyright(C) 1984-2000 David L. Clark\n"
			"Copyright(C) 2001 Simon Howard\n");
		GtkWidget *button = gtk_button_new_with_label("Close");

		window = gtk_dialog_new(); 

		gtk_box_pack_start(GTK_BOX(GTK_DIALOG(window)->vbox),
				   label,
				   TRUE, TRUE, 10);

		gtk_widget_show(label);
	
		gtk_box_pack_start(GTK_BOX(GTK_DIALOG(window)->action_area),
				   button,
				   TRUE, TRUE, 0);

		gtk_signal_connect(GTK_OBJECT(button), "clicked",
				   GTK_SIGNAL_FUNC(hide_callback), window);

		gtk_widget_show(button);
		
		gtk_window_set_title(GTK_WINDOW(window), 
				     "About Sopwith");

		gtk_signal_connect(GTK_OBJECT(window), "destroy",
				   GTK_SIGNAL_FUNC(delete_event), window);
		gtk_signal_connect(GTK_OBJECT(window), "delete_event",
				   GTK_SIGNAL_FUNC(delete_event), window);
	}

	gtk_widget_show(window);
}

static GtkItemFactoryEntry menu_items[] = {
	{ "/_Game",              NULL,         NULL,  0,    "<Branch>" },
	{ "/Game/_New Game",     NULL,         NULL,  0,    "<Branch>"},
	{ "/Game/New Game/Novice",       NULL,    new_game,   PLAYMODE_NOVICE},
	{ "/Game/New Game/Expert",       NULL,    new_game,   PLAYMODE_SINGLE},
	{ "/Game/New Game/sep1",         NULL,    NULL,       0,   "<Separator>"},
	{ "/Game/New Game/vs. Computer", NULL,    new_game,   PLAYMODE_COMPUTER},
#ifdef TCPIP
	{ "/Game/_Network Game", NULL,         NULL,  0,    "<Branch>"},
	{ "/Game/Network Game/_Listen",  NULL,    NULL,       0,   NULL},
	{ "/Game/Network Game/_Connect", NULL,    NULL,       0,   NULL},
#endif
	{ "/Game/_End Game",     NULL,         new_game,  0,    PLAYMODE_UNSET},
	{ "/Game/sep1",          NULL,         NULL,  0,    "<Separator>"},
	{ "/Game/Settings"   ,   NULL,         settings_dialog,  0,    NULL},
	{ "/Game/sep1",          NULL,         NULL,  0,    "<Separator>"},
	{ "/Game/_Quit",         NULL,         exit,  0,    NULL},
	{ "/_Help",              NULL,         NULL,  0,    "<LastBranch>"},
	{ "/Help/_About",        NULL,         about_window,  0,    NULL},
};

static GtkWidget *build_menus()
{
	GtkItemFactory *item_factory;
	GtkAccelGroup *accel_group;
	int num_menu_items = sizeof (menu_items) / sizeof (menu_items[0]);

	accel_group = gtk_accel_group_new ();

	item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, 
					    "<main>", 
					    accel_group);

	gtk_item_factory_create_items(item_factory, 
				      num_menu_items, 
				      menu_items, 
				      NULL);

	gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);

	return gtk_item_factory_get_widget (item_factory, "<main>");

}

static gint screen_context(GtkWidget *widget, GdkEvent *event)
{
	if (event->type == GDK_BUTTON_PRESS) {
		gtk_menu_popup(GTK_MENU(menubar), widget, NULL,
			       NULL,
			       NULL, 0, 0);
	}

	return 0;
}

static gint key_snooper(GtkWidget *widget,
			GdkEventKey *event,
			gpointer func_data)
{
	int key = event->keyval;
	static BOOL ctrldown = FALSE;

	if (key >= GDK_A && key <= GDK_Z) 
		key += GDK_a - GDK_A;

	if (event->type == GDK_KEY_PRESS) {
		if (key >= 0 && key < 0xff)
			keysdown[key] |= 3;
		
		if (key == GDK_c && ctrldown) {
			++ctrlbreak;
			if (ctrlbreak > 3) {
				fprintf(stderr, "User aborted with 3 ^C's\n");
				exit(-1);
			}
		}

		if (key == GDK_Control_L)
			ctrldown = TRUE;
		else if (key == GDK_Return)
			input_buffer_push('\n');
		else if (key == GDK_Escape)
			input_buffer_push(27);
		else
			input_buffer_push(key);
	} else if (event->type == GDK_KEY_RELEASE) {
		if (key >= 0 && key < 0xff)
			keysdown[key] &= ~1;
		if(key == GDK_Control_L)
			ctrldown = FALSE;
	}
	
	return 0;
}

static GtkWidget *build_gui()
{
	GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
	menubar = build_menus();

	screen_widget = gtk_image_new(screen, NULL);

	gtk_signal_connect_object(GTK_OBJECT(window),
				  "button_press_event",
				  GTK_SIGNAL_FUNC(screen_context), NULL);

	gtk_key_snooper_install(key_snooper, NULL);

	gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), screen_widget, TRUE, FALSE, 0);

	gtk_widget_set_events(window, GDK_ALL_EVENTS_MASK);

	gtk_widget_show(menubar);
	gtk_widget_show(screen_widget);

	return vbox;
}

void get_gtk_events()
{
	while(gtk_events_pending()) 
		gtk_main_iteration_do(0);
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
	GdkVisual *v = screen->visual;
	unsigned long l = 0;

	if (v->type == GDK_VISUAL_TRUE_COLOR) {
		l = (r << v->red_shift) & v->red_mask;
		l += (g << v->green_shift) & v->green_mask;
		l += (b << v->blue_shift) & v->blue_mask;
	} else if (v->type == GDK_VISUAL_PSEUDO_COLOR) {
		GdkColormap *colormap = gdk_colormap_get_system();
		int i;
		int best=0xffffff, l = 0;

		for (i=0; i<colormap->size; ++i) {
			GdkColor *c = &colormap->colors[i];
			int r2 = c->red >> 8, g2 = c->green >> 8,
				b2 = c->blue >> 8;
			int diff = (r2-r)*(r2-r) + (g2-g)*(g2-g) + (b2-b)*(b2-b);
			
			if (!diff) {
				return i;
			}

			if (diff < best) {
				best = diff;
				l = i;
			}
		}

/*
		printf("best: %i (%i)\n", l, best);
		printf("[%i, %i, %i], [%i, %i, %i]\n", 
		       r, g, b,
		       colormap->colors[l].red,
		       colormap->colors[l].green,
		       colormap->colors[l].blue);
*/

	} else {
		l = (r + g + b) / 3;
	}

	return l;
}

inline int getpixel(int x, int y)
{
	return 0;
}

// 2x scale

static void Vid_UpdateScaled()
{
	register int x, y;
	register int pitch = screen->bpl / screen->bpp;

	// drawing is different depending on the pixel depth
	// this is probably screwed up endianwise, i dont know
	// enough about gdks image drawing to know for sure.
	// if it doesnt work i'm incredibly sorry

	if (screen->bpp == 1) {
		register unsigned char *pixels = (unsigned char *) screen->mem;
		register unsigned char *pixels2 = screenbuf;

		for (y=0; y<SCR_HGHT; ++y) {
			register unsigned char *p = pixels;
			register unsigned char *p2 = pixels2;

			for (x=0; x<SCR_WDTH; ++x) {
				p[0] = p[1] = p[pitch] = p[pitch+1]
					= colors[*p2++];
				p += 2;
			}

			pixels += pitch << 1;
			pixels2 += SCR_WDTH;
		}
	} else if(screen->bpp == 2) {
		register unsigned short *pixels = 
			(unsigned short *) screen->mem;
		register unsigned char *pixels2 = screenbuf;

		for (y=0; y<SCR_HGHT; ++y) {
			register unsigned short *p = pixels;
			register unsigned char *p2 = pixels2;

			for (x=0; x<SCR_WDTH; ++x) {
				p[0] = p[1] = p[pitch] = p[pitch+1]
					= colors[*p2++];
				p += 2;
			}

			pixels += pitch << 1;
			pixels2 += SCR_WDTH;
		}
	} else if(screen->bpp == 3) {

		// 24-bit true color
		// this is *UNTESTED*

		register unsigned char *pixels = (unsigned char *) screen->mem;
		register unsigned char *pixels2 = screenbuf;

		pitch *= 3;

		for (y = 0; y < SCR_HGHT; ++y) {
			register unsigned char *p = pixels;
			register unsigned char *p2 = pixels2;

			for (x = 0; x < SCR_WDTH; ++x) {
				int c = *p2++;
				p[0] = p[3] = p[pitch] = p[pitch+3] = c & 0xff;
				++p;
				p[0] = p[3] = p[pitch] = p[pitch+3] 
					= (c >> 8) & 0xff;
				++p;
				p[0] = p[3] = p[pitch] = p[pitch+3] 
					= (c >> 16) & 0xff;
				++p;
				p += 3;
			}
			pixels += pitch << 1;
			pixels2 += SCR_WDTH;
		}
	} else if(screen->bpp == 4) {
		register unsigned long *pixels = (unsigned long *) screen->mem;
		register unsigned char *pixels2 = screenbuf;

		for (y = 0; y < SCR_HGHT; ++y) {
			register unsigned long *p = pixels;
			register unsigned char *p2 = pixels2;

			for (x = 0; x < SCR_WDTH; ++x) {
				p[0] = p[1] = p[pitch] = p[pitch+1] 
					= colors[*p2++];
				p += 2;
			}

			pixels += pitch << 1;
			pixels2 += SCR_WDTH;
		}
	}
}

static void Vid_UpdateUnscaled()
{
	register int x, y;
	register int pitch = screen->bpl / screen->bpp;

	// drawing is different depending on the pixel depth
	// this is probably screwed up endianwise, i dont know
	// enough about gdks image drawing to know for sure.
	// if it doesnt work i'm incredibly sorry

	if (screen->bpp == 1) {
		register unsigned char *pixels = 
			(unsigned char *) screen->mem;
		register unsigned char *pixels2 = screenbuf;

		for (y=0; y<SCR_HGHT; ++y) {
			register unsigned char *p = pixels;
			register unsigned char *p2 = pixels2;

			for (x=0; x<SCR_WDTH; ++x) 
				*p++ = colors[*p2++];

			pixels += pitch;
			pixels2 += SCR_WDTH;
		}
	} else if (screen->bpp == 2) {
		register unsigned short *pixels = 
			(unsigned short *) screen->mem;
		register unsigned char *pixels2 = screenbuf;

		for (y=0; y<SCR_HGHT; ++y) {
			register unsigned short *p = pixels;
			register unsigned char *p2 = pixels2;

			for (x=0; x<SCR_WDTH; ++x)
				*p++ = colors[*p2++];
			
			pixels += pitch;
			pixels2 += SCR_WDTH;
		}

	}  else if(screen->bpp == 3) {

		// untested:

		register unsigned char *pixels = (unsigned char *) screen->mem;
		register unsigned char *pixels2 = screenbuf;

		pitch *= 3;

		for (y = 0; y < SCR_HGHT; ++y) {
			register unsigned char *p = pixels;
			register unsigned char *p2 = pixels2;

			for (x = 0; x < SCR_WDTH; ++x) {
				int c = colors[*p2++];
				*p++ = c & 0xff;
				*p++ = (c >> 8) & 0xff;
				*p++ = (c >> 16) & 0xff;
			}

			pixels += pitch;
			pixels2 += SCR_WDTH;
		}
	} else if(screen->bpp == 4) {
		register unsigned long *pixels = (unsigned long *) screen->mem;
		register unsigned char *pixels2 = screenbuf;

		for (y = 0; y < SCR_HGHT; ++y) {
			register unsigned long *p = pixels;
			register unsigned char *p2 = pixels2;

			for (x = 0; x < SCR_WDTH; ++x)
				*p++ = colors[*p2++];

			pixels += pitch;
			pixels2 += SCR_WDTH;
		}
	}
}

void Vid_Update()
{
	GdkRectangle area;

	if (!initted)
		Vid_Init();

	if (vid_double_size)
		Vid_UpdateScaled();
	else
		Vid_UpdateUnscaled();

	// redraw screen

	area.x = area.y = 0;
	area.width = screen->width;
	area.height = screen->height;

	gtk_widget_draw(screen_widget, &area);

	get_gtk_events();
}


static void set_icon(char *icon_file)
{
}

static void Vid_UnsetMode()
{
	// destroy screen image object

	gdk_image_destroy(screen);
}

static void init_colormap()
{
	int n;

	for (n = 0; n < 4; n++) {
		colors[n * 4 + 0] = getcolor(0, 0, 0);
		colors[n * 4 + 1] = getcolor(0, 255, 255);
		colors[n * 4 + 2] = getcolor(255, 0, 255);
		colors[n * 4 + 3] = getcolor(255, 255, 255);
	}
}

static void Vid_SetMode()
{
	int w, h;
	GdkVisual *visual;
	int flags = 0;

//	set_icon("icon.bmp");

	w = SCR_WDTH;
	h = SCR_HGHT;

	if (vid_double_size) {
		w *= 2;
		h *= 2;
	}

	visual = gtk_widget_get_visual(window);
//	visual->type = GDK_VISUAL_STATIC_GRAY;

	screen = gdk_image_new(GDK_IMAGE_NORMAL,
			       visual,
			       w, h);

	if (!screen) {
		printf("Vid_Init: cant create screen\n");
		exit(-1);
	}

	printf("mode: %ix%ix%i\n", w, h, screen->bpp * 8);

	if (screen->bpp == 3) {
		printf("ATTENTION: 24 bit colour mode is untested. If you see this message,\n"
		       "please email me and let me know if it works or not! :)\n"
		       "sdh300@ecs.soton.ac.uk\n");
	}

	if (screen_widget) {
		gtk_image_set(GTK_IMAGE(screen_widget), 
			      screen, NULL);
	}

//	for (n = 0; n < SDLK_LAST; ++n)
//		keysdown[n] = 0;

}

void Vid_Shutdown()
{
	if (!initted)
		return;

	Vid_UnsetMode();
//	gdk_key_repeat_restore();

	free(screenbuf);

	initted = 0;
}

void Vid_Init()
{
	int xargc=1;
	char **xargv;
	GtkWidget *gui;

	if (initted)
		return;

	// eww, we dont have the actual arguments
	// so lets hack some together

	xargv = malloc(sizeof(*xargv) * 2);
	xargv[0] = "hello";
	xargv[1] = NULL;

	// set up gtk and build window

	gtk_init(&xargc, &xargv);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_window_set_policy(GTK_WINDOW(window), 0, 0, 1);
	
	Vid_SetMode();

	gui = build_gui();

//	gdk_key_repeat_disable();
	gtk_container_add(GTK_CONTAINER(window), gui);

	gtk_signal_connect(GTK_OBJECT(window), "destroy",
			   GTK_SIGNAL_FUNC(exit), NULL);
	gtk_signal_connect(GTK_OBJECT(window), "delete_event",
			   GTK_SIGNAL_FUNC(exit), NULL);

	gtk_window_set_title(GTK_WINDOW(window), 
			     "Gtk+ Sopwith");

	gtk_widget_show(gui);
	gtk_widget_show(window);	

	init_colormap();

	// create screen mem buffer

	vid_vram = screenbuf = malloc(SCR_WDTH * SCR_HGHT);
	vid_pitch = SCR_WDTH;

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

static void getevents()
{
	get_gtk_events();
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
// $Log$
// Revision 1.1  2003/02/14 19:03:36  fraggle
// Initial revision
//
//
// sdh 14/2/2003: change license header to GPL
// sdh 25/04/2002: rename vga_{pitch,vram} to vid_{pitch,vram}
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
