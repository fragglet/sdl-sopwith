;
;       _inta    -      Generalized interrupt service routine
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
;                       84-02-22        Development
;                       87-03-10        Microsoft compiler
;                       94-12-18        C6 Compiler
;       Usage:
;               ---------------------------------------------------
;
;               _int1vec()
;
;               Provides a list of override interrupt entry points.
;               Performs an iret to the override routine in order
;               to preserve any returned registers.
;
;               IMPORTANT:
;                       The old (overridden) interrupt service
;                       routines IP:CS is pushed on the stack,
;                       as well as the new interrupt routines
;                       data segment.
;                       To terminate the new routine, do:
;                               add     SP,8
;                               iret
;                       To transfer to the old routine, do:
;                               add     sp,2
;                               iret
;               ---------------------------------------------------
;
;               get_ivec( inum )
;               int     inum;
;
;               returns the long address for the interrupt number
;               ---------------------------------------------------
;
;               set_ivec( inum, server )
;               int     inum;
;               long    server;
;
;               returns the long address for the interrupt number
;


%       .MODEL  model,lang
        INCLUDE mixed.inc

        .CODE


INTTABSZ        equ     10

        include segments.h

        public  _int1vec
        public  _int2vec
        public  set_ivec
        public  get_ivec

;
;       vector of interrupt entry points
;

_int1vec:
        push    CS:i0
        jmp     common
_int2vec:
        push    CS:i1
        jmp     common
        push    CS:i2
        jmp     common
        push    CS:i3
        jmp     common
        push    CS:i4
        jmp     common
        push    CS:i5
        jmp     common
        push    CS:i6
        jmp     common
        push    CS:i7
        jmp     common
        push    CS:i8
        jmp     common
        push    CS:i9
        jmp     common

;
;       common interrupt handler
;

common:
        sub     SP,20                   ; save BX,DS on stack
        push    BX
        push    DS
        add     SP,24

        mov     BX,DGROUP               ; get DS from save area
        mov     DS,BX                   ;
        pop     BX                      ; get interrupt table offset from stack

        pushf                           ; old routine flags
        push    _inttab[BX+6]           ;             CS
        push    _inttab[BX+4]           ;             IP
        push    DS                      ; new routine DS
        pushf                           ;             flags
        push    CS                      ;             CS
        push    _inttab[BX+0]           ;             IP


        sub     SP,12                   ; restore BX,DS
        pop     DS
        pop     BX
        add     SP,8

        iret                            ; iret to interrupt service routine

;
;       if here, the service routine did not iret
;

        xor     AH,AH                   ; terminate program DOS call
        int     21H


;       data for push of interrupt number

i0      dw      0*INTTABSZ
i1      dw      1*INTTABSZ
i2      dw      2*INTTABSZ
i3      dw      3*INTTABSZ
i4      dw      4*INTTABSZ
i5      dw      5*INTTABSZ
i6      dw      6*INTTABSZ
i7      dw      7*INTTABSZ
i8      dw      8*INTTABSZ
i9      dw      9*INTTABSZ



get_ivec:
        push    BP                      ; save frame pointer
        mov     BP,SP                   ; new frame pointer
        push    ES                      ; save registers

        mov     AH,35H                  ; get interrupt function
        mov     AL,@AB[BP]              ; interrupt number
        int     21H                     ; get server in ES:BX
        mov     DX,ES                   ; return in DX:AX
        mov     AX,BX                   ;

        pop     ES                      ; restore registers
        pop     BP                      ;
        ret                             ; return


set_ivec:
        push    BP                      ; save frame pointer
        mov     BP,SP                   ; new frame pointer
        push    DS                      ; save registers

        mov     AH,25H                  ; set interrupt function
        mov     AL,@AB[BP]              ; interrupt number
        lds     DX,dword ptr @AB+2[BP]  ; get server in DS:DX
        int     21H                     ; set new server

        pop     DS                      ; restore registers
        pop     BP                      ;
        ret                             ; return



        .DATA

        extrn   _inttab:word

        end
