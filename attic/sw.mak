#                       Copyright (C) 1984-2000 David L. Clark.
#
#                       All rights reserved except as specified in the
#                       file license.txt.  Distribution of this file
#                       without the license.txt file accompanying is
#                       prohibited.
#

.asm.obj:
        masm /Dmodel=small /Dlang=c /Mx $*.asm;

.c.obj:
        cl -c -Gs -Zl -FPc -AS -Osa $*.c

_ints.obj:         _ints.asm

swcomm.obj:        swcomm.asm

_inta.obj:         _inta.asm

swgrph.obj:        swgrph.asm

swhist.obj:        swhist.asm

swutil.obj:        swutil.asm

_dkio.obj:         _dkio.asm


bmblib.obj:        bmblib.c

_intc.obj:         _intc.c

swasynio.obj:      swasynio.c

swauto.obj:        swauto.c

swcollsn.obj:      swcollsn.c

swdisp.obj:        swdisp.c

swend.obj:         swend.c

swgames.obj:       swgames.c

swground.obj:      swground.c

swinit.obj:        swinit.c

swmain.obj:        swmain.c

swmisc.obj:        swmisc.c

swmove.obj:        swmove.c

swmultio.obj:      swmultio.c

swobject.obj:      swobject.c

swplanes.obj:      swplanes.c

swsound.obj:       swsound.c

swsymbol.obj:      swsymbol.c

swtitle.obj:       swtitle.c

sopwith.exe:       _ints.obj \
                   swcomm.obj \
                   _inta.obj \
                   swgrph.obj \
                   swutil.obj \
                   _dkio.obj \
                   _intc.obj \
                   swasynio.obj \
                   swmain.obj \
                   swmove.obj \
                   swinit.obj \
                   swauto.obj \
                   swdisp.obj \
                   swcollsn.obj \
                   swplanes.obj \
                   swsymbol.obj \
                   swend.obj \
                   swgames.obj \
                   swground.obj \
                   swhist.obj \
                   swmisc.obj \
                   swmultio.obj \
                   swobject.obj \
                   swsound.obj \
                   swtitle.obj \
                   bmblib.obj \
                   sw.mak \
                   sw.lnk
        link @sw.lnk
