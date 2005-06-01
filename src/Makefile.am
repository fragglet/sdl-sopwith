bindir=$(prefix)/bin

SUBDIRS = @SOPWITH_SUBDIRS@
EXTRA_SUBDIRS = sdl gtk psion 
DIST_SUBDIRS = $(EXTRA_SUBDIRS)

bin_PROGRAMS = @SOPWITH_BINS@
EXTRA_PROGRAMS = sopwith gtksopwith psopwith

CFLAGS = @CFLAGS@ 

COMMON_SOURCES =       \
video.h	    swcollsn.c  swgames.h   swmain.h    swutil.h 		\
font.h      swcollsn.h  swground.c  swtext.c    tcpcomm.c 		\
pcsound.h   swconf.c    swground.h  swtext.h    swsound.c   tcpcomm.h 	\
std.h       swconf.h    swgrpha.c   swmove.c    swsound.h   timer.h 	\
sw.h        swdisp.c    swgrpha.h   swmove.h    swsymbol.c 		\
swasynio.c  swdisp.h    swinit.c    swsymbol.h 				\
swasynio.h  swend.c     swinit.h    swtitle.c 				\
swauto.c    swend.h     swobject.c  swtitle.h 				\
swauto.h    swgames.c   swmain.c    swobject.h  swutil.c		\
swsplat.c   swsplat.h	swstbar.c   swstbar.h                           \
video.c

# include dos port sources in distribution

DOS_SOURCES = dos/Makefile  dos/pcsound.c  dos/README  dos/timer.c

EXTRA_DIST=$(DOS_SOURCES)

# SDL build

sopwith_SOURCES = $(COMMON_SOURCES) vid_vga.c
sopwith_LDADD = sdl/libsdlsopwith.a @SDL_LIBS@

# Gtk+ build

gtksopwith_SOURCES = $(COMMON_SOURCES) vid_vga.c
gtksopwith_LDADD = gtk/libgtksopwith.a @GTK_LIBS@

# Psion build

psopwith_SOURCES = $(COMMON_SOURCES)
psopwith_LDADD = psion/libpsopwith.a

