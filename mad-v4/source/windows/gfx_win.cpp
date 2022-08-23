/*
                            GFX_WIN.CPP

              Graphix Engine for MAD - Windows Version
			 This one requires at least DirectDraw 2.0

                   (c) 1998, 1999 RiNK SPRiNGER
                           The MAD Crew

  Description:
  This will handle the actual low-level graphics functions. This Windows version also
  does most keyboard input routines.

  Todo: Nothing

  Portability notes: Needs to be fully rewritten for other platforms.
*/
#include <windows.h>
#include <ddraw.h>
#include <dinput.h>
#include "archive.h"
#include "controls.h"
#include "maderror.h"
#include "gfx_drv.h"
#include "mad.h"

HWND hWnd;
extern HINSTANCE hInstance;
static LPDIRECTDRAW lpDD;
static LPDIRECTDRAWSURFACE lpDDSPrimary;
static LPDIRECTDRAWSURFACE lpDDSBack;
static LPDIRECTDRAWPALETTE lpDDPal;
UCHAR got_focus;

unsigned char gfx_keydown[256];

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
     case WM_SYSKEYDOWN: /* Check for ALT keys */
						 gfx_keydown[CONTROLS_KEY_ALT] = 1;
						 /* ROLLOVER */
        case WM_KEYDOWN: /* Check for keys */
                         switch(wParam) {
						 case VK_F12: /* F12 will quit (error 6: emergency exit)*/
				                       die (MAD_ERROR_6);
				                       /* NOTREACHED */
                         }
	                     gfx_keydown[LOBYTE (HIWORD (lParam))] = 1;
	                     break;
       case WM_SYSKEYUP: /* Check for ALT key releases */
						 gfx_keydown[CONTROLS_KEY_ALT] = 0;
						 /* ROLLOVER */
		  case WM_KEYUP: /* A key was released. disable it in the buffer */
	                     gfx_keydown[LOBYTE (HIWORD (lParam))] = 0;
						 break;
    case WM_LBUTTONDOWN: /* left mouse button is down. */
 						 controls->setbuttonstatus(controls->getbuttonstatus() | CONTROLS_LEFTBUTTON);
 						 break;
      case WM_LBUTTONUP: /* left mouse button is up */
					   	 controls->setbuttonstatus(controls->getbuttonstatus() & ~CONTROLS_LEFTBUTTON);
						 break;
	case WM_RBUTTONDOWN: /* left mouse button is down. */
					  	 controls->setbuttonstatus(controls->getbuttonstatus() | CONTROLS_RIGHTBUTTON);
						 break;
	  case WM_RBUTTONUP: /* left mouse button is up */
						 controls->setbuttonstatus(controls->getbuttonstatus() & ~CONTROLS_RIGHTBUTTON);
						 break;
				default: return DefWindowProc (hWnd, uMsg, wParam, lParam);
      }
      /* Tell Windows we processed the event */
      return TRUE;
}

void
GFXDRIVER::init() {
		char blackpal[1024];
		DDSURFACEDESC ddsd;
		DDSCAPS ddscaps;
		WNDCLASS wc;

		/* First NULL out all structures (GFXDRIVER::done() wants this) */
		lpDDPal = NULL; lpDD = NULL; lpDDSPrimary = NULL; lpDDSBack = NULL; got_focus = 1;

		/* First register my class */
		wc.style = 0;										/* No extra styles */
		wc.lpfnWndProc = (WNDPROC)WindowProc;				/* Window procedure */
		wc.cbClsExtra = 0;									/* No extra memory needed */
		wc.cbWndExtra = 0;									/* No extra memory needed */
		wc.hInstance = hInstance;							/* hInstance handle */
		wc.hIcon = LoadIcon (NULL, IDI_APPLICATION);		/* Default icon (for now) */
		wc.hCursor = LoadCursor (NULL, IDC_ARROW);          /* Default cursor (will be hidden) */
		wc.hbrBackground = GetStockObject (BLACK_BRUSH);	/* Black background */
		wc.lpszMenuName = NULL;								/* No menu */
		wc.lpszClassName = "wcMAD_DXGFX";					/* class name */
		/* Register this class */
		if (!RegisterClass(&wc)) {
			/* This failed, so die (shouldnt happend) (error 1000: unable to register class) */
			die (MAD_ERROR_1000);
			/* NOTREACHED */
		}
		/* Create the main window */
		hWnd = CreateWindowEx (0, "wcMAD_DXGFX", "MAD", WS_POPUP, 0, 0,
			                   GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
							   NULL, NULL, hInstance, NULL);
        if (hWnd == NULL) {
			/* This failed, so die (error 1001: unable to create window)*/
			die (MAD_ERROR_1001);
			/* NOTREACHED */
		}
		/* Show the window and give it focus */
        ShowWindow (hWnd, SW_SHOWMAXIMIZED);
        UpdateWindow (hWnd);
        SetFocus (hWnd);

		/* Create the DirectDraw object */
		if (DirectDrawCreate (NULL, &lpDD, NULL) != DD_OK) {
			/* This failed, so die (error 1002: unable to initialize directdraw) */
			die (MAD_ERROR_1002);
			/* NOTREACHED */
		}
		/* Now active the full screen exclusive mode for this window */
		if (lpDD->SetCooperativeLevel (hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT) != DD_OK) {
			/* This failed, so die (error 1003: SetCooperativeLevel() failed) */
			die(MAD_ERROR_1003);
			/* NOTREACHED */
		}
		/* Tell DirectDraw what resolution we want (we always use 256 colors) */		
		if (lpDD->SetDisplayMode (GFX_DRIVER_HRES, GFX_DRIVER_VRES, 8) != DD_OK) {
			/* This failed, so die (error 1004: SetDisplayMode() failed) */
			die (MAD_ERROR_1004);
			/* NOTREACHED */
		}

        /* Create the primary (visible) surface */
        ddsd.dwSize = sizeof (ddsd);
        ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
        ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_COMPLEX | DDSCAPS_FLIP;
        ddsd.dwBackBufferCount = 1;
        if (lpDD->CreateSurface (&ddsd, &lpDDSPrimary, NULL) != DD_OK) {
			/* This failed, so die (error 1005: CreateSurface() failed) */
			die (MAD_ERROR_1005);
			/* NOTREACHED */
		}

		ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
        if (lpDDSPrimary->GetAttachedSurface(&ddscaps, &lpDDSBack) != DD_OK) {
		    /* error 1006: GetAttachedSurface() failed */
			die (MAD_ERROR_1006);
			/* NOTREACHED */
		}

		/* Now nul out the blackpal[] array and create a palette out of it */
		memset(blackpal,0,1024);
		if (lpDD->CreatePalette(DDPCAPS_8BIT, (LPPALETTEENTRY)&blackpal, &lpDDPal, NULL) != DD_OK) {
		       /* error 1007: CreatePalette() failed */
			   die(MAD_ERROR_1007);
			   /* NOTREACHED */
		}

		/* Select the palette */
		if(lpDDSPrimary->SetPalette(lpDDPal)!=DD_OK) {
			   /* error 1008: SetPalette() failed */
			   die(MAD_ERROR_1008);
			   /* NOTREACHED */
		}
}

void
GFXDRIVER::done() {
		/* If there once was a window, destroy it */
	    if (hWnd != NULL) {
	        DestroyWindow (hWnd);
		}
	    /* Was the DirectDraw interface ever initialized? */
	    if (lpDD != NULL) {
		    /* Yes. Was a primary surface ever created? */
			if (lpDDSPrimary != NULL) {
			    /* Yeah. Deinitialize it */
				lpDDSPrimary->Release();
			}
			/* Did we get as far as creating a palette? */
			if (lpDDPal != NULL) {
				/* Yeah. Destroy it */
			    lpDDPal->Release();
			}
			/* And fry the DirectDraw interface class */
			lpDD->Release();
		}
}

void
GFXDRIVER::draw(void *data) {
		RECT r;
		DDSURFACEDESC ddsd;
		HRESULT result;
		char *surface;

		/* If no focus, don't draw */
		if (!got_focus) return;

		/* First lock the surface */
		ddsd.dwSize = sizeof(ddsd);
		result = DDERR_WASSTILLDRAWING;

		/* Now lock the background and retry if it is busy */
		while (result == DDERR_WASSTILLDRAWING) {
            result = lpDDSBack->Lock(NULL, &ddsd, 0, NULL);
	    }
		if (result != DD_OK) {
			/* This failed. die (error 1009: Lock() failed) */
			die (MAD_ERROR_1009);
			/* NOTREACHED */
		}

        /* Create a pointer to the surface buffer */
	    surface=(char *)ddsd.lpSurface;

        /* Dump the data to the surface */   
		memcpy (surface, data, GFX_DRIVER_HRES * GFX_DRIVER_VRES);

		/* Unlock the background buffer */
		lpDDSBack->Unlock(NULL);

		/* And jam it to the screen */
		r.left = 0;
	    r.top = 0;
	    r.right = GFX_DRIVER_HRES;
	    r.bottom = GFX_DRIVER_VRES;

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
				   /* This failed. die (error 1010: Restore() failed) */
				   die (MAD_ERROR_1010);
				   /* NOTREACHED */
			   }
		   }
         }
}

UINT
GFXDRIVER::get_hres() {
        return GFX_DRIVER_HRES;
}

UINT
GFXDRIVER::get_vres() {
        return GFX_DRIVER_VRES;
}

UINT
GFXDRIVER::get_nofcols() {
        return GFX_DRIVER_NOFCOLS;
}

void
GFXDRIVER::setpalette(char *ptr) {
		char tmp[1024];
		UINT i;

		if (lpDDPal == NULL) return;

		/* DirectDraw wants the palette in 4 bytes instead of the standard 3.
		   Convert them */
		for(i=0;i<256;i++) {
			tmp[i*4]=ptr[i*3];						/* Red value */
			tmp[i*4+1]=ptr[i*3+1];					/* Green value */
			tmp[i*4+2]=ptr[i*3+2];					/* Blue value */
			tmp[i*4+3]=0;							/* Flags field */
		}

		/* Update the palette */
		if (lpDDPal->SetEntries(0, 0, 255, (LPPALETTEENTRY)&tmp) != DD_OK) {
			/* This failed, so die (error 1011: SetEntries() failed) */
			    die(MAD_ERROR_1011);
			   /* NOTREACHED */
		}
}

void
GFXDRIVER::poll() {
	/* This will be called whenever the interprinter is idle. It allows the driver to poll
       statuses of devices etc. */
    MSG msg;
	//POINT p;

	/* Is there a message waiting? */
	if (PeekMessage (&msg, NULL, 0, 0, 0)) {
	     /* Yeah, get the message, remove it from the queue and handle it */
         GetMessage (&msg, NULL, 0, 0);
         TranslateMessage (&msg); 
         DispatchMessage (&msg);
	}

    /* Now retrieve the mouse cursor status */
	/*GetCursorPos(&p);
	device_x=(UINT)p.x;
	device_y=(UINT)p.y;  */
}