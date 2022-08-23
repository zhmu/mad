/* are we already included? */
#ifndef __MAD_INCLUDED__
/* no. set flag we are included, and define things */
#define __MAD_INCLUDED__

#ifndef __NOINCLUDES__
#include "types.h"
#endif

#define STARTUPTITLE "MAD Version 1.0"

#ifndef WINDOWS
#define MAD_MAINARCHIVEFILE "data.mad"
#else
#define MAD_MAINARCHIVEFILE "data.mad"
#endif

#define MAD_SCREXTENSION    ".lm"
#define MAD_PALEXTENSION    ".pal"
#define MAD_FNTEXTENSION    ".fnt"
#define MAD_TXFEXTENSION    ".txf"
#define MAD_INVEXTENSION    ".inv"
#define MAD_SYSMOUSEXT      ".spr"
#define MAD_PROJECTFILE     "project.prj"

#define DEBUGFILENAME	"MAD!BUG.NFO"

#define MAD_TEMPSTR_SIZE    512

#define MAD_FLAG_STARTSTEP  1

#define DRV_TYPE_GFX		1
#define PROFILE_FPS

#ifdef DEBUG
#define DEBUG_AVAILABLE
#endif

#ifdef DOS
#define KEYBOARD_HANDLER
#endif

void mad_exit();
void die(char *msg);

extern char debugmode;

extern class ARCHIVE *archive;
extern class BAR *bar;
extern class GFX *gfx;
extern class GFXDRIVER *gfxdriver;
extern class MOUSE *mouse;
extern class PROJECT *project;
extern class SCRIPT *mainscript;
#ifndef MAD_COMPILER
extern class SCRIPT *script;
#endif
extern class CONTROLS *controls;
extern class FONTMAN *fontman;
extern class OBJMAN *objman;
extern class DLGMAN *dlgman;
extern class PARSER *parser;
extern class TEXTMAN *textman;
extern class SNDDRIVER *snddriver;
extern class INV *inv;
extern class CTRLDRV *ctrldrv;

extern void idler();

extern _ULONG main_speed;
extern _UINT ego_object;
extern _ULONG mad_flags;

#endif
