AC_INIT([SDL Sopwith], [2.9.0], [fraggle@gmail.com], [sdl-sopwith])
AC_CONFIG_AUX_DIR(autotools)

VERSION=$CONFIG_VERSION

PACKAGE_COPYRIGHT="Copyright (C) 1984-2024"
PACKAGE_LICENSE="GNU General Public License, version 2"

AC_SUBST(VERSION)
AC_SUBST(PACKAGE_COPYRIGHT)
AC_SUBST(PACKAGE_LICENSE)

AC_CANONICAL_TARGET
AC_SEARCH_LIBS([sin], [m])

AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE([1.8.0 foreign])

AC_PROG_CC
AC_PROG_RANLIB
AC_CHECK_TOOL(STRIP, strip, )
AC_CHECK_TOOL(CONVERT, convert, )
AC_CHECK_HEADERS(stdbool.h)
AC_CHECK_FUNCS(isatty)

if test "$GCC" = "yes"; then
        WARNINGS="-Wall -Wdeclaration-after-statement -Wredundant-decls"
        CFLAGS="-O2 -g $WARNINGS $CFLAGS"
fi

dnl network libraries; not all systems have the sockets API in their libc:
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

case "$host" in
    *cygwin* | *mingw* | *emscripten* | *haiku*)
        HISCORES_PATH=
        ;;
    *)
        HISCORES_PATH=/var/games/sopwith
        ;;
esac
AC_ARG_WITH([hiscores-path],
            [AS_HELP_STRING([--with-hiscores-path=dir],
                            [directory for system-wide high scores file])],
            [HISCORES_PATH=${with_hiscores_path}], [])
AC_SUBST(HISCORES_PATH)

AM_CONDITIONAL(GLOBAL_HISCORES, test "$HISCORES_PATH" != "")
AC_DEFINE_UNQUOTED(HISCORES_PATH, "$HISCORES_PATH",
                   [Path to directory containing high scores file.])

AM_CONDITIONAL(HAVE_WINDRES, test "$WINDRES" != "")
AM_CONDITIONAL(HAVE_CONVERT, test "$CONVERT" != "")
AC_CONFIG_HEADERS([config.h:config.hin])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_PROG_INSTALL
PKG_CHECK_MODULES(SDL, [sdl2 >= 2.0.7])

dnl Unit tests using Check framework
AC_ARG_ENABLE([tests],
              [AS_HELP_STRING([--enable-tests],
                              [build unit tests (requires Check framework)])])
if test "x$enable_tests" = "xyes"; then
    PKG_CHECK_MODULES([CHECK], [check >= 0.9.4])
    AC_DEFINE([HAVE_TESTS], [1], [Define if unit tests are enabled])
fi
AM_CONDITIONAL([HAVE_TESTS], [test "x$enable_tests" = "xyes"])
AC_CHECK_HEADERS(netinet/in.h winsock.h)

AC_SUBST(ac_aux_dir)

AC_CONFIG_FILES([
Makefile
doc/Makefile
pkg/config.make
pkg/macos/Info.plist
pkg/Makefile
src/Makefile
src/resource.rc
src/sdl/Makefile
tests/Makefile
])
AC_OUTPUT

