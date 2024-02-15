AC_INIT([SDL Sopwith], [2.4.0], [fraggle@gmail.com], [sdl-sopwith])
AC_CONFIG_AUX_DIR(autotools)

VERSION=$CONFIG_VERSION

PACKAGE_COPYRIGHT="Copyright (C) 1984-2024"
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
AC_CHECK_HEADERS(stdbool.h)
AC_CHECK_FUNCS(isatty)

if test "$GCC" = "yes"
then
        WARNINGS="-Wall -Wdeclaration-after-statement -Wredundant-decls"
        CFLAGS="-O$OPT_LEVEL -g $WARNINGS $CFLAGS"
fi

dnl network libraries

SOCKET_LIBS=
case "$target" in
    *-*-cygwin* | *-*-mingw32*)
	SOCKET_LIBS="-lwsock32"
	;;
    sparc*-*-solaris*)
	SOCKET_LIBS="-lsocket -lnsl"
	;;
    *-*-qnx*)
	SOCKET_LIBS="-lsocket"
	;;
    *haiku*)
	SOCKET_LIBS="-lnetwork"
	 ;;
esac

AC_SUBST(SOCKET_LIBS)

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

AC_CHECK_HEADERS(netinet/in.h winsock.h)

AC_SUBST(ac_aux_dir)

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

