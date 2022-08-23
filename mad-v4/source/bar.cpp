/*
                               BAR.CPP

                 MAD Bar Engine - Handles the icon bar

                  (c) 1998, 1999, 2000 The MAD Crew

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
    /* say no icons loaded, set selected icon to the first icon, and disable
       the bar */
    noficons=0; bar_selno=0; bar_enabled=0; bar_spriteno=GFX_NOSPRITE;
    bar_ypos=0; inv_curitem=BAR_INV_NOINVITEM;old_aim_spriteno=GFX_NOSPRITE;

    /* add a system wildcard entry */
    icon[noficons].spriteno=GFX_NOSPRITE;
    icon[noficons].flags=BAR_HIDDEN;
    noficons++;

    /* does the project say we don't have a bar? */
    if(project->info.flags&PROJECT_FLAG_NOBAR) {
        /* yeah. get outta here */
        return;
    }

    /* load the icon bar */
    bar_spriteno=gfx->loadsprite(BAR_MAINSPRITENAME);

    /* fix the correct vertical position */
    bar_ypos=-gfx->getspriteheight(bar_spriteno);
}

/*
 * BAR::draw()
 *
 * This will draw the bar. It will be drawn to GFX::virtualscreen.
 *
 */
void
BAR::draw() {
    _UINT i;

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

    /* show the icon bar itself */
    gfx->putsprite(0,bar_ypos,bar_spriteno,gfx->virtualscreen);

    /* trace all icons */
    for(i=0;i<noficons;i++){
        /* if it is hidden, skip it */
        if(!(icon[i].flags&BAR_HIDDEN)) {
            /* it's not hidden. is it selected? */
            if (bar_selno==i) {
                /* yup. draw the sprite on the screen */
                gfx->putsprite(icon[i].x,bar_ypos+1+icon[i].y,icon[i].spriteno,gfx->virtualscreen);
            }
        }
    }
}

/*
 * BAR::addicon(char* name,char* spritefile,_UINT x,_UINT y,_UINT flags,_UINT objid,_ULONG method)
 *
 * This will add an icon to the bar. It will get name [name]. The sprite
 * loaded will be [spritefile]. the icon will be put at [x],[y]. It will be
 * gives flags [flags]. object [objid]'s method [method] will be called if the
 * user clicks on the icon.
 *
 */
void
BAR::addicon(char* name,char* spritefile,_UINT x,_UINT y,_UINT flags,_UINT objid,_ULONG method) {
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
    icon[noficons].spriteno=gfx->loadsprite(spritefile);
    icon[noficons].x=x;
    icon[noficons].y=y;
    icon[noficons].flags=flags;
    icon[noficons].objid=objid;
    icon[noficons].method=method;
    icon[noficons].def_method=SCRIPT_NULLPROPERTY;

    /* select this icon if we didn't have one */
    if(!bar_selno) bar_selno=noficons;

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
    _UINT i,j,inv_itemno,h,w,no,a,b,mx,my;
    _ULONG o;

    /* if the button 2 is clicked, active next icon */
    if (controls->button2pressed()) {
        /* does the project say we don't have a bar? */
        if(project->info.flags&PROJECT_FLAG_NOBAR) {
            /* yeah. get outta here */
            return;
        }

        /* return if no icons in bar */
        if (!noficons) return;

        /* keep doing this if the icon cannot be activated */
        while(1) {
            bar_selno++;
            if(bar_selno==noficons) bar_selno=0;

            /* is this icon visible? */
            if((!(icon[bar_selno].flags&BAR_HIDDEN))) {
                /* yup. is it cycleable? */
                if((!(icon[bar_selno].flags&BAR_NOCYCLE))) {
                    /* yup. this is the icon! */
                    break;
                }
            }

            /* poll the controls, so that the emergency exit will work */
            idler();
        }

        /* wait until user stops clicking */
        controls->waitnobutton();
        return;
    }
    /* return immediatly if button 1 is not clicked */
    if (!controls->button1pressed()) return;

    /* if a bar has been loaded, also handle icon changes */
    if(bar_spriteno!=GFX_NOSPRITE) {
//        if(controls->getypos()>gfx->getspriteheight(bar_spriteno)) return;

        /* get the mouse position */
        mx=controls->getxpos();my=controls->getypos();

        /* check all icons */
        for(i=0;i<noficons;i++) {
            if(icon[i].spriteno!=GFX_NOSPRITE) {
                a=icon[i].x; b=icon[i].y+bar_ypos;
                h=gfx->getspriteheight(icon[i].spriteno);
                w=gfx->getspritewidth(icon[i].spriteno);
                /* if the sprite is valid, skip it */
                if((mx>=a)&&(my>=b)&&(mx<=(a+w))&&(my<=(b+h))) {
                    j=bar_selno; bar_selno=i;

                    /* is this bar icon bound to a procedure? */
                    if((icon[i].objid!=OBJMAN_NOBJECT)&&(icon[i].method!=SCRIPT_NULLPROPERTY)) {
                        /* yes. summon the procedure */
                        rootscript->go_method(icon[i].objid,icon[i].method);
                        bar_selno=j;
                    }
                    return;
                }
            }
        }
    }

    /* already clear inventory item stuff */
    inv_itemno=inv_curitem;
    inv_curitem=BAR_INV_NOINVITEM;

    /* restore the mouse cursor if needed */
    if(old_aim_spriteno!=GFX_NOSPRITE) {
        gfx->set_mouse_spriteno(old_aim_spriteno);
    }

    /* is the bar enabled? */
    if(!bar_enabled) {
        /* no. use the wildcard icon */
        no=0;
    } else {
        /* yes. use the selected bar icon */
        no=bar->getselno();
    }

    /* return immediatly if pointer is not within bar */
    /* only check if bar icon has no special flags */
    if (!(bar->getflags(no)&(~BAR_HIDDEN))) {
        h=gfx->getspriteheight(gfx->get_mouse_spriteno());
        w=gfx->getspritewidth(gfx->get_mouse_spriteno());
        /* scan all sprites for a collision with the mouse pointer */
        a=0;
        for (i=0;i<OBJMAN_MAXOBJECTS;i++) {
            /* is this a true object? */
            if(objman->getype(i)==OBJMAN_TYPE_ANM) {
                /* yup. do they collide? */
                if(gfx->objectcollideswith(i,controls->getxpos(),controls->getypos(),h,w)) {
                    /* yes. wait until user stops clicking */
                    controls->waitnobutton();

                    /* check the property */
                    if ((o=objman->getaction(i,no))!=OBJMAN_NO_ACTION) {
                        /* inherited? */
                        if(o==OBJMAN_ACTION_INHERIT) {
                            /* yup (FIXME!) */
                            a++;
                        } else {
                            /* yes. run it */
                            script->go_method(script->get_fake_objid(i),o);
                            a++;
                        }
                    }
                }
            }
        }
        /* wait until buttons are up */
        controls->waitnobutton();

        /* did something happend? */
        if (!a) {
            /* no. summon the default handler */
            if (icon[no].def_method!=SCRIPT_NULLPROPERTY) {
                script->go_method(icon[no].def_objid,icon[no].def_method);
            }
        }
        /* and get outta here! */
        return;
     } else {
        /* check if bit 2 is set (move ego object to) */
        if((bar->getflags(bar->getselno())&BAR_MOVETO)&&
           (!bar->mouseinreach())&&
           (bar_enabled)&&
           (ego_object!=OBJMAN_NOBJECT)) {
               /* check if the ego object exists */
               objman->setdestcoords(ego_object,controls->getxpos()-(gfx->getspritewidth(gfx->get_mouse_spriteno())),
                                                controls->getypos()-(gfx->getspriteheight(gfx->get_mouse_spriteno())));
               objman->setmove(ego_object,1);
        }
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
        /* yeah. it can never reach it */
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
        old_aim_spriteno=gfx->get_mouse_spriteno();
    }

    /* activate the new item */
    inv_curitem=no;

    /* load the appropiate mouse cursor */
    gfx->set_mouse_spriteno(gfx->loadsprite(inv->getitemdata(no)->inv_icon));
}

/*
 * BAR::isenabled()
 *
 * This will return zero if the bar is disabled, otherwise non-zero.
 *
 */
_UCHAR
BAR::isenabled() {
    return bar_enabled;
}

/*
 * BAR::setenabled(_UCHAR i)
 *
 * This will set the enabled state of the bar to [i].
 *
 */
void
BAR::setenabled(_UCHAR i) {
    bar_enabled=i;
}

/*
 * BAR::bindefault(_UINT no,_UINT objid,_ULONG method)
 *
 * This will bind default icon actions for number [no] to object [objid],
 * method [method].
 *
 */
void
BAR::bindefault(_UINT no,_UINT objid,_ULONG method) {
    icon[no].def_objid=objid;
    icon[no].def_method=method;
}
