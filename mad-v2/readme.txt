Greetings, fellow Hero6 members,

This is the official release of the MAD project to the Hero6 team. If you make
any useful modifications to MAD (add new stuff, bugfixes, optimalizations etc.)
please email them to me (emailing the source file along with your story is
enough).

The source of MAD is now 100% portable, so whatever you do, don't break this.
The Windows 9x version source of MAD is included in SOURCE\WINDOWS, and it
compiled perfectly under my version Microsoft Visual C (I use 4.2). Ignore
the warnings...

Well, here is the TODO list. Please look for a more accurate copy at my home
page at http://www.rink.dhs.org/mad. Please contact me if you are willing to
do any of these tasks.

TODO:
- Fix the Windows version:
  - keyboard input sucks
  - Release version does not compile
  - WinConfig works, but the interprinter doesn't actually use it.
  - Gives a lot of general protection errors. Anyone know why?
  - Fix MAD in a window.
- Create a sound engine.
- Change the font manager so all chars don't have to be of the same size.
- Maybe some optimalization?
- Fix the MAD Studio so it displays all pictures with the correct palette
  (anyone knows how to do this?)
- Create a Xwindows port (I already got someone for this, but he can use some
  assistance)
- all things I forgot...

One more thing: DO NOT USE ANY OF THE MAD SOURCE CODE IN YOUR OWN PROJECT!!!
I've been working on MAD since May 1998 or so, and MAD is NOT meant to be
a 'copy-and-paste' project. If you would like to use MAD on a project other
than Hero6, please send me some email.

The MAD directory structure:
\PROJECTS\MAD (or whatever)
      |
      |--> ANM
      |    This lists all the animations, along with scripts to create them.
      |    Be sure to check it out, if you are an artist.
      |
      |--> DATAFILE
      |    This directory lists all stuff that goes in the MAD data file,
      |    along with a MAKE.BAT file to make it.
      |
      |--> DOC
      |    This lists all MAD documentation (actually, only the file formats)
      |    If you want to be sure you read the latest documentation, please
      |    visit http://www.rink.dhs.org/mad.
      |
      |--> GFX
      |    Here are all MAD graphics, all in Deluxe Paint II Enhanced .LBM
      |    format. Since much people consider Deluxe Paint abandonware, since
      |    it isn't sold anymore and really an OLD program, I decided to use
      |    it. Most of the graphics are ripped from Quest for Glory 1 EGA, so
      |    don't use anything from this directory. The font in SYSFNT is our
      |    system font. It will be in use until PHAttiE gives me the new font,
      |    which is almost done (I hope)
      |
      |--> OBSOLETE
      |    This is all obsolete stuff from the MAD project. It is no longer in
      |    use, but maybe you'd like to check it out.
      |
      |--> RELEASE
      |    This is the MAD release directory. It contains the latest MAD
      |    releases, along with some other info. Be sure to read RELEASE.TXT
      |
      |--> SCRIPTS
      |    Here are all MAD scripts, along with a Makefile to build them. If
      |    you need documentation about the script language, you can find it
      |    at http://www.rink.dhs.org/mad. Please email me if there's any-
      |    thing you don't understand.
      |
      |--> SOURCE
      |    This contains all MAD source (surprise, surprise) as well as the
      |    Windows source.
      |
      |--> TEXT
      |    This contains the MAD text script to create the text files.
      |
      |--> UTILS
           This contains all MAD utilities with the source.

This version of MAD should be considered the development version. All Hero6
members should have it. All patches will be from this version. You should
visit my home page at least once in a week to check whether files have been
updated.

Please feel free to email me any questions you might have (and you do, I'm
sure!)

Rink aka tuoic
email: rink@rink.dhs.org
MAD home page: http://www.rink.dhs.org/mad
