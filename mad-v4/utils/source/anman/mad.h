/* are we already included? */
#ifndef __MAD_INCLUDED__
/* no. set flag we are included, and define things */
#define __MAD_INCLUDED__

#define STARTUPTITLE "MAD Version 1.0"

#define MAD_MAINARCHIVEFILE "DATA.MAD"
#define MAD_SCREXTENSION    ".scr"
#define MAD_PALEXTENSION    ".pal"
#define MAD_PROJECTFILE     "project.prj"

#define DEBUGMODE_AVAILABLE									/* can activate debugging mode */

#define DEBUGFILENAME	"MAD!BUG.NFO"

void shutdown();
void mad_exit();

extern char debugmode;

extern class ARCHIVE *archive;
extern class BAR *bar;
extern class GFX *gfx;
extern class GFXDRIVER *gfxdriver;
extern class MOUSE *mouse;
extern class PROJECT *project;
extern class SCRIPT *script;
extern class CONTROLS *controls;

#endif
