//
// gl_dial.s
//
// script file for the dialogue screen of greenleaf
//
// (c) 2000 The Hero6 Team
//
#include <stdmad.h>

object objEyes : GAMEOBJ {
}

object objMouth : GAMEOBJ {
    handlevent()
}

// the game room
object rmGreenleaf : ROOM {
    init()
    run()
    done()
    dlg_handler()
}

external object ego : GAMEOBJ

//
// rmGreenleaf.init()
//
// This will initialize the room
//
rmGreenleaf.init() {
    // unbind the actions
    bar_bindaction 2,NULL
    bar_bindaction 3,NULL
    bar_bindaction 4,NULL

    rmGreenleaf.leaving=0
    rmGreenleaf.dialog_text=0
    rmGreenleaf.dialog_base=0
    rmGreenleaf.dialog_active=1

    ego.hide
    loadcursor "mouse.spr"
    bar_hide

    // initialize the eyes
    objEyes.loadanimation "gleaf.anm"
    objEyes.setanim "eyes_blink"
    objEyes.setspeed 1,1,10
    objEyes.animate
    objEyes.move 84,38
    objEyes.hide

    // initialize the mouth
    objMouth.loadanimation "gleaf.anm"
    objMouth.setanim "mouth_move"
    objMouth.setspeed 1,1,10
    objMouth.move 57,47
    objMouth.setloop 1
    objMouth.animate

    // load the screen
    loadscreen "gl_dlg.pic"

    // tell the dialog manager what to call when text is chosen
    rmGreenleaf.dlg_bindresult dlg_handler

    // fire up the conversation
    dlg_converse 136,10,0xff00ff,0xffffff,11
} rmGreenleaf.init()

//
// rmGreenleaf.run()
//
// This will be called every cycle of the greenleaf screen
//
rmGreenleaf.run() { 
    refresh

    // do we have to leave?
    if (rmGreenleaf.leaving==1) {
        // yup, get out of here (leaving isn't allowed in a dialog routine)
        leave
        return
    }

    // if we are not animating anymore, hide the eyes
    if (objEyes.animating==0) objEyes.hide

    // there's a 1/50th chance of greenleaf closing her eye every cycle
    if (random{50}==1) {
        objEyes.animate
        objEyes.show
    }
} rmGreenleaf.run()

//
// rmGreenleaf.done()
//
// This will deinitialize the greenleaf screen
//
rmGreenleaf.done() {
    // nuke all objects
    objEyes.unload
    objMouth.unload
} rmGreenleaf.done()

//
// rmGreenleaf.dlg_handler()
//
// This will be called whenever the user selects something in the
// communications dialog
//
rmGreenleaf.dlg_handler() {
    // ask about gl menu?
    if (rmGreenleaf.dialog_text==1) {
        // yup. enough already?
        if (dlg_choice{}==2) {
            // yup. do it
            rmGreenleaf.dialog_text=0
            rmGreenleaf.dialog_base=0
            rmGreenleaf.dialog_active=0
        }
    }

    // ask about settlements menu?
    if (rmGreenleaf.dialog_text==2) {
        // yup. enough already?
        if (dlg_choice{}==1) {
            // yup. do it
            rmGreenleaf.dialog_text=10001
            rmGreenleaf.dialog_base=6
            rmGreenleaf.dialog_active=0
        }
    }

    // ask about albion/helmsdale/duke menu?
    if (rmGreenleaf.dialog_text==3) {
        // yup. enough already?
        if (dlg_choice{}==3) {
            // yup. do it
            rmGreenleaf.dialog_text=10002
            rmGreenleaf.dialog_base=8
            rmGreenleaf.dialog_active=0
        }
    }

    // ask about monster?
    if (rmGreenleaf.dialog_text==4) {
        // yup. enough already?
        if (dlg_choice{}==1) {
            // yup. do it
            rmGreenleaf.dialog_text=10000
            rmGreenleaf.dialog_base=0
            rmGreenleaf.dialog_active=0
        }
    }

    // ask about valley?
    if (rmGreenleaf.dialog_text==5) {
        // yup. enough already?
        if (dlg_choice{}==1) {
            // yup. do it
            rmGreenleaf.dialog_text=10004
            rmGreenleaf.dialog_base=12
            rmGreenleaf.dialog_active=0
        }
    }

    // asking about trouble?
    if (rmGreenleaf.dialog_text==6) {
        // yup. enough already?
        if (dlg_choice{}==1) {
            // yup. cycle menu
            rmGreenleaf.dialog_text=10005
            rmGreenleaf.dialog_base=13
            rmGreenleaf.dialog_active=0
        }
    }

    // asking about duke etc?
    if (rmGreenleaf.dialog_text==7) {
        // yup. enough already?
        if (dlg_choice{}==2) {
            // yup. cycle menu
            rmGreenleaf.dialog_text=10006
            rmGreenleaf.dialog_base=14
            rmGreenleaf.dialog_active=0
        }
    }

    // asking about someone?
    if (rmGreenleaf.dialog_text==8) {
        // yup. enough already?
        if (dlg_choice{}==1) {
            // yup. cycle menu
            rmGreenleaf.dialog_text=10007
            rmGreenleaf.dialog_base=16
            rmGreenleaf.dialog_active=0
        }
    }

    // generic menu
    if (rmGreenleaf.dialog_text==0) {
        // goodbye?
        if (dlg_choice{}==4) {
            // yup. move to the correct room and leave
            ego.teleport 102
            rmGreenleaf.leaving=1
        }
    }

    // need to show the text?
    if (rmGreenleaf.dialog_active==1) {
        // yup. show it
        objMouth.show
        dlg_showtext 136,10,30+dlg_choice{}+rmGreenleaf.dialog_base,120
        // were we doing the asking about trouble?
        if (dlg_choice{}+rmGreenleaf.dialog_base==14) {
            // yes. do the second part too
            dlg_showtext 136,10,45,120
        }
        objMouth.hide
    }

    // make sure next cycle will do show text
    rmGreenleaf.dialog_active=1

    // now, handle the menu changes

    // generic menu
    if (rmGreenleaf.dialog_text==0) {
        // asking about name
        if (dlg_choice{}==1) {
            // yup. cycle menu
            rmGreenleaf.dialog_text=10001
            rmGreenleaf.dialog_base=6
        }
        // asking about how he got here?
        if (dlg_choice{}==3) {
            // yup. cycle menu
            rmGreenleaf.dialog_text=10004
            rmGreenleaf.dialog_base=12
        }
    }

    // ask about gl/tell about yourself menu
    if (rmGreenleaf.dialog_text==1) {
        // telling about yourself?
        if (dlg_choice{}==1) {
            // yup. cycle menu
            rmGreenleaf.dialog_text=10002
            rmGreenleaf.dialog_base=8
        }
    }

    // ask about settlements?
    if (rmGreenleaf.dialog_text==2) {
        // yup. asking about settlements?
        if (dlg_choice{}==0) {
            // yup. cycle menu
            rmGreenleaf.dialog_text=10003
            rmGreenleaf.dialog_base=9
        }
    }

    // asking about monster?
    if (rmGreenleaf.dialog_text==4) {
        // yup. asking about the monster?
        if (dlg_choice{}==0) {
            // yup. cycle menu
            rmGreenleaf.dialog_text=10005
            rmGreenleaf.dialog_base=13
        }
    }

    // asking about valley?
    if (rmGreenleaf.dialog_text==5) {
        // yup. asking about the valley?
        if (dlg_choice{}==0) {
            // yup. cycle menu
            rmGreenleaf.dialog_text=10006
            rmGreenleaf.dialog_base=14
        }
    }

    // asking about trouble?
    if (rmGreenleaf.dialog_text==6) {
        // yup. asking about trouble?
        if (dlg_choice{}==0) {
            // yup. cycle menu
            rmGreenleaf.dialog_text=10007
            rmGreenleaf.dialog_base=16
         }
    }

    // asking about duke/golems/etc?
    if (rmGreenleaf.dialog_text==7) {
        // yup. asking about golems?
        if (dlg_choice{}==1) {
            // yup. cycle menu
            rmGreenleaf.dialog_text=10008
            rmGreenleaf.dialog_base=18
        }
    }

    // fix the dialog text number
    if (rmGreenleaf.dialog_text>9999) {
        rmGreenleaf.dialog_text=rmGreenleaf.dialog_text-10000
    }

    // aren't we leaving yet?
    if (rmGreenleaf.leaving==0) {
       // yup. fire up the conversation
       dlg_converse 136,10,0xff00ff,0xffffff,11+rmGreenleaf.dialog_text
    }
} rmGreenleaf.dlg_handler()
