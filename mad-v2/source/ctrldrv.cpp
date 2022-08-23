/*
                            CTRLDRV.CPP

         Low Level Controls Routines for MAD - DOS version

                       (c) 1999 The MAD Crew

  Description:
  This will handle low-level input from all devices.

  Todo: Nothing

  Portability notes: Needs to be fully rewritten for other platforms.
*/
#include <ctype.h>
#include <dos.h>
#include <dpmi.h>
#include <go32.h>
#include <pc.h>
#include <string.h>
#include <sys/segments.h>
#include "controls.h"
#include "ctrldrv.h"
#include "debugmad.h"
#include "maderror.h"
#include "gadgets.h"
#include "gfx.h"
#include "mad.h"
#include "project.h"
#include "types.h"

volatile _UCHAR ctrldrv_keydown[128];

static int keyboard_handler_start(void) { };

keyboard_handler() {
    _UCHAR i;

    i=inportb(0x60);
    if(i>0x7f) {
        ctrldrv_keydown[i-0x80]=0;
    } else {
        ctrldrv_keydown[i]=1;
    }
    outportb(0x20,0x20);
}

static int keyboard_handler_end(void) { };

static _go32_dpmi_seginfo wrapper,old_kbd_vector;

/*
 * CTRLDRV::init()
 *
 * This will initialize the controls driver.
 *
 */
void
CTRLDRV::init() {
    _UCHAR i;
    union REGS r;

    /* first zero out the keydown array */
    memset(&ctrldrv_keydown,0,128);

    /* get the old vector */
    _go32_dpmi_get_protected_mode_interrupt_vector(0x9,&old_kbd_vector);

    /* get a wrapper to wrap to my interrupt handler */
    wrapper.pm_offset = (int)keyboard_handler;
    wrapper.pm_selector=_go32_my_cs();
    _go32_dpmi_allocate_iret_wrapper(&wrapper);

    /* lock our stuff */
    _go32_dpmi_lock_code(keyboard_handler_start,(long)keyboard_handler_end-(long)keyboard_handler_start);
    _go32_dpmi_lock_data((void *)&ctrldrv_keydown,128);

    /* and activate the new one */
    _go32_dpmi_set_protected_mode_interrupt_vector(0x9,&wrapper);

    /* now initialize the mouse */
    memset(&r,0,sizeof(union REGS));
    r.x.ax=0x0;                     /* mouse driver: reset and return status */
    int86(0x33,&r,&r);
    /* check if driver installed */
    if (r.x.ax!=0xffff) {
        /* driver is not installed, set flag mouse driver is not
          installed */
        mouse_installed=0;
    } else {
        /* mouse driver is installed. set flag indicating mouse
           driver is installd */
        mouse_installed=1;
    }
    /* center the mouse with no buttons */
    x=160;y=100;buttonstat=0;
}

/*
 * CTRLDRV::done()
 *
 * This will deinitialize the controls driver.
 *
 */
void
CTRLDRV::done() {
    /* unhook keyboard irq */
    _go32_dpmi_set_protected_mode_interrupt_vector(0x9,&old_kbd_vector);
    _go32_dpmi_free_iret_wrapper(&wrapper);
}

/*
 * CTRLDRV::poll()
 *
 * This will poll the controls driver.
 *
 */
void
CTRLDRV::poll() {
    union REGS r;
    _UCHAR i;

    /* do we have a mouse to poll? */
    if (mouse_installed) {
        /* yes, so poll it */

        memset(&r,0,sizeof(union REGS));
        r.x.ax=0x3;                     /* mouse driver: return position and button status */
        int86(0x33,&r,&r);
        x=r.x.cx>>1;                    /* store x position (must be divided by 2) */
        y=r.x.dx;                       /* store y position */
        buttonstat=r.h.bl;
    }
    /* handle emergency exit [KEEP FREE! :-)] */
    if(ctrldrv_keydown[CONTROLS_KEY_F12]) {
        /* emergency exit. die (error 6: emergency exit) */
        die(MAD_ERROR_6);
    }
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
