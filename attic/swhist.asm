;
;       swhist -      SW history processing  ( dummy )
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
;                       84-07-23        Development
;                       85-11-05        ATARI
;                       87-03-10        Microsoft compiler
;                       94-12-18        C6 Compiler
;
;


%       .MODEL  model,lang
        INCLUDE mixed.inc

        .CODE

        include segments.h
        include sw.ha

        public  histinit
        public  history
        public  histmult
        public  histend


;----------------------------------------------------------------------------


histinit:
        push    BP                      ; save frame pointer
        mov     BP,SP                   ; set new frame pointer
        mov     AX,@AB[BP]              ; default explosion seed on stack
        pop     BP
        ret

;----------------------------------------------------------------------------

history:
        ret




;----------------------------------------------------------------------------

histmult:
        push    BP                      ; save frame pointer
        mov     BP,SP                   ; set new frame pointer
        mov     AX,@AB+2[BP]
        pop     BP
        ret


;----------------------------------------------------------------------------

histend:
        ret                             ; return


;----------------------------------------------------------------------------


        end
