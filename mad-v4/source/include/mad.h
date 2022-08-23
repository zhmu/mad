/* are we already included? */
#ifndef __MAD_INCLUDED__
/* no. set flag we are included, and define things */
#define __MAD_INCLUDED__

#ifndef __NOINCLUDES__
#include "types.h"
#endif

#define STARTUPTITLE "MAD Version 3.0"

#define MAD_MAINARCHIVEFILE   "data.mad"
#define MAD_PATCHARCHIVEFILE  "patch.mad"
#define MAD_SPEECHARCHIVEFILE "speech.mad"
#define MAD_MUSICARCHIVEFILE  "music.mad"
#define MAD_LANGARCHIVEFILE  "language.mad"

#define MAD_SCREXTENSION    ".m"
#define MAD_PALEXTENSION    ".pal"
#define MAD_FNTEXTENSION    ".fnt"
#define MAD_TXFEXTENSION    ".txf"
#define MAD_INVEXTENSION    ".inv"
#define MAD_SYSMOUSEXT      ".spr"
#define MAD_PROJECTFILE     "project.prj"

#define MAD_SOCKETNO        12345

#define DEBUGFILENAME	"MAD!BUG.NFO"

#define MAD_TEMPSTR_SIZE    512

#define MAD_FLAG_STARTSTEP  1
#define MAD_FLAG_MPLAYER    2
#define MAD_FLAG_NOSPRITES  4

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
void* archive_readfile(char*,_ULONG*);

extern char debugmode;

extern class ARCHIVE *archive;
extern class ARCHIVE *patch_archive;
extern class ARCHIVE *lang_archive;
extern class ARCHIVE *speech_archive;
extern class ARCHIVE *music_archive;
extern class BAR *bar;
extern class GFX *gfx;
extern class GFXDRIVER *gfxdriver;
extern class MOUSE *mouse;
extern class PROJECT *project;
extern class SCRIPT *rootscript;
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
#ifdef MULTIPLAYER
extern class MPLAYER* mplayer;
#endif

extern void idler();

extern _ULONG main_speed;
extern _UINT ego_object;
extern _ULONG mad_flags;

extern char* my_name;

#endif
