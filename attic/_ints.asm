;
;       ints     -      Extra DOS Interrupt Control
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
;                       84-03-06        Development
;                       87-03-10        Microsoft compiler
;                       94-12-18        C6 Compiler
;

%       .MODEL  model,lang
        INCLUDE mixed.inc

        .CODE

        include segments.h

        public  intson
        public  intsoff


intson:
        sti
        ret

intsoff:
        cli
        ret

        end
