EXTRA_DIST = sopwith.ico

appdir = $(prefix)/share/applications
app_DATA = sopwith.desktop

iconsdir = $(prefix)/share/icons/hicolor/128x128/apps
icons_DATA = sopwith.png

bindir=$(prefix)/bin

SUBDIRS = sdl
bin_PROGRAMS = sopwith

COMMON_SOURCES =                   \
        font.h                     \
        pcsound.h                  \
        std.h                      \
        swasynio.c   swasynio.h    \
        swauto.c     swauto.h      \
        swcollsn.c   swcollsn.h    \
        swconf.c     swconf.h      \
        swdisp.c     swdisp.h      \
        swend.c      swend.h       \
        swgames.c    swgames.h     \
        swgrpha.c    swgrpha.h     \
        sw.h                       \
        swinit.c     swinit.h      \
        swmain.c     swmain.h      \
        swmove.c     swmove.h      \
        swobject.c   swobject.h    \
        swsound.c    swsound.h     \
        swsplat.c    swsplat.h     \
        swstbar.c    swstbar.h     \
        swsymbol.c   swsymbol.h    \
        swtext.c     swtext.h      \
        swtitle.c    swtitle.h     \
        tcpcomm.c    tcpcomm.h     \
        timer.h                    \
        video.c      video.h       \
        vid_vga.c                  \
        yocton.c     yocton.h

if HAVE_WINDRES
sopwith_SOURCES = $(COMMON_SOURCES) resource.rc
else
sopwith_SOURCES = $(COMMON_SOURCES)
endif

sopwith_LDADD = sdl/libsdlsopwith.a @SDL_LIBS@ sdl/libsdlsopmain.a \
                $(SOCKET_LIBS)

.rc.o:
	$(WINDRES) $< -o $@
%.o : %.rc
	$(WINDRES) $< -o $@

resource.rc: sopwith.ico

if HAVE_CONVERT
sopwith.ico: ../icon.png
	convert $< $@
endif

