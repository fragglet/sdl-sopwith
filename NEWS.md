# v2.1.0 (2022-08-31)

Bug fixes and changes in this version:

* libm is included as a dependency when needed (thanks slicer69).
* No error is printed if the config file doesn't exist (thanks Kenneth
  Pronovici).
* The screen is now redrawn properly when in menus and the window is uncovered
  or resized (thanks Kenneth Pronovici).
* Some potential crashes were fixed when near the edges of the map.
* Some of the code has been refactored to centralize how levels are defined. In
  the future this will help to support custom levels.

Documentation changes:

* Build instructions were clarified (thanks Kenneth Pronovici).
* Better documentation of novice mode was added to the manpage (thanks Kenneth
  Pronovici).
* References to the -p argument were removed from the manpage and the help text
  (thanks Kenneth Pronovici).

# v2.0.0

Changes in this version:

* Sound has been reenabled. Sound output is now passed through a bandpass
  filter intended to make it sound like a real PC speaker, and also plays
  back at a much quieter volume, to be less startling to new players.
* Medals are now awarded to reward players (thanks creichen). The Unix
  manpage includes some details about the medals and how to attain them.
* There is now a configuration screen for setting key bindings, which are
  stored in the same configuration file as other settings.
* Thanks to SDL2's renderer API, the game now scales up to arbitrary window
  sizes and resolutions, and isn't limited to just 320x200 or 640x400.
* Network performance has been improved significantly.
* The Unix manpage has been significantly improved and expanded. A separate
  manpage was added for the configuration file.
* Sopwith now runs on macOS, and Mac .dmg packages are generated.
* Also now builds and runs cleanly on Haiku.
* The Windows .exe now has metadata and an icon.
* The missiles feature was changed to a hidden setting since it's not very
  fun and doesn't really fit the game's theme.
* The networking option to use a "TCP loop" has now been merged with the
  normal "connect" option, for simplicity. The target server may be a real
  server or a TCP loop server, and the clients will autodiscover.

Bug fixes:

* Movement precision within the game was made significantly more precise.
  This fixes a long-standing issue where fractional movement values were
  just being discarded, instead of being included in calculations as they
  should have been.
* Fix computer planes not shooting when tailing the player's plane (creichen).
* Fix planes crashing into their own bullets (creichen).
* Fix players able to turn quickly when wounded and using the autopilot.
* Damaged ground no longer remains on the title screen after the game ends.
* Network games no longer desync when only one player uses harry keys mode.
* Big oil tank explosions now match the magnitude of the old explosions from
  Sopwith 1 and Sopwith 2. The command line option to disable them has been
  replaced with a configuration variable.
* A bug was fixed where it was possible to stop the plane in midair when
  playing the game in novice mode (thanks Alaux).

Restructuring:

* Code has been ported to SDL2.
* Rewritten graphics code
* Major internal restructuring: removal of obsolete code, removal of
  some global variables
* Sopwith configuration file is no longer ~/.sopwithrc on \*nix systems, but
  can now be found at ~/.local/share/SDL\ Sopwith/sopwith.cfg

# v1.8.4

* Updated man page (doc/sopwith.6) to match Debian's manual page. Provided by
  Kenneth Pronovici.
* Applied patches from Adrian Reber to bring Sopwith up to date with bug fixes
  provided by Fedora.  This should avoid crash on start-up problems and makes
  compiling more robust.

# v1.8.3

* The function asynclos() in swasynio.c includes a parameter which is never
  used. Calling functions do not bother passing a parameter. To avoid undefined
  behaviour the definition of aynclos() has been fixed.  Thanks for Michael
  Tautchnig for reporting this bug.  Closes Debian bug #748172.
* Updated manual page to provide current default keys and mention the location
  of the custom keys file (~/.sopwith/keys)
* Removed stray object files from tarball.
* Added rule to makefile "tarball" which performs more complete clean-up and
  makes tarball in parent directory.

# v1.8.2

* Added `SDL_CFLAGS` flag to src/Makefile to bring us in line with FreeBSD patches.
* Updated copyright notice on help screen.
* Removed compiler optimiations to avoid crashes when game is built using
  versions of GCC newer than 4.6.

# v1.8.1

* Corrected initialization error that would prevent sound from playing at
  start-up.
* Updated documentation to include command line flags for playing music (-p)
  and starting Sopwith in quiet mode (-q). Thanks to John Corrado for pointing
  out this bug.

# v1.8.0

* Dropped support for GTK-2 as toolkit is being obsolete.
* Tidied up configurating script.
* Fixed program crash when video display is not available. (Thanks to Denis M
  from the Gentoo team for reporting this bug and testing the patch.)

# v1.7.4

* Brought game into compliance with Debian patches. Fixed errors in
  documentation. (Thanks to Ken from the Debian project for helping with these
  items.)
* Added slightly more intense explosions to oil tanks. This new feature can be
  turned off from the command line with the "-e" flag.

# v1.7.3

* Fixed most outstanding bugs reported in Debian. Fixed some compiler warnings.
  Added intructions for exiting the program on the title page. Sound is now
  disabled by default.

# v1.7.2

* Cleaned up some compile errors from gcc 4.4.1 on Ubuntu.  Added -g command
  line option. -g and a number, such as -g2 will start the game on difficulty
  level 2. Normally the game starts at zero.

# v1.7.1

* Fix some compilation errors due to an error in swinit.c
* Fix networking which got broken somewhere.

# v1.7.0

* Allow control via arrow keys, for people with "unusual" keyboard layouts
* Fix right-ctrl for ctrl-c (thanks to electricelf)
* Support Gtk+ 2.0 and some GNOME HiGification

Psion Port:

* devfs support
* improved screen squishing for revo

Code cleanup:

* Removed a load of redundant code including the last remnants of the old
  Imaginet code. Also some archaic stuff like the ATARI/IBMPC defines
* Some of the keyboard code moved out into a system-independent module.

# v1.6.0

* Fixed bug with harry keys mode and returning to home from the left (thanks to
  Kodath duMatri)
* Fixed bullets being colored (instead of always white like they should be).
* Now totally under the GPL!

# v1.5.0

* Removed old redundant collision detection code
* PsiLinux port: Keyboard code added, video code improved. This port is now
  playable!
* Fixed gtk+ port turning off key repeat
* Fixed SDL version drawing a blank screen on changing video modes
* Fixed consistency failure with flocks between clients with different
  screen widths
* renamed "getmode" to "getgamemode" to avoid conflicts with standard unix
  function (thanks to Edwin Groothuis)
* SDL Icon is now set from the internal sprite table, not from an external
  file (no more "icon.bmp not found" errors)
* Added manual page

# v1.4.0 (29/06/2002)

* Start of PsiLinux port (for Psion Revo/5MX)
* Collision detection code rewritten
* Save button added to settings dialog in Gtk+ version
* Added original documentation to doc/origdoc.txt
* Code restructure:
  * All rendering is now done directly to the appropriate buffer, the old
    Atari rendering functions (which I thought were CGA functions) are no
    longer used as there are now native 8-bit versions.
  * `sopsym_t` is used for storing sprites; `ob->ob_symwdt/symhgt` removed as
    they are no longer of any use

# v1.3.0 (??/11/2001)

* Gtk+ port! (work in progress, no sound yet but it is playable)
* Game no longer thrashes the CPU when waiting for keypresses or the next
  gametic.
* 'S' key to toggle sound fixed
* Network connect now allows you to connect to a different port than the
  default (use host.domain.com:port)
* Network option to connect to a TCP loop (forwards between two sopwith
  clients using a third "man in the middle" server)
* Fixed "holes" in ground when using solid ground (the solid ground function
  was originally hacked from the line ground function, it is now practically
  rewriteen).
* Fixed out of sync netgames when starting a netgame after playing a
   single player game already.
* Build now uses autoconf/automake
* Code cleanup:
  * intson, intsoff calls removed, bmblib.[ch] removed from build
  * Build process changed: SDL code moved into a seperate subdirectory, Gtk+
    code is in another directory. All the game code is compiled into a single
    module "sopwith.o" and this is linked with the appropriate modules for
    Sound, Graphics and Timer. This allows multiple ports for Gtk+, SDL etc,
    while keeping the game itself seperate from the display specific code.
  * Added a 'dos' subdirectory with relevant code for if/when I do a DOS port.

# v1.2.0 (4/11/2001)

* Added options menu, game options are saved to disk (sopwith.ini on
  Windows systems, ~/.sopwithrc on Unix). These were available before but
  required command line args). Game options:
  * enable/disable missiles
  * solid ground (draw as a block like sopwith 1)
  * hud splats (option to disable)
  * wounded planes (option to disable)
  * animals (oxen and birds)
  * harry keys (pull up/down keys are relative to screen, not plane)
  * fullscreen/double scale video options
* More sensible versioning system
* When you die, you are taken back to the menu rather than immediately
  restarting.
* Splatted oxen turn the screen pink (as in Authors Edition)
* Icon
* Code cleanup: lots of reordering of the initial setup and restart code
* Menu code is now all in swtitle.c (except the options menu which is in
  swconf.c)

# v1.10 (2x/10/2001)

* Networking! I have hacked in TCP/IP Networking based on the old asyncronous
  multiplayer option code. There is a new option on the main menu "Network"
  through which network games can be started.
* Fixed use of auxdisp which was why the score was not displayed and was
  also why the map was not being properly refreshed.
* Fixed command line args and updated help screen
* Title screen based on the "network edition" title screen.
* Extra command line args:
  * `-f` (fullscreen)
  * `-2` (double size window for easier viewing)
  * `-l` (network mode, listen for connection)
  * `-j` (network mode, connect to host)
* Sopwith now quits if you hit ctrl-c, not escape, like in the original.
  However, for lazy people not wanting to wait while their plane crashes
  and the game exits, if you hit ctrl-c 3 times it quits immediately.
* Hitting escape in menus now takes you back to the previous menu.
* Major code cleanup - this is the main difference. I have created headers
  for all the source modules so that it is no longer neccesary to include
  millions of "extern" definitions in every file (which is tedious, ugly
  and prone to bugs). I have also reformatted all the code so that it looks
  nicer, and removed some obfuscations so that the code is more readable
  in places.
* CVS Tags have been added to all source files for if/when I put this into
  CVS.
* Various extra code hacks: sopwith1 theme music (was different music in
  sopwith 1, I extracted the music sequence from the hexdump),
* solid ground (also ala sopwith1)

# v1.00 (15/10/2001)

* Video and Sound is all working (afaik)
* ASM removed/replaced.
* Internally it draws everything as if it were still using a CGA screen, I have
  written some SDL code that "emulates" a CGA screen and draws it to an SDL
  screen.
* Sound is done in a similar way: The digital output in LibSDL is used to make
  the appropriately-pitched beepings to sound like a PC Speaker :) The API to
  the PC Speaker code is identical to those of a small library I wrote a while
  back to control the PC Speaker in DJGPP, this is incase I want to do a DOS
  port.
* Moved all of Andrew Jenners decompiled code into amj/, I do not use this
  directly but it is useful in deciphering some ASM functions.  attic/ has all
  the asm source and other source files that are redundant now.
* doc/ has all the documentation that came with the source
* current bugs: crashing into an ox doesnt turn your screen pink, map/status
  bar dont refresh properly (?? score not displayed)

