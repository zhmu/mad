//
// 154.s
//
// script file for a forest screen
//
// (c) 2000 The Hero6 Team
//
#include <stdmad.h>

// the game room
object rm154 : ROOM {
    init()
    run()
    done()
}

external object Hero6 : GAME
external object ego : GAMEOBJ

//
// rm154.init()
//
// This will initialize the room
//
rm154.init() {
    ego.show
    bar_show

    // wait
    loadcursor "hglass.spr" 

    // load the background picture
    loadscreen "154.pic"

    // if ego came from the root script, let him stand up
    if (ego.oldroomno==0) {
        ego.move 127,78
        ego.setanim "standup_e"
        ego.animate

        // while he's standing up, wait
        while (ego.animating==1) refresh

        // face the correct way
        ego.face east
    }

    // if ego came from 169, let him walk accordenly
    if (ego.oldroomno==169) {
       ego.move 320,ego.y
       ego.moveto 275,ego.y

       while (ego.moving==1) refresh

       ego.face west
    }

    // go
    loadcursor "mouse.spr"
} rm154.init()

//
// rm154.run()
//
// This will be called every cycle of the screen
//
rm154.run() {
    // touching the right border?
    if (ego.prioritycode==4) {
        // yup. move to the border
        loadcursor "hglass.spr"

        ego.moveto 320,ego.y
        while (ego.moving==1) refresh

        // move to the new screen
        ego.hide
        ego.teleport 169
        leave
    }

    refresh
} rm154.run()

//
// rm154.done()
//
// This will deinitialize the screen
//
rm154.done() {
} rm154.done()
