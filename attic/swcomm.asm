;
;       swcomm   -      SW communications handler
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
;                       85-04-03        Development
;                       87-01-09        Multiple port support
;                       87-03-10        Microsoft compiler
;                       94-12-18        C6 Compiler
;

%       .MODEL  model,lang
        INCLUDE mixed.inc

        .CODE

        include segments.h
        include sw.ha

        public  commin
        public  commout
        public  comminit
        public  commterm

        extrn   swend:near



;
;       Input character from comm line
;

commin:
        call    dword ptr fastin        ; get character in AL, status in AH
        or      AH,AH                   ; anything read?
        jnz     ci1                     ;
        mov     AX,-1                   ;   NO - return -1
        ret                             ;
ci1:    xor     AH,AH                   ;   YES- return character in AX
        ret                             ;



;
;       Output character to comm line
;

commout:
        push    BP                      ; save frame pointer
        mov     BP,SP                   ; new frame pointer
        mov     AX,4[BP]                ; character in AL
        call    dword ptr fastout       ; output character
        pop     BP                      ; restore BP
        ret                             ; return


;
;       Initialize comm processing
;

comminit:
        mov     DX,offset DGROUP:devname; open NAME device
        mov     AX,3D00H                ;
        int     21H                     ;
        jnc     coop0                   ;
        jmp     coerr1                  ;

coop0:  cmp     word ptr multaddr,03F2H ; port address overriden?
        je      coop1                   ;
        mov     BL,byte ptr multaddr    ;    YES- convert to ascii
        add     BL,30H                  ;
        mov     port,BL                 ;         save as port number

coop1:  mov     BX,AX                   ; read long jump address from NAME
        mov     CX,12                   ;   device for comm port
        mov     AL,port                 ;
        mov     iobuffp,AL              ;
        mov     DX,offset DGROUP:iobuff ;
        mov     AX,4402H                ;
        int     21H                     ;
        jnc     coop2                   ;
        jmp     coerr2                  ;

coop2:  cmp     AX,12                   ; address found?
        je      coop3                   ;
        jmp     coerr3                  ;   NO - error

coop3:  push    ES                      ; save ES
        les     SI,fasttab              ; ES:SI points to fast routine table
        mov     AX,ES:0[SI]             ; load fast I/O jump offsets
        mov     word ptr fastin,AX      ;
        mov     AX,ES:2[SI]             ;
        mov     word ptr fastout,AX     ;
        mov     word ptr fastin+2,ES    ; load fast I/O jump segments
        mov     word ptr fastout+2,ES   ;
        pop     ES                      ; restore ES

        mov     AH,3EH                  ; close NAME device
        int     21H                     ;

        mov     byte ptr fasttab,0      ; terminate serial file name
        mov     DX,offset DGROUP:iobuff ; open SERIAL device
        mov     AX,3D02H                ;
        int     21H                     ;
        jnc     coop4                   ;
        jmp     coerr3                  ;

coop4:  mov     handle,AX               ; save SERIAL handle
        mov     AX,4402H                ; IOCTL read function
        mov     BX,handle               ; SERIAL driver handle in BX
        mov     CX,15                   ; 15 bytes to read/write
        mov     DX,offset DGROUP:savecfg; configuration address in DX
        int     21H                     ; read current configuration
        jnc     coop5                   ;
        jmp     coerr3                  ;

coop5:  mov     byte ptr saveflg,1      ; flag configuration saved
        mov     AX,4403H                ; IOCTL write function
        mov     BX,handle               ; SERIAL driver handle in BX
        mov     CX,15                   ; 15 bytes to read/write
        mov     DX,offset DGROUP:sopcfg ; configuration address in DX
        int     21H                     ; set sopwith configuration
        jnc     coop6                   ;
        jmp     coerr3                  ;

coop6:  ret                             ; return

coerr1: lea     DX,DGROUP:enameop       ; display NAMEDEV open error
        jmp     coerror                 ;   and terminate

coerr2: lea     DX,DGROUP:enamerd       ; display NAMEDEV read error
        jmp     coerror                 ;   and terminate

coerr3: lea     DX,DGROUP:eserial       ; display SERIAL access error
        jmp     coerror                 ;   and terminate

coerror:
        mov     AX,0                    ; flag no update on termination
        push    AX                      ;
        push    DX                      ; save error message to display
        call    swend                   ; leave, to never return


;
;       Terminate comm processing
;

commterm:
        test    byte ptr saveflg,1      ; configuration saved?
        jz      cotm1                   ;   NO - leave

        mov     AX,4403H                ; IOCTL write function
        mov     BX,handle               ; SERIAL driver handle in BX
        mov     CX,15                   ; 15 bytes to read/write
        mov     DX,offset DGROUP:savecfg; configuration address in DX
        int     21H                     ; restore original configuration

        mov     AH,3EH                  ; close SERIAL driver
        int     21H                     ;

cotm1:  ret                             ; return

        .DATA

        extrn   multaddr:word           ; port override

port    db      '1'                     ; communications port
fastin  dd      ?                       ; fast character input address
fastout dd      ?                       ;                output
handle  dw      ?                       ; SERIAL device handle

devname db      'NAME$BMB',0            ; device name for name dictionary
iobuff  db      'SER$BMB'                ; dictionary name for fast I/O
iobuffp db      ?                       ;   ( including port )
fasttab dd      ?                       ; long address of fast I/O table

savecfg db      15 dup (?)              ; save area for comm configuration
sopcfg  label   byte                    ; sopwith comm configuration
        dw      300                     ; 300 baud
        db      'N'                     ; no parity
        db      1                       ; 1 stop bit
        db      8                       ; 8 data bits
        dw      4 dup (?)               ; don't care on buffer sizes
        db      1                       ; no pacing
        db      0                       ; port enabled
saveflg db      0                       ; configuration saved flag

enameop db      'Error opening NAMEDEV dictionary device',0
enamerd db      'Error reading NAMEDEV dictionary device',0
eserial db      'Error accessing SERIAL communications device',0

        end
