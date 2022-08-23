/*
 *                                   window.cpp
 *                                  Version 2.0
 *
 *                             (c) 1999, 2000 The MAD Crew
 *
 * This is the source file of the WINDOW.DLL file. It is a graphics and input driver for MAD in a window.
 *
 * Special thanks to Rosinha <rosinha@helllabs.org> for creating the Sarien AGI interprinter Windows port.
 * The code of that port was used to get this working. Thanks Rosinha!
 *
 */
#pragma hdrstop

#define NOTCPP extern "C"
#define EXPORTED __declspec(dllexport)

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "controls.h"
#include "mad.h"
#include "types.h"
#include "gfx.h"
#include "gfx_drv.h"

#define STRING_ORGSIZE "Original size (1:1)"
#define STRING_DOUBLESIZE "Double size (1:2)"
#define STRING_QUADRASIZE "Quadra size (1:4)"

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

HINSTANCE     hInstance;                              /* Our public instance handle */
HWND	      hWnd;                                   /* Our window class */
void*         emergency_exit_procedure;               /* Emergency exit procedure */
char          keyDown[MAD_KEYARRAY_SIZE];             /* Key down buffer */
int	      device_x, device_y;                     /* Input device X and Y coordinates */
int	      button_state;                           /* Input device button status */

BITMAPINFO*   resbiBitmap;                            /* Screen bitmap information */
char*         tempbuf;                                /* Temponary screen buffer */
HMENU         sysmenu;                                /* System menu handle */

int           h_scale, v_scale;                       /* Horizontal and vertical scale */

/*
 * UINT get_driver_type()
 *
 * EXPORTED
 *
 * This will be called by MAD to ask the driver what type it is. Graphics drivers should return
 * DRV_TYPE_GFX to indicate they are a graphics driver.
 *
 */
NOTCPP UINT EXPORTED
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
NOTCPP UINT EXPORTED
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
NOTCPP UINT EXPORTED
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
NOTCPP UINT EXPORTED
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
NOTCPP UINT EXPORTED
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
NOTCPP void EXPORTED
gfx_done() {
    /* Tell Windows to show the mouse pointer */
    ShowCursor (TRUE);

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
 * gfx_draw_temp_buffer()
 *
 * This will dump tempbuf[] to the screen.
 *
 */
void
gfx_draw_temp_buffer() {
    HDC dc;

    /* Get the window's DC handle */
    dc = GetWindowDC (hWnd);

    /* Draw it! */    
    StretchDIBits (dc,                                          /* Window handle */
                   GetSystemMetrics (SM_CXBORDER),
                   GetSystemMetrics (SM_CYSIZE) + GetSystemMetrics (SM_CYBORDER) + 1,
                   GFX_DRIVER_HRES * h_scale,
                   GFX_DRIVER_VRES * v_scale,
       	           0,                                      /* X coordinate */
                   0,                                      /* Y coordinate */
                   GFX_DRIVER_HRES,                        /* Source width */
                   GFX_DRIVER_VRES,                        /* Source height */
                   tempbuf,                                /* Buffer data */
                   resbiBitmap,                            /* Bitmap data */
                   DIB_RGB_COLORS,                         /* Palette format */
                   SRCCOPY);                               /* Draw method */

    /* Nuke the DC handle */
    ReleaseDC (hWnd, dc);
}

/*
 * ResizeWindow (HWND hWnd, int new_hscale, int new_vscale)
 *
 * This will resize window [hWnd] to use 1:[new_hscale] and 1:[new_vscale].
 *
 */
void
ResizeWindow (HWND hWnd, int new_hscale, int new_vscale) {
   h_scale = new_hscale; v_scale = new_hscale;
   SetWindowPos (hWnd,
                 HWND_TOPMOST,
                 0,
                 0,
                 (GFX_DRIVER_HRES * new_hscale),
                 (GFX_DRIVER_VRES * new_vscale),
                 SWP_NOMOVE | SWP_NOZORDER);
}

/*
 * CheckMenuItem (HWND hWnd, int item)
 *
 * This will put a checkmark before system menu item [item] and nothing else.
 *
 */
void
CheckTheMenuItem (HWND hWnd, int item) {
    int i;

    for (i = 0; i < 3; i++) {
        CheckMenuItem (sysmenu, i, MF_BYCOMMAND | MF_UNCHECKED);
    }
    CheckMenuItem (sysmenu, item, MF_BYCOMMAND | MF_CHECKED);
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
     case WM_SYSKEYDOWN:
        case WM_KEYDOWN: /* User pressed a key. Handle it! */
	                 if (LOBYTE (HIWORD (lParam)) == CONTROLS_KEY_F12) {
	    		     /* This is the emergency exit. Call it if it is not NULL */
			     if (emergency_exit_procedure != NULL) {
                        /* XXX */
                    }
                         }

 			 if (!(lParam & (1 << 30))) {
                             keyDown[LOBYTE (HIWORD (lParam))] = 1;
                         }
			 break;
       case WM_SYSKEYUP:
          case WM_KEYUP: /* User released a key. Handle it! */
                         keyDown[LOBYTE (HIWORD (lParam))] = 0;
	                 break;
    case WM_LBUTTONDOWN: /* Left mouse button is down. Set bit in [button_state] */
		         button_state |= CONTROLS_BUTTON1;
			 break;
      case WM_LBUTTONUP: /* Left mouse button is up. Clear bit in [button_state] */
 		         button_state &= ~CONTROLS_BUTTON1;
 			 break;
    case WM_RBUTTONDOWN: /* Right mouse button is down. Set bit in [button_state] */
       		         button_state |= CONTROLS_BUTTON2;
 			 break;
      case WM_RBUTTONUP: /* Right mouse button is up. Clear bit in [button_state] */
    	                 button_state &= ~CONTROLS_BUTTON2;
			 break;
      case WM_MOUSEMOVE: /* User moved the mouse. Handle it */
    	                 device_x = LOWORD (lParam);
   	                 device_y = HIWORD (lParam);
 	                 break;
          case WM_CLOSE: /* User tried to close our window. Ignore it */
                         break;
     case WM_SYSCOMMAND: /* This will handle commands from our system menu */
                         switch (LOWORD (wParam)) {
                              case 0: /* Original size */
                                      ResizeWindow (hWnd, 1, 1);
                                      CheckTheMenuItem (hWnd, 0);
                                      break;
                              case 1: /* Double size */
                                      ResizeWindow (hWnd, 2, 2);
                                      CheckTheMenuItem (hWnd, 1);
                                      break;
                              case 2: /* Quadra size */
                                      ResizeWindow (hWnd, 4, 4);
                                      CheckTheMenuItem (hWnd, 2);
                                      break;
                             default: /* We don't handle this. Let Windows do it */
                                      return DefWindowProc (hWnd, uMsg, wParam, lParam);
                         }
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
 * EXPORTED
 *
 * This will be called by MAD whenever it needs to initialize the graphics driver. This function
 * must return zero if all went OK, otherwise an error code.
 *
 */
NOTCPP UINT EXPORTED
gfx_init() {
    WNDCLASS wc;
    MENUITEMINFO mii;

    /* Set all buffers to NULL to prevent them from being freed */
    resbiBitmap = NULL; tempbuf = NULL;

    /* Set up our window class stuff */
    memset (&wc, 0, sizeof (wc));
    wc.style = 0;								       /* Special style options (see WNDCLASS help) */
    wc.lpfnWndProc = (WNDPROC)MainWindowProc;		                               /* Window procedure */
    wc.cbClsExtra = 0;							               /* No extra memory needed */
    wc.cbWndExtra = 0;							               /* No extra memory needed */
    wc.hInstance = hInstance;					           	       /* Instance handle */
    wc.hIcon = LoadIcon (NULL, IDI_APPLICATION);          	                       /* XXX: default icon for now */
    wc.hCursor = NULL;                                                                 /* Always hide the cursor in our window */
    wc.hbrBackground = NULL;                                                           /* We handle all painting */
    wc.lpszMenuName = NULL;							       /* No menu needed */
    wc.lpszClassName = MAD_CLASSNAME;				                       /* Class name */
    /* Try to register the class (shouldn't fail) */
    if (!RegisterClass (&wc)) {
        /* This failed. Return error 1003: Unable to register window class */
    	return 1000;
    }

    /* Create the main window */
    hWnd = CreateWindow (MAD_CLASSNAME,		                                       /* Class name */
                         MAD_WINDOWTITLE,	                                       /* Window title */
			 WS_CAPTION | WS_VISIBLE | WS_SYSMENU | WS_MINIMIZEBOX,        /* Window style */
			 0,		                                               /* X postion */
			 0,	         	                                       /* Y postion */
			 GFX_DRIVER_HRES,                                              /* Width */
			 GFX_DRIVER_VRES + GetSystemMetrics (SM_CYSIZE),               /* Height */
			 NULL,		         		                       /* Parent window */
			 NULL,				                               /* XXX: No menu */
			 hInstance,		          	                       /* Instance handle */
			 NULL);				                               /* Window creation data (unused) */
    if (hWnd == NULL) {
	    /* This failed. Return error 1004: Unable to create the main window */
            return 1001;
    }

    /* Give ourselves the focus (just in case) */
    SetFocus (hWnd);

    /* Add entries to the system menu */
    sysmenu = GetSystemMenu (hWnd, FALSE);

    /* Add a separator */
    memset (&mii, 0, sizeof (mii));
    mii.cbSize = sizeof (mii);
    mii.fMask = MIIM_TYPE | MIIM_ID | MIIM_STATE;
    mii.fType = MFT_SEPARATOR;
    mii.fState = MFS_GRAYED;
    InsertMenuItem (sysmenu, 0, TRUE, &mii);

    /* Add the "Original size (1:1)" thing */
    mii.fType = MFT_STRING;
    mii.fState = MFS_ENABLED | MFS_CHECKED;
    mii.wID = 0;
    mii.hSubMenu = NULL;
    mii.hbmpChecked = NULL;
    mii.hbmpUnchecked = NULL;
    mii.dwTypeData = STRING_ORGSIZE;
    mii.cch = strlen (mii.dwTypeData);
    InsertMenuItem (sysmenu, 0, TRUE, &mii);

    /* Add the "Double size (1:2)" thing */
    mii.fState = MFS_ENABLED;
    mii.wID = 1;
    mii.dwTypeData = STRING_DOUBLESIZE;
    mii.cch = strlen (mii.dwTypeData);
    InsertMenuItem (sysmenu, 0, TRUE, &mii);

    /* Add the "Quadra size (1:4)" thing */
    mii.fState = MFS_ENABLED;
    mii.wID = 2;
    mii.dwTypeData = STRING_QUADRASIZE;
    mii.cch = strlen (mii.dwTypeData);
    InsertMenuItem (sysmenu, 0, TRUE, &mii);
    
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
    resbiBitmap->bmiHeader.biBitCount = 32;                                  /* 32 bit colors */
    resbiBitmap->bmiHeader.biCompression = BI_RGB;                           /* Uncompressed */
    resbiBitmap->bmiHeader.biSizeImage = 0;                                  /* Size (must be zero) */
    resbiBitmap->bmiHeader.biXPelsPerMeter = 0;                              /* Useless stuff */
    resbiBitmap->bmiHeader.biYPelsPerMeter = 0;                              /* Useless stuff */
    resbiBitmap->bmiHeader.biClrUsed = 0;                                    /* Number of used colors */
    resbiBitmap->bmiHeader.biClrImportant = 0;                               /* All colors are important */

    /* Black out the palette */
    memset (resbiBitmap->bmiColors, 0, 1024);

    /* Set mouse stuff to non-existant */
    button_state = 0; device_x = 0; device_y = 0;

    /* Allocate memory for the temponary screen */
    if ((tempbuf = (char*)malloc (GFX_DRIVER_HRES * GFX_DRIVER_VRES * 4)) == NULL) {
    	/* This failed. Die */
    	return 1002;
    }

    /* Hide the cursor in our window! */
    ShowCursor (FALSE);

    /* default scale is 1:1 */
    h_scale = 1; v_scale = 1;

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
NOTCPP UINT EXPORTED
gfx_draw (char* source) {
    int i;

    /* Flip the image horizontally (Aargh!!! Stupid Windoze!) */
    for (i = 0; i < GFX_DRIVER_VRES; i++) {
    	memcpy ((void *)(tempbuf + ((GFX_DRIVER_VRES - 1 - i) * GFX_DRIVER_HRES * GFX_BPP)), (void *)(source + (i * GFX_DRIVER_HRES * GFX_BPP)), GFX_DRIVER_HRES * GFX_BPP);
    }
    
    /* Draw it! */
    gfx_draw_temp_buffer();

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
NOTCPP UINT EXPORTED
gfx_setpalette(char* data) {
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
NOTCPP char* EXPORTED
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
NOTCPP void EXPORTED
gfx_poll() {
    MSG msg;

    /* Is there a message waiting? */
    while (PeekMessage (&msg, NULL, 0, 0, 0)) {
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
NOTCPP void EXPORTED
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
NOTCPP void EXPORTED
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
NOTCPP UINT EXPORTED
gfx_setquithandler(void* handler) {
    /* Activate it */
    emergency_exit_procedure = handler;

    /* Say all went OK */
    return 0;
}

//---------------------------------------------------------------------------
//   Important note about DLL memory management when your DLL uses the
//   static version of the RunTime Library:
//
//   If your DLL exports any functions that pass String objects (or structs/
//   classes containing nested Strings) as parameter or function results,
//   you will need to add the library MEMMGR.LIB to both the DLL project and
//   any other projects that use the DLL.  You will also need to use MEMMGR.LIB
//   if any other projects which use the DLL will be perfomring new or delete
//   operations on any non-TObject-derived classes which are exported from the
//   DLL. Adding MEMMGR.LIB to your project will change the DLL and its calling
//   EXE's to use the BORLNDMM.DLL as their memory manager.  In these cases,
//   the file BORLNDMM.DLL should be deployed along with your DLL.
//
//   To avoid using BORLNDMM.DLL, pass string information using "char *" or
//   ShortString parameters.
//
//   If your DLL uses the dynamic version of the RTL, you do not need to
//   explicitly add MEMMGR.LIB as this will be done implicitly for you
//---------------------------------------------------------------------------
int WINAPI DllEntryPoint (HINSTANCE hInst, unsigned long reason, void*) {
    /* Are we about to be initialized? */
    if (reason != DLL_PROCESS_ATTACH) {
    	/* Nope. Just say it's ok */
        return TRUE;
    }

    /* Save the module handle */
    hInstance = hInst;

    /* Nuke all keystroke data */
    memset (keyDown, 0, MAD_KEYARRAY_SIZE);

    /* Tell Windows it's OK */
    return 1;
}
//---------------------------------------------------------------------------


