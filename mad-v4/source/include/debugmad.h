#include "dlgman.h"
#include "mad.h"
#include "types.h"

#ifdef DEBUG_AVAILABLE
#define DEBUG_MAX_BREAKPOINTS 8

#define DEBUG_GFXMODE_BACKSCR 0
#define DEBUG_GFXMODE_MASKSCR 1
#define DEBUG_GFXMODE_PRIOSCR 2

#define DEBUG_BREAKPOINTCOLOR 4         /* red */
#define DEBUG_NORMALCOLOR     DLGMAN_DIALOG_TITLECOLOR  /* normal control color */
#define DEBUG_WINDOW_NOF_OBJS 8

void debug_init();
void debug_done();
void debug_launchdebugger(SCRIPT*);
void debug_handlekeys();
void debug_setdebugtext(char*);
void debug_setregstext(char*);
void debug_setobjinfotext(char*);

extern _UCHAR debug_gfxmode;
extern _UCHAR debug_active;
extern _UCHAR debug_step;
#endif
