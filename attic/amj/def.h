/* Source code for Sopwith
   Reverse-engineered by Andrew Jenner

   Copyright (c) 1984-2000 David L Clark
   Copyright (c) 1999-2000 Andrew Jenner

   All rights reserved except as specified in the file license.txt.
   Distribution of this file without the license.txt file accompanying is
   prohibited.
*/

typedef int bool;

#define TRUE (-1)
#define FALSE 0

#define fsin(r,a) ((r)*sine[(a)&15])
#define fcos(r,a) ((r)*sine[((a)+4)&15])

#define SPAREBIRDS 1

#define SC_S     0x1f
#define SC_H     0x23
#define SC_Z     0x2c
#define SC_X     0x2d
#define SC_B     0x30
#define SC_COMMA 0x33
#define SC_DOT   0x34
#define SC_SLASH 0x35
#define SC_SPACE 0x39
#define SC_BREAK 0x46

#define JOY_BUT1 0x10
#define JOY_BUT2 0x20

#define PORT_TIMER0 0x40
#define PORT_TIMER2 0x42
#define PORT_TIMERC 0x43
#define PORT_KEYB   0x60
#define PORT_SPKR   0x61
#define PORT_JOY    0x201

/* There is no significance to these values */

#define BUILDING_FLAG 0
#define BUILDING_CHIM 1
#define BUILDING_FUEL 2
#define BUILDING_TANK 3

#define GAME_SINGLE   0
#define GAME_MULTIPLE 1
#define GAME_COMPUTER 2
#define GAME_ASYNCH   3

#define KEY_ACCEL   1
#define KEY_BRAKE   2
#define KEY_CLIMB   4
#define KEY_FLIP    8
#define KEY_DESCEND 0x10
#define KEY_FIRE    0x20
#define KEY_BOMB    0x100
#define KEY_HOME    0x200
#define KEY_SOUND   0x400
#define KEY_BREAK   0x800

#define OBJ_NONE     0
#define OBJ_PLANE    1
#define OBJ_BOMB     2
#define OBJ_BULLET   3
#define OBJ_BUILDING 4
#define OBJ_SHRAPNEL 5
#define OBJ_SMOKE    6
#define OBJ_FLOCK    7
#define OBJ_BIRD     8
#define OBJ_COW      9

#define STATUS_UNKNOWN    0
#define STATUS_NORMAL     1
#define STATUS_DEAD       4
#define STATUS_FALLING    5
#define STATUS_INTACT     6 /* Used for buildings and cows */
#define STATUS_SPINNING   7
#define STATUS_ELIMINATED 0x5b /* Was 3 in Sopwith 1 */
#define STATUS_ELIMNORM   0x5c
#define STATUS_ELIMDEAD   0x5d
#define STATUS_ELIMSPIN   0x5e

#define FINALE_NONE 0
#define FINALE_WON  1
#define FINALE_LOST 2

#define SOUND_OFF      0
#define SOUND_TUNE     5
#define SOUND_SHRAPNEL 10
#define SOUND_BOMB     20
#define SOUND_FIRING   30
#define SOUND_FALLING  40
#define SOUND_STUTTER  50
#define SOUND_ENGINE   60
#define SOUND_NONE     0x7fff

