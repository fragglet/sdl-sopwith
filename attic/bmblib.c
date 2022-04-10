// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------------
//
// $Id$
//
// Copyright(C) 1984-2000 David L. Clark
// Copyright(C) 2001 Simon Howard
//
// All rights reserved except as specified in the file license.txt.
// Distribution of this file without the license.txt file accompanying
// is prohibited.
//
//---------------------------------------------------------------------------
//
//        bmblib  -       SW Old BMB STDLIB routines
//
//---------------------------------------------------------------------------

#include <ctype.h>
#include <string.h>

#include "bmblib.h"
#include "sw.h"

int _systype = PCDOS;

// sdh: a lot of these functions are unused now, especially the dos
// interrupt ones. remove at some point.

// sdh: rewritten arg checking, not as powerful but it works

int Args_CheckArg(int argc, char **argv, char arg)
{
	int i;

	for (i = 1; i < argc; ++i) {
		char *p = argv[i];
		if (*p == '-')
			++p;
		if (*p == arg && !*(p + 1))
			return i;
	}

	return -1;
}

/*----------------------------------------------------------------------------
                GETFLAGS flags processing ( Jack Cole )
----------------------------------------------------------------------------*/

// sdh: this is broken somewhere, giving unused flags (eg -f) crashes

#if 0
int getflags(int *ac, char **av, char *fmt, int *flds[])
{
	char **arg, *apend;
	int i, j, k;
	register char *aptr;
	char *flag;
	register char *fptr;
	int **var;
	int *adrvar;
	char *cfmt;
	int sts = 0;
	int got_next;

	arg = (char **) *av;
	i = *ac;
	++arg;			/* point past program name */

	while (--i) {		/* for all args */
		aptr = *arg;	/* point at string */
		if (*aptr++ != '-')
			break;	/* past the switches */
		if (*aptr == '-') {	/* or -- at eol */
			++arg;	/* flush it */
			--i;
			break;
		}

	      nextbool:
		flag = aptr;	/* get the switch */
		var = (int **) &flds;	/* find in format */

		for (fptr = fmt; *fptr && *fptr != ':'; ++var) {
			j = 0;
			while (isalnum(*(fptr + j)))
				++j;	/* get switch length */

			/* match and number or space following? */

			if (!strncmp(flag, fptr, j)
			    && (!isalpha(*(flag + j))
				|| *(fptr + j) != '#')) break;

			fptr += j;	/* skip to next */
			if (*fptr)
				++fptr;	/* past format */
		}

		if (!(*fptr) || *fptr == ':') {	/* no match? */
			if ((k = strindex(fmt, ':'))) {	/* find usage info */
				cfmt = fmt + k - 1;
				*cfmt++ = '\0';
				if (!(*cfmt)) {	/* return on error? */
					sts = 1;
					goto ret;
				}
			}
			exit(0);
		}

		flag = fptr + j;	/* the type */
		aptr += j;
		adrvar = *var;	/* this is addr of real var */
		got_next = NO;
		if (*aptr == 0 && *flag != '&') {	/* more expected */
			if (i > 1) {	/* any more args? */
				aptr = *(++arg);	/* step to next arg */
				--i;
				got_next = YES;
			}
		}

		switch (*flag) {	/* what kind expected */
		case '#':
			j = 0;
			if (*aptr) {	/* any more chars? */
				*adrvar = strtol(aptr, &apend, 10);
				j = apend - aptr;
				aptr = apend;
			}
			if (!j) {	/* how many digits? */
				if (got_next) {	/* none - push back? */
					--arg;	/* yes, push back */
					++i;
				}
				*(int *) adrvar = -1;	/* flag present, but no arg */
			}
			break;

		case '*':
			*(char **) adrvar = aptr;
			break;

		case '&':
			*(int *) adrvar = YES;	/* boolean */
			break;
		}

		if (*flag == '&' && *aptr)
			goto nextbool;
		++arg;
	}

      ret:
	*av = (char *) arg;	/* point past those processed */
	*ac = i;
	return (sts);		/* successful */
}
#endif

int strindex(char *str, int c)
{
	char *s;

	return ((s = strchr(str, c)) == NULL ? 0 : s - str + 1);
}


#ifdef IBMPC
int inportb(unsigned port)
{
	return (inp(port));
}

int outportb(unsigned port, int data)
{
	return (outp(port, data));
}

#endif


// sdh 16/11/2001: movmem/setmem removed

// sdh 19/10/2001: removed sysint functions

void intson()
{
}

void intsoff()
{
}


//---------------------------------------------------------------------------
//
// $Log$
// Revision 1.1  2003/02/14 19:02:45  fraggle
// Initial revision
//
//
// sdh 16/11/2001: movmem, setmem removed
// sdh 21/20/2001: rearranged headers, added cvs tags
// sdh 21/10/2001: reformatted with indent
// sdh 20/10/2001: rewritten argument checking
//
// 94-12-19        Original development
//
//---------------------------------------------------------------------------

