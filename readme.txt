0. Preface

This archive contains the entire source of MAD, including all assorted
tools. A few example projects have been included as well.

1. Introduction

Back in 1998, the MAD project was started. The idea was to create a script
language plus interpreter, intended to create adventure games. One of the
main sources of inspiration was Sierra's Creative Interpreter (SCI), which
was used as the driving force of the famous King's Quest and Quest for Glory
series.

During the project's lifetime, it was eventually accepted as the system of
choice for the Hero6 project (http://www.hero6.com). This resulted in requests
for feature that were not currently possible, such as true-color support, a
dialogue system and much more.

After working on MAD for over 4 years, Nunzio Hayslip and Javier Gonzales
offered to continue the project, which is the current generation. One of the
main changes was to switch to the Lua script language (http://www.lua.org), as
it seemed to work very well for major games like Grim Fadango and Escape from
Monkey Island. I was barely involved in this effort, and extremely little of the
original MAD code and utilities were left intact. However, the overall ideas of
the MAD engine were followed extremely closely.

2. Versions

There are basically three main versions of MAD: version 2, version 4 and version
5. Version 5 is the Lua version, which is available from SourceForge
(http://mad-project.sourceforge.net) and not included in this archive.

Version 2, which is shipped in the mad-v2 subdirectory, is the version as it
was presented to the Hero6 project (the original readme.txt as shipped to Hero6
is included). This version supports only 320x200 pixels at 256 colors, which is
identical to what the old adventure games used as well.  A minor remake of
Quest for Glory 1 EGA edition, using the MAD engine, was presented along with
the project.

Version 4, which is shipped in the mad-v4 subdirectory, is the version used in
the later Hero6 demos. It supports resolutions from 320x200 onward (which was
the only resolution ever to be used in reality) at 32 bit colors - the latter
as computers of that time easily supported this resolution and it made the job
for the artists a lot easier.

Between the two versions, the script language had massive changes. One of the
most important is the support for inheritance in version 4, which made writing
the game logic code a lot easier.

3. Example projects

Within the archive, 4 example projects are included. Two of them are for 
version 2, the other two are for version 4. Building instructions are presented
as appendix A.

3.1. QG1MAD (Version 2)

This is the remake for Quest for Glory 1 EGA version in MAD. It was intended
only to show off the features of the MAD system, never to be a serious game.
All graphics were taken from Quest for Glory 1, the font and icons were drawn
by myself (as you can see, I'm not an artist :-).

3.2. H6DIALOGUE (Version 2)

The first official use of MAD, this is the dialogue demo released by the Hero6
project. Even though it showed only a faction of MAD's power, it did fairly well.

3.3. H6POOL (Version 4)

The second official use of MAD, an interactive demo which shows some very nice
features of the interpreter. Due to the lack of a sound system (which was never
written for version 2 or 4), the demo released officially was created using the
Adventure Game Studio, even though the MAD demo was already available.

3.4. H6MULTI (Version 4)

About my final work on MAD was trying to make it multiuser, as I always fancied
the idea of a multiperson online RPG (and mind you that things like World of
Warcraft did not exist at the time). The main idea was that each client would
run by itself as much as possible - if games are joined by multiple clients,
only the main player object would be visible. This would prevent issues like
running out of sync.

However, the idea was never truly finished. Since only (unfinished) content
from the Hero6 project was available to use at that time, you can walk in front
of trees and such, and there are only two scenes included yet there is more
data available. Multiplayer mode can be enabled by launching MAD with a '-m'
parameter; ALT-J can be used to join a game and ALT-T to talk. However, I'm
willing to bet that the interpreter will crash immediately due to bugs :-/

4. Debugging mode

All versions of MAD have a rich debugging mode, which can be enabled by
building the interpreter with the DEBUG define (the makefiles shipped do this
automatically). You can enter debugging mode by hitting the tilde key (located
left to the 1), which gives you a console-like mode. Hitting tilde again
leaves debugging mode.

4.1. Version 2

Once in debugging mode, the following commands are available:

a	Show background screen
b	Add breakpoint at current location
d	Delete breakpoint at current instruction
j	Skip the current instruction
l	Show current breakpoints
m	Show mask screen
n	Replaces the current instruction by No OPeration opcodes.
o	Display list of objects
p	Show priority screen
r	Reload the current script
s	Single steps through the script
t	Trace (like single step, but ignores the main script)
q	Quit the interpreter
z	Zap all breakpoints
=	Increase instruction pointer to next instruction
-	Decrease instruction pointer to previous instruction
.	Set instruction pointer to current instruction

Additionally, it's possible to hit the following keys anytime in the game:

ALT-D	Defrost controls (makes you able to control the character)
ALT-F	Freeze controls
ALT-M	Show mask screen
ALT-P	Show priority screen
ALT-R	Show reachable areas

The instructions used by the VM are documented in the doc/madvm.txt file.

4.2. Version 4

MAD version introduced a much more advanced debugger; the status of the main
character was now shown in the debugging screen, and it was possible to
inspect much more of the game's status.

Again, the tilde had to be used to enter the debugger. Below is a list of
commands. Commands marked with a (*) are not (fully) implemented in version 4.

a	Display background graphics screen
b	Add a breakpoint at the current location (*)
d	Delete breakpoint at current location (*)
j	Skip current instruction (*)
l	Display list of breakpoints
m	Display mask screen
n	Animation checker, allows you to inspect all loaded .ANM animations
o	Display list of objects
p	Display priority screen
q	Quits the interpreter
r	Reload current script (*)
s	Single step an instruction
t	Trace (like single step, but ignores the main script)
z	Zaps all breakpoints (*)
=	Increase instruction pointer to next instruction
-	Decrease instruction pointer to previous instruction
.	Set instruction pointer to current instruction

Additionally, it's possible to hit the following keys anytime in the game:

ALT-D	Defrost controls (makes you able to control the character)
ALT-F	Freeze controls
ALT-M	Show mask screen
ALT-P	Show priority screen
ALT-R	Show reachable areas
ALT-T	Teleport to another room

5. Concluding words

Why did I chose to release this project after so many years? Actually, I had the
idea much earlier, but kind of forgot about it as it really is a lot of work to
clean this project up to such an extent that I felt confident releasing it.

However, a friend of mine (Willem van Engen) sent me an email claiming that the
MAD project was famous: it is described in the book "Game programming with
Python, Lua and Ruby" by Tom Gutschmidt! It covered MAD version 5 in pretty
good detail (I'll even forgive him the typo he made in my name - it's Rink, not
Rick), so I was like: well, if someone wrote a book including this project, why
not be fair and release the entire works?

Does the MAD project have future? I think so - if some daring soul would pick
it up again. I'm still not convinced that the current alternatives are
feasible, since you need not only a solid language but also a good toolset to
accompany it with. If I'd do it all over again, I would really open-source it
since day one, and put it in a version control system immediately (which didn't
make much sense for a mostly one-man-project at the time :-). Furthermore, you
need exposure - such a project is a huge effort and even discussing your ideas
with people is a very good way of getting a better system (ever wondered why
there were 4 different invocations of the language?)

Anyway, it's time for me to close this project. It was a very good project at
the time, and I hope a truly open and powerful interpreter like MAD will hit
the internet some day...

A. Build instructions

In theory, MAD should build on MS-DOS (using DJGPP), UNIX (using GCC) and
Windows (using MS Visual Studio). I must admit I have only cleaned the code
up to such an extent that it builds on my FreeBSD 7.0 machine (both i386 and
amd64 have been tested). You are very welcome to try it on Windows and DJGPP -
if you have anything to contribute, let me know and I'll add it to the archive.

There are minor build differences between MAD version 2 and 4, which are
detailed in the next section. I list only the UNIX build instructions - mind
that I use 'gmake' because that is GNU Make on FreeBSD systems.

A.1 Version 2

In order to build the interpreter, do the following:

$ cd source
$ gmake x                 To build the X11 version
$ gmake ggi               To build the libggi version

The libggi version provides scaling and is probably the most-tested version.

In order to build the utilities, do the following:

$ cd utils
$ gmake

Once the utilities have been build, the example projects can be built using:

$ cd projects/qg1mad/datafile
$ gmake

This will result in a file 'data.mad' placed in the current folder. This file
should be copied to wherever your 'mad' interpreter binary is.

$ cd projects/qg1mad/h6dialogue
$ gmake

This is used to create the Hero6 dialogue demo.

A.2 Version 4

There are multiple makefile for the interpreter. For the UNIX version, do the
following to build the interpreter:

$ cd source
$ gmake -f makefile.unix x		For the X11 version
$ gmake -f makefile.unix ggi		For the libggi version

The utilities can still be built in the same was as with version 2:

$ cd utils
$ gmake

Building the example projects is in the same way:

$ cd projects/h6pool
$ gmake

Again, place the data.mad file in the same directory as the interpreter and
you are good to go.

$ cd projects/h6multi
$ gmake

Same as above.

B. Differences between the original version

Below is a hopefully complete list of difference between the original thing I
started out with and the files in this archive:

- The X11 driver has been vastly improved, it has keyboard support for one.
- The original code has the 'delay_ms(gamespeed);' call commented out, since
  most computers were too slow at the time to use such a way of guaranteeing
  a fixed value and I never cared enough to improve it (this is an obvious
  mistake, as the old Hero6 demos run with lightspeed now without it :-/)
- The makefiles as shipped in this archive used to build the demo projects
  were never present - you needed to use batch files (which have been removed)
- The makefiles used to build the utilities were broken.
- The 'flic2spr' tool shipped in the MADv4 distribution was never properly
  integrated in the distribution until now.
- Of course, there are a lot of tiny GCC 4+ fixed. Remember that most of this
  code was written when GCC 2.95 was the de facto standard!

C. Known issues

Of course, no project this large is perfect, and since this project was never
really finished this especially holds for the MAD project. Thus, I present a
list of known issues:

- Portability issues: while MAD was designed to be portable, I did not think
  about that while reading/writing files, which always assume native
  endianness. Compiling MAD works fine, but using any datafile will result in
  '<name> is not a <type> file' errors (this is for one the case when try to
  use MAD on my iBook running Linux)

- There are still a lot of bugs present. Even while preparing this release,
  I fixed a crash bug in the H6DIALOGUE demo. A lot of these bugs are due to
  assumptions that no longer seem to hold due to compiler changes, etc.

- The colors in the H6DIALOGUE demo are incorrect. This may have to do with an
  incorrect palette file, but then again, it may not: I haven't analyzed it.
 
D. Acknowledgments

I would like to personally thank Willem van Engen for listening to all my rants
about the project while it still was in active development, and his insightful
comments at the time. I believe this has influenced the project majorly in a
good way.

Secondly, Nunzio Hayslip and Javier Gonzales, have done a very good job on
MADv5 and I'm grateful that they had the energy and dedication to continue this
project. It's a shame it never quote got the attention it deserved, but maybe
it will prove to be useful again in the future.

Finally, Hero6 was the sole user of the MAD engine, and I believe the demos have
always been quite impressive. I would like to thank them for making this choice
(which was by no means a easy at the time) and wish them the very best with the
project.

Signed,

Rink Springer
October 2008

E. License

The MAD code, including all assorted tools and other utilities, is licensed
under the GNU Public License version 2. The supplied projects are licensed as
follows:

qg1mad: All graphics used are copyright (c) 1989 Sierra Online, with the
exception of the font, icon bar and mouse cursor (which were drawn by me and
therefore released as public domain). All scripts are licensed under the GPLv2.

h6dialogue, h6multi, h6pool: All content is copyright (c) 1999 - 2008 by the
Hero6 project. Details concerning use of the content should be addressed to the
Hero6 project; contact details are available at http://www.hero6.com.
