# Unit tests using Check framework

TESTS = swcollsn_test
check_PROGRAMS = swcollsn_test

# Use the same compiler flags as the main build
AM_CFLAGS = $(CFLAGS) -I$(top_srcdir)/src

# Test source files
swcollsn_test_SOURCES = swcollsn_test.c
swcollsn_test_LDADD = $(top_builddir)/src/swcollsn.o $(CHECK_LIBS)
swcollsn_test_CFLAGS = $(CHECK_CFLAGS)