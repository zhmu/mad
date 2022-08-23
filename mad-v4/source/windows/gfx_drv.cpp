/*
                             GFX_DRV.CPP

              Graphix Engine for MAD - Windows Version
  		    This one requires a valid graphics driver DLL

                        (c) 1999 The MAD Crew

  Description:
  This will perform generic graphics. It will also do the controls if the driver wants us to.

  Todo: Nothing

  Portability notes: Needs to be fully rewritten for other platforms.
*/
#define __NOINCLUDES__				/* hack for mad.h to stop including types.h */

#include <stdio.h>
#include <windows.h>
#include "gfx_drv.h"
#include "mad.h"
#include "maderror.h"

#undef  BUILDER_DLL

#ifdef BUILDER_DLL
#define DLL_FUNCNAME_PRE "_"
#else
#define DLL_FUNCNAME_PRE ""
#endif
                    
typedef UINT (*GFXFUNC)();

typedef UINT (*GFXDRAW)(void *buf); 
typedef void (*GFXPOLL)(); 
typedef char *(*GFXRESOLVERROR)(UINT no);

typedef void (*GFXGETKBDSTAT)(void *dest); 
typedef void (*GFXGETDEVSTAT)(UINT *x,UINT *y,UINT *stat); 

GFXFUNC gfx_init, gfx_done, gfx_getcaps;
GFXFUNC get_driver_type;
GFXDRAW gfx_draw,gfx_setpalette,gfx_setquithandler;
GFXGETKBDSTAT gfx_get_keyboard_status;
GFXGETDEVSTAT gfx_get_device_status;
GFXRESOLVERROR gfx_resolverror;
GFXPOLL gfx_poll;

HINSTANCE gfxdrv;

UINT gfx_caps;

#define MAD_GFXDRIVER_NAME "GFXDriver.dll"

void
GFXDRIVER::init() {
    UINT i;
    char tempstr[MAD_TEMPSTR_SIZE];

    gfxdrv = NULL; gfx_done = NULL;
    if ((gfxdrv = LoadLibrary (MAD_GFXDRIVER_NAME)) == NULL) {
        /* Oops, this failed. die with error 13: cannot load graphics driver */
	sprintf(tempstr,MAD_ERROR_13,MAD_GFXDRIVER_NAME);
	die(tempstr);
    }
    if ((get_driver_type = (GFXFUNC) GetProcAddress (gfxdrv, DLL_FUNCNAME_PRE"get_driver_type")) == NULL) {
        /* Damn, the driver doesn't contain the get_driver_type function. die (error 12: graphics driver doesn't contain the function) */
	sprintf(tempstr,MAD_ERROR_12,"get_driver_type()");
	die(tempstr);
    }
    /* Check if this really is a graphics driver */
    if ((_UINT)get_driver_type() != (_UINT)DRV_TYPE_GFX) {
    	/* It is not. die (error 14: is not a graphics driver) */
    	sprintf(tempstr,MAD_ERROR_14,MAD_GFXDRIVER_NAME);
    	die(tempstr);
    }
    if ((gfx_getcaps = (GFXFUNC) GetProcAddress (gfxdrv, DLL_FUNCNAME_PRE"gfx_getcaps")) == NULL) {
    	/* Damn, the driver doesn't contain the gfx_getcaps function. die (error 12: graphics driver doesn't contain the function) */
    	sprintf(tempstr,MAD_ERROR_12,"gfx_getcaps()");
    	die(tempstr);
    }
    if ((gfx_init = (GFXFUNC) GetProcAddress (gfxdrv, DLL_FUNCNAME_PRE"gfx_init")) == NULL) {
    	/* Damn, the driver doesn't contain the init function. die (error 12: graphics driver doesn't contain the function) */
    	sprintf(tempstr,MAD_ERROR_12,"init()");
    	die(tempstr);
    }
    if ((gfx_done = (GFXFUNC) GetProcAddress (gfxdrv, DLL_FUNCNAME_PRE"gfx_done")) == NULL) {
	/* Damn, the driver doesn't contain the done function. die (error 12: graphics driver doesn't contain the function) */
	sprintf(tempstr,MAD_ERROR_12,"done()");
	die(tempstr);
    }
    if ((gfx_draw = (GFXDRAW) GetProcAddress (gfxdrv, DLL_FUNCNAME_PRE"gfx_draw")) == NULL) {
        /* Damn, the driver doesn't contain the draw function. die (error 12: graphics driver doesn't contain the function) */
      	sprintf(tempstr,MAD_ERROR_12,"draw()");
      	die(tempstr);
    }
    if ((gfx_setpalette = (GFXDRAW) GetProcAddress (gfxdrv, DLL_FUNCNAME_PRE"gfx_setpalette")) == NULL) {
        /* Damn, the driver doesn't contain the setpalette function. die (error 12: graphics driver doesn't contain the function) */
    	sprintf(tempstr,MAD_ERROR_12,"setpalette()");
     	die(tempstr);
    }
    if ((gfx_resolverror = (GFXRESOLVERROR) GetProcAddress (gfxdrv, DLL_FUNCNAME_PRE"gfx_resolverror")) == NULL) {
        /* Damn, the driver doesn't contain the setpalette function. die (error 12: graphics driver doesn't contain the function) */
     	sprintf(tempstr,MAD_ERROR_12,"resolverror()");
     	die(tempstr);
    }
    if ((gfx_poll = (GFXPOLL) GetProcAddress (gfxdrv, DLL_FUNCNAME_PRE"gfx_poll")) == NULL) {
        /* Damn, the driver doesn't contain the setpalette function. die (error 12: graphics driver doesn't contain the function) */
     	sprintf(tempstr,MAD_ERROR_12,"poll()");
     	die(tempstr);
    }
    /* Dump the capabilities in [gfx_caps] */
    gfx_caps = gfx_getcaps();
    /* Check whether the keyboard functions are available */
    if (gfxdriver->get_caps() & GFX_DRIVER_CAPS_ALSOKBD) {
    	/* Oh boy, this driver also does controls! Create the right pointers to procedures */
        if ((gfx_get_keyboard_status = (GFXGETKBDSTAT) GetProcAddress (gfxdrv, DLL_FUNCNAME_PRE"gfx_get_keyboard_status")) == NULL) {
            /* Damn, the driver doesn't contain the setpalette function. die (error 12: graphics driver doesn't contain the function) */
	    sprintf(tempstr,MAD_ERROR_12,"get_keyboard_status");
	    die(tempstr);
        }
   	if ((gfx_setquithandler = (GFXDRAW) GetProcAddress (gfxdrv, DLL_FUNCNAME_PRE"gfx_setquithandler")) == NULL) {
            /* Damn, the driver doesn't contain the setpalette function. die (error 12: graphics driver doesn't contain the function) */
	    sprintf(tempstr,MAD_ERROR_12,"setquithandler");
	    die(tempstr);
        }
    }
    /* Check whether the aiming device functions are available */
    if (gfxdriver->get_caps() & GFX_DRIVER_CAPS_ALSODEV) {
        /* Yeah. Well, let use them! */
        if ((gfx_get_device_status = (GFXGETDEVSTAT) GetProcAddress (gfxdrv, DLL_FUNCNAME_PRE"gfx_get_device_status")) == NULL) {
            /* Damn, the driver doesn't contain the setpalette function. die (error 12: graphics driver doesn't contain the function) */
	    sprintf(tempstr,MAD_ERROR_12,"get_device_status");
	    die(tempstr);
        }
    }
    /* Initialize the beast */
    if ((i = gfx_init())) {
        /* This failed. die with error 15: Initialization of graphics driver failed, it reported */
        sprintf(tempstr,MAD_ERROR_15,gfx_resolverror(i));
    	die (tempstr);
    }
}

void
GFXDRIVER::done() {
    /* Was a graphics driver ever loaded? */
    if (gfxdrv != NULL) {
    	/* Yeah, deinitialize it */
    	if (gfx_done != NULL) {
            gfx_done();
    	}
    	/* And free it */
    	FreeLibrary (gfxdrv);
    	/* And set the pointer to NULL */
    	gfxdrv = NULL;
    }
}

void
GFXDRIVER::draw(void *data) {
    UINT i;
    char tempstr[MAD_TEMPSTR_SIZE];

    if ((i = gfx_draw(data))) {
        /* This failed. die with error 16: graphics driver reported */
        sprintf(tempstr,MAD_ERROR_16,gfx_resolverror(i));
    	die (tempstr);
    }
}

_UINT
GFXDRIVER::get_hres() {
    return GFX_DRIVER_HRES;
}

_UINT
GFXDRIVER::get_vres() {
    return GFX_DRIVER_VRES;
}

_UINT
GFXDRIVER::get_nofcols() {
    return GFX_DRIVER_NOFCOLS;
}

void
GFXDRIVER::setpalette(char *ptr) {
    UINT i;
    char tempstr[MAD_TEMPSTR_SIZE];

    if ((i = gfx_setpalette(ptr))) {
        /* This failed. die with error 16: graphics driver reported */
        sprintf(tempstr,MAD_ERROR_16,gfx_resolverror(i));
    	die (tempstr);
    }
}

_UINT
GFXDRIVER::get_caps() {
    /* Just return the capabilities field */
    return gfx_caps;
}

void
GFXDRIVER::poll() {
    gfx_poll();
}

void
GFXDRIVER::get_device_stat(_UINT *x,_UINT *y,_UINT *stat) {
    UINT xx, yy, statstat;

    /* Only do this if we are capable of it */
    if (gfxdriver->get_caps() & GFX_DRIVER_CAPS_ALSODEV) {
        gfx_get_device_status(&xx,&yy,&statstat);
     	*x = (_UINT)xx;
     	*y = (_UINT)yy;
     	*stat = (_UINT)statstat;
     }
}

void
GFXDRIVER::get_keyboard_status(void *dest) {
     /* Only do this if we are capable of it */
     if (gfxdriver->get_caps() & GFX_DRIVER_CAPS_ALSOKBD) {
        gfx_get_keyboard_status(dest);
     }
}

void
GFXDRIVER::set_quit_handler(void *dest) {
    UINT i;
    char tempstr[MAD_TEMPSTR_SIZE];
    if ((i = gfx_setquithandler(dest))) {
      	/* This failed. die with error 16: graphics driver reported */
        sprintf(tempstr,MAD_ERROR_16,gfx_resolverror(i));
    	die (tempstr);
    }
}
