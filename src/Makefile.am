bindir=$(prefix)/bin

SUBDIRS = @SOPWITH_SUBDIRS@
EXTRA_SUBDIRS = sdl gtk psion

bin_PROGRAMS = @SOPWITH_BINS@
EXTRA_PROGRAMS = sopwith gtksopwith psopwith

CFLAGS = @CFLAGS@ -I..

COMMON_SRC =       \
video.h	    swcollsn.c  swgames.h   swmain.h    swutil.h 	\
font.h      swcollsn.h  swground.c  swmisc.c    tcpcomm.c 	\
pcsound.h   swconf.c    swground.h  swmisc.h    swsound.c   tcpcomm.h 	\
std.h       swconf.h    swgrpha.c   swmove.c    swsound.h   timer.h 	\
sw.h        swdisp.c    swgrpha.h   swmove.h    swsymbol.c 		\
swasynio.c  swdisp.h    swinit.c    swsymbol.h 		\
swasynio.h  swend.c     swinit.h    swtitle.c 		\
swauto.c    swend.h     swobject.c  swtitle.h 		\
swauto.h    swgames.c   swmain.c    swobject.h  swutil.c		\
video.c

sopwith_SOURCES = $(COMMON_SRC)
sopwith_LDADD = sdl/libsdlsopwith.a @SDL_LIBS@

gtksopwith_SOURCES = $(COMMON_SRC)
gtksopwith_LDADD = gtk/libgtksopwith.a @GTK_LIBS@

psopwith_SOURCES = $(COMMON_SRC)
psopwith_LDADD = psion/libpsopwith.a

