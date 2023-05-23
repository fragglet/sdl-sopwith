# Makefile for updating HTML versions of Unix manpages (should be re-run
# with each new release).

all: sopwith.6.html sopwith.cfg.5.html

sopwith.6.html: synced
	pandoc -f man -s sdl-sopwith/doc/sopwith.6 -o sopwith.6.html

sopwith.cfg.5.html: synced
	pandoc -f man -s ~/sdl-sopwith/doc/sopwith.cfg.5 -o sopwith.cfg.5.html

synced:
	git submodule update --checkout sdl-sopwith

