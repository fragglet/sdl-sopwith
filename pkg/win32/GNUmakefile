
include ../config.make

PANDOC_FLAGS = -s --template=default.html5 -H ../style.html
TOPLEVEL=../..

ZIPFILE = $(PACKAGE_TARNAME)-$(PACKAGE_VERSION)-win32.zip

all: $(ZIPFILE)

$(ZIPFILE): staging
	zip -j -r $@ $</*

staging:
	mkdir $@
	LC_ALL=C ./cp-with-libs --ldflags="$(LDFLAGS)" \
	               $(TOPLEVEL)/src/sopwith.exe $@

	$(STRIP) $@/*.exe $@/*.dll

	-for f in $(DOC_FILES); do \
		pandoc $(PANDOC_FLAGS) -f gfm -o $@/$$(basename $$f .md).html \
		       $(TOPLEVEL)/$$f; \
	done
	pandoc $(PANDOC_FLAGS) -f man -o $@/Manual.html \
	       $(TOPLEVEL)/doc/sopwith.6
	pandoc $(PANDOC_FLAGS) -f man -o $@/Configuration.html \
	       $(TOPLEVEL)/doc/sopwith.cfg.5
	pandoc $(PANDOC_FLAGS) -f man -o $@/Mission.html \
	       $(TOPLEVEL)/doc/sopwith-mission.5

clean:
	rm -f $(ZIPFILE)
	rm -rf staging
