// Emacs style mode select -*- C++ -*-
//--------------------------------------------------------------------------
//
// $Id$
//
// Copyright(C) 2001-2004 Simon Howard
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
#include "swinit.h"
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

	swsaveconf();
}

static GtkWidget *build_settings_dialog()
{
	GtkWidget *window;
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
			gtk_toggle_button_set_active
				(GTK_TOGGLE_BUTTON(widget),
				 *confoptions[i].value.b);
			gtk_signal_connect(GTK_OBJECT(widget),
					   "toggled",
					   GTK_SIGNAL_FUNC(settings_bool_toggle),
					   confoptions[i].value.b);
			gtk_widget_show(widget);
		default:
			break;
		}
	}
		
	gtk_widget_show(vbox);
	gtk_widget_show(vbox2);
	gtk_widget_show(hbox);

	vbox = GTK_DIALOG(window)->action_area;

	// close button

	button = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	gtk_box_pack_start(GTK_BOX(vbox), button, TRUE, TRUE, 0);
	gtk_signal_connect(GTK_OBJECT(button), "clicked",
			   GTK_SIGNAL_FUNC(hide_callback), 
			   window);
	gtk_widget_show(button);

	return window;
}

static void settings_dialog()
{
	static GtkWidget *window = NULL;

	if (!window)
		window = build_settings_dialog();

	gtk_widget_show(window);
}

static void new_game(gpointer callback_data,
		     guint callback_action,
		     GtkWidget *widget)
{
	playmode = callback_action;
	longjmp(envrestart, 0);
}

static GtkWidget *build_about_window()
{
	GtkWidget *window;
	GtkWidget *label, *button;

	// window

	window = gtk_dialog_new(); 

	gtk_container_set_border_width(GTK_CONTAINER(GTK_DIALOG(window)->vbox), 6);
	gtk_window_set_title(GTK_WINDOW(window), 
			     "About Sopwith");

	gtk_signal_connect(GTK_OBJECT(window), "destroy",
			   GTK_SIGNAL_FUNC(delete_event), window);
	gtk_signal_connect(GTK_OBJECT(window), "delete_event",
			   GTK_SIGNAL_FUNC(delete_event), window);

	// label:

	label = gtk_label_new(NULL);

	gtk_label_set_justify(GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_label_set_markup(GTK_LABEL(label), 
		"<span size=\"xx-large\" weight=\"bold\">"
		"Gtk+ Sopwith " VERSION "</span>\n\n"
		"Classic biplane shoot-em-up game.\n\n"
		"<span size=\"small\">"
		"Copyright(C) 1984, 1985, 1987 "
		"BMB Compuscience Canada Ltd.\n"
		"Copyright(C) 1984-2000 David L. Clark\n"
		"Copyright(C) 2001-2004 Simon Howard"
		"</span>");

	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(window)->vbox),
			   label,
			   TRUE, TRUE, 10);

	gtk_widget_show(label);

	// button:

	button = gtk_button_new_from_stock(GTK_STOCK_OK);

	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(window)->action_area),
			   button,
			   TRUE, TRUE, 0);

	gtk_signal_connect(GTK_OBJECT(button), "clicked",
			   GTK_SIGNAL_FUNC(hide_callback), window);

	gtk_widget_show(button);

	return window;
}

static void about_window(gpointer callback_data, 
			 guint callback_action,
			 GtkWidget *widget)
{
	static GtkWidget *window = NULL;

	if (!window)
		window = build_about_window();

	gtk_widget_show(window);
}

static GtkItemFactoryEntry menu_items[] = {
	{ "/_Game",              NULL,         NULL,  0,    "<Branch>" },
	{ "/Game/_Single Player",     NULL,         NULL,  0,    "<Branch>"},
	{ "/Game/Single Player/Play in _Novice Mode",       NULL,    
			new_game,   PLAYMODE_NOVICE},
	{ "/Game/Single Player/Play in _Expert Mode",       NULL,    
			new_game,   PLAYMODE_SINGLE},
	{ "/Game/Single Player/sep1",         NULL,    NULL,       0,   "<Separator>"},
	{ "/Game/Single Player/Play vs. _Computer", NULL,    
			new_game,   PLAYMODE_COMPUTER},
#ifdef TCPIP
	// not done yet:
//	{ "/Game/_Network Game", NULL,         NULL,  0,    "<Branch>"},
//	{ "/Game/Network Game/_Listen",  NULL,    NULL,       0,   NULL},
//	{ "/Game/Network Game/_Connect", NULL,    NULL,       0,   NULL},
#endif
	{ "/Game/sep1",          NULL,         NULL,  0,    "<Separator>"},
	{ "/Game/_End Game",     NULL,         new_game,  PLAYMODE_UNSET,
			"<StockItem>", GTK_STOCK_CLOSE},
	{ "/Game/sep1",          NULL,         NULL,  0,    "<Separator>"},
	{ "/Game/_Quit",         NULL,         exit,  0,    
			"<StockItem>", GTK_STOCK_QUIT},
	{ "/_Settings",		 NULL,	       NULL,  0,    "<Branch>"},
	{ "/Settings/_Preferences",   NULL,    settings_dialog,  0,    
			"<StockItem>", GTK_STOCK_PREFERENCES},
	{ "/_Help",              NULL,         NULL,  0,    "<Branch>"},
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

static sopkey_t translate_key(int gdk_key)
{
	switch (gdk_key) {
	case GDK_Down:
	case GDK_period:
		return KEY_FLIP;
	case GDK_Left:
	case GDK_comma:
		return KEY_PULLUP;
	case GDK_Right:
	case GDK_slash:
		return KEY_PULLDOWN;
	case GDK_x:
		return KEY_ACCEL;
	case GDK_z:
		return KEY_DECEL;
	case GDK_b:
		return KEY_BOMB;
	case GDK_space:
		return KEY_FIRE;
	case GDK_h:
		return KEY_HOME;
	case GDK_v:
		return KEY_MISSILE;
	case GDK_c:
		return KEY_STARBURST;
	case GDK_s:
		return KEY_SOUND;
	default:
		return KEY_UNKNOWN;
	}
}

static gint key_snooper(GtkWidget *widget,
			GdkEventKey *event,
			gpointer func_data)
{
	int key = event->keyval;
	static BOOL ctrldown = FALSE;
	sopkey_t translated;

	if (key >= GDK_A && key <= GDK_Z) 
		key += GDK_a - GDK_A;

	if (event->type == GDK_KEY_PRESS) {
		translated = translate_key(key);

		if (translated)
			keysdown[translated] |= 3;
		
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
		translated = translate_key(key);

		if (translated)
			keysdown[translated] &= ~1;

		if(key == GDK_Control_L)
			ctrldown = FALSE;
	}
	
	return 0;
}

static GtkWidget *build_gui()
{
	GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
	menubar = build_menus();

#if GTK_MAJOR_VERSION < 2
	screen_widget = gtk_image_new(screen, NULL);
#else
	screen_widget = gtk_image_new();

	gtk_image_set(GTK_IMAGE(screen_widget),
		      screen, NULL);
#endif

	gtk_key_snooper_install(key_snooper, NULL);

	gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), screen_widget, TRUE, FALSE, 0);

	gtk_widget_set_events(window, GDK_ALL_EVENTS_MASK);

	gtk_widget_show(menubar);
	gtk_widget_show(screen_widget);
	gtk_widget_show(vbox);

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

#if GTK_MAJOR_VERSION < 2
	// redraw screen

	area.x = area.y = 0;
	area.width = screen->width;
	area.height = screen->height;

	gtk_widget_draw(screen_widget, &area);
#else
	// this appears to work for gtk2, the above gtk1 code does
	// not work for gtk2, parts of the screen dont get redrawn (??)

	gtk_image_set(GTK_IMAGE(screen_widget), screen, NULL);
#endif

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

	screen = gdk_image_new(GDK_IMAGE_FASTEST,
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
		       "sdh300@zepler.net\n");
	}

	if (screen_widget) {
		gtk_image_set(GTK_IMAGE(screen_widget), 
			      screen, NULL);
	}
}

void Vid_Shutdown()
{
	if (!initted)
		return;

	Vid_UnsetMode();

	free(screenbuf);

	initted = 0;
}

void Vid_Init()
{
	GtkWidget *gui;
	int i;

	if (initted)
		return;

	for (i=0; i<NUM_KEYS; ++i)
		keysdown[i] = 0;

	// eww, we dont have the actual arguments
	// so lets hack some together

	// build window

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_window_set_policy(GTK_WINDOW(window), 0, 0, 1);
	
	Vid_SetMode();

	gui = build_gui();

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

BOOL Vid_GetCtrlBreak()
{
	getevents();
	return ctrlbreak;
}


//-----------------------------------------------------------------------
// 
// $Log$
// Revision 1.11  2004/10/15 18:06:17  fraggle
// Fix copyright notice
//
// Revision 1.10  2004/10/14 08:48:46  fraggle
// Wrap the main function in system-specific code.  Remove g_argc/g_argv.
// Fix crash when unable to initialise video subsystem.
//
// Revision 1.9  2003/06/10 21:08:03  fraggle
// Move email to zepler.net
//
// Revision 1.8  2003/06/08 00:48:30  fraggle
// use GDK_IMAGE_FASTEST instead of GDK_IMAGE_NORMAL for speed
//
// Revision 1.7  2003/06/05 01:51:18  fraggle
// Remove broken popup menu code (was useless anyway)
//
// Revision 1.6  2003/06/04 17:22:11  fraggle
// Remove "save settings" option in settings menus. Just save it anyway.
//
// Revision 1.5  2003/06/04 15:41:07  fraggle
// Remove some dead code
//
// Revision 1.4  2003/05/26 20:07:15  fraggle
// Pseudo GNOME HiG-ify
// Remove Gtk+ 1.x support
//
// Revision 1.3  2003/03/26 14:11:52  fraggle
// Gtk+ 2.0 support
//
// Revision 1.2  2003/03/26 13:53:29  fraggle
// Allow control via arrow keys
// Some code restructuring, system-independent video.c added
//
// Revision 1.1.1.1  2003/02/14 19:03:36  fraggle
// Initial Sourceforge CVS import
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

