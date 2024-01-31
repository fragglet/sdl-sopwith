
include ../config.make

TOPLEVEL = ../..
DIST_FILES = error.png loading.gif
SOPWITH_JS = $(TOPLEVEL)/src/sopwith
SOPWITH_WASM = $(TOPLEVEL)/src/sopwith.wasm

all: page

page: $(SOPWITH_JS) $(SOPWITH_WASM) $(DIST_FILES) sopwith.html
	mkdir -p $@
	sed "s/__PACKAGE_STRING__/$(PACKAGE_STRING)/" \
	     < sopwith.html > $@/sopwith.html
	cp $(DIST_FILES) $(SOPWITH_WASM) $@/
	cp $(SOPWITH_JS) $@/sopwith.js

clean:
	rm -rf page
