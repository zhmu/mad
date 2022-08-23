/*
                              PARSER.CPP

                           MAD Input Parser

                       (c) 1999 The MAD Crew

  Description:
  This will handle the parser input from the keyboard.

  Todo: Nothing

  Portability notes: Fully portable, no changes required
*/
#include <stdio.h>
#include "controls.h"
#include "debugmad.h"
#include "dlgman.h"
#include "fontman.h"
#include "game.h"
#include "gfx.h"
#include "gfx_drv.h"
#include "inv.h"
#include "mad.h"
#include "parser.h"
#include "project.h"

#define  PARSER_TITLEBAR
#define  PARSER_TITLE "Enter your input"

/*
 * PARSER::init()
 *
 * This will initialize the parser.
 *
 */
void
PARSER::init() {
    _UINT i,flags;
    _UCHAR mulfact;

    i=fontman->getfontheight(fontman->getsysfontno());

    /* create a dialog */
    #ifdef PARSER_TITLEBAR
    flags=DLGMAN_DIALOG_FLAG_TITLEBAR|DLGMAN_DIALOG_FLAG_MOVEABLE;mulfact=3;
    #else
    flags=0;mulfact=2;
    #endif /* PARSER_TITLEBAR */
    dialog_no=dlgman->createdialog(flags|DLGMAN_DIALOG_FLAG_NODEFAULT,PARSER_SPAREX,GFX_DRIVER_VRES-(i*mulfact),i,GFX_DRIVER_HRES-(2*PARSER_SPAREX));

    /* create the text box */
    text_no=dlgman->dialog[dialog_no]->createcontrol(DLGMAN_CONTROL_TYPE_TEXTINPUT);

    #ifdef PARSER_TITLEBAR
    dlgman->dialog[dialog_no]->setitle(PARSER_TITLE);
    #endif

    /* initialize the control */
    dlgman->dialog[dialog_no]->control[text_no]->move(0,0);
    dlgman->dialog[dialog_no]->control[text_no]->resize(i-DLGMAN_BORDER_HEIGHT,dlgman->dialog[dialog_no]->getwidth()-DLGMAN_BORDER_WIDTH-2);
    dlgman->dialog[dialog_no]->control[text_no]->setfocus(1);
    dlgman->dialog[dialog_no]->control[text_no]->setcaption("");
}

/*
 * PARSER::done()
 *
 * This will deinitialize the parser.
 *
 */
void
PARSER::done() {
    /* destroy the dialog */
    dlgman->destroydialog(dialog_no);
}

/*
 * PARSER::handlevent()
 *
 * This will give the parser time to do its own things.
 *
 */
void
PARSER::handlevent() {
    _UCHAR c;

    #ifdef DEBUG_AVAILABLE
    /* don't pop up while debugging */
    if (debug_active) return;
    #endif /* DEBUG_AVAILABLE */

    /* if the user pressed CONTROLS_HOTKEY_SAVEGAME, spawn the save_game()
       procedure */
    if(controls->iskeydown(CONTROLS_HOTKEY_SAVEGAME)) {
        /* nuke the keystroke */
        controls->clearkey(CONTROLS_HOTKEY_SAVEGAME);
        /* spawn the save game procedure */
        save_game();
    }

    /* if the user pressed CONTROLS_HOTKEY_LOADGAME, spawn the load_game()
       procedure */
    if(controls->iskeydown(CONTROLS_HOTKEY_LOADGAME)) {
        /* nuke the keystroke */
        controls->clearkey(CONTROLS_HOTKEY_LOADGAME);
        /* spawn the load game procedure */
        load_game();
    }

    /* if the user pressed CONTROLS_HOTKEY_INVENTORY, spawn the inventory
       dialog */
    if(controls->iskeydown(CONTROLS_HOTKEY_INVENTORY)) {
        /* nuke the keystroke */
        controls->clearkey(CONTROLS_HOTKEY_INVENTORY);
        /* spawn the load game procedure */
        inv->show_inventory();
    }

    /* is the parser disabled in the project? */
    if(project->info.flags&PROJECT_FLAG_NOPARSER) {
        /* yeah, get outta here */
        return;
    }

    /* did the user press anything on the controls? */
    if(controls->keydown()) {
         /* is it a parsable thing? */
         c=controls->resolvecode(controls->peekcode());
         if(c) {
             /* yeah. pop up our window */
             if(dlgman->dodialog(dialog_no)!=DLGMAN_DIALOG_NOHIT) {
             }
         }
    }
}
