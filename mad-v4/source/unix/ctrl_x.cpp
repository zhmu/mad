/*
		CTRL_X.CPP

	Controls Engine for MAD - X Windows version
	This one requires a valid graphics driver which takes care of it.

	    (c) 1999, 2000 The MAD Crew

    Description:
    This will handle input from all devices.

    Todo: write everything

    Portability notes: needs to be fully rewritten for other platforms.
*/
#include <string.h>
#include "controls.h"
#include "ctrldrv.h"
#include "debugmad.h"
#include "gfx_drv.h"
#include "mad.h"
#include "maderror.h"
#include "types.h"

#ifdef UNIX
  #include <ctype.h>
#endif /* UNIX */

unsigned char ctrldrv_keydown[128];
int x_mouse_x,x_mouse_y;
char x_mouse_buttonstat;

void
emergency_exit() {
  /* die (error6: emergency exit) */
  die(MAD_ERROR_6);
}

void
CTRLDRV::init() {
    /* make sure there's no data in the ctrldrv_keydown[] buffer */
    memset(ctrldrv_keydown,0,sizeof(ctrldrv_keydown));
    x_mouse_x=0; x_mouse_y=0; x_mouse_buttonstat=0;
}

void
CTRLDRV::done() {
}

void
CTRLDRV::poll() {
  gfxdriver->poll();
}

_UINT
CTRLDRV::getxpos() {
  return x_mouse_x;
}

_UINT
CTRLDRV::getypos() {
  return x_mouse_y;
}

_UCHAR
CTRLDRV::getbuttonstat() {
  return x_mouse_buttonstat;
}
