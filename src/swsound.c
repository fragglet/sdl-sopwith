// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------------
//
// $Id$
//
// Copyright(C) 1984-2000 David L. Clark
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
//---------------------------------------------------------------------------
//
//        swsound  -      SW sound generation
//
//---------------------------------------------------------------------------

#include <ctype.h>

#include "pcsound.h"
#include "timer.h"

#include "sw.h"
#include "swmain.h"
#include "swsound.h"

#define TIMER   0x40
#define PORTB   0x61
#define SNDSIZE 100

static int soundtype = 32767;		/*  Current sound type and          */
static int soundparm = 32767;		/*     and priority parameter       */
static OBJECTS *soundobj = NULL;	/*  Object making sound             */
static unsigned lastfreq = 0;		/*  Last frequency used             */
static OBJECTS *lastobj = NULL;		/*  Previous object making sound    */
static void (*toneadj) () = NULL;	/*  Tone adjustment on clock tick   */

static TONETAB tonetab[SNDSIZE];	/*  Continuous tone table           */
static TONETAB *frsttone, *freetone;	/*  Tone list and free list         */
static unsigned soundticks;		/*  Ticks since last sound selection */

static int numexpls;			/*  Number of explosions currently  */
					/*  active                          */
static int explplace;			/*  Place in explosion tune;        */
static int explline;			/* Line in explosion tune           */
static unsigned expltone;		/*  Current explosion tone          */
static int explticks;			/*  Ticks until note change         */
static int exploctv;			/*  Octave                          */

//#define SOPWITH1_TUNE

#ifdef SOPWITH1_TUNE
static char      *expltune[7] = {
        ">e4./d8/c4/d4/e4/d+4/e4/c4/d4/d4/d4/d1/",
        "d4./c8/b4/c4/d4/c+4/d4/b4/c4/c4/c4/c1/<g4./g+8/"
        ">a4./a-8/<g4./g+8/>a4/a-4/<g4/>d4/d4/d2./<g4./g+8/",
        ">a4./a-8/<g4./g+8/>a4/a-4/<g4/>e4/e4/e2./",
        "e4./d8/c4/d4/e4/d+4/e4/c4/d4/d4/d4/d2/c4/<g+4/>a4/",
        "d2/e2/g1/",
        ""
};
#else
static  char     *expltune[7] = {
       "b4/d8/d2/r16/c8/b8/a8/b4./c4./c+4./d4./",
       "e4/g8/g2/r16/>a8/<g8/e8/d2./",
       "b4/d8/d2/r16/c8/b8/a8/b4./c4./c+4./d4./",
       "e4/>a8/a2/r16/<g8/f+8/e8/d2./",
       "d8/g2/r16/g8/g+2/r16/g+8/>a2/r16/a8/c2/r16/",
       "b8/a8/<g8/>b4/<g8/>b4/<g8/>a4./<g1/",
       ""
};
#endif

static BOOL titleflg;		/* Playing title tune               */
static int titlplace;		/*  Place in title tune;            */
static int titlline;		/* Line in title tune               */
static unsigned titltone;	/*  Current title tone              */
static int titlticks;		/*  Ticks until note change         */
static int titloctv;		/*  Octave                          */



static char **tune;		/* Tune player statics              */
static int line;
static int place;
static unsigned tunefreq;
static int tunedura;
static int octavefactor;


// random number generator

static int seed[50] = {
	0x90B9, 0xBCFB, 0x6564, 0x3313, 0x3190, 0xA980, 0xBCF0, 0x6F97,
	0x37F4, 0x064B, 0x9FD8, 0x595B, 0x1EEE, 0x820C, 0x4201, 0x651E,
	0x848E, 0x15D5, 0x1DE7, 0x1585, 0xA850, 0x213B, 0x3953, 0x1EB0,
	0x97A7, 0x35DD, 0xAF2F, 0x1629, 0xBE9B, 0x243F, 0x847D, 0x313A,
	0x3295, 0xBC11, 0x6E6D, 0x3398, 0xAD43, 0x51CE, 0x8F95, 0x507E,
	0x499E, 0x3BC1, 0x5243, 0x2017, 0x9510, 0x9865, 0x65F6, 0x6B56,
	0x36B9, 0x5026
};



static unsigned int swrand(unsigned int modulo)
{
	static int i = 0;

	if (i >= 50)
		i = 0;
	return (seed[i++] % modulo);
}


static TONETAB *allocton()
{
	register TONETAB *tt;

	if (!freetone)
		return 0;

	tt = freetone;
	freetone = tt->tt_next;

	tt->tt_next = frsttone;
	tt->tt_prev = NULL;

	if (frsttone)
		frsttone->tt_prev = tt;

	frsttone = tt;

	return frsttone;
}



static void deallton(TONETAB * ttp)
{
	register TONETAB *tt = ttp;
	register TONETAB *ttb = tt->tt_prev;

	if (ttb)
		ttb->tt_next = tt->tt_next;
	else
		frsttone = tt->tt_next;

	ttb = tt->tt_next;

	if (ttb)
		ttb->tt_prev = tt->tt_prev;

	tt->tt_next = freetone;
	freetone = tt;
}



void initsndt()
{
	register TONETAB *tt;
	register int i;

	for (i = 0, tt = tonetab; i < (SNDSIZE - 1); ++i, ++tt)
		tt->tt_next = tt + 1;
	tt->tt_next = NULL;
	frsttone = NULL;
	freetone = tonetab;
}


void stopsound(OBJECTS * ob)
{
	TONETAB *tt = ob->ob_sound;

	if (!tt)
		return;

	if (ob->ob_type == EXPLOSION)
		--numexpls;
	else
		deallton(tt);
	ob->ob_sound = NULL;
}



void soundoff()
{
	if (lastfreq) {

		// sdh: use the emulated sdl pc speaker code

		Speaker_Off();
#ifdef IBMPC
		// old dos calls
		outportb(PORTB, 0xFC & inportb(PORTB));
#endif
		lastfreq = 0;
		dispdbg = 0;
	}
}


static void tone(unsigned int freq)
{
	if (!soundflg)
		return;

	if (lastfreq == freq)
		return;

	// sdh: use the emulated sdl pc speaker code

	Speaker_Output(freq);

#ifdef IBMPC
	// old dos system stuff

	if (!lastfreq)
		outportb(TIMER + 3, 0xB6);
	outportb(TIMER + 2, freq & 0x00FF);
	outportb(TIMER + 2, freq >> 8);
	if (!lastfreq)
		outportb(PORTB, 0x03 | inportb(PORTB));
#endif

	lastfreq = freq;
	dispdbg = freq;
}


// music note generation

#define NOTEEND     '/'
#define UPOCTAVE    '>'
#define DOWNOCTAVE  '<'
#define SHARP       '+'
#define FLAT        '-'
#define DOT         '.'
#define REST        'R'

void playnote()
{

	static int noteindex[] = { 0, 2, 3, 5, 7, 8, 10 };
	static int notefreq[] =
	    { 440, 466, 494, 523, 554, 587, 622, 659, 698, 740, 784, 831 };

	static int durplace, test, freq, duration;
	static int index;
	static int indexadj;

	static char durstring[5];
	static char charatplace, noteletter;

	static int noteoctavefactor;
	static int dottednote;

	BOOL firstplace = TRUE;

	indexadj = 0;
	durplace = 0;
	dottednote = 2;
	noteoctavefactor = 256;

	FOREVER {
		if (!line && !place)
			octavefactor = 256;

		charatplace = toupper(tune[line][place++]);
		if (!charatplace) {
			place = 0;
			charatplace = tune[++line][0];
			if (!charatplace) {
				line = 0;
			}

			if (firstplace)
				continue;
			break;
		}
		firstplace = FALSE;
		if (charatplace == NOTEEND)
			break;

		test = isalpha(charatplace);
		if (test) {
			index = *(noteindex + (charatplace - 'A'));
			noteletter = charatplace;
		} else
			switch (charatplace) {
			case UPOCTAVE:
				octavefactor <<= 1;
				break;
			case DOWNOCTAVE:
				octavefactor >>= 1;
				break;
			case SHARP:
				indexadj++;
				break;
			case FLAT:
				indexadj--;
				break;
			case DOT:
				dottednote = 3;
				break;
			default:
				test = isdigit(charatplace);
				if (test)
					*(durstring + durplace++) =
					    charatplace;
				break;
			}

	}

	durstring[durplace] = '\0';
	duration = atoi(durstring);
	if (duration <= 0)
		duration = 4;
	duration = (1440 * dottednote / (60 * duration)) / 2;

	if (noteletter == REST) {
		tunefreq = 0;
	} else {
		index += indexadj;
		while (index < 0) {
			index += 12;
			noteoctavefactor /= 2;
		}
		while (index >= 12) {
			index -= 12;
			noteoctavefactor *= 2;
		}

		// sdh: soundmul and sounddiv were asm functions. i cant
		// read x86 asm so i have to guess

		freq = notefreq[index];
		freq *= octavefactor;
		freq >>= 8;
		freq *= noteoctavefactor;
		freq >>= 8;
//        freq = soundmul( *(notefreq+index), octavefactor, noteoctavefactor );
		tunefreq = 1331000 / freq;
//    tunefreq = sounddiv( 1331000L, freq );

	}
#ifdef SOPWITH1_TUNE
	tunedura = duration * 0.7;
#else 
	tunedura = duration;
#endif
}


static void adjcont()
{
	register TONETAB *tt = lastobj->ob_sound;

	if (tt)
		tone(tt->tt_tone + tt->tt_chng * soundticks);
}




static void adjshot()
{
	static unsigned savefreq;

	if (lastfreq == 0xF000)
		tone(savefreq);
	else {
		savefreq = lastfreq;
		tone(0xF000);
	}
}


static void explnote()
{
	line = explline;
	place = explplace;
	tune = expltune;
	octavefactor = exploctv;
	playnote();
	explline = line;
	explplace = place;
	expltone = tunefreq;
	explticks += tunedura;
	exploctv = octavefactor;
}


static void adjexpl()
{
	if (--explticks >= 0)
		return;

	explnote();
}


static void titlnote()
{
	line = titlline;
	place = titlplace;
	tune = expltune;
	octavefactor = titloctv;
	playnote();
	titlline = line;
	titlplace = place;
	titltone = tunefreq;
	titlticks += tunedura;
	titloctv = octavefactor;
	soundoff();
	tone(titltone);
}


static void adjtitl()
{
	if (--titlticks >= 0)
		return;
	titlnote();
}


void soundadj()
{

	++soundticks;

	if (lastfreq && toneadj)
		(*toneadj) ();

	if (numexpls)
		adjexpl();

	if (titleflg)
		adjtitl();
}



void swsound()
{
	register TONETAB *tt;

	tt = frsttone;
	while (tt) {
		tt->tt_tone += (tt->tt_chng * soundticks);
		tt = tt->tt_next;
	}

	soundticks = 0;
	titleflg = FALSE;

	switch (soundtype) {

	case 0:
	case 32767:
	default:
		soundoff();
		lastobj = NULL;
		toneadj = NULL;
		break;

	case S_PLANE:
		if (soundparm)
			tone(0xF000 + soundparm * 0x1000);
		else
			tone(0xD000);
		lastobj = NULL;
		toneadj = NULL;
		break;

	case S_BOMB:
		if (soundobj == lastobj)
			break;
		toneadj = adjcont;
		lastobj = soundobj;
		adjcont();
		break;

	case S_FALLING:
		if (soundobj == lastobj)
			break;
		toneadj = adjcont;
		lastobj = soundobj;
		adjcont();
		break;

	case S_HIT:
		tone(swrand(2) ? 0x9000 : 0xF000);
		lastobj = NULL;
		toneadj = NULL;
		break;

	case S_EXPLOSION:
		tone(expltone);
		toneadj = NULL;
		lastobj = NULL;
		break;

	case S_SHOT:
		tone(0x1000);
		toneadj = adjshot;
		lastobj = NULL;
		break;

	}

	soundtype = soundparm = 32767;
}


void sound(int type, int parm, OBJECTS * ob)
{
	// sdh 28/10/2001: moved code for title music setup here
	// if we are already playing the title music, ignore

	if (type == S_TITLE) {
		if (!titleflg) {
			titlline = 0;
			titlplace = 0;
			titlnote();
			toneadj = NULL;
			lastobj = NULL;
			titleflg = TRUE;
		}
	} else if (type < soundtype) {
		soundtype = type;
		soundparm = parm;
		soundobj = ob;
	} else if (type == soundtype && parm < soundparm) {
		soundparm = parm;
		soundobj = ob;
	}
}



void initsound(OBJECTS * obp, int type)
{
	register OBJECTS *ob;
	register TONETAB *tt;

	if ((ob = obp)->ob_sound)
		return;

	if (ob->ob_type == EXPLOSION) {
		if (++numexpls == 1) {
			explline = 0;
			explplace = 0;
			explnote();
		}
		ob->ob_sound = (struct tt *) 1;
		return;
	}

	if ((tt = allocton())) {
		switch (type) {
		case S_BOMB:
			tt->tt_tone = 0x0300;
			tt->tt_chng = 8;
			break;
		case S_FALLING:
			tt->tt_tone = 0x1200;
			tt->tt_chng = -8;
			break;
		default:
			break;
		}
		ob->ob_sound = tt;
		return;
	}
}



// sdh:
// in original sopwith this was done with interrupts
// we dont have access to interrupts and its ugly in
// any case, so instead we call this function occasionally
// to check for updating the sound

// in fact for continuity i have put a call to this
// in the sdl sound callback function, so it is done
// rather like the old interrupt system after all :)

static int lastclock = 0;

void swsndupdate()
{
	int thisclock = Timer_GetMS();

	if (thisclock > lastclock + 1000 / 18.2) {
		lastclock = thisclock;
		soundadj();
	}
}

//---------------------------------------------------------------------------
//
// $Log$
// Revision 1.1  2003/02/14 19:03:18  fraggle
// Initial revision
//
//
// sdh 14/2/2003: change license header to GPL
// sdh 25/11/2001: remove intsoff, intson calls
// sdh 21/10/2001: rearranged headers, added cvs tags
// sdh 21/10/2001: reformatted with indent. adjusted some code by hand
//                 to make more readable
// sdh 20/10/2001: added #define to use sopwith 1 theme (extracted from 
//                 hexdump of sopwith1.exe)
// sdh 19/10/2001: removed externs (now in headers)
//                 shuffled some functions round to shut up compiler
// sdh 18/10/2001: converted all functions to ANSI-style arguments
// sdh ??/10/2001: use SDL digitally emulated PC speaker
//
// 87-03-10        Microsoft compiler.
// 84-04-11        Development
//
//---------------------------------------------------------------------------

