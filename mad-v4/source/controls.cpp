/*
                            CONTROLS.CPP

                      Controls Engine for MAD

                    (c) 1999, 2000 The MAD Crew

  Description:
  This will handle input from all devices.

  Todo: Nothing

  Portability notes: Use #ifdef's when code must differ for other platforms.
*/
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "controls.h"
#include "ctrldrv.h"
#include "debugmad.h"
#include "maderror.h"
#include "gadgets.h"
#include "gfx.h"
#include "mad.h"
#include "mplayer.h"
#include "project.h"
#include "types.h"

int controls_quit,controls_busy;

/*
 * CONTROLS::init()
 *
 * This will initialize the controls engine.
 *
 */
void
CONTROLS::init() {
    /* load the system mouse cursor */
    sysmouse_sprno=GFX_NOSPRITE;
    loadcursor(NULL);
    controls_quit = 0; controls_busy = 0;
}

/*
 * CONTROLS::done()
 *
 * This will deinitialize the controls engine.
 *
 */
void
CONTROLS::done() {
}

/*
 * CONTROLS::poll()
 *
 * This will poll the controls engine.
 *
 */
void
CONTROLS::poll() {
    /* poll the controls low-level driver */
    ctrldrv->poll();

    #ifdef MULTIPLAYER
    /* alt-j down? */
    if (ctrldrv_keydown[CONTROLS_KEY_ALT]&&ctrldrv_keydown[CONTROLS_KEY_J]&&(!controls_busy)) {
        /* yup. the controls are busy now (to prevent looping) */
        controls_busy=1;

        /* fire up the join game dialog */
        mplayer->joingamedlg();

        /* release them */
        controls_busy=0;
    }

    /* alt-t down? */
    if (ctrldrv_keydown[CONTROLS_KEY_ALT]&&ctrldrv_keydown[CONTROLS_KEY_T]&&(!controls_busy)) {
        /* yup. the controls are busy now (to prevent looping) */
        controls_busy=1;

        /* fire up the join game dialog */
        mplayer->talkdlg();

        /* release them */
        controls_busy=0;
    }
    #endif

    /* f10 down? */
    if (ctrldrv_keydown[CONTROLS_KEY_F10] && (!controls_quit)) {
        /* yup. seet [controls_quit] to one to prevent looping */
        controls_quit = 1;

        /* wait for the key release */
        waitforelease (CONTROLS_KEY_F10);

        if(dlgman->askyesno(DLGMAN_DIALOG_ASKEXIT,DLGMAN_DIALOG_ASKEXITITLE)) {
            mad_exit();
            /* NOTREACHED */
        }
        
        controls_quit = 0;
    }
}

/*
 * CONTROLS::getxpos()
 *
 * This will return the current X position of the device.
 *
 */
_UINT
CONTROLS::getxpos() {
    return ctrldrv->getxpos();
}

/*
 * CONTROLS::getypos()
 *
 * This will return the current Y position of the device.
 *
 */
_UINT
CONTROLS::getypos() {
    return ctrldrv->getypos();
}

/*
 * CONTROLS::button1pressed()
 *
 * This will return zero if button 1 is not pressed, otherwise non-zero.
 *
 */
_UCHAR
CONTROLS::button1pressed() {
    return(ctrldrv->getbuttonstat()&CONTROLS_BUTTON1);
}

/*
 * CONTROLS::button2pressed()
 *
 * This will return zero if button 2 is not pressed, otherwise non-zero.
 *
 */
_UCHAR
CONTROLS::button2pressed() {
    return(ctrldrv->getbuttonstat()&CONTROLS_BUTTON2);
}

/*
 * CONTROLS::waitnobutton()
 *
 * This will wait until no button is touched.
 *
 */
void
CONTROLS::waitnobutton() {
   _ULONG now = time(NULL) + CONTROLS_TIMEOUT;
   while (button1pressed()||button2pressed()) {
       idler();

       if (time(NULL) > now) return;
   }
}

/*
 * CONTROLS::keydown()
 *
 * This will return zero if no keys are down, otherwise non-zero.
 *
 */
_UCHAR
CONTROLS::keydown() {
    _UCHAR i;

    /* scan the entire keydown[] array for a 1 value */
    for(i=0;i<128;i++) {
        if(ctrldrv_keydown[i]) return 1;
    }
    /* no keys down */
    return 0;
}

/*
 * CONTROLS::clearkey(_UCHAR no)
 *
 * This will mark key [no] as released.
 *
 */
void
CONTROLS::clearkey(_UCHAR no) {
    ctrldrv_keydown[no]=0;
}

/*
 * CONTROLS::waitevent()
 *
 * This waits until something happends (device does something).
 *
 */
void
CONTROLS::waitevent() {
    _UCHAR done;

    done=0;
    while (!done) {
	idler();

        /* some key on the keyboard hit? */
        if (keydown()) {
            done=1;
        }
        /* did the user click a button? */
        if ((button1pressed())||(button2pressed())) {
            /* yes. wait until he releases it and continue */
            waitnobutton();
            done=1;
        }
    }
}

/*
 * CONTROLS::resolvecode(_UCHAR code)
 *
 * This will resolve keyboard code [code] into a human-readable charachter.
 *
 */
char
CONTROLS::resolvecode(_UCHAR code) {
    char i;

    /* wait until the key is released */
    while (iskeydown (code)) poll();

    i=0;
    switch(code) {
        /* alfabet */
        case CONTROLS_KEY_A: i='a'; break;
        case CONTROLS_KEY_B: i='b'; break;
        case CONTROLS_KEY_C: i='c'; break;
        case CONTROLS_KEY_D: i='d'; break;
        case CONTROLS_KEY_E: i='e'; break;
        case CONTROLS_KEY_F: i='f'; break;
        case CONTROLS_KEY_G: i='g'; break;
        case CONTROLS_KEY_H: i='h'; break;
        case CONTROLS_KEY_I: i='i'; break;
        case CONTROLS_KEY_J: i='j'; break;
        case CONTROLS_KEY_K: i='k'; break;
        case CONTROLS_KEY_L: i='l'; break;
        case CONTROLS_KEY_M: i='m'; break;
        case CONTROLS_KEY_N: i='n'; break;
        case CONTROLS_KEY_O: i='o'; break;
        case CONTROLS_KEY_P: i='p'; break;
        case CONTROLS_KEY_Q: i='q'; break;
        case CONTROLS_KEY_R: i='r'; break;
        case CONTROLS_KEY_S: i='s'; break;
        case CONTROLS_KEY_T: i='t'; break;
        case CONTROLS_KEY_U: i='u'; break;
        case CONTROLS_KEY_V: i='v'; break;
        case CONTROLS_KEY_W: i='w'; break;
        case CONTROLS_KEY_X: i='x'; break;
        case CONTROLS_KEY_Y: i='y'; break;
        case CONTROLS_KEY_Z: i='z'; break;
    }
    /* did this resolve? */
    if(i) {
        /* yeah, check shift state */
        if((ctrldrv_keydown[CONTROLS_KEY_LSHIFT])||(ctrldrv_keydown[CONTROLS_KEY_RSHIFT])) {
            /* shift is active. make it uppercase */
            i=toupper(i);
        }
        /* return it */
        return i;
    }
    switch(code) {
        /* numbers */
        case CONTROLS_KEY_0: i='0'; break;
        case CONTROLS_KEY_1: i='1'; break;
        case CONTROLS_KEY_2: i='2'; break;
        case CONTROLS_KEY_3: i='3'; break;
        case CONTROLS_KEY_4: i='4'; break;
        case CONTROLS_KEY_5: i='5'; break;
        case CONTROLS_KEY_6: i='6'; break;
        case CONTROLS_KEY_7: i='7'; break;
        case CONTROLS_KEY_8: i='8'; break;
        case CONTROLS_KEY_9: i='9'; break;
    }
    /* did this resolve? */
    if(i) {
        /* yeah, check shift state */
        if((ctrldrv_keydown[CONTROLS_KEY_LSHIFT])||(ctrldrv_keydown[CONTROLS_KEY_RSHIFT])) {
            /* shift is active. make it uppercase */
            switch(i) {
                case '0': return ')';
                case '1': return '!';
                case '2': return '@';
                case '3': return '#';
                case '4': return '$';
                case '5': return '%';
                case '6': return '^';
                case '7': return '&';
                case '8': return '*';
                case '9': return '(';
            }
        }
        /* return the char */
        return i;
    }
    switch(code) {
        /* special chars */
        case CONTROLS_KEY_MINUS: i='-'; break;
        case CONTROLS_KEY_EQUAL: i='='; break;
     case CONTROLS_KEY_LBRACKET: i='['; break;
     case CONTROLS_KEY_RBRACKET: i=']'; break;
        case CONTROLS_KEY_COLON: i=';'; break;
        case CONTROLS_KEY_TILDE: i='`'; break;
        case CONTROLS_KEY_COMMA: i=','; break;
          case CONTROLS_KEY_DOT: i='.'; break;
        case CONTROLS_KEY_SLASH: i='/'; break;
        case CONTROLS_KEY_SPACE: i=' '; break;
    }
    /* did this resolve? */
    if(i) {
        /* yeah, check shift state */
        if((ctrldrv_keydown[CONTROLS_KEY_LSHIFT])||(ctrldrv_keydown[CONTROLS_KEY_RSHIFT])) {
            /* shift is active. make it uppercase */
            switch(i) {
                case '-': return '_';
                case '=': return '+';
                case '[': return '{';
                case ']': return '}';
                case ';': return ':';
                case '`': return '~';
                case ',': return '<';
                case '.': return '>';
                case '/': return '?';
            }
        }
        /* return the new char */
        return i;
    }
    /* we can't resolve this. return zero */
    return 0;
}

/*
 * CONTROLS::getch()
 *
 * This will return the first typed char from the keyboard buffer, but only
 * resolvable ones will be returned.
 *
 */
_UCHAR
CONTROLS::getch() {
    _UCHAR i,j;

    for(i=0;i<128;i++) {
        if(ctrldrv_keydown[i]) {
            j=resolvecode(i);
            if (j) {
                ctrldrv_keydown[i]=0;
                return i;
            }
        }
    }
    return 0;
}

/*
 * CONTROLS::peekcode()
 *
 * This will scan for the first typed char, but not remove it from the buffer.
 *
 */
_UCHAR
CONTROLS::peekcode() {
    _UCHAR i;
    _UCHAR j;

    for(i=0;i<128;i++) {
        if(ctrldrv_keydown[i]) {
            j=resolvecode(i);
            if (j) return i;
        }
    }
    return 0;
}

/*
 * CONTROLS::iskeydown(_UCHAR no)
 *
 * This will return zero if key [no] is not pushed, otherwise non-zero.
 *
 */
_UCHAR
CONTROLS::iskeydown(_UCHAR no) {
    return ctrldrv_keydown[no];
}

/*
 * CONTROLS::waitforelease(_UCHAR code)
 *
 * This will wait until key [code] is released.
 *
 */
void
CONTROLS::waitforelease(_UCHAR code) {
    while (ctrldrv_keydown[code]) poll();
}

/*
 * CONTROLS::loadcursor(char* fname)
 *
 * This will load a mouse cursor. If [fname] is NULL, the system mouse cursor
 * will be used.
 *
 */
void
CONTROLS::loadcursor(char* fname) {
    char  tempstr[MAD_TEMPSTR_SIZE];

    /* got a valid old cursor? */
    if (sysmouse_sprno!=GFX_NOSPRITE) {
        /* yup free it */
        gfx->unloadsprite(sysmouse_sprno);
        /* make sure we don't do it again */
        sysmouse_sprno=GFX_NOSPRITE;
    }

    /* null supplied? */
    if (fname==NULL) {
        /* yup. load the system mouse cursor */
        strcpy(tempstr,project->info.sysmouse);
        strcat(tempstr,MAD_SYSMOUSEXT);
    } else {
        /* no. use our filename */
        strcpy(tempstr,fname);
    }

    sysmouse_sprno=gfx->loadsprite(tempstr);
    gfx->set_mouse_spriteno(sysmouse_sprno);
}
