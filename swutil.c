
// sdh: asm routines
// some of these arent used (dos specific empty functions)
// some of them are modified from andrew jenners source functions
// as i cant read x86 asm :(

#include "sw.h"

int ctlbflag;

void
movexy(ob, x, y)
OBJECTS *ob;
int *x, *y;
{
	long pos, vel;
//	pos = (((long) (ob->ob_x)) << 16) + ob->ob_lx;
//	vel = (((long) (ob->ob_dx)) << 16) + ob->ob_ldx;
	pos = (ob->ob_x + ob->ob_dx) << 16;
	ob->ob_x = (short) (pos >> 16);
	ob->ob_lx = (short) pos;
	*x = ob->ob_x;
//	pos = (((long) (ob->ob_y)) << 16) + ob->ob_ly;
//	vel = (((long) (ob->ob_dy)) << 16) + ob->ob_ldy;
	pos = (ob->ob_y + ob->ob_dy) << 16;
	ob->ob_y = (short)(pos >> 16);
	ob->ob_ly = (short) pos;
	*y = ob->ob_y;
}

void
setdxdy(obj, dx, dy)
OBJECTS *obj;
int dx, dy;
{
	obj->ob_dx = dx >> 8;
	obj->ob_ldx = dx << 8;
	obj->ob_dy = dy >> 8;
	obj->ob_ldy = dy << 8;
}

int swgetc()
{
	return CGA_GetLastKey();
}

void swputc(int a) {} // display character
void commin() {}  // communications 
void commout() {}
void trap14() {}  // used a lot but i dont know what it does
void swflush() {} // something to do with the keyboard
void swsetblk() {}
void swprint() {}  // print screen?
void swgetjoy() {} // yeah right
void histend() {} // demos?

