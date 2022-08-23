//  
// stdmad.h -- standard MAD include file that defines a lot o' stuff
//
// (c) 1999, 2000 The MAD Crew
//
//
classdef GAME {
    init() flags 1
    done() flags 2
    run() flags 4
    controls() flags 0
    magic() flags 0
    restart() flags 8
    quit() propno 0x80
    score() propno 0x81
    dialog_choice() propno 0x82
    flags() propno 0x83
}

classdef ROOM {
    init() flags 1
    done() flags 2
    run() flags 4
    dlg_handler() flags 0
    genLook() flags 0
    genUse() flags 0
    genTalk() flags 0
    dlg_bindresult(%p) kernelfunc 0x1d
    leaving() propno 0x80
    dialog_text() propno 0x81
    dialog_base() propno 0x82
    dialog_active() propno 0x83
}

classdef GAMEOBJ {
    setanim(%q) kernelfunc 0x01
    setframe(%e) kernelfunc 0x02
    setmove(%q) kernelfunc 0x03
    moveto(%e,%e) kernelfunc 0x04
    setspeed(%w,%w,%w) kernelfunc 0x05
    sethalt(%q) kernelfunc 0x06
    face(%d) kernelfunc 0x07
    setloop(%w) kernelfunc 0x08
    setmask(%d) kernelfunc 0x09
    setclip(%b) kernelfunc 0x0a
    stop() kernelfunc 0x0b
    animate() kernelfunc 0x0c
    move(%e,%e) kernelfunc 0x0d
    show() kernelfunc 0x0e
    hide() kernelfunc 0x0f
    destroy() kernelfunc 0x10
    unload() kernelfunc 0x11
    setpriority(%e) kernelfunc 0x14
    teleport(%e) kernelfunc 0x15
    loadanimation(%q) kernelfunc 0x16
    setmoveobj() kernelfunc 0x17
    bindaction(%e %p) kernelfunc 0x18
    handlevent() flags 0
    use_it() flags 0
    talk_to() flags 0
    look_at() flags 0
    x() propno 0x0
    y() propno 0x1
    frameno() propno 0x2
    moving() propno 0x3
    animating() propno 0x4
    prioritycode() propno 0x5
    roomno() propno 0x6
    oldroomno() propno 0x7
    dialogchoice() propno 0x8
}

classdef TEXT {
    moveto(%e,%e) kernelfunc 0x04
    setcolor(%e) kernelfunc 0x19
    loadtext(%e) kernelfunc 0x1a
    setalignment(%e) kernelfunc 0x1b
    initext(%q) kernelfunc 0x1c
    move(%e,%e) kernelfunc 0x0d
    show() kernelfunc 0x0e
    hide() kernelfunc 0x0f
    unload() kernelfunc 0x11
    setspeed(%w,%w,%w) kernelfunc 0x05
    moving() propno 0x3
}

kernelfuncs {
    leave() kernelfunc 0x00
    invoke(%q) kernelfunc 0x01
    exit() kernelfunc 0x02
    loadscreen(%q) kernelfunc 0x03
    refresh() kernelfunc 0x04
    delay(%e) kernelfunc 0x05
    loadpalette(%q) kernelfunc 0x06
    return() kernelfunc 0x07
    break() kernelfunc 0xff
    bar_addicon(%q,%q,%e,%e,%e,%f) kernelfunc 0x20
    bar_show() kernelfunc 0x21
    bar_hide() kernelfunc 0x22
    bar_bindaction(%e,%f) kernelfunc 0x23
    debug_break() kernelfunc 0xff
    dlg_showtext(%e,%e,%e,%e) kernelfunc 0x30
    dlg_converse(%e,%e,%e,%e,%e) kernelfunc 0x31
    dlg_emptychoice() kernelfunc 0x32
    dlg_alert(%e) kernelfunc 0x33
    inv_enable() kernelfunc 0x40
    inv_disable() kernelfunc 0x41
    inv_giveitem(%q,%e) kernelfunc 0x42
    inv_dropitem(%q) kernelfunc 0x43
    inv_setquantity(%q,%u) kernelfunc 0x44
    inv_open() kernelfunc 0x45
    loadcursor(%q) kernelfunc 0x50
}

sysfuncs {
    random(%e) kernelfunc 0x80
    dlg_choice() kernelfunc 0x81
    touched(%e,%e,%e,%e) kernelfunc 0x82
    inv_getquantity(%q) kernelfunc 0x46
}
