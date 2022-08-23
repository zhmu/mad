/*
                            MAIN.CPP

                MAD Configuration Utility for Windows
                     (c) 1999 The MAD Crew

  Description:
  This is a configuration utility for MAD. It will allow the user to select which graphics
  driver he wants to use, and configurate that driver.

  Todo: Add control drivers to this utility as well.

  Portability notes: Unneeded for other platforms
*/
#include <stdio.h>
#include <windows.h>
#include <commdlg.h>
#include "resource.h"
#include "../../gfx_drv.h"
#include "../../mad.h"

#define APPNAME "MAD Windows Setup"

#define MAX_TEMPSTR_LEN 512
#define MAX_FILENAME_LEN 512

#ifdef DRV_OK
#undef DRV_OK
#endif

#define DRV_OK          0
#define DRV_LOADERROR   1 
#define DRV_NOGFXDRV    2

#define MAX_DRIVER_DATA_LEN 64

#define MSG_GFXDRVLOADERR "Unable to load GFX driver '%s': %s"

#define DRV_ERROR0 "everything is ok"
#define DRV_ERROR1 "unable to load driver"
#define DRV_ERROR2 "this is not a MAD graphics driver"

#define DRV_ERRORUNK "unknown error. help"

#define MSG_GFXDRVFILE "Graphics driver: "
#define MSG_GFXDRVRES "Resolution supported: %ux%u with %u colors"
#define MSG_GFXDRVNODRV "<no driver>"
#define MSG_REGOPENFAILED "Unable to open registry key"

#define DLG_GFXDRVTYPE "MAD graphics drivers"
#define DLG_GFXALLTYPE "All files"
#define DLG_OPENTITLE "Select MAD driver to use"
#define DLG_DEFAULTEXT "dll"

#define REG_ROOTKEY "Software\\The MAD Crew\\MAD"
#define REG_GFXDRVFILE "GFXDriverFile"
#define REG_GFXCFGDATA "GFXConfig"
#define REG_CLASSNAME "MAD"

#define DEF_GFXDRVFILE "\0"

HMODULE gfxdrv;
HINSTANCE hInstance;

char gfxDriverFile[MAX_FILENAME_LEN];
char gfxDriverData[MAX_DRIVER_DATA_LEN];

typedef UINT (*GFXFUNC)();

typedef UINT (*GFXSPAWNCFGDIALOG)(HWND hParent, char *buf);
typedef UINT (*GFXSETCONFIG)(char *buf);

GFXFUNC get_driver_type,gfx_get_hres,gfx_get_vres,gfx_get_nofcols,gfx_getcaps;
GFXSPAWNCFGDIALOG gfx_spawnconfigdialog;
GFXSETCONFIG gfx_setconfig;
UINT gfx_caps;

extern void dlgSpawnAboutDialog(HINSTANCE hInstance, HWND hWnd);

/*
 *
 * dlgSetGFXDriverName (HWND hWnd, char *name);
 *
 * This will set the driver name to [name] or a message indicating no driver has
 * been loaded. [hWnd] should be the handle of the dialog on which the label resides
 *
 */
void
dlgSetGFXDriverName (HWND hWnd, char *name) {
    char tempstr[MAX_TEMPSTR_LEN];

	/* Did the user supply a valid name? */
    if ((name != NULL) && (name[0] != '\0')) {
		/* Yeah, display it */
	    sprintf(tempstr, "%s%s",MSG_GFXDRVFILE, name);
	} else {
		/* Otherwise say no driver installed */
		sprintf(tempstr, "%s%s",MSG_GFXDRVFILE, MSG_GFXDRVNODRV);
	}
	/* Copy the text to the control */
    SetDlgItemText (hWnd, IDC_GFXDRV, tempstr);
}

/*
 * dlgSetGFXResolution (HWND hWnd);
 *
 * This will set the driver's resolution or to zero's if no driver is loaded. [hWnd] should
 * be the handle on which the label resides
 *
 */
void
dlgSetGFXResolution (HWND hWnd) {
    char tempstr[MAX_TEMPSTR_LEN];

    /* Are the driver's API's valid? */
	if ((gfx_get_hres != NULL) && (gfx_get_vres != NULL) && (gfx_get_nofcols != NULL)) {
		/* Yeah, copy the actual information */
	    sprintf(tempstr, MSG_GFXDRVRES, gfx_get_hres(), gfx_get_vres(), gfx_get_nofcols());
	} else {
		/* No, just display a bunch of zero's */
	    sprintf(tempstr, MSG_GFXDRVRES, 0, 0, 0);
	}
	/* Copy the text to the control */
    SetDlgItemText (hWnd, IDC_GFXRES, tempstr);
}

/*
 * dlgEnableDisableControl (HWND hWnd, UINT CTRLID, UCHAR state);
 *
 * This will enable/disable control [CTRLID] on window handle [hWnd]. If [state] is
 * zero, it will disable the control, otherwise the control will be enabled.
 *
 */
void
dlgEnableDisableControl (HWND hWnd, UINT CTRLID, UCHAR state) {
    EnableWindow (GetDlgItem (hWnd, CTRLID), state);
}

/*
 * dlgSetForDriver (HWND hWnd);
 *
 * This will update the dialog whose handle is in [hWnd] according to the driver's
 * capabilities and state.
 *
 */
void
dlgSetForDriver (HWND hWnd) {
	/* Was the actual driver loaded? */
	if (gfxdrv != NULL) {
		/* Yeah, pass the driver name */
	    dlgSetGFXDriverName (hWnd, gfxDriverFile);
	} else {
		/* No, pass a NULL name */
	    dlgSetGFXDriverName (hWnd, NULL);
	}
    dlgSetGFXResolution (hWnd);

    /* Enable/disable the Configuration button depending on capabilities */
	dlgEnableDisableControl (hWnd, IDC_GFXCFG, (gfx_caps & GFX_DRIVER_CAPS_CFG));
}

/*
 * UCHAR dlgGetDriverName(HWND hWnd, char *destfile);
 *
 * This will pop-up a dialog which will ask for a driver filename. The owning window
 * handle should be in [hWnd]. The filename will be copied to [destfile], which should
 * be MAX_FILENAME_LEN bytes. If the user presses the Cancel button, zero is returned,
 * otherwise a non-zero number.
 *
 */
UCHAR
dlgGetDriverName(HWND hWnd, char *destfile) {
    OPENFILENAME ofn;
	char filename[MAX_FILENAME_LEN];

	memset(filename, 0, MAX_FILENAME_LEN);				/* Null out filename */

	ofn.lStructSize = sizeof (OPENFILENAME);			/* size of the structure */
	ofn.hwndOwner = hWnd;								/* Window that owns it */
	ofn.hInstance = NULL;								/* Unneeded instance handle */
	ofn.lpstrFilter = DLG_GFXDRVTYPE"\0*.dll\0"DLG_GFXALLTYPE"\0*.*\0\0"; /* Filter */
	ofn.lpstrCustomFilter = NULL;						/* Unneeded custom filter */
	ofn.nMaxCustFilter = 0;								/* Unneeded custom filter size */
	ofn.nFilterIndex = 1;								/* Use first filter */
	ofn.lpstrFile = filename;						    /* Buffer to copy filename to */
	ofn.nMaxFile = MAX_FILENAME_LEN;					/* Max length of filename */
	ofn.lpstrFileTitle = NULL;							/* Unneeded filename buffer */
	ofn.lpstrInitialDir = NULL;							/* Just use current directory */
	ofn.lpstrTitle = DLG_OPENTITLE;						/* Dialog title to use */
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;	/* Flags */
	ofn.lpstrDefExt = DLG_DEFAULTEXT;					/* Default extension */
	
    if (GetOpenFileName (&ofn) == FALSE) {
		/* User pressed cancel button. Return 0 */
		return 0;
	}
	/* Copy the filename selected to [destfile] */
	strcpy (destfile, filename);
	/* Return 1 to indicate success */
	return 1;
}

/*
 * Cleanup(HWND hWnd);
 *
 * This will preform a cleanup. It will unload the graphics driver, and update the controls
 * on dialog [hWnd].
 *
 */
void
Cleanup(HWND hWnd) {
    /* Disable configuration buttons */
    dlgSetGFXDriverName (hWnd, NULL);
	dlgEnableDisableControl (hWnd, IDC_GFXCFG, 0);
	dlgSetGFXResolution (hWnd);

    /* Clear all graphics driver functions */
    get_driver_type = NULL; gfx_get_hres = NULL; gfx_get_vres = NULL;
	gfx_get_nofcols = NULL; gfx_getcaps = NULL; gfx_spawnconfigdialog = NULL;

	/* Was the graphics driver loaded? */
	if (gfxdrv != NULL) {
		/* Yeah, unload it */
		FreeLibrary (gfxdrv);
      	gfxdrv = NULL;

	}
}

/*
 * UINT gfxLoadDriver (char *drvname);
 *
 * This will load driver [drvname]. If anything fails, it will return one of the DRV_xxx
 * error codes. If all goes well, it will return DRV_OK.
 *
 */
UINT
gfxLoadDriver (char *drvname) {
	/* Load the graphics driver */
	if ((gfxdrv = LoadLibrary (drvname)) == NULL) {
        /* Oops, this failed. Return error code DRV_LOADERROR */
		return DRV_LOADERROR;
 	}
    /* Get the get_driver_type() function */
	if ((get_driver_type = (GFXFUNC)GetProcAddress (gfxdrv, "get_driver_type")) == NULL) {
		/* This failed. Return error code DRV_NOGFXDRV */
		return DRV_NOGFXDRV;
	}
	/* Check whether this is a MAD graphics driver */
	if ((unsigned short)get_driver_type() != (unsigned short)DRV_TYPE_GFX) {
		/* It isn't. Return error code DRV_NOGFXDRV */
		return DRV_NOGFXDRV;
    }
    /* Get the gfx_get_hres() function */
	if ((gfx_get_hres = (GFXFUNC)GetProcAddress (gfxdrv, "gfx_get_hres")) == NULL) {
		/* This failed. Return error code DRV_NOGFXDRV */
		return DRV_NOGFXDRV;
	}
    /* Get the gfx_get_vres() function */
	if ((gfx_get_vres = (GFXFUNC)GetProcAddress (gfxdrv, "gfx_get_vres")) == NULL) {
		/* This failed. Return error code DRV_NOGFXDRV */
		return DRV_NOGFXDRV;
	}
    /* Get the gfx_get_nofcols() function */
	if ((gfx_get_nofcols = (GFXFUNC)GetProcAddress (gfxdrv, "gfx_get_nofcols")) == NULL) {
		/* This failed. Return error code DRV_NOGFXDRV */
		return DRV_NOGFXDRV;
	}
	/* Get the gfx_get_caps() function */
	if ((gfx_getcaps = (GFXFUNC)GetProcAddress (gfxdrv, "gfx_getcaps")) == NULL) {
		/* This failed. Return error code DRV_NOGFXDRV */
		return DRV_NOGFXDRV;
	}
	gfx_caps = gfx_getcaps();
    /* Does this driver support the configuration? */
    if (gfx_caps & GFX_DRIVER_CAPS_CFG) {
		/* Yeah. Get the gfx_spawnconfigdialog() function */
	    if ((gfx_spawnconfigdialog = (GFXSPAWNCFGDIALOG)GetProcAddress (gfxdrv, "gfx_spawnconfigdialog")) == NULL) {
		    /* This failed. Return error code DRV_NOGFXDRV */
		    return DRV_NOGFXDRV;
	    }
		/* And the gfx_setconfig() function */
	    if ((gfx_setconfig = (GFXSETCONFIG)GetProcAddress (gfxdrv, "gfx_setconfig")) == NULL) {
		    /* This failed. Return error code DRV_NOGFXDRV */
		    return DRV_NOGFXDRV;
	    }
	}
	/* Wow, this all seemed to be ok. Return DRV_OK */
	return DRV_OK;
}

/*
 * char *gfxResolvError (UINT errcode);
 *
 * This will return a string describing error code [errcode].
 *
 */
char
*gfxResolvError (UINT errcode) {
    switch (errcode) {
	           case DRV_OK: return DRV_ERROR0;
        case DRV_LOADERROR: return DRV_ERROR1;
		 case DRV_NOGFXDRV: return DRV_ERROR2;
	}
	return DRV_ERRORUNK;
}

/*
 * madGetSettings();
 *
 * This will retrieve the current graphics driver and its settings from the registry. If no
 * registry entry exists, it will create one and initialize it with the default settings.
 *
 */
void
madGetSettings() {
    HKEY regkey;
	DWORD i;

	memset(&gfxDriverData, 0, MAX_DRIVER_DATA_LEN);
	
	/* Try to open/create the key */
	if (RegCreateKeyEx (HKEY_CURRENT_USER, REG_ROOTKEY, 0, REG_CLASSNAME, 0, KEY_ALL_ACCESS, NULL, &regkey, &i) == ERROR_SUCCESS) {
		/* What did Windows excactly do? */
		if (i == REG_CREATED_NEW_KEY) {
			/* Oohhh, Windows created the key for us! That's nice. Set the default settings in it */
			strcpy (gfxDriverFile, DEF_GFXDRVFILE);
			RegSetValueEx (regkey, REG_GFXDRVFILE, 0, REG_SZ, (BYTE *)&gfxDriverFile, sizeof (gfxDriverFile) + 1);
			RegSetValueEx (regkey, REG_GFXCFGDATA, 0, REG_BINARY, (BYTE *)&gfxDriverData, MAX_DRIVER_DATA_LEN);
		} else {
			/* Copy the old settings from the registry */
			i = sizeof (gfxDriverFile) + 1;
			RegQueryValueEx (regkey, REG_GFXDRVFILE, 0, NULL, (BYTE *)&gfxDriverFile, &i);
			i = MAX_DRIVER_DATA_LEN;
			RegQueryValueEx (regkey, REG_GFXCFGDATA, 0, NULL, (BYTE *)&gfxDriverData, &i);
		}
	}
	RegCloseKey (regkey);
}

/*
 * madSetSettings();
 *
 * This will copy the current settings to the registry.
 *
 */
void
madSetSettings() {
    HKEY regkey;

	/* Try to open/create the key */
	if (RegOpenKeyEx (HKEY_CURRENT_USER, REG_ROOTKEY, 0, KEY_ALL_ACCESS, &regkey) == ERROR_SUCCESS) {
        /* Dump the settings to the registry */
		RegSetValueEx (regkey, REG_GFXDRVFILE, 0, REG_SZ, (BYTE *)&gfxDriverFile, sizeof (gfxDriverFile) + 1);
		RegSetValueEx (regkey, REG_GFXCFGDATA, 0, REG_BINARY, (BYTE *)&gfxDriverData, MAX_DRIVER_DATA_LEN);
	} else {
		/* Oops, this failed. Report it */
		MessageBox (NULL, MSG_REGOPENFAILED, APPNAME, MB_OK);
	}
	/* Close the key */
	RegCloseKey (regkey);
}

/*
 * BOOL CALLBACK MainDialogProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
 *
 * This is the window procedure for the main dialog.
 *
 */
BOOL CALLBACK
MainDialogProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    char filename[MAX_FILENAME_LEN];
    char tempstr[MAX_TEMPSTR_LEN];
    UINT i;

 	switch (uMsg) {
	    case WM_INITDIALOG: /* We have been initialized! Set the labels to the correct values */
			                dlgSetForDriver (hWnd);
			                break;
		   case WM_COMMAND: /* We got a command from Windows. Process it */
			                switch (LOWORD (wParam)) {
							        case IDOK: /* User pushed the OK button */
										       madSetSettings();
									           DestroyWindow (hWnd);
										       break;
							    case IDCANCEL: /* User pushed the cancel button. */
									           DestroyWindow (hWnd);
											   break;
							   case IDC_ABOUT: /* User pushed the about button */
								               dlgSpawnAboutDialog (hInstance, hWnd);
								               break;
							  case IDC_GFXCFG: /* Launch the configuration dialog */
								               i = gfx_spawnconfigdialog (hWnd, (char *)&gfxDriverData);
								               break;
						case IDC_GFXSELECTDRV: /* Launch the select driver dialogbox */
                                               if (dlgGetDriverName (hWnd, filename)) {
                                                   /* Store driver name */
												   strcpy (gfxDriverFile, filename);												   
												   /* Cleanup old driver */
												   Cleanup(hWnd);
	                                               /* Load the driver */
	                                               if ((i = gfxLoadDriver (gfxDriverFile)) != DRV_OK) {
		                                               /* We couldn't load the driver. Inform user */
		                                               sprintf (tempstr, MSG_GFXDRVLOADERR, gfxDriverFile, gfxResolvError (i));
		                                               MessageBox  (NULL, tempstr, APPNAME, MB_OK);
													   break;
												   }
												   dlgSetForDriver(hWnd);
											   }
											   break;
										
							}
		                    break;		
			 case WM_CLOSE: /* The window was closed. Cleanup and quit */
   	       case WM_DESTROY: /* Windows is about to destroy our window. Cleanup and quit */
			                Cleanup(hWnd);
			                PostQuitMessage (0);
			                break;
                   default: /* Let Windows take care of it */
					        return FALSE;
	}
	/* Tell Windows we processed the event */
	return TRUE;
}

/*
 * int WINAPI WinMain (HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
 *
 * This is the main procedure.
 *
 */
int WINAPI WinMain (HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	UINT result;
    char tempstr[MAX_TEMPSTR_LEN];

	/* Save the instance handle */
	hInstance = hInst;

    /* Get the previous settings */
	madGetSettings();

	/* Make sure everything is tidy when we begin */
    Cleanup (NULL);

	/* Is there a valid driver name supplied? */
	if ((gfxDriverFile != NULL) && (gfxDriverFile[0] != '\0')) {
	    /* Yeah, load the driver */
     	if ((result = gfxLoadDriver (gfxDriverFile)) != DRV_OK) {
		     /* We couldn't load the driver. Inform user */
		     sprintf (tempstr, MSG_GFXDRVLOADERR, gfxDriverFile, gfxResolvError (result));
		     MessageBox  (NULL, tempstr, APPNAME, MB_OK);
	     } else {
             /* Does this driver support the configuration? */
             if (gfx_caps & GFX_DRIVER_CAPS_CFG) {
			     /* Yeah. Tell the graphics driver the settings as we know them */
			     gfx_setconfig((char *)&gfxDriverData);
			 }
		 }
    }
	
	result = DialogBox (hInstance, MAKEINTRESOURCE (IDD_MAINDLG), NULL, (DLGPROC)MainDialogProc);

	return result;
}