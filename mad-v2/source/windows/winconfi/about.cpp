/*
                            ABOUT.CPP

            MAD Configuration Utility for Windows - About dialog code
                      (c) 1999 The MAD Crew

  Description:
  This file will take care of the "about" dialog of the MAD configuration utility for
  Windows.

  Todo: Nothing

  Portability notes: Unneeded for other platforms
*/
#include <windows.h>
#include "resource.h"

/*
 * BOOL CALLBACK AboutDialogProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
 *
 * This is the window procedure for the about dialog.
 *
 */
BOOL CALLBACK
AboutDialogProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
 	switch (uMsg) {
		   case WM_COMMAND: /* We got a command from Windows. Process it */
			                switch (LOWORD (wParam)) {
							        case IDOK: /* User pushed the OK button. End this dialog */
									           EndDialog (hWnd, 0);
   										       break;								
							}
		                    break;		
                   default: /* Let Windows take care of it */
					        return FALSE;
	}
	/* Tell Windows we processed the event */
	return TRUE;
}

/*
 * dlgSpawnAboutDialog(HWND hWnd);
 *
 * This will spawn the about dialog. The window with handle [hWnd] will be the owner of the
 * window. [hInstance] must be the application's instance handle.
 *
 */
void
dlgSpawnAboutDialog(HINSTANCE hInstance, HWND hWnd) {
	DialogBox (hInstance, MAKEINTRESOURCE (IDD_ABOUTDLG), hWnd, (DLGPROC)AboutDialogProc);
}