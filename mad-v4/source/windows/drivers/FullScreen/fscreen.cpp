/*
                         MAD DirectX Windows Driver
                        (c) 1999, 2000 The MAD Crew

  Description:
  This will handle all graphics functions. It requires DirectX 3.0. This version
  will also do the keyboard and mouse handling.
*/
#pragma hdrstop

#define NOTCPP extern "C"
#define EXPORTED __declspec(dllexport)

#include <stdio.h>
#include <windows.h>
#include <ddraw.h>
#include <dinput.h>
#include "controls.h"
#include "ctrldrv.h"
#include "gfx_drv.h"
#include "maderror.h"

/* This driver will do the keyboard handling and aiming device handling */
#define GFX_CAPS GFX_DRIVER_CAPS_DEFAULT | GFX_DRIVER_CAPS_ALSOKBD | GFX_DRIVER_CAPS_ALSODEV

HINSTANCE hInstance;
HWND hWnd;
static LPDIRECTDRAW lpDD;
static LPDIRECTDRAWSURFACE lpDDSPrimary;
static LPDIRECTDRAWSURFACE lpDDSBack;

UCHAR got_focus;
UCHAR keydown[256];
UINT button_state, mouse_installed;
signed int device_x, device_y;
RECT rScreen, rWindow, rClient;

void *emergency_exit_proc;

BOOL WINAPI DllMain (HINSTANCE hInst, ULONG ul_reason_for_call, LPVOID lpReserved) {
    /* Did the user initialize us? */
	if (ul_reason_for_call != DLL_PROCESS_ATTACH) {
		/* No, so just return OK status */
		return 1;
	}
	
    /* save the hInstance handle */
	hInstance = hInst;
	/* Reset variables */
	button_state = 0; device_x = 0; device_y = 0;
    lpDD = NULL; lpDDSPrimary = NULL; lpDDSBack = NULL; got_focus = 1;
    emergency_exit_proc = NULL;

    /* Check for possible installation of a mouse */
	mouse_installed = GetSystemMetrics (SM_MOUSEPRESENT);
	if ((mouse_installed == TRUE) || (mouse_installed != 0)) {
	    mouse_installed = 1;
	} else {
	    mouse_installed = 0;
	}

	/* Clear out the keydown[] array */
	memset(keydown,0,256);
	
	/* say all went ok */
	return 1;
}

LRESULT CALLBACK WindowProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
   switch (uMsg) {      
	  case WM_KILLFOCUS: /* we lost the focus! set variable we did */
                         got_focus = 0;
		                 break;
	  case WM_SETFOCUS: /* we regained the focus! set variable we did */
						 got_focus = 1;
		                 break;
      case WM_SETCURSOR: /* Always hide the cursor in our window */
		                 SetCursor (NULL);
		                 break;
     case WM_SYSKEYDOWN: /* ALT key was pressed with some other key. Enable ALT state */
						 keydown[CONTROLS_KEY_ALT] = 1;
						 /* ROLLOVER */
        case WM_KEYDOWN: /* Check for keys */
                         switch(wParam) {
						     case VK_F12: /* Is [emergency_exit_proc] not equal to NULL? */
							              if (emergency_exit_proc != NULL) {
								   		      /* Yeah, spawn the procedure */
                                              void (*myfunc)();
                                              (void*)myfunc = (void*)emergency_exit_proc;
                                              (*myfunc)();
									      }
									      /* Otherwise just ignore it */
							              break;
                         }
	                     keydown[LOBYTE (HIWORD (lParam))] = 1;
	                     break;
       case WM_SYSKEYUP: /* for ALT key was pressed and a key was release. Disable ALT state */
						 keydown[CONTROLS_KEY_ALT] = 0;
						 /* ROLLOVER */
		  case WM_KEYUP: /* A key was released. disable it in the buffer */
	                     keydown[LOBYTE (HIWORD (lParam))] = 0;
						 break;
    case WM_LBUTTONDOWN: /* left mouse button is down. Set bit in [button_state] */
		                 /* Is DirectInput not active? */
							 /* Yeah, change it */
                             button_state |= 1;
						 break;
      case WM_LBUTTONUP: /* left mouse button is up. Clear bit in [button_state] */
                             button_state &= ~1;
						 break;
	case WM_RBUTTONDOWN: /* right mouse button is down. Set bit in [button_state] */
                             button_state |= 2;
						 break;
	  case WM_RBUTTONUP: /* right mouse button is up. Clear bit in [button_state] */
                             button_state &= ~2;
						 break;
			    default: /* We don't trap this. Let Windows take care of it */
					     return DefWindowProc (hWnd, uMsg, wParam, lParam);
      }
      /* Tell Windows we processed the event */
      return TRUE;
}

UINT
gfx_kill() {
	    /* Was the DirectDraw interface ever initialized? */
	    if (lpDD != NULL) {
		    /* Yes. Was a primary surface ever created? */
			if (lpDDSPrimary != NULL) {
			    /* Yeah. Deinitialize it */
				lpDDSPrimary->Release();
			}
			/* And fry the DirectDraw interface class */
			lpDD->Release();
		}        

		/* Tell the user it's all right */
		return 0;
}

UINT
gfx_reinit() {
		DDSURFACEDESC ddsd;
		DDSCAPS ddscaps;
		LPDIRECTDRAWCLIPPER clipper;
		char blackpal[1024];
		HRESULT i;

		/* Nuke the old interfaces */
		if ((i = gfx_kill())) {
			/* Oops, this failed. Return error code */
			return 1234;
		}
		/* Create the DirectDraw object */
		if (DirectDrawCreate (NULL, &lpDD, NULL) != DD_OK) {
			/* This failed, return error 1002: unable to initialize directdraw */
			return 1002;
		}

        /* Active the full screen exclusive mode for this window */
        if (lpDD->SetCooperativeLevel (hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT | DDSCL_ALLOWMODEX) != DD_OK) {
			    /* This failed, so return error 1003: SetCooperativeLevel() failed */
			    return 1003;
		    }
		    /* Tell DirectDraw what resolution we want (we always use 256 colors) */		
            if ((i = lpDD->SetDisplayMode (GFX_DRIVER_HRES, GFX_DRIVER_VRES, 32)) != DD_OK) {
				/* This failed, return error 1004: SetDisplayMode() failed */
			    return 1004;
	        }
            /* Create the primary (visible) surface */
            ddsd.dwSize = sizeof (ddsd);
            ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
            ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_COMPLEX | DDSCAPS_FLIP;
            ddsd.dwBackBufferCount = 1;
		    if ((i = lpDD->CreateSurface (&ddsd, &lpDDSPrimary, NULL)) != DD_OK) {
			    /* This failed, so return error 1005: CreateSurface() failed */
			    return 1005;
		    }
		    ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
            if ((i = lpDDSPrimary->GetAttachedSurface(&ddscaps, &lpDDSBack)) != DD_OK) {
		        /* return error 1006: GetAttachedSurface() failed */
                return 1006;
		    }
		/* Now our DirectDraw initialization is okey-dokey */

		/* Oh boy, everything worked! Return ok status */
		return 0;
}

NOTCPP UINT EXPORTED
gfx_init() {
		WNDCLASS wc;
		UINT cx, cy;
		
		/* First register my class */
		wc.style = CS_HREDRAW | CS_VREDRAW;                 /* Redraw if anything changes */
		wc.lpfnWndProc = (WNDPROC)WindowProc;				/* Window procedure */
		wc.cbClsExtra = 0;									/* No extra memory needed */
		wc.cbWndExtra = 0;									/* No extra memory needed */
		wc.hInstance = hInstance;							/* hInstance handle */
		wc.hIcon = LoadIcon (NULL, IDI_APPLICATION);		/* Default icon (for now) */
		wc.hCursor = LoadCursor (NULL, IDC_ARROW);          /* Default cursor (will be hidden) */
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);	    /* Default window background */
		wc.lpszMenuName = NULL;								/* No menu */
		wc.lpszClassName = "wcMAD_DXGFX";					/* class name */
		/* Register this class */
		if (!RegisterClass(&wc)) {
			/* This failed, return error 1000: unable to register class */
			return 1000;
		}
		/* Calculate position */
		cx = GFX_DRIVER_HRES + GetSystemMetrics (SM_CXSIZEFRAME) * 2;
		cy = GFX_DRIVER_VRES + GetSystemMetrics (SM_CYSIZEFRAME) * 2; /* + GetSystemMetrics (SM_CYMENU);
		/* Create the main window */
        hWnd = CreateWindowEx (0, "wcMAD_DXGFX", "MAD", 0, 0, 0,
                               0, 0,
							   NULL, NULL, hInstance, NULL);
        if (hWnd == NULL) {
			/* This failed, return error error 1001: unable to create window */
			return 1001;
		}
		/* Show the window and give it focus */
        ShowWindow (hWnd, SW_HIDE);
        UpdateWindow (hWnd);
        SetFocus (hWnd);

		/* Get window info */
		GetWindowRect (hWnd, &rWindow);

		/* Return status of gfx_reinit() */
		return gfx_reinit();
}

NOTCPP UINT EXPORTED
gfx_done() {
		/* If there once was a window, destroy it */
	    if (hWnd != NULL) {
	        DestroyWindow (hWnd);
		}
		/* Return the status of gfx_kill() */
		return gfx_kill();
}

NOTCPP UINT EXPORTED
gfx_draw(void *data) {
	RECT r;
	DDSURFACEDESC ddsd;
	HRESULT result;
	HRESULT i;
	char *surface;
	char tempstr[128];

	/* If no focus, don't draw, but just return ok status */
    if (!got_focus) return 0;

	/* Initialize the coordinates */
	r.left = 0;
	r.top = 0;
	r.right = GFX_DRIVER_HRES;
	r.bottom = GFX_DRIVER_VRES;
		        
		/* If the lpDDSBack object is NULL, just return ok status */
		if (lpDDSBack == NULL) return 0;

		/* First lock the surface */
		ddsd.dwSize = sizeof(ddsd);
		result = DDERR_WASSTILLDRAWING;

		/* Now lock the background and retry if it is busy */
		while (result == DDERR_WASSTILLDRAWING) {
	        result = lpDDSBack->Lock(NULL, &ddsd, 0, NULL);
	    }
		if (result != DD_OK) {
			/* This failed. return error 1009: Lock() failed */
			return 1009;
		}
	
        /* Create a pointer to the surface buffer */
	    surface=(char *)ddsd.lpSurface;
	
    	/* Dump the data to the surface */   
        memcpy (surface, data, GFX_DRIVER_HRES * GFX_DRIVER_VRES * 4);
		
		/* Unlock the background buffer */
		lpDDSBack->Unlock(NULL);
	
    	/* Go blit */
		lpDDSPrimary->BltFast (0, 0, lpDDSBack, &r, DDBLTFAST_NOCOLORKEY);
	 
    	while(1) {
		    /* Do a page flip */
           	result = lpDDSPrimary->Flip( NULL, 0 );

           	/* If this succeeded, exit the loop */
	     	if (result == DD_OK) break;
           
		    /* If the surface was lost, find it */
		    if(result == DDERR_SURFACELOST) {		
		        if (lpDDSPrimary->Restore() != DD_OK) {
				    /* This failed. return error 1010: Restore() failed */
				    return 1010;
			    }
		    }
        }
	/* Oh boy, everything went ok. return ok status */
	return 0;
}

NOTCPP UINT EXPORTED
gfx_get_hres() {
        return GFX_DRIVER_HRES;
}

NOTCPP UINT EXPORTED
gfx_get_vres() {
        return GFX_DRIVER_VRES;
}

NOTCPP UINT EXPORTED
gfx_get_nofcols() {
        return GFX_DRIVER_NOFCOLS;
}

NOTCPP UINT EXPORTED
gfx_setpalette(char *ptr) {
		/* Return ok status */
		return 0;
}

NOTCPP UINT EXPORTED
gfx_setquithandler(void *proc) {
	/* This will specify which procedure needs to be called whenever the emergency exit
	   key was hit */
	
	/* Is [proc] NULL? */
	if (proc == NULL) {
	    /* Yeah. We'd better not call this. Return error 1012: Cannot use a NULL void as emergency handler */
		return 1012;
	}
	emergency_exit_proc = proc;

	/* All went ok. return ok status */
	return 0;
}

NOTCPP UINT EXPORTED
gfx_getcaps() {
	/* This will return the capabilities of this driver */
	return GFX_CAPS;
}

NOTCPP EXPORTED char*
gfx_resolverror(UINT no) {
	switch (no) {
	    case 1000: return MAD_ERROR_1000;
	    case 1001: return MAD_ERROR_1001;
	    case 1002: return MAD_ERROR_1002;
	    case 1003: return MAD_ERROR_1003;
	    case 1004: return MAD_ERROR_1004;
	    case 1005: return MAD_ERROR_1005;
	    case 1006: return MAD_ERROR_1006;
	    case 1007: return MAD_ERROR_1007;
	    case 1008: return MAD_ERROR_1008;
	    case 1009: return MAD_ERROR_1009;
	    case 1010: return MAD_ERROR_1010;
	    case 1011: return MAD_ERROR_1011;
	    case 1012: return MAD_ERROR_1012;
	}
	return MAD_ERROR_UNKNOWN;
}

NOTCPP EXPORTED UINT
get_driver_type() {
	/* We are a graphics driver, so return DRV_TYPE_GFX */
	return DRV_TYPE_GFX;
}

NOTCPP EXPORTED void
gfx_poll() {
	/* This will be called whenever the interprinter is idle. It allows the driver to poll
       statuses of devices etc. */
    MSG msg;
	POINT p,p2;
	
	DIDEVICEOBJECTDATA data;
	HRESULT i;
	DWORD dwElements;
	BOOL bDone;
	UCHAR c;

	/* Is there a message waiting? */
	if (PeekMessage (&msg, NULL, 0, 0, 0)) {
	     /* Yeah, get the message, remove it from the queue and handle it */
         GetMessage (&msg, NULL, 0, 0);
         TranslateMessage (&msg); 
         DispatchMessage (&msg);
	}

    /* Now retrieve the mouse cursor status */

//        GetWindowOrgEx(hWnd, &p2);
	    GetCursorPos(&p);
	    device_x=(UINT)p.x; device_y=(UINT)p.y;
}

NOTCPP EXPORTED void
gfx_get_keyboard_status(char *dest) {
	/* The interprinter wants the status of the controls. Copy it to the [dest] buffer */
	memcpy (dest, keydown, 256);
}

NOTCPP EXPORTED void
gfx_get_device_status(UINT *x, UINT *y, UINT *stat) {
	/* The interprinter wants the status of the buttons. Give it to him */
	*x = device_x; *y = device_y; *stat = button_state;
}
