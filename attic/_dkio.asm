;
;       _dkio    -      Interrupt driven diskette i/o
;                       ** Modified to support short diskette sectors **
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
;                       84-05-17        32 byte sectors
;                       84-05-23        Disable interrupts around stack
;                                       changes
;                       85-06-09        Non-transparent board
;                       87-03-10        Microsoft compiler
;                       94-12-18        C6 Compiler
;       Usage:
;               ---------------------------------------------------
;
;               _dkioset( address )
;
;               Initialize interrupt driven disktte I/O for the
;               diskette adapter at the given status port address
;               returns 1 if successful, 0 if not.
;               ---------------------------------------------------
;
;               _dkiosts()
;
;               Returns status of I/O requests.
;                       0 - no request pending
;                       1 - request pending
;               ---------------------------------------------------
;
;               _dkio( function, drive, head, track, sector,
;                      nsectors, buffoff, buffseg )
;
;               performs an int 13 with the following parameter to
;               register transfers:
;                       AH - function
;                       DL - drive
;                       DH - head
;                       CH - track number
;                       CL - sector number
;                       AL - number of sectors
;                       BX - buffer offset
;                       ES - buffer segment
;               returns AX from int 13.
;               ------------------------------------------------------
;
;               _dktick( tickcount )
;
;               wait for specified number of ticks before resuming
;               process.  Returns tick count.
;               ------------------------------------------------------
;
;               int 13
;
;               _dkioset overrides the diskette I/O software interrupt
;               vector to jump to _dksftint.  _dksftint does:
;
;                     if executing from a diskette process
;                            . duplicate BIOS int 13 using interrupt
;                              driven IO
;                     else
;                            . jump to BIOS int 13 routine
;               -----------------------------------------------------


SEEKSTS equ     ES:[DI+0]
MOTRSTS equ     ES:[DI+1]
MOTRCNT equ     ES:[DI+2]
DKTESTS equ     ES:[DI+3]
NECSTS0 equ     ES:[DI+4]
NECSTS1 equ     ES:[DI+5]



%       .MODEL  model,lang
        INCLUDE mixed.inc

        .CODE


        include segments.h

        public  _dkiosts
        public  _dkioset
        public  _dkproc
        public  _dkio
        public  _dktick
        public  _dkhrdint
        public  _dksftint
        extrn   _intsetup:near
        extrn   _intreset:near

;-----------------------------------------------------------------------------

dkiosts         db      0               ; interrupt io status
DKIDLE          equ     0               ; no I/O pending
DKWAIT          equ     1               ; waiting for hardware interrupt
DKINPROC        equ     2               ; software interrrupt in process
DKTICK          equ     3               ; waiting for tick interrupt

userss          dw      ?               ; user SS and SP
usersp          dw      ?               ;
procss          dw      ?               ; process SS and SP
procsp          dw      ?

tickcount       dw      ?               ; wait request tick count

diskaddr        dw      ?               ; diskette status port address
realsk          dd      0040003EH       ; segment/offset of real seek status
dktable         db      12 dup (0)      ; replacement low memory tables

;
;       table of DMA setup modes and NEC commands for
;       read/write/verify
;

cmdtab  db      46H                 ;                       EF51 46
        db      66H                 ;                       EF52 66
        db      4AH                 ;                       EF53 4A
        db      45H                 ;                       EF54 45
        db      42H                 ;                       EF55 42
        db      66H                 ;                       EF56 66

;
;       Diskette parameters used to define 32 byte sector I/O
;

diskprm db      0DFH
        db      02
        db      25H
        db      00H                     ; take data length as sector length
        db      09H
        db      2AH
        db      20H                    ; data length is 32
        db      50H
        db      0F6H
        db      00H
        db      02H


;-----------------------------------------------------------------------------

_dkiosts:
        mov     AL,CS:dkiosts
        xor     AH,AH
        ret

;------------------------------------------------------------------------------

_dkioset:
        push    bp                      ; save frame pointer
        mov     bp,sp                   ; new frame pointer
        mov     ax,4[bp]                ; get diskette status port address
        mov     CS:diskaddr,ax          ;

        push    bx                      ; save registers
        push    cx
        push    dx
        push    si
        push    di

        cli                             ; prevent interrupts

        push    CS                      ; setup hardware int override
        push    CS
        lea     AX,_dkhrdint
        push    AX
        mov     AX,0EH
        push    AX
        call    _intsetup
        add     SP,8

        cmp     AX,0                    ; return successful
        jge     s1                      ;    YES- continue
        xor     AX,AX                   ;    NO - clear AX, error
        jmp     setret                  ;         return

s1:
        push    CS                      ; setup software int override
        push    CS
        lea     AX,_dksftint
        push    AX
        mov     AX,13H
        push    AX
        call    _intsetup
        add     SP,8

        cmp     AX,0                    ; return successful
        jge     s2                      ;     YES- continue
        xor     AX,AX                   ;          clear AX, error
        jmp     setret                  ;          return
s2:
        push    CS                      ; setup clock int override
        push    CS
        lea     AX,_dktckint
        push    AX
        mov     AX,1CH
        push    AX
        call    _intsetup
        add     SP,8

        cmp     AX,0                    ; return successful
        jge     s3                      ;     YES- continue
        xor     AX,AX                   ;          clear AX, error
        jmp     setret                  ;          return
s3:
        mov     AX,1                    ; successful return status in AX

setret:
        sti                             ; allow interrupts
        pop     di                      ; restore registers
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     bp
        ret

;------------------------------------------------------------------------------

_dkproc:
        pop     AX                  ; return address in AX
        pushf                       ; setup stack for iret
        push    CS
        push    AX

        call    _dkpushuser         ; save registers

        mov     userss,SS           ; save SS and SP
        mov     usersp,SP

        mov     AX,[BP+28]          ; process address
        mov     SP,[BP+30]          ; process stack pointer

        mov     byte ptr CS:dkiosts,DKINPROC ; in process flag
        call    AX                           ; execute process
        mov     byte ptr CS:dkiosts,DKIDLE   ; process complete flag

        cli                         ; prevent interrupts
        mov     SS,userss           ; restore user SS and SP
        mov     SP,usersp
        call    _dkpopuser          ; restore registers
        iret                        ; return to user routine.


;------------------------------------------------------------------------------

_dkio:
        push    BP                  ; save frame pointer
        mov     BP,SP               ; get new frame pointer
        push    BX
        push    CX
        push    DX
        push    ES

        mov     AX,[BP+18]          ; load registers for int
        mov     ES,AX
        mov     BX,[BP+16]
        mov     AL,[BP+14]
        mov     CL,[BP+12]
        mov     CH,[BP+10]
        mov     DH,[BP+08]
        mov     DL,[BP+06]
        mov     AH,[BP+04]

        int     13H


        pop     ES                  ; restore registers
        pop     DX
        pop     CX
        pop     BX
        pop     BP
        ret                         ; return

;------------------------------------------------------------------------------

_dktick:                            ; save service routine registers and user
                                    ; stack for use by the service routine
                                    ; on receiving a request to wait

        push    BP                  ; save frame pointer
        mov     BP,SP               ; get new frame pointer
        mov     AX,[BP+04]          ; save tick count

        cmp     AX,0                ; if zero, leave
        jnz     dkt1
        pop     BP
        ret

dkt1:   cli                         ; prevent interrupts
        mov     CS:tickcount,AX
        mov     byte ptr CS:dkiosts,DKTICK
        pop     BP                  ; restore frame pointer

        call    _dkpushuser         ; save process registers

        mov     procss,SS           ; save process SS and SP
        mov     procsp,SP

        mov     SS,userss           ; restore user SS and SP
        mov     SP,usersp
        sti

        call    _dkpopuser          ; pop user registers

        iret

;------------------------------------------------------------------------------

_dkpushuser:                        ; save user registers and record
                                    ; stack pointer for return

        pushf
        push    BP                  ;                       EC5A 55
        push    DS                  ;                       EC5B 1E
        push    ES                  ;                       EC5C 06
        push    DI                  ;                       EC5D 57
        push    SI                  ;                       EC5E 56
        push    DX                  ;                       EC5F 52
        push    CX                  ;                       EC60 51
        push    BX                  ;                       EC61 53
        push    AX                  ;                       EC62 50
        mov     BP,SP               ;                       EC63 8BEC
        jmp     word ptr [BP+20]    ; return to caller

;------------------------------------------------------------------------------

_dkpopuser:                         ; restore user registers from the stack
                                    ; after adjusting SP to its original value

        pop     BX                  ; return address
        mov     BP,SP               ;
        mov     [BP+20],BX          ; return address for ret

        pop     AX                  ; restore registers
        pop     BX
        pop     CX
        pop     DX
        pop     SI
        pop     DI
        pop     ES
        pop     DS
        pop     BP
        popf

        ret

;------------------------------------------------------------------------------

_dkwait:                            ; save service routine registers and user
                                    ; stack for use by the service routine
                                    ; on receiving a hardware interrupt.

        mov     byte ptr CS:dkiosts,DKWAIT

        call    _dkpushuser         ; save process registers

        cli                         ; prevent interrupts
        mov     procss,SS           ; save process SS and SP
        mov     procsp,SP

        mov     SS,userss           ; restore user SS and SP
        mov     SP,usersp

        call    _dkpopuser          ; pop user registers

        iret

;------------------------------------------------------------------------------

_dkresume:                          ; resume execution of the software
                                    ; interrupt by restoring the stack,
                                    ; popping off registers, and returning
                                    ; to the caller of _dkwait

        mov     byte ptr CS:dkiosts,DKINPROC

        call    _dkpushuser         ; save user registers

        cli                         ; prevent interrupts
        mov     userss,SS           ; save user SS and SP
        mov     usersp,SP

        mov     SS,procss           ; restore process SS and SP
        mov     SP,procsp

        call    _dkpopuser          ; pop process registers

        sti                         ; allow interrupts
        ret

;------------------------------------------------------------------------------

_dkhrdint:
        push    AX                  ; save registers        EF57 50
        push    ES                  ;                       EF58 1E
        push    DI                  ;
        les     DI,CS:realsk        ; DS set to BIOS data   EF59 B84000
                                    ;    segment.           EF5C 8ED8
        or      byte ptr SEEKSTS,80H; Set interrupt flag.   EF5E 800E3E0080
        mov     AL,20H              ; Acknowledge interrupt EF63 B020
        out     20H,AL              ;                       EF65 E620
        pop     DI                  ; Restore registers     EF67 1F
        pop     ES                  ;
        pop     AX                  ;                       EF68 58
        add     SP,8                ; clean up stack

        cmp     byte ptr CS:dkiosts,DKWAIT ; waiting for int?
        je      _dkresume                  ;    YES - resume
        iret                               ;    NO  - return  EF69 CF

;------------------------------------------------------------------------------

_dktckint:
        add     SP,2                       ; clean up stack
        cmp     byte ptr CS:dkiosts,DKTICK ; waiting for tick?
        jne     tickdk                     ;    NO  - try hardware int
        dec     word ptr CS:tickcount      ;    YES - decrement wait count
        jg      tickcon                    ;          wait over? NO -skip
        jmp     tckres                     ;                     YES-resume
tickdk:
        cmp     byte ptr CS:dkiosts,DKWAIT ; waiting for int?
        jne     tickcon                    ;    NO  - skip

tckres: pushf                              ;    YES - setup stack for iret
        push    CS                         ;            from resume
        call    _dkresume                  ;          resume diskette exec.

tickcon:
        iret                               ; iret to old tick routine

;------------------------------------------------------------------------------

_dksftint:

        cmp     byte ptr CS:dkiosts,DKWAIT  ; software interrupt in process
        je      _dksftint                   ;    YES- busy wait until complete

        cmp     byte ptr CS:dkiosts,DKINPROC; interrupt from diskette process
        jz      si2                         ;    YES- branch to new code
        add     SP,2                        ;    NO-  cleanup stack for iret
        iret                                ;         iret to rom int routine

si2:    add     SP,8                ; cleanup stack
        sti                         ; interrupts back on    EC59 FB
        call    _dkpushuser         ; save registers

        xor     AX,AX               ; get diskette parm     EC65 33C0
        mov     DS,AX               ;   address into        EC67 8ED8
        mov     BX,0078H            ;   DS - segment        EC69 BB7800
        lds     SI,dword ptr [BX]   ;   SI - offset         EC6C C537

        push    CS                  ; override to point to disk
        pop     DS                  ; parms in this driver
        mov     SI,offset diskprm   ;

        les     DI,CS:realsk        ; set extra segment to  EC6E B84000
                                    ;   BIOS data segment   EC71 8EC0
                                    ; diskette port address
        cli                         ; disallow interrupts   EC73 FA
        mov     AL,[SI+02]          ; motor wait parm in AH EC74 8A4402
        mov     AL,0FFH             ;  (override to long count)
                                    ; set timer count for   EC7E 26
        mov     MOTRCNT,AL          ;   motor               EC7F A24000
        sti                         ; allow interrupts      EC82 FB

        cmp     CS:diskaddr,3F2H    ; floppy board?
        je      si3                 ;   YES - skip
        mov     DX,3F2H             ;   NO  - disable floppy board
        mov     AL,04H              ;
        out     DX,AL               ;
        xor     AL,AL               ;         tell BIOS its all off
        mov     MOTRSTS,AL          ;
        mov     DX,CS:diskaddr      ;         enable network board
        mov     AL,0CH              ;
        out     DX,AL               ;
        push    CS                  ;         point ES:DI to internal table
        pop     ES                  ;
        lea     DI,CS:dktable       ;

si3:    xor     AH,AH               ; clear AH              EC83 32E4
        mov     AL,[BP+01]          ; function in AX        EC85 8A4601
        cmp     AX,0006             ; function < 6 ?        EC88 3D0600
        jl      bg2                 ;    YES- continue      EC8B 7C07
        mov     AH,01               ;    NO - error flag    EC8D B401
        xor     AL,AL               ;            in AX(AH)  EC8F 32C0
        jmp     return              ;         return        EC91 E95701

bg2:    test    AL,02               ; read or write?        EC94 A802
        jz      bg3                 ;    NO - continue      EC96 7402
        jmp     rdwrvr              ;    YES- jump          EC98 EB23
bg3:    test    AL,04               ; verify or format?     EC9A A804
        jz      bg4                 ;    NO - continue      EC9C 7403
        jmp     vrfm                ;    YES- jump          EC9E E90701
bg4:    test    AL,01               ; read status?          ECA1 A801
        jz      bg5                 ;    NO - continue      ECA3 7402
        jmp     stat                ;    YES- jump          ECA5 EB0D

;
;       reset diskette
;
bg5:
        call    reset               ; reset diskette code   ECA7 E82402
                                    ; update diskette       ECAA 26
        mov     DKTESTS,AH          ;   status              ECAB 88264100
        xor     AL,AL               ; status in AX(AH)      ECAF 32C0
        jmp     return              ; return                ECB1 E93701

;
;       read status
;

stat:
                                    ; get diskette status   ECB4 26
        mov     AL,DKTESTS          ;    into AL            ECB5 A04100
        xor     AH,AH               ; clear AH no error     ECB8 32E4
        jmp     return              ; return                ECBA E92E01

;
;       read/write/verify
;

rdwrvr:
        call    setupdma            ; setup DMA             ECBD E85D02

        xor     AX,AX               ; clear AX              ECC0 33C0
        mov     AL,[BP+00]          ; num of sectors in AX  ECC2 8A4600
        mov     CL,[SI+03]          ; bytes/sector indicat. ECC5 8A4C03
        mov     DX,0080H            ;                       ECC8 BA8000
        shl     DX,CL               ; bytes/sector in DX    ECCB D3E2

        mov     DL,[SI+06]          ; use bytes in parm override
        xor     DH,DH

        mul     DX                  ; bytes in DX:AX DX==0  ECCD F7E2
        sub     AX,0001H            ;       less 1          ECCF 2D0100
        out     05,AL               ; output low byte       ECD2 E605
        xchg    AH,AL               ;                       ECD4 86C4
        out     05,AL               ; output high byte      ECD6 E605
        xchg    AH,AL               ;                       ECD8 86C4

        add     AX,BX               ; over 64k boundary?    ECDA 03C3
        jnb     rwv1                ;    NO - skip          ECDC 730D

        mov     AL,[BP+01]          ;    YES-function in AL ECDE 8A4601
        test    AH,04H              ;        verify?        ECE1 F6C404
        jnz     rwv1                ;          YES-skip     ECE4 7505
        mov     AH,09H              ;          NO -return   ECE6 B409
        jmp     rwve                ;              bnd err. ECE8 E98400

rwv1:   call    seek                ; perform seek          ECEB E85F01
        jz      rwv2                ; OK?  YES-continue     ECEE 7403
        jmp     rwve                ;      NO -return err.  ECF0 EB7D

        nop                         ;                       ECF2 90

rwv2:   xor     AH,AH               ;                       ECF3 32E4
        mov     AL,[BP+01]          ; function in AX        ECF5 8A4601
        shl     AL,1                ; multiply by 2         ECF8 D0E0
        mov     BX,offset cmdtab-4  ; address table in BX   ECFA BB4DEF
        add     BX,AX               ; add func. offset to BXECFD 03D8
                                    ;                       ECFF 2E
        mov     AX,CS:[BX]          ; get function command  ED00 8B07
        out     0BH,AL              ; DMA ??????            ED02 E60B
        mov     AL,02H              ; mode for DMA          ED04 B002
        out     0AH,AL              ; init. diskette channelED06 E60A

        mov     BH,AH               ; output function       ED08 8AFC
        call    necout              ;   command to NEC.     ED0A E80701
        call    outdht              ; output drive/head/trk ED0D E82D02
        mov     BH,[BP+07]          ; output head number    ED10 8A7E07
        call    necout              ;   to NEC              ED13 E8FE00
        mov     BH,[BP+04]          ; output sector number  ED16 8A7E04
        call    necout              ;   to NEC              ED19 E8F800
        add     SI,+03              ; bytes/sector flag add.ED1C 83C603
        call    nsp1                ; output byte/sector    ED1F E8E600
        inc     SI                  ;   flag and track      ED22 46
        inc     SI                  ;   size to NEC         ED23 46

        push    ES                  ; save internal table
        push    DI                  ;   location
        les     DI,CS:realsk        ; get BIOS table
        and     byte ptr SEEKSTS,7FH; turn off inter. flag
        pop     DI                  ; restore internal table
        pop     ES                  ;

        call    nsp1                ; output gap length     ED24 E8E100
                                    ;   and DTL to NEC
        sub     SI,+05              ; restore SI            ED27 83EE05

rwvf1:  call    waitwrun            ; wait while motor on   ED2A E87D01
        jnz     rwve                ; timeout - return err. ED2D 7540

        push    DI                  ; save DI
        mov     CX,0007H            ; 7 status bytes to get ED2F B90700
        add     DI,4                ; address of 1'st byte  ED32 BF4200
        cld                         ; autoincrement DI      ED35 FC

rwv3:   call    neccheck            ; is NEC ready?         ED36 E87202
        jz      rwv3                ;    NO - loop back     ED39 74FB
        mul     DL                  ; waste time            ED3B F6E2
        in      AL,DX               ; read status byte      ED3D EC
        stosb                       ; store status byte     ED3E AA
        loop    rwv3                ; get next byte         ED3F E2F5
        pop     DI                  ; restore DI

                                    ;                       ED41 26
        test    byte ptr NECSTS0,0C0H; abnormal terminate?  ED42 F6064200C0
        jz      rwvok               ;   NO - return success ED47 7424

                                    ;                       ED49 26
        test    byte ptr NECSTS0,08H; equipment check?      ED4A F606420008
        jz      rwv4                ;   NO - continue       ED4F 7404
        mov     AH,80H              ;   YES- return time-   ED51 B480
        jmp     rwve                ;        out error      ED53 EB1A

rwv4:                               ;                       ED55 26
        mov     AH,NECSTS1          ; status reg1 in AH     ED56 8A264300
        test    AH,30H              ; bad CRC or bad DMA    ED5A F6C430
        jz      rwv5                ;   NO - continue       ED5D 7404
        shr     AH,1                ;   YES- return corres- ED5F D0EC
        jmp     rwve                ;        ponding error  ED61 EB0C

rwv5:   test    AH,03               ; bad addr mark or      ED63 F6C403
                                    ;    write protect?
        jz      rwv6                ;    NO-  return AH     ED66 7403
        add     AH,01               ;    YES- return corres-ED68 80C401
rwv6:   jmp     rwve                ;         ponding error ED6B EB02

rwvok:  mov     AH,00               ; no error flag         ED6D B400

rwve:                               ;                       ED6F 26
        mov     DKTESTS,AH          ; update diskette sts.  ED70 88264100
        mov     AL,06               ; ???? mode for DMA     ED74 B006
        out     0AH,AL              ; mode set for DMA      ED76 E60A
        xor     AL,AL               ;                       ED78 32C0
        test    AH,54H              ;       ????            ED7A F6C454
        jmp     rwvfe               ; return                ED7D EB27

        xchg    CX,AX               ;                       ED7F 91
        out     0CH,AL              ;                       ED80 E60C
        in      AL,05               ;                       ED82 E405
        mov     BL,AL               ;                       ED84 8AD8
        in      AL,05               ;                       ED86 E405
        mov     BH,AL               ;                       ED88 8AF8
        mov     CL,[SI+03]          ;                       ED8A 8A4C03
        mov     AX,0080H            ;                       ED8D B88000
        shl     AX,CL               ;                       ED90 D3E0
        dec     AX                  ;                       ED92 48
        xchg    BX,AX               ;                       ED93 93
        xor     DX,DX               ;                       ED94 33D2
        div     BX                  ;                       ED96 F7F3
        or      DX,DX               ;                       ED98 0BD2
        jz      l1                  ;                       ED9A 7401
        inc     AX                  ;                       ED9C 40
l1:     mov     AH,[BP+00]          ;                       ED9D 8A6600
        sub     AH,AL               ;                       EDA0 2AE0
        mov     AL,AH               ;                       EDA2 8AC4
        mov     AH,CH               ;                       EDA4 8AE5

rwvfe:  jmp     return              ; return                EDA6 EB43

;
;       verify/format logic
;

vrfm:
        test    AL,01               ; format command?       EDA8 A801
        jnz     format              ;   YES-continue        EDAA 7503
        jmp     rdwrvr              ;   NO -goto r/w logic  EDAC E90EFF

;
;       format logic
;

format:
        mov     AL,4AH              ; write to diskette     EDAF B04A
        out     0BH,AL              ;   mode                EDB1 E60B
        call    setupdma            ; setup DMA             EDB3 E86701
        mov     AL,0FFH             ; output dummy byte     EDB6 B0FF
        out     05,AL               ;   count               EDB8 E605
        out     05,AL               ;                       EDBA E605

        call    seek                ; perform seek          EDBC E88E00
        jz      fm1                 ; successful?           EDBF 7402
        jmp     rwve                ;    NO - return error  EDC1 EBAC

fm1:    mov     AL,02               ; mode for DMA          EDC3 B002
        out     0AH,AL              ; init diskette channel EDC5 E60A

        mov     BH,4DH              ; output format command EDC7 B74D
        call    necout              ;    to NEC             EDC9 E84800
        mov     BH,[BP+07]          ; head in BH            EDCC 8A7E07
        shl     BH,1                ; shift BH by 2         EDCF D0E7
        shl     BH,1                ;                       EDD1 D0E7
        or      BH,[BP+06]          ; or drive into BH      EDD3 0A7E06
        call    necout              ; output head/drive     EDD6 E83B00
        add     SI,+03              ; output bytes/sector   EDD9 83C603
        call    nsp1                ;    and sectors/track  EDDC E82900
        add     SI,+04              ; output gap length     EDDF 83C604
        call    nsp1                ;    and fill byte      EDE2 E82300
        sub     SI,+07              ; reset SI              EDE5 83EE07
        jmp     rwvf1               ; jump to r/w/v code    EDE8 E93FFF

return:
        mov     [BP],AX             ; save return status

        cmp     CS:diskaddr,3F2H    ; floppy board?
        je      ret1                ;   YES - skip
        mov     AL,04H              ;   NO  - disable network board
        mov     DX,CS:diskaddr      ;
        out     DX,AL               ;
        mov     DX,3F2H             ;         enable floppy board
        mov     AL,0CH              ;
        out     DX,AL               ;

ret1:   call    _dkpopuser          ; restore registers
        iret                        ;                       EE02 CF

;
;       Output NEC specify command along with two specify
;       bytes found in diskette parms.   Nsp1 is an entry
;       point to output two bytes pointed to by SI
;

necspec:
        mov     BH,03               ; NEC specify command   EE03 B703
        call    necout              ; output command        EE05 E80C00
nsp1:   call    neccheck            ; NEC direction bit OK? EE08 E8A001
        jnz     nsp1                ;   NO - loop           EE0B 75FB
        mul     DL                  ;   YES- waste time     EE0D F6E2
        mov     BX,[SI]             ;        output 1st     EE0F 8B1C
        mov     AL,BL               ;          specify byte EE11 8AC3
        out     DX,AL               ;          to NEC.      EE13 EE
                                    ;        fall through to
                                    ;        output 2nd byte
;
;       output command to NEC controller after waiting
;       on ready status and direction bit.  Command to
;       be output in BH
;

necout:
        call    neccheck            ; NEC direction bit OK? EE14 E89401
        jnz     necout              ;    NO - loop          EE17 75FB
        mul     DL                  ;    YES- waste time    EE19 F6E2
        mov     AL,BH               ;         output cmd    EE1B 8AC7
        out     DX,AL               ;                       EE1D EE
        ret                         ;         return        EE1E C3

;
;       recalibrate drive
;

recal:
        call    startmot            ; start motor           EE1F E84801
        mov     CX,0064H            ; NEC check count       EE22 B96400
rcb1:   call    neccheck            ; NEC controller ready  EE25 E88301
        jz      rcb2                ;    YES- continue      EE28 7407
        dec     CX                  ;    NO - decrement     EE2A 49
        jnz     rcb1                ;         loop back     EE2B 75F8
        mov     AH,80H              ;         time out      EE2D B480
        jmp     rcbr                ;         return        EE2F EB19

rcb2:   mul     DL                  ; waste time            EE31 F6E2
        mov     AL,07               ; recalibrate command   EE33 B007
        out     DX,AL               ;                       EE35 EE
        mov     BH,[BP+06]          ; drive number in BH    EE36 8A7E06
        call    necout              ; output drive number   EE39 E8D8FF
        call    waitwrun            ; wait while motor on   EE3C E86B00
        jnz     rcbr                ; time out error        EE3F 7509

        call    necsensi            ; NEC sense int. sts.   EE41 E84800
        test    AL,0C0H             ; test for error        EE44 A8C0
        mov     AH,080H             ; time out if error     EE46 B480
        jmp     rcbr2               ; return                EE48 EB02

rcbr:   or      AH,AH               ; test for error        EE4A 0AE4
rcbr2:  ret                         ; return                EE4C C3


;
;       perform diskette seek
;

seek:
        mov     AL,01               ;                       EE4D B001
        mov     CL,[BP+06]          ; drive number          EE4F 8A4E06
        shl     AL,CL               ; drive mask            EE52 D2E0
                                    ;                       EE54 26
        test    SEEKSTS,AL          ; drive needs recal?    EE55 84063E00
        jnz     sk1                 ;   NO - skip           EE59 750A
                                    ;                       EE5B 26
        or      SEEKSTS,AL          ;   YES- set flag       EE5C 08063E00
        call    recal               ;        recalibrate    EE60 E8BCFF
        jnz     skr                 ;        successful?    EE63 7526
                                    ;           NO - return

sk1:    call    startmot            ; start motor           EE65 E80201
        mov     BH,0FH              ; seek command in BX    EE68 B70F
        call    necout              ; output seek command   EE6A E8A7FF
        call    outdht              ; output drive/head/trk EE6D E8CD00
        call    waitwrun            ; wait while motor on   EE70 E83700
        jnz     skr                 ; timeout? YES-return   EE73 7516

        xor     AX,AX               ;                       EE75 33C0
        or      AL,[SI+09]          ; head settle time AX   EE77 0A4409
        jz      sk4                 ; if zero skip          EE7A 7408

                                    ; head settle wait
sk2:    mov     CX,0104H            ; inner loop count      EE7C B90401
sk3:    loop    sk3                 ; inner loop            EE7F E2FE
        dec     AX                  ; dec head settle count EE81 48
        jnz     sk2                 ; loop back             EE82 75F8

sk4:    call    necsensi            ; NEC sense int. sts.   EE84 E80500
        test    AL,0C0H             ; test for error        EE87 A8C0
        mov     AH,040H             ; bad seek if error     EE89 B440
skr:    ret                         ; return                EE8B C3

;
;       Generate a sense interupt status command to the NEC
;       and read the two corresponding status and cylinder
;       number reponses.
;

necsensi:
        mov     BH,08               ; sense int. stat. cmd  EE8C B708
        call    necout              ; output to NEC         EE8E E883FF

        mov     CL,02               ; do it twice           EE91 B102
ns1:    call    neccheck            ; NEC direction bit OK? EE93 E81501
        jz      ns1                 ;    NO - loop          EE96 74FB
        mul     DL                  ; waste time            EE98 F6E2
        in      AL,DX               ; get status            EE9A EC
        dec     CL                  ; second time around?   EE9B FEC9
        jz      ns2                 ;                       EE9D 7406
                                    ;    NO - save status   EE9F 26
        mov     NECSTS0,AL          ;         byte          EEA0 A24200
        jmp     ns1                 ;         get again.    EEA3 EBEE

ns2:                                ;    YES- return first  EEA5 26
        mov     AL,NECSTS0          ;         status in AH  EEA6 A04200
        ret                         ;                       EEA9 C3

;
;       wait while motor running
;

waitwrun:
        cli                         ; prevent interrupts    EEAA 26
        push    ES                  ; save internal table
        push    DI                  ;   location
        les     DI,CS:realsk        ; get BIOS table
        test    byte ptr SEEKSTS,80H; interrupt flag on?    EEAB F6063E0080
        jnz     wtr1                ;  YES- continue        EEB0 750A
                                    ;                       EEB2 26
        pop     DI                  ;  NO - restore internal table
        pop     ES                  ;
        test    byte ptr MOTRSTS,0FH;       a drive running?EEB3 F6063F000F
        jz      wtr2                ;      NO - timeout     EEB8 740C
        call    _dkwait             ;      YES- new int code
        jmp     waitwrun            ;           loop back   EEBA EBEE

wtr1:   sti                         ; allow interrupts      EEBC 26
        and     byte ptr SEEKSTS,7FH; turn off inter. flag  EEBD 80263E007F
        pop     DI                  ; restore internal table
        pop     ES                  ;
        mov     AH,00               ; flag no error         EEC2 B400
        jmp     wtr3                ; return                EEC4 EB05

wtr2:   sti                         ; allow interrupts
        call    reset               ; reset diskette        EEC6 E80500
        mov     AH,80H              ; time out error        EEC9 B480

wtr3:   or      AH,AH               ; set error flag        EECB 0AE4
        ret                         ; return                EECD C3

;
;       reset the diskette system
;

reset:
        mov     DX,CS:diskaddr      ; adapter control port  EECE BAF203
                                    ;                       EED1 26
        mov     AL,MOTRSTS          ; reset motors          EED2 A03F00
        and     AL,0FH              ;     .                 EED5 240F
        jz      rst2                ;     .                 EED7 7413
        shr     AL,1                ;     .                 EED9 D0E8
        test    AL,04               ;     .                 EEDB A804
        jz      rst1                ;     .                 EEDD 7402
        mov     AL,03               ;     .                 EEDF B003
rst1:                               ;     .                 EEE1 26
        mov     AH,MOTRSTS          ;     .                 EEE2 8A263F00
        mov     CL,04               ;     .                 EEE6 B104
        shl     AH,CL               ;     .                 EEE8 D2E4
        or      AL,AH               ;     .                 EEEA 0AC4
rst2:   or      AL,08               ; turn on int. enable   EEEC 0C08
        out     DX,AL               ; reset the adapter     EEEE EE

        push    AX                  ;  waste time           EEEF 50
        mul     DL                  ;                       EEF0 F6E2
        pop     AX                  ;                       EEF2 58

                                    ; flag all drives as    EEF3 26
        and     byte ptr SEEKSTS,00 ;   recal needed        EEF4 80263E0000
        push    ES                  ; save internal table
        push    DI                  ;   location
        les     DI,CS:realsk        ; get BIOS table
        and     byte ptr SEEKSTS,7FH; turn off interrupt flag

        or      AL,04               ; turn off reset        EEF9 0C04
        out     DX,AL               ; turn off reset        EEFB EE

        mov     CX,0FFFFH           ; wait routine          EEFC B9FFFF

rst3:   cli                         ; prevent interrupts    EEFF 26
        test    byte ptr SEEKSTS,80H; interrupt flag on?    EF00 F6063E0080
        jnz     rst4                ;   YES - leave         EF05 7507
        dec     CX                  ;   NO  - decrement     EF07 49
        call    _dkwait             ;         new int code
        jnz     rst3                ;         loop back     EF08 75F5

        pop     DI                  ; restore internal table
        pop     ES                  ;
        mov     AH,80H              ; flag time out error   EF0A B480
        jmp     rst5                ; return                EF0C EB0E

rst4:   sti                         ; restore interrupts    EF0E 26
        and     byte ptr SEEKSTS,7FH; reset interrupt flag  EF0F 80263E007F
        pop     DI                  ; restore internal table
        pop     ES                  ;
        call    necsensi            ; NEC sense int. sts.   EF14 E875FF
        call    necspec             ; NEC specify command   EF17 E8E9FE
        xor     AH,AH               ; flag no error         EF1A 32E4
rst5:   ret                         ; return                EF1C C3

;
;       setup DMA for read/write
;

setupdma:
        mov     DX,0010H            ; segment increment     EF1D BA1000
        mov     AX,[BP+0CH]         ; segment of buffer     EF20 8B460C
        mul     DX                  ; absolute in DX:AX     EF23 F7E2
        mov     BX,[BP+02]          ; offset of buffer      EF25 8B5E02
        add     AX,BX               ; address in DX:AX      EF28 03C3
        adc     DL,DH               ;   (add carry) DH==0   EF2A 12D6
        out     0CH,AL              ; first/last flag ???   EF2C E60C
        out     04,AL               ; output low address    EF2E E604
        xchg    AH,AL               ;                       EF30 86C4
        out     04,AL               ; output high address   EF32 E604
        xchg    AH,AL               ;                       EF34 86C4
        mov     BX,AX               ; low half of 20-bits   EF36 8BD8
        mov     AL,DL               ;                       EF38 8AC2
        out     81H,AL              ; output high 4 bits    EF3A E681
        ret                         ; return                EF3C C3

;
;       output drive/head/track
;

outdht:
        mov     BH,[BP+07]          ; head number in BH     EF3D 8A7E07
        shl     BH,1                ; shift left two        EF40 D0E7
        shl     BH,1                ;                       EF42 D0E7
        or      BH,[BP+06]          ; or drive number in BH EF44 0A7E06
        call    necout              ; output head/drive     EF47 E8CAFE
        mov     BH,[BP+05]          ; track number in BH    EF4A 8A7E05
        call    necout              ; output track number   EF4D E8C4FE
        ret                         ; return                EF50 C3

;
;       start up motor and select drive.  Perform a
;       startup wait if operation is a write.
;

startmot:
        cli                         ; dis-allow interrupts  EF6A FA
        mov     AL,[SI+02]          ; motor wait count      EF6B 8A4402
                                    ;                       EF6E 26
        mov     MOTRCNT,AL          ; update motor wait cnt EF6F A24000
        mov     AL,10H              ;                       EF72 B010
        mov     CL,[BP+06]          ; drive number in CL    EF74 8A4E06
        shl     AL,CL               ; drive mask in AL      EF77 D2E0
        or      AL,0CH              ; no reset, enb dma int EF79 0C0C
        or      AL,[BP+06]          ; drive select bits on  EF7B 0A4606
        mov     DX,CS:diskaddr      ; control port address  EF7E BAF203
        out     DX,AL               ;                       EF81 EE

        mov     CL,04               ; rotate drive map to   EF82 B104
        shr     AL,CL               ;  low order nib of AL  EF84 D2E8
                                    ; drive already flagged EF86 26
        test    MOTRSTS,AL          ;     as running?       EF87 84063F00
        jnz     lr                  ;     Yes - return      EF8B 751C

                                    ; flag drive as running EF8D 26
        mov     MOTRSTS,AL          ;                       EF8E A23F00
        mov     AL,01               ; function in AL        EF91 B001
        test    [BP+01],AL          ; is function a write?  EF93 844601
        jz      lr                  ;     NO - return       EF96 7411

        sti                         ; allow interrupts      EF98 FB
        mov     AX,007DH            ; wait count in AX      EF99 B87D00
        mov     DL,[SI+0AH]         ; motor start time      EF9C 8A540A
        mul     DL                  ; multiply for total wt EF9F F6E2
        cmp     AX,0000             ; is wait 0?            EFA1 3D0000
        jz      lr                  ;    YES- skip          EFA4 7403
        call    varwait             ;    NO - perform wait  EFA6 E81300
lr:     sti                         ; allow interrupts      EFA9 FB
        ret                         ; return                EFAA C3

;
;       Wait until NEC controller is ready,
;       then test direction bit.
;

neccheck:
        mov     DX,CS:diskaddr      ; status port           EFAB BAF403
        add     DX,2                ;
chn1:   mul     DL                  ; waste time            EFAE F6E2
        in      AL,DX               ; get status            EFB0 EC
        test    AL,80H              ; is controller ready?  EFB1 A880
        jz      chn1                ;    NO - loop          EFB3 74F9

        mul     DL                  ; waste time            EFB5 F6E2
        in      AL,DX               ; get status            EFB7 EC
        inc     DX                  ; increment port addr.  EFB8 42
        test    AL,40H              ; test direction        EFB9 A840
        ret                         ; return                EFBB C3

;
;       variable wait.  perform outer loop AX times
;

varwait:
        mov     CX,0104H            ; inner loop count      EFBC B90401
vw1:    loop    vw1                 ; tight loop            EFBF E2FE
        dec     AX                  ; decrement wait count  EFC1 48
        jnz     varwait             ; loop back             EFC2 75F8
        ret                         ; return                EFC4 C3

        end
