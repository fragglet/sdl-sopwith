;
;       swutil   -      SW assembler utilities
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
;                       84-02-23        Development
;                       84-06-12        PCjr Speed-up
;                       84-06-15        Joystick support
;                       87-03-10        Microsoft compiler
;                       87-03-04        Missile and starburst support
;                       87-03-05        Remap missile and starburst keys
;                       87-04-09        Fix to non-IBM keyboard.
;                       94-12-18        C6 Compiler
;


%       .MODEL  model,lang
        INCLUDE mixed.inc

        .CODE


        include segments.h
        include sw.ha

        public  swbreak
        public  swgetc
        public  swgetjoy
        public  swputc
        public  swflush
        public  swkeyint
        public  swposcur
        public  swcolour
        public  swshfprt
        public  swprint
        public  swsetblk
        public  swtick
        public  movexy
        public  setdxdy
        public  soundmul
        public  sounddiv
        public  dsseg
        public  csseg

        extrn   swtickc:near
        extrn   ctlbreak:near
        extrn   history:near
        extrn   swsound:near
        extrn   sound:near



;------------------------------------------------------------------------------

swbreak:
        push    DS
        push    BX
        push    BP

        mov     BP,SP                   ; setup DS from stack
        mov     BX,[BP+06]
        mov     DS,BX

        mov     ctlbflag,1              ; flag control break

        pop     BP                      ; restore registers
        pop     BX
        pop     DS

        add     SP,8                    ; skip swbreak DS and old routine
        iret                            ; return to interrupted process


;-----------------------------------------------------------------------------

swkeyint:
        sti                             ; allow interrupts

        push    BP                      ; save registers
        push    DS                      ;
        mov     BP,SP                   ; set DS to sw data segment
        mov     BP,[BP+4]               ;
        mov     DS,BP                   ;
        mov     byte ptr keyflag,1      ; flag keystroke having occurred

        cmp     word ptr ibmkeybd,0     ; IBM keyboard?
        jne     kii                     ;   YES - skip
        pop     DS                      ;   NO  - use old key int routine
        pop     BP                      ;
        add     SP,2                    ;
        iret                            ;

kii:    push    AX                      ; save registers
        push    BX
        push    CX
        push    DX
        push    ES
        push    SI
        push    DI

        mov     ES,BP                   ; set ES to data space
        mov     byte ptr paused,0       ; flag no longer in paused mode

        cmp     word ptr _systype,PCDOS ; running under DOS on PC
        je      kir                     ;   YES- read key from port
        mov     BL,AL                   ;   NO - key already in AL
        jmp     ki0                     ;

kir:    in      AL,60H                  ; read scan code
        mov     BL,AL                   ; save it
        in      AL,61H                  ; read control port
        mov     AH,AL                   ; save it
        or      AL,80H                  ; reset keyboard
        out     61H,AL
        mov     AL,AH                   ; restore old control port
        out     61H,AL                  ; keyboard now reset

ki0:    cmp     BL,55                   ; print screen key?
        jne     ki1                     ;  NO - continue
        int     5                       ;  YES- perform print screen int
        jmp     kiret                   ;       leave

ki1:    cmp     BL,99H                  ; release of P key ( pause )
        jne     ki15                    ;  NO - continue
        mov     byte ptr paused,1       ;  YES- flag in pause mode
        cmp     _systype,PCDOS          ;       running under DOS on PC?
        jne     ki11                    ;          NO - skip
        mov     AL,20H                  ;          YES- signal end of interrupt
        out     20H,AL                  ;
ki11:   cmp     byte ptr soundflg,1     ;       sound on?
        jne     ki13                    ;
        xor     AX,AX                   ;          YES- turn off sound
        push    AX                      ;
        push    AX                      ;
        call    sound                   ;
        add     SP,4                    ;
        call    swsound                 ;
        mov     byte ptr soundflg,0     ;
ki12:   test    byte ptr paused,1       ;               loop until next keyboard
        jnz     ki12                    ;               interrupt
        mov     byte ptr soundflg,1     ;               turn sound on
        jmp     ki5                     ;               leave

ki13:   test    byte ptr paused,1       ;          NO - loop until next keyboard
        jnz     ki13                    ;               interrupt
        jmp     ki5                     ;               leave

ki15:   cmp     BL,70                   ; break key?
        jne     ki2                     ;  NO - continue
        int     1BH                     ;  YES- perform break int, don't leave


ki2:    mov     CX,12                   ; length of scan code table
        mov     AL,BL                   ; scan code to be searched
        and     AL,7FH                  ; strip off break code
        lea     DI,scantable            ; scan code table address
        mov     SI,DI                   ; save it
        cld                             ; clear direction flag
        repnz   scasb                   ; search table for scan code
        jnz     kiret                   ; leave if not found

        dec     DI                      ; back DI off 1 byte
        sub     DI,SI                   ; offset into scan code table
        shl     DI,1                    ; offset into scan mask table
        lea     SI,scanmasks            ; address of mask table
        add     SI,DI                   ; address of mask
        mov     AX,[SI]                 ; load mask
        cli                             ; prevent interrupts

        test    BL,80H                  ; break code?
        jz      ki3
        test    prevkey,AX              ;   YES-was key up on last input
        not     AX                      ;       invert flag
        jz      ki25                    ;       NO - skip
        and     nextkey,AX              ;       YES- set bit off on next input
ki25:   and     currkey,AX              ;       set off bit in current
        jmp     kiret

ki3:    or      currkey,AX              ;   NO -set bit in current key word
        or      nextkey,AX              ;       set bit in next key word

kiret:  cmp     _systype,PCDOS          ; running under DOS on PC?
        jne     ki5                     ;   NO - skip

        cli                             ; prevent interrupts
        mov     AL,20H                  ; signal end of interrupt
        out     20H,AL

ki5:    pop     DI                      ; restore registers
        pop     SI
        pop     ES
        pop     DX
        pop     CX
        pop     BX
        pop     AX
        pop     DS
        pop     BP
        add     SP,8                    ; cleanup stack
        iret

;------------------------------------------------------------------------------



;       swgetc()
;
;       get a character from the keyboard.  The ascii character
;       returned is in in AL, the scan code in AH.   Routine
;       will return 0 if no character is available.
;

swgetc:
        cmp     word ptr inplay,0       ; Currently in play?
        jnz     getc2                   ;   YES- jump to new keyboard code
        call    getbio                  ;   NO - perform BIOS get character
        call    history                 ;        post to history
        ret                             ;

getc2:  cmp     word ptr ibmkeybd,0     ; Using IBM lookalike keyboard?
        jz      getc3                   ;   NO - jump to non-IBM code
        cli                             ; turn off interrupts
        push    nextkey                 ; save word to be returned
        mov     AX,currkey              ; reset to current keyboard status
        mov     nextkey,AX              ;
        pop     AX                      ; return current keyboard word
        mov     prevkey,AX              ; save it
        sti                             ; allow interrupts
        jmp     getc7                   ; go to joystick code

getc3:  push    DI                      ; save registers
        push    SI
        call    getbio                  ; get a character in AX
        cmp     AX,0                    ; anything read?
        jz      getc5                   ; NO - leave

        mov     SI,DS                   ; point ES to data
        mov     ES,SI                   ;
        lea     DI,lkeytable            ; search lower case table for key mask
        call    getcm
        jz      getc5                   ; character mask found
        lea     DI,ukeytable            ; search upper case table
        call    getcm
        jz      getc5                   ; character mask found

        xor     AX,AX                   ; no valid key pressed

getc5:  push    AX                      ; save return mask
        call    ctlbreak                ; control break pressed?
        cmp     AX,0                    ;
        jz      getc6                   ;   NO - skip
        mov     AX,K_BREAK              ;   YES- load control break mask

getc6:  pop     SI                      ; pop key mask
        or      AX,SI                   ; or with possible ctlbreak mask
        pop     SI                      ; restore registers
        pop     DI

getc7:  cmp     word ptr joystick,0     ; using joystick?
        jz      getcret                 ;   NO - leave
        push    AX                      ;   YES- save keyboard status
        mov     AX,nextjoy              ;        reset joystick words
        mov     prevjoy,AX              ;
        mov     AX,currjoy              ;
        mov     nextjoy,AX              ;
        pop     AX                      ;        restore keyboard status
        or      AX,prevjoy              ;        include joystick status

getcret:call    history                 ; post to history
        ret                             ; return



getcm:
        mov     SI,DS                   ; point ES to data
        mov     ES,SI                   ;
        mov     CX,12                   ; length of key table
        mov     SI,DI                   ; save address of table
        cld                             ; clear direction flag
        repnz   scasb                   ; search table for scan code
        jnz     getcmr                  ; leave if not found

        dec     DI                      ; back DI off 1 byte
        sub     DI,SI                   ; offset into key table
        shl     DI,1                    ; offset into scan mask table
        lea     SI,scanmasks            ; address of mask table
        add     SI,DI                   ; address of mask
        mov     AX,[SI]                 ; load mask
        xor     SI,SI                   ; turn on zero bit
getcmr: ret



getbio:
        mov     AH,01H                  ; Character available function code
        int     16H                     ; KEYBOARD_IO interrupt
        jnz     getb1                   ; character available?
        xor     AX,AX                   ;   NO - return with 0
        ret
getb1:
        mov     AH,00H                  ; Read character function code
        int     16H                     ; KEYBOARD_IO interrupt
        ret




;
;       swgetjoy();
;

JOYHIGH equ     0780H
JOYLOW  equ     0280H
JOYMAX  equ     0A00H

swgetjoy:
        cmp     word ptr joystick,0     ; Using joysticks?
        jz      swjret                  ;   NO - leave

        mov     AX,0                    ; get joystick 0 in DX
        call    getjoy
        mov     DX,AX
        mov     AX,1                    ; get joystick 1 in AX
        call    getjoy

        lea     BX,joymasks+8
        cmp     AX,JOYLOW               ; get index into table 012
        jg      swj1                    ; in BX                345
        sub     BX,6                    ;                      678
        jmp     swjx
swj1:   cmp     AX,JOYHIGH
        jl      swjx
        add     BX,6
swjx:   cmp     DX,JOYLOW
        jg      swj2
        sub     BX,2
        jmp     swj3
swj2:   cmp     DX,JOYHIGH
        jl      swj3
        add     BX,2

swj3:   mov     BX,[BX]                 ; joystick mask in BX

        mov     DX,201H                 ; get button status
        in      AL,DX
        test    AL,10H                  ; button 1 pushed?
        jnz     swj4                    ;   NO -skip
        or      BX,K_SHOT               ;   YES-shoot
swj4:   test    AL,20H                  ; button 2 pushed?
        jnz     swj5                    ;   NO - skip
        or      BX,K_BOMB               ;   YES- drop bomb

swj5:   mov     currjoy,BX              ; update current joystick status
        or      nextjoy,BX              ; turn on bits in next status
        mov     AX,BX                   ; invert mask
        not     AX
        and     AX,prevjoy              ; previously on to turn off
        not     AX                      ; invert for and
        and     nextjoy,AX              ; turn off if previosly on

swjret: ret


getjoy:
        push    BX
        push    CX
        push    DX
        cli                             ; disable interrupts

        mov     CX,AX                   ; joystick requested
        mov     CH,1                    ; initial joystick mask
        shl     CH,CL                   ; actual joystick mask

        mov     DX,201H                 ; adapter address
        in      AL,DX                   ; is joystick bit already up?
        test    AL,CH
        jz      gj0                     ;   NO - continue

        mov     AX,JOYMAX               ;   YES- return high value
        jmp     gjret

gj0:    mov     byte ptr keyflag,0      ; set off keyboard int flag
        mov     AL,00H                  ; Read timer 0
        out     43H,AL
        in      AL,40H                  ; read LSB
        mov     BL,AL
        in      AL,40H                  ; read MSB
        mov     BH,AL

        out     DX,AL                   ; trigger games controller read

gj1:    in      AL,DX                   ; get adapter status
        test    AL,CH                   ; bit still high
        jnz     gj1                     ; yes, try again

        mov     AL,00H                  ; Read timer 0
        out     43H,AL
        in      AL,40H                  ; read LSB
        mov     AH,AL
        in      AL,40H                  ; read MSB
        xchg    AL,AH

        cmp     byte ptr keyflag,0      ; keyboard interrupt occured?
        je      gj2                     ;    NO  - skip
        mov     AX,(JOYHIGH-JOYLOW)/2   ;    YES - return mid range
        jmp     gjret                   ;

gj2:    sub     BX,AX                   ; elapsed time
        mov     AX,BX                   ; return it.

gjret:  sti                             ; enable interrupts
        pop     DX
        pop     CX
        pop     BX
        ret


;       swflush()
;
;       flush the keyboard buffer.
;

swflush:
        mov     AX,inplay               ; Currently in play?
        and     AX,ibmkeybd             ; and IBM like keyboard?
        jnz     swfret                  ;   YES - ignore request

swflp:  mov     AH,01H                  ; Character available function code
        int     16H                     ; KEYBOARD_IO interrupt
        jnz     flush1                  ; character available?

swfret: xor     AX,AX                   ;   NO - return with 0
        ret

flush1:
        mov     AH,00H                  ; Read character function code
        int     16H                     ; KEYBOARD_IO interrupt
        jmp     swflp                   ; loop back until nothing left




;       swputc( c )
;
;       output a character
;

swputc:
        push    BP                      ; save frame pointer
        mov     BP,SP                   ; access stack pointer

        mov     AL,4[BP]                ; get character to output
        cmp     AL,9                    ; horizontal tab?
        jne     swpc1                   ;   NO - skip
        push    CX                      ;   YES- save registers
        push    DX                      ;
        mov     AH,3                    ;        read cursor position
        xor     BH,BH                   ;
        int     10H                     ;
        add     DL,8                    ;       goto next tab
        and     DL,0F8H                 ;
        mov     AH,2                    ;
        int     10H                     ;
        pop     DX                      ;       restore registers
        pop     CX                      ;
        jmp     swpc2                   ;       leave

swpc1:  mov     BL,textclr              ; set colour and page
        xor     BH,BH                   ;
        mov     AH,0EH                  ; output character
        int     10H                     ;

        cmp     word ptr hires,1        ; hi resolution screen?
        jne     swpc2                   ;   NO - skip
        cmp     byte ptr 4[BP],' '      ;   YES- control character?
        jb      swpc2                   ;          YES- skip
        mov     AX,0E20H                ;          NO - output blank
        int     10H                     ;

swpc2:  pop     BP                      ; restore registers
        ret



;       swposcur( x, y )   -  Position the character cursor to the
;       character position (x,y)

swposcur:
        push    BP                      ; save frame pointer
        mov     BP,SP                   ; access stack pointer

        mov     DH,6[BP]                ; y coordinate
        mov     DL,4[BP]                ; x coordinate
        xor     BH,BH                   ; page 0

        cmp     word ptr hires,1        ; hi resolution screen?
        jne     swpos1                  ;   NO - skip
        shl     DL,1                    ;   YES- double x coordinate
swpos1: mov     AH,02H                  ; position cursor
        int     10H                     ;

        pop     BP
        ret




;       swcolour( x )   -  Set the text display colour.

swcolour:
        push    BP                      ; save frame pointer
        mov     BP,SP                   ; access stack pointer
        mov     AL,4[BP]                ; save colour
        mov     textclr,AL              ; save colour
        pop     BP
        ret


;       swshfprt()     - new interrupt server for shift print screen

swshfprt:
        push    DS                      ; save registers
        push    BP
        push    BX

        mov     BP,SP                   ; setup DS from DS on stack
        mov     BX,[BP+6]
        mov     DS,BX

        mov     word ptr printflg,1     ; flag print screen pressed
        mov     BX,[BP+8]               ; save old print screen location
        mov     printip,BX
        mov     BX,[BP+10]
        mov     printcs,BX

        pop     BX                      ; restore registers
        pop     BP
        pop     DS
        add     SP,8                    ; cleanup stack
        iret

;       swprint()      - perform a shift print screen function

swprint:
        pushf                           ; duplicate shift print int
        call    dword ptr printip
        ret                             ; return


;------------------------------------------------------------------------------



swtick:
        push    DS
        push    ES
        push    AX
        push    BX
        push    CX
        push    DX
        push    SI
        push    DI
        push    BP

        mov     BP,SP                   ; setup DS,ES from DS on stack
        mov     BX,[BP+18]
        mov     DS,BX
        mov     ES,BX

        cmp     byte ptr paused,1       ; currently paused?
        je      swtck1                  ;   YES - skip
        call    swtickc                 ;   NO  - call SW tick processing

swtck1: pop     BP                      ; restore registers
        pop     DI
        pop     SI
        pop     DX
        pop     CX
        pop     BX
        pop     AX
        pop     ES
        pop     DS

        add     SP,2                    ; skip swtick DS on stack
        iret                            ; transfer to old tick routine


;------------------------------------------------------------------------------



swsetblk:
        push    bp
        mov     bp,sp
        push    es
        mov     ax,6[bp]        ;get the extra seg
        mov     es,ax
        cld                     ;and clear the direction flag
        mov     di,4[bp]        ;the address to set
        mov     cx,8[bp]        ;the number of bytes to set
        mov     al,10[bp]       ;the value to set
        rep     stosb           ;set the area
        pop     es
        pop     bp
        ret


;------------------------------------------------------------------------------



OB      equ     @AB
X       equ     @AB+2
Y       equ     @AB+4


movexy:
        push    BP                      ; save frame pointer
        mov     BP,SP                   ; access stack pointer
        push    SI
        push    DI

        mov     DI,OB[BP]               ; object pointer

        mov     AX,OB_X[DI]             ; load registers
        mov     SI,OB_LX[DI]
        mov     CX,OB_DX[DI]
        mov     DX,OB_LDX[DI]

        add     SI,DX                   ; new X
        adc     AX,CX

        mov     OB_X[DI],AX
        mov     OB_LX[DI],SI
        mov     BX,X[BP]                ; return new X
        mov     [BX],AX

        mov     AX,OB_Y[DI]             ; load registers
        mov     SI,OB_LY[DI]
        mov     CX,OB_DY[DI]
        mov     DX,OB_LDY[DI]

        add     SI,DX                   ; new Y
        adc     AX,CX

        mov     OB_Y[DI],AX
        mov     OB_LY[DI],SI
        mov     BX,Y[BP]                ; return new Y
        mov     [BX],AX

        pop     DI                      ; restore registers
        pop     SI
        pop     BP
        ret




OB      equ     @AB
X       equ     @AB+2
Y       equ     @AB+4


setdxdy:
        push    BP                      ; save frame pointer
        mov     BP,SP                   ; access stack pointer

        mov     BX,OB[BP]               ; object pointer

        mov     CX,X[BP]                ; X increment *256
        mov     AL,CH
        cbw                             ; X increment in AX
        mov     OB_DX[BX],AX            ;   save it.
        mov     AH,CL
        xor     AL,AL                   ; X remainder in AX
        mov     OB_LDX[BX],AX           ;   save it.

        mov     CX,Y[BP]                ; Y increment *256
        mov     AL,CH
        cbw                             ; Y increment in AX
        mov     OB_DY[BX],AX            ;   save it.
        mov     AH,CL
        xor     AL,AL                   ; Y remainder in AX
        mov     OB_LDY[BX],AX           ;   save it.

        pop     BP
        ret


;------------------------------------------------------------------------------
;
;       sound routine arithmetic
;

soundmul:
        push    BP
        mov     BP,SP

        mov     AX,4[BP]
        mul     word ptr 6[BP]
        mov     BX,DX

        mul     word ptr 8[BP]

        mov     AX,BX
        mov     BX,DX
        mul     word ptr 8[BP]

        add     AX,BX

        pop     BP
        ret


sounddiv:
        push    BP
        mov     BP,SP

        mov     AX,4[BP]
        mov     DX,6[BP]
        div     word ptr 8[BP]

        pop     BP
        ret


;------------------------------------------------------------------------------
;
;       dsseg();
;       csseg();
;
;       segment returning functions
;


dsseg:
        mov     AX,DS
        ret

csseg:
        mov     AX,CS
        ret



;------------------------------------------------------------------------------

        .DATA

        public  ctlbflag

        extrn   joystick:word           ; joystick being used flag
        extrn   ibmkeybd:word           ; IBM-like keyboard being used
        extrn   inplay:word             ; game is in play flag
        extrn   printflg:word           ; print screen requested
        extrn   soundflg:word           ; sound flag
        extrn   hires:word              ; hi resolution flag

ctlbflag dw     0                       ; control break flag
textclr db      3                       ; current text colour
prevkey dw      0                       ; previous keyboard return word
currkey dw      0                       ; current keyboard return word
nextkey dw      0                       ; next keyboard return word
prevjoy dw      0                       ; previous joystick return word
currjoy dw      0                       ; current joystick return word
nextjoy dw      0                       ; next joystick return word
keyflag db      0                       ; keyboard interrupt has occured
paused  db      0                       ; paused mode flag

scantable db    45,44,51,53,52,57,48,35,31,70,47,46     ; scan code table
scanmasks dw    K_ACCEL,K_DEACC,K_FLAPU,K_FLAPD,K_FLIP  ; scan mask table
          dw    K_SHOT, K_BOMB, K_HOME, K_SOUND,K_BREAK
          dw    K_MISSILE, K_STARBURST
lkeytable db    "xz,/. bhs vc"          ; lower case keystroke table
ukeytable db    "XZ<?> BHS VC"          ; upper case keystroke table
joymasks  dw    K_FLIP,K_FLAPD,K_FLIP,K_DEACC,0,K_ACCEL ; jostick scan masks
          dw    K_FLIP,K_FLAPU,K_FLIP

printip   dw    0                       ; shift-print-screen server offset
printcs   dw    0                       ;                           segment

          extrn _systype:word           ; operating system

        end
