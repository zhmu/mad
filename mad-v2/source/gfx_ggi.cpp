/*
				GFX_GGI.CPP

			GGI Graphix Engine for MAD

	    	   	  (c) 1999 The MAD Crew

  Description:
  Will handle actual graphics functions.

  Todo: Nothing.

  Portability notes: Needs to be fully rewritten for other platforms.
*/
#include <ggi/ggi.h>
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
#define MAD_ERROR_701 "Error 701: Unable to open GGI"
#define MAD_ERROR_702 "Error 702: Unable to set GGI mode"

#define ZOOM 2 /* Zoom level */

ggi_visual_t ggiVisual;
ggi_pixel palette_color[256];

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
 * This will initialize the GGI graphics driver.
 *
 */
void
GFXDRIVER::init() {
  ggi_mode mode = { 1,                                       /* number of frames */
                    { GFX_DRIVER_HRES * ZOOM, GFX_DRIVER_VRES * ZOOM },    /* resolution */
                    { GFX_DRIVER_HRES * ZOOM, GFX_DRIVER_VRES * ZOOM },    /* virtual */
                    { 0, 0 },                                /* size in mm */
                    GT_AUTO,                                 /* color depth */
                    { GGI_AUTO, GGI_AUTO } };                /* font size */
  ggi_visual_t retval;

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

  /* initialize the ggi stuff */
  ggiInit();

  /* open the display */
  if (!(retval = ggiOpen (NULL))) {
    /* this failed. die (error 701: unable to open ggi) */
    die(MAD_ERROR_701);
  }

  /* set the mode */
  if (ggiSetMode (retval, &mode)) {
    if (ggiSetMode (retval, &mode)) {
      /* unable to set mode. die (error 702: unable to set ggi mode) */
      die(MAD_ERROR_702);
    }
  }

#if 0
  /* mask out the events we want */
  ggiSetEventMask (retval, emKey | emPointer);
#endif

  /* get a visual */
  ggiVisual = retval;
}

/*
 * GFXDRIVER::done()
 *
 * This will deinitialize the GGI graphics driver.
 */
void
GFXDRIVER::done() {
  /* close the visual */
  ggiClose (ggiVisual);
  /* and exit ggi */
  ggiExit ();
}

/*
 * GFXDRIVER::draw(void* data)
 *
 * This will draw data [data].
 *
 */
void
GFXDRIVER::draw(void *data) {
    int i,j;
    _ULONG tempbuf[GFX_DRIVER_HRES * ZOOM];
    char col;
    char* ptr;

    ptr=(char*)data;

    for (i=0;i<GFX_DRIVER_VRES;i++) {
        for(j=0;j<GFX_DRIVER_HRES;j++) {
            col=ptr[GFX_DRIVER_HRES*i+j];
	    tempbuf[j*ZOOM]=palette_color[col];
	    tempbuf[j*ZOOM+1]=palette_color[col];
	}
        ggiPutHLine (ggiVisual, 0, i*ZOOM, GFX_DRIVER_HRES * ZOOM, tempbuf);
        ggiPutHLine (ggiVisual, 0, i*ZOOM+1, GFX_DRIVER_HRES * ZOOM, tempbuf);
    }
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
  ggi_color color;
  char pal;
  ggi_mode mode;

  ggiGetMode (ggiVisual, &mode);
  pal=(mode.graphtype&GT_PALETTE);

  for(i=0;i<256;i++) {
      color.r = ptr[i*3];
      color.g = ptr[i*3+1];
      color.b = ptr[i*3+2];

      if (pal) {
	  palette_color[i]=i;
	  ggiSetPalette (ggiVisual, i, 1, &color);
      } else {
          palette_color[i] = ggiMapColor (ggiVisual, &color);
      }
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
}
