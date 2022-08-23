/*
                              DEBUGMAD.CPP

              MAD Debugging Engine - Handles all debugging

                    (c) 1998, 1999, 2000 The MAD Crew

  Description:
  This will handle all debugging, like the ALT-[key] combinations.

  Todo: Nothing.

  Portability notes: Fully portable, no changes required
*/
#include <stdlib.h>
#include <string.h>
#include "controls.h"
#include "debugmad.h"
#include "dlgman.h"
#include "fontman.h"
#include "gfx.h"
#include "gfx_drv.h"
#include "objman.h"
#include "script.h"
#include "mad.h"
#include "maderror.h"

#ifdef DEBUG_AVAILABLE
_UINT debug_dialogno;
_UINT debug_scriptlabel_id;
_UINT debug_textlabel_id;
_UINT debug_regs_id;
_UINT debug_objinfo_id;
_UCHAR debug_active=0;
_UCHAR debug_step=0;
_UCHAR debug_gfxmode=DEBUG_GFXMODE_BACKSCR;

#define DEBUG_MESSAGE_BREAKPOINTITLE "Breakpoints"
#define DEBUG_MESSAGE_PROCTITLE "Procedure"
#define DEBUG_MESSAGE_NOBREAKPOINT "<unused breakpoint>"
#define DEBUG_MESSAGE_SCRIPTLABEL "%s (ip=$%04x%04x)"
#define DEBUG_MESSAGE_INVALIDIP "<invalid ip>"

#define DEBUG_PROCS_PER_DIALOG 8
#define DEBUG_MESSAGE_BUTTON_DELETEBP "Nuke"
#define DEBUG_MESSAGE_OUTOFBREAKPOINTS "Out of breakpoints"
#define DEBUG_MESSAGE_BUTTON_GOPROC "Go"
#define DEBUG_MESSAGE_BUTTON_UP "Up"
#define DEBUG_MESSAGE_BUTTON_DOWN "Down"
#define DEBUG_MESSAGE_OBJECTSTITLE "Objects"
#define DEBUG_MESSAGE_UNUSEDOBJ "<unused object>"
#define DEBUG_MESSAGE_NOEGOBJECT "<no ego object set>"
#define DEBUG_MESSAGE_EGOINFO "Obj $%x: Pos (%u,%u) DPos (%u,%u) PC: %u BP: %u V: %u M: %u"
#define DEBUG_MESSAGE_REGINFO "A: $%04x\nF: %s\nsp: $%02x\n(sp) = $%04x"

#define DEBUG_ANIM_TITLE "Animations"
#define DEBUG_ANIM_VIEW " View "
#define DEBUG_ANIM_OK " OK "
#define DEBUG_ANIM_CANCEL " Cancel "
#define DEBUG_ANIM_SPRITE "Sprite: %lu of %lu"
#define DEBUG_ANIM_NEXT " Next "
#define DEBUG_ANIM_PREV " Prev "
#define DEBUG_ANIM_SELECTANIM " Anim... "

#define DEBUG_TELEPORT_TITLE "Teleport"
#define DEBUG_TELEPORT_ROOMNO "Room Number"
#define DEBUG_TELEPORT_DEFROOMNO "888"
#define DEBUG_TELEPORT_NOTVALID "'%s' is not a valid number"

/*
 * debug_init()
 *
 * This will initialize the debugging module.
 *
 */
void
debug_init() {
    _UINT width,height;

    width=GFX_DRIVER_HRES-1; height=50; debug_active=0; debug_step=0;
    debug_gfxmode=DEBUG_GFXMODE_BACKSCR;

    /* create the dialog */
    debug_dialogno=dlgman->createdialog(DLGMAN_DIALOG_FLAG_NOMOUSE|DLGMAN_DIALOG_FLAG_NODEFAULT|DLGMAN_DIALOG_FLAG_GRAYBACK,GFX_DRIVER_HRES-width-1,0,height,width);
    /* create the script name label in it */
    debug_scriptlabel_id=dlgman->dialog[debug_dialogno]->createcontrol(DLGMAN_CONTROL_TYPE_LABEL);
    dlgman->dialog[debug_dialogno]->control[debug_scriptlabel_id]->move(0,0);
    dlgman->dialog[debug_dialogno]->control[debug_scriptlabel_id]->resize(fontman->getfontheight(fontman->getsysfontno())+1,width);
    /* create the script text label in it */
    debug_textlabel_id=dlgman->dialog[debug_dialogno]->createcontrol(DLGMAN_CONTROL_TYPE_LABEL);
    dlgman->dialog[debug_dialogno]->control[debug_textlabel_id]->move(0,fontman->getfontheight(fontman->getsysfontno())+1);
    dlgman->dialog[debug_dialogno]->control[debug_textlabel_id]->resize(fontman->getfontheight(fontman->getsysfontno())+1,width);
    /* create the register text label in it */
    debug_regs_id=dlgman->dialog[debug_dialogno]->createcontrol(DLGMAN_CONTROL_TYPE_LABEL);
    dlgman->dialog[debug_dialogno]->control[debug_regs_id]->move(GFX_DRIVER_HRES-(16*fontman->getfontwidth(fontman->getsysfontno())),0);
    dlgman->dialog[debug_dialogno]->control[debug_regs_id]->resize(fontman->getfontheight(fontman->getsysfontno())+1,width);
    /* create the object info label in it */
    debug_objinfo_id=dlgman->dialog[debug_dialogno]->createcontrol(DLGMAN_CONTROL_TYPE_LABEL);
    dlgman->dialog[debug_dialogno]->control[debug_objinfo_id]->move(0,4*fontman->getfontheight(fontman->getsysfontno()));
    dlgman->dialog[debug_dialogno]->control[debug_objinfo_id]->resize(fontman->getfontheight(fontman->getsysfontno())+1,width);
}

/*
 * debug_done()
 *
 * This will deinitialize the debugging module.
 *
 */
void
debug_done() {
    /* nothing needs to be deinitialized yet */
}

/*
 * debug_listbreakpoints()
 *
 * This will create a window which lists all break points.
 *
 */
void
debug_listbreakpoints() {
    _UINT no,i,j,result,idok,lx,ly;
    _UINT width,height;
    _UINT breakpoint_no[DEBUG_MAX_BREAKPOINTS];
    _UINT breakpoint_del[DEBUG_MAX_BREAKPOINTS];
    char  tempstr[MAD_TEMPSTR_SIZE];

    width=200; height=125; debug_active=0; debug_step=0;

    /* create the dialog */
    no=dlgman->createdialog(DLGMAN_DIALOG_FLAG_TITLEBAR|DLGMAN_DIALOG_FLAG_MOVEABLE,0,0,height,width);

    /* create the breakpoint labels in it */
    for(i=0;i<DEBUG_MAX_BREAKPOINTS;i++) {
        /* set [lx] to the breakpoint label length and [ly] to the y coordinate of it */
        lx=width-(fontman->getextwidth(fontman->getsysfontno(),DEBUG_MESSAGE_BUTTON_DELETEBP)*2);
        ly=2+i*(fontman->getfontheight(fontman->getsysfontno())+4);

        /* create the breakpoint and set it up */
        breakpoint_no[i]=dlgman->dialog[no]->createcontrol(DLGMAN_CONTROL_TYPE_LABEL);
        dlgman->dialog[no]->control[breakpoint_no[i]]->move(0,ly);
        dlgman->dialog[no]->control[breakpoint_no[i]]->resize(fontman->getfontheight(fontman->getsysfontno())+1,lx);

        /* let's be pessimistic and default to no breakpoint */
//        strcpy(tempstr,DEBUG_MESSAGE_NOBREAKPOINT);
//        if(mainscript->mainscriptactive()) {
            /* check the main (root) script */
//            if(mainscript->getbreakpoint(i)!=SCRIPT_NOBREAKPOINTADDRESS) {
                /* wow, got a breakpoint here! */
//                sprintf(tempstr,"%s/$%04x%04x",mainscript->getfilename(),mainscript->getbreakpoint(i)>>16,mainscript->getbreakpoint(i)&0xffff);
//            }
//        } else {
            /* check the secondary script */
//             if(script->getbreakpoint(i)!=SCRIPT_NOBREAKPOINTADDRESS) {
               /* wow, got a breakpoint here! */
//                sprintf(tempstr,"%s/$%04x%04x",script->getfilename(),script->getbreakpoint(i)>>16,script->getbreakpoint(i)&0xffff);
//            }
//       }
        dlgman->dialog[no]->control[breakpoint_no[i]]->setcaption(tempstr);
        /* get the length of the ok message */
        j=fontman->getextwidth(fontman->getsysfontno(),DEBUG_MESSAGE_BUTTON_DELETEBP)+(2*fontman->getfontwidth(fontman->getsysfontno()));

        /* create the 'delete' button next to it */
        breakpoint_del[i]=dlgman->dialog[no]->createcontrol(DLGMAN_CONTROL_TYPE_BUTTON);
        dlgman->dialog[no]->control[breakpoint_del[i]]->move(lx,ly);
        dlgman->dialog[no]->control[breakpoint_del[i]]->resize(fontman->getfontheight(fontman->getsysfontno())+1,j);
        dlgman->dialog[no]->control[breakpoint_del[i]]->setcaption(DEBUG_MESSAGE_BUTTON_DELETEBP);
    }
    /* create an ok button */
    idok=dlgman->dialog[no]->createcontrol(DLGMAN_CONTROL_TYPE_BUTTON);

    /* get the length of the ok message */
    j=fontman->getextwidth(fontman->getsysfontno(),DLGMAN_TEXT_OKBUTTON)+(2*fontman->getfontwidth(fontman->getsysfontno()));

    /* move the button to the correct place and resize it */
    dlgman->dialog[no]->control[idok]->move(((dlgman->dialog[no]->getwidth()-j)/2),dlgman->dialog[no]->getheight()-(2*fontman->getfontheight(fontman->getsysfontno())));
    dlgman->dialog[no]->control[idok]->resize(fontman->getfontheight(fontman->getsysfontno())+1,j);
    dlgman->dialog[no]->control[idok]->setcaption(DLGMAN_TEXT_OKBUTTON);
    dlgman->dialog[no]->control[idok]->setfocus(1);

    /* center the dialog */
    dlgman->dialog[no]->centerdialog();

    /* set the dialog title */
    dlgman->dialog[no]->setitle(DEBUG_MESSAGE_BREAKPOINTITLE);

    /* do the dialog */
    while((result=dlgman->dodialog(no))!=idok) {
         /* did the user touch one of those 'delete' buttons? */
         if((result>=breakpoint_del[0])&&(result<=breakpoint_del[DEBUG_MAX_BREAKPOINTS-1])) {
             /* yeah. figure out the id code (order is [label[button] etc) */
             j=(result-breakpoint_del[0])>>1;
             /* are we in the main (root) script? */
//             if(mainscript->mainscriptactive()) {
                 /* yeah. nuke the breakpoint in the main (root) script */
//                 mainscript->clearbreakpoint(j);
//             } else {
                 /* nope. nuke the breakpoint in the secondary script */
//                 script->clearbreakpoint(j);
//             }
             /* and update the label */
            dlgman->dialog[no]->control[breakpoint_no[j]]->setcaption(DEBUG_MESSAGE_NOBREAKPOINT);
        }
    }

    /* and nuke it */
    dlgman->destroydialog(no);
}

/*
 * debug_listobjects()
 *
 * This will pop up a window which lists all objects currently in use.
 *
 */
void
debug_listobjects() {
    _UINT width,height,fontheight,fontwidth,base_object_no;
    _UINT i,no,object[DEBUG_WINDOW_NOF_OBJS],idup,iddn,idok,result;
    char  tempstr[MAD_TEMPSTR_SIZE];

    fontheight=fontman->getfontheight(fontman->getsysfontno());
    fontwidth=fontman->getfontwidth(fontman->getsysfontno());
    width=200; height=(fontheight*(3+DEBUG_WINDOW_NOF_OBJS)); base_object_no=0;

    /* create the dialog */
    no=dlgman->createdialog(DLGMAN_DIALOG_FLAG_TITLEBAR|DLGMAN_DIALOG_FLAG_MOVEABLE,0,0,height,width);

    /* set the dialog title */
    dlgman->dialog[no]->setitle(DEBUG_MESSAGE_OBJECTSTITLE);

    /* create all object names in it */
    for(i=0;i<DEBUG_WINDOW_NOF_OBJS;i++) {
        /* create the object label */
        object[i]=dlgman->dialog[no]->createcontrol(DLGMAN_CONTROL_TYPE_LABEL);

        /* move and resize it */
        dlgman->dialog[no]->control[object[i]]->move(0,i*fontheight+1);
        dlgman->dialog[no]->control[object[i]]->resize(fontheight,width-((strlen(DEBUG_MESSAGE_BUTTON_DOWN)+8)*fontwidth));
    }

    /* create the up button */
    idup=dlgman->dialog[no]->createcontrol(DLGMAN_CONTROL_TYPE_BUTTON);

    /* move and resize it */
    dlgman->dialog[no]->control[idup]->move(width-((strlen(DEBUG_MESSAGE_BUTTON_UP)+4)*fontwidth),1);
    dlgman->dialog[no]->control[idup]->resize(fontheight,fontman->getextwidth(fontman->getsysfontno(),DEBUG_MESSAGE_BUTTON_UP)+(2*fontwidth));
    dlgman->dialog[no]->control[idup]->setcaption(DEBUG_MESSAGE_BUTTON_UP);

    /* create the down button */
    iddn=dlgman->dialog[no]->createcontrol(DLGMAN_CONTROL_TYPE_BUTTON);

    /* move and resize it */
    dlgman->dialog[no]->control[iddn]->move(width-((strlen(DEBUG_MESSAGE_BUTTON_DOWN)+4)*fontwidth),((DEBUG_WINDOW_NOF_OBJS-1)*fontheight)+1);
    dlgman->dialog[no]->control[iddn]->resize(fontheight,fontman->getextwidth(fontman->getsysfontno(),DEBUG_MESSAGE_BUTTON_DOWN)+(2*fontwidth));
    dlgman->dialog[no]->control[iddn]->setcaption(DEBUG_MESSAGE_BUTTON_DOWN);

    /* create the ok button */
    idok=dlgman->dialog[no]->createcontrol(DLGMAN_CONTROL_TYPE_BUTTON);

    /* move and resize it */
    dlgman->dialog[no]->control[idup]->move(width-((strlen(DEBUG_MESSAGE_BUTTON_UP)+4)*fontwidth),1);
    dlgman->dialog[no]->control[idup]->resize(fontheight,fontman->getextwidth(fontman->getsysfontno(),DEBUG_MESSAGE_BUTTON_UP)+(2*fontwidth));
    dlgman->dialog[no]->control[idup]->setcaption(DEBUG_MESSAGE_BUTTON_UP);

    /* get the length of the ok message */
    i=fontman->getextwidth(fontman->getsysfontno(),DLGMAN_TEXT_OKBUTTON)+(2*fontman->getfontwidth(fontman->getsysfontno()));

    /* move the button to the correct place and resize it */
    dlgman->dialog[no]->control[idok]->move(((width-i)/2),height-(2*fontman->getfontheight(fontman->getsysfontno())));
    dlgman->dialog[no]->control[idok]->resize(fontman->getfontheight(fontman->getsysfontno())+1,i);
    dlgman->dialog[no]->control[idok]->setcaption(DLGMAN_TEXT_OKBUTTON);
    dlgman->dialog[no]->control[idok]->setfocus(1);

    /* center the dialog */
    dlgman->dialog[no]->centerdialog();

    result=DLGMAN_DIALOG_NOHIT;
    do {
        /* need to scroll down? */
        if(result==iddn) {
            /* user wants to scroll down. is it in range? */
            if(base_object_no<(OBJMAN_MAXOBJECTS-DEBUG_WINDOW_NOF_OBJS)) {
                /* yeah. do it */
                base_object_no++;
            }
        }
        /* need to scroll up? */
        if(result==idup) {
            /* user wants to scroll up. is it in range? */
            if(base_object_no>0) {
                /* yeah. do it */
                base_object_no--;
            }
        }
        /* fill in the object labels */
        for(i=0;i<DEBUG_WINDOW_NOF_OBJS;i++) {
            if(objman->getype(i+base_object_no)!=OBJMAN_TYPE_UNUSED) {
                sprintf(tempstr,"$%x: %s",(i+base_object_no),objman->getobjectname(i+base_object_no));
                dlgman->dialog[no]->control[object[i]]->setcaption(tempstr);
            } else {
                sprintf(tempstr,"$%x: %s",(i+base_object_no),DEBUG_MESSAGE_UNUSEDOBJ);
                dlgman->dialog[no]->control[object[i]]->setcaption(tempstr);
            }
        }
    } while ((result=dlgman->dodialog(no))!=idok);

    /* nuke the dialog */
    dlgman->destroydialog(no);
}

_UINT
debug_selectanm(ANM* anm) {
    _UINT dialogno,height,width,list,i,j,c,ok,cancel;
    _SINT count;
    DIALOG* dlg;
    char   tempstr[MAD_TEMPSTR_SIZE];

    /* set height and width */
    height=12*fontman->getfontheight(fontman->getsysfontno()); width=200;

    /* create a dialog */
    dialogno=dlgman->createdialog(DLGMAN_DIALOG_FLAG_TITLEBAR|DLGMAN_DIALOG_FLAG_MOVEABLE,0,0,height,width);
    dlg=dlgman->dialog[dialogno];

    /* set the title */
    dlg->setitle(DEBUG_ANIM_TITLE);

    /* center it */
    dlg->centerdialog();

    /* create the list */
    list=dlg->createcontrol(DLGMAN_CONTROL_TYPE_LIST);

    /* set it up */
    dlg->control[list]->move(fontman->getfontwidth(fontman->getsysfontno()),fontman->getfontheight(fontman->getsysfontno()));
    dlg->control[list]->resize(9*fontman->getfontheight(fontman->getsysfontno()),width-(4*DLGMAN_BORDER_WIDTH)-fontman->getfontwidth(fontman->getsysfontno()));

    /* create the OK button */
    ok=dlg->createcontrol(DLGMAN_CONTROL_TYPE_BUTTON);

    /* set it up */
    dlg->control[ok]->move(fontman->getfontwidth(fontman->getsysfontno()),10*fontman->getfontheight(fontman->getsysfontno())+4);
    dlg->control[ok]->resize(fontman->getfontheight(fontman->getsysfontno()),fontman->getextwidth(fontman->getsysfontno(),DEBUG_ANIM_OK));
    dlg->control[ok]->setcaption(DEBUG_ANIM_OK);

    /* create the ok button */
    cancel=dlg->createcontrol(DLGMAN_CONTROL_TYPE_BUTTON);

    /* set it up */
    dlg->control[cancel]->move(width-(4*DLGMAN_BORDER_WIDTH)-fontman->getextwidth(fontman->getsysfontno(),DEBUG_ANIM_CANCEL),10*fontman->getfontheight(fontman->getsysfontno())+4);
    dlg->control[cancel]->resize(fontman->getfontheight(fontman->getsysfontno()),fontman->getextwidth(fontman->getsysfontno(),DEBUG_ANIM_CANCEL));
    dlg->control[cancel]->setcaption(DEBUG_ANIM_CANCEL);

    /* now, scan all animations */
    for(i=0;i<anm->header->nofanims;i++) {
        dlg->control[list]->additem(anm->anim[i].anm.name);
    }

    /* handle the dialog */
    i=dlgman->dodialog(dialogno);

    /* did the user click the 'CANCEL' button? */
    if (i==cancel) {
        /* yup. nuke the dialog and just get out of here */
        dlgman->destroydialog(dialogno);
        return GFX_NOANIM;
    }

    /* no. figure out the index */
    j=dlg->control[list]->getcuritem();

    /* nuke the dialog */
    dlgman->destroydialog(dialogno);

    return j;
}

void
debug_viewanm(_UINT no) {
    _UINT dialogno,height,width,i,ok,nameinf,sprinf,animinf,prev,next,img,selanm;
    _ULONG spriteno,animno,frameno;
    DIALOG* dlg;
    struct ANM* anm;
    char   tempstr[MAD_TEMPSTR_SIZE];

    /* retrieve the pointer to the animation */
    anm=gfx->get_anim(no);

    /* is it loaded? */
    if(!anm->loaded) {
        /* no. leave */
        return;
    }

    /* set height and width */
    height=14*fontman->getfontheight(fontman->getsysfontno()); width=270;

    /* create a dialog */
    dialogno=dlgman->createdialog(DLGMAN_DIALOG_FLAG_TITLEBAR|DLGMAN_DIALOG_FLAG_MOVEABLE,0,0,height,width);
    dlg=dlgman->dialog[dialogno];

    /* set the title */
    sprintf(tempstr,"%s - %s",DEBUG_ANIM_TITLE,anm->filename);
    dlg->setitle(tempstr);

    /* center it */
    dlg->centerdialog();

    /* create the animation information label */
    nameinf=dlg->createcontrol(DLGMAN_CONTROL_TYPE_LABEL);

    /* set it up */
    dlg->control[nameinf]->move(0,0);
    dlg->control[nameinf]->resize(fontman->getfontheight(fontman->getsysfontno()),width);

    /* create the sprite information label */
    sprinf=dlg->createcontrol(DLGMAN_CONTROL_TYPE_LABEL);

    /* set it up */
    dlg->control[sprinf]->move(0,2*fontman->getfontheight(fontman->getsysfontno()));
    dlg->control[sprinf]->resize(fontman->getfontheight(fontman->getsysfontno()),width);

    /* create the sprite information label */
    animinf=dlg->createcontrol(DLGMAN_CONTROL_TYPE_LABEL);

    /* set it up */
    dlg->control[animinf]->move(0,fontman->getfontheight(fontman->getsysfontno()));
    dlg->control[animinf]->resize(fontman->getfontheight(fontman->getsysfontno()),width);

    /* create the next button */
    next=dlg->createcontrol(DLGMAN_CONTROL_TYPE_BUTTON);

    /* set it up */
    dlg->control[next]->move(fontman->getfontwidth(fontman->getsysfontno()),12*fontman->getfontheight(fontman->getsysfontno())+4);
    dlg->control[next]->resize(fontman->getfontheight(fontman->getsysfontno()),fontman->getextwidth(fontman->getsysfontno(),DEBUG_ANIM_NEXT));
    dlg->control[next]->setcaption(DEBUG_ANIM_NEXT);
    dlg->control[next]->setfocus(1);

    /* create the ok button */
    ok=dlg->createcontrol(DLGMAN_CONTROL_TYPE_BUTTON);

    /* set it up */
    dlg->control[ok]->move((width-(2*DLGMAN_BORDER_WIDTH)-fontman->getextwidth(fontman->getsysfontno(),DEBUG_ANIM_OK))/2,12*fontman->getfontheight(fontman->getsysfontno())+4);
    dlg->control[ok]->resize(fontman->getfontheight(fontman->getsysfontno()),fontman->getextwidth(fontman->getsysfontno(),DEBUG_ANIM_OK));
    dlg->control[ok]->setcaption(DEBUG_ANIM_OK);

    /* create the next button */
    prev=dlg->createcontrol(DLGMAN_CONTROL_TYPE_BUTTON);

    /* set it up */
    dlg->control[prev]->move(width-(fontman->getextwidth(fontman->getsysfontno(),DEBUG_ANIM_PREV))-(2*fontman->getfontwidth(fontman->getsysfontno())),12*fontman->getfontheight(fontman->getsysfontno())+4);
    dlg->control[prev]->resize(fontman->getfontheight(fontman->getsysfontno()),fontman->getextwidth(fontman->getsysfontno(),DEBUG_ANIM_PREV));
    dlg->control[prev]->setcaption(DEBUG_ANIM_PREV);

    /* create the image */
    img=dlg->createcontrol(DLGMAN_CONTROL_TYPE_ANM);
    dlg->control[img]->move(0,3*fontman->getfontheight(fontman->getsysfontno()));
    dlg->control[img]->resize(0,0);
    dlg->control[img]->loadanim(anm->filename);

    /* create the next button */
    selanm=dlg->createcontrol(DLGMAN_CONTROL_TYPE_BUTTON);

    /* set it up */
    dlg->control[selanm]->move(fontman->getfontwidth(fontman->getsysfontno()),11*fontman->getfontheight(fontman->getsysfontno()));
    dlg->control[selanm]->resize(fontman->getfontheight(fontman->getsysfontno()),fontman->getextwidth(fontman->getsysfontno(),DEBUG_ANIM_SELECTANIM));
    dlg->control[selanm]->setcaption(DEBUG_ANIM_SELECTANIM);

    /* reset all counters */
    spriteno=0;animno=GFX_NOANIM;frameno=0;

    do {
        sprintf(tempstr,DEBUG_ANIM_SPRITE,spriteno,anm->header->nofsprites-1);
        dlg->control[nameinf]->setcaption(tempstr);
        sprintf(tempstr,"Size: %ux%u, transparancy (%u, %u, %u)",anm->sprite[spriteno].hdr->width,anm->sprite[spriteno].hdr->heigth,anm->sprite[spriteno].hdr->transcol_r,anm->sprite[spriteno].hdr->transcol_g,anm->sprite[spriteno].hdr->transcol_b);
        dlg->control[sprinf]->setcaption(tempstr);
        dlg->control[img]->setspriteno(spriteno);

        if (animno==GFX_NOANIM) {
            sprintf(tempstr,"Animation: none selected");
        } else {
            sprintf(tempstr,"Animation %s: frame %u of %u",anm->anim[animno].anm.name,frameno,anm->anim[animno].anm.noframes-1);
        }
        dlg->control[animinf]->setcaption(tempstr);

        /* handle the dialog */
        i=dlgman->dodialog(dialogno);

        /* if the user clicked next, go to the next frame */
        if (i==next) {
            /* do we have an animation selected? */
            if (animno!=GFX_NOANIM) {
                /* yup. cycle the animation frame */
                frameno++;

                /* about to exceed the number of frames? */
                if(frameno>=anm->anim[animno].anm.noframes) {
                    /* yup. get back to frame #0 */
                    frameno=0;
                }
                /* get the correct sprite number */
                spriteno=anm->anim[animno].frame[frameno];
            } else {
                /* no. cycle the sprite number */
                spriteno++;
                /* about to exceed number of sprites? */
                if (spriteno>=anm->header->nofsprites) {
                    /* yup. back to number #1 */
                    spriteno=0;
                }
            }
        }

        /* if the user clicked next, go to the next frame */
        if (i==prev) {
            /* do we have an animation selected? */
            if (animno!=GFX_NOANIM) {
                /* need to wrap? */
                if (frameno>0) {
                    /* no. just decrement */
                    frameno--;
                } else {
                    /* yes. use the last frame */
                    frameno=anm->anim[animno].anm.noframes-1;
                }
                /* get the correct sprite number */
                spriteno=anm->anim[animno].frame[frameno];
            } else {
                /* no. cycle the sprite number */
                /* about to exceed number of sprites? */
                if (spriteno>0) {
                    spriteno--;
                } else {
                    /* yup. back to the last one */
                    spriteno=anm->header->nofsprites-1;
                }
            }
        }

        /* if the user clicked on the 'select anim' thing, do it */
        if (i==selanm) {
            /* select the animation */
            animno=debug_selectanm(anm);

            frameno=0;

            /* if we have a valid animation, fix the sprite number */
            if(animno!=GFX_NOANIM) {
                spriteno=anm->anim[animno].frame[frameno];
            }
        }
    } while (i!=ok);

    /* nuke the dialog */
    dlgman->destroydialog(dialogno);
}

void
debug_doanims() {
    _UINT dialogno,height,width,list,i,j,c,view,ok;
    _SINT count;
    DIALOG* dlg;
    char   tempstr[MAD_TEMPSTR_SIZE];
    struct ANM* anm;


    /* set height and width */
    height=12*fontman->getfontheight(fontman->getsysfontno()); width=200;

    /* create a dialog */
    dialogno=dlgman->createdialog(DLGMAN_DIALOG_FLAG_TITLEBAR|DLGMAN_DIALOG_FLAG_MOVEABLE,0,0,height,width);
    dlg=dlgman->dialog[dialogno];

    /* set the title */
    dlg->setitle(DEBUG_ANIM_TITLE);

    /* center it */
    dlg->centerdialog();

    /* create the list */
    list=dlg->createcontrol(DLGMAN_CONTROL_TYPE_LIST);

    /* set it up */
    dlg->control[list]->move(fontman->getfontwidth(fontman->getsysfontno()),fontman->getfontheight(fontman->getsysfontno()));
    dlg->control[list]->resize(9*fontman->getfontheight(fontman->getsysfontno()),width-(4*DLGMAN_BORDER_WIDTH)-fontman->getfontwidth(fontman->getsysfontno()));

    /* create the view button */
    view=dlg->createcontrol(DLGMAN_CONTROL_TYPE_BUTTON);

    /* set it up */
    dlg->control[view]->move(fontman->getfontwidth(fontman->getsysfontno()),10*fontman->getfontheight(fontman->getsysfontno())+4);
    dlg->control[view]->resize(fontman->getfontheight(fontman->getsysfontno()),fontman->getextwidth(fontman->getsysfontno(),DEBUG_ANIM_VIEW));
    dlg->control[view]->setcaption(DEBUG_ANIM_VIEW);

    /* create the ok button */
    ok=dlg->createcontrol(DLGMAN_CONTROL_TYPE_BUTTON);

    /* set it up */
    dlg->control[ok]->move(width-(4*DLGMAN_BORDER_WIDTH)-fontman->getextwidth(fontman->getsysfontno(),DEBUG_ANIM_OK),10*fontman->getfontheight(fontman->getsysfontno())+4);
    dlg->control[ok]->resize(fontman->getfontheight(fontman->getsysfontno()),fontman->getextwidth(fontman->getsysfontno(),DEBUG_ANIM_OK));
    dlg->control[ok]->setcaption(DEBUG_ANIM_OK);

    /* now, scan all animations */
    for(i=0;i<GFX_MAXANIMS;i++) {
        /* retrieve the pointer to the animation */
        anm=gfx->get_anim(i);

        /* is it loaded? */
        if (anm->loaded) {
            /* yup. add the name */
            sprintf(tempstr,"$%02x: %s",i,anm->filename);
            dlg->control[list]->additem(tempstr);
        }
    }

    /* handle the dialog */
    i=dlgman->dodialog(dialogno);

    /* did the user click the 'OK' button? */
    if (i==ok) {
        /* yup. nuke the dialog and just get out of here */
        dlgman->destroydialog(dialogno);
        return;
    }

    /* no. figure out the index */
    j=dlg->control[list]->getcuritem();

    /* nuke the dialog */
    dlgman->destroydialog(dialogno);

    /* retrieve the animation number */
    c=0;
    for(i=0;i<GFX_MAXANIMS;i++) {
        /* retrieve the pointer to the animation */
        anm=gfx->get_anim(i);

        /* is it loaded? */
        if (anm->loaded) {
            /* yup. is this the one? */
            if(c==j) {
                /* yup! */
                break;
            }
            c++;
        }
    }

    /* summon the view animation procedure */
    debug_viewanm(c);
}

void
debug_launchdebugger(SCRIPT* scr) {
    char  *displaydata;
    char  tempstr[MAD_TEMPSTR_SIZE];
    _UCHAR noskip,b;
    _ULONG l,i,j,curip,inspect_obj;
    _ULONG save_breakpoint[DEBUG_MAX_BREAKPOINTS];

    /* wait until the user stops pushing the tilde */
    controls->waitforelease (CONTROLS_KEY_TILDE);

    debug_active=1;
    /* allocate memory for a temponary display buffer */
    if((displaydata=(char*)malloc(GFX_DRIVER_HRES*GFX_DRIVER_VRES*GFX_BPP))==NULL) {
        /* out of memory. die (Error 2: out of memory (allocation of bytes failed) */
        sprintf(tempstr,MAD_ERROR_2,(GFX_DRIVER_HRES*GFX_DRIVER_VRES*GFX_BPP));
        die(tempstr);
    }

    /* get the instruction pointer */
    curip=scr->get_ip();inspect_obj=ego_object;

    /* XXX */ inspect_obj = 0x3e;

    /* keep doing it until tilde is pressed */
    while(!controls->iskeydown(CONTROLS_KEY_TILDE)) {
        /* update script name label */
        /* reset the disassembled instruction color */
        dlgman->dialog[debug_dialogno]->control[debug_textlabel_id]->setcolor(DEBUG_NORMALCOLOR);
        /* figure out whether it's the root script or the secondary script we're debugging */
        sprintf(tempstr,DEBUG_MESSAGE_SCRIPTLABEL,scr->getfilename(),curip>>16,curip&0xffff);
        dlgman->dialog[debug_dialogno]->control[debug_scriptlabel_id]->setcaption(tempstr);

            /* breakpoint here? */
//            if(scr->isbreakpoint(cureip)!=SCRIPT_NOBREAKPOINT) {
                /* yeah, change the color of the text */
//                dlgman->dialog[debug_dialogno]->control[debug_textlabel_id]->setcolor(DEBUG_BREAKPOINTCOLOR);
//            }

        /* is the ip within range? */
        if (curip<scr->get_code_size()){
            /* yup. disassemble it */
            l=scr->disassemble(curip,tempstr);
        } else {
            /* no. show some message */
            strcpy(tempstr,DEBUG_MESSAGE_INVALIDIP);
        }
        debug_setdebugtext(tempstr);

        /* set up the register display */
        sprintf(tempstr,DEBUG_MESSAGE_REGINFO,scr->get_a(),(scr->get_flag()) ? "true" : "false",scr->get_sp(),scr->get_tos_value());
        debug_setregstext(tempstr);

        /* set ego info */
        if(inspect_obj!=OBJMAN_NOBJECT) {
            sprintf(tempstr,DEBUG_MESSAGE_EGOINFO,inspect_obj,objman->getobjectx(inspect_obj),objman->getobjecty(inspect_obj),objman->getobjectdestx(inspect_obj),objman->getobjectdesty(inspect_obj),objman->getprioritycode(inspect_obj),objman->getbasepriority(inspect_obj),objman->isvisible(inspect_obj),objman->objmustmove(inspect_obj));
            debug_setobjinfotext(tempstr);
        } else {
            debug_setobjinfotext(DEBUG_MESSAGE_NOEGOBJECT);
        }

        /* need to step? */
        if(controls->iskeydown(CONTROLS_KEY_S)) {
            /* yeah. do it */
            debug_step=1;
            free(displaydata);
            /* nuke the keystroke */
            controls->waitforelease (CONTROLS_KEY_S);
            return;
        }

        /* need to trace? (step but not through main script) */
        if(controls->iskeydown(CONTROLS_KEY_T)) {
            /* yeah. do it */
            debug_step=2;
            free(displaydata);
            /* nuke the keystroke */
            controls->waitforelease (CONTROLS_KEY_T);
            return;
        }

        /* need to check animations? */
        if(controls->iskeydown(CONTROLS_KEY_N)) {
            /* yeah. do it */
            debug_doanims();

            /* nuke the keystroke */
            controls->waitforelease (CONTROLS_KEY_N);
        }

        /* need to jump and is skippable instruction? */
        if((controls->iskeydown(CONTROLS_KEY_J)&&(!noskip))) {
            /* yeah. do it */
//            scr->seteip(scr->geteip()+l);

            debug_step=1;
            free(displaydata);

            /* nuke the keystroke */
            controls->waitforelease (CONTROLS_KEY_J);
            return;
        }

        /* need to reload the script? */
        if(controls->iskeydown(CONTROLS_KEY_R)) {
            /* yeah. do it */
            /* first save all breakpoints */
            for(i=0;i<DEBUG_MAX_BREAKPOINTS;i++) {
//                save_breakpoint[i]=scr->getbreakpoint(i);
            }

            /* save the script file name and eip */
//            strcpy(tempstr,scr->getfilename());
//            i=scr->geteip();
            /* deinitialize the script and reload it */
//            scr->done();
//            scr->init(tempstr);
            /* and restore the eip */
//            scr->seteip(i);

            /* and restore all breakpoints */
            for(i=0;i<DEBUG_MAX_BREAKPOINTS;i++) {
//                scr->addbreakpoint(save_breakpoint[i]);
            }

            /* nuke the keystroke */
            controls->waitforelease (CONTROLS_KEY_R);
        }

        /* need to list breakspoints? */
        if(controls->iskeydown(CONTROLS_KEY_L)) {
            /* yeah. do it */
            debug_listbreakpoints();

            /* nuke the keystroke */
            controls->waitforelease (CONTROLS_KEY_L);
        }

        /* need to quit? */
        if(controls->iskeydown(CONTROLS_KEY_Q)) {
            /* yeah. do it */
            mad_exit();
            /* NOTREACHED */
        }

        /* need to show background screen? */
        if(controls->iskeydown(CONTROLS_KEY_A)) {
            /* yeah. do it */
            debug_gfxmode=DEBUG_GFXMODE_BACKSCR;

            /* update the screen, but not the animation numbers, positions etc. */
            gfx->redraw(0);

            /* nuke the keystroke */
            controls->clearkey(CONTROLS_KEY_A);
        }

        /* need to show mask screen? */
        if(controls->iskeydown(CONTROLS_KEY_M)) {
            /* yeah. do it */
            debug_gfxmode=DEBUG_GFXMODE_MASKSCR;

            /* update the screen, but not the animation numbers, positions etc. */
            gfx->redraw(0);

            /* nuke the keystroke */
            controls->waitforelease (CONTROLS_KEY_M);
        }

        /* need to show priority screen? */
        if(controls->iskeydown(CONTROLS_KEY_P)) {
            /* yeah. do it */
            debug_gfxmode=DEBUG_GFXMODE_PRIOSCR;

            /* update the screen, but not the animation numbers, positions etc. */
            gfx->redraw(0);

            /* nuke the keystroke */
            controls->waitforelease (CONTROLS_KEY_P);
        }

        /* need to add a breakpoint? */
        if(controls->iskeydown(CONTROLS_KEY_B)) {
            /* yeah. do it */
//            if(scr->addbreakpoint(cureip)==SCRIPT_NOBREAKPOINT) {
                /* no luck. display message  */
                dlgman->messagebox(DEBUG_MESSAGE_OUTOFBREAKPOINTS,DEBUG_MESSAGE_BREAKPOINTITLE);
//            }

            /* nuke the keystroke */
            controls->waitforelease (CONTROLS_KEY_B);
        }

        /* need to zap all breakpoints? */
        if(controls->iskeydown(CONTROLS_KEY_Z)) {
            /* yeah. do it */
            for(i=0;i<DEBUG_MAX_BREAKPOINTS;i++) {
//                scr->clearbreakpoint(i);
            }

            /* nuke the keystroke */
            controls->waitforelease (CONTROLS_KEY_Z);
        }

        /* need to go to next instruction? */
        if((controls->iskeydown(CONTROLS_KEY_EQUAL))||(controls->iskeydown(CONTROLS_KEY_KPADD))) {
            /* yeah. do it */
            if ((curip+l)<scr->get_code_size()) curip+=l;

            /* nuke the keystroke */
            controls->waitforelease (CONTROLS_KEY_EQUAL);
            controls->waitforelease (CONTROLS_KEY_KPADD);
        }

        /* need to go to previous instruction? */
        if((controls->iskeydown(CONTROLS_KEY_MINUS))||(controls->iskeydown(CONTROLS_KEY_KPSUB))) {
            /* yeah. do it (pretty lame-coded, but who cares :-) */
            /*
             * how this works:
             *
             * 1. we set the [j] (phony eip) to zero.
             * 2. we disassemble position [j]. the length of the instruction
             *    will be stored in [i]
             * 3. when [j] (eip) + [i] (length) is equal to the current eip,
             *    [j] will point to the instruction BEFORE [cureip].
             *
             */
            j=0;i=0;
            while(j+i!=curip) {
                j+=i;
                i=scr->disassemble(j,tempstr);

                /* poll the controls in case this loop never ends (emergency
                   exit will still work this way) */                                    
                controls->poll();
            }

            curip=j;
            /* nuke the keystroke */
            controls->waitforelease (CONTROLS_KEY_MINUS);
            controls->waitforelease (CONTROLS_KEY_KPSUB);
        }

        /* need to go to the current instruction? */
        if(controls->iskeydown(CONTROLS_KEY_DOT)) {
            /* yeah. do it */
//            cureip=scr->geteip();
            /* nuke the keystroke */
            controls->waitforelease (CONTROLS_KEY_DOT);
        }

        /* need to delete a breakpoint? */
        if(controls->iskeydown(CONTROLS_KEY_D)) {
            /* yeah. do it */
            /* is there *really* a breakpoint? */
//            b=scr->isbreakpoint(cureip);
            if(b!=SCRIPT_NOBREAKPOINT) {
                /* yeah. let's nuke it! */
//                scr->clearbreakpoint(i);
            }

            /* nuke the keystroke */
            controls->waitforelease (CONTROLS_KEY_D);
        }

        /* need to list objects? */
        if(controls->iskeydown(CONTROLS_KEY_O)) {
            /* yeah. do it */
            debug_listobjects();

            /* nuke the keystroke */
            controls->waitforelease (CONTROLS_KEY_O);
        }

        /* draw/update the dialog */
        dlgman->drawdialog(debug_dialogno,controls->getxpos(),controls->getypos(),displaydata);

        /* poll the controls */
        controls->poll();
    }

    /* wait until the user stops pushing the tilde */
    controls->waitforelease (CONTROLS_KEY_TILDE);

    free(displaydata);
    debug_active=0;
    debug_step=0;
}

void
debug_setdebugtext(char* text) {
   dlgman->dialog[debug_dialogno]->control[debug_textlabel_id]->setcaption(text);
}

void
debug_setregstext(char* text) {
   dlgman->dialog[debug_dialogno]->control[debug_regs_id]->setcaption(text);
}

void
debug_setobjinfotext(char* text) {
   dlgman->dialog[debug_dialogno]->control[debug_objinfo_id]->setcaption(text);
}

void
debug_showpriority() {
    _ULONG i, j;

    j = 0;
    for (i = 0; i < (GFX_DRIVER_HRES * GFX_DRIVER_VRES); i++) {
        gfx->virtualscreen[j++] = gfx->priorityscreen[i];
        gfx->virtualscreen[j++] = gfx->priorityscreen[i];
        gfx->virtualscreen[j++] = gfx->priorityscreen[i];
        gfx->virtualscreen[j++] = 0;
    }

    gfxdriver->draw(gfx->virtualscreen);
    controls->waitforelease (CONTROLS_KEY_P);
}

void
debug_showmask() {
    _ULONG i, j;

    gfxdriver->draw(gfx->virtualscreen);
    controls->waitforelease (CONTROLS_KEY_M);
}

void
debug_freeze() {
    controls->waitforelease (CONTROLS_KEY_F);
}

void
debug_showreachable() {
    _ULONG i, j;

    memcpy(gfx->virtualscreen,gfx->backscreen,(gfxdriver->get_hres()*gfxdriver->get_vres()*4));
    j = 0;
    for (i = 0; i < (GFX_DRIVER_HRES * GFX_DRIVER_VRES); i++) {
        if (gfx->priorityscreen[i]) {
            gfx->virtualscreen[j] ^= 0xff;
            gfx->virtualscreen[j + 1] ^= 0xff;
        }
        j += 4;
    }
    gfxdriver->draw(gfx->virtualscreen);
    controls->waitforelease (CONTROLS_KEY_R);
}

void
debug_teleport() {
    _UINT dialogno,height,width,ok,cancel,roomno,text;
    _ULONG  i;
    DIALOG* dlg;
    char   tempstr[MAD_TEMPSTR_SIZE];
    char*  ptr;

    /* got an ego object? */
    if(ego_object==OBJMAN_NOBJECT) {
        /* no. leave */
        return;
    }

    /* set height and width */
    height=4*fontman->getfontheight(fontman->getsysfontno()); width=200;

    /* create a dialog */
    dialogno=dlgman->createdialog(DLGMAN_DIALOG_FLAG_TITLEBAR|DLGMAN_DIALOG_FLAG_MOVEABLE,0,0,height,width);
    dlg=dlgman->dialog[dialogno];

    /* set the title */
    dlg->setitle(DEBUG_TELEPORT_TITLE);

    /* center it */
    dlg->centerdialog();

    /* create the list */
    /* create the OK button */
    ok=dlg->createcontrol(DLGMAN_CONTROL_TYPE_BUTTON);

    /* set it up */
    dlg->control[ok]->move(fontman->getfontwidth(fontman->getsysfontno()),2*fontman->getfontheight(fontman->getsysfontno())+4);
    dlg->control[ok]->resize(fontman->getfontheight(fontman->getsysfontno()),fontman->getextwidth(fontman->getsysfontno(),DEBUG_ANIM_OK));
    dlg->control[ok]->setcaption(DEBUG_ANIM_OK);

    /* create the ok button */
    cancel=dlg->createcontrol(DLGMAN_CONTROL_TYPE_BUTTON);

    /* set it up */
    dlg->control[cancel]->move(width-(4*DLGMAN_BORDER_WIDTH)-fontman->getextwidth(fontman->getsysfontno(),DEBUG_ANIM_CANCEL),2*fontman->getfontheight(fontman->getsysfontno())+4);
    dlg->control[cancel]->resize(fontman->getfontheight(fontman->getsysfontno()),fontman->getextwidth(fontman->getsysfontno(),DEBUG_ANIM_CANCEL));
    dlg->control[cancel]->setcaption(DEBUG_ANIM_CANCEL);

    /* create the 'room no' label */
    roomno=dlg->createcontrol(DLGMAN_CONTROL_TYPE_LABEL);

    /* set it up */
    dlg->control[roomno]->move(fontman->getfontwidth(fontman->getsysfontno()),4);
    dlg->control[roomno]->resize(fontman->getfontheight(fontman->getsysfontno()),fontman->getextwidth(fontman->getsysfontno(),DEBUG_TELEPORT_ROOMNO));
    dlg->control[roomno]->setcaption(DEBUG_TELEPORT_ROOMNO);

    /* create the 'room no' input field */
    text=dlg->createcontrol(DLGMAN_CONTROL_TYPE_TEXTINPUT);

    /* set it up */
    dlg->control[text]->move(width-(4*DLGMAN_BORDER_WIDTH)-fontman->getextwidth(fontman->getsysfontno(),DEBUG_TELEPORT_DEFROOMNO),4);
    dlg->control[text]->resize(fontman->getfontheight(fontman->getsysfontno()),fontman->getextwidth(fontman->getsysfontno(),DEBUG_TELEPORT_DEFROOMNO));
    sprintf(tempstr,"%u",objman->getroomno(ego_object));
    dlg->control[text]->setcaption(tempstr);
    dlg->control[text]->setfocus(1);

    /* handle the dialog */
    i=dlgman->dodialog(dialogno);

    /* did the user click the 'CANCEL' button? */
    if (i==cancel) {
        /* yup. nuke the dialog and just get out of here */
        dlgman->destroydialog(dialogno);
        return;
    }

    /* get the room number */
    i=strtol(dlg->control[text]->getcaption(),&ptr,0);
    /* did it work? */
    if(ptr==strchr(dlg->control[text]->getcaption(),0)) {
        /* yup. set the new ego location */
        objman->setroomno(ego_object,0);
        objman->setroomno(ego_object,i);
        /* tell the script to enforce a leave */
        script->forceleave();
    } else {
        /* tell the user it's not valid! */
        sprintf(tempstr,DEBUG_TELEPORT_NOTVALID,dlg->control[text]->getcaption());
        dlgman->messagebox(tempstr,DEBUG_TELEPORT_TITLE);
    }

    /* nuke the dialog */
    dlgman->destroydialog(dialogno);
}

void
debug_defrostcontrols() {
//    objman->setenabled(OBJMAN_BAR_OBJNO,1);
//    objman->setenabled(OBJMAN_CONTROLS_OBJNO,1);
}

void
debug_handlekeys() {
    if(controls->iskeydown(CONTROLS_CONSOLE)) {
        /* the tilde (~) key was hit. set flag to launch script debugger */
          debug_step=1;
    }
    /* just return if alt not held down */
    if(!controls->iskeydown(CONTROLS_KEY_ALT)) return;
    /* alt-p shows the priority screen */
    if(controls->iskeydown(CONTROLS_KEY_P)) debug_showpriority();
    /* alt-m shows the mask screen */
    if(controls->iskeydown(CONTROLS_KEY_M)) debug_showmask();
    /* alt-r shows the reachable area's */
    if(controls->iskeydown(CONTROLS_KEY_R)) debug_showreachable();
    /* alt-d will defrost controls */
    if(controls->iskeydown(CONTROLS_KEY_D)) debug_defrostcontrols();
    /* alt-f will defrost controls */
    if(controls->iskeydown(CONTROLS_KEY_F)) debug_freeze();
    /* alt-t will teleport */
    if(controls->iskeydown(CONTROLS_KEY_T)) {
        controls->waitforelease(CONTROLS_KEY_T);
        debug_teleport();
    }
}
#endif
