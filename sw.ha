;
;       sw.ah    -      SW Assembler Manifests
;
;                       Copyright (C) 1984-2000 David L. Clark.
;
;                       All rights reserved except as specified in the
;                       file license.txt.  Distribution of this file
;                       without the license.txt file accompanying is
;                       prohibited.
;
;                       Author: Dave Clark
;
;       Modification History:
;                       84-02-02        Development
;                       84-06-12        PCjr Speed-up
;                       87-03-04        Missile and starburst support
;
;-----------------------------------------------------------------------------

MAX_X           equ     3000            ; Maximum X coordinate
MAX_Y           equ     200             ; Maximum Y coordinate

SCR_WDTH        equ     320             ; Screen Width
SCR_HGHT        equ     200             ; Screen Height
SCR_CENTR       equ     152             ; Centre column of screen
SCR_SEGM        equ     0B800H          ; Screen Segment
SCR_ROFF        equ     02000H          ; Screen odd rastor line offset
SCR_LINW        equ     80              ; Screen line width in bytes
SCR_MNSH        equ     16              ; Minumum line number to shift
SCR_MXSH        equ     75              ; Maximum line numbner to shift


PCDOS           equ     0               ;  Operating environments
JRROMDOS        equ     1
JRROM           equ     2

SINGLE          equ     0               ;  Playmodes
MULTIPLE        equ     1
COMPUTER        equ     2

K_ACCEL         equ     0001H           ; Keyboard word masks
K_DEACC         equ     0002H
K_FLAPU         equ     0004H
K_FLAPD         equ     0008H
K_FLIP          equ     0010H
K_SHOT          equ     0020H
K_BOMB          equ     0100H
K_HOME          equ     0200H
K_SOUND         equ     0400H
K_BREAK         equ     0800H
K_MISSILE       equ     1000H
K_STARBURST     equ     2000H
K_ASYNACK       equ     40H

K_ASYNACK       equ     40H


OB_STATE        equ     0               ; Object list element
OB_X            equ     2
OB_Y            equ     4
OB_DX           equ     6
OB_DY           equ     8
OB_ANGLE        equ     10
OB_ORIENT       equ     12
OB_SPEED        equ     14
OB_ACCEL        equ     16
OB_FLAPS        equ     18
OB_FIRING       equ     20
OB_SCORE        equ     22
OB_ROUNDS       equ     24
OB_HITCOUNT     equ     26
OB_UPDCOUNT     equ     28
OB_LIFE         equ     30
OB_OWNER        equ     32
OB_SYMHGT       equ     34
OB_SYMWDT       equ     36
OB_BOMBING      equ     38
OB_BOMBS        equ     40
OB_CLR          equ     42
OB_LX           equ     44
OB_LY           equ     46
OB_LDX          equ     48
OB_LDY          equ     50
OB_NEXT         equ     52
OB_PREV         equ     54
OB_INDEX        equ     56
OB_OLDX         equ     58
OB_OLDY         equ     60
OB_DRWFLG       equ     62
OB_DELFLG       equ     64
OB_OLDSYM       equ     66
OB_DRAWF        equ     68
OB_MOVEF        equ     70
OB_XNEXT        equ     72
OB_XPREV        equ     74
OB_CRASHCNT     equ     76
OB_NEWSYM       equ     78
OB_BDELAY       equ     80
OB_HOME         equ     82
OB_HX1          equ     84
OB_HX2          equ     86
OB_HX3          equ     88
OB_HY1          equ     90
OB_HY2          equ     92
OB_HY3          equ     94
OB_TYPE         equ     96
OB_DNEXT        equ     98
OB_ATHOME       equ    100
OB_SOUND        equ    102
OB_MISSILES     equ    104
OB_MFIRING      equ    106
OB_MDELAY       equ    108
OB_TARGET       equ    110
OB_BURSTS       equ    112
OB_BFIRING      equ    114
OB_FILL1        equ    116
OB_SIZEOF       equ    118
