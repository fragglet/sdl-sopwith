AC_INIT([SDL Sopwith],[1.99.0],[fraggle@gmail.com],[sdl_sopwith])
AC_CONFIG_AUX_DIR(autotools)

VERSION=$CONFIG_VERSION
AC_SUBST(VERSION)

AC_CANONICAL_TARGET

dnl optimisations

AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE([1.8.0 foreign])

dnl check for cc

AC_PROG_CC
AC_PROG_RANLIB

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
esac

AC_CONFIG_HEADERS([config.h:config.hin])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl install programs

AC_PROG_INSTALL

dnl libraries

AM_PATH_SDL(1.1.3)

dnl check for TCPIP support

AC_CHECK_HEADER(netinet/ip.h, [
    AC_DEFINE(TCPIP, 1, [TCPIP multiplayer support])
],,)

AC_SUBST(ac_aux_dir)

dnl generate Makefiles

AC_CONFIG_FILES([
Makefile
src/Makefile
src/sdl/Makefile
doc/Makefile
])
AC_OUTPUT

