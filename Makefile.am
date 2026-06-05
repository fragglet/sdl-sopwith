AUX_DIST_GEN =                          \
        $(ac_aux_dir)/install-sh        \
        $(ac_aux_dir)/missing

# The `make distcheck` command does a simulated install to check that
# `make install` works correctly; we need to pick a different path for
# the high scores directory, otherwise we will likely get permissions
# errors with the default path:
AM_DISTCHECK_CONFIGURE_FLAGS = \
	"--with-hiscores-path=$$dc_install_base/hiscores"

DOC_FILES=            \
        COPYING.md    \
        FAQ.md        \
        NEWS.md       \
        README.md     \
        PHILOSOPHY.md

missionsdir = $(docdir)/examples
missions_DATA =               \
        maps/README.md        \
        maps/bogey.sop        \
        maps/doghouse.sop     \
        maps/empty.sop        \
        maps/new_targets.sop  \
        maps/original.sop     \
        maps/sopwith1.sop     \
        maps/tank_strike.sop  \
        maps/title.sop        \
        maps/triplane.sop

EXTRA_DIST = $(AUX_DIST_GEN) $(DOC_FILES) $(missions_DATA) \
             .clang-format \
             embuild.sh \
             icon.png

MAINTAINERCLEANFILES = $(AUX_DIST_GEN)

SUBDIRS=src doc pkg
if HAVE_TESTS
SUBDIRS+=tests
endif

docdir=$(prefix)/share/doc/@PACKAGE@

format:
	clang-format -i src/*.[ch] src/sdl/*.[ch]
