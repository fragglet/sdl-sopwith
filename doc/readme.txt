Release notes on the source code:

A message from Dave Clark:
> I have already begun to take emails requesting assistance with the
> compilation of the Sopwith code place on sopwith.org.  Although I would
> like to have the time to help everyone with its recompilation, I simply
> don't have the time.  If I can, I would like to ask that emails directed to
> me concerning the code be restricted to the higher level "Why did you do it
> this way", rather than the more "nuts and bolts" type questions (i.e. The
> source won't compile under Frisbot C++, what's wrong?).  If you really hit
> a roadblock with a particular piece of the code, I will help if I can, but
> please be very specific as to what the problem is (specific lines of codes,
> error messages, etc).  If anyone else wants to become a Sopwith porting
> expert, and is able to handle the support calls involved, my best wishes to
> that person.
>
> The code is written to compile under Microsoft C V6.0 (for DOS) and
> Microsoft Assembler V5.1 (for DOS).  I'm truly sorry, but I do not have the
> time to assist in porting it to any other compilers.
>
> I would ask that all questions concerning the code be sent to the Sopwith
> development eGroup, rather than to me directly.  In that way, the greater
> number of people can contribute to and learn from the answers.
>
> I hope I'm not coming across too heavy here.  I thoroughly encourage what
> folks are doing with Sopwith.  Time permitting, I will attempt to answer
> every eMail regarding Sopwith.  Please be patient, as Sopwith has become an
> unexpected addition to an already busy schedule.
>
> Thanks for you consideration
> Dave Clark

The Sopwith developers mailing list can be joined by going to
  http://www.egroups.com/subscribe/sopwith
and following the instructions there.

The files in the source code archive are as follows:
Dave Clark stuff:
  General stuff:
    LICENSE.TXT - The Sopwith license agreement
    NOTES.TXT   - This file
    SW.LNK      - Linker response file
    SW.MAK      - Makefile

  C Stuff:
    SW.H        - Main Sopwith header file. Included by all the C files.
                  Includes STD.H, SYSINT.H, SWDEVE.H and SWMACH.H. Has lots of
                  useful macros for constants and enumerations, as well as
                  definitions of the various structs.
    STD.H       - Header defining some handy macros and prototypes for some of
                  the functions in BMBLIB.C
    SWDEVE.H    - Defines the "DEVELOPE" macro.
    SWMACH.H    - Defines either the "IBMPC" macro or the "ATARI" macro
                  depending on the machine being compiled for.
    SYSINT.H    - Defines register structures used by sysint and sysint21 and
                  interrupts.
    SWMISC.C    - Miscellaneous support functions ("puts" function)
    _INTC.C     - Functions to handle Sopwith's interrupt vector table
    BMBLIB.C    - BMB library routines (command line processing, system
                  interrupts, hardware I/O and memory copying).
    SWASYNIO.C  - Asynchronous (serial port) communications
    SWAUTO.C    - The autopilot (computer player and automatic homing device)
    SWCOLLSN.C  - Collision detection routines
    SWDISP.C    - Display players and objects
    SWEND.C     - End of game cleanup and statistics
    SWGAMES.C   - Defines the positions, types and orientations of the runways
                  and buildings in the "world".
    SWGROUND.C  - Defines the array of altitudes which makes up the landscape
    SWGRPHA.C   - Low-level graphics routines
    SWINIT.C    - Initialization routines, command line and menus
    SWMAIN.C    - The main game loop
    SWMOVE.C    - Update objects
    SWMULTIO.C  - Imaginet multiplayer communications
    SWNETIO.C   - Modern networking replacement for SWMULTIO.C (or at least, it
                  will be...)
    SWOBJECT.C  - Routines for manipulating the object list
    SWSOUND.C   - Sound routines
    SWTITLE.C   - The title screen
    SWPLANES.C  - Graphics data for planes
    SWSYMBOL.C  - Other graphics data

  Assembler stuff:
    SEGMENTS.H  - Assembler macro defining relative position of function call
                  arguments on stack
    SW.HA       - Assembler include for various useful constants
    _INTA.ASM   - Interrupt service routines, routines to get and set interrupt
                  vectors
    _INTS.ASM   - Functions to turn interrupts on and off
    SWCOMM.ASM  - Low level serial communications
    SWGRPH.ASM  - Low level graphics routines (duplication of some of the stuff
                  in SWGRPHA.C)
    SWHIST.ASM  - Dummy history processing
    _DKIO.ASM   - Low level disk I/O for Imaginet
    SWUTIL.ASM  - Keyboard, joystick, timing, sound, text output and object
                  movement routines

Andrew Jenner stuff:
  General stuff:
    MAKEFILE     - Make file. Works with Borland make. Doesn't work with GNU
                   make for reasons which escape me (please tell me if you
                   know). Don't know about other flavours of make.
    RESPONSE.RSP - Linker command line (response) file to link Sopwith. Works
                   with Borland TLINK 2.0.
    RESPONS2.RSP - Linker command line (response) file to link Sopwith 2. Works
                   with Borland TLINK 2.0.
    SOPWITH.PRJ  - Turbo C 2.01 project file for Sopwith.
    SOPWITH2.PRJ - Turbo C 2.01 project file for Sopwith 2.
    DLCVSAMJ.TXT - File giving correspondances between identifiers in Dave
                   Clark's code and Andrew Jenner's. Work in progress.

  Code:
    SOPWITH.C    - Sopwith.
    SOPWITH2.C   - Sopwith 2.
    DEF.H        - Generic header file
    SOPASM.ASM   - Assembler support routines, common to Sopwith and Sopwith 2.
                   Works with A86.
    SOPASM.H     - Header file declaring the functions in SOPASM.ASM.
    GROUND.C     - Landscape height field, common to Sopwith and Sopwith 2.
    SPRITES.C    - Graphics data, common to Sopwith and Sopwith 2.

