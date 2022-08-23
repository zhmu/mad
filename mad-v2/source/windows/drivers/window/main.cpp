/*
 *                                   main.cpp
 *                                  Version 1.0
 *
 *                             (c) 1999 The MAD Crew
 *
 * This is the source file of the WINDOW.DLL file. It is a graphics and input driver for MAD in a window.
 *
 * Special thanks to Rosinha <rosinha@helllabs.org> for creating the Sarien AGI interprinter Windows port.
 * The code of that port was used to get this working. Thanks Rosinha!
 *
 */
#include <stdio.h>
#include <windows.h>
#include "../../../controls.h"
#include "../../../mad.h"
#include "../../../types.h"
#include "../../../gfx_drv.h"

/* Our graphics driver supports keyboard and device handling too. */
#define GFX_CAPS	GFX_DRIVER_CAPS_DEFAULT | GFX_DRIVER_CAPS_ALSOKBD | GFX_DRIVER_CAPS_ALSODEV

/* Define our class name */
#define MAD_CLASSNAME       "MaDWnDCLaSS"
#define MAD_WINDOWTITLE     "MAD"

#define MAD_KEYARRAY_SIZE   128

/* Error messages */
#define MAD_ERROR_1000      "Error 1000: Unable to register window class"
#define MAD_ERROR_1001      "Error 1001: Unable to create main window"
#define MAD_ERROR_1002      "Error 1002: Out of memory"
#define MAD_ERROR_UNKNOWN   "Unknown error. Help!"

HINSTANCE	  hInstance;                              /* Our public instance handle */
HWND		  hWnd;                                   /* Our window class */
void*         emergency_exit_procedure;              /* Emergency exit procedure */
char          keyDown[MAD_KEYARRAY_SIZE];             /* Key down buffer */
int	          device_x, device_y;                     /* Input device X and Y coordinates */
int			  button_state;                           /* Input device button status */

BITMAPINFO*   resbiBitmap;                           /* Screen bitmap information */
char*         tempbuf;                                /* Temponary screen buffer */

/*
 * UINT get_driver_type()
 *
 * EXPORTED
 *
 * This will be called by MAD to ask the driver what type it is. Graphics drivers should return
 * DRV_TYPE_GFX to indicate they are a graphics driver.
 *
 */
UINT
get_driver_type() {
	/* Tell MAD we are a graphics driver */
	return DRV_TYPE_GFX;
}

/*
 * UINT gfx_getcaps()
 *
 * EXPORTED
 *
 * This will be called by MAD to query the capabilities of the driver.
 *
 */
UINT
gfx_getcaps() {
	/* Tell MAD our capabilities */
	return GFX_CAPS;
}

/*
 * UINT gfx_get_hres()
 *
 * EXPORTED
 *
 * This will be called by MAD to query the horizontal resolution of the driver
 *
 */
UINT
gfx_get_hres() {
	/* Tell MAD our horizontal resolution */
	return GFX_DRIVER_HRES;
}

/*
 * UINT gfx_get_vres()
 *
 * EXPORTED
 *
 * This will be called by MAD to query the vertical resolution of the driver
 *
 */
UINT
gfx_get_vres() {
	/* Tell MAD our vertical resolution */
	return GFX_DRIVER_VRES;
}

/*
 * UINT gfx_get_nofcols()
 *
 * EXPORTED
 *
 * This will be called by MAD to query the number of colors supported by the driver
 *
 */
UINT
gfx_get_nofcols() {
	/* Tell MAD the number of colors we support */
	return GFX_DRIVER_NOFCOLS;
}

/*
 * void gfx_done()
 *
 * EXPORTED
 *
 * This will be called by MAD whenever it needs to deinitialize the driver. It can also be called by
 * the driver itself whenever it needs to reinitialize itself.
 *
 */
void
gfx_done() {
	/* Is there a main window? */
	if (hWnd != NULL) {
		/* Yeah, nuke it */
		DestroyWindow (hWnd);
		/* Make sure we don't do this twice */
		hWnd = NULL;
	}
	/* Is there bitmap data allocated? */
	if (resbiBitmap != NULL) {
		/* Yeah, nuke it */
		free (resbiBitmap);
		/* Make sure we don't do this twice */
		resbiBitmap = NULL;	
	}
	/* Is there temponary data allocated? */
	if (tempbuf != NULL) {
		/* Yeah, nuke it */
		free (tempbuf);
		/* Make sure we don't do this twice */
		tempbuf = NULL;	
	}
	/* Get rid of our class */
	UnregisterClass (MAD_CLASSNAME, hInstance);
}

/*
 * LRESULT CALLBACK MainWindowProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
 *
 * This is the window handling procedure of the main window.
 *
 */
LRESULT CALLBACK MainWindowProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	/* What kind of event is it? */
	switch (uMsg) {
	    case WM_KEYDOWN: /* User pressed a key. Handle it! */
			             if (LOBYTE (HIWORD (lParam)) == CONTROLS_KEY_F12) {
							 /* This is the emergency exit. Call it if it is not NULL */
							 if (emergency_exit_procedure != NULL) {
								 _asm {
									 call emergency_exit_procedure
								 }
							 }
						 }
						 /* Otherwise just enable it in the array */
			             keyDown[LOBYTE (HIWORD (lParam))] = 1;
			             break;
		  case WM_KEYUP: /* User released a key. Handle it! */
			             keyDown[LOBYTE (HIWORD (lParam))] = 0;
			             break;
    case WM_LBUTTONDOWN: /* Left mouse button is down. Set bit in [button_state] */
				         button_state |= CONTROLS_LEFTBUTTON;
						 break;
      case WM_LBUTTONUP: /* Left mouse button is up. Clear bit in [button_state] */
				         button_state &= ~CONTROLS_LEFTBUTTON;
						 break;
    case WM_RBUTTONDOWN: /* Right mouse button is down. Set bit in [button_state] */
				         button_state |= CONTROLS_RIGHTBUTTON;
						 break;
      case WM_RBUTTONUP: /* Right mouse button is up. Clear bit in [button_state] */
				         button_state &= ~CONTROLS_RIGHTBUTTON;
						 break;
	  case WM_MOUSEMOVE: /* User moved the mouse. Handle it */
		                 device_x = LOWORD (lParam);
		                 device_y = HIWORD (lParam);
		                 break;
	            default: /* We don't handle this event. Let Windows take care of it! */
	                     return DefWindowProc (hWnd, uMsg, wParam, lParam);
	}
	/* Tell Windows we handled the event */
	return TRUE;
}

/*
 * UINT gfx_init()
 *
 * This will be called by MAD whenever it needs to initialize the graphics driver. This function
 * must return zero if all went OK, otherwise an error code.
 *
 */
UINT
gfx_init() {
    WNDCLASS wc;

	/* Set all buffers to NULL to prevent them from being freed */
	resbiBitmap = NULL; tempbuf = NULL;

	/* Set up our window class stuff */
	memset (&wc, 0, sizeof (wc));
    wc.style = 0;									/* Special style options (see WNDCLASS help) */
	wc.lpfnWndProc = (WNDPROC)MainWindowProc;		/* Window procedure */
	wc.cbClsExtra = 0;								/* No extra memory needed */
	wc.cbWndExtra = 0;								/* No extra memory needed */
	wc.hInstance = hInstance;						/* Instance handle */
	wc.hIcon = LoadIcon (NULL, IDI_APPLICATION);	/* XXX: default icon for now */
	wc.hCursor = NULL;                              /* Always hide the cursor in our window */
	wc.hbrBackground = NULL;                        /* We handle all painting */
	wc.lpszMenuName = NULL;							/* No menu needed */
	wc.lpszClassName = MAD_CLASSNAME;				/* Class name */
	/* Try to register the class (shouldn't fail) */
	if (!RegisterClass (&wc)) {
	    /* This failed. Return error 1003: Unable to register window class */
		return 1000;
	}

	/* Create the main window */
	hWnd = CreateWindow (MAD_CLASSNAME,		                                           /* Class name */
						 MAD_WINDOWTITLE,	                                           /* Window title */
						 WS_POPUP | WS_VISIBLE | WS_SYSMENU | WS_MINIMIZEBOX,          /* Window style */
						 0,		                                                       /* X postion */
						 0,	         	                                               /* Y postion */
						 GFX_DRIVER_HRES,                                              /* Width */
						 GFX_DRIVER_VRES,                                              /* Height */
						 NULL,		         		                                   /* Parent window */
						 NULL,				                                           /* XXX: No menu */
						 hInstance,		          	                                   /* Instance handle */
						 NULL);				                                           /* Window creation data (unused) */
    if (hWnd == NULL) {
	    /* This failed. Return error 1004: Unable to create the main window */
		return 1001;
	}

	/* Give ourselves the focus (just in case) */
	SetFocus (hWnd);

	/* Allocate memory for the bitmap information */
	resbiBitmap = (BITMAPINFO *)malloc (sizeof (*resbiBitmap) + 256 * sizeof (RGBQUAD));
	if (resbiBitmap == NULL) {
		/* This failed. Die */
		return 1002;
	}

	/* Initialize structures */
	memset ((void *)&resbiBitmap->bmiHeader, 0, sizeof (resbiBitmap->bmiHeader));
	resbiBitmap->bmiHeader.biSize = sizeof (BITMAPINFOHEADER);				 /* Structure size */
	resbiBitmap->bmiHeader.biWidth = GFX_DRIVER_HRES;                        /* Width */
    resbiBitmap->bmiHeader.biHeight = GFX_DRIVER_VRES;                       /* Height */
    resbiBitmap->bmiHeader.biPlanes = 1;                                     /* Number of planes */
    resbiBitmap->bmiHeader.biBitCount = 8;                                   /* 8 bits */
    resbiBitmap->bmiHeader.biCompression = BI_RGB;                           /* Uncompressed */
    resbiBitmap->bmiHeader.biSizeImage = 0;                                  /* Size (must be zero) */
	resbiBitmap->bmiHeader.biXPelsPerMeter = 1;                              /* Useless stuff */
	resbiBitmap->bmiHeader.biYPelsPerMeter = 1;                              /* Useless stuff */
    resbiBitmap->bmiHeader.biClrUsed = 0;                                    /* Number of used colors */
	resbiBitmap->bmiHeader.biClrImportant = 0;                               /* All colors are important */

	/* Black out the palette */
	memset (resbiBitmap->bmiColors, 0, 1024);

	/* Set mouse stuff to non-existant */
	button_state = 0; device_x = 0; device_y = 0;

	/* Allocate memory for the temponary screen */
	if ((tempbuf = (char*)malloc (GFX_DRIVER_HRES * GFX_DRIVER_VRES)) == NULL) {
		/* This failed. Die */
		return 1002;
	}

	/* Hide the cursor in our window! */
	ShowCursor (FALSE);
	
	/* Say it's OK */
	return 0;
}

/*
 * UINT gfx_draw(char* source)
 *
 * EXPORTED
 *
 * This will put buffer [source] on the screen.
 *
 */
UINT
gfx_draw (char* source) {
	HDC dc;
	int i;
	RECT r;

	/* Get the window position */
	GetWindowRect (hWnd, &r);

	/* Get the window's DC handle */
	dc = GetWindowDC (hWnd);

	/* Flip the image horizontally (Aargh!!! Stupid Windoze!) */
	for (i = 0; i < GFX_DRIVER_VRES; i++) {
 		memcpy ((void *)(tempbuf + ((GFX_DRIVER_VRES - 1 - i) * GFX_DRIVER_HRES)), (void *)(source + (i * GFX_DRIVER_HRES)), GFX_DRIVER_HRES);
	}

	/* Draw it! */
	SetDIBitsToDevice (dc,                                     /* Window handle */
		               r.left,                                 /* X coordinate */
					   r.top,                                  /* Y coordinate */
					   GFX_DRIVER_HRES,                        /* Width */
					   GFX_DRIVER_VRES,                        /* Height */
		               0,                                      /* X coordinate */
					   0,                                      /* Y coordinate */
					   0,                                      /* Starting scan line */
					   GFX_DRIVER_VRES,                        /* Number of scan lines */
					   tempbuf,                                /* Buffer data */
					   resbiBitmap,                            /* Bitmap data */
					   DIB_RGB_COLORS);                        /* Palette format */

	/* Nuke the DC handle */
	ReleaseDC (hWnd, dc);

	/* Say it's all OK */
	return 0;
}

/*
 * UINT gfx_setpalette(char* data)
 *
 * EXPORTED
 *
 * This will activate palette data [data].
 *
 */
UINT
gfx_setpalette(char* data) {
	int i;

	/* Convert the normal RGB format to the weird RGB0 Windows format */
	for (i = 0; i < 256; i++) {
		resbiBitmap->bmiColors[i].rgbRed = data[i * 3];
		resbiBitmap->bmiColors[i].rgbGreen = data[i * 3 + 1];
		resbiBitmap->bmiColors[i].rgbBlue = data[i * 3 + 2];
		resbiBitmap->bmiColors[i].rgbReserved = 0;
	}

	/* Say it's all OK */
	return 0;
}

/*
 * char* gfx_resolverror(_UINT errorcode)
 *
 * EXPORTED
 *
 * This will resolve graphics driver error codes. It will not resolve any MAD errors.
 *
 */
char*
gfx_resolverror(_UINT errorcode) {
    switch (errorcode) {
	        case 1000: return MAD_ERROR_1000;
	        case 1001: return MAD_ERROR_1001;
	        case 1002: return MAD_ERROR_1002;
	}
	return MAD_ERROR_UNKNOWN;
}

/*
 * gfx_poll()
 *
 * EXPORTED
 *
 * This will be called by MAD whenever it is idle. It will allow the graphics driver to take care of
 * its own business.
 *
 */
void
gfx_poll() {
	MSG msg;

	/* Is there a message waiting? */
	if (PeekMessage (&msg, NULL, 0, 0, 0)) {
	     /* Yeah, handle it */
		 GetMessage (&msg, NULL, 0, 0);
         DispatchMessage (&msg);
	}
}

/*
 * gfx_get_keyboard_status(char* dest)
 *
 * EXPORTED
 *
 * This will be called by MAD whenever it wants to know what keys are currently pressed. It will pass a
 * 128 byte buffer. An index should list zero if the key is not pressed or non-zero if the key is pressed.
 *
 */
void
gfx_get_keyboard_status(char* dest) {
	/* Copy our keyDown[] buffer to [dest] */
	memcpy (dest, keyDown, MAD_KEYARRAY_SIZE);
}

/*
 * gfx_get_device_status(UINT *x,UINT *y,UINT *stat)
 *
 * EXPORTED
 *
 * This will be called by MAD whenever it wants to know where the current device is. It expects to return
 * the coordinates in ([*x],[*y]). The button status should be returned in [*stat].
 *
 */
void
gfx_get_device_status(UINT *x,UINT *y,UINT *stat) {
	/* Give MAD our status */
    *x = device_x; *y = device_y; *stat = button_state;
}

/*
 * UINT gfx_setquithandler(void* handler)
 *
 * EXPORTED
 *
 * This will be called by MAD whenever it wants to set the quit handler. The quit handler is the procedure
 * which will be called whenever the Emergency Exit key (F12) is hit.
 *
 */
UINT
gfx_setquithandler(void* handler) {
    /* Activate it */
	emergency_exit_procedure = handler;

	/* Say all went OK */
	return 0;
}

BOOL WINAPI
DllMain (HINSTANCE hInst, ULONG ulReasonForCall, LPVOID lpReserved) {
    /* Are we about to be initialized? */
	if (ulReasonForCall != DLL_PROCESS_ATTACH) {
		/* Nope. Just say it's ok */
		return TRUE;
	}

    /* Save the module handle */
	hInstance = hInst;

	/* Nuke all keystroke data */
	memset (keyDown, 0, MAD_KEYARRAY_SIZE);

	/* Tell Windows it's OK */
	return TRUE;
}