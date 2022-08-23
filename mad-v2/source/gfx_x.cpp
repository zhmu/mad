/*
				GFX_X.cpp

			X Graphix Engine for MAD

	    	   	  (c) 1999 The MAD Crew

  Description:
  Will handle actual graphics functions.

  Todo: Make it properly working

  Portability notes: Needs to be fully rewritten for other platforms.
*/
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/extensions/XShm.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include "controls.h"
#include "ctrldrv.h"
#include "gfx_drv.h"
#include "gfx.h"
#include "mad.h"
#include "maderror.h"

#define MAD_WINDOW_TITLE "MAD"
#define MAD_ICON_TITLE MAD_WINDOW_TITLE

#define MAD_ERROR_700 "Error 700: Caught signal %u"
#define MAD_ERROR_701 "Error 701: Unable to open display"
#define MAD_ERROR_702 "Error 702: Unable to create window"
#define MAD_ERROR_703 "Error 703: Unable to create image"
#define MAD_ERROR_704 "Error 704: Unable to allocate colors"

struct RGB {
    _UCHAR red;
    _UCHAR green;
    _UCHAR blue;
    _UCHAR filler;
};

Display* display;
Visual* visual;
Window window;
int screen;
GC gc;
XImage* ximage;
Colormap colormap;
XColor color[GFX_DRIVER_NOFCOLS];
struct RGB rgb_palette[GFX_DRIVER_NOFCOLS];

int depth;

/*
 * signal_handler(int i)
 *
 * This will catch signals and handle them.
 *
 */
void
signal_handler(int i) {
    char tempstr[MAD_TEMPSTR_SIZE];

    /* die (error 700: caught signal) */
    sprintf(tempstr,MAD_ERROR_700,i);
    die(tempstr);
}

/*
 * NukeCursor()
 *
 * This will nuke the mouse cursor. Code based on LinuxDOOM by ID Software.
 *
 */
void
NukeCursor() {
    Pixmap cursormask;
    XGCValues xgc;
    GC gc;
    XColor dummycolour;
    Cursor cursor;

    /* first, create a cursor mask */
    cursormask = XCreatePixmap (display, DefaultRootWindow (display), 1, 1, 1);
    /* build a gc to it */
    xgc.function = GXclear;
    gc =  XCreateGC(display, cursormask, GCFunction, &xgc);
    /* clean the cursormask */
    XFillRectangle (display, cursormask, gc, 0, 0, 1, 1);
    dummycolour.pixel = 0;
    dummycolour.red = 0;
    dummycolour.flags = 0;
    /* create the pixmap of the cursor */
    cursor = XCreatePixmapCursor (display, cursormask, cursormask,
                                  &dummycolour, &dummycolour, 0, 0);
    /* get rid of the cursormask */
    XFreePixmap (display, cursormask);
    XFreeGC (display, gc);

    /* and select the new cursor */
    XDefineCursor (display, window, cursor);
}

int
mapKeycode(int key)
{
    switch(key) {
	case XK_0: return CONTROLS_KEY_0;
	case XK_1: return CONTROLS_KEY_1;
	case XK_2: return CONTROLS_KEY_2;
	case XK_3: return CONTROLS_KEY_3;
	case XK_4: return CONTROLS_KEY_4;
	case XK_5: return CONTROLS_KEY_5;
	case XK_6: return CONTROLS_KEY_6;
	case XK_7: return CONTROLS_KEY_7;
	case XK_8: return CONTROLS_KEY_8;
	case XK_9: return CONTROLS_KEY_9;
	case XK_a:
	case XK_A: return CONTROLS_KEY_A;
	case XK_b:
	case XK_B: return CONTROLS_KEY_B;
	case XK_c:
	case XK_C: return CONTROLS_KEY_C;
	case XK_d:
	case XK_D: return CONTROLS_KEY_D;
	case XK_e:
	case XK_E: return CONTROLS_KEY_E;
	case XK_f:
	case XK_F: return CONTROLS_KEY_F;
	case XK_g:
	case XK_G: return CONTROLS_KEY_G;
	case XK_h:
	case XK_H: return CONTROLS_KEY_H;
	case XK_i:
	case XK_I: return CONTROLS_KEY_I;
	case XK_j:
	case XK_J: return CONTROLS_KEY_J;
	case XK_k:
	case XK_K: return CONTROLS_KEY_K;
	case XK_l:
	case XK_L: return CONTROLS_KEY_L;
	case XK_m:
	case XK_M: return CONTROLS_KEY_M;
	case XK_n:
	case XK_N: return CONTROLS_KEY_N;
	case XK_o:
	case XK_O: return CONTROLS_KEY_O;
	case XK_p:
	case XK_P: return CONTROLS_KEY_P;
	case XK_q:
	case XK_Q: return CONTROLS_KEY_Q;
	case XK_r:
	case XK_R: return CONTROLS_KEY_R;
	case XK_s:
	case XK_S: return CONTROLS_KEY_S;
	case XK_t:
	case XK_T: return CONTROLS_KEY_T;
	case XK_u:
	case XK_U: return CONTROLS_KEY_U;
	case XK_v:
	case XK_V: return CONTROLS_KEY_V;
	case XK_w:
	case XK_W: return CONTROLS_KEY_W;
	case XK_x:
	case XK_X: return CONTROLS_KEY_X;
	case XK_y:
	case XK_Y: return CONTROLS_KEY_Y;
	case XK_z:
	case XK_Z: return CONTROLS_KEY_Z;
	case XK_grave: return CONTROLS_KEY_TILDE;
	case XK_BackSpace: return CONTROLS_KEY_BSPACE;
	case XK_Tab: return CONTROLS_KEY_TAB;
	case XK_Escape: return CONTROLS_KEY_ESCAPE;
	case XK_Left: return CONTROLS_KEYLEFT;
	case XK_Right: return CONTROLS_KEYRIGHT;
	case XK_Up: return CONTROLS_KEYUP;
	case XK_Down: return CONTROLS_KEYDOWN;
	case XK_plus: return CONTROLS_KEY_EQUAL;
	case XK_minus: return CONTROLS_KEY_MINUS;
	case XK_bracketleft: return CONTROLS_KEY_LBRACKET;
	case XK_bracketright: return CONTROLS_KEY_RBRACKET;
	case XK_equal: return CONTROLS_KEY_EQUAL;
	case XK_Return: return CONTROLS_KEY_ENTER;
	case XK_colon: return CONTROLS_KEY_COLON;
	case XK_semicolon: return CONTROLS_KEY_COLON;
	case XK_comma: return CONTROLS_KEY_COMMA;
	case XK_period: return CONTROLS_KEY_DOT;
	case XK_slash: return CONTROLS_KEY_SLASH;
	case XK_Alt_R:
	case XK_Alt_L: return CONTROLS_KEY_ALT;
    }
    return 0;
}

/*
 * GFXDRIVER::init()
 *
 * This will initialize the XWindows graphics driver.
 *
 */
void
GFXDRIVER::init() {
  char* apptext = MAD_WINDOW_TITLE;
  char* icontext = MAD_ICON_TITLE;
  XWMHints hints;
  XSizeHints sizehints;
  XClassHint classhint;
  XTextProperty appname, iconame;
  XSetWindowAttributes attributes;
  char tempstr[MAD_TEMPSTR_SIZE];

  /* make sure the done() routine won't mess things up */
  display=NULL; window=NULL; ximage=NULL;

  /* hook most signals */
  signal (SIGHUP, signal_handler);
  signal (SIGINT, signal_handler);
  signal (SIGQUIT, signal_handler);
  signal (SIGILL, signal_handler);
  signal (SIGTRAP, signal_handler);
  signal (SIGABRT, signal_handler);
  #ifdef FREEBSD
  signal (SIGEMT, signal_handler);
  signal (SIGSYS, signal_handler);
  #endif
  signal (SIGFPE, signal_handler);
  signal (SIGKILL, signal_handler);
  signal (SIGBUS, signal_handler);
  signal (SIGSEGV, signal_handler);
  signal (SIGPIPE, signal_handler);
  signal (SIGALRM, signal_handler);
  signal (SIGTERM, signal_handler);
  signal (SIGXCPU, signal_handler);
  signal (SIGXFSZ, signal_handler);

  /* open the x display */
  if ((display = XOpenDisplay (NULL)) == NULL) {
    /* this failed. die (error 701: unable to open display) */
    die(MAD_ERROR_701);
  }

  /* get the screen */
  screen = DefaultScreen (display);
  visual = DefaultVisual (display, screen);
  depth = DefaultDepth (display, screen);
  colormap = DefaultColormap (display, screen);

  /* now, let's make a window */
  attributes.event_mask = KeyPressMask | KeyReleaseMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask;
  window = XCreateWindow (display, DefaultRootWindow (display), 0, 0,
                          GFX_DRIVER_HRES, GFX_DRIVER_VRES, 1, depth,
                          InputOutput, CopyFromParent, CWEventMask,
                          &attributes);
  if (!window) {
    /* this failed. die (error 702: unable to create window */
    die(MAD_ERROR_702);
  }

  /* create a color map */
  // colormap = XCreateColormap (display, window, visual, AllocAll);

  /* activate it */
  // XSetWindowColormap (display, window, colormap);

  /* set up the window */
  XStringListToTextProperty (&apptext, 1, &appname);
  XStringListToTextProperty (&icontext, 1, &iconame);

  sizehints.flags = PSize | PMinSize | PMaxSize;
  sizehints.min_width = GFX_DRIVER_HRES;
  sizehints.min_height = GFX_DRIVER_VRES;
  sizehints.max_width = GFX_DRIVER_HRES;
  sizehints.max_height = GFX_DRIVER_VRES;

  hints.flags = StateHint | IconPixmapHint | InputHint;
  hints.initial_state = NormalState;
  hints.input = 1;

  classhint.res_name = "MAD";
  classhint.res_class = "MAD";

  XSetWMProperties (display, window, &appname, &iconame, NULL, NULL,
                    &sizehints, &hints, &classhint);
  
  /* create a graphics context */
  gc = XCreateGC (display, window, 0, NULL);

  /* make an image */
  ximage = XCreateImage (display, visual, depth, ZPixmap, 0, NULL,
                         GFX_DRIVER_HRES, GFX_DRIVER_VRES, 8, 0);
  if (ximage == NULL) {
    /* this failed. die (error 703: unable to create image */
    die(MAD_ERROR_703);
  }

  /* allocate memory */
  if((ximage->data=(char*)malloc(ximage->bytes_per_line*ximage->height))==NULL) {
    /* this failed. die (error 2: out of memory (allocation of bytes failed)) */    sprintf(tempstr,MAD_ERROR_2,ximage->bytes_per_line*ximage->height);
    die(tempstr);
  }

  /* make sure the window will be shown */
  XMapWindow (display, window);
  XSetWindowBackground (display, window, BlackPixel (display, screen));
  XClearWindow (display, window);
  XSync (display, False);

  /* now we need to nuke the mouse cursor */
  NukeCursor();
}

/*
 * GFXDRIVER::done()
 *
 * This will deinitialize the XWindows graphics driver.
 */
void
GFXDRIVER::done() {
  /* is there an image? */
  if(ximage!=NULL) {
    /* yes. is there also image data? */
    if(ximage->data!=NULL) {
      /* yes, free it */
      free(ximage->data);
      /* make sure we don't do this twice */
      ximage->data=NULL;
    }
    /* nuke the image */
    XDestroyImage (ximage);
    /* make sure we don't do this twice */
    ximage=NULL;
  }
  /* do we have a window? */
  if(!window) {
    /* yes, close it */
    XDestroyWindow (display, window);
    /* make sure we don't do this twice */
    window=NULL;
  }

  /* do we have a display? */
  if(display!=NULL) {
    /* yes, free it */
    XCloseDisplay (display);
    /* make sure this won't be done twice */
    display=NULL;
  }
}

/*
 * GFXDRIVER::draw(void* data)
 *
 * This will draw data [data].
 *
 */
void
GFXDRIVER::draw(void *data) {
  long i;
  long count;
  _UCHAR col;
  char* ptr;

  count=0;ptr=(char*)data;
  for(i=0;i<(GFX_DRIVER_HRES*GFX_DRIVER_VRES);i++) {
    //col=(char*)*(ptr+i);
    col=ptr[i];
    switch (depth) {
	 case 8: /* 8 bit colors */
		 *(_UCHAR*)(ximage->data + count) = /*rgb_palette[col].red;*/col;
		 count++;
	  	 break;
        case 16: /* 16 bit colors */
		 *(_UCHAR*)(ximage->data + count) = rgb_palette[col].red;
		 *(_UCHAR*)(ximage->data + count + 1) = rgb_palette[col].green;
		 count+=2;
	  	 break;
        case 24: /* 24 bit colors */
	case 32: /* 32 bit colors */
		 *(_UCHAR*)(ximage->data + count) = rgb_palette[col].red;
		 *(_UCHAR*)(ximage->data + count + 1) = rgb_palette[col].green;
		 *(_UCHAR*)(ximage->data + count + 2) = rgb_palette[col].blue;
		 *(_UCHAR*)(ximage->data + count + 3) = rgb_palette[col].filler;

		 /* increment the counter */
		 count+=4;
		 break;
    }
  }

  /* draw it */
  XPutImage (display, window, gc, ximage, 0, 0, 0, 0,
             GFX_DRIVER_HRES, GFX_DRIVER_VRES);

  /* sync the display */
  XSync (display, False);
}

/*
 * GFXDRIVER::get_hres()
 *
 * This will return the horizontal resolution.
 *
 */
_UINT
GFXDRIVER::get_hres() {
  return GFX_DRIVER_HRES;
}

/*
 * GFXDRIVER::get_vres()
 *
 * This will return the vertical resolution.
 *
 */
_UINT
GFXDRIVER::get_vres() {
  return GFX_DRIVER_VRES;
}

/*
 * GFXDRIVER::get_nofcols()
 *
 * This will return the number of colors supported.
 *
 */
_UINT
GFXDRIVER::get_nofcols() {
  return GFX_DRIVER_NOFCOLS;
}

/*
 * GFXDRIVER::setpalette(char* ptr)
 *
 * This will set the palette to [ptr].
 *
 */
void
GFXDRIVER::setpalette(char *ptr) {
  int i;

  for(i=0;i<GFX_DRIVER_NOFCOLS;i++) {
    color[i].red = ptr[i * 3] >> 2;
    color[i].green = ptr[i * 3 + 1] >> 2;
    color[i].blue = ptr[i * 3 + 2] >> 2;
    if (!XAllocColor (display, colormap, &color[i])) {
      /* this failed. die (error 704: unable to allocate colors) */
      die(MAD_ERROR_704);
     }
    switch (depth) {
       default:
        case 8: /* 8 bit colors */
                rgb_palette[i].red = color[i].pixel;
		break;
       case 16: /* 16 bit colors */
                rgb_palette[i].red = color[i].pixel & 0xff;
                rgb_palette[i].green = color[i].pixel >> 8;
                break;
       case 24: /* 24 bit colors */
       case 32: /* 32 bit colors */
                rgb_palette[i].red = color[i].pixel & 0xff;
                rgb_palette[i].green = color[i].pixel >> 8;
                rgb_palette[i].blue = color[i].pixel >> 16;
		break;
    }
  }
  // XStoreColors(display, colormap, color, 256);
}

/*
 * GFXDRIVER::poll()
 *
 * This will poll the graphics driver.
 *
 */
void
GFXDRIVER::poll() {
  XEvent ev;
  XMotionEvent* mev;
  XButtonEvent* bev;
  XKeyEvent* kev;
  KeySym ks;

  while (XEventsQueued (display, QueuedAfterReading)) {
    XNextEvent (display, &ev);

    /* take care of it */
    switch(ev.type) {
      case MotionNotify: /* the mouse has moved */
                         mev=(XMotionEvent*)&ev;

			 x_mouse_x=mev->x;
			 x_mouse_y=mev->y;
			 break;
       case ButtonPress: /* a mouse button has been pressed */
                         bev=(XButtonEvent*)&ev;

		         if (bev->button&Button1) {
			   /* enable it in the [x_mouse_buttonstat] */
			   x_mouse_buttonstat|=CONTROLS_BUTTON1;
			 }
		         if (bev->button&Button2) {
			   /* enable it in the [x_mouse_buttonstat] */
			   x_mouse_buttonstat|=CONTROLS_BUTTON2;
			 }
  		         break;
     case ButtonRelease: /* a mouse button has been released */
                         bev=(XButtonEvent*)&ev;

		         if (bev->button&Button1) {
			   /* disable it in the [x_mouse_buttonstat] */
			   x_mouse_buttonstat&=~CONTROLS_BUTTON1;
			 }
		         if (bev->button&Button2) {
			   /* disable it in the [x_mouse_buttonstat] */
			   x_mouse_buttonstat&=~CONTROLS_BUTTON2;
			 }
			 break;
  	  case KeyPress: /* a keyboard button has been pressed */
                         kev=(XKeyEvent*)&ev;
			 ks = XKeycodeToKeysym(display, kev->keycode, 0);
			 //printf ("KEYPRESS: %x => %x\n",kev->keycode, ks);
			 ctrldrv_keydown[mapKeycode(ks)]=1;

			if (ks==XK_F12) {
                            /* die (error6: emergency exit) */
			    die(MAD_ERROR_6);
			}
  		         break;
  	case KeyRelease: /* a keyboard button has been released */
                         kev=(XKeyEvent*)&ev;
			 ks = XKeycodeToKeysym(display, kev->keycode, 0);
			//printf ("KEYRELEASE: %x => %x\n",kev->keycode,ks);

			 ctrldrv_keydown[mapKeycode(ks)]=0;
  		         break;
    }
  }
}
