/*
                               BAR.CPP

                 MAD Bar Engine - Handles the icon bar

                      (c) 1998, 1999 The MAD Crew

  Description:
  This will handle the button bar at the top of the screen and mouse events
  of it.

  Todo: Nothing

  Portability notes: Fully portable, no changes required
*/
#include <stdlib.h>
#include <string.h>
#include "archive.h"
#include "bar.h"
#include "controls.h"
#include "objman.h"
#include "maderror.h"
#include "gfx.h"
#include "inv.h"
#include "mad.h"
#include "project.h"
#include "script.h"
#include "types.h"

/*
 * BAR::init()
 *
 * This will initialize the bar object. It will load the bar if neccesary.
 *
 */
void
BAR::init() {
    /* does the project say we don't have a bar? */
    if(project->info.flags&PROJECT_FLAG_NOBAR) {
        /* yeah. get outta here */
        return;
    }

    /* load the icon bar */
    bar_spriteno=gfx->loadsprite(BAR_MAINSPRITENAME);
    /* say no icons loaded, set selected icon to the first icon, and disable
       the bar */
    noficons=0; bar_selno=0; objman->setenabled(OBJMAN_BAR_OBJNO,0);
    bar_ypos=0; inv_curitem=BAR_INV_NOINVITEM;old_aim_spriteno=GFX_NOSPRITE;
}

/*
 * BAR::draw()
 *
 * This will draw the bar. It will be drawn to GFX::virtualscreen.
 *
 */
void
BAR::draw() {
    _UINT i,pos,no;

    /* does the project say we don't have a bar? */
    if(project->info.flags&PROJECT_FLAG_NOBAR) {
        /* yeah. get outta here */
        return;
    }

    /* dont draw anything if bar is not within controls reach */
    if(controls->getypos()>gfx->getspriteheight(bar_spriteno)) {
        if(bar_ypos==(-(_SINT)gfx->getspriteheight(bar_spriteno))) return;
            bar_ypos--;
        } else {
            if(bar_ypos!=0) {
                bar_ypos++;
            }
        }

        gfx->putsprite(0,bar_ypos,bar_spriteno,gfx->virtualscreen);
        pos=1;

        /* trace all icons */
        for(i=0;i<noficons;i++){
            /* if icon is selected, let no be the selected no, otherwise unselected no */
            no=(i==bar_selno)?icon[i].pressedno:icon[i].releasedno;
            /* draw the sprite on the screen */
            gfx->putsprite(pos,bar_ypos+1,no,gfx->virtualscreen);
            /* increment position */
            pos+=gfx->getspritewidth(no);
        }
}

/*
 * BAR::addicon(char* name,char* pressedname,char* releasedname,char* disabledname,_UINT flags)
 *
 * This will add an icon to the bar. It will get name [name]. The sprites loaded are [pressedname],
 * [releasedname] and [disabledname] if it is not a dash. The flags will be set
 * to [flags].
 *
 */
void
BAR::addicon(char *name,char *pressedname,char *releasedname,char *disabledname,_UINT flags) {
    char tempstr[MAD_TEMPSTR_SIZE];

    /* does the project say we don't have a bar? */
    if(project->info.flags&PROJECT_FLAG_NOBAR) {
        /* yeah. get outta here */
        return;
    }

    /* is there any space left for the icon? */
    if (noficons>BAR_MAXICONS) {
        /* no. report and bomb out (error 200: too much icons in bar) */
        sprintf(tempstr,MAD_ERROR_200,name);
        die(tempstr);
    }
    /* fill in name */
    strcpy(icon[noficons].name,name);
    icon[noficons].pressedno=gfx->loadsprite(pressedname);
    icon[noficons].releasedno=gfx->loadsprite(releasedname);
    icon[noficons].flags=flags;
    /* was an actual name supplied in the disable filename? */
    if (strcmp(disabledname,BAR_NOFIELD)) {
        /* yes? add that too */
        icon[noficons].disabledno=gfx->loadsprite(disabledname);
    } else {
        /* no? set the disabled sprite number to an unused sprite marker */
        icon[noficons].disabledno=GFX_NOSPRITE;
    }
    noficons++;
}

/*
 * BAR::handlevent()
 *
 * This will handle all events of the bar.
 *
 */
void
BAR::handlevent() {
    char tempstr[MAD_TEMPSTR_SIZE];
    _UINT i,inv_itemno;

    /* does the project say we don't have a bar? */
    if(project->info.flags&PROJECT_FLAG_NOBAR) {
        /* yeah. get outta here */
        return;
    }

    /* return if no icons in bar */
    if (!noficons) return;
    /* if the button 2 is clicked, active next icon */
    if (controls->button2pressed()) {
        /* cycle to next icon */
        bar_selno++;
        /* keep doing this if the icon cannot be activated */
        while((icon[bar_selno].flags&BAR_NOCYCLE)) {
            bar_selno++;
            if (bar_selno>=noficons) bar_selno=0;
        }
        /* wait until user stops clicking */
        controls->waitnobutton();
        return;
    }
    /* return immediatly if button 1 is not clicked */
    if (!controls->button1pressed()) return;

    /* already clear inventory item stuff */
    inv_itemno=inv_curitem;
    inv_curitem=BAR_INV_NOINVITEM;

        /* restore the mouse cursor if needed */
        if(old_aim_spriteno!=GFX_NOSPRITE) {
            objman->setsourceno(OBJMAN_MOUSE_OBJNO,old_aim_spriteno);
        }

        /* return immediatly if pointer is not within bar */
        /* only check if bar icon has no special flags */
        if(((inv_itemno!=BAR_INV_NOINVITEM)||(!bar->getflags(bar->getselno())))&&(!mouseinreach())) {
            /* scan all sprites for a collision with the mouse pointer */
            for (i=0;i<OBJMAN_MAXOBJECTS;i++) {
                /* only check non-internal sprites and enabled sprites */
                if ((!objman->isinternal(i))&&(objman->isenabled(i))) {
                    if(gfx->objectscollide(OBJMAN_MOUSE_OBJNO,i)) {
                        /* wait until user stops clicking */
                        controls->waitnobutton();
                        /* initialize the procedure name */
                        /* if the bar is enabled add <bar_method>
                           and seperator bar to the object name */
                        /* is it not the inventory thingy? */
                        if(inv_itemno==BAR_INV_NOINVITEM) {
                            if(objman->isenabled(OBJMAN_BAR_OBJNO)) {
                                strcpy(tempstr,bar->getname(bar->getselno()));
                                strcat(tempstr,SCRIPT_EVENTSEPERATOR);
                                strcat(tempstr,objman->getobjectname(i));
                            } else {
                               /* otherwise just copy the object name */
                                strcpy(tempstr,objman->getobjectname(i));
                            }
                        } else {
                            /* it is the inventory thingy. construct the
                               inventory thingy name (like foo#bar) */
                            strcpy(tempstr,inv->getitemdata(inv_itemno)->name);
                            strcat(tempstr,SCRIPT_EVENTSEPERATOR);
                            strcat(tempstr,objman->getobjectname(i));
                        }
                        /* does this procedure exist? */
                        if (script->proc_exists(tempstr)) {
                            /* yeah, call it */
                            script->goproc(tempstr);
                        } else {
                            /* is this the inventory thingy? */
                            if(inv_itemno==BAR_INV_NOINVITEM) {
                                /* no, call the generic procedure (from the main script) */
                                mainscript->goproc(bar->getname(bar->getselno()));
                            } else {
                                /* yeah. call the generic use procedure (from the main script) */
                                mainscript->goproc(BAR_USE_NOWAY_PROC);
                            }
                        }
                        controls->waitnobutton();
                        /* and get outta here! */
                        return;
                    }
                }
            }
            /* do the generic action stuff */
            /* is this the inventory thingy? */
            if(inv_itemno==BAR_INV_NOINVITEM) {
                /* no, call the generic procedure (from the main script) */
                mainscript->goproc(bar->getname(bar->getselno()));
            } else {
                /* yeah. call the generic use procedure (from the main script) */
                mainscript->goproc(BAR_USE_NOWAY_PROC);
            }
            /* wait until buttons are up */
            controls->waitnobutton();
            /* and get outta here! */
            return;
        } else {
            /* check if bit 2 is set (move ego object to) */
            if((bar->getflags(bar->getselno())&BAR_MOVETO)&&
               (!bar->mouseinreach())&&
               (objman->isenabled(OBJMAN_CONTROLS_OBJNO))&&
               (ego_object!=OBJMAN_NOBJECT)) {
                   /* check if the ego object exists */
                   objman->setdestcoords(ego_object,(objman->getobjectx(OBJMAN_MOUSE_OBJNO)-(gfx->getspritewidth(objman->getsourceno(OBJMAN_MOUSE_OBJNO)))),
                                      objman->getobjecty(OBJMAN_MOUSE_OBJNO)-(gfx->getspriteheight(objman->getsourceno(OBJMAN_MOUSE_OBJNO))));
                   objman->setmove(ego_object,1);
            }
        }
        if (controls->getypos()>gfx->getspriteheight(bar_spriteno)) return;
        i=controls->getxpos()/gfx->getspritewidth(icon[0].pressedno);
        /* is this icon valid? */
        if(i>=noficons) {
            /* no, just return */
            return;
        }
        /* is this a special icon that wants immediatly to call a procedure? */
        if (icon[i].flags&BAR_SPAWNRIGHTAWAY) {
                mainscript->goproc(icon[i].name);
                return;
        }
        /* no, so just add selection number */
        if(i<(noficons-1)) {
                /* pointer points to an icon. select it */
                bar_selno=i;
        }
}

/*
 * BAR::getname(_UINT icono)
 *
 * This will retrieve the name of icon number [icono]. It will die if the
 * icon does not exists.
 *
 */
char*
BAR::getname(_UINT icono) {
        char tempstr[MAD_TEMPSTR_SIZE];

        /* does the project say we don't have a bar? */
        if(project->info.flags&PROJECT_FLAG_NOBAR) {
            /* yeah. get outta here */
            return BAR_NOICON;
        }

        /* is it a valid icon number? */
        if(icono>=noficons) {
                /* no? report and die (error 201: cannot retrieve name, icon never loaded) */
                sprintf(tempstr,MAD_ERROR_201,icono);
                die(tempstr);
        }
        return (icon[icono].name);
}

/*
 * BAR::getselno()
 *
 * This will return the number of the currently selected icon.
 *
 */
_UINT
BAR::getselno() {
        return bar_selno;
}

/*
 * BAR::getflags()
 *
 * This will return the flags of icon [icono].
 *
 */
_UINT
BAR::getflags(_UINT icono) {
        return(icon[icono].flags);
}

/*
 * BAR::done()
 *
 * This will deinitialize the icon bar.
 *
 */
void
BAR::done() {
        /* nothing has to be deinitialised, but this will be called anyway.
           it is meant for future expansions */
}

/*
 * BAR::mouseinreach()
 *
 * This will return zero if the current aiming device does not touch the bar,
 * otherwise a non-zero value.
 *
 */
_UCHAR
BAR::mouseinreach() {
        /* does the project say we don't have a bar? */
        if(project->info.flags&PROJECT_FLAG_NOBAR) {
            /* yeah. it can never be reached then */
            return 0;
        }

        return (controls->getypos()<=gfx->getspriteheight(bar_spriteno));
}

/*
 * BAR::setobjtomove(_UINT objno)
 *
 * This will tell the icon bar which object it needs to move.
 *
 */
void
BAR::setobjtomove(_UINT objno) {
    char tempstr[MAD_TEMPSTR_SIZE];

    /* set new object to move when a move button is active */
    /* is user trying to set this to a non-existant object? */
    if(objno>=OBJMAN_MAXOBJECTS) {
        /* yeah. report and die (error 202: tried to use object as move object which has a too great number) */
        sprintf(tempstr,MAD_ERROR_202,objno,OBJMAN_MAXOBJECTS);
        die(tempstr);
    }
    /* is this an internal object? */
    if(objman->isinternal(objno)) {
        /* yeah. report and die (error 203: tried to use internal object as move object) */
        sprintf(tempstr,MAD_ERROR_203,objno);
        die(tempstr);
    }
    ego_object=objno;
}

/*
 * BAR::set_inv_item(_UINT no)
 *
 * This will tell the bar that the user has selected inventory item [no]. The
 * icon bar will use it as mouse cursor.
 *
 */
void
BAR::set_inv_item(_UINT no) {
    /* save the old aiming device cursor sprite number */
    if(inv_curitem==BAR_INV_NOINVITEM) {
        old_aim_spriteno=objman->getsourceno(OBJMAN_MOUSE_OBJNO);
    }

    /* activate the new item */
    inv_curitem=no;

    /* load the appropiate mouse cursor */
    objman->setsourceno(OBJMAN_MOUSE_OBJNO,gfx->loadsprite(inv->getitemdata(no)->inv_icon));
}
