/*
				GFX_X.cpp

			VGL Graphix Engine for MAD

	    	   	  (c) 1999 The MAD Crew

  Description:
  Will handle actual graphics functions. It will only work on FreeBSD's VGL
  library. It also requires MAD to be installed with permissions 4111.

  Todo: Implement keyboard input.

  Portability notes: Needs to be fully rewritten for other platforms.
*/
#include <machine/console.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include "hacked_vgl.h"
#include "controls.h"
#include "ctrldrv.h"
#include "gfx_drv.h"
#include "gfx.h"
#include "mad.h"
#include "maderror.h"

#define MAD_ERROR_700 "Error 700: Caught signal %u"
#define MAD_ERROR_701 "Error 701: Unable to initialize VGL"
#define MAD_ERROR_702 "Error 702: Unable to initialize mouse -- load moused"

VGLBitmap bitmap;
int vgl_inited;

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
 * GFXDRIVER::init()
 *
 * This will initialize the vgl graphics driver.
 *
 */
void
GFXDRIVER::init() {
  char tempstr[MAD_TEMPSTR_SIZE];

  /* set some flags */
  vgl_inited=0;

  /* hook most signals */
  signal (SIGHUP, signal_handler);
  signal (SIGINT, signal_handler);
  signal (SIGQUIT, signal_handler);
  signal (SIGILL, signal_handler);
  signal (SIGTRAP, signal_handler);
  signal (SIGABRT, signal_handler);
  signal (SIGEMT, signal_handler);
  signal (SIGFPE, signal_handler);
  signal (SIGKILL, signal_handler);
  signal (SIGBUS, signal_handler);
  signal (SIGSEGV, signal_handler);
  signal (SIGSYS, signal_handler);
  signal (SIGPIPE, signal_handler);
  signal (SIGALRM, signal_handler);
  signal (SIGTERM, signal_handler);
  signal (SIGXCPU, signal_handler);
  signal (SIGXFSZ, signal_handler);

  /* init vgl */
  if (VGLInit(SW_VGA_CG320)) {
    /* this failed. die (error 701: unable to initialize vgl) */
    die(MAD_ERROR_701);
  }
  vgl_inited=1;

  /* create the bitmap */
  bitmap.Type = MEMBUF;
  bitmap.Bitmap = (unsigned char*)malloc (GFX_DRIVER_HRES * GFX_DRIVER_VRES);
  bitmap.Xsize = GFX_DRIVER_HRES; bitmap.Ysize = GFX_DRIVER_VRES;
  if (bitmap.Bitmap == NULL) {
    /* this failed. die (error 2: out of memory (allocate of bytes failed)) */
    sprintf(tempstr,MAD_ERROR_2,(GFX_DRIVER_HRES*GFX_DRIVER_VRES));
    die(tempstr);
  }
 
  /* initialize vgl */
  if (VGLMouseInit (VGL_MOUSEHIDE)) {
    /* this failed. die (error 702: unable to initialize mouse -- run moused) */
    die(MAD_ERROR_702);
  }
}

/*
 * GFXDRIVER::done()
 *
 * This will deinitialize the XWindows graphics driver.
 */
void
GFXDRIVER::done() {
  /* was vgl initialized? */
  if(vgl_inited) {
    /* yes, get rid of it */
    VGLEnd();
    /* make sure we don't do that twice */
    vgl_inited=0;

    /* was there bitmap data? */
    if (bitmap.Bitmap != NULL) {
      /* yes. free it */
      free(bitmap.Bitmap);
      /* make sure we don't do that twice */
      bitmap.Bitmap=NULL;
    }
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
  memcpy(bitmap.Bitmap,data,GFX_DRIVER_HRES*GFX_DRIVER_VRES);

  VGLBitmapCopy(&bitmap,0, 0, VGLDisplay, 0, 0, GFX_DRIVER_HRES, GFX_DRIVER_VRES);
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
    VGLSetPaletteIndex (i, ptr[i*3],ptr[i*3+1],ptr[i*3+2]);
  }
}

/*
 * GFXDRIVER::poll()
 *
 * This will poll the graphics driver.
 *
 */
void
GFXDRIVER::poll() {
    VGLMouseStatus (&x_mouse_x,&x_mouse_y,&x_mouse_buttonstat);
}
