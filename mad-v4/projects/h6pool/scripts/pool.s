//
// pool.s
//
// script file for the title screen of the demo
//
// (c) 2000 The Hero6 Team
//
#include <stdmad.h>

object objGreenleaf : GAMEOBJ {
    handlevent()
    talk_to()
    look_at()
    use_it()
}

object objPoolStream : GAMEOBJ {
    handlevent()
}

object objTears : GAMEOBJ {
}

// the game room
object rmPool : ROOM {
    init()
    run()
    done()
    genLook()
    genUse()
    genTalk()
}

external object Hero6 : GAME
external object ego : GAMEOBJ

//
// objGreenleaf.talk_to()
//
// This will load the talking screen of greenleaf
//
objGreenleaf.talk_to() {
    // teleport to the greanleaf chat scene :-)
    ego.teleport 103
    leave
} objGreenleaf.talk_to()

//
// objGreenleaf.use_it()
//
// This will show the use messages for greenleaf
//
objGreenleaf.use_it() {
    dlg_alert 52+random{2}
} objGreenleaf.use_it()

//
// objGreenleaf.look_at()
//
// This will show the looking at messages for greenleaf
//
objGreenleaf.look_at() {
    dlg_alert 54+random{2}
} objGreenleaf.look_at()

//
// rmPool.genLook()
//
// This is the generic Look procedure
//
rmPool.genLook() {
    dlg_alert 50
} rmPool.genLook()

//
// rmPool.genUse()
//
// This is the generic Use procedure
//
rmPool.genUse() {
    dlg_alert 52
} rmPool.genUse()

//
// rmPool.genTalk()
//
// This is the generic Talk procedure
//
rmPool.genTalk() {
    dlg_alert 51
} rmPool.genTalk()

//
// rmPool.init()
//
// This will initialize the room
//
rmPool.init() {
    rmPool.leaving=0

    ego.show
    bar_show
    inv_enable

    // bind the generic actions to the room
    bar_bindaction 2,rmPool.genLook
    bar_bindaction 3,rmPool.genUse
    bar_bindaction 4,rmPool.genTalk

    // initialize greenleaf
    objGreenleaf.loadanimation "gleaf.anm"
    objGreenleaf.sethalt "standing"
    objGreenleaf.setmove "walking"
    objGreenleaf.setspeed 2,2,4
    objGreenleaf.face south
    objGreenleaf.show
    objGreenleaf.setclip no
    objGreenleaf.setpriority 2
    objGreenleaf.move 61,90

    objGreenleaf.bindaction 2,look_at
    objGreenleaf.bindaction 3,use_it
    objGreenleaf.bindaction 4,talk_to

    // initialize the pool stream
    objPoolStream.loadanimation "poolstr.anm"
    objPoolStream.setspeed 1,1,10
    objPoolStream.move 94,114
    objPoolStream.setanim "flow_day"
    objPoolStream.setloop 1
    objPoolStream.animate
    objPoolStream.setpriority 3
    objPoolStream.show

    // initialize the statue's tears
    objTears.loadanimation "tears.anm"
    objTears.setspeed 1,1,10
    objTears.move 49,62
    objTears.setanim "tears_flow"
    objTears.setloop 1
    objTears.animate
    objTears.setpriority 3
    objTears.show

    // load the background picture
    loadscreen "poolday.pic"

    // if we came from the title pic, do this intro thingy
    if (ego.oldroomno==100) {
        // make greenleaf walk to us
        objGreenleaf.move 170,208
        objGreenleaf.moveto 61,90

        // set ego up correctly
        ego.move 70,80
        ego.sethalt "lying_e"
        ego.face south

        // activate the hourglass mouse cursor
        loadcursor "hglass.spr"
        bar_hide

        // wait until greenleaf has moved to us
        while (objGreenleaf.moving==1) refresh

        // set greenleaf up correctly
        objGreenleaf.setanim "kneeling_east"
        objGreenleaf.animate

        // wait until greenleaf has stopped petting us
        while (objGreenleaf.animating==1) refresh

        // make sure greenleaf looks in the correct way
        ego.face west

        // let greenleaf tell us stuff
        dlg_alert 10

        // do the narrator
        dlg_alert 35

        // restore the mouse cursor
        loadcursor "mouse.spr"
        bar_show

        // now, pay greenleaf a visit :)
        ego.teleport 103
        rmPool.leaving=1
        return
    }

    // haven't gotten up yet?
    if (Hero6.flags==0) {
        // yup. activate the hourglass mouse cursor
        loadcursor "hglass.spr"
        bar_hide

        // show the ego char slowly getting up
        ego.setspeed 2,2,5
        ego.setanim "standup_e"
        ego.animate
        while (ego.animating==1) refresh

        // set him up
        ego.move 100,80
        ego.setmove "walking"
        ego.sethalt "standing"
        ego.face west
        ego.setspeed 2,2,3

        // restore the mouse cursor
        loadcursor "mouse.spr"
        bar_show

        // now he has
        Hero6.flags=1
    }
} rmPool.init()

//
// rmPool.run()
//
// This will be called every cycle of the pool screen
//
rmPool.run() {
    // are we about to leave?
    if (rmPool.leaving==1) {
        // yup. do it
        leave
        return
    }

    // there's a 1/150th chance of greenleaf combing her hair
    if (random{150}==16) {
        // if greanleaf isn't animating, make her!
        if (objGreenleaf.animating==0) {
            objGreenleaf.setanim "hair_anim"
            objGreenleaf.animate
        }
    }
    refresh
} rmPool.run()

//
// rmPool.done()
//
// This will deinitialize the title screen
//
rmPool.done() {
    // nuke all objects
    objGreenleaf.unload
    objPoolStream.unload
    objTears.unload
} rmPool.done()
