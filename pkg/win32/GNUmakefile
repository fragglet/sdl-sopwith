
include ../config.make

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

	-for f in $(DOC_FILES); do                              \
		pandoc  -f gfm -o $@/$$(basename $$f .md).html  \
		        -s $(TOPLEVEL)/$$f;                     \
	done

clean:
	rm -f $(ZIPFILE)
	rm -rf staging
