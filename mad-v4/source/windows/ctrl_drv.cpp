/*
                            CTRL_DRV.CPP

                Controls Engine for MAD - Windows version
 	This one requires a valid controls DLL or a graphics driver which takes care of it.

                        (c) 1999 The MAD Crew

  Description:
  This will handle input from all devices.

  Todo: Implement this driver as well for controls driver DLL's.

  Portability notes: Needs to be fully rewrittien for other platforms.
*/
#include <windows.h>
#include "controls.h"
#include "ctrldrv.h"
#include "debugmad.h"
#include "gfx.h"
#include "gfx_drv.h"
#include "mad.h"
#include "maderror.h"
#include "project.h"
#include "types.h"

_UCHAR ctrldrv_keydown[128];

void                                        
emergency_exit() {
    /* die (error 6: emergency exit) */
	die(MAD_ERROR_6);
}

/*
 * CTRLDRV::init()
 *
 * This will initialize the controls driver.
 *
 */
void
CTRLDRV::init() {
	/* Does this driver do keyboard support? */
	if (gfxdriver->get_caps() & GFX_DRIVER_CAPS_ALSOKBD) {
	    /* Yeah, initialize the emergency exit procedure to call */
        gfxdriver->set_quit_handler((void*)&emergency_exit);
	} else {
	    /* ...todo: create controls driver support */
	}
}

/*
 * CTRLDRV::done()
 *
 * This will deinitialize the controls driver.
 *
 */
void
CTRLDRV::done() {
	if (gfxdriver->get_caps() & GFX_DRIVER_CAPS_ALSOKBD) {
   	    /* Nothing to deinitialize, since the graphics driver does all controls */
	} else {
	    /* ...todo: create controls driver support */
	}
}

/*
 * CTRLDRV::poll()
 *
 * This will poll the controls driver.
 *
 */
void
CTRLDRV::poll() {
    /* First let the graphics engine do it's polling */
	gfxdriver->poll();
	
	/* Does this driver also do keyboard polling? */
	if (gfxdriver->get_caps() & GFX_DRIVER_CAPS_ALSOKBD) {
		/* Yeah, figure out the status */
		gfxdriver->get_keyboard_status(ctrldrv_keydown);
	} else {
	    /* ...todo: create keyboard controls driver support */
	}

	/* This this driver also do aiming device polling? */
	if (gfxdriver->get_caps() & GFX_DRIVER_CAPS_ALSODEV) {
	    /* Yeah, let it poll it */
		_UINT i;

		gfxdriver->get_device_stat(&x, &y, &i);
		buttonstat = i & 0xff;			/* [buttonstat] is lo byte of driver's button status */
	} else {
	    /* ...todo: create aiming controls driver support */
	}

    #ifdef DEBUGGING_AVAILABLE
	debug_handlekeys();
    #endif
}

/*
 * CTRLDRV::getxpos()
 *
 * This will return the X coordinate of the controls device.
 *
 */
_UINT
CTRLDRV::getxpos() {
	return x;
}

/*
 * CTRLDRV::getypos()
 *
 * This will return the Y coordinate of the controls device.
 *
 */
_UINT
CTRLDRV::getypos() {
	return y;
}

/*
 * CTRLDRV::getbuttonstat()
 *
 * This will return the button state of the aiming device.
 *
 */
_UCHAR
CTRLDRV::getbuttonstat() {
    return buttonstat;
}
