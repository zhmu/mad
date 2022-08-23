/*
                               GFX.CPP

              MAD Graphix Engine - Handles drawing of all graphics

                       (c) 1999 The MAD Crew

  Description:
  This is the core of all MAD graphics routines. This will make sure
  sprites will be put correctly on the screen, background are used and all
  that. It will also handle the priority and mask screens.

  Todo: Nothing

  Portability notes: Fully portable, no changes required
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "anm.h"
#include "archive.h"
#include "bar.h"
#include "controls.h"
#include "debugmad.h"
#include "maderror.h"
#include "mad.h"
#include "gadgets.h"
#include "gfx.h"
#include "gfx_drv.h"
#include "objman.h"
#include "project.h"
#include "script.h"
#include "types.h"
#include "fontman.h"

/*
 * GFX::init()
 *
 * This will initialize the graphics engine
 *
 */
void
GFX::init() {
    _UINT i;
    char s[MAD_TEMPSTR_SIZE];

    /* mark all data sprites as unused */
    for(i=0;i<GFX_MAXSPRITES;i++) {
        spritedata[i].inuse=0;
        spritedata[i].height=0;
        spritedata[i].width=0;
        spritedata[i].bpp=0;
    }
    /* mark all animations as unused */
    for(i=0;i<GFX_MAXANIMS;i++) {
        animation[i].loaded=0;
    }

    /* set the bpp variable */
    bpp=project->info.bpp;

    /* try to allocate GFX_DRIVER_HRES*GFX_DRIVER_VRES*bpp bytes for the virtual screen */
    if((virtualscreen=(char *)malloc(GFX_DRIVER_HRES*GFX_DRIVER_VRES*bpp))==NULL) {
        /* failed. report this and quit (error 2: out of memory) */
        sprintf(s,MAD_ERROR_2,GFX_DRIVER_HRES*GFX_DRIVER_VRES*bpp);
        die(s);
    }
    /* try to allocate GFX_DRIVER_HRES*GFX_DRIVER_VRES*bpp bytes for the background screen */
    if((backscreen=(char *)malloc(GFX_DRIVER_HRES*GFX_DRIVER_VRES*bpp))==NULL) {
        /* failed. report this and quit (error 2: out of memory) */
        sprintf(s,MAD_ERROR_2,GFX_DRIVER_HRES*GFX_DRIVER_VRES*bpp);
        die(s);
    }
    /* try to allocate GFX_DRIVER_HRES*GFX_DRIVER_VRES*bpp bytes for the priority screen */
    if((priorityscreen=(char *)malloc(GFX_DRIVER_HRES*GFX_DRIVER_VRES*bpp))==NULL) {
        /* failed. report this and quit (error 2: out of memory) */
        sprintf(s,MAD_ERROR_2,GFX_DRIVER_HRES*GFX_DRIVER_VRES*bpp);
        die(s);
    }
    /* try to allocate GFX_DRIVER_HRES*GFX_DRIVER_VRES*bpp bytes for the mask screen */
    if((maskscreen=(char *)malloc(GFX_DRIVER_HRES*GFX_DRIVER_VRES*bpp))==NULL) {
        /* failed. report this and quit (error 2: out of memory) */
        sprintf(s,MAD_ERROR_2,GFX_DRIVER_HRES*GFX_DRIVER_VRES*bpp);
        die(s);
    }
    /* zero out all buffers */
    memset(virtualscreen,0,GFX_DRIVER_HRES*GFX_DRIVER_VRES*bpp);
    memset(backscreen,0,GFX_DRIVER_HRES*GFX_DRIVER_VRES*bpp);
    memset(priorityscreen,0,GFX_DRIVER_HRES*GFX_DRIVER_VRES*bpp);
    memset(maskscreen,0,GFX_DRIVER_HRES*GFX_DRIVER_VRES*bpp);

    /* create the line lookup table */
    for(i=0;i<GFX_DRIVER_VRES;i++) {
        linetable[i]=i*GFX_DRIVER_HRES;
    }

    /* and the last step is to load the palette */
    strcpy(s,(char *)project->info.palettefile);
    strcat(s,MAD_PALEXTENSION);
    loadpalette(s);
}

/*
 * GFX::done()
 *
 * This will deinitialize the graphics engine
 *
 */
void
GFX::done() {
    _UINT i;

    /* check all sprites and if they are used, free the sprite data */
    for(i=0;i<GFX_MAXSPRITES;i++) {
        if (spritedata[i].inuse) {
            free(spritedata[i].data);
        }
    }
    /* check all animations and if they are used, free the animation data */
    for(i=0;i<GFX_MAXANIMS;i++) {
        if (animation[i].loaded) {
            animation[i].done();
        }
    }
    free(virtualscreen);
    free(backscreen);
}

/*
 * GFX::drawpriorityline(_UINT pcode,_SINT lineno,_UCHAR update) {
 *
 * This will draw priority like [lineno], with priority code [pcode]. If
 * [update] is non-zero, all animations will be updated to cycle frames etc.
 *
 */
void
GFX::drawpriorityline(_UINT pcode,_SINT lineno,_UCHAR update) {
    _SINT i,x,y,movex,movey;
    _UINT no,mask,sourceno;
    _UCHAR c;
    char tempstr[MAD_TEMPSTR_SIZE];

    /* put the animations on the screen */
    for(i=0;i<OBJMAN_MAXOBJECTS;i++) {
        /* only visible animations */
        if(objman->isvisible(i)) {
            if(objman->getype(i)==OBJMAN_TYPE_ANM) {
                if(objman->getbasepriority(i)==pcode) {
                    /* is this object at the line we are ordered to draw? */
                    if (hasbaseline(i,lineno)) {
                        /* make [sourceno] the asked source number */
                        sourceno=objman->getsourceno(i);
                        /* yeah, get the sprite number to use */
                        /* NOTE: [sourcedata] is the ANIMATION[X] to use! */
                        /* check if it actually has an animation */
                        if(sourceno==GFX_NOBJECT) {
                            /* it hasnt, so bomb out (error 300: animation doesn't have a valid source number) */
                            sprintf(tempstr,MAD_ERROR_300,objman->getobjectname(i));
                            die(tempstr);
                        }
                        /* check if it actually has a valid sequence number */
                        if(objman->getanimno(i)>animation[sourceno].header.nofanims) {
                            /* it hasnt, so bomb out (error 301: animation doesn't have a valid sequence number) */
                            sprintf(tempstr,MAD_ERROR_301,objman->getobjectname(i));
                            die(tempstr);
                        }
                        /* check if it actually has a valid animation frame */
                        if(objman->getframeno(i)>animation[sourceno].anim[objman->getanimno(i)].anm.noframes) {
                            /* it hasnt, so bomb out (error 302: animation doesn't have a valid animation frame number) */
                            sprintf(tempstr,MAD_ERROR_302,objman->getobjectname(i));
                            die(tempstr);
                        }
                        no=animation[sourceno].anim[objman->getanimno(i)].frame[objman->getframeno(i)]-1;

                        /* no need to check if the sprite number is valid,
                           hasbaseline() already does this */

                        /* draw the object */
                        mask=animationhitmask(i);
                        /* need to update? */
                        if(update) {
                            /* yeah, do it */
                            objman->setprioritycode(i,posok(objman->getobjectx(i),objman->getobjecty(i),i));
                        }

                        /* draw the sprite */
                        drawdata(objman->getobjectx(i),objman->getobjecty(i),animation[sourceno].sprite[no].height,animation[sourceno].sprite[no].width,animation[sourceno].sprite[no].data,(mask)?maskscreen:NULL,virtualscreen);

                        /* now update the moves for the animation */
                        /* does the object want to move? */
                        if(objman->objmustmove(i)&&(update)) {
                            /* yeah, check direction */
                            movex=0;movey=0;
                            /* tweak the destination into the grid */
                            objman->setdestcoords(i,objman->getobjectdestx(i)-(objman->getobjectdestx(i)%objman->getobjecthspeed(i)),
                                                    objman->getobjectdesty(i)-(objman->getobjectdesty(i)%objman->getobjectvspeed(i)));
                            objman->setcoords(i,objman->getobjectx(i)-(objman->getobjectx(i)%objman->getobjecthspeed(i)),
                                                objman->getobjecty(i)-(objman->getobjecty(i)%objman->getobjectvspeed(i)));
                            /* do we have to move north? */
                            if(objman->getobjectdesty(i)<objman->getobjecty(i)) {
                                /* yeah, initialize sequence */
                                objman->setlastdirection(i,SCRIPT_MOVE_NORTH_NO);
                                movey=1;
                            }
                            /* do we have to move south? */
                            if(objman->getobjectdesty(i)>objman->getobjecty(i)) {
                                /* yeah, initialize sequence */
                                objman->setlastdirection(i,SCRIPT_MOVE_SOUTH_NO);
                                movey=1;
                            }
                            /* do we have to move east? */
                            if(objman->getobjectdestx(i)>objman->getobjectx(i)) {
                                /* yeah, initialize sequence */
                                objman->setlastdirection(i,SCRIPT_MOVE_EAST_NO);
                                movex=1;
                            }
                            /* do we have to move west? */
                            if(objman->getobjectdestx(i)<objman->getobjectx(i)) {
                                /* yeah, initialize sequence */
                                objman->setlastdirection(i,SCRIPT_MOVE_WEST_NO);
                                movex=1;
                            }
                            /* do we need to move? */
                            if((movex+movey)) {
                                /* yeah. check whether the speed is ok */
                                objman->setcurwait(i,objman->getcurwait(i)+1);
                                if((objman->getcurwait(i)==objman->getspeed(i))||(!objman->getspeed(i))) {
                                    /* dont change original x and y's yet */
                                    x=objman->getobjectx(i);y=objman->getobjecty(i);objman->setcurwait(i,0);
                                    /* does object need to move east? */
                                    if(x<objman->getobjectdestx(i)) {
                                        /* yeah, update x coordinate */
                                        x+=objman->getobjecthspeed(i);
                                    }
                                    /* does object need to move west */
                                    if(x>objman->getobjectdestx(i)) {
                                        /* yeah, update x coordinate */
                                        x-=objman->getobjecthspeed(i);
                                    }
                                    /* does object need to move north? */
                                    if(y>objman->getobjectdesty(i)) {
                                        /* yeah, update y coordinate */
                                        y-=objman->getobjectvspeed(i);
                                    }
                                    /* does object need to move south? */
                                    if(y<objman->getobjectdesty(i)) {
                                        /* yeah, update y coordinate */
                                        y+=objman->getobjectvspeed(i);
                                    }
                                    /* check if the position is ok */
                                    if (posok(x,y,i)!=GFX_PRIORITY_UNREACHABLE) {
                                        /* change frame number if needed */
                                        /* if destination is ok, update sequence */
                                        objman->setanimno(i,objman->getmovesequence(i,objman->getlastdirection(i)));
                                        /* make sure the frame number is ok */
                                        if (objman->getframeno(i)>=animation[sourceno].anim[objman->getanimno(i)].anm.noframes) {
                                            /* reset the animation to frame 0 */
                                            objman->setframeno(i,0);
                                        }
                                        /* yeah, move to this */
                                        objman->setyankcoords(i,x,y);
                                    } else {
                                        objman->setdestcoords(i,objman->getobjectx(i),objman->getobjecty(i));
                                    }
                                    /* next frame */
                                    objman->setframeno(i,objman->getframeno(i)+1);
                                    /* check if we must loop back to frame 0 */
                                    if (objman->getframeno(i)==animation[sourceno].anim[objman->getanimno(i)].anm.noframes) {
                                        /* yeah, lets do it */
                                        objman->setframeno(i,0);
                                    }
                                    /* if object is at the right place, make it halt */
                                    if(((objman->getobjectx(i)==objman->getobjectdestx(i))&&
                                        (objman->getobjecty(i)==objman->getobjectdesty(i)))||
                                        ((objman->getobjectyankx(i)==objman->getobjectdestx(i))&&
                                         (objman->getobjectyanky(i)==objman->getobjectdesty(i)))) {
                                        objman->setmove(i,0);
                                        objman->setanimno(i,objman->getmovesequence(i,SCRIPT_MOVE_HALT));
                                        objman->setframeno(i,objman->getlastdirection(i));
                                        objman->setanimating(i,0);
                                    }
                                } /* speed is ok */
                            } /* need to move */
                        } /* moving */
                        /* do we have to animate this object? */
                        if(objman->isanimating(i)&&(update)) {
                            /* yeah, check if this is the right time */
                            objman->setcurwait(i,objman->getcurwait(i)+1);
                            if((objman->getcurwait(i)==objman->getspeed(i))||(!objman->getspeed(i))) {
                                /* it is. jump to next frame and reset counter */
                                objman->setframeno(i,objman->getframeno(i)+1);
                                if(objman->getframeno(i)==animation[sourceno].anim[objman->getanimno(i)].anm.noframes) {
                                    objman->setframeno(i,0);
                                }
                                /* reset the wait counter */
                                objman->setcurwait(i,0);
                                /* did the object return to frame 0 and is it non-loopable? */
                                if((!objman->getframeno(i))&&(!objman->islooping(i))) {
                                    /* yeah. remove the animating flag */
                                    objman->setanimating(i,0);
                                }
                            }
                        } /* animating */
                    } /* hasbaseline */
                } /* base priority is ok */
            } /* is animation */
        } /* is visible */
    } /* for i loop */
}

/*
 * GFX::redraw(_UCHAR update)
 *
 * This will redraw the entire graphics screen. If [update] is non-zero, it
 * will update animation frame numbers and coordinates as well.
 *
 */
void
GFX::redraw(_UCHAR update) {
    _SINT i,j;
    _UINT sprno;

    #ifndef DEBUG_AVAILABLE
        /* copy the back screen to the virtual screen */
        memcpy(virtualscreen,backscreen,GFX_DRIVER_HRES*GFX_DRIVER_VRES);
    #else
        /* check what needs to be drawn */
        switch(debug_gfxmode) {
            case DEBUG_GFXMODE_MASKSCR: /* show backscreen */
                                        memcpy(virtualscreen,maskscreen,GFX_DRIVER_HRES*GFX_DRIVER_VRES*bpp);
                                        break;
            case DEBUG_GFXMODE_PRIOSCR: /* priority screen */
                                        memcpy(virtualscreen,priorityscreen,GFX_DRIVER_HRES*GFX_DRIVER_VRES*bpp);
                                        break;
                               default:
            case DEBUG_GFXMODE_BACKSCR: /* show backscreen (default) */
                                        memcpy(virtualscreen,backscreen,GFX_DRIVER_HRES*GFX_DRIVER_VRES*bpp);
                                        break;

        }
    #endif

    /* need to update? */
    if(update) {
        /* yeah, do it */
        for(i=0;i<OBJMAN_MAXOBJECTS;i++) {
            /* set the yank_x and yank_y of all objects to GFX_NOCOORD */
            objman->setyankcoords(i,GFX_NOCOORD,GFX_NOCOORD);
        }
    }
    /* Is there a valid ego object? (XXX) */
//    if(ego_object!=GFX_NOBJECT) {
        /* yeah, handle it */
//        object[ego_object].ego_x=object[ego_object].x;object[ego_object].ego_y=object[ego_object].y;
//        handlegomoves();
//    }
    /* put the sprite objects on the screen */
    for(j=GFX_MAX_PRIORITIES;j>-1;j--) {
        for(i=0;i<GFX_DRIVER_VRES;i++) {
            drawpriorityline(j,i,update);
        }
    }
    /* need to update? */
    if(update) {
        /* yeah, do it */
        for(i=0;i<OBJMAN_MAXOBJECTS;i++) {
            /* handle any x/y coordinate changes */
            if ((objman->getobjectyankx(i)!=GFX_NOCOORD)&&(objman->getobjectyanky(i)!=GFX_NOCOORD)) {
                objman->setcoords(i,objman->getobjectyankx(i),objman->getobjectyanky(i));
            }
         }
    }
    /* bump the mouse to the correct coordinates */
    objman->setcoords(OBJMAN_MOUSE_OBJNO,controls->getxpos(),controls->getypos());

    /* draw the fonts */
    fontman->draw(virtualscreen);

    /* draw the bar and handle its events, if it is enabled */
    if (objman->isenabled(OBJMAN_BAR_OBJNO)) {
        bar->draw();
        if (update) bar->handlevent();
    }

    /* if the main script has a dialog, draw it */
    if (mainscript->dialogno!=DLGMAN_NODIALOG) {
        dlgman->dialog[mainscript->dialogno]->draw(virtualscreen);
    }

    /* draw the mouse pointer if it is enabled */
    if(bar->mouseinreach()) {
        sprno=controls->sysmouse_sprno;
    } else {
        sprno=objman->getsourceno(OBJMAN_MOUSE_OBJNO);
    }
    if ((sprno!=GFX_NOSPRITE)&&(objman->isenabled(OBJMAN_MOUSE_OBJNO))) {
        putsprite(objman->getobjectx(OBJMAN_MOUSE_OBJNO),objman->getobjecty(OBJMAN_MOUSE_OBJNO),sprno,virtualscreen);
    }

    if(update) {
        /* now let the graphics driver copy the data to the screen */
        gfxdriver->draw(virtualscreen);
    }

    /* and do a short delay (XXX) */
    delay_ms((_UINT)main_speed);
}

/*
 * GFX::drawdata(_SINT xpos,_SINT ypos,_UINT height,_UINT width,char* data,char* mask,char* dest)
 *
 * This will draw a block of data to [xpos;ypos]. The height and width are
 * expected to be in [height] and [width]. It will use [data] as the actual
 * picture data. When [mask] is not NULL, the data will be used as the mask
 * screen.
 *
 */
void
GFX::drawdata(_SINT xpos,_SINT ypos,_UINT height,_UINT width,char* data,char* mask,char* dest) {
    _SINT x,y,x2,y2;
    _UINT i,j,no;
    _UCHAR c;
    char* ptr;

    /* make sure the data needs to be drawn */
    if((xpos+width)<0) return;
    if((ypos+height)<0) return;

    x=xpos;
    y=ypos;
    x2=xpos+width;
    y2=ypos+height;
    ptr=data;

    /* make sure we only draw what is needed */
    if(x2>GFX_DRIVER_HRES) x2=GFX_DRIVER_HRES;
    if(y2>GFX_DRIVER_VRES) y2=GFX_DRIVER_VRES;

    if(xpos<0) {
        ptr=ptr+(-xpos);
        x=0;
    }
    if(ypos<0) {
        y=0;
        ptr=ptr+((-ypos)*width);
    }

    if(mask==NULL) {
        /* draw it (no masking) */
        for(j=y;j<y2;j++) {
            for(i=x;i<x2;i++) {
                c=ptr[(width*(j-y))+(i-x)];
                if(c) dest[linetable[j]+i]=c;
            }
        }
    } else {
        /* draw it (with masking) */
        for(j=y;j<y2;j++) {
            for(i=x;i<x2;i++) {
                c=ptr[(width*(j-y))+(i-x)];
                no=linetable[j]+i;
                if((c)&&(!mask[no])) dest[no]=c;
            }
        }
    }
}

/*
 * GFX::putsprite(_SINT xpos,_SINT ypos,_UINT spriteno,char* dest)
 *
 * This will draw sprite [spriteno] to [xpos;ypos] at [dest].
 *
 */
void
GFX::putsprite(_SINT xpos,_SINT ypos,_UINT spriteno,char* dest) {
    drawdata(xpos,ypos,spritedata[spriteno].height,spritedata[spriteno].width,(spritedata[spriteno].data+4),NULL,dest);
}

/*
 * GFX::getfreedatasprite()
 *
 * This will return the first available data sprite or GFX_NOSPRITE when there
 * is no one available.
 *
 */
_UINT
GFX::getfreedatasprite() {
    _UINT i;

    /* trace all data sprites */
    for(i=0;i<GFX_MAXSPRITES;i++) {
        /* is this one unused? */
        if (!spritedata[i].inuse) {
            /* yes, so return its number */
            return i;
        }
    }
    /* no sprites are available. return GFX_NOSPRITE */
    return GFX_NOSPRITE;
}

/*
 * GFX::findatasprite(char *name)
 *
 * This will scan for a data sprite with as filename [name]. It will return
 * the data sprite ID when it is found or GFX_NOSPRITE when it isn't.
 *
 */
_UINT
GFX::findatasprite(char *name) {
    _UINT i;

    /* trace all data sprites */
    for(i=0;i<GFX_MAXSPRITES;i++) {
        /* is it used? */
        if (spritedata[i].inuse) {
            /* yes. if it's equal, return this number, otherwise keep looking */
            if (!strcmp(name,spritedata[i].filename)) return i;
        }
    }
    /* sprite wasn't found. return GFX_NOSPRITE */
    return GFX_NOSPRITE;
}

/*
 * GFX::findanm(char *name)
 *
 * This will scan for an animation with as filename [name]. It will return
 * the animation ID if it is found or GFX_NOBJECT if not.
 *
 */
_UINT
GFX::findanm(char *name) {
    _UINT i;

    /* trace all animations */
    for(i=0;i<GFX_MAXANIMS;i++) {
        /* is it used? */
        if (animation[i].loaded) {
            /* yes. if it's equal, return this number, otherwise keep looking */
            if (!strcmp(name,animation[i].filename)) return i;
        }
    }
    /* sprite wasn't found. return GFX_NOBJECT */
    return GFX_NOBJECT;
}

/*
 * GFX::findanmsequence(_UINT animno,char *name)
 *
 * This will scan for animation sequence [name] in animation [animno]. It will
 * return the animation ID if it is found or GFX_NOBJECT if not.
 *
 */
_UINT
GFX::findanmsequence(_UINT animno,char *name) {
        _UINT i;

        /* if no object specified, say we didn't find it */
        if(animno==GFX_NOBJECT) return GFX_NOBJECT;

        /* scan the animation object for the animation sequence name */
        for(i=0;i<animation[animno].header.nofanims;i++) {
            if(!strcmp(name,animation[animno].anim[i].anm.name)) {
                /* found the animation. return this number */
                return i;
            }
        }
        /* it wasnt found. return GFX_NOBJECT */
        return GFX_NOBJECT;
}

/*
 * GFX::findfreeanm()
 *
 * This will return an ID of the first available animation sequence or
 * GFX_NOBJECT if there are no more available.
 *
 */
_UINT
GFX::findfreeanm() {
    _UINT i;

    /* trace all animations */
    for(i=0;i<OBJMAN_MAXOBJECTS;i++) {
        /* is this one unused? */
        if (!animation[i].loaded) {
            /* yes, so return its number */
            return i;
        }
    }
    /* no animations are available. return GFX_NOBJECT */
    return GFX_NOBJECT;
}

/*
 * GFX::loadsprite(char *fname)
 *
 * This will load sprite [fname]. It will not load it twice. It will return
 * the sprite ID if it was loaded, otherwise it will quit.
 *
 */
_UINT
GFX::loadsprite(char *fname) {
        char tempstr[MAD_TEMPSTR_SIZE];
        _UINT i;

        /* it the sprite already loaded? */
        if ((i=findatasprite(fname))!=GFX_NOSPRITE) {
              /* yes, return its number */
              return i;
        }
        /* no. let's search a new free data sprite */
        if ((i=getfreedatasprite())==GFX_NOSPRITE) {
                /* no more data sprites available, so bomb out (error 304: cannot load, no more data sprites available) */
                sprintf(tempstr,MAD_ERROR_304,fname);
                die(tempstr);
        }

        /* load the sprite. size is NULL because we are not interested in the size */
        spritedata[i].data=(char *)archive->readfile(fname,NULL);

        /* fill in the height, width and filename fields */
        strcpy(spritedata[i].filename,fname);

        spritedata[i].width=(_UINT)spritedata[i].data[0]+((_UINT)spritedata[i].data[1]<<(_UINT)8);
        spritedata[i].height=(_UINT)spritedata[i].data[2]+((_UINT)spritedata[i].data[3]<<(_UINT)8);

        /* mark it as used */
        spritedata[i].inuse=1;

        return i;
}

/*
 * GFX::loadanm(char *fname)
 *
 * This will load animation sequence [fname]. It will not load it twice. When
 * done, it returns the animation number. It will die on any error.
 *
 */
_UINT
GFX::loadanm(char *fname) {
        char tempstr[MAD_TEMPSTR_SIZE];
        _UINT i,j;

        /* it the animation already loaded? */
        if ((i=findanm(fname))!=GFX_NOSPRITE) {
              /* yes, return its number */
              return i;
        }
        /* no. let's search a new free animation object */
        if ((i=findfreeanm())==GFX_NOBJECT) {
                /* no more animation sprites available, so bomb out (error 305: cannot load, no more animation data objects) */
                sprintf(tempstr,MAD_ERROR_305,fname);
                die(tempstr);
        }                                

        /* try to load the animation */
        if((j=animation[i].init(fname))!=ANM_OK) {
                /* load failed. report and die (error 5: animation error when loading file) */
                sprintf(tempstr,MAD_ERROR_5,fname,animation[i].err2msg((_UCHAR)j));
                die(tempstr);
        }

        return i;
}

/*
 * RLEdecompress(char *inbuf,char *outbuf,_UINT destsize)
 *
 * This will decompress the RLE encoded data at [inbuf] to [outbuf]. It will
 * stop when [outbuf] contains [destsize] bytes.
 *
 */
void
RLEdecompress(char *inbuf,char *outbuf,_UINT destsize) {
        _UCHAR c,loop;
        _ULONG pos,offset;

        pos=0;offset=0;
        /* decompress it to [destsize] bytes */
        while (offset<destsize) {
                c=inbuf[pos];
                if ((c&192)==192) {     /* are bits 7 and 8 set? */
                        /* yes, its a loop */
                        loop=(c&63);    /* clear them, we have our loop value */
                        while (loop--) {
                                outbuf[offset++]=inbuf[pos+1];
                        }
                        pos++;
                } else {
                        /* just one byte */
                        outbuf[offset++]=inbuf[pos];
                }
                pos++;
        }

}

/*
 * GFX::loadscreen(char *fname)
 *
 * This will load a screen. It will die on any error.
 *
 */
void
GFX::loadscreen(char *fname) {
        char tempstr[MAD_TEMPSTR_SIZE];
        char *ptr;
        struct PICHEADER *hdr;

        /* first load the file */
        ptr=(char *)archive->readfile(fname,NULL);
        hdr=(struct PICHEADER *)ptr;

        /* magic number ok? */
        if(hdr->idcode!=GFX_PIC_MAGICNO) {
            /* no, die (error 306: cannot load, not a picture) */
            sprintf(tempstr,MAD_ERROR_306,fname);
            die(tempstr);
        }
        /* version number ok? */
        if(hdr->version!=GFX_PIC_VERSIONO) {
            /* no, die (error 307, cannot load picture, wrong version) */
            sprintf(tempstr,MAD_ERROR_307,fname);
            die(tempstr);
        }
        /* decode the background picture */
        if(hdr->gfx_offset!=GFX_PIC_NOPIC) {
            /* and decompress it into [backscreen] */
            RLEdecompress((char *)(ptr+hdr->gfx_offset),backscreen,GFX_DRIVER_HRES*GFX_DRIVER_VRES);
        } else {
            memset(backscreen,0,GFX_DRIVER_HRES*GFX_DRIVER_VRES);
        }
        /* decode the priority screen */
        if(hdr->pri_offset!=GFX_PIC_NOPIC) {
            /* and decompress it into [backscreen] */
            RLEdecompress((char *)(ptr+hdr->pri_offset),priorityscreen,GFX_DRIVER_HRES*GFX_DRIVER_VRES);
        } else {
            memset(priorityscreen,0,GFX_DRIVER_HRES*GFX_DRIVER_VRES);
        }
        /* decode the priority screen */
        if(hdr->msk_offset!=GFX_PIC_NOPIC) {
            /* and decompress it into [backscreen] */
            RLEdecompress((char *)(ptr+hdr->msk_offset),maskscreen,GFX_DRIVER_HRES*GFX_DRIVER_VRES);
        } else {
            memset(maskscreen,0,GFX_DRIVER_HRES*GFX_DRIVER_VRES);
        }
        /* save the file name */
        strcpy(current_picture,fname);
        free(ptr);
}

/*
 * GFX::objectscollide(_UINT spr1,_UINT spr2)
 *
 * This will return zero if objects [spr1] and [spr2] do not collide,
 * otherwise non-zero.
 *
 */
_UCHAR
GFX::objectscollide(_UINT spr1,_UINT spr2) {
        _UINT no,frame;
        _UINT x1,y1,x2,y2,h,w;

        /* always return 0 with unused objects */
        if (objman->getype(spr1)==OBJMAN_TYPE_UNUSED) return 0;
        if (objman->getype(spr2)==OBJMAN_TYPE_UNUSED) return 0;

        /* always return 0 with invalid sprite numbers */
        if (objman->getsourceno(spr1)==GFX_NOSPRITE) return 0;
        if (objman->getsourceno(spr2)==GFX_NOSPRITE) return 0;

        /* x and y coordinates are normal for all objects */
        x1=objman->getobjectx(spr1);y1=objman->getobjecty(spr1);
        x2=objman->getobjectx(spr2);y2=objman->getobjecty(spr2);

        no=objman->getsourceno(spr2);
        switch(objman->getype(spr2)) {
/*            case OBJMAN_TYPE_SPRITE: /* object 2 is a sprite */
/*                                  h=spritedata[no].height;
                                  w=spritedata[no].width;
                                  break;*/
            case OBJMAN_TYPE_ANM: /* it's an animation object */
                                  frame=animation[no].anim[objman->getanimno(spr2)].frame[objman->getframeno(spr2)]-1;
                                  /* if invalid frame, return false */
                                  if(frame>animation[no].header.nofsprites) {
                                      return 0;
                                  }
                                  h=animation[no].sprite[frame].height;
                                  w=animation[no].sprite[frame].width;
                                  break;
                         default: /* can't recognize this type. return false */
                                  return 0;
        }
        /* check for collision */
        if((x1>=x2)&&(y1>=y2)&&(x1<=(x2+w))&&(y1<=(y2+h))) {
            /* it collides! */
            return 1;
        }
        /* it doesn't collide. oh well... */
        return 0;
}

/*
 * GFX::unloadsprite(_UINT no)
 *
 * This will unload data sprite [no]. It will not unload it if it is used
 * elsewhere.
 *
 */
void
GFX::unloadsprite(_UINT no) {
        _UINT i,used;

        used=0;
        /* trace all data sprites if this sprite number is used */
        for(i=0;i<OBJMAN_MAXOBJECTS;i++) {
                /* has this one the sprite number of the sprite we
                   want to unload ? */
                if ((objman->getsourceno(i)==no)&&(objman->getype(i)==OBJMAN_TYPE_SPRITE)) {
                   /* yes, so set flag */
                   used=1;
                }
        }
        /* do nothing if it is still used by sprites */
        if (used) return;
        /* also do nothing if no data for this sprite allocated */
        if (!spritedata[no].inuse) return;
        /* sprite is unused, so unload it */
        free(spritedata[no].data);
        /* mark the sprite as unused */
        spritedata[no].inuse=0;
}

/*
 * GFX::unloadanimation(_UINT no)
 *
 * This will unload animation [no]. It will not unload the animation if it is
 * used elsewhere.
 *
 */
void
GFX::unloadanimation(_UINT no) {
        _UINT i,used;

        used=0;
        /* trace all data sprites if this sprite number is used */
        for(i=0;i<OBJMAN_MAXOBJECTS;i++) {
                /* has this one the sprite number of the sprite we
                   want to unload ? */
                if ((objman->getsourceno(i)==no)&&(objman->getype(i)==OBJMAN_TYPE_ANM)) {
                   /* yes, so set flag */
                   used=1;
                }
        }
        /* do nothing if it is used by an active sprite */
        if (used) return;
        animation[no].done();
}

/*
 * _UCHAR getpriority(_SINT x,_SINT y,_SINT x2,_SINT y2,_UCHAR wrap_coords)
 *
 * this will return the priorities between (x,y) and (x2,y2). it will mask
 * out GFX_PRIORITY_REACHABLE values, leaving only special values. if
 * [wrap_coords] is unequal to zero, the coordinates will be wrapped to be
 * in the range (0,0)-(hres,vres).
 *
 */
_UCHAR
GFX::getpriority(_SINT x,_SINT y,_SINT x2,_SINT y2,_UCHAR wrap_coords) {
    _SINT i,j,c,a,b;
    _UCHAR ok;

    c=GFX_PRIORITY_REACHABLE;
    for(j=y;j<y2;j++) {
        for(i=x;i<x2;i++) {
            if ((j<gfxdriver->get_vres())&&(i<gfxdriver->get_hres())&&(i>=0)&&(j>=0)) {
                ok=priorityscreen[linetable[j]+i];
                if (ok==GFX_PRIORITY_UNREACHABLE) {
                    c=GFX_PRIORITY_UNREACHABLE;
                    return c;
                }
                if ((ok!=GFX_PRIORITY_REACHABLE)&&(ok!=GFX_PRIORITY_UNREACHABLE)) {
                    c=priorityscreen[linetable[j]+i];
                }
            } else {
                /* need to wrap coords? */
                if(wrap_coords) {
                    /* yeah. if priority was outside the screen, yank it to
                     * the sides
                     */
                    a=i;b=j;
                    if(a>GFX_DRIVER_HRES) a=GFX_DRIVER_HRES-1;
                    if(b>GFX_DRIVER_VRES) b=GFX_DRIVER_VRES-1;
                    if(a<0) a=0;
                    if(b<0) b=0;
                    /* we now have good (a,b) coordinates */
                    if(priorityscreen[linetable[b]+a]==GFX_PRIORITY_UNREACHABLE) { c=GFX_PRIORITY_UNREACHABLE; return c; }
                    if(priorityscreen[linetable[b]+a]!=GFX_PRIORITY_REACHABLE) c=priorityscreen[linetable[b]+a];
                }
            }
        }
    }
    return c;
}

/*
 * GFX::posok(_SINT x,_SINT y,_UINT objectno)
 *
 * This will return the priority code of object [objectno] when it is at
 * [x;y].
 *
 */
_UCHAR
GFX::posok(_SINT x,_SINT y,_UINT objectno) {
    char tempstr[MAD_TEMPSTR_SIZE];
    _UINT h,w,no;

    /* does the object clip? if not, just return ok */
    if(!objman->isclipping(objectno)) return 1;

    no=animation[objman->getsourceno(objectno)].anim[objman->getanimno(objectno)].frame[objman->getframeno(objectno)]-1;
    /* check if the sprite number is valid */
    if(no>animation[objman->getsourceno(objectno)].header.nofsprites) {
        /* it isnt, so bomb out (error 303: animation object doesn't have a valid sprite number)*/
        sprintf(tempstr,MAD_ERROR_303,objman->getobjectname(objectno));
        die(tempstr);
    }
    h=animation[objman->getsourceno(objectno)].sprite[no].height;
    w=animation[objman->getsourceno(objectno)].sprite[no].width;

    /* return the priority */
    return getpriority(x,y+h-1,x+w,y+h,1);
}

/*
 * GFX::getspriteheight(_UINT no)
 *
 * This will return the height of data sprite [no], or zero if it is not
 * loaded.
 *
 */
_UINT
GFX::getspriteheight(_UINT no) {
    /* check if sprite data is in use */
    if(!spritedata[no].inuse) {
        /* no, just return 0 */
        return 0;
    }
    /* return the height */
    return (spritedata[no].height);
}

/*
 * GFX::getspritewidth(_UINT no)
 *
 * This will return the width of data sprite [no], or zero if it is not
 * loaded.
 *
 */
_UINT
GFX::getspritewidth(_UINT no) {
    /* return the height of sprite [no] */
    /* check if sprite data is in use */
    if(!spritedata[no].inuse) {
        /* no, just return 0 */
        return 0;
    }
    /* return the width */
    return (spritedata[no].width);
}

/*
 * GFX::getnoframes(_UINT objectno)
 *
 * This will return the number of frames object [objectno] has in its current
 * animation.
 *
 */
_UINT
GFX::getnoframes(_UINT objectno) {
    return (animation[objman->getsourceno(objectno)].anim[objman->getanimno(objectno)].anm.noframes);
}

/*
 * GFX::animationhitmask(_UINT objectno)
 *
 * This will return zero if animation [objectno] is masked, and non-zero if
 * not.
 *
 */
_UCHAR
GFX::animationhitmask(_UINT objectno) {
    char tempstr[MAD_TEMPSTR_SIZE];
    _UINT h,w,i,j,no;

    no=animation[objman->getsourceno(objectno)].anim[objman->getanimno(objectno)].frame[objman->getframeno(objectno)]-1;
    /* check if the sprite number is valid */
    if(no>animation[objman->getsourceno(objectno)].header.nofsprites) {
        /* it isnt, so bomb out (error 303: animation object doesn't have a valid sprite number)*/
        sprintf(tempstr,MAD_ERROR_303,objman->getobjectname(objectno));
        die(tempstr);
    }
    h=animation[objman->getsourceno(objectno)].sprite[no].height;
    w=animation[objman->getsourceno(objectno)].sprite[no].width;

    switch (objman->getmaskdir(objectno)) {
     case GFX_MASK_BOTTOM: j=(objman->getobjecty(objectno)+h); break;
        case GFX_MASK_TOP: j=(objman->getobjecty(objectno)); break;
            case DIR_NONE: /* no mask. return 0 */ return 0;
                  default: /* error 308: animation object doesn't have a valid mask direction */
                           sprintf(tempstr,MAD_ERROR_308,objman->getobjectname(objectno));
                           die(tempstr);
    }
    for(i=objman->getobjectx(objectno);i<(objman->getobjectx(objectno)+w);i++) {
        /* check if the mask is not 0. if it is, we are masked */
        if(maskscreen[linetable[j]+i]) {
            return 1;
        }
    }
    /* we are not masked. return 0 */
    return 0;
}

/*
 * GFX::hasbaseline(_UINT objectno,_UINT lineno)
 *
 * this will return non-zero if the base line of object [objectno] is
 * [lineno], and zero if not.
 *
 */
_UCHAR
GFX::hasbaseline(_UINT objectno,_UINT lineno) {
    char tempstr[MAD_TEMPSTR_SIZE];
    _UINT h,no,ly;

    /* get the number */
    no=animation[objman->getsourceno(objectno)].anim[objman->getanimno(objectno)].frame[objman->getframeno(objectno)]-1;
    /* check if the sprite number is valid */
    if(no>animation[objman->getsourceno(objectno)].header.nofsprites) {
        /* it isnt, so bomb out (error 303: animation object doesn't have a valid sprite number)*/
        sprintf(tempstr,MAD_ERROR_303,objman->getobjectname(objectno));
        die(tempstr);
    }
    h=animation[objman->getsourceno(objectno)].sprite[no].height;
    ly=(objman->getobjecty(objectno)+h)-1;

    if(lineno==GFX_DRIVER_VRES-1) {
        if(ly>GFX_DRIVER_VRES) return 1;
    }

    return (ly==lineno);
}

/*
 * GFX::getcuranmsprite(_UINT objectno)
 *
 * This will return the current animation sprite of object [objectno].
 *
 */
_UINT
GFX::getcuranmsprite(_UINT objectno) {
    return animation[objman->getsourceno(objectno)].anim[objman->getanimno(objectno)].frame[objman->getframeno(objectno)]-1;
}

/*
 * GFX::getanmheight(_UINT objectno)
 *
 * This will return the height of the current animation sprite of object
 * [objectno].
 *
 */
_UINT
GFX::getanmheight(_UINT objectno) {
    return animation[objman->getsourceno(objectno)].sprite[getcuranmsprite(objectno)].height;
}

/*
 * GFX::getanmwidth(_UINT objectno)
 *
 * This will return the width of the current animation sprite of object
 * [objectno].
 *
 */
_UINT
GFX::getanmwidth(_UINT objectno) {
    return animation[objman->getsourceno(objectno)].sprite[getcuranmsprite(objectno)].width;
}

/*
 * GFX::sethaltsequence(_UINT objectno,char *seqname)
 *
 * This will activate halt sequence [seqname] for object [objectno]. It will
 * return non-zero if successful, otherwise non-zero.
 *
 */
_UINT
GFX::sethaltsequence(_UINT objectno,char *seqname) {
    _UINT i;

    if ((i=findanmsequence(objman->getsourceno(objectno),seqname))==GFX_NOBJECT) {
        /* no sequence found, so return 0 */
        return 0;
    }
    objman->setmovesequence(objectno,SCRIPT_MOVE_HALT,i);
    return 1;
}

/*
 * GFX::setmovesequence(_UINT objectno,char *seqname)
 *
 * This will activate move sequence [seqname] for object [objectno]. It will
 * return non-zero if successful, otherwise non-zero.
 *
 */
_UINT
GFX::setmovesequence(_UINT objectno,char *seqname) {
    _UINT i;
    char tempstr[MAD_TEMPSTR_SIZE];

    strcpy(tempstr,seqname);
    strcat(tempstr,SCRIPT_MOVE_SEPARATOR);
    strcat(tempstr,SCRIPT_MOVE_NORTH);
    if ((i=findanmsequence(objman->getsourceno(objectno),tempstr))==GFX_NOBJECT) {
        /* unable to find sequence. return 0 */
        return 0;
    }
    objman->setmovesequence(objectno,SCRIPT_MOVE_NORTH_NO,i);

    strcpy(tempstr,seqname);
    strcat(tempstr,SCRIPT_MOVE_SEPARATOR);
    strcat(tempstr,SCRIPT_MOVE_EAST);
    if ((i=findanmsequence(objman->getsourceno(objectno),tempstr))==GFX_NOBJECT) {
        /* unable to find sequence. return 0 */
        return 0;
    }
    objman->setmovesequence(objectno,SCRIPT_MOVE_EAST_NO,i);

    strcpy(tempstr,seqname);
    strcat(tempstr,SCRIPT_MOVE_SEPARATOR);
    strcat(tempstr,SCRIPT_MOVE_SOUTH);
    if ((i=findanmsequence(objman->getsourceno(objectno),tempstr))==GFX_NOBJECT) {
        /* unable to find sequence. return 0 */
        return 0;
    }
    objman->setmovesequence(objectno,SCRIPT_MOVE_SOUTH_NO,i);

    strcpy(tempstr,seqname);
    strcat(tempstr,SCRIPT_MOVE_SEPARATOR);
    strcat(tempstr,SCRIPT_MOVE_WEST);
    if ((i=findanmsequence(objman->getsourceno(objectno),tempstr))==GFX_NOBJECT) {
        /* unable to find sequence. return 0 */
        return 0;
    }
    objman->setmovesequence(objectno,SCRIPT_MOVE_WEST_NO,i);

    /* all went ok. return 1 */
    return 1;
}

/*
 * GFX::loadpalette(char *fname)
 *
 * This will load palette [fname]. It will die on any error.
 *
 */
void
GFX::loadpalette(char *fname) {
    char *ptr;
    int i;

    /* first read the file to the memory */
    ptr=(char *)archive->readfile(fname,NULL);

    memcpy(palette,ptr,GFX_DRIVER_PALSIZE);

    #ifndef WINDOWS
    /* Not needed for Windows, anyone knows why? */

    /* We need to shift right the palette by 2 steps */
    for(i=0;i<GFX_DRIVER_PALSIZE;i++) {
        palette[i]=palette[i]>>2;
    }
    #endif

    gfxdriver->setpalette(palette);

    free(ptr);
}

/*
 * GFX::darkenpalette(_UCHAR step)
 *
 * This will darken the palette by [step] steps.
 *
 */
void
GFX::darkenpalette(_UCHAR step) {
    _UINT i;
    _UCHAR tempal[GFX_DRIVER_PALSIZE];

    memcpy(tempal,palette,GFX_DRIVER_PALSIZE);
    for(i=0;i<GFX_DRIVER_PALSIZE;i++) {
        tempal[i]=tempal[i]>>step;
    }
    gfxdriver->setpalette((char *)tempal);
}

/*
 * GFX::restorepalette()
 *
 * This will restore the palette colors.
 *
 */
void
GFX::restorepalette() {
    gfxdriver->setpalette((char *)palette);
}

/*
 * GFX::handlegomoves()
 *
 * This will handle the ego keyboard moves.
 *
 */
void
GFX::handlegomoves() {
    _UINT i;
    _SINT x,y;

    /* handle the keyboard event of the ego character */
/*    if (ego_object!=GFX_NOBJECT) {
        if(controls->leftpressed()) {
            objman->setdestcoords(ego_object,0,objman->getobjectdesty());
        }
    }*/
/*        if (controls->left_pressed()) {
            if(object[ego_object].move_dir_h==DIR_UNKNOWN) {
                object[ego_object].move_dir_h=DIR_WEST;
            } else {
                object[ego_object].move_dir_h=DIR_UNKNOWN;
            }
            object[ego_object].move_dir_v=DIR_UNKNOWN;
        }
        if (controls->right_pressed()) {
            if(object[ego_object].move_dir_h==DIR_UNKNOWN) {
                object[ego_object].move_dir_h=DIR_EAST;
            } else {
                object[ego_object].move_dir_h=DIR_UNKNOWN;
            }
            object[ego_object].move_dir_v=DIR_UNKNOWN;
        }
        if (controls->up_pressed()) {
            if(object[ego_object].move_dir_v==DIR_UNKNOWN) {
                object[ego_object].move_dir_v=DIR_NORTH;
            } else {
                object[ego_object].move_dir_v=DIR_UNKNOWN;
            }
            object[ego_object].move_dir_h=DIR_UNKNOWN;
        }
        if (controls->down_pressed()) {
            if(object[ego_object].move_dir_v==DIR_UNKNOWN) {
                object[ego_object].move_dir_v=DIR_SOUTH;
            } else {
                object[ego_object].move_dir_v=DIR_UNKNOWN;
            }
            object[ego_object].move_dir_h=DIR_UNKNOWN;
        }
        if (controls->halt_pressed()) {
            object[ego_object].move_dir_h=DIR_UNKNOWN;
            object[ego_object].move_dir_v=DIR_UNKNOWN;
        }
        i=ego_object;

        /* setup the destination coordinates */
/*        if ((object[i].move_dir_h!=DIR_UNKNOWN)||(object[i].move_dir_v!=DIR_UNKNOWN)) {
            x=object[i].ego_x;y=object[i].ego_y;
            switch(object[i].move_dir_h) {
                case DIR_EAST: /* move the charachter east */
/*                               x+=object[i].h_speed;
                               break;
                case DIR_WEST: /* move the charachter east */
/*                               x-=object[i].h_speed;
                               break;
             case DIR_UNKNOWN: /* ignore it */
/*                               break;
            }
            switch(object[i].move_dir_v) {
               case DIR_NORTH: /* move the charachter north */
/*                               y-=object[i].v_speed;
                               break;
               case DIR_SOUTH: /* move the charachter south */
/*                               y+=object[i].v_speed;
                               break;
             case DIR_UNKNOWN: /* ignore it */
/*                               break;
             }
             if(posok(x,y,i,1)!=GFX_PRIORITY_UNREACHABLE) {
                 /* geez, this position is ok. update it */
/*                 object[i].destx=x; object[i].desty=y;
                 object[i].move=1;
             }
        }
    }*/
}

/*
 * GFX::getgamescreen(char *dest)
 *
 * This will copy the game screen to [dest].
 *
 */
void
GFX::getgamescreen(char *dest) {
    redraw(0);
    memcpy(dest,virtualscreen,GFX_DRIVER_HRES*GFX_DRIVER_VRES);
}

/*
 * GFX::getspritedata(_UINT no)
 *
 * This will return a pointer to the sprite data of object [no].
 *
 */
char*
GFX::getspritedata(_UINT no) {
    /* return the sprite data */
    return (spritedata[no].data);
}

/*
 * GFX::fadeout()
 *
 * This will do a fade out
 *
 */
void
GFX::fadeout() {
    unsigned char tempal[GFX_DRIVER_PALSIZE];
    _UINT i,j,v;

    memcpy(tempal,palette,GFX_DRIVER_PALSIZE);

    for(j=0;j<256;j++) {
        v=0;
        for(i=0;i<GFX_DRIVER_NOFCOLS;i++) {
            if (tempal[v]>0) tempal[v]--;
            if (tempal[v+1]>0) tempal[v+1]--;
            if (tempal[v+2]>0) tempal[v+2]--;

            v+=3;

            /* make sure the emergency exit still works */
            controls->poll();
        }
        gfxdriver->setpalette((char*)tempal);

        delay_ms(10);
    }
}

/*
 * GFX::fadein()
 *
 * This will do a fade in
 *
 */
void
GFX::fadein() {
    char tempal[GFX_DRIVER_PALSIZE];
    _UINT i,j,v;

/*    memset(tempal,0,GFX_DRIVER_PALSIZE);

    for(j=0;j<256;j++) {
        for(i=0;i<GFX_DRIVER_NOFCOLS;i++) {
            v=i*3;

            if (palette[v]>tempal[v]) tempal[v]++;
            if (palette[v+1]>tempal[v+1]) tempal[v+1]++;
            if (palette[v+2]>tempal[v+2]) tempal[v+2]++;

            /* make sure the emergency exit still works */
  /*          controls->poll();
        }
        gfxdriver->setpalette(tempal);
    }*/
//    gfxdriver->setpalette(palette);
}
