# Makefile for updating HTML versions of Unix manpages (should be re-run
# with each new release).

PANDOC_FLAGS = -s --template=default.html5 -H sdl-sopwith/pkg/style.html

all: sopwith.6.html sopwith.cfg.5.html

sopwith.6.html: synced
	pandoc -f man $(PANDOC_FLAGS) -s sdl-sopwith/doc/sopwith.6 -o $@

sopwith.cfg.5.html: synced
	pandoc -f man $(PANDOC_FLAGS) -s ~/sdl-sopwith/doc/sopwith.cfg.5 -o $@

synced:
	git submodule update --remote sdl-sopwith

