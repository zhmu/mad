/*
                              OBJMAN.CPP

                         MAD Object Manager

                       (c) 1999 The MAD Crew

  Description:
  This will manage all MAD objects.

  Todo: Nothing

  Portability notes: Fully portable, no changes required
*/
#include <string.h>
#include "controls.h"
#include "gfx.h"
#include "fontman.h"
#include "mad.h"
#include "objman.h"
#include "types.h"

/*
 * OBJMAN::init()
 *
 * This will initialize the object manager. It will set all objects to zero.
 *
 */
void
OBJMAN::init() {
    _UINT i;

    /* first clear the object list */
    for(i=0;i<OBJMAN_MAXOBJECTS;i++) {
        destroyobject(i);
    }
    /* and create the internal objects */
    /* add the internal objects, beginning with the mouse object*/
    strcpy(object[OBJMAN_MOUSE_OBJNO].name,OBJMAN_MOUSE_OBJECTNAME);
    object[OBJMAN_MOUSE_OBJNO].type=OBJMAN_TYPE_SPRITE;
    object[OBJMAN_MOUSE_OBJNO].visible=1;
    object[OBJMAN_MOUSE_OBJNO].internal=1;
    object[OBJMAN_MOUSE_OBJNO].enabled=0;
    /* add the bar object */
    strcpy(object[OBJMAN_BAR_OBJNO].name,OBJMAN_BAR_OBJECTNAME);
    object[OBJMAN_BAR_OBJNO].type=OBJMAN_TYPE_SPRITE;
    object[OBJMAN_BAR_OBJNO].visible=0;
    object[OBJMAN_BAR_OBJNO].enabled=0;
    object[OBJMAN_BAR_OBJNO].internal=1;
    /* add the controls object */
    strcpy(object[OBJMAN_CONTROLS_OBJNO].name,OBJMAN_CONTROLS_OBJECTNAME);
    object[OBJMAN_CONTROLS_OBJNO].type=OBJMAN_TYPE_SPRITE;
    object[OBJMAN_CONTROLS_OBJNO].visible=0;
    object[OBJMAN_CONTROLS_OBJNO].enabled=1;
    object[OBJMAN_CONTROLS_OBJNO].internal=1;
    /* add the system object */
    strcpy(object[OBJMAN_SYSTEM_OBJNO].name,OBJMAN_SYSTEM_OBJECTNAME);
    object[OBJMAN_SYSTEM_OBJNO].type=OBJMAN_TYPE_SPRITE;
    object[OBJMAN_SYSTEM_OBJNO].visible=0;
    object[OBJMAN_SYSTEM_OBJNO].enabled=1;
    object[OBJMAN_SYSTEM_OBJNO].internal=1;
    /* add the script object */
    strcpy(object[OBJMAN_SCRIPT_OBJNO].name,OBJMAN_SCRIPT_OBJECTNAME);
    object[OBJMAN_SCRIPT_OBJNO].type=OBJMAN_TYPE_SPRITE;
    object[OBJMAN_SCRIPT_OBJNO].visible=0;
    object[OBJMAN_SCRIPT_OBJNO].enabled=1;
    object[OBJMAN_SCRIPT_OBJNO].internal=1;
    /* add the parser object */
    strcpy(object[OBJMAN_PARSER_OBJNO].name,OBJMAN_PARSER_OBJECTNAME);
    object[OBJMAN_PARSER_OBJNO].type=OBJMAN_TYPE_SPRITE;
    object[OBJMAN_PARSER_OBJNO].visible=0;
    object[OBJMAN_PARSER_OBJNO].enabled=0;
    object[OBJMAN_PARSER_OBJNO].internal=1;
    /* add the dialog object */
    strcpy(object[OBJMAN_DIALOG_OBJNO].name,OBJMAN_DIALOG_OBJECTNAME);
    object[OBJMAN_DIALOG_OBJNO].type=OBJMAN_TYPE_SPRITE;
    object[OBJMAN_DIALOG_OBJNO].visible=0;
    object[OBJMAN_DIALOG_OBJNO].enabled=0;
    object[OBJMAN_DIALOG_OBJNO].internal=1;
}

/*
 * OBJMAN::done()
 *
 * This will deinitialize the object manager.
 *
 */
void
OBJMAN::done() {
}

/*
 * OBJMAN::destroyobject(_UINT objno)
 *
 * This will destroy object [objno] so it is free to use again. Please note
 * that it will NOT deallocate any memory
 *
 */
void
OBJMAN::destroyobject(_UINT objno) {
    memset(&object[objno],0,sizeof(OBJECT));

    object[objno].sourceno=GFX_NOSPRITE;
    object[objno].mask_dir=GFX_MASK_BOTTOM;
    object[objno].clip=1;
    object[objno].oldanimno=GFX_NOANIM;
    object[objno].oldframeno=GFX_NOANIM;
    object[objno].animno=GFX_NOANIM;
    object[objno].frameno=GFX_NOANIM;
}

/*
 * OBJMAN::setvisible(_UINT objno,_UCHAR visible)
 *
 * This will set the visibility of object [objno] to [visible]
 * If [visible] is 0, the object is invisible, otherwise it is visible.
 *
 */
void
OBJMAN::setvisible(_UINT objno,_UCHAR visible) {
    object[objno].visible=visible;
}

/*
 * OBJMAN::setvisible(_UINT objno,_UCHAR enabled)
 *
 * This will set whether object [objno] if enabled.
 * If [enabled] is 0, the object is disabled, otherwise it is enabled.
 *
 */
void
OBJMAN::setenabled(_UINT objno,_UCHAR enabled) {
    object[objno].enabled=enabled;
}

/*
 * OBJMAN::isenabled(_UINT objno)
 *
 * This will return 0 if [objno] is disabled, otherwise a non-zero number
 *
 */
_UCHAR
OBJMAN::isenabled(_UINT objno) {
    return object[objno].enabled;
}

/*
 * OBJMAN::isvisible(_UINT objno)
 *
 * This will return 0 if [objno] is visible, otherwise a non-zero number
 *
 */
_UCHAR
OBJMAN::isvisible(_UINT objno) {
    return object[objno].visible;
}

/*
 * OBJMAN::isdirty(_UINT objno)
 *
 * This will return non-zero if [objno] is dirty, otherwise zero
 *
 */
_UCHAR
OBJMAN::isdirty(_UINT objno) {
    return object[objno].dirty;
}

/*
 * OBJMAN::getype(_UINT objno)
 *
 * This will return type object type of object [objno]
 *
 */
_UCHAR
OBJMAN::getype(_UINT objno) {
    return object[objno].type;
}

/*
 * OBJMAN::getbasepriority(_UINT objno)
 *
 * This will return the base priority of object [objno]
 *
 */
_UCHAR
OBJMAN::getbasepriority(_UINT objno) {
    return object[objno].base_priority;
}

/*
 * OBJMAN::getsourceno(_UINT objno)
 *
 * This will return the source data number object [objno]
 *
 */
_ULONG
OBJMAN::getsourceno(_UINT objno) {
    return object[objno].sourceno;
}

/*
 * OBJMAN::getanimno(_UINT objno)
 *
 * This will return the animation object number of object [objno]
 *
 */
_UCHAR
OBJMAN::getanimno(_UINT objno) {
    return object[objno].animno;
}

/*
 * OBJMAN::getframeno(_UINT objno)
 *
 * This will return the animation frame number of object [objno]
 *
 */
_UCHAR
OBJMAN::getframeno(_UINT objno) {
    return object[objno].frameno;
}

/*
 * OBJMAN::isinternal(_UINT objno)
 *
 * This will return zero if [objno] is non-internal, otherwise non-zero
 *
 */
_UCHAR
OBJMAN::isinternal(_UINT objno) {
    return object[objno].internal;
}

/*
 * OBJMAN::getobjectname(_UINT objno)
 *
 * This will return the object's name
 *
 */
char*
OBJMAN::getobjectname(_UINT objno) {
    return object[objno].name;
}

/*
 * OBJMAN::getobjectx(_UINT objno)
 *
 * This will return the X coordinate of object [objno]
 *
 */
_SINT
OBJMAN::getobjectx(_UINT objno) {
    return object[objno].x;
}

/*
 * OBJMAN::getobjecty(_UINT objno)
 *
 * This will return the Y coordinate of object [objno]
 *
 */
_SINT
OBJMAN::getobjecty(_UINT objno) {
    return object[objno].y;
}

/*
 * OBJMAN::setmove(_UINT objno,_UCHAR move)
 *
 * This will set whether object [objno] should move or not.
 * If [move] is 0, the object will stop moving, otherwise it
 * will move to its [destx] and [desty].
 *
 */
void
OBJMAN::setmove(_UINT objno,_UCHAR move) {
    object[objno].move=move;
}

/*
 * OBJMAN::setcoords(_UINT objno,_SINT x,_SINT y)
 *
 * This will set the coordinates of object [objno] to ([x],[y])
 *
 */
void
OBJMAN::setcoords(_UINT objno,_SINT x,_SINT y) {
    object[objno].old_x=object[objno].x;
    object[objno].old_y=object[objno].y;

    object[objno].x=x;
    object[objno].y=y;
}

/*
 * OBJMAN::setdestcoords(_UINT objno,_SINT x,_SINT y)
 *
 * This will set the destination coordinates of object [objno] to ([x],[y])
 *
 */
void
OBJMAN::setdestcoords(_UINT objno,_SINT x,_SINT y) {
    object[objno].destx=x;
    object[objno].desty=y;
}

/*
 * OBJMAN::setprioritycode(_UINT objno,_UCHAR code)
 *
 * This will set the priority code of object [objno] to [code].
 *
 */
void
OBJMAN::setprioritycode(_UINT objno,_UCHAR code) {
    object[objno].prioritycode=code;
}

/*
 * OBJMAN::objmustmove(_UINT objno)
 *
 * This will return zero if object [objno] doesn't want to move, otherwise a
 * non-zero number.
 *
 */
_UCHAR
OBJMAN::objmustmove(_UINT objno) {
    return object[objno].move;
}

/*
 * OBJMAN::getobjectdestx(_UINT objno)
 *
 * This will return the destination X coordinate of object [objno]
 *
 */
_SINT
OBJMAN::getobjectdestx(_UINT objno) {
    return object[objno].destx;
}

/*
 * OBJMAN::getobjectdesty(_UINT objno)
 *
 * This will return the destination Y coordinate of object [objno]
 *
 */
_SINT
OBJMAN::getobjectdesty(_UINT objno) {
    return object[objno].desty;
}

/*
 * OBJMAN::getobjecthspeed(_UINT objno)
 *
 * This will return the horizontal speed of object [objno].
 *
 */
_UINT
OBJMAN::getobjecthspeed(_UINT objno) {
    return object[objno].h_speed;
}

/*
 * OBJMAN::getobjectvspeed(_UINT objno)
 *
 * This will return the vertical speed of object [objno].
 *
 */
_UINT
OBJMAN::getobjectvspeed(_UINT objno) {
    return object[objno].v_speed;
}

/*
 * OBJMAN::setlastdirection(_UINT objno,_UCHAR dirno)
 *
 * This will set the last direction number of object [objno] to [dirno].
 *
 */
void
OBJMAN::setlastdirection(_UINT objno,_UCHAR dirno) {
    object[objno].last_direction=dirno;
}

/*
 * OBJMAN::getcurwait(_UINT objno)
 *
 * This will return the current number of frames object [objno] has waited
 * since its last animation.
 *
 */
_UINT
OBJMAN::getcurwait(_UINT objno) {
    return object[objno].cur_wait;
}

/*
 * OBJMAN::setcurwait(_UINT objno,_UINT newcurwait)
 *
 * This will set the current number of frames the object has waited since
 * its last animation to [newcurwait].
 *
 */
void
OBJMAN::setcurwait(_UINT objno,_UINT newcurwait) {
    object[objno].cur_wait=newcurwait;
}

/*
 * OBJMAN::setspeed(_UINT objno,_UINT speed)
 *
 * This will set the number of frames object [objno] will wait before it will
 * switch animation/move to [speed]. If [speed] is zero, the object will not
 * wait.
 *
 */
void
OBJMAN::setspeed(_UINT objno,_UINT speed) {
    object[objno].speed=speed;
}

/*
 * OBJMAN::getspeed(_UINT objno)
 *
 * This will return the number of frames object [objno] will wait before it
 * will  switch animation/move.
 *
 */
_UINT
OBJMAN::getspeed(_UINT objno) {
    return object[objno].speed;
}

/*
 * OBJMAN::getmovesequence(_UINT objno,_UCHAR seqno)
 *
 * This will return move sequence number [seqno] for object [objno].
 *
 */
_UINT
OBJMAN::getmovesequence(_UINT objno,_UCHAR seqno) {
    return object[objno].move_sequence[seqno];
}

/*
 * OBJMAN::setmovesequence(_UINT objno,_UCHAR seqno,_UINT seq)
 *
 * This will set move sequence number [seqno] for object [objno] to [seq].
 *
 */
void
OBJMAN::setmovesequence(_UINT objno,_UCHAR seqno,_UINT seq) {
    object[objno].move_sequence[seqno]=seq;
}

/*
 * OBJMAN::setanimno(_UINT objno,_UCHAR animno)
 *
 * This will set the animation object number of object [objno] to [animno].
 *
 */
void
OBJMAN::setanimno(_UINT objno,_UCHAR animno) {
    object[objno].oldanimno=object[objno].animno;
    object[objno].animno=animno;
}

/*
 * OBJMAN::getlastdirection(_UINT objno)
 *
 * This will return the last direction number of object [objno].
 *
 */
_UCHAR
OBJMAN::getlastdirection(_UINT objno) {
    return object[objno].last_direction;
}

/*
 * OBJMAN::setframeno(_UINT objno,_UCHAR frameno)
 *
 * This will set the animation frame number of object [objno] to [frameno].
 *
 */
void
OBJMAN::setframeno(_UINT objno,_UCHAR frameno) {
    object[objno].oldframeno=object[objno].frameno;
    object[objno].frameno=frameno;
}

/*
 * OBJMAN::setyankcoords(_UINT objno,_SINT x,_SINT y)
 *
 * This will set the yank coordinates of object [objno] to ([x],[y])
 *
 */
void
OBJMAN::setyankcoords(_UINT objno,_SINT x,_SINT y) {
    object[objno].yank_x=x;
    object[objno].yank_y=y;
}

/*
 * OBJMAN::getobjectyankx(_UINT objno)
 *
 * This will return the yank X coordinate of object [objno]
 *
 */
_SINT
OBJMAN::getobjectyankx(_UINT objno) {
    return object[objno].yank_x;
}

/*
 * OBJMAN::getobjectyanky(_UINT objno)
 *
 * This will return the yank Y coordinate of object [objno]
 *
 */
_SINT
OBJMAN::getobjectyanky(_UINT objno) {
    return object[objno].yank_y;
}

/*
 * OBJMAN::setanimating(_UINT objno,_UCHAR animate)
 *
 * This will set whether object [objno] should animate or not.
 * If [animate] is 0, the object will stop animating, otherwise it
 * will animate.
 *
 */
void
OBJMAN::setanimating(_UINT objno,_UCHAR animate) {
    object[objno].animating=animate;
}

/*
 * OBJMAN::isanimating(_UINT objno)
 *
 * This will return 0 if object [objno] isn't animating, otherwise
 * a non-zero number.
 *
 */
_UCHAR
OBJMAN::isanimating(_UINT objno) {
    return object[objno].animating;
}

/*
 * OBJMAN::islooping(_UINT objno)
 *
 * This will return 0 if object [objno] isn't looping, otherwise a non-zero
 * number.
 *
 */
_UCHAR
OBJMAN::islooping(_UINT objno) {
    return object[objno].looping;
}

/*
 * OBJMAN::findobject(char* name)
 *
 * This will browse the object tree for an object named [name]. If it is
 * found, this will return the objects number, otherwise it will return
 * OBJMAN_NOBJECT.
 *
 */
_UINT
OBJMAN::findobject(char* name) {
    _UINT i;

    /* trace all objects */
    for(i=0;i<OBJMAN_MAXOBJECTS;i++) {
        /* is it used? */
        if (object[i].type!=OBJMAN_TYPE_UNUSED) {
            /* yes. if the name's equal, return this number, otherwise keep
               looking */
            if (!strcmp(name,object[i].name)) return i;
        }
    }
    /* the object wasn't found. return OBJMAN_NOBJECT */
    return OBJMAN_NOBJECT;
}

/*
 * OBJMAN::findfreeobject()
 *
 * This will browse the object tree for an unused entry. If it is found, it
 * will return its number, otherwise it will return OBJMAN_NOBJECT.
 *
 */
_UINT
OBJMAN::findfreeobject() {
    _UINT i;

    /* trace all objects */
    for(i=0;i<OBJMAN_MAXOBJECTS;i++) {
        /* is it unused? */
        if (object[i].type==OBJMAN_TYPE_UNUSED) {
            /* yeah. return this objects number */
            return i;
        }
    }
    /* a free object couldn't be found. return OBJMAN_NOBJECT */
    return OBJMAN_NOBJECT;
}


/*
 * OBJMAN::isclipping(_UINT objno)
 *
 * This will return 0 if object [objno] isn't clipping, otherwise
 * a non-zero number.
 *
 */
_UCHAR
OBJMAN::isclipping(_UINT objno) {
    return object[objno].clip;
}

/*
 * OBJMAN::setclipping(_UINT objno,_UCHAR clip)
 *
 * This will set the clipping state of object [objno] to [clip]. If [clip]
 * is zero, the object will not clip and it can stand anywhere. If [clip] is
 * non-zero, the object will clip, and must be ok in the priority screen.
 *
 */
void
OBJMAN::setclipping(_UINT objno,_UCHAR clip) {
    object[objno].clip=clip;
}

/*
 * OBJNO::unloadobject(_UINT objno)
 *
 * This will destroy object [objno] and unload any data it has.
 *
 */
void
OBJMAN::unloadobject( _UINT objno) {
    _UINT no,type;

    /* save source number and object type */
    no=object[objno].sourceno;
    type=object[objno].type;

    /* destroy the object */
    destroyobject(objno);

    /* unload the object data */
    if(type==OBJMAN_TYPE_SPRITE) gfx->unloadsprite(no);
    if(type==OBJMAN_TYPE_ANM) gfx->unloadanimation(no);
    if(type==OBJMAN_TYPE_TEXT) unloadtext(no);
}

/*
 * OBJMAN::getmaskdir(_UINT objno)
 *
 * This will return the mask direction of object [objno].
 *
 */
_UCHAR
OBJMAN::getmaskdir(_UINT objno) {
    return object[objno].mask_dir;
}

/*
 * OBJMAN::setmaskdir(_UINT objno,_UCHAR maskdir)
 *
 * This will set the mask direction of object [objno] to [maskdir].
 *
 */
void
OBJMAN::setmaskdir(_UINT objno,_UCHAR maskdir) {
    object[objno].mask_dir=maskdir;
}

/*
 * OBJMAN::setobjectspeed(_UINT objno,_UINT speed,_UINT hspeed,_UINT vspeed)
 *
 * This will set the speed of object [objno]. [speed] will be the number of
 * frames to wait until the object is updated, and ([hspeed],[vspeed]) will
 * be the amount of pixels in which the object will move.
 *
 */
void
OBJMAN::setobjectspeed(_UINT objno,_UINT speed,_UINT hspeed,_UINT vspeed) {
    object[objno].speed=speed;
    object[objno].h_speed=hspeed;
    object[objno].v_speed=vspeed;
    object[objno].cur_wait=0;

    /* tweak the x and y coordinates in shape */
    object[objno].destx-=(object[objno].destx%object[objno].h_speed);
    object[objno].desty-=(object[objno].desty%object[objno].v_speed);
}

/*
 * OBJMAN::getprioritycode(_UINT objno)
 *
 * This will return the priority code of object [objno].
 *
 */
_UCHAR
OBJMAN::getprioritycode(_UINT objno) {
    return object[objno].prioritycode;
}

/*
 * OBJMAN::setsourceno(_UINT objno,_ULONG sourceno)
 *
 * This will set the source data number of object [objno] to [sourceno].
 *
 */
void
OBJMAN::setsourceno(_UINT objno,_ULONG sourceno) {
    object[objno].sourceno=sourceno;
}

/*
 * OBJMAN::setbasepriority(_UINT objno,_UCHAR basepri)
 *
 * This will set the base priority of object [objno] to [basepri].
 *
 */
void
OBJMAN::setbasepriority(_UINT objno,_UCHAR basepri) {
    object[objno].base_priority=basepri;
}

/*
 * OBJMAN::setlooping(_UINT objno,_UCHAR loop)
 *
 * This set the looping state of [objno] to [loop].
 *
 */
void
OBJMAN::setlooping(_UINT objno,_UCHAR loop) {
    object[objno].looping=loop;
}

/*
 * OBJMAN::setanmobject(_UINT objno,char* datafile,char* name)
 *
 * This will set the state of object [objno] so it'll be an animation object
 * with the data from file [datafile] and name [name].
 */
void
OBJMAN::setanmobject(_UINT objno,char* datafile,char* name) {
    /* first make sure it's nuked */
    destroyobject(objno);

    /* now edit all fields */
    object[objno].type=GFX_TYPE_ANM;
    object[objno].x=0;
    object[objno].y=0;
    object[objno].dirty=1;
    object[objno].enabled=0;
    object[objno].base_priority=GFX_MAX_PRIORITIES;
    object[objno].move_dir_h=DIR_UNKNOWN;
    object[objno].move_dir_v=DIR_UNKNOWN;
    object[objno].sourceno=gfx->loadanm(datafile);
    strcpy(object[objno].datafile,datafile);
    strcpy(object[objno].name,name);
    object[objno].mask_dir=GFX_MASK_BOTTOM;
}

/*
 * OBJMAN::setdirection(_UINT objno,_UCHAR dirno)
 *
 * This will change object [objno] so it will face in direction [dirno].
 *
 */
void
OBJMAN::setdirection(_UINT objno,_UCHAR dirno) {
    /* first store old stuff */
    object[objno].oldframeno=object[objno].frameno;
    object[objno].oldanimno=object[objno].animno;

    /* now set new stuff */
    object[objno].animno=(_UCHAR)object[objno].move_sequence[SCRIPT_MOVE_HALT];
    object[objno].frameno=dirno;
}

/*
 * OBJMAN::setext(_UINT objno,char *text)
 *
 * This will change the text of object [objno] to [text]. It will return
 * zero if all went ok, anything else means the string was too long
 *
 */
_UCHAR
OBJMAN::setext(_UINT objno,char *text) {
    /* is the length valid? */
    if(strlen(text)>=OBJMAN_MAX_TEXT_LEN) {
        /* no. string is too long. return 1 */
        return 1;
    }
    /* yeah, copy it */
    strcpy(object[objno].text,text);

    /* say it's ok */
    return 0;
}

/*
 * OBJMAN::setcolor(_UINT objno,_UCHAR color)
 *
 * This will change object [objno] so the color will be [color]. This method
 * only does anything with text objects
 *
 */
void
OBJMAN::setcolor(_UINT objno,_UCHAR color) {
    object[objno].color=color;
}

/*
 * OBJMAN::setcolor(_UINT objno,_UCHAR color)
 *
 * This will return the color of object [objno]. This only does something
 * with text objects.
 *
 */
_UCHAR
OBJMAN::getcolor(_UINT objno) {
    return object[objno].color;
}

/*
 * OBJMAN::getext(_UINT objno)
 *
 * This will return the object's text.
 *
 */
char*
OBJMAN::getext(_UINT objno) {
    return object[objno].text;
}

/*
 * OBJMAN::setextobject(_UINT objno,char* name)
 *
 * This will set the state of object [objno] so it'll be a text object. It
 * will be named [name]. The font used will be from file [fontfile], which
 * will be loaded if neccesary.
 *
 */
void
OBJMAN::setextobject(_UINT objno,char* name,char* fontfile) {
    destroyobject(objno);

    object[objno].type=OBJMAN_TYPE_TEXT;
    object[objno].x=0;
    object[objno].y=0;
    object[objno].dirty=1;
    object[objno].enabled=0;
    object[objno].base_priority=GFX_MAX_PRIORITIES;
    object[objno].sourceno=fontman->loadfont(fontfile);
    strcpy(object[objno].name,name);
}

/*
 * OBJMAN::unloadtext(_UINT no)
 *
 * This will unload the data of font [no].
 *
 */
void
OBJMAN::unloadtext(_UINT no) {
    _UINT i,used;

    used=0;
    /* trace all text objects if this font is used */
    for(i=0;i<OBJMAN_MAXOBJECTS;i++) {
        /* has this one the sprite number of the sprite we
           want to unload ? */
        if ((object[i].sourceno==no)&&(object[i].type==OBJMAN_TYPE_TEXT)) {
            /* yes, so set flag */
            used=1;
        }
    }
    /* is it still used? if yes, just return */
    if(used) return;
    /* is it the system font? if yes, just return */
    if(no==fontman->getsysfontno()) return;

    /* well font manager, unload it */
    fontman->unloadfont(no);
}

/*
 * OBJMAN::getobjectptr(_ULONG objno)
 *
 * This will retrieve a pointer to the object data of [objno].
 *
 */
OBJECT*
OBJMAN::getobjectptr(_ULONG objno) {
    return &object[objno];
}

/*
 * OBJMAN::getdatafile(_UINT objno)
 *
 * This will receive the data file attached to object [objno].
 *
 */
char*
OBJMAN::getdatafile(_UINT objno) {
    return object[objno].datafile;
}

/*
 * OBJMAN::setdirty(_UINT objno,_UCHAR dirty)
 *
 * This will set the dirty state of [objno] to [dirty].
 *
 */
void
OBJMAN::setdirty(_UINT objno,_UCHAR dirty) {
    object[objno].dirty=dirty;
}

/*
 * OBJMAN::getobjectoldx(_UINT objno)
 *
 * This will return the old X coordinate of object [objno].
 *
 */
_SINT
OBJMAN::getobjectoldx(_UINT objno) {
    return object[objno].old_x;
}

/*
 * OBJMAN::getobjectoldy(_UINT objno)
 *
 * This will return the old Y coordinate of object [objno].
 *
 */
_SINT
OBJMAN::getobjectoldy(_UINT objno) {
    return object[objno].old_y;
}

/*
 * OBJMAN::setoldxy(_UINT objno,_SINT x,_SINT y)
 *
 * This will set the old X and Y coordinates of object [objno] to [x,y].
 *
 */
void
OBJMAN::setoldxy(_UINT objno,_SINT x,_SINT y) {
    object[objno].old_x=x;
    object[objno].old_y=y;
}

/*
 * OBJMAN::getoldframeno(_UINT objno)
 *
 * This will return the previously set frame number.
 *
 */
_UCHAR
OBJMAN::getoldframeno(_UINT objno) {
    /* if there's no old frame number, just return the current frame
       number */
    if (object[objno].oldframeno==GFX_NOANIM) return object[objno].frameno;

    /* return the old frame number */
    return object[objno].oldframeno;
}

/*
 * OBJMAN::getoldanimno(_UINT objno)
 *
 * This will return the previously set animation number.
 *
 */
_UCHAR
OBJMAN::getoldanimno(_UINT objno) {
    /* if there's no old animation number, just return the current animation
       number */
    if (object[objno].oldanimno==GFX_NOANIM) return object[objno].animno;

    /* return the old animation number */
    return object[objno].oldanimno;
}
