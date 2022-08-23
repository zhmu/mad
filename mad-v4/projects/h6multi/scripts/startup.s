//
// startup.s
//
// script file for the Hero6 MultiPlayer demo!
//
// (c) 2000 The Hero6 Team
//
#include <stdmad.h>

//
// ************************************************************************
//  superclasses
// ************************************************************************
//
object GAME : GAME {
    init()
    run()
    done()
    controls()
    magic()
    restart()
}

object GAMEOBJ : GAMEOBJ {
    handlevent()
    talk_to()
}

object ROOM : ROOM {
    init()
    run()
    done()
    dlg_handler()
}

object TEXT : TEXT {
}

//
// ************************************************************************
//  the shared objects
// ************************************************************************
//

// the game object
object Hero6 : GAME {
    init()
    run()
    done()
    controls()
    magic()
}

// the ego char
object ego : GAMEOBJ {
}

// the system room
object sysRoom : ROOM {
    dlg_handler()
}

//
// ************************************************************************
//  superclasses default actions
// ************************************************************************
//

//
// GAME.init()
//
// This will be called whenever a game object doesn't have an 'init' procedure
//
GAME.init() {
} GAME.init()

//
// GAME.run()
//
// This will be called whenever a game object doesn't have a 'run' procedure
//
GAME.run() {
} GAME.run()

//
// GAME.done()
//
// This will be called whenever a game object doesn't have a 'done' procedure
//
GAME.done() {
} GAME.done()

//
// GAME.controls()
//
// This will be called whenever a game object doesn't have a 'controls' procedure
//
GAME.controls() {
} GAME.controls()

//
// GAME.magic()
//
// This will be called whenever a game object doesn't have a 'magic' procedure
//
GAME.magic() {
} GAME.magic()

//
// GAME.restart()
//
// This will be called whenever a game object doesn't have a 'restart' procedure
//
GAME.restart() {
} GAME.restart()

//
// GAMEOBJ.handlevent()
//
// This will be inherited if a 'gameobj' object doesn't have a 'handlevent' method.
//
GAMEOBJ.handlevent() {
} GAMEOBJ.handlevent()

//
// GAMEOBJ.talk_to()
//
// This will be inherited if a 'gameobj' object doesn't have a 'talk_to' method.
//
GAMEOBJ.talk_to() {
} GAMEOBJ.talk_to()

//
// ROOM.run()
//
// This will be inherited if a 'room' object doesn't have a 'run' method.
//
ROOM.run() {
} ROOM.run()

//
// ROOM.dlg_handler()
//
// This is the default dialog handler.
//
ROOM.dlg_handler() {
} ROOM.dlg_handler()

//
// ************************************************************************
//  game methods
// ************************************************************************
//

//
// sysRoom.dlg_handler()
//
// This is the dialog handler for the system room.
//
sysRoom.dlg_handler() {
   if (dlg_choice{}==3) {
       exit
   }
} sysRoom.dlg_handler()

//
// Hero6.controls()
//
// This will be called whenever the user clicks on the controls icon in the
// icon bar.
//
Hero6.controls() {
    // fire up the panel
//    dlg_converse 80,80,0xff00ff,0xffffff,0
} Hero6.controls()

//
// Hero6.magic()
//
// This will be called whenever the user clicks on the magic icon in the icon
// bar.
//
Hero6.magic() {
    // show the 'man, that sucks' message (for this demo)
//    dlg_alert 2
} Hero6.magic()

//
// Hero6.init()
//
// This will initialize the Hero6 demo game
//
Hero6.init() {
    // initialize the hero
    ego.loadanimation "hero.anm"
    ego.setspeed 3,2,2
    ego.setmove "walking"
    ego.sethalt "standing"
    ego.setpriority 2
    ego.face south
    ego.setmoveobj
    ego.hide

    // start in room 154 (forest pic)
    ego.teleport 154

    // initialize the icon bar
    bar_addicon "walk","b_walk.spr",32,4,2,NULL
    bar_addicon "look","b_look.spr",60,5,0,NULL
    bar_addicon "use","b_use.spr",92,4,0,NULL
    bar_addicon "talk","b_talk.spr",125,5,0,NULL
    bar_addicon "magic","b_magic.spr",161,4,4,Hero6.magic
    bar_addicon "sys","b_sys.spr",264,4,4,Hero6.controls

    // give the hero some generic items
    inv_disable

    // bind dialog results here to the dialog manager
    sysRoom.dlg_bindresult dlg_handler

    // set up the game
    Hero6.quit=0
    Hero6.flags=0

    // keep loading scripts if needed
    while (Hero6.quit==0) {
        // handle the scripts
        if (ego.roomno==154) invoke "154.m"
        if (ego.roomno==169) invoke "169.m"
    }

    // get outta here
    exit
} Hero6.init()

//
// Hero6.run()
//
// This will be called in every cycle of the Hero6 demo game
//
Hero6.run() {
    refresh
} Hero6.run()

//
// Hero6.done()
//
// This will deinitialize the Hero6 demo game
//
Hero6.done() {
} Hero6.done()
