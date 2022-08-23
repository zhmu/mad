/*
                               MAIN.CPP

              MAD Main code - Handles (de)initalization and lanching

                    (c) 1998, 1999, 2000 The MAD Crew

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
#include "mplayer.h"
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

ARCHIVE* archive;
ARCHIVE* patch_archive;
ARCHIVE* music_archive;
ARCHIVE* speech_archive;
ARCHIVE* lang_archive;
BAR* bar;
CONTROLS* controls;
GFX* gfx;
GFXDRIVER* gfxdriver;
SCRIPT* rootscript;
SCRIPT* script;
PROJECT* project;
FONTMAN* fontman;
OBJMAN* objman;
DLGMAN* dlgman;
PARSER* parser;
TEXTMAN* textman;
SNDDRIVER* snddriver;
INV* inv;
CTRLDRV* ctrldrv;
#ifdef MULTIPLAYER
MPLAYER* mplayer;
#endif

_ULONG  mad_flags;
_ULONG main_speed;
_UINT ego_object;

#ifdef MULTIPLAYER
char*   join_game;
char*   my_name;
#endif

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
    printf("    -h or -?   This little help\n\n");
    #ifdef DEBUG_AVAILABLE
    printf("Debugging switches:\n");
    printf("    -s         Start stepping immediatly\n");
    printf("    -p         Turn off display of sprites (huge speedup)\n");
    printf("\n");
    #endif
    #ifdef MULTIPLAYER
    printf("Multiplayer switches:\n");
    printf("    -m         Multiplayer mode\n");
    printf("    -j<ip>     Join the game at this IP address\n");
    printf("    -n<name>   Specify player name\n");
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
    #ifdef MULTIPLAYER
    join_game=NULL;
    my_name=NULL;
    #endif
    for (i=1;i<argc;i++) {
        if ((argv[i][0]=='-')||(argv[i][0]=='/')) {
            switch (argv[i][1]) {
                #ifdef DEBUG_AVAILABLE
                case 's': mad_flags|=MAD_FLAG_STARTSTEP; break;
                case 'p': mad_flags|=MAD_FLAG_NOSPRITES; break;
                #endif
                #ifdef MULTIPLAYER
                case 'm': mad_flags|=MAD_FLAG_MPLAYER; break;
                case 'j': join_game=(char*)(argv[i]+2); break;
                case 'n': my_name=strdup((char*)(argv[i]+2)); break;
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
    patch_archive=NULL;
    lang_archive=NULL;
    speech_archive=NULL;
    music_archive=NULL;
    bar=NULL;
    ctrldrv=NULL;
    controls=NULL;
    gfx=NULL;
    gfxdriver=NULL;
    rootscript=NULL;
    project=NULL;
    script=NULL;
    objman=NULL;
    dlgman=NULL;
    parser=NULL;
    textman=NULL;
    snddriver=NULL;
    inv=NULL;
    ego_object=OBJMAN_NOBJECT;
    #ifdef MULTIPLAYER
    mplayer=NULL;
    #endif

    /* need to do multiplayer? */
    #ifdef MULTIPLAYER
        if(mad_flags&MAD_FLAG_MPLAYER) {
            /* yup. do it */
            if (!(mplayer=new MPLAYER)) {
                /* error 7: cannot create object */
                sprintf(tempstr,MAD_ERROR_7,"mplayer");
                die(tempstr);
            }
            mplayer->init();
        }
    #endif

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

    /* try to open the lpatch archive */
    if (!(patch_archive=new ARCHIVE)) {
        /* error 7: cannot create object */
        sprintf(tempstr,MAD_ERROR_7,"archive");
        die(tempstr);
    }
    if ((i=patch_archive->init(MAD_PATCHARCHIVEFILE))!=ARCHIVE_OK) {
        /* this failed. kill the class */
        delete patch_archive; patch_archive=NULL;
    }

    /* try to open the language archive */
    if (!(lang_archive=new ARCHIVE)) {
        /* error 7: cannot create object */
        sprintf(tempstr,MAD_ERROR_7,"archive");
        die(tempstr);
    }
    if ((i=lang_archive->init(MAD_LANGARCHIVEFILE))!=ARCHIVE_OK) {
        /* this failed. kill the class */
        delete lang_archive; lang_archive=NULL;
    }

    /* try to open the speech archive */
    if (!(speech_archive=new ARCHIVE)) {
        /* error 7: cannot create object */
        sprintf(tempstr,MAD_ERROR_7,"archive");
        die(tempstr);
    }
    if ((i=speech_archive->init(MAD_SPEECHARCHIVEFILE))!=ARCHIVE_OK) {
        /* this failed. kill the class */
        delete speech_archive; speech_archive=NULL;
    }

    /* try to open the music archive */
    if (!(music_archive=new ARCHIVE)) {
        /* error 7: cannot create object */
        sprintf(tempstr,MAD_ERROR_7,"archive");
        die(tempstr);
    }
    if ((i=music_archive->init(MAD_MUSICARCHIVEFILE))!=ARCHIVE_OK) {
        /* this failed. kill the class */
        delete music_archive; music_archive=NULL;
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
    if(!(rootscript=new SCRIPT)) {
        /* error 7: cannot create object */
        sprintf(tempstr,MAD_ERROR_7,"root script");
        die(tempstr);
    }
    strcpy(tempstr,(char *)project->info.scriptfile);
    strcat(tempstr,MAD_SCREXTENSION);
    rootscript->init(tempstr,1);
    if(!(script=new SCRIPT)) {
        /* error 7: cannot create object */
        sprintf(tempstr,MAD_ERROR_7,"script");
        die(tempstr);
    }
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
    /* need to do multiplayer? */
    #ifdef MULTIPLAYER
        if(mad_flags&MAD_FLAG_MPLAYER) {
            /* yup. do it */
            if(mplayer!=NULL) { mplayer->done(); delete mplayer; mplayer=NULL; }
        }
    #endif
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
    if (rootscript!=NULL) { rootscript->done(); delete rootscript; rootscript=NULL; }
    if (objman!=NULL) { objman->done(); delete objman; objman=NULL; }
    if (project!=NULL) { project->done(); delete project; project=NULL; }
    if (archive!=NULL) { archive->done(); delete archive; archive=NULL; }
    if (patch_archive!=NULL) { patch_archive->done(); delete patch_archive; patch_archive=NULL; }
    if (music_archive!=NULL) { music_archive->done(); delete music_archive; music_archive=NULL; }
    if (speech_archive!=NULL) { speech_archive->done(); delete speech_archive; speech_archive=NULL; }
    if (lang_archive!=NULL) { lang_archive->done(); delete lang_archive; lang_archive=NULL; }
}

void
idler() {
    gfxdriver->poll();
    controls->poll();
    ctrldrv->poll();
    #ifdef MULTIPLAYER
        /* need to do multiplayer? */
        if(mplayer!=NULL) {
            /* yup. do it */
            mplayer->poll();
        }
    #endif
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
    #endif
    #ifdef WINDOWS
        MessageBox (NULL, msg, MAD_WIN_ERROR_TITLE, MB_ICONERROR);
    #endif
    #if UNIX
        printf(MAD_DOS_ERROR_MSG);
        printf("%s\n",msg);
    #endif
    }
    exit(1);
}

void
mad_exit() {
    shutdown();
    #ifdef DOS
        printf("%s - (c) 1998, 1999, 2000 The MAD Crew\n",STARTUPTITLE);
        #ifdef HERO6
            printf("\nHero6 Demo Version 0.1 *ALPHA* - NOT INTENDED FOR RELEASE!\n\n");
            printf("All contents copyright (c) 1999, 2000 The Hero6 development team.\n\n");
            printf("The MAD interprinter is (c) 1998, 1999, 2000 by Rink Springer\n\n");
            printf("Visit the Hero6 web site at http://www.hero6.com\n");
            printf("The MAD Web site is located at http://www.mad-project.cx");
            printf("\nThis demo/the Hero6 project is NOT affiliated with Sierra.\n");
        #else
        #endif /* HERO6 */
    #endif /* DOS */

    exit(0);
}

/*
 * archive_readfile(char* fname,_ULONG* rsize)
 *
 * This will read file [fname] from the archive file and return a pointer to
 * the file's data. It will die if the file couldn't be found in all archive
 * files. It will copy the size to [rsize] if [rsize] is not NULL.
 *
 *
 */
void*
archive_readfile(char* fname,_ULONG* rsize) {
    void* ptr;
    char tempstr[MAD_TEMPSTR_SIZE];
    _ULONG size;
    FILE* f;

    /* first check whether the file exists in the current directory */
    if ((f=fopen(fname,"rb"))!=NULL) {
        /* yeah, it does. lets read that file instead of the archive */
        /* figure out the size */
        fseek(f,0,SEEK_END); size=ftell(f); rewind(f);

        /* allocate memory for the data */
        if ((ptr=malloc(size))==NULL) {
            /* error 2: out of memory */
            sprintf(tempstr,MAD_ERROR_2,size);
            die(tempstr);
        }
        /* read the file to the buffer */
        if (!fread(ptr,size,1,f)) {
            /* this failed. die (error 3: read error) */
            sprintf(tempstr,MAD_ERROR_3,fname);
            die(tempstr);
        }
        /* and close the file */
        fclose (f);

        /* if the user specified a valid pointer to copy the size to, copy it */
        if (rsize!=NULL) *rsize=size;

        /* return the pointer to the file */
        return ptr;
    }

    /* do we have a language archive? */
    if(lang_archive!=NULL) {
        /* yup. try it */
        if((ptr=lang_archive->readfile(fname,rsize))!=NULL) {
            /* it worked! return this pointer */
            return ptr;
        }
    }

    /* do we have a patch archive? */
    if(patch_archive!=NULL) {
        /* yup. try it */
        if((ptr=patch_archive->readfile(fname,rsize))!=NULL) {
            /* it worked! return this pointer */
            return ptr;
        }
    }

    /* do we have a speech archive? */
    if(speech_archive!=NULL) {
        /* yup. try it */
        if((ptr=speech_archive->readfile(fname,rsize))!=NULL) {
            /* it worked! return this pointer */
            return ptr;
        }
    }

    /* do we have a music archive? */
    if(music_archive!=NULL) {
        /* yup. try it */
        if((ptr=music_archive->readfile(fname,rsize))!=NULL) {
            /* it worked! return this pointer */
            return ptr;
        }
    }

    /* does the main archive file have it? */
    if((ptr=archive->readfile(fname,rsize))==NULL) {
        /* no. die */
        /* the file was not found. die (error 4: file '%s' not found in archive) */
        sprintf(tempstr,MAD_ERROR_4,fname);
        die(tempstr);
    }

    /* return this pointer */
    return ptr;
}

int
main(int argc,char* argv[]) {
    main_speed=25000;
    #ifdef DEBUG_AVAILABLE
    debug_active=0;
    #endif /* DEBUG_AVAILABLE */

    /* parse the parameters */
    parseparams(argc,argv);

    /* if we wanted to join, kick the multiplayer flag on too */
    #ifdef MULTIPLAYER
    if (join_game!=NULL) {
        mad_flags|=MAD_FLAG_MPLAYER;
    }
    if (my_name==NULL) {
        my_name=strdup(MPLAYER_DEFAULT_NAME);
    }
    #endif

    startup();

    #ifdef DEBUG_AVAILABLE
        /* need to step immediatly? */
        if(mad_flags&MAD_FLAG_STARTSTEP) {
            /* yeah, do it */
            debug_step=1;
        }
    #endif /* DEBUG_AVAILABLE */

    #ifdef MULTIPLAYER
    if (join_game!=NULL) {
        mplayer->joingame(join_game,NULL);
    }
    #endif

    /* go! */
    rootscript->go();
	
    /* exit */
    mad_exit();
    /* NOTREACHED */

    /* this will be never reached, but otherwise the compiler complains */
    return 0;
}
