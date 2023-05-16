AUX_DIST_GEN =                          \
        $(ac_aux_dir)/install-sh        \
        $(ac_aux_dir)/missing

DOC_FILES=            \
        COPYING.md    \
        FAQ.md        \
        NEWS.md       \
        README.md

EXTRA_DIST = $(AUX_DIST_GEN) $(DOC_FILES) \
             icon.png

MAINTAINERCLEANFILES = $(AUX_DIST_GEN)

docdir=$(prefix)/share/doc/@PACKAGE@
SUBDIRS=src doc pkg

