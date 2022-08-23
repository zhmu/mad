/*
 *                          The MAD Compiler 4.0
 *
 *                          (c) 1999 The MAD Crew
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compile.h"
#include "mc.h"
#include "../../../source/objman.h"
#include "../../../source/script.h"
#include "../../../source/types.h"

_UCHAR   mc_message_level;

/*
 * usuage()
 *
 * This will print the usuage of the program and exit.
 *
 */
void
usuage() {
    fprintf(stderr,"The MAD Compiler 4.0 - (c) 1999 The MAD Crew\n\n");
    fprintf(stderr,"Usuage: MC [flags] [file.scr]\n\n");
    fprintf(stderr,"Flags:\n");
    fprintf(stderr,"  -? or\n");
    fprintf(stderr,"  -h    This help\n");
    fprintf(stderr,"  -q    Quiet mode, don't print anything (except errors)\n");
    fprintf(stderr,"  -v    Verbose mode, print very much stuff\n");
    fprintf(stderr,"  -b    Show build information\n\n");
    fprintf(stderr,"Please note that all parameters are CaSe SeNsItIvE!\n");
    exit(1);
}

/*
 * showbuildinfo()
 *
 * This will show information about this build of The MAD Compiler.
 *
 */
void
showbuildinfo() {
    fprintf(stdout,"The MAD Compiler version 4.0\n");
    fprintf(stdout,"(c) 1999 The MAD Crew\n");
}

/*
 * parseparams(int argc,char* argv[])
 *
 * This will parse the parameters supplied [argv]. It will parse [argc]
 * parameters. It will exit on any error.
 *
 */
void parseparams(int argc,char* argv[]) {
    int i;

    /* set defaults */
    mc_message_level=MC_MESSAGELEVEL_DEFAULT;

    /* scan all parameters */
    for(i=1;i<argc;i++) {
        /* is the first char a dash (-)? */
        if(argv[i][0]=='-') {
            /* Yes, figure out what the parameter means */
            switch(argv[i][1]) {
                case 'h':
                case '?': /* h or ?: show help */
                          usuage();
                          break;
                case 'q': /* q: set silence mode */
                          mc_message_level=MC_MESSAGELEVEL_SILENT;
                          break;
                case 'v': /* v: set verbose mode */
                          mc_message_level=MC_MESSAGELEVEL_VERBOSE;
                          break;
                case 'b': /* b: show build information */
                          showbuildinfo();
                          break;
                 default: /* this is an invalid parameter. show that */
                          fprintf(stderr,"invalid parameter -- %c\n",argv[i][1]);
                          exit(0xff);
            }
        } else {
            /* last parameter? */
            if(i!=(argc-1)) {
                /* nope. complain about the filename not being the last parameter */
                fprintf(stderr,"script file must be the last parameter\n");
                exit(1);
            }
        }
    }
}

/*
 * main(int argc,char* argv[])
 *
 * This is the main procedure.
 *
 */
int
main(int argc,char* argv[]) {
    _UINT i;

    /* disable buffering */
    setbuf(stdout,NULL);
    setbuf(stderr,NULL);

    /* any parameters? */
    if(argc==1) {
        /* nope. print usuage and leave */
        usuage();
    }
    /* parse the parameters */
    parseparams(argc,argv);

    /* initialize the compiler routines */
    cmp_init();

    /* when we are not silent, show the banner */
    if(mc_message_level!=MC_MESSAGELEVEL_SILENT) {
        fprintf(stdout,"The MAD Compiler 4.0 - (c) 1999 The MAD Crew\n\n");
    }

    /* when we are not in silent mode, print the file name */
    if(mc_message_level!=MC_MESSAGELEVEL_SILENT) {
        fprintf(stdout,"Compiling script file '%s'...",argv[argc-1]);
        /* add a newline if verbose */
        if(mc_message_level==MC_MESSAGELEVEL_VERBOSE) {
            printf("\n");
        }
    }
    /* compile it */
    cmp_compilefile(argv[argc-1]);
    /* when we are not in silent mode, print done thingy */
    if(mc_message_level!=MC_MESSAGELEVEL_SILENT) {
        /* add a space if not verbose */
        if(mc_message_level!=MC_MESSAGELEVEL_VERBOSE) {
            printf(" ");
        }
        fprintf(stdout,"done\n");
    }

    /* make sure all memory is freed */
    cmp_cleanup();
}
