//
// title.scr
//
// script file for the title screen of the demo
//
// (c) 2000 The Hero6 Team
//
#include <stdmad.h>

// the hero
object objHero : GAMEOBJ {
    handlevent()
}

// the game room
object rmTitle : ROOM {
    init()
    done()
    run()
}

// the demo button
object btnDemo : GAMEOBJ {
    handlevent()
}

// the credits button
object btnCredits : GAMEOBJ {
    handlevent()
}

// the exit button
object btnExit : GAMEOBJ {
    handlevent()
}

// the credits text
object txtCredits : TEXT {
}

external object ego : GAMEOBJ

//
// btnDemo.handlevent()
//
// This will be called whenever the demo button is clicked
//
btnDemo.handlevent() {
    // use the hourglass mouse pointer
    loadcursor "hglass.spr"

    // show the button go 'down' and come back 'up' again
    btnDemo.show
    refresh
    delay 250
    btnDemo.hide
    refresh
    delay 250

    // go to the pool room
    ego.teleport 102
    leave
} btnDemo.handlevent()

//
// btnCredits.handlevent()
//
// This will be called whenever the credits button is clicked
//
btnCredits.handlevent() {
    // show the button go 'down' and come back 'up' again
    btnCredits.show
    refresh
    delay 250
    btnCredits.hide
    refresh
    delay 250

    // initialize the credits text
    txtCredits.initext "<system>"
    txtCredits.loadtext 1
    txtCredits.setalignment 2
    txtCredits.setcolor 0xffff00
    txtCredits.setspeed 1,1,2
    txtCredits.move 160,200
    txtCredits.moveto 160,_500
    txtCredits.show
} btnCredits.handlevent()

//
// btnExit.handlevent()
//
// This will be called whenever the exit button is clicked
//
btnExit.handlevent() {
    // show the button go 'down' and come back 'up' again
    btnExit.show
    refresh
    delay 250
    btnExit.hide
    refresh
    delay 250

    // quit
    exit
} btnExit.handlevent()

//
// rmTitle.init()
//
// This will initialize the title screen
//
rmTitle.init() {
    // initialize the hero
    objHero.loadanimation "dhero.anm"
//    objHero.move 172,54
    objHero.move 174,54
    objHero.setanim "animating"
    objHero.setloop 1
    objHero.setspeed 1,1,15
    objHero.animate
    objHero.show

    // bind every action to it
    objHero.bindaction 0,handlevent

    // initialize the demo button
    btnDemo.loadanimation "d_but.anm"
//    btnDemo.move 244,49
    btnDemo.move 236,50
    btnDemo.sethalt "demo"
    btnDemo.face south
    btnDemo.hide
    btnDemo.bindaction 0,handlevent

    // initialize the credits button
    btnCredits.loadanimation "d_but.anm"
//    btnCredits.move 237,71
    btnCredits.move 237,73
    btnCredits.sethalt "credits"
    btnCredits.face south
    btnCredits.hide
    btnCredits.bindaction 0,handlevent

    // initialize the exit button
    btnExit.loadanimation "d_but.anm"
//    btnExit.move 243,93
    btnExit.move 236,100
    btnExit.sethalt "exit"
    btnExit.face south
    btnExit.hide
    btnExit.bindaction 0,handlevent

    // load the background picture
    loadscreen "title.pic"
} rmTitle.init()

//
// objHero.handlevent()
//
// This will handle the hero events
//
objHero.handlevent() {
    // show a nice message when the user clicks on the hero
    dlg_alert 9
} objHero.handlevent()

//
// rmTitle.run()
//
// This will be called every cycle of the title screen
//
rmTitle.run() {
    refresh
} rmTitle.run()

//
// rmTitle.done()
//
// This will deinitialize the title screen
//
rmTitle.done() {
    // nuke all objects
    objHero.unload
    btnDemo.unload
    btnCredits.unload
    btnExit.unload
    txtCredits.unload
} rmTitle.done()
