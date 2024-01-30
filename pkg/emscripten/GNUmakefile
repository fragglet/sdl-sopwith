
TOPLEVEL = ../..
DIST_FILES = error.png loading.gif sopwith.html
SOPWITH_JS = $(TOPLEVEL)/src/sopwith
SOPWITH_WASM = $(TOPLEVEL)/src/sopwith.wasm

all: page

page: $(SOPWITH_JS) $(SOPWITH_WASM) $(DIST_FILES)
	mkdir -p $@
	cp $(DIST_FILES) $(SOPWITH_WASM) $@/
	cp $(SOPWITH_JS) $@/sopwith.js

clean:
	rm -rf page
