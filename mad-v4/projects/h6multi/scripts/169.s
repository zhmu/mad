//
// 169.s
//
// script file for a forest screen
//
// (c) 2000 The Hero6 Team
//
#include <stdmad.h>

// the game room
object rm169 : ROOM {
    init()
    run()
    done()
}

external object Hero6 : GAME
external object ego : GAMEOBJ

//
// rm169.init()
//
// This will initialize the room
//
rm169.init() {
    ego.show
    bar_show

    // load the background picture
    loadscreen "169.pic"

    // if ego came from the root script, let him stand up
    if (ego.oldroomno==154) {
        // wait
        loadcursor "hglass.spr"

        ego.move _10,ego.y
        ego.moveto 10,ego.y

        // let ego walk
        while (ego.moving==1) refresh

        // face the correct way
        ego.face east
    }

    // go
    loadcursor "mouse.spr"
} rm169.init()

//
// rm169.run()
//
// This will be called every cycle of the screen
//
rm169.run() {
    // touching the right border?
    if (ego.prioritycode==5) {
        // yup. move to the new screen
        ego.teleport 154
        leave
    }

    refresh
} rm169.run()

//
// rm169.done()
//
// This will deinitialize the screen
//
rm169.done() {
} rm169.done()
