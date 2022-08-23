/*
 *                        MAD DirectX 3.0 Driver Version 2.0
 *                             (c) 1999 The MAD Crew
 *
 * This is a completly rewritten DirectX driver for MAD. I hope this one does not bug so much anymore.
 *
 */
#include <stdio.h>
#include <windows.h>
#include <ddraw.h>
#include "../../../controls.h"
#include "../../../mad.h"
#include "../../../types.h"
#include "../../../gfx_drv.h"

/* Our graphics driver supports keyboard and device handling too. */
#define GFX_CAPS	GFX_DRIVER_CAPS_DEFAULT | GFX_DRIVER_CAPS_ALSOKBD | GFX_DRIVER_CAPS_ALSODEV

#define MAD_CLASSNAME	"wcMAD"
#define MAD_WINDOWTITLE	"MAD"

#define DEBUG		"DXDEBUG.LOG"

HINSTANCE			hInstance;
HWND				hMainWindow;

LPDIRECTDRAW lpDD;
LPDIRECTDRAWSURFACE lpDDPrimary;

#ifdef DEBUG
    #define LOG(s) fprintf(f,s); fflush(f);

    FILE* f;
#else
    #define LOG(s) ();
#endif

int flagFullScreen;
char keyDown[128];
void* emergency_exit_procedure;

/*
 * _UINT get_driver_type()
 *
 * EXPORTED
 *
 * This will be called by MAD to ask the driver what type it is. Graphics drivers should return
 * DRV_TYPE_GFX to indicate they are a graphics driver.
 *
 */
UINT
get_driver_type() {
	return DRV_TYPE_GFX;
}

/*
 * _UINT gfx_getcaps()
 *
 * EXPORTED
 *
 * This will be called by MAD to query the capabilities of the driver.
 *
 */
UINT
gfx_getcaps() {
	return GFX_CAPS;
}

/*
 * gfx_reinit()
 *
 * This will reinitialize the DirectX stuff. It is for internal use only.
 *
 */
UINT
gfx_reinit() {
    DDSURFACEDESC ddsd;
 
	/* Create the DirectDraw interface */
	LOG("DirectDrawCreate: ");
	if (DirectDrawCreate (NULL, &lpDD, NULL) != DD_OK) {
		/* This did not work. Return error code 1000: Unable to create DirectDraw object */
		LOG("FAIL\n");
		return 1000;
	}
	LOG("ok\n");

	/* Set the cooperative level */
	LOG("SetCooperativeLevel(): ");
	if (lpDD->SetCooperativeLevel (hMainWindow, (flagFullScreen) ? DDSCL_NORMAL : DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN) != DD_OK) {
	    /* This failed. Return error code 1002: Unable to set cooperative level */
		LOG("FAIL\n");
		return 1003;
	}
	LOG("ok\n");

	/* Now create a primary surface */
	ddsd.dwSize = sizeof (ddsd);
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	LOG("CreateSurface: ");
	if (lpDD->CreateSurface (&ddsd, &lpDDPrimary, NULL) != DD_OK) {
	    /* This did not work. Return error 1001: CreateSurface() failed */
     	LOG("FAIL\n");
		return 1001;
	}
	LOG ("ok\n");

	/* Say it's OK */
	return 0;
}

/*
 * void gfx_kill()
 *
 * This is internally used to deinitialize the driver. It is not exported.
 *
 */
void
gfx_kill() {
	/* Was the DirectDraw object initialized? */
	if (lpDD != NULL) {
		/* Yeah. Was there a primary surface created? */
		if (lpDDPrimary != NULL) {
			/* Yeah, free it*/
			lpDDPrimary->Release();
	    	/* Make sure we don't do this twice */
     		lpDDPrimary = NULL;
		}
		/* Nuke the DirectDraw object */
		lpDD->Release();
		/* Make sure we don't do this twice */
		lpDD = NULL;
	}
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
	gfx_kill();

	/* Is there a main window? */
	if (hMainWindow != NULL) {
		/* Yeah, nuke it */
		DestroyWindow (hMainWindow);
		/* Make sure we don't do this twice */
		hMainWindow = NULL;
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
			             keyDown[LOBYTE (HIWORD (lParam))] = 1;
			             break;
		  case WM_KEYUP: /* User released a key. Handle it! */
			             keyDown[LOBYTE (HIWORD (lParam))] = 0;
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

	/* Set up our window class stuff */
    wc.style = 0;									/* Special style options (see WNDCLASS help) */
	wc.lpfnWndProc = (WNDPROC)MainWindowProc;		/* Window procedure */
	wc.cbClsExtra = 0;								/* No extra memory needed */
	wc.cbWndExtra = 0;								/* No extra memory needed */
	wc.hInstance = hInstance;						/* Instance handle */
	wc.hIcon = LoadIcon (NULL, IDI_APPLICATION);	/* XXX: default icon for now */
	wc.hCursor = NULL;								/* Always hide the cursor in our window */
	wc.hbrBackground = NULL;						/* We handle all painting */
	wc.lpszMenuName = NULL;							/* XXX: no menu */
	wc.lpszClassName = MAD_CLASSNAME;				/* Class name */

	LOG ("RegisterClass: ");
	if (!RegisterClass (&wc)) {
	    /* This failed. Return error 1003: Unable to register window class */
		LOG ("FAIL\n");
		return 1003;
	}
	LOG ("ok\n");

	/* Create the main window */
	LOG("CreateWindow: ");
	hMainWindow = CreateWindow (MAD_CLASSNAME,		/* Class name */
								MAD_WINDOWTITLE,	/* Window title */
								WS_POPUPWINDOW,		/* Window style */
								CW_USEDEFAULT,		/* X postion */
								CW_USEDEFAULT,		/* Y postion */
								CW_USEDEFAULT,		/* Height */
								CW_USEDEFAULT,		/* Width */
								NULL,				/* Parent window */
								NULL,				/* XXX: No menu */
								hInstance,			/* Instance handle */
								NULL);				/* Window creation data (unused) */
    if (hMainWindow == NULL) {
	    /* This failed. Return error 1004: Unable to create the main window */
		LOG ("FAIL\n");
		return 1004;
	}
	LOG ("ok\n");

	/* Show the window */
	ShowWindow (hMainWindow, SW_SHOW);

	/* Pass it through gfx_reinit() */
	return gfx_reinit();
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
gfx_draw(char* source) {
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
	LOG ("Resolve error %u",(_UINT)(errorcode&0xffff));
	return "???";
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
	     /* Yeah, get the message, remove it from the queue and handle it */
         GetMessage (&msg, NULL, 0, 0);
         TranslateMessage (&msg); 
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

/*
 * BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
 *
 * EXPORTED
 *
 * This is the core of the DLL. It will do initialization and stuff.
 *
 */
BOOL APIENTRY
DllMain(HANDLE hModule, DWORD ulReasonForCall, LPVOID lpReserved) {
    /* Are we about to be initialized? */
	if (ulReasonForCall != DLL_PROCESS_ATTACH) {
		/* Nope. Just say it's ok */
		return TRUE;
	}

    /* Save the module handle */
	hInstance = hModule;

	/* Set most pointers to NULL */
	lpDD = NULL; lpDDPrimary = NULL; hMainWindow = NULL; memset (keyDown, 0, sizeof(keyDown));
	emergency_exit_procedure = NULL;

	/* Set default options */
	flagFullScreen = 0;

	/* If we have debugging, open the debug file */
    #ifdef DEBUG
	    if((f=fopen(DEBUG,"wt"))==NULL) {
			MessageBox (NULL, "Unable to create debugging file", "DX", MB_OK);
			return FALSE;
		}
    #endif

	/* Tell Windows it's OK */
	return TRUE;
}