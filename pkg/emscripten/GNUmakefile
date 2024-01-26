
DIST_FILES = error.png loading.gif sopwith.html

all: page

page:
	mkdir $@
	cp $(DIST_FILES) $@/
	cp ../../src/sopwith $@/sopwith.js
	cp ../../src/sopwith.wasm $@/

clean:
	rm -rf staging
