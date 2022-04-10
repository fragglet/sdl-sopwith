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
//        swmultio -      SW multiple plane communications I/O
//
//                        This module is a replacement to the
//                        original "swmultio.c".
//
//---------------------------------------------------------------------------

#include <dirent.h>

#include "sw.h"
#include "swasynio.h"
#include "swcollsn.h"
#include "swdisp.h"
#include "swend.h"
#include "swgames.h"
#include "swgrpha.h"
#include "swinit.h"
#include "swmain.h"
#include "swmisc.h"
#include "swmove.h"
#include "swnetio.h"
#include "swtitle.h"

// sdh: dummy definitions to shut up compiler

struct bpbtab {
	int fnord;
};


char multstck[400];		/* Stack area for I/O processes     */
int *multstack;			/* Initial stack pointer            */

char *multfile = COMM_FILE;	/* Multiple user files              */

static BOOL first;		/* first player flag              */

#if 0

static char *errormsg;
static BOOL errflg1;
static BOOL errorflg = FALSE;
static unsigned errtab[50];
static unsigned errwhr[50];
static int errptr = -1;

static char *prtbuf = auxdisp;

static BOOL multterm = FALSE;
static BOOL swlocked = FALSE;
#endif

#define OREAD           1
#define OWRITE          2
#define OREADPLAY       3
#define OWRITEPLAY      4

// endianness conversions
// should use htons/ntohs/htonl/ntohl for these

int _word(char *ptr)
{
	return ((*ptr & 0x00FF) | ((*(ptr + 1) & 0x00FF) << 8));
}



int _byte(char *ptr)
{
	return (*ptr & 0x00FF);
}



void _fromintel()
{
#ifdef ATARI
	register int *mu, word, i;

	mu = (int *) multbuff;
	for (i = sizeof(MULTIO) >> 1; i; --i) {
		word = *mu;
		*mu++ = ((word >> 8) & 0x00FF) | (word << 8);
	}
#endif
}



char *_tointel()
{
#ifdef IBMPC
	return ((char *) multbuff);
#endif


#ifdef ATARI
	static MULTIO mubuff;
	register int *mu1, *mu2, i, word;

	mu1 = (int *) multbuff;
	mu2 = (int *) &mubuff;
	for (i = sizeof(MULTIO) >> 1; i; --i) {
		word = *mu1++;
		*mu2++ = ((word >> 8) & 0x00FF) | (word << 8);
	}
	return ((char *) &mubuff);
#endif
	return 0;
}


// this function is called by the multiplayer planes

BOOL movemult(OBJECTS * obp)
{
	register OBJECTS *ob;

	plyrplane = compplane = FALSE;

	endstat = endsts[currobx = (ob = obp)->ob_index];

	if (!dispcnt)
		interpret(ob, (playmode == PLAYMODE_MULTIPLE) ? multget(ob)
			  : asynget(ob));
	else {
		ob->ob_flaps = 0;
		ob->ob_bombing = FALSE;
	}
	if (((ob->ob_state == CRASHED) || (ob->ob_state == GHOSTCRASHED))
	    && (ob->ob_hitcount <= 0))
		if (ob->ob_life > QUIT) {
			// sdh 25/10/2001: infinite lives in multiplayer
			//++ob->ob_crashcnt;
			initpln(ob);
		}

	return movepln(ob);
}

// open connection or something

void multopen(char *cmndfile, char *multfile)
{
	multparm(cmndfile);

//        commdriv = multdriv;
//        commhead = multhead;
//        commtrk  = multtrk;
//        commsect = multsect;
//        commasect = multasect;

	multparm(multfile);

}

// no idea

void multparm(char *multfile)
{
#ifdef IBMPC
	char dev[3];
	BIOFD fd;
	register int i;
	register char *mf;

	mf = multfile;
	dev[2] = 0;
	if (((multdriv = (dev[0] = tolower(*mf++)) - 'a') < 0)
	    || (multdriv > 25)
	    || ((dev[1] = *mf++) != ':')
	    || (!(fd = devfd = bopen(dev, "rw"))))
		swend("Improper device specification", NO);


	i = 0;
	while (mf[i]) {
		mf[i] = toupper(mf[i]);
		++i;
	}
	if (!(multsect = name_to_sec(fd, mf)))
		swend("File not found", NO);

	multasect = --multsect;
	sectparm();
#endif
}



#ifdef IBMPC
static int name_to_sec(BIOFD fd, char *name)
{
	struct bpbtab bpb;
	struct dirent entry;

	make_bpb(fd, &bpb);
	if (!get_ent(fd, &bpb, name, &entry))
		return (NO);
	return (clu_to_sec(_word(entry.dc + DIR_CLU), &bpb));
}
#endif



#ifdef IBMPC
static int clu_to_sec(int cluster, struct bpbtab *bpb)
{
	return ((cluster - 2) * _byte(bpb->bc + BP_CLUSIZE)
		+ (_word(bpb->bc + BP_DIRENT) >> 4)
		+ 1 + _word(bpb->bc + BP_RESERVED)
		+ (_byte(bpb->bc + BP_NFAT) * _word(bpb->bc + BP_FATSIZE))
	    );
	return 0;
}
#endif



#ifdef IBMPC
static void make_bpb(BIOFD fd, struct bpbtab *bpb)
{
	bseek(fd, 0l, 0);
	bread(prtbuf, 512, fd);
	memcpy(bpb, prtbuf+11, sizeof(bpbtab));
	swbpb = bpb;
}
#endif




#ifdef IBMPC
static int get_ent(BIOFD fd, struct bpbtab *bpb,
		   char *name, struct dirent *entry)
{
	if (*name == '\\')
		++name;
	return lookup(fd, bpb, name, entry);
}
#endif



#ifdef IBMPC
static int lookup(BIOFD fd, struct bpbtab *bpb, char *name,
		  struct dirent *entry)
{
	long block;
	int strtdir;
	register int i;
	register struct dirent *current;
	char want[12];

	for (i = 0; *name && (i < 12); ++name) {
		if (*name == '.')
			while (i < 8)
				want[i++] = ' ';
		else
			want[i++] = *name;
	}
	while (i < 11)
		want[i++] = ' ';
	want[11] = 0;

	block = 1 + _word(bpb->bc + BP_RESERVED)
	    + (_byte(bpb->bc + BP_NFAT) * _word(bpb->bc + BP_FATSIZE));
	bseek(fd, (long) ((block - 1) << 9), 0);
	for (strtdir = 0;
	     strtdir < _word(bpb->bc + BP_DIRENT); strtdir += 0x10) {
		bread(prtbuf, 512, fd);
		for (current = (struct dirent *) prtbuf, i = 0;
		     i < 0x10; ++i, ++current) {
			if ((current->dc + DIR_NAME)[0] == 0)
				return (NO);
			if (strncmp(want, current->dc + DIR_NAME, 11) == 0) {
				memcpy(entry, current, sizeof(struct dirent));
				return (YES);
			}
		}
	}

	return NO;
}
#endif


void sectparm()
{
#ifdef  IBMPC
	int key;
	struct regval reg;
	struct {
		char fill1[55];
		unsigned firstdrive;
		char fill2[2];
		unsigned diskbase;
		char fill3[5];
		unsigned signat;
	} iocbuf;
	int spc;

	spc = _word(swbpb->bc + BP_SECPTRK) * _word(swbpb->bc + BP_NHEADS);
	multtrk = multsect / spc;
	multsect -= multtrk * spc;
	multhead = multsect / _word(swbpb->bc + BP_SECPTRK);
	multsect -= (multhead * _word(swbpb->bc + BP_SECPTRK)) - 1;

	reg.axr = 0x4404;
	reg.bxr = multdriv + 1;
	reg.cxr = sizeof(iocbuf);
	reg.dxr = (unsigned) &iocbuf;
	reg.dsr = dsseg();
	sysint21(&reg, &reg);
	if (iocbuf.signat == 0x4003) {
		multaddr = iocbuf.diskbase;
		multdriv -= iocbuf.firstdrive;
		multhead |= ((multdriv % 3) + 1) << 6;
		multdriv = 3 - (multdriv / 3);
	}
#endif

#ifdef  ATARI
	multhead = 0;
	multtrk = 0;
#endif

/*      puts( "\r\nAddress: " );
        dispd( multaddr, 6 );
        puts( "\r\nDrive  : " );
        dispd( multdriv, 6 );
        puts( "\r\nHead   : " );
        dispd( multhead, 6 );
        puts( "\r\nTrack  : " );
        dispd( multtrk,  6 );
        puts( "\r\nSector : " );
        dispd( multsect, 6 );
        puts( "\r\nOK? (Y/N)" );
        FOREVER {
                if ( ctlbreak() )
                        swend( NULL, NO );
                if ( ( key = toupper( swgetc() & 0x00FF ) ) == 'Y' )
                        break;
                if ( key == 'N' )
                        swend( NULL, NO );
        }
*/
}

// read in queued data 

int multread()
{
#ifdef IBMPC
	bseek(devfd, commasect << 9, 0);
	if (bread(prtbuf, 512, devfd) != 512) {
		errlog((bioerr() & 0xFF00) + 1, OREAD);
		return (0);
	}
	if (*prtbuf != 0xFE)
		return (-1);

	*prtbuf = 0xFF;
	bseek(devfd, commasect << 9, 0);
	if (bwrite(prtbuf, 512, devfd) != 512) {
		errlog((bioerr() & 0xFF00) + 2, OREAD);
		return (0);
	}

	bseek(devfd, commasect << 9, 0);
	if (bread(prtbuf, 512, devfd) != 512) {
		errlog((bioerr() & 0xFF00) + 3, OREAD);
		return (0);
	}
	if (*prtbuf != 0xFF)
		return (-1);

	swlocked = TRUE;

	bseek(devfd, multasect << 9, 0);
	if (bread(multbuff, 512, devfd) != 512) {
		errlog((bioerr() & 0xFF00) + 4, OREAD);
		return (0);
	}
	_fromintel();
#endif
	return 1;
}



// write queued data

int multwrite()
{
#ifdef IBMPC
	register int dkerr;
	register char *buff;

	FOREVER {
		bseek(devfd, multasect << 9, 0);
		buff = _tointel();
		if (bwrite(buff, 512, devfd) == 512)
			return (1);
		dkerr = 0xFF00 & bioerr();
		errlog(dkerr, OWRITE);
		if (dkerr != 0x0300)
			return (0);
	}
#endif
	return 0;
}


// unlock multiplayer file

int multunlock()
{
#ifdef IBMPC
	if (!swlocked)
		return (1);

	*prtbuf = 0xFE;
	bseek(devfd, commasect << 9, 0);
	if (bwrite(prtbuf, 512, devfd) != 512)
		return (0);

	swlocked = FALSE;
#endif

	return 1;
}


// wait until new data arrives

void multwait()
{
#ifdef IBMPC
	int _multwait();

	_dkproc(_multwait, multstack);
	while (_dkiosts());
	if (errorflg)
		swend(errormsg, errflg1);
#endif
}



// get next character from network pipe

int multget(OBJECTS * ob)
{
#ifdef IBMPC
	register int o;

	if (errorflg)
		swend(errormsg, errflg1);

	while (_dkiosts());
	if (errorflg)
		swend(errormsg, errflg1);

	o = ob->ob_index;
	if (o != player)
		updstate(ob, multbuff->mu_state[o]);
	return (histmult(o, multbuff->mu_key[o]));
#endif
	return 0;
}

// dont know

#ifdef IBMPC
static void updstate(OBJECTS * obp, obstate_t statep)
{
	register OBJECTS *ob = obp;
	register obstate_t state = statep;

	if (ob->ob_state != state
	    && (state == FINISHED
		|| state == WAITING
		|| ob->ob_state == FINISHED
		|| ob->ob_state == WAITING)) {
		ob->ob_state = state;
		setvdisp();
		dispwobj(ob);
	}
}
#endif


// send another character?

void multput()
{
#ifdef IBMPC
	int _multput();

	while (_dkiosts());
	if (errorflg)
		swend(errormsg, errflg1);
	_dkproc(_multput, multstack);
#endif
}



// disconnect

char *multclos(BOOL update)
{
#ifdef IBMPC
	register int rc, n;
	char *closeret = NULL;
	BOOL alldone;
	int tickwait;

	if (repflag) {
		errrep();
		delayrep();
		statrep();
	}

	multterm = TRUE;
	while (_dkiosts());

	if (update) {
		for (n = 0; n < 25; ++n) {
			if ((rc = multread()) >= 0)
				break;
			tickwait = 18;
			counttick = 0;
			//while (counttick < tickwait);
		}
		if (!rc)
			closeret =
			    "Read error on communications file during close";
		else if (rc < 0)
			closeret =
			    "Communications file locked during close";
		else if (editnum())
			closeret = "Bad player counter";
		else {
			multbuff->mu_state[player] = FINISHED;
			multbuff->mu_key[player] = 0;
			alldone = TRUE;
			for (n = 0; n < multbuff->mu_maxplyr; ++n)
				if (multbuff->mu_state[n] != FINISHED) {
					alldone = FALSE;
					break;
				}
			if (alldone) {
				multbuff->mu_numplyr = 0;
				multbuff->mu_lstplyr = 0;
				for (n = 0; n < MAX_PLYR; ++n)
					multbuff->mu_state[n] = WAITING;
			}

			if (!multwrite())
				closeret
				    = "Write error on communications file";
		}
	}

	if (swlocked)
		if (!multunlock())
			closeret = "Unlock error on communications file";

	return closeret;
#endif
	return NULL;
}



// wait until there is new data incoming

#ifdef IBMPC
static void _multwait()
{
	register MULTIO *mu;
	register int i;
	int count, dkerr;

	mu = multbuff;
	FOREVER {
		_dktick(18);
		for (count = 0; count < 25; ++count) {
			if (ctlbreak()) {
				error(NULL, YES);
				return;
			}
			if (!
			    (dkerr =
			     0xFF00 & _dkio(0x2, multdriv, multhead,
					    multtrk, multsect, 1, mu,
					    dsseg())))break;
			errlog(dkerr, OREADPLAY);
			_dkio(0x0);
		}
		_fromintel();
		if (count == 25) {
			error("Read error during wait", YES);
			return;
		}
		if (editnum()) {
			error("Bad player count", YES);
			return;
		}
		for (i = 0; i < mu->mu_maxplyr; ++i)
			if (mu->mu_state[i] != FLYING
			    && mu->mu_state[i] != FINISHED)
				break;
		if (i == mu->mu_maxplyr)
			return;
	}
}
#endif




static unsigned curtry = 0;

// get keyboard state and send

#ifdef IBMPC
static void _multput()
{
	register MULTIO *mu;
	register OBJECTS *ob;
	int count, dkerr;
	int tickwait;
	static BOOL first = TRUE;
	char *buff;

	ob = &nobjects[player];
	mu = multbuff;

	delaymov();
	updated(0, player);

	if (multterm)
		return;
	if (ctlbreak()) {
		error(NULL, YES);
		return;
	}

	if (first)
		first = FALSE;
	else
		mu->mu_key[player] = swgetc();

	mu->mu_state[player] = ob->ob_state;
	mu->mu_lstplyr = player;
	swflush();

	tickwait = 180;
	counttick = 0;
	count = 0;
	buff = _tointel();
	while (count < 25) {
		if (!
		    (dkerr =
		     0xFF00 & _dkio(0x3, multdriv, multhead, multtrk,
				    multsect, 1, buff, dsseg())))
			break;
		_dkio(0x0);

		if ((counttick > tickwait) && ctlbreak()) {
			error(NULL, YES);
			return;
		}

		delay();
		errlog(dkerr, OWRITEPLAY);
		if (dkerr != 0x0300)
			++count;
	}
	if (count == 25) {
		error("Write error during play", YES);
		return;
	}

	delay();

	if (editnum())
		error("Bad Player count", YES);

	updated(player + 1, mu->mu_maxplyr);
	changedelay();
}
#endif

// no idea

#ifdef IBMPC
static void updated(int n1, int n2)
{
	int n, count;
	register MULTIO *mu;
	register OBJECTS *ob;
	BOOL done;
	int dkerr, last;
	int tickwait;
	BOOL readdone = FALSE;

	mu = multbuff;
	tickwait = 180;
	counttick = 0;
	FOREVER {
		last = mu->mu_lstplyr;
		done = TRUE;
		for (n = n1; n < n2; ++n)
			if (mu->mu_state[n] != FINISHED)
				break;
		if (n < n2)
			if (player == last)
				done = FALSE;
			else if (last >= n)
				for (; n < n2; ++n)
					if (mu->mu_state[n] != FINISHED)
						done = (n == last);
		if (done)
			return;

		if (readdone)
			delay();

		for (count = 0; count < 25; ++count) {
			if (!
			    (dkerr =
			     0xFF00 & _dkio(0x2, multdriv, multhead,
					    multtrk, multsect, 1, mu,
					    dsseg())))break;
			errlog(dkerr, OREADPLAY);
			_dkio(0x0);

			if ((counttick > tickwait) && ctlbreak()) {
				error(NULL, YES);
				return;
			}
		}
		_fromintel();
		if (count == 25) {
			error("Read error during play", YES);
			return;
		}
		if (editnum())
			error("Bad player count", YES);

		readdone = TRUE;
		++curtry;
	}
}
#endif


// error stuff

#if 0
static void error(char *msg, BOOL flag1)
{
	errorflg = TRUE;
	errflg1 = flag1;
	errormsg = msg;
}




static void errlog(unsigned int err, unsigned int where)
{
	register int i;

	errtab[i = ++errptr % 50] = err;
	errwhr[i] = where;
}




static void errrepl(int i)
{

	switch (errwhr[i]) {
	case OREAD:
		puts("READ      ");
		break;
	case OWRITE:
		puts("WRITE     ");
		break;
	case OREADPLAY:
		puts("READPLAY  ");
		break;
	case OWRITEPLAY:
		puts("WRITEPLAY ");
		break;
	}
	switch (errtab[i]) {
#ifdef IBMPC
	case 0x8000:
		puts("TIME OUT");
		break;
	case 0x4000:
		puts("BAD SEEK");
		break;
	case 0x2000:
		puts("BAD NEC");
		break;
	case 0x1000:
		puts("BAD CRC");
		break;
	case 0x0900:
		puts("DMA BOUNDARY ");
		break;
	case 0x0800:
		puts("BAD DMA");
		break;
	case 0x0400:
		puts("RECORD NOT FOUND");
		break;
	case 0x0300:
		puts("WRITE PROTECT");
		break;
	case 0x0200:
		puts("BAD ADDR MARK ");
		break;
	case 0x0100:
		puts("BAD COMMAND");
		break;
#endif
	default:
		dispd(errtab[i], 6);
		break;
	}
	puts("\r\n");
}



static void errrep()
{
	register int i;

	puts("\r\n\r\n");
	dispd(errptr + 1, 5);
	puts(" i/o errors recorded\r\n\r\n");
	if (errptr >= 50) {
		puts("last 50:\r\n\r\n");
		for (i = errptr % 50 + 1; i < 50; ++i)
			errrepl(i);
		for (i = 0; i <= (errptr % 50); ++i)
			errrepl(i);
	} else
		for (i = 0; i <= errptr; ++i)
			errrepl(i);
}

#endif



static unsigned nextdelay = 2;
static unsigned maxtry = 0;
static unsigned mintry = 10000;
static unsigned numtry = 0;
static unsigned nummov = 0;
static unsigned numdel = 0;
static unsigned numadjup = 0;
static unsigned numadjdn = 0;

#if 0
static void delay()
{
#ifdef IBMPC
	register int t;

	if (multtick == -1)
		return;

	if (multtick)
		t = multtick;
	else
		t = nextdelay;

	numdel += t;
	_dktick(t);
#endif
}
#endif


void delaymov()
{
	++nummov;
	curtry = 0;
}



void changedelay()
{
	static unsigned upcnt, dncnt;

	numtry += curtry;
	if (curtry > maxtry)
		maxtry = curtry;
	if (curtry && (curtry < mintry))
		mintry = curtry;

	if (curtry <= 1) {
		if (++dncnt == 3) {
			if (nextdelay > 0) {
				--nextdelay;
				++numadjdn;
			}
			dncnt = 0;
		}
		upcnt = 0;
	} else {
		if (++upcnt == 3) {
			if (nextdelay < 5) {
				++nextdelay;
				++numadjup;
			}
			upcnt = 0;
		}
		dncnt = 0;
	}
}




void delayrep()
{
	puts("\r\nNumber of moves:        ");

	dispd(nummov, 6);

	puts("\r\nNumber of read tries:   ");

	dispd(numtry, 6);

	puts("\r\nMinimum tries/move:     ");

	if (mintry == 10000)
		dispd(0, 6);
	else
		dispd(mintry, 6);

	puts("\r\nMaximum tries/move:     ");
	dispd(maxtry, 6);

	puts("\r\nNumber of tick delays:  ");
	dispd(numdel, 6);

	puts("\r\n# of tick adjusts up:   ");
	dispd(numadjup, 6);

	puts("\r\n# of tick adjusts down: ");
	dispd(numadjdn, 6);
}




void statrep()
{
	register OBJECTS *ob;
	register int i;

	puts("\n\r\n\r");
	if (editnum()) {
		puts("Bad player count");
		return;
	}

	for (i = 0; i < multbuff->mu_maxplyr; ++i) {
		ob = &nobjects[i];
		puts("\r\nPlayer: ");
		dispd(i, 2);
		dispd(ob->ob_state, 3);
		dispd(multbuff->mu_state[i], 3);
		if (i == multbuff->mu_lstplyr)
			puts("  <- last update ");
	}
}





int editnum()
{
	register char max, num, lst;

	return (((max = multbuff->mu_maxplyr) < 0)
		|| (max > MAX_PLYR)
		|| ((num = multbuff->mu_numplyr) < 0)
		|| (num > max)
		|| ((lst = multbuff->mu_lstplyr) < 0)
		|| (lst >= max));
}




static int getmaxplyr()
{
	register int max;

	clrprmpt();
	puts(" Key maximum number of players allowed");
	FOREVER {
		if (ctlbreak())
			swend(NULL, NO);
		if (((max = (swgetc() & 0x00FF) - '0') >= 1)
		    && (max <= MAX_PLYR))
			return (max);
	}

	return 0;
}




static void mulreset()
{
	register int i;

	multbuff->mu_maxplyr = 0;
	multbuff->mu_numplyr = 0;
	multbuff->mu_lstplyr = 0;
	multbuff->mu_explseed = 0;

	for (i = 0; i < MAX_PLYR; ++i) {
		multbuff->mu_key[i] = 0;
		multbuff->mu_state[i] = WAITING;
	}
}


// connect to host

void init1mul(BOOL reset, char *device)
{
	register MULTIO *mu;
	register int n;
	int tickwait, rc;

	if (*device)
		*multfile = *device;

//        multfile[9] = '0' + getgame();

	multopen(multfile, NULL);

	multstack = (int *) (multstck + 398);

	for (n = 0; n < 25; ++n) {
		if ((rc = multread()) >= 0)
			break;
		tickwait = 18;
		counttick = 0;
		//while (counttick < tickwait);
	}
	if (!rc)
		swend("Read error on communications file ", NO);
	if (rc < 0)
		swend("Communications file locked ", NO);

	if (reset)
		mulreset();

	mu = multbuff;
	if ((first = !mu->mu_numplyr)) {
		mu->mu_maxplyr = getmaxplyr();
		mu->mu_numplyr = 0;
		mu->mu_explseed = explseed;
	} else
		explseed = mu->mu_explseed;

	clrprmpt();
	currgame = &swgames[0];
	if (mu->mu_numplyr >= mu->mu_maxplyr)
		swend("Mamimum number of players already playing", NO);
	++mu->mu_numplyr;
}

// serve

void init2mul()
{
	register OBJECTS *ob;
	int n;
	register MULTIO *mu;
	BOOL playinit = FALSE;

	mu = multbuff;
	for (n = 0; n < mu->mu_maxplyr; ++n) {
		if ((!playinit) && (mu->mu_state[n] == WAITING)) {
			player = n;
			initplyr(NULL);
			mu->mu_key[n] = 0;
			mu->mu_state[n] = FLYING;
			mu->mu_lstplyr = n;
			playinit = TRUE;
		} else {
			ob = initpln(NULL);
			ob->ob_drawf = dispmult;
			ob->ob_movef = movemult;
			ob->ob_clr = ob->ob_index % 2 + 1;
			ob->ob_owner = ob;
			ob->ob_state = mu->mu_state[n];
			oobjects[ob->ob_index] = *ob; // sdh 16/11: movmem removed
		}
	}
	if (mu->mu_maxplyr % 2)
		initcomp(NULL);

	if (!multwrite())
		swend("Write error on communications file", NO);
	if (!multunlock())
		swend("Unlock error on communications file", YES);

	clrprmpt();
	puts("      Waiting for other player(s)");
	multwait();
}



//---------------------------------------------------------------------------
//
// $Log$
// Revision 1.1  2003/02/14 19:03:17  fraggle
// Initial revision
//
//
// sdh 14/2/2003: change license header to GPL
// sdh 25/11/2001: remove intsoff, intson calls
// sdh 21/10/2001: use new obtype_t and obstate_t
// sdh 21/10/2001: rearranged headers, added cvs tags
// sdh 21/10/2001: reformatted with indent
// sdh 19/10/2001: converted all arguments to ANSI-style arguments
//                 removed externs (now in headers)
//                 shuffled some function to shut up compiler
//
// 96-12-27        Development
//
//---------------------------------------------------------------------------
