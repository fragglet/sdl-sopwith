AC_INIT([SDL Sopwith], [2.1.0], [fraggle@gmail.com], [sdl-sopwith])
AC_CONFIG_AUX_DIR(autotools)

VERSION=$CONFIG_VERSION

PACKAGE_COPYRIGHT="Copyright (C) 1984-2022"
PACKAGE_LICENSE="GNU General Public License, version 2"

AC_SUBST(VERSION)
AC_SUBST(PACKAGE_COPYRIGHT)
AC_SUBST(PACKAGE_LICENSE)

AC_CANONICAL_TARGET
AC_SEARCH_LIBS([sin], [m])

dnl optimisations

AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE([1.8.0 foreign])

dnl check for cc

AC_PROG_CC
AC_PROG_RANLIB
AC_CHECK_TOOL(STRIP, strip, )
AC_CHECK_TOOL(CONVERT, convert, )

if test "$GCC" = "yes"
then
        WARNINGS="-Wall -Wdeclaration-after-statement -Wredundant-decls"
        CFLAGS="-O$OPT_LEVEL -g $WARNINGS $orig_CFLAGS"
fi

dnl network libraries

case "$target" in
    *-*-cygwin* | *-*-mingw32*)
	LDFLAGS="$LDFLAGS -lwsock32"
	;;
    sparc*-*-solaris*)
	LDFLAGS="$LDFLAGS -lsocket -lnsl"
	;;
    *-*-qnx*)
	LDFLAGS="$LDFLAGS -lsocket"
	;;
    *haiku*)
	LDFLAGS="$LDFLAGS -lnetwork"
	 ;;
esac

case "$host" in
    *cygwin* | *mingw* )
        AC_CHECK_TOOL(WINDRES, windres, )
        ;;
    *)
        WINDRES=
        ;;
esac

AM_CONDITIONAL(HAVE_WINDRES, test "$WINDRES" != "")
AM_CONDITIONAL(HAVE_CONVERT, test "$CONVERT" != "")
AC_CONFIG_HEADERS([config.h:config.hin])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl install programs

AC_PROG_INSTALL

dnl libraries

PKG_CHECK_MODULES(SDL, [sdl2 >= 2.0.7])

dnl check for TCPIP support

AC_CHECK_HEADER(netinet/in.h, [
    AC_DEFINE(TCPIP, 1, [TCPIP multiplayer support])
],,)

AC_SUBST(ac_aux_dir)

WINDOWS_RC_VERSION=$(echo $PACKAGE_VERSION | sed 's/-.*//; s/\./, /g; s/$/, 0/')
AC_SUBST(WINDOWS_RC_VERSION)

dnl generate Makefiles

AC_CONFIG_FILES([
Makefile
doc/Makefile
pkg/config.make
pkg/macos/Info.plist
pkg/Makefile
src/Makefile
src/resource.rc
src/sdl/Makefile
])
AC_OUTPUT

