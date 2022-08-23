/*
                            CONTROLS.CPP

                Controls Engine for MAD - Windows version

                       (c) 1999 The MAD Crew

  Description:
  This will handle input from all devices.

  Todo: Create a gets() or simular, maybe joystick support?

  Portability notes: Needs to be fully rewritten for other platforms.
*/
#include <windows.h>
#include "controls.h"
#include "debugmad.h"
#include "mad.h"
#include "types.h"

extern HINSTANCE hInstance;
extern HWND hWnd;

extern unsigned char gfx_keydown[256];

void
CONTROLS::init() {
	buttonstat = 0;
	mouse_installed = GetSystemMetrics (SM_MOUSEPRESENT);
	if ((mouse_installed == TRUE) || (mouse_isinstalled != 0)) {
	    mouse_installed = 1;
	} else {
	    mouse_installed = 0;
	}
} 

void
CONTROLS::done() {
}

UCHAR
CONTROLS::poll() {
	POINT p;

	GetCursorPos(&p);
	x=(UINT)p.x;
	y=(UINT)p.y;

    #ifdef DEBUGGING_AVAILABLE
	debug_handlekeys();
    #endif

    idler();	
	return 0;
}

UINT
CONTROLS::getxpos() {
		return x;
}

UINT
CONTROLS::getypos() {
		return y;
}

UCHAR
CONTROLS::button1pressed() {
        return buttonstat & CONTROLS_LEFTBUTTON;
}

UCHAR
CONTROLS::button2pressed() {
        return buttonstat & CONTROLS_RIGHTBUTTON;
}

UCHAR
CONTROLS::mouse_isinstalled() {
		return mouse_installed;
}

void
CONTROLS::waitnobutton() {
        while (button1pressed()||button2pressed()) { poll(); }
}

UCHAR
CONTROLS::kbd_keydown() {
    UCHAR i;

    /* scan the entire keydown[] array for a 1 value */
    for(i=0;i<128;i++) {
        if(gfx_keydown[i]) return 1;
    }
    return 0;
}

UCHAR
CONTROLS::iskeydown(UCHAR keyno) {
    return(gfx_keydown[keyno]);
}

void
CONTROLS::waitevent() {
        UCHAR done;

        done=0;
        while (!done) {
            poll();
            /* some key on the keyboard hit? */
            if (kbd_keydown()) {
                done=1;
            }
            /* did the user click a button? */
            if ((button1pressed())||(button2pressed())) {
                /* yes. wait until he releases it and continue */
                waitnobutton();
                done=1;
            }
        }
}

UCHAR
CONTROLS::getbuttonstatus() {
	 return buttonstat;
}

void
CONTROLS::setbuttonstatus(UCHAR stat) {
	 buttonstat=stat;
}

UCHAR
CONTROLS::left_pressed() {
    UCHAR i;

    i=iskeydown(CONTROLS_KEYLEFT);
    gfx_keydown[CONTROLS_KEYLEFT]=0;

    return i;
}

UCHAR
CONTROLS::right_pressed() {
    UCHAR i;

    i=iskeydown(CONTROLS_KEYRIGHT);
    gfx_keydown[CONTROLS_KEYRIGHT]=0;

    return i;
}

UCHAR
CONTROLS::up_pressed() {
    UCHAR i;

    i=iskeydown(CONTROLS_KEYUP);
    gfx_keydown[CONTROLS_KEYUP]=0;

    return i;
}

UCHAR
CONTROLS::down_pressed() {
    UCHAR i;

    i=iskeydown(CONTROLS_KEYDOWN);
    gfx_keydown[CONTROLS_KEYDOWN]=0;

    return i;
}

UCHAR
CONTROLS::halt_pressed() {
    UCHAR i;

    i=iskeydown(CONTROLS_KEYSTOP);
    gfx_keydown[CONTROLS_KEYSTOP]=0;

    return i;
}