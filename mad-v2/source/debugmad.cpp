/*
                              DEBUGMAD.CPP

              MAD Debugging Engine - Handles all debugging

                     (c) 1998, 1999 The MAD Crew

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
_UCHAR debug_active=0;
_UCHAR debug_step=0;
_UCHAR debug_gfxmode=DEBUG_GFXMODE_BACKSCR;

#define DEBUG_MESSAGE_BREAKPOINTITLE "Breakpoints"
#define DEBUG_MESSAGE_PROCTITLE "Procedure"
#define DEBUG_MESSAGE_NOBREAKPOINT "<unused breakpoint>"
#define DEBUG_MESSAGE_SCRIPTLABEL "%s (%s)/$%04x%04x"

#define DEBUG_PROCS_PER_DIALOG 8
#define DEBUG_MESSAGE_BUTTON_DELETEBP "Nuke"
#define DEBUG_MESSAGE_OUTOFBREAKPOINTS "Out of breakpoints"
#define DEBUG_MESSAGE_BUTTON_GOPROC "Go"
#define DEBUG_MESSAGE_BUTTON_UP "Up"
#define DEBUG_MESSAGE_BUTTON_DOWN "Down"
#define DEBUG_MESSAGE_OBJECTSTITLE "Objects"
#define DEBUG_MESSAGE_UNUSEDOBJ "<unused object>"
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
    debug_dialogno=dlgman->createdialog(DLGMAN_DIALOG_FLAG_NOMOUSE|DLGMAN_DIALOG_FLAG_NODEFAULT,GFX_DRIVER_HRES-width-1,0,height,width);
    /* create the script name label in it */
    debug_scriptlabel_id=dlgman->dialog[debug_dialogno]->createcontrol(DLGMAN_CONTROL_TYPE_LABEL);
    dlgman->dialog[debug_dialogno]->control[debug_scriptlabel_id]->move(0,0);
    dlgman->dialog[debug_dialogno]->control[debug_scriptlabel_id]->resize(fontman->getfontheight(fontman->getsysfontno())+1,width);
    /* create the script text label in it */
    debug_textlabel_id=dlgman->dialog[debug_dialogno]->createcontrol(DLGMAN_CONTROL_TYPE_LABEL);
    dlgman->dialog[debug_dialogno]->control[debug_textlabel_id]->move(0,fontman->getfontheight(fontman->getsysfontno())+1);
    dlgman->dialog[debug_dialogno]->control[debug_textlabel_id]->resize(fontman->getfontheight(fontman->getsysfontno())+1,width);
}

/*
 * debug_done()
 *
 * This will deinitialize the debugging module.
 *
 */
void
debug_done() {
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
        strcpy(tempstr,DEBUG_MESSAGE_NOBREAKPOINT);
        if(mainscript->mainscriptactive()) {
            /* check the main (root) script */
            if(mainscript->getbreakpoint(i)!=SCRIPT_NOBREAKPOINTADDRESS) {
                /* wow, got a breakpoint here! */
                sprintf(tempstr,"%s/$%04x%04x",mainscript->getfilename(),mainscript->getbreakpoint(i)>>16,mainscript->getbreakpoint(i)&0xffff);
            }
        } else {
            /* check the secondary script */
            if(script->getbreakpoint(i)!=SCRIPT_NOBREAKPOINTADDRESS) {
                /* wow, got a breakpoint here! */
                sprintf(tempstr,"%s/$%04x%04x",script->getfilename(),script->getbreakpoint(i)>>16,script->getbreakpoint(i)&0xffff);
            }
        }
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
             if(mainscript->mainscriptactive()) {
                 /* yeah. nuke the breakpoint in the main (root) script */
                 mainscript->clearbreakpoint(j);
             } else {
                 /* nope. nuke the breakpoint in the secondary script */
                 script->clearbreakpoint(j);
             }
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

void
debug_launchdebugger() {
    char  *displaydata;
    char  tempstr[MAD_TEMPSTR_SIZE];
    SCRIPT *scr;
    _UCHAR noskip,b;
    _ULONG l,i,j,cureip;
    _ULONG save_breakpoint[DEBUG_MAX_BREAKPOINTS];

    /* wait until the user stops pushing the tilde */
    while(controls->iskeydown(CONTROLS_KEY_TILDE)) controls->poll();

    debug_active=1;
    /* allocate memory for a temponary display buffer */
    if((displaydata=(char*)malloc(GFX_DRIVER_HRES*GFX_DRIVER_VRES))==NULL) {
        /* out of memory. die (Error 2: out of memory (allocation of bytes failed) */
        sprintf(tempstr,MAD_ERROR_2,(GFX_DRIVER_HRES*GFX_DRIVER_VRES));
        die(tempstr);
    }

    /* make sure [cureip] will be set */
    cureip=0xffffffff;

    /* keep doing it until tilde is pressed */
    while(!controls->iskeydown(CONTROLS_KEY_TILDE)) {
        /* update script name label */
        /* reset the disassembled instruction color */
        dlgman->dialog[debug_dialogno]->control[debug_textlabel_id]->setcolor(DEBUG_NORMALCOLOR);
        /* figure out whether it's the root script or the secondary script we're debugging */
        if(mainscript->mainscriptactive()) {
            /* root script (main script) */
            scr=mainscript;
        } else {
            /* secondary script */
            scr=script;
        }
        /* is [cureip] not set? */
        if(cureip==0xffffffff) {
            /* nope. set [cureip] to the current eip */
            cureip=scr->geteip();
        }

        sprintf(tempstr,DEBUG_MESSAGE_SCRIPTLABEL,scr->getfilename(),"child",cureip>>16,cureip&0xffff);
        dlgman->dialog[debug_dialogno]->control[debug_scriptlabel_id]->setcaption(tempstr);

        /* if the instruction is within range, disassemble it */
        if(cureip<scr->getscriptsize()) {
            /* breakpoint here? */
            if(scr->isbreakpoint(cureip)!=SCRIPT_NOBREAKPOINT) {
                /* yeah, change the color of the text */
                dlgman->dialog[debug_dialogno]->control[debug_textlabel_id]->setcolor(DEBUG_BREAKPOINTCOLOR);
            }

            l=scr->disassemble(cureip,tempstr,&noskip);
            debug_setdebugtext(tempstr);
        } else {
            debug_setdebugtext("eip is out of range");
        }

        /* need to step? */
        if(controls->iskeydown(CONTROLS_KEY_S)) {
            /* yeah. do it */
            debug_step=1;
            free(displaydata);
            /* nuke the keystroke */
            controls->clearkey(CONTROLS_KEY_S);
            return;
        }

        /* need to trace? (step but not through main script) */
        if(controls->iskeydown(CONTROLS_KEY_T)) {
            /* yeah. do it */
            debug_step=2;
            free(displaydata);
            /* nuke the keystroke */
            controls->clearkey(CONTROLS_KEY_T);
            return;
        }

        /* need to nop it out? (fill the instruction with nops) */
        if(controls->iskeydown(CONTROLS_KEY_N)) {
            /* yeah. do it */
            for(i=cureip;i<(cureip+l);i++) {
                scr->changescriptcode(i,OPCODE_NOP);
            }

            /* nuke the keystroke */
            controls->clearkey(CONTROLS_KEY_N);
        }

        /* need to jump and is skippable instruction? */
        if((controls->iskeydown(CONTROLS_KEY_J)&&(!noskip))) {
            /* yeah. do it */
            scr->seteip(scr->geteip()+l);

            debug_step=1;
            free(displaydata);

            /* nuke the keystroke */
            controls->clearkey(CONTROLS_KEY_J);
            return;
        }

        /* need to reload the script? */
        if(controls->iskeydown(CONTROLS_KEY_R)) {
            /* yeah. do it */
            /* first save all breakpoints */
            for(i=0;i<DEBUG_MAX_BREAKPOINTS;i++) {
                save_breakpoint[i]=scr->getbreakpoint(i);
            }

            /* save the script file name and eip */
            strcpy(tempstr,scr->getfilename());
            i=scr->geteip();
            /* deinitialize the script and reload it */
            scr->done();
            scr->init(tempstr);
            /* and restore the eip */
            scr->seteip(i);

            /* and restore all breakpoints */
            for(i=0;i<DEBUG_MAX_BREAKPOINTS;i++) {
                scr->addbreakpoint(save_breakpoint[i]);
            }

            /* nuke the keystroke */
            controls->clearkey(CONTROLS_KEY_R);
        }

        /* need to list breakspoints? */
        if(controls->iskeydown(CONTROLS_KEY_L)) {
            /* yeah. do it */
            debug_listbreakpoints();

            /* nuke the keystroke */
            controls->clearkey(CONTROLS_KEY_L);
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
            controls->clearkey(CONTROLS_KEY_M);
        }

        /* need to show priority screen? */
        if(controls->iskeydown(CONTROLS_KEY_P)) {
            /* yeah. do it */
            debug_gfxmode=DEBUG_GFXMODE_PRIOSCR;

            /* update the screen, but not the animation numbers, positions etc. */
            gfx->redraw(0);

            /* nuke the keystroke */
            controls->clearkey(CONTROLS_KEY_P);
        }

        /* need to add a breakpoint? */
        if(controls->iskeydown(CONTROLS_KEY_B)) {
            /* yeah. do it */
            if(scr->addbreakpoint(cureip)==SCRIPT_NOBREAKPOINT) {
                /* no luck. display message  */
                dlgman->messagebox(DEBUG_MESSAGE_OUTOFBREAKPOINTS,DEBUG_MESSAGE_BREAKPOINTITLE);
            }

            /* nuke the keystroke */
            controls->clearkey(CONTROLS_KEY_B);
        }

        /* need to zap all breakpoints? */
        if(controls->iskeydown(CONTROLS_KEY_Z)) {
            /* yeah. do it */
            for(i=0;i<DEBUG_MAX_BREAKPOINTS;i++) {
                scr->clearbreakpoint(i);
            }

            /* nuke the keystroke */
            controls->clearkey(CONTROLS_KEY_Z);
        }

        /* need to go to next instruction? */
        if((controls->iskeydown(CONTROLS_KEY_EQUAL))||(controls->iskeydown(CONTROLS_KEY_KPADD))) {
            /* yeah. do it */
            if((cureip+l)<scr->getscriptsize()) {
                cureip=cureip+l;
            }
            /* nuke the keystroke */
            controls->clearkey(CONTROLS_KEY_EQUAL);
            controls->clearkey(CONTROLS_KEY_KPADD);
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
            while(j+i!=cureip) {
                j+=i;
                i=scr->disassemble(j,tempstr,&noskip);

                /* poll the controls in case this loop never ends (emergency
                   exit will still work this way) */
                controls->poll();
            }

            cureip=j;
            /* nuke the keystroke */
            controls->clearkey(CONTROLS_KEY_MINUS);
            controls->clearkey(CONTROLS_KEY_KPSUB);
        }

        /* need to go to the current instruction? */
        if(controls->iskeydown(CONTROLS_KEY_DOT)) {
            /* yeah. do it */
            cureip=scr->geteip();
            /* nuke the keystroke */
            controls->clearkey(CONTROLS_KEY_DOT);
        }

        /* need to delete a breakpoint? */
        if(controls->iskeydown(CONTROLS_KEY_D)) {
            /* yeah. do it */
            /* is there *really* a breakpoint? */
            b=scr->isbreakpoint(cureip);
            if(b!=SCRIPT_NOBREAKPOINT) {
                /* yeah. let's nuke it! */
                scr->clearbreakpoint(i);
            }

            /* nuke the keystroke */
            controls->clearkey(CONTROLS_KEY_D);
        }

        /* need to list objects? */
        if(controls->iskeydown(CONTROLS_KEY_O)) {
            /* yeah. do it */
            debug_listobjects();

            /* nuke the keystroke */
            controls->clearkey(CONTROLS_KEY_O);
        }

        /* draw/update the dialog */
        dlgman->drawdialog(debug_dialogno,controls->getxpos(),controls->getypos(),displaydata);

        /* poll the controls */
        controls->poll();
    }

    /* wait until the user stops pushing the tilde */
    while(controls->iskeydown(CONTROLS_KEY_TILDE)) controls->poll();

    free(displaydata);
    debug_active=0;
    debug_step=0;
}

void
debug_setdebugtext(char* text) {
   dlgman->dialog[debug_dialogno]->control[debug_textlabel_id]->setcaption(text);
}

void
debug_showpriority() {
    gfxdriver->draw(gfx->priorityscreen);
    while(controls->iskeydown(CONTROLS_KEY_P)) controls->poll();
}

void
debug_showmask() {
    gfxdriver->draw(gfx->maskscreen);
    while(controls->iskeydown(CONTROLS_KEY_M)) controls->poll();
}

void
debug_freeze() {
    while(controls->iskeydown(CONTROLS_KEY_F)) controls->poll();
}

void
debug_showreachable() {
    _UINT i;

    memcpy(gfx->virtualscreen,gfx->backscreen,(gfxdriver->get_hres()*gfxdriver->get_vres()));
    for(i=0;i<(gfxdriver->get_hres()*gfxdriver->get_vres());i++) {
        if(gfx->priorityscreen[i]) {
            gfx->virtualscreen[i]=(_UCHAR)0xff;
        }
    }
    gfxdriver->draw(gfx->virtualscreen);
    while(controls->iskeydown(CONTROLS_KEY_R)) controls->poll();
}

void
debug_defrostcontrols() {
    objman->setenabled(OBJMAN_BAR_OBJNO,1);
    objman->setenabled(OBJMAN_CONTROLS_OBJNO,1);
}

void
debug_handlekeys() {
    if(controls->iskeydown(CONTROLS_CONSOLE)) {
        /* the tilde (~) key was hit. set flag to launch script debugger */
          debug_step=1;
          /* and clear the key */
          controls->clearkey(CONTROLS_CONSOLE);
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
}
#endif
