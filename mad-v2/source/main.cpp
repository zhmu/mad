/*
                               MAIN.CPP

              MAD Main code - Handles (de)initalization and lanching

                      (c) 1998, 1999 The MAD Crew

  Description:
  This will handle the actual initialization and deinitialization of all
  MAD modules.

  Todo: Nothing

  Portability notes: Use #ifdef's when code must differ for other platforms.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "archive.h"
#include "bar.h"
#include "controls.h"
#include "ctrldrv.h"
#include "dlgman.h"
#include "debugmad.h"
#include "inv.h"
#include "objman.h"
#include "parser.h"
#include "mad.h"
#include "maderror.h"
#include "fontman.h"
#include "gfx.h"
#include "gfx_drv.h"
#include "snd_drv.h"
#include "project.h"
#include "script.h"
#include "textman.h"

#ifdef WINDOWS
#include <windows.h>
#endif /* WINDOWS */

#ifdef PROFILE_FPS
#include <math.h>
#endif /* PROFILE_FPS */

ARCHIVE *archive;
BAR *bar;
CONTROLS *controls;
GFX *gfx;
GFXDRIVER *gfxdriver;
SCRIPT *mainscript;
SCRIPT *script;
PROJECT *project;
FONTMAN *fontman;
OBJMAN *objman;
DLGMAN *dlgman;
PARSER *parser;
TEXTMAN *textman;
SNDDRIVER *snddriver;
INV *inv;
CTRLDRV *ctrldrv;

_ULONG  mad_flags;
_ULONG main_speed;
_UINT ego_object;

/*
 * show_help()
 *
 * This will show the help about the switches.
 *
 */
void
show_help() {
    printf("Usuage: MAD [...switches...]\n\n");
    printf("Switches can be:\n");
    printf("    -h or -?   This little help\n");
    #ifdef DEBUG_AVAILABLE
    printf("\nDebugging switches:\n");
    printf("    -s         Start stepping immediatly\n");
    printf("\n");
    #endif
    printf("As you might have guessed, the parameters are cAsE sEnSiTiVe\n");
}

/*
 * parseparams(_UINT argc,char *argv[])
 *
 * This will parse the parameters.
 *
 */
void
parseparams(_UINT argc,char *argv[]) {
    _UINT i;

    mad_flags=0;
    for (i=1;i<argc;i++) {
        if ((argv[i][0]=='-')||(argv[i][0]=='/')) {
            switch (argv[i][1]) {
                #ifdef DEBUG_AVAILABLE
                case 's': mad_flags|=MAD_FLAG_STARTSTEP; break;
                #endif
                case 'h':
                case '?': show_help();
                          exit(0xfe);
                          /* NOTREACHED */
                 default: printf("unknown option -- %c\nUse -h for help\n",argv[i][1]);
                          exit(0xfe);
                          /* NOTREACHED */
            }
        } else {
            printf("not an optino -- %s\nUse -h for help\n",argv[i]);
            exit(0xfe);
            /* NOTREACHED */
	}
    }
}

/*
 * startup()
 *
 * This will initialize the interprinter. It will initialize all modules, and
 * spawn the init() procedure of the root script.
 *
 */
void
startup() {
    char tempstr[MAD_TEMPSTR_SIZE];
    _UCHAR i;
    archive=NULL;
    bar=NULL;
    ctrldrv=NULL;
    controls=NULL;
    gfx=NULL;
    gfxdriver=NULL;
    mainscript=NULL;
    project=NULL;
    script=NULL;
    objman=NULL;
    dlgman=NULL;
    parser=NULL;
    textman=NULL;
    snddriver=NULL;
    inv=NULL;
    ego_object=OBJMAN_NOBJECT;

    if (!(gfxdriver=new GFXDRIVER)) {
        /* error 7: cannot create object */
        sprintf(tempstr,MAD_ERROR_7,"gfx driver");
        die(tempstr);
    }
    gfxdriver->init();

    if (!(archive=new ARCHIVE)) {
        /* error 7: cannot create object */
        sprintf(tempstr,MAD_ERROR_7,"archive");
        die(tempstr);
    }
    if ((i=archive->init(MAD_MAINARCHIVEFILE))!=ARCHIVE_OK) {
        /* error 9: archive error when trying to initialize */
        sprintf(tempstr,MAD_ERROR_9,archive->err2msg(i),MAD_MAINARCHIVEFILE);
        die(tempstr);
    }
    if (!(project=new PROJECT)) {
        /* error 7: cannot create object */
        sprintf(tempstr,MAD_ERROR_7,"project");
        die(tempstr);
    }
    if ((i=project->init(MAD_PROJECTFILE))!=PROJECT_OK) {
        /* error 10: cannot initialize, project error */
        sprintf(tempstr,MAD_ERROR_10,project->err2msg(i),MAD_PROJECTFILE);
        die(tempstr);
    }
    strcpy(tempstr,(char*)project->info.textfile);
    strcat(tempstr,MAD_TXFEXTENSION);
    if (!(textman=new TEXTMAN)) {
        /* error 7: cannot create object */
        sprintf(tempstr,MAD_ERROR_7,"text manager");
        die(tempstr);
    }
    textman->init(tempstr);

    if (!(objman=new OBJMAN)) {
        /* error 7: cannot create object */
        sprintf(tempstr,MAD_ERROR_7,"object manager");
        die(tempstr);
    }
    objman->init();
    #ifdef SNDDRIVER_WORKS
    if(!(snddriver=new SNDDRIVER)) {
        /* error 7: cannot create object */
        sprintf(tempstr,MAD_ERROR_7,"sound driver");
        die(tempstr);
    }
    if((i=snddriver->init())!=SNDDRV_ERR_OK) {
        /* this failed. die (error 17: sound driver reported) */
        sprintf(tempstr,MAD_ERROR_17,snddriver->err2msg(i));
        die(tempstr);
    }
    #endif
    if ((!(mainscript=new SCRIPT))||(!(script=new SCRIPT))) {
        /* error 7: cannot create object */
        sprintf(tempstr,MAD_ERROR_7,"script");
        die(tempstr);
    }
    strcpy(tempstr,(char *)project->info.scriptfile);
    strcat(tempstr,MAD_SCREXTENSION);
    mainscript->init(tempstr);
    mainscript->parse();
    if (!(gfx=new GFX)) {
        /* error 7: cannot create object */
        sprintf(tempstr,MAD_ERROR_7,"gfx");
        die(tempstr);
    }
    gfx->init();
    if (!(ctrldrv=new CTRLDRV)) {
        /* error 7: cannot create object */
        sprintf(tempstr,MAD_ERROR_7,"controls driver");
        die(tempstr);
    }
    ctrldrv->init();
    if (!(controls=new CONTROLS)) {
        /* error 7: cannot create object */
        sprintf(tempstr,MAD_ERROR_7,"controls");
        die(tempstr);
    }
    controls->init();
    if (!(bar=new BAR)) {
        /* error 7: cannot create object */
        sprintf(tempstr,MAD_ERROR_7,"bar");
        die(tempstr);
    }
    bar->init();
    if (!(fontman=new FONTMAN)) {
        /* error 7: cannot create object */
        sprintf(tempstr,MAD_ERROR_7,"font manager");
        die(tempstr);
    }
    strcpy(tempstr,(char *)project->info.sysfont);
    strcat(tempstr,MAD_FNTEXTENSION);
    fontman->init(tempstr);
    if (!(dlgman=new DLGMAN)) {
        /* error 7: cannot create object */
        sprintf(tempstr,MAD_ERROR_7,"dialog manager");
        die(tempstr);
    }
    dlgman->init();
    if (!(parser=new PARSER)) {
        /* error 7: cannot create object */
        sprintf(tempstr,MAD_ERROR_7,"parser");
        die(tempstr);
    }
    parser->init();
    if (!(inv=new INV)) {
        /* error 7: cannot create object */
        sprintf(tempstr,MAD_ERROR_7,"inventory manager");
        die(tempstr);
    }
    strcpy(tempstr,(char *)project->info.invfile);
    strcat(tempstr,MAD_INVEXTENSION);
    inv->init(tempstr);
    #ifdef DEBUG_AVAILABLE
    debug_init();
    #endif
}

void
shutdown() {
    #ifdef DEBUG_AVAILABLE
    debug_done();
    #endif
    #ifdef SNDDRIVER_WORKS
    if (snddriver!=NULL) { snddriver->done(); delete snddriver; snddriver=NULL; }
    #endif
    if (inv!=NULL) { inv->done(); delete inv; inv=NULL; }
    if (parser!=NULL) { parser->done(); delete parser; parser=NULL; }
    if (dlgman!=NULL) { dlgman->done(); delete dlgman; dlgman=NULL; }
    if (controls!=NULL) { controls->done(); delete controls; controls=NULL; }
    if (ctrldrv!=NULL) { ctrldrv->done(); delete ctrldrv; ctrldrv=NULL; }
    if (gfxdriver!=NULL) { gfxdriver->done(); delete gfxdriver; gfxdriver=NULL; }
    if (gfx!=NULL) { gfx->done(); delete gfx; gfx=NULL; }
    if (fontman!=NULL) { fontman->done(); delete fontman; fontman=NULL; }
    if (bar!=NULL) { bar->done(); delete bar; bar=NULL; }
    if (textman!=NULL) { textman->done(); delete textman; textman=NULL; }
    if (script!=NULL) { script->done(); delete script; script=NULL; }
    if (mainscript!=NULL) { mainscript->done(); delete mainscript; mainscript=NULL; }
    if (objman!=NULL) { objman->done(); delete objman; objman=NULL; }
    if (project!=NULL) { project->done(); delete project; project=NULL; }
    if (archive!=NULL) { archive->done(); delete archive; archive=NULL; }
}

void
idler() {
    gfxdriver->poll();
    controls->poll();
    ctrldrv->poll();

    #ifdef DEBUG_AVAILABLE
    debug_handlekeys();
    #endif /* DEBUG_AVAILABLE */
}

void
die(char *msg) {
    shutdown();
    if (msg != NULL) {
    #ifdef DOS
        printf(MAD_DOS_ERROR_MSG);
        printf("%s\n",msg);
    #elif WINDOWS
        MessageBox (NULL, msg, MAD_WIN_ERROR_TITLE, MB_ICONERROR);
    #elif UNIX
        printf(MAD_DOS_ERROR_MSG);
        printf("%s\n",msg);
    #endif
    }
    exit(1);
}

void
mad_exit() {
    #ifdef PROFILE_FPS
    double f;

    f=gfx->fps;
    #endif /* PROFILE_FPS */

    shutdown();
    #ifdef DOS
        #ifdef HERO6
            printf("All contents copyright (c) 1999 The Hero6 development team.\n\n");
            printf("The MAD interprinter was done by TUOIC (Rink Springer) and Willem van Engen\n");
            printf("Original Artwork by PHattiE (Joe McCombs)\n");
            printf("Dialogue written by Aftershock (Joseph Yarrow)\n\n");
            printf("Visit the Hero6 web site at http://www.hero6.com\n");
            printf("The MAD Web site is located at http://rink.nu\n");
            printf("\nThis demo/the Hero6 project is NOT affiliated with Sierra.\n");
        #else
            printf("%s - (c) 1998, 1999 The MAD Crew\n",STARTUPTITLE);
        #endif /* HERO6 */
        #ifdef PROFILE_FPS
        printf("\nFrames per second: %u (certainly not 100%% correct)\n",(_UINT)ceil(f));
        #endif /* PROFILE_FPS */
    #endif /* DOS */

    exit(0);
}

#ifdef WINDOWS
int WINAPI WinMain (HINSTANCE hMainInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
#else
int
main(int argc,char* argv[]) {
#endif /* WINDOWS */
    char tempstr[MAD_TEMPSTR_SIZE];

    main_speed=50000;
    #ifdef DEBUG_AVAILABLE
    debug_active=0;
    #endif /* DEBUG_AVAILABLE */
    /* parse the parameters */
    #ifdef DOS
    parseparams(argc,argv);
    #endif /* DOS */
    startup();

    /* build the script name */
    strcpy(tempstr,(char *)project->info.scriptproc);

    #ifdef DEBUG_AVAILABLE
        /* need to step immediatly? */
        if(mad_flags&MAD_FLAG_STARTSTEP) {
            /* yeah, do it */
            debug_step=1;
        }
    #endif /* DEBUG_AVAILABLE */

    /* go! */
    mainscript->goproc(tempstr);
	
    /* exit */
    mad_exit();
    /* NOTREACHED */

    /* this will be never reached, but otherwise the compiler complains */
    return 0;
}
