# Makefile for building macOS app and DMG package.

include ../config.make

export MACOSX_DEPLOYMENT_TARGET=10.7

STAGING_DIR=staging
DMG=$(PACKAGE_TARNAME)-$(PACKAGE_VERSION).dmg

TOPLEVEL=../..

# DMG file containing package:

$(DMG) : tmp.dmg
	rm -f $@
	./dmgfix "$(realpath tmp.dmg)" "$(PACKAGE_STRING)" "$(PACKAGE_NAME).app"
	hdiutil convert -format UDBZ -o $@ tmp.dmg
	rm -f tmp.dmg

tmp.dmg : $(STAGING_DIR)
	rm -f $@
	hdiutil makehybrid -hfs -hfs-volume-name "$(PACKAGE_STRING)"     \
	                   -hfs-openfolder $(STAGING_DIR) $(STAGING_DIR) \
			   -o tmp.dmg

# Staging dir build for package:

APP_DIR=$(STAGING_DIR)/$(PACKAGE_NAME).app
DOC_DIR=$(STAGING_DIR)/Documentation
APP_TOP_DIR=$(APP_DIR)/Contents
APP_BIN_DIR=$(APP_DIR)/Contents/MacOS
SRC_INFO_PLIST=Info.plist

$(STAGING_DIR): app.icns
	rm -rf $(STAGING_DIR)
	mkdir $(STAGING_DIR)

	mkdir -p "$(APP_TOP_DIR)"
	mkdir -p "$(APP_TOP_DIR)/Resources"
	cp PkgInfo "$(APP_TOP_DIR)"
	cp app.icns "$(APP_TOP_DIR)/Resources"
	cp $(SRC_INFO_PLIST) "$(APP_TOP_DIR)"

	mkdir -p "$(APP_BIN_DIR)"

	cp $(TOPLEVEL)/COPYING.md "$(STAGING_DIR)/Software License"
	ln -s /Applications "$(STAGING_DIR)"

	./cp-with-libs $(TOPLEVEL)/src/sopwith "$(APP_BIN_DIR)"
	$(STRIP) "$(APP_BIN_DIR)/sopwith"

	mkdir -p "$(DOC_DIR)"
	for d in $(DOC_FILES); do \
		pandoc -f gfm -o $(DOC_DIR)/$$(basename $$d .md).html  \
		       -s $(TOPLEVEL)/$$d;                             \
	done
	pandoc -f man -o $(DOC_DIR)/Manual.html \
	       -s $(TOPLEVEL)/doc/sopwith.6
	pandoc -f man -o $(DOC_DIR)/Configuration.html \
	       -s $(TOPLEVEL)/doc/sopwith.cfg.5
	cp disk/dir.DS_Store $(STAGING_DIR)/.DS_Store
	cp disk/background.png $(STAGING_DIR)/background.png

app.icns:
	mkdir app.iconset
	sips -z 64 64 ../../icon.png --out app.iconset/icon_32x32@2x.png
	iconutil -c icns app.iconset

clean:
	rm -f $(DMG)
	rm -rf $(STAGING_DIR)
	rm -rf app.icns app.iconset

