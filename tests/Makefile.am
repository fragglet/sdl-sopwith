# Unit tests using Check framework

TESTS = swconf_test swcollsn_test
check_PROGRAMS = swconf_test swcollsn_test

# Use the same compiler flags as the main build
AM_CFLAGS = $(CFLAGS) -I$(top_srcdir)/src

# Link against Check framework and the actual source files
swconf_test_LDADD = $(top_builddir)/src/swconf.o $(CHECK_LIBS)
swconf_test_CFLAGS = $(CHECK_CFLAGS)

# Test source files
swconf_test_SOURCES = swconf_test.c

swcollsn_test_SOURCES = swcollsn_test.c
swcollsn_test_LDADD = $(top_builddir)/src/swcollsn.o $(CHECK_LIBS)
swcollsn_test_CFLAGS = $(CHECK_CFLAGS)
