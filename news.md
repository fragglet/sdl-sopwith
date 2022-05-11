
## November 17, 2014

Updated manual page with additional information, provided by the Debian
project. Also applied some fixes to avoid crash at start-up, provided by
Fedora. Updated README, COPYING and AUTHORS files to provide current
information.

## May 22, 2014:

A few minor bug fixes have been added to Sopwith to make the code more stable
and make packaging easier.

## March 21, 2014:

A change in the way the GNU Compiler handles optimizations has been causing
some people to report Sopwith crashing on start-up. We have added a fix for
this issue and released a new version of Sopwith. Version 1.8.2 should fix all
known bugs to date.

## March 26, 2013:

John Corrado pointed out a bug in Sopwith which would prevent sound from
initializing on some systems. This bug has been fixed and the manual page has
been updated to include the command line flags for playing sound (-p) and
running Sopwith in quiet mode (-q). Thanks to John for reporting this issue.

## February 22, 2013:

A bug has been brought to our attention which could cause Sopwith to crash if
the game is unable to find a suitable display screen. This won't be an issue
for most people, but in case it does we want to make sure the game exits
gracefully and displays a reason why. Version 1.8.0 of Sopwith fixes this bug.
Thanks to Denis M and Sergey P for reporting this issue and testing the fix.

This new release also cleans up some warnings we were getting from the
configuration script and drops support for GTK-2 builds as GTK-2 is obsolete.

## September 19, 2012:

We are pleased to announce a new version of Sopwith is now ready for download.
Mostly this release focuses on fixing minor bugs and code cleanup. The game now
compiles cleanly (no errors or warnings) when built using GNU GCC or the Clang
compiler.

We are also happy to say players can now customize their control keys. When
Sopwith runs it checks for the presents of a file called *~/.sopwith/keys*. If
this file exists, then key configurations are read from the file. If the file
does not exist, then Sopwith will create the file with the default keys,
allowing the player to use it as a template. The file is plain text and can be
altered with any simple text editor. Windows users should find this file under
their profile in the *AppData/Local/.sopwith/keys* folder.

Clear skies!

## September 6, 2010:

There is a new version of Sopwith availabe for download! This new release,
1.7.4, is mostly focused on minor fixes. We have applied patches to bring us
into line with Debian and updated the documentation. Thanks to Ken from the
Debian project for helping us! This release also includes slightly more intense
oil tank explosions. The extra force can be turned off by using the "-e" flag
on the command line.

## June 18, 2010:

We have a new version of Sopwith, 1.7.3, now available on the download mirrors.
This release is focused on cleaning up bugs which had been reported from
various distributions (mostly Debian). Some highlights are: firing missiles at
the side of the screen should no longer cause a crash, the mouse pointer will
no longer appear over the game window, sound is now off by default (but can be
turned on using the -p flag). The AI has been slightly improved and the
throttle should now work more smoothly.

I am also happy to report that Sopwith is now included in the
[PC-BSD](http://www.pcbsd.org/) package repository. Many thanks to Kris Moore
for helping us accomplish this.

## June 6, 2010:

It's been a while since we had any news to share, but things are stirring again
at at SDL-Sopwith. To start off, we have a new member on the team, Jesse, who
will be making some small changes and fixing a few bugs. If you have questions,
comments, complaints or fan mail, he'll be happy to hear from you, 'cause
that's what we pay him for.

The other piece of news we have today is that there is a new release of Sopwith
available on our download page. This release, 1.7.2, fixes some compiler errors
on modern Linux machines and on FreeBSD. It also introduces the "-g" command
line flag, which will let players jump to a higher game level. Generally,
Sopwith starts at level zero and gets more difficult. Using *sopwith -g2* will
jump the player to level two.

More updates will come soon. Clear skies, everyone!

