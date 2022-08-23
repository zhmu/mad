/*
                            DLGMAN.CPP

                         MAD Dialog Manager

                    (c) 1999, 2000 The MAD Crew

  Description:
  This will manage all MAD dialogs.

  Todo: Nothing

  Portability notes: Fully portable, no changes required
*/
#include <stdlib.h>
#include <string.h>
#include "controls.h"
#include "dlgman.h"
#include "fontman.h"
#include "gadgets.h"
#include "gfx.h"
#include "gfx_drv.h"
#include "objman.h"
#include "project.h"
#include "script.h"
#include "mad.h"
#include "maderror.h"
#include "textman.h"
#include "types.h"

/*
 * CONTROL::CONTROL()
 *
 * This is the constructor of a control.
 *
 */
CONTROL::CONTROL() {
    type=0;                           /* clear type */
    flags=0;                          /* default flags */
    x=0;y=0;                          /* top-left */
    height=0;width=0;                 /* make us 0 by 0 pixels */
    state=0;                          /* normal state */
    focus=0;                          /* no focus */
    color=DLGMAN_DIALOG_TITLECOLOR;   /* default color */
    curitem=0;
    nofitems=0;
    caption[0]=0;
    list_flags=0;
    baseno=0;
    spriteno=GFX_NOSPRITE;
    animno=GFX_NOANIM;
}

/*
 * CONTROL::init(_UCHAR type)
 *
 * This will initialize the control. It will set the type of the control
 * to [type].
 *
 */
void
CONTROL::init(_UCHAR desired_type) {
    /* set the type */
    type=desired_type;

    /* set the default name */
    if(type!=DLGMAN_CONTROL_TYPE_LIST) {
        strcpy(caption,DLGMAN_CONTROL_DEFAULTCAPTION);
    } else {
        caption[0]=0;
    }
}

/*
 * CONTROL::done()
 *
 * This will deinitialize the control. If there is sprite data, it will be
 * freed.
 *
 */
void
CONTROL::done() {
    /* an animation assigned to us? */
    if(animno!=GFX_NOANIM) {
        /* yeah, free it */
        gfx->unloadanimation(animno);
        /* make sure we don't do this twice */
        animno=GFX_NOANIM;spriteno=GFX_NOSPRITE;
    }
    /* any sprite data allocated? */
    if(spriteno!=GFX_NOSPRITE) {
        /* yeah, free it */
        gfx->unloadsprite(spriteno);
        /* and make sure we don't do this twice */
        spriteno=GFX_NOSPRITE;
    }
}

/*
 * CONTROL::move(_SINT dest_x,_SINT dest_y)
 *
 * This will set the coordinates of this control to [dest_x,dest_y]
 *
 */
void
CONTROL::move(_SINT dest_x,_SINT dest_y) {
    x=dest_x;y=dest_y;
}

/*
 * CONTROL::resize(_UINT dest_height,_UINT dest_width)
 *
 * This will set the height and width of this control to
 * [dest_height,dest_width]
 *
 */
void
CONTROL::resize(_UINT dest_height,_UINT dest_width) {
    height=dest_height;width=dest_width;
}

/*
 * CONTROL::setcaption(char* text)
 *
 * This will set the caption of the control to [text].
 *
 */
void
CONTROL::setcaption(char* text) {
    if(strlen(text)>DLGMAN_CONTROL_MAXCAPTIONLEN) return;
    strcpy(caption,text);
}

/*
 * CONTROL::setflags(_UINT desired_flags)
 *
 * This will set the flags of this control to [desired_flags]
 *
 */
void
CONTROL::setflags(_UINT desired_flags) {
    flags=desired_flags;
}

/*
 * CONTROL::setcolor(_UINT desired_color)
 *
 * This will set the color of the control to [desired_color]. This will only
 * affect label and text controls.
 *
 */
void
CONTROL::setcolor(_ULONG desired_color) {
    color=desired_color;
}

/*
 * CONTROL::getflags()
 *
 * This will return the flags of the control
 *
 */
_UINT
CONTROL::getflags() {
    return flags;
}

/*
 * CONTROL::draw(char* dest,_SINT basex,_SINT basey)
 *
 * This will cause the control to draw itself to [dest], beginning from
 * ([basex],[basey]).
 *
 */
void
CONTROL::draw(char* dest,_SINT basex,_SINT basey) {
    _UINT caption_width,c,n;
    _ULONG i,j;
    char tempstr[MAD_TEMPSTR_SIZE];
    struct ANM* anm;

    /* set [caption_width] to the width of the caption */
    caption_width=fontman->getextwidth(fontman->getsysfontno(),caption);
    switch(type) {
         case DLGMAN_CONTROL_TYPE_LABEL: /* We are a label! */
                                         fontman->textout(fontman->getsysfontno(),color,caption,basex+x,basey+y,FONTMAN_ALIGN_LEFT,dest);
                                         break;
        case DLGMAN_CONTROL_TYPE_BUTTON: /* We are a button! */
                                         if (state) {
                                             i=DLGMAN_BUTTON_BORDERCOL2;
                                             j=DLGMAN_BUTTON_BORDERCOL;
                                         } else {
                                           i=DLGMAN_BUTTON_BORDERCOL;
                                             j=DLGMAN_BUTTON_BORDERCOL2;
                                         }
                                         if(focus) {
                                             box(x+basex-1,y+basey-1,height+2,width+2,DLGMAN_FOCUSCOL,dest);
                                         }
                                         box(x+basex,y+basey,height,width,DLGMAN_BUTTON_BACKCOLOR,dest);
                                         hline(x+basex,y+basey,(x+width+basex),i,dest);
                                         vline(x+basex,y+basey,(y+height+basey),i,dest);
                                         hline(x+basex,(y+height+basey),(x+width+basex),j,dest);
                                         vline((x+width+basex),y+1+basey,(y+height+basey),j,dest);
                                         /* draw the caption */
                                         fontman->textout(fontman->getsysfontno(),DLGMAN_DIALOG_TITLECOLOR,caption,basex+x+((width-caption_width)/2),y+basey+1,FONTMAN_ALIGN_LEFT,dest);
                                         break;
     case DLGMAN_CONTROL_TYPE_TEXTINPUT: /* We are a text input box! */
                                         box(x+basex-1,y+basey-1,height+2,width+2,DLGMAN_TEXTINPUT_BORDERCOL,dest);
                                         box(x+basex,y+basey,height,width,DLGMAN_TEXTINPUT_BACKCOL,dest);
                                         /* draw the caption */
                                         fontman->textout(fontman->getsysfontno(),color,caption,basex+x,y+basey,FONTMAN_ALIGN_LEFT,dest);
                                         /* and draw the cursor (if we have the focus) */
                                         if(focus) {
                                             fontman->textout(fontman->getsysfontno(),color,DLGMAN_CONTROL_CURSOR,basex+x+fontman->getextwidth(fontman->getsysfontno(),caption),y+basey,FONTMAN_ALIGN_LEFT,dest);
                                         }
                                         break;
          case DLGMAN_CONTROL_TYPE_LIST: /* We are a list! */
                                         box(x+basex-1,y+basey-1,height+2,width+2,DLGMAN_TEXTINPUT_BORDERCOL,dest);
                                         box(x+basex,y+basey,height,width,DLGMAN_TEXTINPUT_BACKCOL,dest);

                                         /* figure out how many entries we can handle */
                                         i=(height/fontman->getfontheight(fontman->getsysfontno()))-2;

                                         /* draw the up button */
                                         if(list_flags&DLGMAN_CONTROL_LIST_FLAG_UP) {
                                             box(x+basex,y+basey,fontman->getfontheight(fontman->getsysfontno()),width,DLGMAN_TEXTINPUT_BORDERCOL,dest);
                                         } else {
                                             hline(x+basex,y+basey+fontman->getfontheight(fontman->getsysfontno()),x+basex+width,DLGMAN_TEXTINPUT_BORDERCOL,dest);
                                         }

                                         /* draw the 'up' text */
                                         fontman->textout(fontman->getsysfontno(),color,DLGMAN_LIST_UPBUTTON,x+basex+(width-fontman->getextwidth(fontman->getsysfontno(),DLGMAN_LIST_UPBUTTON))/2,y+basey,FONTMAN_ALIGN_LEFT,dest);

                                         n=i;
                                         if(i>nofitems) i=nofitems;

                                         /* draw them */
                                         for(j=0;j<i;j++) {
                                             getitem(j+baseno,tempstr);
                                             if(curitem==j+baseno) {
                                                 box(basex+x,y+basey+((j+1)*fontman->getfontheight(fontman->getsysfontno()))+1,fontman->getfontheight(fontman->getsysfontno())-2,width,DLGMAN_LIST_BACKSELCOLOR,dest);
                                                 c=DLGMAN_LIST_SELCOLOR;
                                             } else {
                                                 c=color;
                                             }
                                             fontman->textout(fontman->getsysfontno(),c,tempstr,x+basex,y+basey+((j+1)*fontman->getfontheight(fontman->getsysfontno())),FONTMAN_ALIGN_LEFT,dest);
                                         }

                                         /* draw the down button */
                                         if(list_flags&DLGMAN_CONTROL_LIST_FLAG_DOWN) {
                                             box(x+basex,y+basey+((n+1)*fontman->getfontheight(fontman->getsysfontno())),fontman->getfontheight(fontman->getsysfontno()),width,DLGMAN_TEXTINPUT_BORDERCOL,dest);
                                         } else {
                                             hline(x+basex,y+basey+((n+1)*fontman->getfontheight(fontman->getsysfontno())),x+basex+width,DLGMAN_TEXTINPUT_BORDERCOL,dest);
                                         }

                                         /* draw the 'down' text */
                                         fontman->textout(fontman->getsysfontno(),color,DLGMAN_LIST_DNBUTTON,x+basex+(width-fontman->getextwidth(fontman->getsysfontno(),DLGMAN_LIST_DNBUTTON))/2,y+basey+((n+1)*fontman->getfontheight(fontman->getsysfontno()))+1,FONTMAN_ALIGN_LEFT,dest);
                                         break;
        case DLGMAN_CONTROL_TYPE_SPRITE: /* We are a sprite! */
                                         /* Valid data assigned to us? */
                                         if(spriteno!=GFX_NOSPRITE) {
                                             gfx->putsprite(x+basex+1,y+basey+2,spriteno,dest);
                                         }
                                         /* if we have the focus, draw a box
                                            around it */
                                         if((focus)&&(spriteno!=GFX_NOSPRITE)) {
                                             hline(x+basex,y+basey,x+basex+gfx->getspritewidth(spriteno)+1,DLGMAN_CONTROL_SPRITE_BORDERCOL,dest);
                                             hline(x+basex,y+basey+gfx->getspriteheight(spriteno)+2,x+basex+gfx->getspritewidth(spriteno)+1,DLGMAN_CONTROL_SPRITE_BORDERCOL,dest);
                                             vline(x+basex,y+basey,y+basey+gfx->getspriteheight(spriteno)+1,DLGMAN_CONTROL_SPRITE_BORDERCOL,dest);
                                             vline(x+basex+gfx->getspritewidth(spriteno)+2,y+basey,y+basey+gfx->getspriteheight(spriteno)+2,DLGMAN_CONTROL_SPRITE_BORDERCOL,dest);
                                         }
                                         break;
           case DLGMAN_CONTROL_TYPE_ANM: /* animation! */
                                         /* do we have valid data? */
                                         if((animno!=GFX_NOANIM)&&(spriteno!=GFX_NOSPRITE)) {
                                             /* yes. draw it */
                                             anm=gfx->get_anim(animno);
                                             gfx->drawrgbdata(x+basex+1,y+basey+2,anm->sprite[spriteno].hdr->heigth,anm->sprite[spriteno].hdr->width,anm->sprite[spriteno].hdr->transcol_r,anm->sprite[spriteno].hdr->transcol_g,anm->sprite[spriteno].hdr->transcol_b,anm->sprite[spriteno].data,NULL,dest);
                                         }
                                         break;
    }
}

/*
 * CONTROL::istouched(_UINT posx,_UINT posy)
 *
 * This will return 1 if the control is touched if the cursor is in at
 * ([posx],[posy]).
 *
 */
_UCHAR
CONTROL::istouched(_UINT posx,_UINT posy) {
    if((posx>=x)&&(posx<=(x+width))&&(posy>=y)&&(posy<=(y+height))) {
        /* we are touched! */
        return 1;
    }
    /* we are not touched */
    return 0;
}

/*
 * CONTROL::setstate(_UCHAR newstate)
 *
 * This will set the state of the control to [newstate].
 *
 */
void
CONTROL::setstate(_UCHAR newstate) {
    state=newstate;
}

/*
 * CONTROL::setfocus(_UCHAR newfocus)
 *
 * This will set the focus of the control to [newfocus].
 *
 */
void
CONTROL::setfocus(_UCHAR newfocus) {
    focus=newfocus;
}

/*
 * CONTROL::hasfocus()
 *
 * This will return zero if the control doesn't have the focus, otherwise
 * a non-zero number.
 *
 */
_UCHAR
CONTROL::hasfocus() {
    return focus;
}

/*
 * CONTROL::getstate()
 *
 * This will return the state of the control.
 *
 */
_UCHAR
CONTROL::getstate() {
    return state;
}

/*
 * CONTROL::getype()
 *
 * This will return the type of the control.
 *
 */
_UCHAR
CONTROL::getype() {
    return type;
}

/*
 * CONTROL::getcaption()
 *
 * This will return the caption of the dialog.
 *
 */
char*
CONTROL::getcaption() {
    return caption;
}

/*
 * CONTROL::additem(char* name)
 *
 * This will add item [name] to the list.
 *
 */
void
CONTROL::additem(char* name) {
    /* add the text */
    strcat(caption,name);
    /* and the split char */
    strcat(caption,"|");

    /* update counters */
    nofitems++;
}

/*
 * CONTROL::getitem(_UINT no,char* dest)
 *
 * This will copy list item [no] to [dest].
 *
 */
void
CONTROL::getitem(_UINT no,char* dest) {
    _UINT i,j,c;
    char* ptr;

    ptr=caption;i=0;c=0;

    while(c!=no) {
        if(ptr[i]=='|') c++;

        /* next position */
        i++;
    }

    /* found the entry. now copy it to [dest] */
    j=0;
    while((ptr[i]!='|')&&(ptr[i]!=0)) {
        dest[j]=ptr[i];
        i++;j++;
    }
    dest[j]=0;
}

/*
 * CONTROL::finditem(char* name)
 *
 * This will scan the entry list for [name]. It will return its ID if it is
 * found or DLGMAN_CONTROL_LIST_NOENTRY if not.
 *
 */
_UINT
CONTROL::finditem(char* name) {
    _UINT i;
    char  tempstr[MAD_TEMPSTR_SIZE];

    /* scan all items */
    for(i=0;i<nofitems;i++) {
        /* get the item */
        getitem(i,tempstr);
        /* it is the one we are looking for? */
        if(!strcmp(tempstr,name)) {
            /* yeah, return its ID */
            return i;
        }
    }
    /* return DLGMAN_CONTROL_LIST_NOENTRY */
    return DLGMAN_CONTROL_LIST_NOENTRY;
}

/*
 * CONTROL::handlevent(_UINT dialogno,_UINT device_x,_UINT device_y)
 *
 * This will handle events of the control. [device_x] is the aiming device
 * X and [device_y] is the aiming device Y. [dialog_no] should be the number
 * of whatever dialog we are in.
 *
 */
void
CONTROL::handlevent(_UINT dialogno,_UINT device_x,_UINT device_y) {
    _UINT fh,basex,basey,i;
    _SINT j;

    basex=dlgman->dialog[dialogno]->getx();
    basey=dlgman->dialog[dialogno]->gety();

    /* are we a list control? */
    if(type==DLGMAN_CONTROL_TYPE_LIST) {
        /* yeah. handle events */

        /* figure out how many entries we can handle */
        i=(height/fontman->getfontheight(fontman->getsysfontno()))-3;

        /* set [fh] to the fontheight */
        fh=fontman->getfontheight(fontman->getsysfontno());

        /* check whether the 'up' section has been hit */
        if((device_x>=(basex+x))&&(device_y>=(basey+y+fh))&&
           (device_x<=(basex+x+width))&&(device_y<=(basey+y+fh+fh))) {
           /* it has. is the button down */
           if(controls->button1pressed()) {
               /* yeah. set flag */
               list_flags|=DLGMAN_CONTROL_LIST_FLAG_UP;
           } else {
               /* was the flag set? */
               if(list_flags&DLGMAN_CONTROL_LIST_FLAG_UP) {
                  /* yeah. avance the item number */
                  if(curitem!=0) {
                      curitem--;
                  }
                  /* and clear the flag */
                  list_flags&=~DLGMAN_CONTROL_LIST_FLAG_UP;
               }
           }
        } else {
            /* make sure up flag is not set */
            list_flags&=~DLGMAN_CONTROL_LIST_FLAG_UP;
        }

        /* set [j] to the y coordinate of the down text */
        j=y+basey+height;

        /* check whether the 'down' section has been hit */
        if((device_x>=(basex+x))&&(device_y>=(j))&&
           (device_x<=(basex+x+width))&&(device_y<=(j+fh))) {
           /* it has. is the button down */
           if(controls->button1pressed()) {
               /* yeah. set flag */
               list_flags|=DLGMAN_CONTROL_LIST_FLAG_DOWN;
           } else {
               /* was the flag set? */
               if(list_flags&DLGMAN_CONTROL_LIST_FLAG_DOWN) {
                  /* yeah. avance the item number */
                  if(curitem<(nofitems-1)) {
                      curitem++;
                  }
                  /* and clear the flag */
                  list_flags&=~DLGMAN_CONTROL_LIST_FLAG_DOWN;
               }
           }
        } else {
            /* make sure down flag is not set */
            list_flags&=~DLGMAN_CONTROL_LIST_FLAG_DOWN;
        }

        /* if the user just clicked, check whether it was on an item */
        if((device_x>=(basex+x))&&(device_x<=(basex+x+width))&&(controls->button1pressed())) {
            /* figure out the number */
            j=((device_y-y-basey)/fh)-2;

            if((j>=0)&&(j<=i)) {
                if((j+baseno)<nofitems) {
                    curitem=j+baseno;
                }
            }
        }

        /* check keyboard keys */
        /* is up hit? */
        if(controls->iskeydown(CONTROLS_KEYUP)) {
            /* yeah. handle it */
            if(curitem!=0) {
                curitem--;
            }
            /* nuke the keystroke */
            controls->clearkey(CONTROLS_KEYUP);
        }
        /* is down hit? */
        if(controls->iskeydown(CONTROLS_KEYDOWN)) {
            /* yeah. handle it */
            if(curitem<(nofitems-1)) {
                curitem++;
            }
            /* nuke the keystroke */
            controls->clearkey(CONTROLS_KEYDOWN);
        }
        /* is end hit? */
        if(controls->iskeydown(CONTROLS_KEY_END)) {
            /* yeah. handle it */
            curitem=nofitems-1;
            /* nuke the keystroke */
            controls->clearkey(CONTROLS_KEY_END);
        }
        /* is home hit? */
        if(controls->iskeydown(CONTROLS_KEY_HOME)) {
            /* yeah. handle it */
            curitem=0;
            /* nuke the keystroke */
            controls->clearkey(CONTROLS_KEY_HOME);
        }
        /* make sure the choice can be seen */
        while((baseno+i)<curitem) baseno++;
        while(curitem<baseno) baseno--;

        /* get outta here */
        return;
    }
}

/*
 * CONTROL::getcuritem()
 *
 * This will retrieve the currently selected item in a list box.
 *
 */
_UINT
CONTROL::getcuritem() {
    return curitem;
}

/*
 * CONTROL::loadsprite(char*)
 *
 * This will load a sprite to use for this control.
 *
 */
void
CONTROL::loadsprite(char* fname) {
    /* previous sprite no unloaded yet? */
    if(spriteno!=GFX_NOSPRITE) {
        /* nope. free it */
        gfx->unloadsprite(spriteno);
        /* make sure this is not done twice */
        spriteno=GFX_NOSPRITE;
    }
    spriteno=gfx->loadsprite(fname);
    /* and resize the control according to the sprite's dimensions */
    height=gfx->getspriteheight(spriteno);
    width=gfx->getspritewidth(spriteno);
}

/*
 * CONTROL::loadanim(char*)
 *
 * This will load an animation to use for this control.
 *
 */
void
CONTROL::loadanim(char* fname) {
    /* previous sprite no unloaded yet? */
    if(animno!=GFX_NOANIM) {
        /* nope. free it */
        gfx->unloadanimation(animno);
        /* make sure this is not done twice */
        animno=GFX_NOANIM;
    }
    animno=gfx->loadanm(fname);
    spriteno=GFX_NOSPRITE;
}


/*
 * CONTROL::setspriteno(_ULONG no)
 *
 * This will activate the animation's sprite number [no].
 *
 */
void
CONTROL::setspriteno(_ULONG no) {
    spriteno=no;
}

/*
 * DIALOG::DIALOG()
 *
 * This is the constructor of the dialog class. It will reset all variables.
 *
 */
DIALOG::DIALOG() {
    _UINT i;

    /* set all control[] pointers to NULL */
    for(i=0;i<DLGMAN_MAX_CONTROLS;i++) {
        control[i]=NULL;
    }
    for(i=0;i<DLGMAN_DIALOG_NOFBORDERS;i++) {
        border[i]=dlgman->default_border[i];
    }
    backdrop=dlgman->default_backdrop;
}

/*
 * DIALOG::init(_UINT theflags,_SINT thex,_SINT they,_UINT theheight,_UINT thewidth)
 *
 * This will initialize a dialog. It will put the dialog at [thex,they] and it
 * will be [theheight,thewidth] pixels long. The dialog will get flags
 * [theflags].
 *
 */
void
DIALOG::init(_UINT theflags,_SINT thex,_SINT they,_UINT theheight,_UINT thewidth) {
    _UINT i;

    x=thex; y=they; height=theheight; width=thewidth; flags=theflags;
    title=strdup(DLGMAN_DIALOG_DEFAULTITLE);

    /* need to ignore the defaults? */
    if(theflags&DLGMAN_DIALOG_FLAG_NODEFAULT) {
        /* yeah, set borders and backdrop to the GFX_NOSPRITE stuff */
        for(i=0;i<DLGMAN_DIALOG_NOFBORDERS;i++) {
            border[i]=GFX_NOSPRITE;
        }
        backdrop=GFX_NOSPRITE;
    }
}

/*
 * DIALOG::done()
 *
 * This will deinitialize the dialog. It will free all control memory used.
 *
 */
void
DIALOG::done() {
    _UINT i;

    /* do we have a title? */
    if (title!=NULL) {
        /* yup. free it */
        free(title);
        /* make sure we don't do this twice */
        title=NULL;
    }

    /* clear the borders */
    for(i=0;i<DLGMAN_DIALOG_NOFBORDERS;i++) {
        /* is a border loaded? */
        if ((border[i]!=GFX_NOSPRITE)&&(border[i]!=dlgman->default_border[i])) {
            /* yeah, unload the border */
            gfx->unloadsprite(border[i]);
            /* make sure we don't do this twice */
            border[i]=GFX_NOSPRITE;
        }
    }

    /* make sure the backdrop is freed */
    loadbackdrop(NULL);

    /* free all control[] pointers in use */
    for(i=0;i<DLGMAN_MAX_CONTROLS;i++) {
        /* is this control[] object used? */
        if(control[i]!=NULL) {
            /* yes. signal the control it's going to die */
            control[i]->done();
            /* and free the memory */
            delete control[i];

            /* set the control to null */
            control[i]=NULL;
        }
    }
}

/*
 * DIALOG::createcontrol(_UCHAR type)
 *
 * This will create a control with type [type]. It will die if there are no
 * more entries left to put the control in. It will return the control ID
 * if successful.
 *
 */
_UINT
DIALOG::createcontrol(_UCHAR type) {
    _UINT i;
    char tempstr[MAD_TEMPSTR_SIZE];

    /* scan for a free control */
    for(i=0;i<DLGMAN_MAX_CONTROLS;i++) {
        /* is this control[] object unused? */
        if(control[i]==NULL) {
            /* yes. let's use this one! */
            if((control[i]=new CONTROL)==NULL) {
                /* We were out of memory. die (error 7: cannot create object) */
                sprintf(tempstr,MAD_ERROR_7,"control");
                die(tempstr);
            }
            control[i]->init(type);
            /* return its id */
            return i;
        }
    }
    /* we are out of controls (Error 500: out of controls) */
    die(MAD_ERROR_500);
    /* NOTREACHED */

    /* this is to avoid errors, since it will never be reached */
    return 0;
}

/*
 * DIALOG::drawbackdrop(char* dest)
 *
 * This will draw the dialog back drop to [dest].
 *
 */
void
DIALOG::drawbackdrop(char* dest) {
    _UINT i,j,px,py,dw,dh;
    char *ptr;

    /* set ptr to the backdrop data */
    ptr=gfx->getspritedata(backdrop);
    /* set [dw] to the backdrop width and [dh] to the backdrop height */
    dw=gfx->getspritewidth(backdrop);
    dh=gfx->getspriteheight(backdrop);

    py=0;
    for(j=0;j<height;j++) {
        px=0;
        for(i=0;i<width;i++) {
            dest[gfx->linetable[j+y]+i+x]=(char)(ptr[dw*py+px+4]);
            px++;
            if(px==dw) px=0;
        } /* width */
        py++;
        if(py==(dh-1)) py=0;
    } /* height */
}

/*
 * DIALOG::drawborders(char* dest)
 *
 * This will draw the dialog borders to [dest].
 *
 */
void
DIALOG::drawborders(char* dest) {
    _UINT i,j,py,px,bw,bh,pos;
    _SINT by;
    char* ptr;

    /* set some variables */
    by=y;
    py=0;
    bh=gfx->getspriteheight(border[0])-2;
    /* set ptr to the border data */
    ptr=gfx->getspritedata(border[0]);

    /* draw the border at the left side */
    for(j=0;j<height;j++) {
        px=0;
        for(i=0;i<gfx->getspritewidth(border[0]);i++) {
            pos=i+x-gfx->getspritewidth(border[0]);
            if((pos<GFX_DRIVER_HRES)&&((j+by)<GFX_DRIVER_VRES)) {
                dest[gfx->linetable[j+by]+pos]=(char)(ptr[gfx->getspritewidth(border[0])*py+px+4]);
            }
            px++;
        } /* width */
        py++;
        if(py>=bh) py=0;
    } /* height */

    /* set some variables */
    by=y-gfx->getspriteheight(border[1]);
    py=0;
    bw=gfx->getspritewidth(border[1])-1;
    /* set ptr to the border data */
    ptr=gfx->getspritedata(border[1]);

    /* draw the border at the top */
    for(j=0;j<gfx->getspriteheight(border[1]);j++) {
        px=0;
        for(i=0;i<width+(2*gfx->getspritewidth(border[0]))-1;i++) {
            pos=i+x-gfx->getspritewidth(border[0]);
            if((pos<GFX_DRIVER_HRES)&&((j+by)<GFX_DRIVER_VRES)) {
                dest[gfx->linetable[j+by]+pos]=(char)(ptr[gfx->getspritewidth(border[1])*py+px+4]);
            }
            px++;
            if(px>=bw) px=0;
        } /* width */
        py++;
    } /* height */

    /* set some variables */
    by=y+height;
    py=0;
    bw=gfx->getspritewidth(border[2])-1;
    /* set ptr to the border data */
    ptr=gfx->getspritedata(border[2]);

    /* draw the border at the bottom */
    for(j=0;j<gfx->getspriteheight(border[2]);j++) {
        px=0;
        for(i=0;i<width+(2*gfx->getspritewidth(border[0]))-1;i++) {
            pos=i+x-gfx->getspritewidth(border[0]);
            if((pos<GFX_DRIVER_HRES)&&((j+by)<GFX_DRIVER_VRES)) {
                dest[gfx->linetable[j+by]+pos]=(char)(ptr[gfx->getspritewidth(border[2])*py+px+4]);
            }
            px++;
            if(px>=bw) px=0;
        } /* width */
        py++;
    } /* height */

    /* set some variables */
    by=y;
    py=0;
    bh=gfx->getspriteheight(border[3])-2;
    /* set ptr to the border data */
    ptr=gfx->getspritedata(border[3]);

    /* draw the border at the right side */
    for(j=0;j<height;j++) {
        px=0;
        for(i=0;i<gfx->getspritewidth(border[3]);i++) {
            pos=i+x+width;
            if((pos<GFX_DRIVER_HRES)&&((j+by)<GFX_DRIVER_VRES)) {
                dest[gfx->linetable[j+by]+pos]=(char)(ptr[gfx->getspritewidth(border[3])*py+px+4]);
            }
            px++;
        } /* width */
        py++;
        if(py>=bh) py=0;
    } /* height */
}

/*
 * DIALOG::draw(char* dest)
 *
 * This will draw the dialog to [dest].
 *
 */
void
DIALOG::draw(char* dest) {
    _UINT i,titley;

    /* need to draw a title bar? */
    if(flags&DLGMAN_DIALOG_FLAG_TITLEBAR) {
        titley=fontman->getfontheight(fontman->getsysfontno())+DLGMAN_DIALOG_SPARETITLEBARPIXELS-1;
    } else {
        titley=0;
    }
    /* if no backdrop, draw a normal dialog */
    if(backdrop==GFX_NOSPRITE) {
        /* draw the dialog */
        if (flags&DLGMAN_DIALOG_FLAG_GRAYBACK) {
            /* normal, solid background */
            grayoutbox(x,y,height+titley+DLGMAN_BORDER_HEIGHT,width,dest);
        } else {
            /* normal, solid background */
            box(x,y,height+titley+DLGMAN_BORDER_HEIGHT,width,DLGMAN_DIALOG_BACKCOLOR,dest);
        }
        /* need to draw a title bar? */
        if(titley) {
            /* yeah, create a title bar a la Windows */
            box(x,y+1,titley,width,DLGMAN_DIALOG_TITLEBARCOLOR,dest);
            /* draw the text into the title bar */
            fontman->textout(fontman->getsysfontno(),DLGMAN_DIALOG_TITLECOLOR,title,x+2,y+1,FONTMAN_ALIGN_LEFT,dest);
        }
        /* draw the border (|-) */
        hline(x,y,(x+width),DLGMAN_DIALOG_BORDERCOL,dest);
        vline(x,y,(y+height+titley+DLGMAN_BORDER_HEIGHT),DLGMAN_DIALOG_BORDERCOL,dest);
        /* and the border (_|) */
        hline(x,(y+height+titley+DLGMAN_BORDER_HEIGHT),(x+width),DLGMAN_DIALOG_BORDERCOL2,dest);
        vline((x+width),y+1,(y+height+titley+DLGMAN_BORDER_HEIGHT),DLGMAN_DIALOG_BORDERCOL2,dest);
    } else {
        /* draw a back drop */
        drawbackdrop(dest);
    }
    /* if there are borders, draw them! */
    if((border[0]!=GFX_NOSPRITE)&&(border[1]!=GFX_NOSPRITE)&&(border[2]!=GFX_NOSPRITE)&&(border[3]!=GFX_NOSPRITE)) {
        /* draw them! */
        drawborders(dest);
    }
    /* draw all control[] pointers in use */
    for(i=0;i<DLGMAN_MAX_CONTROLS;i++) {
        /* is this control[] object used? */
        if(control[i]!=NULL) {
            /* yes. draw it */
            control[i]->draw(dest,x+DLGMAN_BORDER_WIDTH,y+titley+DLGMAN_BORDER_HEIGHT);
        }
    }
}

/*
 * DIALOG::getx()
 *
 * This will return the X coordinate of the dialog.
 *
 */
_SINT
DIALOG::getx() {
    return x;
}

/*
 * DIALOG::gety()
 *
 * This will return the Y coordinate of the dialog.
 *
 */
_SINT
DIALOG::gety() {
    return y;
}

/*
 * DIALOG::getheight()
 *
 * This will return the height of the dialog.
 *
 */
_UINT
DIALOG::getheight() {
    return height;
}

/*
 * DIALOG::getwidth()
 *
 * This will return the width of the dialog.
 *
 */
_UINT
DIALOG::getwidth() {
    return width;
}

/*
 * DIALOG::move(_SINT dest_x,_SINT dest_y)
 *
 * This will set the coordinates of the dialog to [dest_x,dest_y]
 *
 */
void
DIALOG::move(_SINT dest_x,_SINT dest_y) {
    x=dest_x;y=dest_y;
}

/*
 * DIALOG::resize(_UINT dest_height,_UINT dest_width)
 *
 * This will resize the dialog to [dest_height],[dest_width].
 *
 */
void
DIALOG::resize(_UINT dest_height,_UINT dest_width) {
    height=dest_height; width=dest_width;
}

/*
 * DIALOG::getwidth()
 *
 * This will return the flags of the dialog.
 *
 */
_UINT
DIALOG::getflags() {
    return flags;
}

/*
 * DIALOG::getcontroltouched(_UINT posx,_UINT posy)
 *
 * This will return which control is touched if the cursor is at
 * ([posx],[posy]). If no control is touched, it will return
 * DLGMAN_DIALOG_NOHIT.
 *
 */
_UINT
DIALOG::getcontroltouched(_UINT posx,_UINT posy) {
    _UINT i,titley;

    if(flags&DLGMAN_DIALOG_FLAG_TITLEBAR) {
        /* yeah, create a title bar a la Windows */
        titley=fontman->getfontheight(fontman->getsysfontno())+DLGMAN_DIALOG_SPARETITLEBARPIXELS-1;
    } else {
        titley=0;
    }
    /* scan through all controls */
    for(i=0;i<DLGMAN_MAX_CONTROLS;i++) {
        /* is this control[] object used? */
        if(control[i]!=NULL) {
            /* is this one touched? */
            if(control[i]->istouched(posx-x-DLGMAN_BORDER_WIDTH,posy-y-titley-DLGMAN_BORDER_HEIGHT-1)) {
                /* yeah, return its id */
                return i;
            }
            /* no, keep looking */
        }
    }
    /* nothing was touched, so return DLGMAN_DIALOG_NOHIT */
    return DLGMAN_DIALOG_NOHIT;
}

/*
 * DIALOG::setitle(char* newtitle)
 *
 * This will set the dialog title to [newtitle].
 *
 */
void
DIALOG::setitle(char* newtitle) {
    title=strdup(newtitle);
}

/*
 * DIALOG::getitle()
 *
 * This will return the title of the dialog.
 *
 */
char*
DIALOG::getitle() {
    return title;
}

/*
 * DIALOG::centerdialog()
 *
 * This will center the dialog on the screen.
 *
 */
void
DIALOG::centerdialog() {
    x=(GFX_DRIVER_HRES-width)/2;
    y=(GFX_DRIVER_VRES-height)/2;
}

/*
 * DIALOG::cyclefocus(_UINT curfocusid)
 *
 * This will cycle the focus to the next control. The current control that
 * has the focus must be in [curfocusid]. This will return the new control id
 * which has focus.
 *
 */
_UINT
DIALOG::cyclefocus(_UINT curfocusid) {
    _UINT i,j;

    j=0;
    for(i=curfocusid+1;i<DLGMAN_MAX_CONTROLS;i++) {
        /* is this control used? */
        if(control[i]!=NULL) {
            /* yeah, is it a button control? */
            if(control[i]->getype()!=DLGMAN_CONTROL_TYPE_LABEL) {
                /* yeah, set the focus to this control */
                control[i]->setfocus(1);
                /* and clear the focus of the old control */
                control[curfocusid]->setfocus(0);
                /* we're outta here! */
                return i;
            }
        }
    }
    /* there are no controls behind us suitable for a focus. check before us */
    for(i=0;i<curfocusid;i++) {
        /* is this control used? */
        if(control[i]!=NULL) {
            /* yeah, is it a button control? */
            if(control[i]->getype()!=DLGMAN_CONTROL_TYPE_LABEL) {
                /* yeah, set the focus to this control */
                control[i]->setfocus(1);
                /* and clear the focus of the old control */
                control[curfocusid]->setfocus(0);
                /* we're outta here! */
                return i;
            }
        }
    }
    /* there are no controls suitable for the focus in this dialog, so just
       return the original id */
    return curfocusid;
}

/*
 * DIALOG::loadbackdrop(char* fname)
 *
 * This will load sprite [fname] as the backdrop of the dialog. If [fname] is
 * NULL, the current backdrop will be discarded.
 *
 */
void
DIALOG::loadbackdrop(char* fname) {
    /* need to discard the backdrop? */
    if(fname==NULL) {
        /* yeah. do it */
        /* invalid backdrop or default backdrop? */
        if((backdrop==GFX_NOSPRITE)||(backdrop==dlgman->default_backdrop)) {
            /* yeah. just ignore it */
            return;
        }
        gfx->unloadsprite(backdrop);
        /* set backdrop to invalid number */
        backdrop=GFX_NOSPRITE;
        /* get outta here! */
        return;
    }
    /* load a new back drop */
    backdrop=gfx->loadsprite(fname);
}

/*
 * DIALOG::loadborder(char* border1,char* border2,char* border3,char* border4)
 *
 * This will load a border for the dialog.
 *
 */
void
DIALOG::loadborder(char* border1,char* border2,char* border3,char* border4) {
    border[0]=gfx->loadsprite(border1);
    border[1]=gfx->loadsprite(border2);
    border[2]=gfx->loadsprite(border3);
    border[3]=gfx->loadsprite(border4);
}

/*
 * DLGMAN::DLGMAN()
 *
 * This is the constructor of the Dialog Manager.
 * It will reset all variables.
 *
 */
DLGMAN::DLGMAN() {
    _UINT i;

    /* set all dialogs to null pointers */
    for(i=0;i<DLGMAN_MAX_DIALOGS;i++) {
        dialog[i]=NULL;
    }
    /* reset default borders and backgrounds to no */
    for(i=0;i<DLGMAN_DIALOG_NOFBORDERS;i++) {
        default_border[i]=GFX_NOSPRITE;
    }
    default_backdrop=GFX_NOSPRITE;
};

/*
 * DLGMAN::init()
 *
 * This will initialize the Dialog Manager.
 *
 */
void
DLGMAN::init() {
    char tempstr[MAD_TEMPSTR_SIZE];

    /* is the PROJECT_FLAG_NODLG bit CLEARED? */
    if(!(project->info.flags&PROJECT_FLAG_NODLG)) {
        /* yeah. load the stuff! */
        /* load the borders */
        sprintf(tempstr,"%s%s",project->info.border,DLGMAN_DEFAULT_SPR0EXT);
        default_border[0]=gfx->loadsprite(tempstr);
        sprintf(tempstr,"%s%s",project->info.border,DLGMAN_DEFAULT_SPR1EXT);
        default_border[1]=gfx->loadsprite(tempstr);
        sprintf(tempstr,"%s%s",project->info.border,DLGMAN_DEFAULT_SPR2EXT);
        default_border[2]=gfx->loadsprite(tempstr);
        sprintf(tempstr,"%s%s",project->info.border,DLGMAN_DEFAULT_SPR3EXT);
        default_border[3]=gfx->loadsprite(tempstr);

        /* load the backdrop */
        memset(tempstr,0,MAD_TEMPSTR_SIZE);
        strncpy(tempstr,project->info.backdrop,8);
        strcat(tempstr,DLGMAN_DEFAULT_BACKEXT);
        default_backdrop=gfx->loadsprite(tempstr);
    }
}

/*
 * DLGMAN::done()
 *
 * This will deinitialize the Dialog Manager.
 *
 */
void
DLGMAN::done() {
    _UINT i;

    /* free all dialog[] pointers in use */
    for(i=0;i<DLGMAN_MAX_DIALOGS;i++) {
        /* is this dialog[] object used? */
        if(dialog[i]!=NULL) {
            /* yes. signal the dialog it's going to die */
            dialog[i]->done();
            /* and free the memory */
            delete dialog[i];

            /* reset the dialog to a null pointer */
            dialog[i]=NULL;
        }
    }
}

/*
 * DLGMAN::destroydialog(_UINT no)
 *
 * This will destroy dialog [no].
 *
 */
void
DLGMAN::destroydialog(_UINT no) {
   /* is the dialog used? */
    if(dialog[no]==NULL) {
       /* no. just return */
       return;
   }
   /* tell the dialog it's going to die */
   dialog[no]->done();
   delete dialog[no];

   /* reset the dialog to a null pointer */
   dialog[no]=NULL;
}

/*
 * DLGMAN::createdialog(_UINT flags,_SINT thex,_SINT they,_UINT theheight,_UINT thewidth)
 *
 * This will create a dialog with flags. It will die if there are no more
 * entries left. It will return the dialog ID if successful. The X and Y
 * coordinates of the dialog will be set the ([thex],[they]), and the height
 * and width to ([thewidth],[theheight]).
 *
 */
_UINT
DLGMAN::createdialog(_UINT flags,_SINT thex,_SINT they,_UINT theheight,_UINT thewidth) {
    _UINT i;
    char tempstr[MAD_TEMPSTR_SIZE];

    /* scan for a free control */
    for(i=0;i<DLGMAN_MAX_DIALOGS;i++) {
        /* is this control[] object unused? */
        if(dialog[i]==NULL) {
            /* yes. let's use this one! */
            if((dialog[i]=new DIALOG)==NULL) {
                /* We were out of memory. die (error 7: cannot create object) */
                sprintf(tempstr,MAD_ERROR_7,"dialog");
                die(tempstr);
            }
            dialog[i]->init(flags,thex,they,theheight,thewidth);
            /* return its id */
            return i;
        }
    }
    /* we are out of dialogs (Error 501: out of dialogs) */
    die(MAD_ERROR_501);
    /* NOTREACHED */

    /* this is to avoid errors, since it will never be reached */
    return 0;
}

/*
 * DLGMAN::drawdialog(_UINT no,_SINT device_x,_SINT device_y,char* tmpbuf)
 *
 * This will cause the dialog manager to redraw dialog [no]. The cursor is
 * supposed to be at ([device_x],[device_y]). [tmpbuf] is supposed to be
 * a buffer with size (GFX_DRIVER_HRES*GFX_DRIVER_VRES).
 *
 */
void
DLGMAN::drawdialog(_UINT no,_SINT device_x,_SINT device_y,char* displaydata) {
    /* get the game screen */
    gfx->getgamescreen(displaydata);

    /* draw the dialog box */
    dialog[no]->draw(displaydata);

    /* need to draw the cursor? */
    if(!(dialog[no]->getflags()&DLGMAN_DIALOG_FLAG_NOMOUSE)) {
        /* is there a valid mouse cursor? */
        if(gfx->get_mouse_spriteno()!=GFX_NOSPRITE) {
            /* draw the cursor */
            gfx->putsprite(device_x,device_y,gfx->get_mouse_spriteno(),displaydata);
        }
    }
    /* draw this */
    gfxdriver->draw(displaydata);
}

/*
 * DLGMAN::dodialog(_UINT no)
 *
 * This will handle dialog [no]. It will return the button id the user
 * clicked.
 *
 */
_UCHAR
DLGMAN::dodialog(_UINT no) {
    char  tempstr[MAD_TEMPSTR_SIZE];
    _SINT device_x,device_y,mx,my;
    _UINT i,retval,curfocus;
    _UCHAR c;
    char  *displaydata;

    /* allocate memory for a temponary display buffer */
    if((displaydata=(char*)malloc(GFX_DRIVER_HRES*GFX_DRIVER_VRES*GFX_BPP))==NULL) {
        /* out of memory. die (Error 2: out of memory (allocation of bytes failed) */
        sprintf(tempstr,MAD_ERROR_2,(GFX_DRIVER_HRES*GFX_DRIVER_VRES*GFX_BPP));
        die(tempstr);
    }
    /* default is the focus to the first control */
    curfocus=0;

    /* scan for the focused control */
    for(i=0;i<DLGMAN_MAX_CONTROLS;i++) {
        /* is this control[] object used? */
        if(dialog[no]->control[i]!=NULL) {
            /* yeah, does it have focus? */
            if(dialog[no]->control[i]->hasfocus()) {
                /* yeah, set [curfocus] to the control id */
                curfocus=i;
            }
        }
    }
    retval=0;
    /* keep handling it */
    while(1) {
        /* get the aiming device's X and Y coordinates */
        device_x=controls->getxpos();
        device_y=controls->getypos();

        /* Handle all control's events */
        for(i=0;i<DLGMAN_MAX_CONTROLS;i++) {
            /* Control there? */
            if(dialog[no]->control[i]!=NULL) {
                /* Yeah, handle it */
                dialog[no]->control[i]->handlevent(no,device_x,device_y);
            }
        }

        /* is button 1 pressed? */
        if(controls->button1pressed()) {
            /* did the user push a control? */
            i=dialog[no]->getcontroltouched(device_x,device_y);
            if(i!=DLGMAN_DIALOG_NOHIT) {
                /* Oh boy, a control was really hit! */
                dialog[no]->control[i]->setstate(1);
                while(controls->button1pressed()) {
                    /* get the aiming device's X and Y coordinates */
                    device_x=controls->getxpos();
                    device_y=controls->getypos();

                    /* Let it handle its events */
                    dialog[no]->control[i]->handlevent(no,device_x,device_y);

                    /* Draw the dialog */
                    drawdialog(no,device_x,device_y,displaydata);

                    /* If control not pushed, restore flag */
                    dialog[no]->control[i]->setstate(dialog[no]->getcontroltouched(device_x,device_y)==i);

                    /* poll the controls */
		            controls->poll();
                }
                /* if the control is still active and a button, return its id */
                if(dialog[no]->control[i]->getstate()) {
                    /* Is it a button? */
                    if(dialog[no]->control[i]->getype()==DLGMAN_CONTROL_TYPE_BUTTON) {
                        /* Yeah. Get outta here! */
                        retval=i+1;
                    }
                    /* Is is a sprite? */
                    if(dialog[no]->control[i]->getype()==DLGMAN_CONTROL_TYPE_SPRITE) {
                        /* Yeah. Get outta here! */
                        retval=i+1;
                    }
                }
                dialog[no]->control[i]->setstate(0);
            } else {
                /* does the dialog have a title bar and is flagged moveable? */
                if((dialog[no]->getflags()&DLGMAN_DIALOG_FLAG_TITLEBAR)&&(dialog[no]->getflags()&DLGMAN_DIALOG_FLAG_MOVEABLE)) {
                    /* yeah, did the user actual touch the bar? */
                    if((device_x>=dialog[no]->getx())&&(device_y>=dialog[no]->gety())&&
                       (device_y<=(dialog[no]->gety()+fontman->getfontheight(fontman->getsysfontno())))&&
                       (device_x<=(dialog[no]->getx()+dialog[no]->getwidth()))) {
                        /* yeah. let's move the dialog */
                        mx=device_x-dialog[no]->getx();
                        my=device_y-dialog[no]->gety();
                        while(controls->button1pressed()) {
                             device_x=controls->getxpos();
                             device_y=controls->getypos();

                             /* Set the dialog to the new position */
                             dialog[no]->move(device_x-mx,device_y-my);

                             /* Draw the dialog */
                             drawdialog(no,device_x,device_y,displaydata);

                             controls->poll();
                        } /* while button down */
                    } /* touch bar */
                } /* have title bar and moveable flag */
            }
        }
        /* did user press tab? */
        if(controls->iskeydown(CONTROLS_KEY_TAB)) {
            /* yeah, cycle to next button control */
            i=dialog[no]->cyclefocus(curfocus);
            curfocus=i;

            controls->clearkey(CONTROLS_KEY_TAB);
        }
        /* did user press the space bar on a non-text control? */
        if((controls->iskeydown(CONTROLS_KEY_SPACE))&&(dialog[no]->control[curfocus]->getype()!=DLGMAN_CONTROL_TYPE_TEXTINPUT)) {
            /* yeah, select this button */
            retval=curfocus+1;
            dialog[no]->control[curfocus]->setstate(1);
            while(controls->iskeydown(CONTROLS_KEY_SPACE)) {
                /* get the aiming device's X and Y coordinates */
                device_x=controls->getxpos();
                device_y=controls->getypos();

                /* Draw the dialog */
                drawdialog(no,device_x,device_y,displaydata);

                /* Poll the controls */
                controls->poll();
            }
            dialog[no]->control[curfocus]->setstate(0);
        }
        /* did the user press the enter key? */
        if(controls->iskeydown(CONTROLS_KEY_ENTER)) {
            /* yeah, select this button */
            retval=curfocus+1;

            /* kill the key */
            controls->waitforelease(CONTROLS_KEY_ENTER);
        }
        /* did the user press the escape key? */
        if(controls->iskeydown(CONTROLS_KEY_ESCAPE)) {
            retval=DLGMAN_DIALOG_NOHIT+1;
        }
        /* Need to quit? */
        if(retval) {
            /* yeah, return */
            retval--;
            break;
        }

        /* Is currently a text input control selected? */
        if(dialog[no]->control[curfocus]->getype()==DLGMAN_CONTROL_TYPE_TEXTINPUT) {
            /* Did the user press on a key on the keyboard? */
            if(controls->keydown()) {
                /* Yeah, is it backspace? */
                if(controls->iskeydown(CONTROLS_KEY_BSPACE)) {
                    /* yeah. is the text long enough? */
                    if(strlen(dialog[no]->control[curfocus]->getcaption())>0) {
                        /* yeah. strip the last char off */
                        strcpy(tempstr,dialog[no]->control[curfocus]->getcaption());
                        /* and nuke the last char */
                        tempstr[strlen(tempstr)-1]=0;
                        /* copy it back */
                        dialog[no]->control[curfocus]->setcaption(tempstr);
                        /* nuke it */
                        controls->clearkey(CONTROLS_KEY_BSPACE);
                    }
                } else {
                    /* nope, get a normal key */
                    i=controls->peekcode();
                    /* try to create an ascii char of the code */
                    c=controls->resolvecode(i&0xff);
                    /* did this work? */
                    if(c) {
                        /* yeah. add the char to the caption */
                        strcpy(tempstr,dialog[no]->control[curfocus]->getcaption());
                        /* add the char */
                        mx=strlen(tempstr);
                        tempstr[mx]=c;
                        tempstr[mx+1]=0;
                        /* copy it back */
                        dialog[no]->control[curfocus]->setcaption(tempstr);
                        /* nuke it */
                        controls->clearkey(i);
                    }

                }
                /* clear the keycode from the buffer */
            }
        }

        /* draw the dialog */
        drawdialog(no,device_x,device_y,displaydata);

        /* poll the controls */
	controls->poll();
    }

    /* free the buffer */
    free(displaydata);
    return retval;
}

/*
 * DLGMAN::messagebox(char* message,char* header)
 *
 * This will cause the dialog manager to pop up a dialog with message
 * [message] and header [header]. It will also wait until the OK button
 * in the dialog is clicked.
 *
 */
void
DLGMAN::messagebox(char* message,char* header) {
     _UINT i,j,no,text_height,text_width;

     /* figure out the height and width of the message */
     text_height=fontman->getextheight(fontman->getsysfontno(),message);
     text_width=fontman->getextwidth(fontman->getsysfontno(),message);

     i=fontman->getextwidth(fontman->getsysfontno(),message);

     /* create the dialog */
     i=text_height+(fontman->getfontheight(fontman->getsysfontno())*3);

     no=createdialog(DLGMAN_DIALOG_FLAG_TITLEBAR|DLGMAN_DIALOG_FLAG_MOVEABLE,0,0,i,text_width+10);

     /* neatly center the dialog */
     dialog[no]->centerdialog();

     /* create the OK button */
     i=dialog[no]->createcontrol(DLGMAN_CONTROL_TYPE_BUTTON);

     /* set the title */
     dialog[no]->setitle(header);

     /* check length of OK button message */
     j=fontman->getextwidth(fontman->getsysfontno(),DLGMAN_TEXT_OKBUTTON)+(2*fontman->getfontwidth(fontman->getsysfontno()));

     /* move the button to the correct place and resize it */
     dialog[no]->control[i]->move(((dialog[no]->getwidth()-j)/2),text_height+fontman->getfontheight(fontman->getsysfontno()));
     dialog[no]->control[i]->resize(fontman->getfontheight(fontman->getsysfontno())+1,j);
     dialog[no]->control[i]->setcaption(DLGMAN_TEXT_OKBUTTON);
     dialog[no]->control[i]->setfocus(1);

     /* create the label */
     i=dialog[no]->createcontrol(DLGMAN_CONTROL_TYPE_LABEL);

     dialog[no]->control[i]->move(0,0);
     dialog[no]->control[i]->resize(dialog[no]->getheight(),dialog[no]->getwidth());
     dialog[no]->control[i]->setcaption(message);

     /* take care of the dialog */
     dodialog(no);

     /* destroy the dialog */
     destroydialog(no);
}

/*
 * DLGMAN::askyesno(char* message,char* header)
 *
 * This will cause the dialog manager to pop up a dialog with message
 * [message] and header [header]. It will also wait until the Yes or No
 * button in the dialog is clicked. It will return zero if the No button
 * has been hit, otherwise a non-zero value.
 *
 */
_UCHAR
DLGMAN::askyesno(char* message,char* header) {
     _UINT i,j,no,text_height,text_width,idyes,idno;

     /* figure out the height and width of the message */
     text_height=fontman->getextheight(fontman->getsysfontno(),message);
     text_width=fontman->getextwidth(fontman->getsysfontno(),message);

     i=fontman->getextwidth(fontman->getsysfontno(),message);

     /* create the dialog */
     i=text_height+(fontman->getfontheight(fontman->getsysfontno())*3);

     no=createdialog(DLGMAN_DIALOG_FLAG_TITLEBAR|DLGMAN_DIALOG_FLAG_MOVEABLE,0,0,i,text_width+10);

     /* neatly center the dialog */
     dialog[no]->centerdialog();

     /* create the Yes and No buttons */
     idyes=dialog[no]->createcontrol(DLGMAN_CONTROL_TYPE_BUTTON);
     idno=dialog[no]->createcontrol(DLGMAN_CONTROL_TYPE_BUTTON);

     /* set the title */
     dialog[no]->setitle(header);

     /* check length of yes button message */
     j=fontman->getextwidth(fontman->getsysfontno(),DLGMAN_TEXT_YESBUTTON)+(2*fontman->getfontwidth(fontman->getsysfontno()));

     /* move the button to the correct place and resize it */
     dialog[no]->control[idyes]->move(fontman->getfontwidth(fontman->getsysfontno()),text_height+fontman->getfontheight(fontman->getsysfontno()));
     dialog[no]->control[idyes]->resize(fontman->getfontheight(fontman->getsysfontno())+1,j);
     dialog[no]->control[idyes]->setcaption(DLGMAN_TEXT_YESBUTTON);
     dialog[no]->control[idyes]->setfocus(1);

     /* check length of no button message */
     j=fontman->getextwidth(fontman->getsysfontno(),DLGMAN_TEXT_NOBUTTON)+(2*fontman->getfontwidth(fontman->getsysfontno()));

     /* move the button to the correct place and resize it */
     dialog[no]->control[idno]->move(dialog[no]->getwidth()-j-(2*fontman->getfontwidth(fontman->getsysfontno())),text_height+fontman->getfontheight(fontman->getsysfontno()));
     dialog[no]->control[idno]->resize(fontman->getfontheight(fontman->getsysfontno())+1,j);
     dialog[no]->control[idno]->setcaption(DLGMAN_TEXT_NOBUTTON);

     /* create the label */
     i=dialog[no]->createcontrol(DLGMAN_CONTROL_TYPE_LABEL);

     dialog[no]->control[i]->move(0,0);
     dialog[no]->control[i]->resize(dialog[no]->getheight(),dialog[no]->getwidth());
     dialog[no]->control[i]->setcaption(message);

     /* take care of the dialog */
     i=dodialog(no);

     /* destroy the dialog */
     destroydialog(no);

     /* return zero or non-zero */
     if(i==idyes) {
         /* yes button was hit! return a non-zero value */
         return 1;
     }
     /* no button must have been hit. return zero */
     return 0;
}

/*
 * DLGMAN::showmessage(char* message)
 *
 * This will show message [message].
 *
 */
void
DLGMAN::showmessage(char* message) {
     _UINT i,no,text_height,text_width,device_x,device_y;

     /* figure out the height and width of the message */
     text_height=fontman->getextheight(fontman->getsysfontno(),message);
     text_width=fontman->getextwidth(fontman->getsysfontno(),message);

     /* create the dialog */
     no=createdialog(0,0,0,text_height,text_width+10);

     /* neatly center the dialog */
     dialog[no]->centerdialog();

     /* create the label */
     i=dialog[no]->createcontrol(DLGMAN_CONTROL_TYPE_LABEL);

     dialog[no]->control[i]->move(0,0);
     dialog[no]->control[i]->resize(dialog[no]->getheight(),dialog[no]->getwidth());
     dialog[no]->control[i]->setcaption(message);

     controls->waitnobutton();

     /* take care of the dialog */
     while(1) {
         /* get the aiming device's X and Y coordinates */
         device_x=controls->getxpos();
         device_y=controls->getypos();

         /* redraw the stuff to the virtual screen */
         gfx->redraw(0);

         /* draw the dialog */
         drawdialog(no,device_x,device_y,gfx->virtualscreen);

         /* show the stuff */
         gfxdriver->draw(gfx->virtualscreen);

         /* get the state of the controls */
         controls->poll();

         /* if any key pressed or any button clicked, get outta here! */
         if(controls->button1pressed()||controls->button2pressed()) break;
         if(controls->getch()) break;
     }
     controls->waitnobutton();

     /* destroy the dialog */
     destroydialog(no);
}

/*
 * DLGMAN::alert(_ULONG)
 *
 * This will bring up an 'alert' dialog box for text id [id].
 *
 */
void
DLGMAN::alert(_ULONG id) {
     _UINT i,no,text_height,text_width,device_x,device_y;
     char  text[OBJMAN_MAX_TEXT_LEN];

     /* get the string */
     textman->getstring(id,text,OBJMAN_MAX_TEXT_LEN);

     /* figure out the height and width of the message */
     text_height=fontman->getextheight(fontman->getsysfontno(),text)+(fontman->getfontheight(fontman->getsysfontno())*2);
     text_width=fontman->getextwidth(fontman->getsysfontno(),text)+(fontman->getfontwidth(fontman->getsysfontno())*2);

     /* create the dialog */
     no=createdialog(DLGMAN_DIALOG_FLAG_GRAYBACK,0,0,text_height,text_width+10);

     /* neatly center the dialog */
     dialog[no]->centerdialog();

     /* create the label */
     i=dialog[no]->createcontrol(DLGMAN_CONTROL_TYPE_LABEL);

     dialog[no]->control[i]->move(fontman->getfontheight(fontman->getsysfontno())+4,fontman->getfontwidth(fontman->getsysfontno())+4);
     dialog[no]->control[i]->resize(dialog[no]->getheight(),dialog[no]->getwidth());
     dialog[no]->control[i]->setcaption(text);
     dialog[no]->control[i]->setcolor(0);

     controls->waitnobutton();

     /* take care of the dialog */
     while(1) {
         /* get the aiming device's X and Y coordinates */
         device_x=controls->getxpos();
         device_y=controls->getypos();

         /* redraw the stuff to the virtual screen */
         gfx->redraw(0);

         /* draw the dialog */
         drawdialog(no,device_x,device_y,gfx->virtualscreen);

         /* show the stuff */
         gfxdriver->draw(gfx->virtualscreen);

         /* get the state of the controls */
         controls->poll();

         /* if any key pressed or any button clicked, get outta here! */
         if(controls->button1pressed()||controls->button2pressed()) break;
         if(controls->getch()) break;
     }
     controls->waitnobutton();

     /* destroy the dialog */
     destroydialog(no);
}
