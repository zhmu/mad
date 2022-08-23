/*
		CTRL_GII.CPP

	    (c) 1999 The MAD Crew

	Controls Engine for MAD - GII version

    Description:
    This will handle input from all devices.

    Todo: write everything

    Portability notes: needs to be fully rewritten for other platforms.
*/
#include <ctype.h>
#include <ggi/ggi.h>
#include <string.h>
#include "controls.h"
#include "ctrldrv.h"
#include "debugmad.h"
#include "gfx_drv.h"
#include "mad.h"
#include "maderror.h"
#include "types.h"

unsigned char ctrldrv_keydown[128];
int x_mouse_x,x_mouse_y;
char x_mouse_buttonstat;
extern ggi_visual_t ggiVisual;

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

_UCHAR
resolve_ggi_keycode(ggi_event *ev) {
    switch (tolower (ev->key.label)) {
	case GIIUC_a: return CONTROLS_KEY_A;
        case GIIUC_b: return CONTROLS_KEY_B;
        case GIIUC_c: return CONTROLS_KEY_C;
        case GIIUC_d: return CONTROLS_KEY_D;
        case GIIUC_e: return CONTROLS_KEY_E;
        case GIIUC_f: return CONTROLS_KEY_F;
        case GIIUC_g: return CONTROLS_KEY_G;
        case GIIUC_h: return CONTROLS_KEY_H;
        case GIIUC_i: return CONTROLS_KEY_I;
        case GIIUC_j: return CONTROLS_KEY_J;
        case GIIUC_k: return CONTROLS_KEY_K;
        case GIIUC_l: return CONTROLS_KEY_L;
        case GIIUC_m: return CONTROLS_KEY_M;
        case GIIUC_n: return CONTROLS_KEY_N;
        case GIIUC_o: return CONTROLS_KEY_O;
        case GIIUC_p: return CONTROLS_KEY_P;
        case GIIUC_q: return CONTROLS_KEY_Q;
        case GIIUC_r: return CONTROLS_KEY_R;
        case GIIUC_s: return CONTROLS_KEY_S;
        case GIIUC_t: return CONTROLS_KEY_T;
        case GIIUC_u: return CONTROLS_KEY_U;
        case GIIUC_v: return CONTROLS_KEY_V;
        case GIIUC_w: return CONTROLS_KEY_W;
        case GIIUC_x: return CONTROLS_KEY_X;
        case GIIUC_y: return CONTROLS_KEY_Y;
        case GIIUC_z: return CONTROLS_KEY_Z;
    case GIIUC_Tilde: return CONTROLS_KEY_TILDE;
    case GIIUC_Grave: return CONTROLS_KEY_TILDE;
case GIIUC_BackSpace: return CONTROLS_KEY_BSPACE;
   case GIIUC_Escape: return CONTROLS_KEY_ESCAPE;
	case GIIK_F1: return CONTROLS_KEY_F1;
	case GIIK_F2: return CONTROLS_KEY_F2;
	case GIIK_F3: return CONTROLS_KEY_F3;
	case GIIK_F4: return CONTROLS_KEY_F4;
	case GIIK_F5: return CONTROLS_KEY_F5;
	case GIIK_F6: return CONTROLS_KEY_F6;
	case GIIK_F7: return CONTROLS_KEY_F7;
	case GIIK_F8: return CONTROLS_KEY_F8;
	case GIIK_F9: return CONTROLS_KEY_F9;
       case GIIK_F10: return CONTROLS_KEY_F10;
       case GIIK_F11: return CONTROLS_KEY_F11;
       case GIIK_F12: return CONTROLS_KEY_F12;
     case GIIK_Enter: return CONTROLS_KEY_ENTER;
    case GIIK_Delete: return CONTROLS_KEY_DELETE;
        case GIIK_Up: return CONTROLS_KEYUP;
      case GIIK_Down: return CONTROLS_KEYDOWN;
      case GIIK_Left: return CONTROLS_KEYLEFT;
     case GIIK_Right: return CONTROLS_KEYRIGHT;
        case GIIK_P5: return CONTROLS_KEYSTOP;
     case GIIK_PPlus: return CONTROLS_KEY_KPADD;
    case GIIK_PMinus: return CONTROLS_KEY_KPSUB;
      case GIIK_PTab: return CONTROLS_KEY_TAB;
    case GIIUC_Space: return CONTROLS_KEY_SPACE;
    }
    /* unknown. return zero */
    return 0;
}

void
CTRLDRV::poll() {
    struct timeval temptime = { 0, 0 };
    ggi_event event;
    _UCHAR i;

    while (ggiEventPoll (ggiVisual, emAll, &temptime)) {
	ggiEventRead (ggiVisual, &event, emAll);

        switch (event.any.type) {
	 case evPtrButtonPress: /* mouse button press */
				if (event.pbutton.button == GII_PBUTTON_LEFT) {
				    x_mouse_buttonstat |= CONTROLS_BUTTON1;
				}
				if (event.pbutton.button == GII_PBUTTON_RIGHT) {
				    x_mouse_buttonstat |= CONTROLS_BUTTON2;
				}
				break;
       case evPtrButtonRelease: /* mouse button release */
				if (event.pbutton.button == GII_PBUTTON_LEFT) {
				    x_mouse_buttonstat &= ~CONTROLS_BUTTON1;
				}
				if (event.pbutton.button == GII_PBUTTON_RIGHT) {
				    x_mouse_buttonstat &= ~CONTROLS_BUTTON2;
				}
				break;
	    case evPtrAbsolute: /* mouse move */
				x_mouse_x = event.pmove.x;
				x_mouse_y = event.pmove.y;
				break;
	       case evKeyPress:
	      case evKeyRepeat: i = resolve_ggi_keycode (&event);
				if (i) {
    				    ctrldrv_keydown[i] = 1;
				}
				break;
	     case evKeyRelease: i = resolve_ggi_keycode (&event);
				if (i) {
    				    ctrldrv_keydown[i] = 0;
				}
				break;
	}
    }
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
