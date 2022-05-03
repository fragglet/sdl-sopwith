AUX_DIST_GEN =                          \
        $(ac_aux_dir)/install-sh        \
        $(ac_aux_dir)/missing

# extra distributed MSVC build files
MSVC_DIST= \
	msvc/README  msvc/config.h msvc/sopwith.vcproj

DOC_FILES=            \
        COPYING.md    \
        FAQ.md        \
        NEWS.md       \
        README.md

EXTRA_DIST = $(AUX_DIST_GEN) $(MSVC_DIST) $(DOC_FILES) \
             icon.png

MAINTAINERCLEANFILES = $(AUX_DIST_GEN)

docdir=$(prefix)/share/doc/@PACKAGE@
SUBDIRS=src doc pkg

