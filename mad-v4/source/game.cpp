/*
                               GAME.CPP

              MAD Game Routines - Handling of load/save games etc.

                   (c) 1998, 1999, 2000 The MAD Crew

  Description:
  This module will take care of saved games.

  Todo: Created checksumming routines, code the inventory and add keyboard
        actions to the conversion code.

  Portability notes: Fully portable, no changes required
*/
#include <stdio.h>
#include <string.h>
#include "controls.h"
#include "dlgman.h"
#include "fontman.h"
#include "game.h"
#include "gfx.h"
#include "objman.h"
#include "mad.h"
#include "project.h"
#include "script.h"
#include "textman.h"
#include "types.h"

#define MAD_GAME_SAVE_DIALOGTITLE   "Save game"
#define MAD_GAME_SAVE_SAVEBUTTON    " Save "
#define MAD_GAME_SAVE_CANCELBUTTON  " Cancel "

#define MAD_GAME_SAVE_ERRORTITLE    "Error"
#define MAD_GAME_FILE_CREATERR      "Unable to create file '%s'"
#define MAD_GAME_FILE_WRITERR       "Unable to write file '%s'"

#define MAD_GAME_LOAD_DIALOGTITLE   "Restore game"
#define MAD_GAME_LOAD_LOADBUTTON    " Restore "
#define MAD_GAME_LOAD_CANCELBUTTON  "  Cancel  "

#define MAD_GAME_LOAD_ERRORTITLE    "Error"
#define MAD_GAME_FILE_OPENERR       "Unable to open file '%s'"
#define MAD_GAME_FILE_READERR       "Unable to read file '%s'"
#define MAD_GAME_FILE_NOSAVEGAME    "This is not a saved game"
#define MAD_GAME_FILE_VERSIONBAD    "This is a saved gamed of a different version"
#define MAD_GAME_FILE_GAMEBAD       "This is a saved game of a different game"

#define MAD_GAME_MAX_CONV_LABELS    8

#define MAD_ERROR_700               "Error 700: Savegame checksum does not match"

/*
 * get_game_title(char* fname,char* title)
 *
 * This will return the saved game title of game [fname] in [title]. It will
 * return non-zero if successful, otherwise zero.
 *
 */
_UCHAR
get_game_title(char *fname,char* title) {
    FILE*  f;
    struct SAVEGAME svg;

    /* try to open the file */
    if((f=fopen(fname,"rb"))==NULL) {
        /* this failed. return error status */
        return 0;
    }
    /* read the header */
    if(!fread(&svg,sizeof(svg),1,f)) {
        /* this failed. return error status */
        return 0;
    }

    /* verify the id code */
    if(svg.id!=MAD_SAVEGAME_ID) {
        /* this is not a mad saved game. return error status */
        return 0;
    }

    /* copy the name */
    strcpy(title,svg.title);

    /* close the file */
    fclose(f);

    /* return ok status */
    return 1;
}

/*
 * load_game()
 *
 * This will pop up the load game dialog. It will load a game if neccesary.
 *
 */
void
load_game() {
    _UINT dialogno,height,width,loadit,list,cancelit,i,nofitems;
    _SINT count;
    DIALOG* dlg;
    char   tempstr[MAD_TEMPSTR_SIZE];
    char   savefile[MAD_SAVEGAME_MAX_SAVED_GAMES][MAD_SAVEGAME_MAX_FILENAME_LEN];

    /* set height and width */
    height=15*fontman->getfontheight(fontman->getsysfontno()); width=200;

    /* create a dialog */
    dialogno=dlgman->createdialog(DLGMAN_DIALOG_FLAG_TITLEBAR|DLGMAN_DIALOG_FLAG_MOVEABLE,0,0,height,width);
    dlg=dlgman->dialog[dialogno];

    /* set the title */
    dlg->setitle(MAD_GAME_LOAD_DIALOGTITLE);

    /* center it */
    dlg->centerdialog();

    /* create the 'load' button */
    loadit=dlg->createcontrol(DLGMAN_CONTROL_TYPE_BUTTON);

    /* set it up */
    dlg->control[loadit]->move(width-(2*DLGMAN_BORDER_WIDTH)-fontman->getextwidth(fontman->getsysfontno(),MAD_GAME_LOAD_CANCELBUTTON),(3*fontman->getfontheight(fontman->getsysfontno())));
    dlg->control[loadit]->resize(fontman->getfontheight(fontman->getsysfontno()),fontman->getextwidth(fontman->getsysfontno(),MAD_GAME_LOAD_CANCELBUTTON));
    dlg->control[loadit]->setcaption(MAD_GAME_LOAD_LOADBUTTON);

    /* create the 'cancel' button */
    cancelit=dlg->createcontrol(DLGMAN_CONTROL_TYPE_BUTTON);

    /* set it up */
    dlg->control[cancelit]->move(width-(2*DLGMAN_BORDER_WIDTH)-fontman->getextwidth(fontman->getsysfontno(),MAD_GAME_LOAD_CANCELBUTTON),(5*fontman->getfontheight(fontman->getsysfontno())));
    dlg->control[cancelit]->resize(fontman->getfontheight(fontman->getsysfontno()),fontman->getextwidth(fontman->getsysfontno(),MAD_GAME_LOAD_CANCELBUTTON));
    dlg->control[cancelit]->setcaption(MAD_GAME_LOAD_CANCELBUTTON);

    /* create the list */
    list=dlg->createcontrol(DLGMAN_CONTROL_TYPE_LIST);

    /* set it up */
    dlg->control[list]->move(fontman->getfontwidth(fontman->getsysfontno()),fontman->getfontheight(fontman->getsysfontno()));
    dlg->control[list]->resize(9*fontman->getfontheight(fontman->getsysfontno()),width-(4*DLGMAN_BORDER_WIDTH)-fontman->getextwidth(fontman->getsysfontno(),MAD_GAME_LOAD_CANCELBUTTON)-fontman->getfontwidth(fontman->getsysfontno()));

    nofitems=0;
    for(count=MAD_SAVEGAME_MAX_SAVED_GAMES;count!=-1;count--) {
        /* construct file name */
        sprintf(savefile[nofitems],"%s.%03u",project->info.projectid,count);
        /* get save game title */
        if(get_game_title(savefile[nofitems],tempstr)) {
            /* it worked! add it to the list! */
            dlg->control[list]->additem(tempstr);

            /* next list item */
            nofitems++;
        }
    }

    /* handle the dialog */
    i=dlgman->dodialog(dialogno);

    /* Need to load? */
    if((i!=cancelit)||(i==DLGMAN_DIALOG_NOHIT)) {
        /* Yeah, do it! */
        do_load(savefile[dlg->control[list]->getcuritem()]);
    }

    /* and nuke the dialog */
    dlgman->destroydialog(dialogno);
}

/*
 * save_game()
 *
 * This will pop up the save game dialog. It will save a game if neccesary.
 *
 */
void
save_game() {
    _UINT dialogno,gamename,height,width,saveit,list,cancelit,i,nofitems;
    _UINT first_free_game;
    _SINT count;
    DIALOG* dlg;
    char   tempstr[MAD_TEMPSTR_SIZE];
    char   savefile[MAD_SAVEGAME_MAX_SAVED_GAMES][MAD_SAVEGAME_MAX_FILENAME_LEN];

    /* set height and width */
    height=15*fontman->getfontheight(fontman->getsysfontno()); width=200;

    /* create a dialog */
    dialogno=dlgman->createdialog(DLGMAN_DIALOG_FLAG_TITLEBAR|DLGMAN_DIALOG_FLAG_MOVEABLE,0,0,height,width);
    dlg=dlgman->dialog[dialogno];

    /* set the title */
    dlg->setitle(MAD_GAME_SAVE_DIALOGTITLE);

    /* center it */
    dlg->centerdialog();

    /* create the text entry control */
    gamename=dlg->createcontrol(DLGMAN_CONTROL_TYPE_TEXTINPUT);

    /* set the control properties */
    dlg->control[gamename]->move(0,0);
    dlg->control[gamename]->resize(fontman->getfontheight(fontman->getsysfontno()),width-(4*DLGMAN_BORDER_WIDTH)-fontman->getextwidth(fontman->getsysfontno(),MAD_GAME_SAVE_CANCELBUTTON));
    dlg->control[gamename]->setfocus(1);

    /* create the 'save' button */
    saveit=dlg->createcontrol(DLGMAN_CONTROL_TYPE_BUTTON);

    /* set it up */
    dlg->control[saveit]->move(width-(2*DLGMAN_BORDER_WIDTH)-fontman->getextwidth(fontman->getsysfontno(),MAD_GAME_SAVE_CANCELBUTTON),(3*fontman->getfontheight(fontman->getsysfontno())));
    dlg->control[saveit]->resize(fontman->getfontheight(fontman->getsysfontno()),fontman->getextwidth(fontman->getsysfontno(),MAD_GAME_SAVE_CANCELBUTTON));
    dlg->control[saveit]->setcaption(MAD_GAME_SAVE_SAVEBUTTON);

    /* create the 'cancel' button */
    cancelit=dlg->createcontrol(DLGMAN_CONTROL_TYPE_BUTTON);

    /* set it up */
    dlg->control[cancelit]->move(width-(2*DLGMAN_BORDER_WIDTH)-fontman->getextwidth(fontman->getsysfontno(),MAD_GAME_SAVE_CANCELBUTTON),(5*fontman->getfontheight(fontman->getsysfontno())));
    dlg->control[cancelit]->resize(fontman->getfontheight(fontman->getsysfontno()),fontman->getextwidth(fontman->getsysfontno(),MAD_GAME_SAVE_CANCELBUTTON));
    dlg->control[cancelit]->setcaption(MAD_GAME_SAVE_CANCELBUTTON);

    /* create the list */
    list=dlg->createcontrol(DLGMAN_CONTROL_TYPE_LIST);

    /* set it up */
    dlg->control[list]->move(0,(3*fontman->getfontheight(fontman->getsysfontno())));
    dlg->control[list]->resize(10*fontman->getfontheight(fontman->getsysfontno()),width-(4*DLGMAN_BORDER_WIDTH)-fontman->getextwidth(fontman->getsysfontno(),MAD_GAME_SAVE_CANCELBUTTON));

    nofitems=0;
    for(count=MAD_SAVEGAME_MAX_SAVED_GAMES;count!=-1;count--) {
        /* construct file name */
        sprintf(savefile[nofitems],"%s.%03u",project->info.projectid,count);
        /* get save game title */
        if(get_game_title(savefile[nofitems],tempstr)) {
            /* it worked! add it to the list! */
            dlg->control[list]->additem(tempstr);

            /* next list item */
            nofitems++;
        } else {
            /* this game is free! */
            first_free_game=count;
        }
    }

    /* set the caption of the edit box to the first game known */
    if(nofitems) {
        dlg->control[list]->getitem(0,tempstr);
        dlg->control[gamename]->setcaption(tempstr);
    } else {
        dlg->control[gamename]->setcaption("");
    }

    /* handle the dialog */
    i=dlgman->dodialog(dialogno);

    /* Need to save? */
    if((i!=cancelit)||(i==DLGMAN_DIALOG_NOHIT)) {
        /* Yeah, do it! */
        /* Was a new entry created? */
        strcpy(tempstr,dlg->control[gamename]->getcaption());
        if((i=dlg->control[list]->finditem(tempstr))!=DLGMAN_CONTROL_LIST_NOENTRY) {
            /* no. recycle this item */
            /* save it */
            do_save(savefile[i],dlg->control[gamename]->getcaption());
        } else {
            /* create a new item */
            /* construct the file name */
            sprintf(tempstr,"%s.%03u",project->info.projectid,first_free_game);
            do_save(tempstr,dlg->control[gamename]->getcaption());
        }
    }

    /* and nuke the dialog */
    dlgman->destroydialog(dialogno);
}

/*
 * do_save(char* fname,char* title)
 *
 * This will actually save the game. The saved game will be put in [fname]
 * and will be called [title].
 *
 */
void
do_save(char* fname,char* title) {
    struct SAVEGAME svg;
    _ULONG i;
    FILE*  f;
    char   tempstr[MAD_TEMPSTR_SIZE];

    /* set up the structure */
    memset(&svg,0,sizeof(svg));
    svg.id=MAD_SAVEGAME_ID;               /* id code */
    svg.version=MAD_SAVEGAME_VERSION;     /* version number */
    for(i=0;i<MAD_SAVEGAME_PRJID_LEN;i++) {
        svg.projectid[i]=project->info.projectid[i];  /* project ID code */
    }
    strcpy(svg.title,title);              /* title */
    svg.checksum=0;                       /* checksum is not known yet */
    svg.nofobjects=OBJMAN_MAXOBJECTS;     /* number of objects stored */
//    strcpy(svg.scriptfile,script->getfilename());
//    svg.rootscript_eip=mainscript->geteip(); /* root script eip */
//    svg.script_eip=script->geteip();         /* secondary script eip */
    strcpy(svg.picturefile,gfx->current_picture); /* current picture */

    /* try to create the file */
    if((f=fopen(fname,"wb"))==NULL) {
        /* this failed. give error */
        sprintf(tempstr,MAD_GAME_FILE_CREATERR,fname);
        dlgman->messagebox(tempstr,MAD_GAME_SAVE_ERRORTITLE);
        return;
    }
    /* try to write the header */
    if(!fwrite(&svg,sizeof(svg),1,f)) {
        /* this failed. give error */
        sprintf(tempstr,MAD_GAME_FILE_WRITERR,fname);
        dlgman->messagebox(tempstr,MAD_GAME_SAVE_ERRORTITLE);
        return;
    }
    /* dump ALL objects */
    for(i=0;i<OBJMAN_MAXOBJECTS;i++) {
        /* try to write the object */
        if(!fwrite(objman->getobjectptr(i),sizeof(OBJECT),1,f)) {
            /* this failed. give error */
            sprintf(tempstr,MAD_GAME_FILE_WRITERR,fname);
            dlgman->messagebox(tempstr,MAD_GAME_SAVE_ERRORTITLE);
            return;
        }
    }
    /* rewind back and edit the file */
    svg.checksum=0x52ad4e4b;
    if(!fwrite(&svg,sizeof(svg),1,f)) {
        /* this failed. give error */
        sprintf(tempstr,MAD_GAME_FILE_WRITERR,fname);
        dlgman->messagebox(tempstr,MAD_GAME_SAVE_ERRORTITLE);
        return;
    }

    /* close it */
    fclose(f);
}

/*
 * do_load(char* fname)
 *
 * This will load saved game file [fname].
 *
 */
void
do_load(char* fname) {
    FILE*  f;
    char   tempstr[MAD_TEMPSTR_SIZE];
    struct SAVEGAME svg;
    _ULONG i;

    /* try to open the file */
    if((f=fopen(fname,"rb"))==NULL) {
        /* this failed. give error */
        sprintf(tempstr,MAD_GAME_FILE_OPENERR,fname);
        dlgman->messagebox(tempstr,MAD_GAME_LOAD_ERRORTITLE);
        return;
    }

    /* read the header */
    if(!fread(&svg,sizeof(svg),1,f)) {
        /* this failed. give error */
        sprintf(tempstr,MAD_GAME_FILE_READERR,fname);
        dlgman->messagebox(tempstr,MAD_GAME_LOAD_ERRORTITLE);
        return;
    }

    /* verify the id code */
    if(svg.id!=MAD_SAVEGAME_ID) {
        /* this is not a mad saved game. give error */
        dlgman->messagebox(tempstr,MAD_GAME_FILE_NOSAVEGAME);
        return;
    }
    /* check version */
    if(svg.version!=MAD_SAVEGAME_VERSION) {
        /* this is a saved game of a wrong game version. give error */
        dlgman->messagebox(tempstr,MAD_GAME_FILE_VERSIONBAD);
        return;
    }
    /* check project id */
    if(strncmp(svg.projectid,project->info.projectid,MAD_SAVEGAME_PRJID_LEN)) {
        /* this is not a saved game of this project. give error */
        dlgman->messagebox(tempstr,MAD_GAME_FILE_GAMEBAD);
        return;
    }
    /* load the secondary script file */
    script->done();
//    script->init(svg.scriptfile);
//    script->seteip(svg.script_eip);

    /* set the eip of the root script */
//    script->seteip(svg.rootscript_eip);

    /* load the picture file */
    gfx->loadscreen(svg.picturefile);

    /* load all objects */
    for(i=0;i<OBJMAN_MAXOBJECTS;i++) {
        /* try to read the object */
        if(!fread(objman->getobjectptr(i),sizeof(OBJECT),1,f)) {
            /* this failed. give error */
            sprintf(tempstr,MAD_GAME_FILE_READERR,fname);
            dlgman->messagebox(tempstr,MAD_GAME_LOAD_ERRORTITLE);
            return;
        }
        /* handle loading of animation data */
        if(objman->getype(i)==OBJMAN_TYPE_ANM) {
            objman->setsourceno(i,gfx->loadanm(objman->getdatafile(i)));
        }
    }

    /* all set. close the file */
    fclose(f);
}

/*
 * do_conversation(_UINT x,_UINT y,_UINT unselcol,_UINT selcol,_ULONG no)
 *
 * This will do a conversation with text number [no]. The dialog will be shown
 * at [x],[y]. Unselected items will be shown using color [unselcol], while
 * selected items will be colored using color [selnol]. It will return the
 * text choice that has been made.
 *
 */
_UINT
do_conversation(_UINT x,_UINT y,_UINT unselcol,_UINT selcol,_ULONG no) {
    _UINT dlgno,height,width,old_dlg_no,nofstrings,i,j,a,save_mousesprno,dx,dy;
    _UCHAR mouse_enabled;
    _UINT label[MAD_GAME_MAX_CONV_LABELS];
    char tempstr[MAD_TEMPSTR_SIZE];
    char tempstr2[MAD_TEMPSTR_SIZE];
    char* ptr;

    /* save the mouse sprite and activate new one */
    save_mousesprno=gfx->get_mouse_spriteno();
//    mouse_enabled=objman->isenabled(OBJMAN_MOUSE_OBJNO);
    mouse_enabled=1;

    gfx->set_mouse_spriteno(controls->sysmouse_sprno);
    old_dlg_no = script->getdialogno();

    /* get the text */
    textman->getstring(no,tempstr,MAD_TEMPSTR_SIZE);

    /* set the height and width values */
    height=fontman->getextheight(fontman->getsysfontno(),tempstr)+fontman->getfontheight(fontman->getsysfontno());
    width=fontman->getextwidth(fontman->getsysfontno(),tempstr);

    /* calculate the number of strings */
    nofstrings=(height/fontman->getfontheight(fontman->getsysfontno()))-1;
    if(nofstrings>MAD_GAME_MAX_CONV_LABELS) nofstrings=MAD_GAME_MAX_CONV_LABELS;

    /* create the dialog */
    dlgno = dlgman->createdialog(0,x,y,height,width+(fontman->getfontwidth(fontman->getsysfontno())*5));
    script->setdialogno (dlgno);

    /* create the labels */
    a=0;
    for(i=0;i<nofstrings;i++) {
        label[i]=dlgman->dialog[dlgno]->createcontrol(DLGMAN_CONTROL_TYPE_LABEL);
        dlgman->dialog[dlgno]->control[label[i]]->move(0,i*fontman->getfontheight(fontman->getsysfontno()));
        dlgman->dialog[dlgno]->control[label[i]]->resize(fontman->getfontheight(fontman->getsysfontno()),width);
        dlgman->dialog[dlgno]->control[label[i]]->setcolor(unselcol);

        j=0;
        while((!((tempstr[a]=='\\')&&(tempstr[a+1]=='n')))&&(a<strlen(tempstr))) {
            tempstr2[j]=tempstr[a];
            a++;j++;
        }
        a+=2;
        tempstr2[j]=0;
        dlgman->dialog[dlgno]->control[label[i]]->setcaption(tempstr2);
    }

    /* save the dialog number of the main script */
//    old_dlg_no=mainscript->dialogno;

    /* and use ours */
//    mainscript->dialogno=dlgno;

    i=0xffff;j=0xffff;
    controls->waitnobutton();
    while(1) {
        /* do all actions */
        dx=controls->getxpos();dy=controls->getypos();

        /* highlight if nessecary */
        if((dx>=x)&&(dx<=(x+width))&&(dy>=y)&&(dy<=(y+height))) {
            /* calculate label number */
            i=(dy-y)/fontman->getfontheight(fontman->getsysfontno());

            /* if within reach, enable it */
            if(i<nofstrings) {
                /* does this string want to be highlighted? */
                ptr=dlgman->dialog[dlgno]->control[label[i]]->getcaption();
                if ((strlen(ptr)<2)||(!((ptr[0]=='\\')&&(ptr[1]=='s')))) {
                    if(j!=0xffff) dlgman->dialog[dlgno]->control[label[j]]->setcolor(unselcol);
                    dlgman->dialog[dlgno]->control[label[i]]->setcolor(selcol);
                    j=i;
                }
            } else {
                /* otherwise disable it */
                if(j!=0xffff) dlgman->dialog[dlgno]->control[label[j]]->setcolor(unselcol);
                i=0xffff;
            }
        } else {
            /* the mouse was not within reach. disable text */
            if(j!=0xffff) dlgman->dialog[dlgno]->control[label[j]]->setcolor(unselcol);
            i=0xffff;
        }
        /* redraw the screen */
//        script->goproc(SCRIPT_PROC_RUN);
//        mainscript->goproc(SCRIPT_PROC_RUN);

        gfx->redraw(1);

        dlgman->dialog[dlgno]->draw(gfx->virtualscreen);

        controls->poll();
        idler();

        if(controls->button1pressed()&&(i<nofstrings)) {
            /* valid choice? */
            ptr=dlgman->dialog[dlgno]->control[label[i]]->getcaption();
            if ((strlen(ptr)<2)||(!((ptr[0]=='\\')&&(ptr[1]=='s')))) {
                /* yup */
                break;
            }
        }
    }
    /* make sure no buttons are pressed */
    controls->waitnobutton();

    /* nuke the dialog */
    dlgman->destroydialog(dlgno);

    /* restore the dialog number of the main script */
    script->setdialogno (old_dlg_no);

    /* restore the mouse sprite */
    gfx->set_mouse_spriteno (save_mousesprno);

    /* return the choice number */
    return i;
}
