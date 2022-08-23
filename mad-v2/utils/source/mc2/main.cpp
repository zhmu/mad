/*
 *                             The MAD Compiler
 *                                Version 2.0
 *
 *                          (c) 1999 The MAD Crew
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gadgets.h"
#include "mc.h"
#include "../../../source/objman.h"
#include "../../../source/script.h"
#include "../../../source/types.h"

#define  START_COMMENT       "/*"
#define  END_COMMENT         "*/"

#define  MC_FUNCTION_RANDOM      "random"

#define  EXCLUDE_PASS1

_UCHAR   verbose;

char     filename[MC_MAX_FILENAME_LEN];

char     ucscript[MC_MAX_SCRIPT_LINES][MC_MAX_SCRIPTLINE_LEN];
_ULONG   ucscriptlines;

struct   MC_OBJECT object[MC_MAX_OBJECTS];
struct   MC_PROC proc[MC_MAX_PROCS];
struct   MC_LABEL label[MC_MAX_LABELS];
struct   MC_FIXLABEL fixlabel[MC_MAX_FIXLABELS];

char     cscript[MC_MAX_CSCRIPT_LEN];
_ULONG   cscriptsize;

_ULONG   nofobjects,nofprocs,noflabels,nofixlabels;

struct COMMANDTAB objectcommandtab[] = { 41, {            /* number of commands */
    "enable",            OPCODE_OBJ_ENABLE,       NULL,               0,
    "disable",           OPCODE_OBJ_DISABLE,      NULL,               0,
    "loadsprite",        OPCODE_OBJ_LOADSPR,      "%q",               0,
    "promptexit",        OPCODE_OBJ_PROMPTEXIT,   NULL,               0,
    "addicon",           OPCODE_OBJ_ADDICON,      "%q %q %q %q %u",   0,
    "setpriority",       OPCODE_OBJ_SETPRI,       "%e",               0,
    "move",              OPCODE_OBJ_MOVE,         "%e %e",            0,
    "sethalt",           OPCODE_OBJ_SETHALT,      "%q",               0,
    "setmove",           OPCODE_OBJ_SETMOVE,      "%q",               0,
    "setspeed",          OPCODE_OBJ_SETSPEED,     "%u %u %u",         0,
    "face",              OPCODE_OBJ_FACE,         "%d",               0,
    "setmoveobj",        OPCODE_OBJ_SETMOVEOBJ,   NULL,               0,
    "settext",           OPCODE_OBJ_SETTEXT,      "%t",               1,
    "setcolor",          OPCODE_OBJ_SETCOLOR,     "%e",               0,
    "show",              OPCODE_OBJ_SHOW,         NULL,               0,
    "hide",              OPCODE_OBJ_HIDE,         NULL,               0,
    "go",                OPCODE_OBJ_GO,           NULL,               0,
    "dispose",           OPCODE_OBJ_DISPOSE,      NULL,               0,
    "load",              OPCODE_OBJ_LOAD,         "%q",               0,
    "stop",              OPCODE_OBJ_STOP,         NULL,               0,
    "setanim",           OPCODE_OBJ_SETANIM,      "%q",               0,
    "setloop",           OPCODE_OBJ_SETLOOP,      "%u",               0,
    "animate",           OPCODE_OBJ_ANIMATE,      NULL,               0,
    "moveto",            OPCODE_OBJ_MOVETO,       "%e %e",            0,
    "unload",            OPCODE_OBJ_UNLOAD,       NULL,               0,
    "destroy",           OPCODE_OBJ_DESTROY,      NULL,               1,
    "setclip",           OPCODE_OBJ_SETCLIP,      "%b",               0,
    "setmask",           OPCODE_OBJ_SETMASK,      "%d",               0,
    "darkenpal",         OPCODE_OBJ_DARKENPAL,    "%i",               0,
    "restorepal",        OPCODE_OBJ_RESTOREPAL,   NULL,               0,
    "fadeout",           OPCODE_OBJ_FADEOUT,      NULL,               0,
    "fadein",            OPCODE_OBJ_FADEIN,       NULL,               0,
    "loadtext",          OPCODE_OBJ_LOADTEXT,     "%e",               0,
    "createtextdialog",  OPCODE_OBJ_CREATETXTDLG, "%u %u",            0,
    "loadbackdrop",      OPCODE_OBJ_LOADBCKDRP,   "%q",               0,
    "exit",              OPCODE_OBJ_EXIT,         NULL,               0,
    "loadborder",        OPCODE_OBJ_LOADBORDER,   "%q %q %q %q",      0,
    "dodialog",          OPCODE_OBJ_DODIALOG,     "%u",               0,
    "teleport",          OPCODE_OBJ_TELEPORT,     "%u",               0,
    "showmessage",       OPCODE_OBJ_SHOWMESSAGE,  "%e",               0,
    "converse",          OPCODE_OBJ_CONVERSE,     "%u %u %u %u %e",   0 } };

struct COMMANDTAB commandtab[] = { 7, {
    "loadscreen",  OPCODE_LOADSCREEN,     "%q",               0,
    "goto",        OPCODE_JUMP,           "%l",               0,
    "refresh",     OPCODE_REFRESH,        NULL,               0,
    "return",      OPCODE_RETURN,         NULL,               0,
    "delay",       OPCODE_DELAY,          "%u",               1,
    "leave",       OPCODE_LEAVE,          NULL,               0,
    "call",        OPCODE_CALL,           "%s",               0 } };

/*
 * command.param is a string which describes the parameters of the string.
 * current values allowed are:
 *
 * %s  = string
 * %u  = unsigned integer (16 bit unsigned number)
 * %i  = signed integer (16 bit signed number)
 * %e  = evaluated unsigned integer. things like ego.<something> etc.
 * %q  = quoted string (string beteween "'s)
 * %d  = direction (north, south etc.)
 * %b  = boolean (yes, no etc.)
 * %t  = text block (the quick brown for jumps over the lazy dog etc.)
 * %l  = label
 * %w  = unsigned double word (32 bit unsigned number)
 *
 */

/*
 * showbuildinfo()
 *
 * This will show information about the current build
 *
 */
void
showbuildinfo() {
    printf("The MAD Compiler version 2.0\n");
    printf("(c) 1999 The MAD Crew\n");
}

/*
 * showhelp()
 *
 * This will show help about the switches
 *
 */
void
showhelp() {
    printf("Usuage: MC [switches] ...file...\n\n");
    printf("Switches:\n");
    printf("    -? or -h    This help\n");
    printf("    -b          Show build information\n");
    printf("    -v          Verbose Mode\n");
}

/*
 * parseparms(_UINT argc,char* argv[])
 *
 * This will parse the parameters.
 *
 */
void
parseparms(_UINT argc,char* argv[]) {
    _UINT i;

    verbose=0;
    for (i=1;i<argc;i++) {
        /* does it begin with - or / (switch)? */
        if ((argv[i][0]=='-')||(argv[i][0]=='/')) {
            /* yeah, what is the second char? */
            switch (argv[i][1]) {
                case 'b': /* version information */
                          showbuildinfo();
                          exit(0);
                          /* NOTREACHED */
                case 'v': verbose=1;
                          break;
                case '?': /* help */
                case 'h': /* help */
                          showhelp();
                          exit(1);
                          /* NOTREACHED */
                 default: /* unknown char */
                          printf("unknown parameter -- %c\n",argv[i][1]);
                          printf("use -h or -? for help\n");
                          exit(0xff);
                          /* NOTREACHED */
            }
        }
    }
    /* is there a filename? */
    if (argc<2) {
        /* no, show the help */
        showhelp();
        exit(1);
    }
    /* the last parameter must be the filename */
    strcpy(filename,argv[argc-1]);
}

/*
 * loadscript(char* fname)
 *
 * This will load uncompiled script [fname].
 *
 */
void
loadscript(char* fname) {
    FILE *f;
    _ULONG length,i;

    /* open the file */
    if((f=fopen(fname,"rb"))==NULL) {
        /* this failed. print message and die */
        printf("unable to open input file '%s'\n",fname);
        exit(2);
    }
    /* set line counter to zero */
    ucscriptlines=0;

    /* keep reading lines */
    while(fgets(ucscript[ucscriptlines],MC_MAX_SCRIPTLINE_LEN,f)) {
         i=strlen(ucscript[ucscriptlines]);
         /* do we find a newline? */
         if((i>2)&&(ucscript[ucscriptlines][i-2]=='\r')) {
              /* yeah, nuke it */
              ucscript[ucscriptlines][i-2]=0;
         }

         /* increment the line counter */
         ucscriptlines++;

          /* do we exceed the maximum number of lines supported? */
         if(ucscriptlines>=MC_MAX_SCRIPT_LINES) {
              /* the script has too many lines. die */
              printf("script is too large! this version only supports %lu lines\n",MC_MAX_SCRIPT_LINES);
              exit(2);
         }
    }

    /* neatly close the file */
    fclose(f);
}

/*
 * findobject(char* objname)
 *
 * This will try to find object [objname]. If it is found, it will return
 * the object number, otherwise MC_NOBJECT will be returned.
 *
 */
_ULONG
findobject(char* objname) {
    _ULONG i;

    /* scan all objects */
    for(i=0;i<nofobjects;i++) {
        /* is this the object we are looking for? */
        if(!strcmp(object[i].name,objname)) {
             /* yeah, return its number */
             return i;
        }
    }
    /* no objects were found. return MC_NOBJECT */
    return MC_NOBJECT;
}

/*
 * findproc(char* procname)
 *
 * This will try to find procedure [procname]. If it is found, it will return
 * the procedure number, otherwise MC_NOPROC will be returned.
 *
 */
_ULONG
findproc(char* procname) {
    _ULONG i;

    /* scan all procedures */
    for(i=0;i<nofprocs;i++) {
        /* is this the procedure we are looking for? */
        if(!strcmp(proc[i].name,procname)) {
             /* yeah, return its number */
             return i;
        }
    }
    /* no procedures were found. return MC_NOPROC */
    return MC_NOPROC;
}

/*
 * findlabel(char* labelname)
 *
 * This will try to find label [labelname]. If it is found, it will return
 * the label number, otherwise MC_NOLABEL will be returned.
 *
 */
_ULONG
findlabel(char* labelname) {
    _ULONG i;

    /* scan all objects */
    for(i=0;i<noflabels;i++) {
        /* is this the label we are looking for? */
        if(!strcmp(label[i].name,labelname)) {
             /* yeah, return its number */
             return i;
        }
    }
    /* no objects were found. return MC_NOLABEL */
    return MC_NOLABEL;
}

/*
 * addfixlabel(char* labelname,_ULONG offset)
 *
 * This will add offset [offset] as a fix offset for label [labelname].
 *
 */
void
addfixlabel(char* labelname,_ULONG offset) {
    _ULONG i;

    /* find out the label id */
    if((i=findlabel(labelname))==MC_NOLABEL) {
        /* this label doesn't exists, so die */
        printf("addfixlabel(): cannot add fix offset for label '%s', label never declared\n",labelname);
        exit(0xfe);
    }
    /* do we still have space for the label? */
    if(nofixlabels>=MC_MAX_FIXLABELS) {
        /* they are all used up, so die */
        printf("addfixlabel(): cannot add fix offset for label '%s', all entries are used\n",labelname);
        exit(0xfe);
    }

    fixlabel[nofixlabels].labelno=i;
    fixlabel[nofixlabels].offset=offset;

    nofixlabels++;
}

/*
 * addobject(char* objname,_UCHAR type,_ULONG flags)
 *
 * This will add object [objname] to the list. It will be given type [type]
 * and flags [flags]. The source datafile is excepted to be in [sourcefile].
 *
 */
void
addobject(char* objname,_UCHAR type,_ULONG flags,char* sourcefile) {
    char tempfile[MC_MAX_FILENAME_LEN];

    /* are all objects used up? */
    if(nofobjects>=MC_MAX_OBJECTS) {
        /* yeah, bomb out */
        printf("addobject(): cannot add object '%s', all object entries are used\n",objname);
        exit(0xfe);
    }

    /* add the object */
    object[nofobjects].type=type;
    object[nofobjects].flags=flags;
    /* check length of object name */
    if(strlen(objname)>=MC_MAX_OBJECT_LEN) {
        /* it is too long. bomb out */
        printf("addobject(): cannot add object '%s', object name cannot exceed %lu chars\n",objname,MC_MAX_OBJECT_LEN);
        exit(0xfe);
    }
    strcpy(object[nofobjects].name,objname);

    /* valid source file? */
    if(sourcefile!=NULL) {
        /* check length of source file */
        if(strlen(sourcefile)>=MC_MAX_FILENAME_LEN) {
            /* it is too long. bomb out */
            printf("addobject(): cannot add object '%s', source file name cannot exceed %lu chars\n",objname,MC_MAX_FILENAME_LEN);
            exit(0xfe);
        }
        /* make sure [sourcefile] has no quotes */
        stripquotes(sourcefile,tempfile);
        /* and add it */
        strcpy(object[nofobjects].sourcefile,tempfile);
    }
    /* and increment the object counter */
    nofobjects++;
}

/*
 * addproc(char* procname,_ULONG startline)
 *
 * This will add procedure [procname] to the list. The starting line will be
 * set to [startline].
 *
 */
void
addproc(char* procname,_ULONG startline) {
    /* are all procedures used up? */
    if(nofprocs>=MC_MAX_PROCS) {
        /* yeah, bomb out */
        printf("addobject(): cannot add procedure '%s', all procedure entries are used\n",procname);
        exit(0xfe);
    }

    /* check whether the name is not too long */
    if(strlen(procname)>=MC_MAX_PROCNAME_LEN) {
        /* it is too long. die */
        printf("addproc(): cannot add procedure '%s', procedure name cannot exceed %lu chars\n",procname,MC_MAX_PROCNAME_LEN);
        exit(0xfe);
    }
    /* fill it in */
    strcpy(proc[nofprocs].name,procname);
    proc[nofprocs].startline=startline;

    /* and increment the procedure counter */
    nofprocs++;
}

/*
 * addlabel(char* labelname,_ULONG line)
 *
 * This will add label [labelname] to the list. The line of it will be set to
 * [line].
 *
 */
void
addlabel(char* labelname,_ULONG line) {
    /* are all labels used up? */
    if(noflabels>=MC_MAX_LABELS) {
        /* yeah, bomb out */
        printf("addlabel(): cannot add label '%s', all label entries are used\n",labelname);
        exit(0xfe);
    }

    /* check whether the name is not too long */
    if(strlen(labelname)>=MC_MAX_LABELNAME_LEN) {
        /* it is too long. die */
        printf("addlabel(): cannot add label '%s', label name cannot exceed %lu chars\n",labelname,MC_MAX_LABELNAME_LEN);
        exit(0xfe);
    }
    /* fill it in */
    strcpy(label[noflabels].name,labelname);
    label[noflabels].line=line;

    /* and increment the label counter */
    noflabels++;
}

/*
 * addobjectchange(char* objname,_ULONG offset)
 *
 * This will add offset [offset] as a change offset for object [objname].
 *
 */
void
addobjectchange(char* objname,_ULONG offset) {
    _ULONG i;

    /* find out the object id */
    if((i=findobject(objname))==MC_NOBJECT) {
        /* this object doesn't exists, so die */
        printf("addobjectchange(): cannot add change offset for object '%s', object never declared\n",objname);
        exit(0xfe);
    }
    /* do we still have space for the object? */
    if(object[i].nofchanges>=MC_MAX_CHANGES) {
        /* they are all used up, so die */
        printf("addobjectchange(): cannot add change offset for object '%s', all entries are used\n",objname);
        exit(0xfe);
    }
    object[i].change[object[i].nofchanges]=offset;
    object[i].nofchanges++;
}

/*
 * parsescript()
 *
 * This will parse the script. It will remove all comments etc.
 *
 */
void
parsescript() {
    _ULONG i,j,v,x,z;
    char s[MC_MAX_SCRIPTLINE_PAIRS][MC_MAX_SCRIPTLINE_LEN];
    char* ptr;

    nofprocs=0;cscriptsize=0;noflabels=0;nofixlabels=0;
    /* pass 1: nuke all comments */
    if(verbose) {
        printf("Pass 1: Removing all comments...");
        fflush(stdout);
    }
    #ifndef EXCLUDE_PASS1
    for(i=0;i<ucscriptlines;i++) {
        /* scan for a START_COMMENT */
        j=scanstring(ucscript[i],START_COMMENT);
        if(j!=GADGETS_NONUM) {
             /* we found a comment start! */
             /* now scan for a comment end */
             for(v=j;v<ucscriptlines;v++) {
                 x=scanstring(ucscript[v],END_COMMENT);
                 if(x!=GADGETS_NONUM) {
                     /* yeah! we found it! */
                     /* now nuke everything after the comment in the first
                        line */
                     for(z=j;z<strlen(ucscript[i]);z++) {
                         ucscript[i][z]=0;
                     }
                     /* and nuke everything before the comment in the last
                        line */
                     for(z=0;z<(x+strlen(END_COMMENT));z++) {
                         ucscript[v][z]=0;
                     }
                     /* and nuke every line between the comment start and end */
                     for(z=(j+1);z<=(v-1);z++) {
                         ucscript[z][0]=0;
                     }
                 }
             }
        }
    }
    #endif
    if(verbose) {
        #ifndef EXCLUDE_PASS1
        printf(" done\n");
        #else
        printf(" EXCLUDED\n");
        #endif
        fflush(stdout);
    }
    /* pass 2: handle all objects, variables and stuff */
    if(verbose) {
        printf("Pass 2: Handling objects, variables and labels...");
        fflush(stdout);
    }
    for(i=0;i<ucscriptlines;i++) {
        /* clear all s[] entries */
        memset(s,0,MC_MAX_SCRIPTLINE_LEN*MC_MAX_SCRIPTLINE_PAIRS);
        /* split line in space seperated variables */
        sscanf(ucscript[i],"%s %s %s %s %s %s %s %s",&s[0],&s[1],&s[2],&s[3],&s[4],&s[5],&s[6],&s[7]);
        /* is there a MC_DEFINE_VAR here? */
        if(!strcmp(s[0],MC_DEFINE_VAR)) {
             /* yeah, the user wants to define a variable */
             /* does it already exists? */
             if(findobject(s[1])!=MC_NOBJECT) {
                 /* yeah, return error */
                 printf("%s[%lu]: object '%s' already declared somewhere else\n",filename,i+1,s[1]);
                 exit(3);
             }
             /* add the object to the list */
             addobject(s[1],OBJMAN_TYPE_VARIABLE,MC_FLAGS_DEFAULT,NULL);
        } else {
             /* is there a MC_DEFINE_OBJ here? */
             if(!strcmp(s[0],MC_DEFINE_OBJ)) {
                 /* yeah, check whether the syntax is ok (object <xxx> = <xxx> */
                 if(strcmp(s[2],"=")) {
                     /* no, it's not ok. die */
                     printf("%s[%lu]: '=' expected\n",filename,i+1);
                     exit(3);
                 }
                 /* syntax is ok. figure out the object type */
                 if(!strcmp(s[3],MC_TYPE_ANM)) {
                     /* it's an animation! add it to the lsit */
                     addobject(s[1],OBJMAN_TYPE_ANM,MC_FLAGS_DEFAULT,s[4]);
                 } else {
                     /* well, maybe a text object? */
                     if(!strcmp(s[3],MC_TYPE_TEXT)) {
                          /* yeah, add it */
                          addobject(s[1],OBJMAN_TYPE_TEXT,MC_FLAGS_DEFAULT,s[4]);
                     } else {
                          /* we don't know this type. die */
                          printf("%s[%lu]: unknown object type '%s'\n",filename,i+1,s[3]);
                          exit(3);
                     }
                 } /* is animation */
             } else { /* end of is define object */
                 /* is there a MC_START_PROC here? */
                 if(!strcmp(s[1],MC_START_PROC)) {
                     /* yeah. does this procedure already exists? */
                     if((j=findproc(s[0]))!=MC_NOPROC) {
                         /* yeah, return error */
                         printf("%s[%lu]: procedure '%s' already declared in line %lu\n",filename,i+1,s[0],proc[j].startline+1);
                         exit(3);
                     }
                     /* add the procedure to the list */
                     addproc(s[0],i);
                 } else {
                     /* maybe a MC_END_PROC here? */
                     if((!strcmp(s[0],MC_END_PROC))&&(s[1][0]!='\0')) {
                         /* yeah, check whether the procedure has been declared */
                         if((j=findproc(s[1]))==MC_NOPROC) {
                             /* this procedure was never declared. die */
                             printf("%s[%lu]: procedure '%s' never declared\n",filename,i+1,s[1]);
                             exit(3);
                         }
                         /* set the end procedure to this line */
                         proc[j].endline=i;
                     } else {
                         /* maybe a label? */
                         if ((ptr=strchr(s[0],':'))!=NULL) {
                             /* it's a label, we found a colon. copy the name to s[1] */
                             for (j=0;j<(ptr-s[0]);j++) {
                                 s[1][j]=s[0][j];
                                 s[1][j+1]=0;
                             }
                             /* check if this label was already declared elsewhere */
                             if((j=findlabel(s[1]))!=MC_NOLABEL) {
                                 /* it was already declared somewhere else. die */
                                 printf("%s[%lu]: label '%s' already declared in line %lu\n",filename,i+1,s[0],label[j].line+1);
                                 exit(3);
                             }
                             /* add it to the list */
                             addlabel(s[1],i);
                         } else {
                             /* is there an external variable here? */
                             if(!strcmp(s[0],MC_DEFINE_EXTERN)) {
                                 /* yeah. figure out the object type */
                                if(!strcmp(s[1],MC_TYPE_ANM)) {
                                    /* it's an animation! add it to the lsit */
                                    addobject(s[2],OBJMAN_TYPE_ANM,MC_FLAG_EXTERNAL,NULL);
                                } else {
                                    /* well, maybe a text object? */
                                    if(!strcmp(s[1],MC_TYPE_TEXT)) {
                                        /* yeah, add it */
                                        addobject(s[2],OBJMAN_TYPE_TEXT,MC_FLAG_EXTERNAL,NULL);
                                    } else {
                                        /* well, maybe a variable object? */
                                        if(!strcmp(s[1],MC_TYPE_VAR)) {
                                            /* yeah, add it */
                                            addobject(s[2],OBJMAN_TYPE_VARIABLE,MC_FLAG_EXTERNAL,NULL);
                                        } else {
                                            /* we don't know this type. die */
                                            printf("%s[%lu]: unknown external object type '%s'\n",filename,i+1,s[1]);
                                            exit(3);
                                        } /* is variable */
                                    } /* is text */
                                } /* is animation */
                             } else { /* is external */
                                 /*
                                  * ...add new parser things here...
                                  *
                                  */
                             }
                         }
                     } /* is a label */
                 } /* is a start proc */
             } /* is a label */
        } /* is define variable */
    } /* for loop */
    if(verbose) {
        printf(" done\n");
        fflush(stdout);
    }
}

/*
 * initobjects()
 *
 * This will add the default system objects to the object list.
 *
 */
void
initobjects() {
    _UINT i;

    for(i=0;i<MC_MAX_OBJECTS;i++) {
        object[i].nofchanges=0;
    }

    nofobjects=0;

    addobject(MC_MOUSE_OBJNAME,OBJMAN_TYPE_ANM,MC_FLAG_INTERNAL,NULL);
    addobject(MC_BAR_OBJNAME,OBJMAN_TYPE_ANM,MC_FLAG_INTERNAL,NULL);
    addobject(MC_CONTROLS_OBJNAME,OBJMAN_TYPE_ANM,MC_FLAG_INTERNAL,NULL);
    addobject(MC_SYSTEM_OBJNAME,OBJMAN_TYPE_ANM,MC_FLAG_INTERNAL,NULL);
    addobject(MC_SCRIPT_OBJNAME,OBJMAN_TYPE_ANM,MC_FLAG_INTERNAL,NULL);
    addobject(MC_PARSER_OBJNAME,OBJMAN_TYPE_ANM,MC_FLAG_INTERNAL,NULL);
    addobject(MC_DIALOG_OBJNAME,OBJMAN_TYPE_ANM,MC_FLAG_INTERNAL,NULL);
}

/*
 * cscript_addbyte(_UCHAR i)
 *
 * This will add byte [i] to the compiled script.
 *
 */
void
cscript_addbyte(_UCHAR i) {
    cscript[cscriptsize]=i;

    cscriptsize++;
    if(cscriptsize>=MC_MAX_CSCRIPT_LEN) {
        printf("compiled script is too large, it may not exceed %lu bytes.\n",MC_MAX_CSCRIPT_LEN);
        exit(0xfe);
    }
}

/*
 * cscript_addword(_UINT i)
 *
 * This will add word [i] to the compiled script.
 *
 */
void
cscript_addword(_UINT i) {
    /* add lo byte */
    cscript_addbyte(i&0xff);
    /* add hi byte */
    cscript_addbyte(i>>8);
}

/*
 * cscript_addlong(_ULONG i)
 *
 * This will add long [i] to the compiled script.
 *
 */
void
cscript_addlong(_ULONG i) {
    /* add lo word */
    cscript_addword(i&0xffff);
    /* add hi word */
    cscript_addword(i>>16);
}

/*
 * cscript_addstring(char* string)
 *
 * This will add string [string] to the script.
 *
 */
void
cscript_addstring(char* string) {
    _UINT i;

    /* add the string length (word) */
    cscript_addword(strlen(string));
    /* and add each byte */
    for(i=0;i<strlen(string);i++) {
        cscript_addbyte(string[i]);
    }
}

/* forward declaration */
void evaluateparam(char* param,_ULONG lineno);

/*
 * do_eval_expr(_UCHAR exprno,char* param,_ULONG lineno)
 *
 * This will add an evaluated expression (for example the - bar in foo = foo -
 * bar) to the script. [lineno] should be the current line.
 *
 */
void
do_eval_expr(_UCHAR exprno,char* param,_ULONG lineno) {
    /* add the expression number */
    cscript_addbyte(exprno);

    /* if there is an actual expression, run it through evaluateparam() */
    if(exprno!=EXPR_NONE) {
        evaluateparam(param,lineno);
        return;
    }
}

/*
 * evaluateparam(char* param,_ULONG lineno)
 *
 * This will evaluate parameter [param]. It will add the results directly to
 * the script. [lineno] should be the current line.
 *
 */
void
evaluateparam(char* param,_ULONG lineno) {
    char script_object[MC_MAX_SCRIPTLINE_LEN];
    char script_method[MC_MAX_SCRIPTLINE_LEN];
    char script_expr[MC_MAX_SCRIPTLINE_LEN];
    char* ptr;
    char* ptr2;
    _UINT i;
    _ULONG no;
    _UCHAR type,expr;

    /* there's no expression */
    expr=EXPR_NONE;

    /* scan if there's a addition sign in it */
    if((ptr=strchr(param,'+'))!=NULL) {
        /* there is! dump it in [script_expr] */
        for(i=0;i<(strlen(param)-(ptr-param));i++) {
            script_expr[i]=param[(ptr-param)+i+1];
            script_expr[i+1]=0;
        }
        /* rip it off the parameter */
        param[(ptr-param)]=0;

        /* there's an expression! */
        expr=EXPR_ADD;
    }

    /* scan if there's a subtraction sign in it */
    if((ptr=strchr(param,'-'))!=NULL) {
        /* there is! dump it in [script_expr] */
        for(i=0;i<(strlen(param)-(ptr-param));i++) {
            script_expr[i]=param[(ptr-param)+i+1];
            script_expr[i+1]=0;
        }
        /* rip it off the parameter */
        param[(ptr-param)]=0;

        /* there's an expression! */
        expr=EXPR_SUB;
    }

    /* scan for a dot */
    if((ptr=strchr(param,'.'))!=NULL) {
        /* this is probably an object/method pair. chop it */
        for (i=0;i<ptr-param;i++) {
            script_object[i]=param[i];
            script_object[i+1]=0;
        }
        /* [script_method] is the method after the dot */
        for (i=0;i<(strlen(param)-(ptr-param))-1;i++) {
            script_method[i]=param[i+1+ptr-param];
            script_method[i+1]=0;
        }
        /* figure out the object id */
        if((no=findobject(script_object))==MC_NOBJECT) {
            /* we couldn't find the object. die */
            printf("%s[%lu]: unknown object '%s'\n",filename,lineno+1,script_object);
            exit(3);
        }
        /* resolve the [script_method] field */
        type=0xff;
        /* is it a cry for the object x? */
        if(!strcmp(script_method,MC_EXPR_X)) {
            /* yeah, it'll be that */
            type=TYPE_OBJECT_X;
        } else {
            /* maybe a cry for the object y? */
            if(!strcmp(script_method,MC_EXPR_Y)) {
                /* yeah, dump that in */
                type=TYPE_OBJECT_Y;
            } else {
                /* maybe a cry for the object frame number? */
                if(!strcmp(script_method,MC_EXPR_FRAMENO)) {
                    /* yeah, dump that in */
                    type=TYPE_OBJECT_FRAMENO;
                } else {
                    /* maybe a cry for the object moving flag? */
                    if(!strcmp(script_method,MC_EXPR_MOVING)) {
                        /* yeah, dump that in */
                        type=TYPE_OBJECT_MOVING;
                    } else {
                        /* maybe a cry for the object animating flag? */
                        if(!strcmp(script_method,MC_EXPR_ANIMATING)) {
                            /* yeah, dump that in */
                            type=TYPE_OBJECT_ANIMATING;
                        } else {
                            /* maybe a cry for the object priority code? */
                            if(!strcmp(script_method,MC_EXPR_PRIOCODE)) {
                                /* yeah, dump that in */
                                type=TYPE_OBJECT_PRIORITY;
                            } else {
                                /* maybe a cry for the is dialog finished
                                   flag? */
                                if(!strcmp(script_method,MC_EXPR_FINISHED)) {
                                    /* yeah, dump that in */
                                    type=TYPE_OBJECT_FINISHED;
                                } else {
                                    /* maybe a cry for the current room? */
                                    if(!strcmp(script_method,MC_EXPR_ROOMNO)) {
                                        /* yeah, dump that in */
                                        type=TYPE_OBJECT_ROOMNO;
                                    } else {
                                        if(!strcmp(script_method,MC_EXPR_OLDROOMNO)) {
                                            /* yeah, dump that in */
                                            type=TYPE_OBJECT_OLDROOMNO;
                                        } else {
                                            if(!strcmp(script_method,MC_EXPR_CHOICE)) {
                                                type=TYPE_OBJECT_CHOICE;
                                            } else {
                                                /* ...add new properties here... */
                                            } /* dialog choice */
                                        } /* previous room */
                                    } /* current room */
                                } /* dialog finished flag */
                            } /* priority code */
                        } /* animating flag */
                    } /* moving flag */
                } /* frame number */
            } /* y */
        } /* x */
        /* did we actually resolve it? */
        if(type==0xff) {
            /* no. die */
            printf("%s[%lu]: unknown object property '%s'\n",filename,lineno+1,script_method);
            exit(3);
        }
        /* add the type byte */
        cscript_addbyte(type);
        /* mark this position */
        addobjectchange(script_object,cscriptsize);
        /* and add a phony object number (since we don't know it yet) */
        cscript_addword(MC_DEFAULT_OBJNO);

        /* do the expression stuff */
        do_eval_expr(expr,script_expr,lineno);

        /* all set */
        return;
    }
    /* is this a variable object? */
    if((no=findobject(param))!=MC_NOBJECT) {
        /* is it variable object? */
        if(object[no].type==OBJMAN_TYPE_VARIABLE) {
            /* boy, it is a variable! */
            type=TYPE_VARIABLE;

            /* add the type byte */
            cscript_addbyte(type);
            /* mark this position */
            addobjectchange(param,cscriptsize);
            /* and add a phony variable number (since we don't know it yet) */
            cscript_addword(MC_DEFAULT_OBJNO);

            /* do the expression stuff */
            do_eval_expr(expr,script_expr,lineno);

            /* all done */
            return;
        }
    }
    /* it is something in the form of xxx[]? */
    if((ptr=strchr(param,'['))!=NULL) {
        /* yeah! figure out the name before the '[' */
        for (i=0;i<ptr-param;i++) {
            script_object[i]=param[i];
            script_object[i+1]=0;
        }
        ptr2=strchr(param,']');
        if(ptr2==NULL) {
            printf("%s[%lu]: Found the [ but not the ]\n",filename,lineno+1);
            exit(3);
        }
        /* and assign [script_method] the name between the []'s */
        for(i=0;i<((ptr2-param)-(ptr-param))-1;i++) {
            script_method[i]=param[ptr-param+1+i];
            script_method[i+1]=0;
        }
        /* check if we know it */
        if(!strcmp(script_object,MC_FUNCTION_RANDOM)) {
             /* it is a call for a random number. take care of it */
             /* add the type byte */
             cscript_addbyte(TYPE_SPECIAL_RANDOM);

             /* resolve the max value */
             no=strtoul(script_method,&ptr,0);
             if(ptr==param) {
                 /* we cannot resolve this. die */
                 printf("%s[%lu]: cannot resolve max value '%s'\n",filename,lineno+1,script_method);
                 exit(3);
             }
             /* and the max value */
             cscript_addword(no);

            /* do the expression stuff */
            do_eval_expr(expr,script_expr,lineno);

        } else {
             /* we don't know this. die */
            printf("%s[%lu]: unknown special value '%s'\n",filename,lineno+1,script_object);
            exit(3);
        }
        return;
    }
    /* ok, last guess: is it a static number? */
    no=strtoul(param,&ptr,0);
    if(ptr==param) {
        /* nope. we cannot resolve this. die */
        printf("%s[%lu]: unresolvable expression '%s'\n",filename,lineno+1,param);
        exit(3);
    }
    /* add the type byte */
    cscript_addbyte(TYPE_STATIC);
    /* and the value itself */
    cscript_addlong(no);

    /* do the expression stuff */
    do_eval_expr(expr,script_expr,lineno);
}

/*
 * handledirection(char* param,_ULONG lineno)
 *
 * This will handle direction parameter [param]. [lineno] is expected to be
 * the current script's line number.
 *
 */
void
handledirection(char* param,_ULONG lineno) {
    _UCHAR i;

    /* resolve the direction */
    i=resolvedirection(param);

    /* did this work? */
    if(i==DIR_UNKNOWN) {
        /* no, bail out */
        printf("%s[%lu]: unresolvable direction '%s'\n",filename,lineno+1,param);
        exit(3);
    }
    /* add the direction to the script */
    cscript_addbyte(i);
}

/*
 * handleboolean(char* param,_ULONG lineno)
 *
 * This will handle dboolean parameter [param]. [lineno] is expected to be
 * the current script's line number.
 *
 */
void
handleboolean(char* param,_ULONG lineno) {
    _UCHAR i;

    /* resolve the boolean */
    i=resolveboolean(param);

    /* did this work? */
    if(i==MC_BOOL_UNKNOWN) {
        /* no, bail out */
        printf("%s[%lu]: unresolvable boolean '%s'\n",filename,lineno+1,param);
        exit(3);
    }
    /* add the boolean to the script */
    cscript_addbyte(i);
}

/*
 * add_param(_UCHAR paramtype,char* param)
 *
 * This will add parameter [param] with type [paramtype] to the script.
 * [lineno] should be the current script line number, since it will be
 * printed if anything fails. [script_line] should be the complete script line.
 *
 */
void
add_param(_UCHAR paramtype,char* param,_ULONG lineno,char* script_line) {
    _ULONG i,j;
    _UCHAR type;
    char* ptr;
    char tmp[MC_MAX_SCRIPTLINE_LEN];
    char tempstr[MC_MAX_SCRIPTLINE_LEN];

    switch(paramtype) {
        case MC_TYPE_STRING: /* add a string */
                             cscript_addstring(param);
                             return;
       case MC_TYPE_QSTRING: /* add a quoted string */
                             /* first nuke the quotes */
                             stripquotes(param,tempstr);

                             cscript_addstring(tempstr);
                             return;
          case MC_TYPE_UINT: /* plain unsigned integer */
          case MC_TYPE_SINT: /* plain signed integer */
                             i=strtoul(param,&ptr,0);
                             /* did this work? */
                             if(param==ptr) {
                                 /* no. die */
                                 printf("%s[%lu]: cannot evaluate '%s'\n",filename,lineno+1,param);
                                 exit(3);
                             }
                             cscript_addword(i);
                             return;
         case MC_TYPE_EUINT: /* evaluated unsigned integer */
                             evaluateparam(param,lineno);
                             return;
     case MC_TYPE_DIRECTION: /* direction */
                             handledirection(param,lineno);
                             return;
       case MC_TYPE_BOOLEAN: /* boolean */
                             handleboolean(param,lineno);
                             return;
     case MC_TYPE_TEXTBLOCK: /* text block */
                             memset(tempstr,0,MC_MAX_SCRIPTLINE_LEN);
                             /* !!!CHANGE THE 128 FOR MC_MAX_SCRIPTLINE_LEN!!! */
                             sscanf(script_line,"%s %128c",&tmp,&tempstr);
                             /* nuke all quotes */
                             i=0;
                             while(tempstr[i]) {
                                 if(tempstr[i]=='"') {
                                     memcpy((void *)(tempstr+i),(void *)(tempstr+i+1),strlen(tempstr)-i);
                                 }
                                 i++;
                             }
                             /* add the string */
                             cscript_addstring(tempstr);
                             return;
         case MC_TYPE_LABEL: /* label */
                             /* add a fix entry */
                             addfixlabel(param,cscriptsize);
                             /* and add a mumbo-jumbo address */
                             cscript_addlong(MC_DEFAULT_LABELOFS);
                             return;
         case MC_TYPE_DWORD: /* plain unsigned doubleword */
                             i=strtoul(param,&ptr,0);
                             /* did this work? */
                             if(param==ptr) {
                                 /* no. die */
                                 printf("%s[%lu]: cannot evaluate '%s'\n",filename,lineno+1,param);
                                 exit(3);
                             }
                             cscript_addlong(i);
                             return;

    }
    /* this parameter type is unknown. die */
    printf("internal error: add_param(): parameter type %u unknown for parameter '%s'\n",paramtype,param);
    exit(0xfe);
}

/*
 * add_opcode(_UCHAR opcode,char* paramstr,char* script_line,_ULONG lineno)
 *
 * This will add opcode [opcode] to the script. [paramstr] is assumed to be
 * the parameter format or NULL if there are no parameters. [script_line]
 * should be the entire script line and [lineno] the line number.
 *
 */
void
add_opcode(_UCHAR opcode,char* paramstr,char* script_line,_ULONG lineno) {
    char s[MC_MAX_SCRIPTLINE_PAIRS][MC_MAX_SCRIPTLINE_LEN];
    char tempstr[MC_MAX_PARAMSTR_LEN];
    _UINT i,curparam,j;

    /* clear all s[] entries */
    memset(s,0,MC_MAX_SCRIPTLINE_LEN*MC_MAX_SCRIPTLINE_PAIRS);
    /* split line in space seperated variables */
    sscanf(script_line,"%s %s %s %s %s %s %s %s",&s[0],&s[1],&s[2],&s[3],&s[4],&s[5],&s[6],&s[7]);

    /* add the opcode */
    cscript_addbyte(opcode);

    /* and handle [paramstr] */
    /* just return if there are no parameters */
    if (paramstr==NULL) return;

    /* begin at the first parameter */
    curparam=1;

    /* do all [paramstr] strings */
    for(i=0;i<strlen(paramstr);i++) {
        sscanf((char*)(paramstr+i),"%s",&tempstr);
        i+=strlen(tempstr);

        /* now [tempstr] contains the parameter */
        /* resolve the type of it */
        j=resolveparamtype(tempstr);

        /* did it resolve? */
        if(j==MC_TYPE_UNKNOWN) {
            /* no. die */
            printf("internal error: add_opcode(): command table string parameter unknown (parameter %s)\n",tempstr);
            exit(0xfe);
        }

        /* add the parameters */
        add_param(j,s[curparam],lineno,script_line);

        /* increment the parameter number */
        curparam++;
    }
}

/*
 * get_labelno_from_line(_ULONG line)
 *
 * This will return the label that uses line [line] or MC_NOLABEL if no label
 * uses it.
 *
 */
_ULONG
get_labelno_from_line(_ULONG line) {
    _ULONG i;

    /* scan all labels */
    for(i=0;i<noflabels;i++) {
        /* does this label use [line]? */
        if(label[i].line==line) {
            /* yeah, return its id */
            return i;
        }
    }
    /* no labels use this line. return MC_NOLABEL */
    return MC_NOLABEL;
}

/*
 * handle_object_opcodes(char* script_object,char* script_method,char* script_line,_ULONG lineno,_ULONG objectno)
 *
 * This will handle object opcodes. It expects the script object to be in
 * [script_object] and the method to be in [script_method]. The complete script
 * line must be in [script_line]. [objectno] must contain the object number of [script_object].
 *
 */
void
handle_object_opcodes(char* script_object,char* script_method,char* script_line,_ULONG lineno,_ULONG objectno) {
    _UINT i,j;

    /* add the object opcode */
    cscript_addbyte(OPCODE_OBJECT);

    /* mark this position */
    addobjectchange(script_object,cscriptsize);

    /* write a phony object id (not known yet) */
    cscript_addword(MC_DEFAULT_OBJNO);

    /* check whether [script_method] can be found in the object table */
    for(i=0;i<objectcommandtab->nofcommands;i++) {
        if(!strcmp(script_method,objectcommandtab->command[i].name)) {
            /* we know this method! */
            add_opcode(objectcommandtab->command[i].opcode,objectcommandtab->command[i].parms,script_line,lineno);

            /* warn user if it is depreciated */
            if(objectcommandtab->command[i].depriciated) {
                printf("%s[%lu]: WARNING: '%s' is depriciated!\n",filename,lineno+1,script_method);
            }
            return;
        }
    }
    /* we don't know this method. die */
    printf("%s[%lu]: unknown object method '%s'\n",filename,lineno+1,script_method);
    exit(3);
}

/*
 * do_expression(char* expr,_ULONG lineno,_ULONG* offset)
 *
 * This will handle expression [expr]. [lineno] should be the line number
 * in the script for the expression. To [offset] will the offset be copied
 * where the script interprinter will jump to if the outcome is false.
 *
 */
void
do_expression(char* expr,_ULONG lineno,_ULONG* offset) {
    _UINT i,count,chek,offs,v,s1,a;
    char first_expr[SCRIPT_MAXIDENTLEN];
    char sec_expr[SCRIPT_MAXIDENTLEN];
    char tmp[SCRIPT_MAXIDENTLEN];
    char *ptr,*ptr2;
    _UCHAR type;

    /* split [expr] into usuable parts */
    ptr=strchr(expr,'(');
    if(ptr==NULL) {
        /* no parathese open found. die */
        printf("%s[%lu]: '(' expected\n",filename,lineno+1);
        exit(1);
    }
    /* scan for an equal sign */
    ptr2=strchr(expr,'=');
    chek=CHECK_NOCHECK;v=1;s1=1;
    if(ptr2==NULL) {
        /* no assign char found, so check for < and > */
        if((ptr2=strchr(expr,'<'))!=NULL) {
            /* yeah. found a <! */
            chek=CHECK_SMALLER;
        } else {
            /* maybe a >? */
            if((ptr2=strchr(expr,'>'))!=NULL) {
                /* yeah. found a >! */
                chek=CHECK_GREATER;
            } else {
                /* i guess nothing... */
                chek=CHECK_GREATER;
                ptr2=strchr(expr,')');
                v=0;s1=0;
            }
        }
    } else {
        /* what the char before the =? */
        switch(expr[ptr2-expr-1]) {
            case '>': /* >=, check greater/equal */
                      chek=CHECK_GREATEREQ;
                      break;
            case '<': /* <=, check smaller/equal */
                      chek=CHECK_SMALLEREQ;
                      break;
            case '!': /* !=, check not equal */
                      chek=CHECK_NOTEQUAL;
                      break;
        }
        if(chek==CHECK_NOCHECK) {
            v=0;
            /* maybe the next char is an equal sign */
            switch(expr[ptr2-expr+1]) {
                case '=': /* ==, check equal */
                          chek=CHECK_EQUAL;
                          break;
                 default: /* what is this for operator? */
                          printf("%s[%lu]: Error: cannot resolve operator in string '%s'\n",filename,lineno+1,expr);
                          exit(1);
            }
        }
    }
    /* is the first char of the first expression an exclamation mark? */
    a=0;
    if(expr[(ptr-expr)+1]=='!') {
        /* yeah. set flags */
        a=1;chek=CHECK_EQUAL;v=0;s1=0;
    }
    /* set first expression the the stuff between the ( and the operator */
    count=0;
    for(i=(ptr-expr)+1+a;i<(ptr2-expr)-v;i++) {
        first_expr[count]=expr[i];
        first_expr[count+1]=0;
        count++;
    }
    /* write the operator to the script */
    cscript_addbyte(chek);
    /* set the second expression the the stuff between the operator and the ) */
    count=0;
    for(i=(ptr2-expr)+2-v;i<(strchr(expr,')')-expr);i++) {
        sec_expr[count]=expr[i];
        sec_expr[count+1]=0;
        count++;
    }
    /* add the first expression */
    evaluateparam(first_expr,lineno);
    /* if it wasnt a if (blah) { thing, do the second expression */
    if(s1) {
        /* handle the second expression */
        evaluateparam(sec_expr,lineno);
    } else {
        /* add a TYPE_STATIC type with value 0 */
        cscript_addbyte(TYPE_STATIC);
        cscript_addlong(0);
        do_eval_expr(EXPR_NONE,NULL,lineno);
    }
    /* now save the offset */
    *offset=cscriptsize;
    /* and add a mumbo-jumbo offset */
    cscript_addlong(MC_DEFAULT_LABELOFS);
}

/*
 * compile_line(char* script_line,_ULONG lineno)
 *
 * This will compile script line [script_line]. It expects [lineno] to be
 * the line number. It will return the number of lines to skip.
 *
 */
_ULONG
compile_line(char* script_line,_ULONG lineno) {
    char s[MC_MAX_SCRIPTLINE_PAIRS][MC_MAX_SCRIPTLINE_LEN];
    char script_object[MC_MAX_SCRIPTLINE_LEN];
    char script_method[MC_MAX_SCRIPTLINE_LEN];
    char script_temp[MC_MAX_SCRIPTLINE_LEN];
    _ULONG i,objectno,count,j;
    char* ptr;

    /* clear all s[] entries */
    memset(s,0,MC_MAX_SCRIPTLINE_LEN*MC_MAX_SCRIPTLINE_PAIRS);
    /* split line in space seperated variables */
    sscanf(script_line,"%s %s %s %s %s %s %s %s",&s[0],&s[1],&s[2],&s[3],&s[4],&s[5],&s[6],&s[7]);

    /* is there a . in the line? */
    if((ptr=strchr(s[0],'.'))!=NULL) {
        /* yeah, there is. */
        /* [script_object] is going to be the object name before the dot */
        for (i=0;i<ptr-s[0];i++) {
            script_object[i]=s[0][i];
            script_object[i+1]=0;
        }
        /* [script_method] is the method after the dot */
        for (i=0;i<(strlen(s[0])-(ptr-s[0]))-1;i++) {
            script_method[i]=s[0][i+1+ptr-s[0]];
            script_method[i+1]=0;
        }
        /* check whether the object was ever declared */
        if((objectno=findobject(script_object))!=MC_NOBJECT) {
            /* it was. handle it as an object method */
            handle_object_opcodes(script_object,script_method,script_line,lineno,objectno);
            /* get outta here! */
            return 0;
        }
    }
    /* well, that wasn't an object with a method. do the rest */
    /* check whether [s[0]] can be found in the object table */
    for(i=0;i<commandtab->nofcommands;i++) {
        if(!strcmp(s[0],commandtab->command[i].name)) {
            /* we know this method! */
            add_opcode(commandtab->command[i].opcode,commandtab->command[i].parms,script_line,lineno);

            if(commandtab->command[i].depriciated) {
                printf("%s[%lu]: WARNING: '%s' is depriciated!\n",filename,lineno+1,s[0]);
            }
            /* get outta here! */
            return 0;
        }
    }
    /* geez, still not done the command. */
    /* is there a equal sign in it? */
    if((ptr=strchr(s[0],'='))!=NULL) {
        /* yeah, this is probably a variable assignment */
        /* [script_object] will become the value name before the equal sign */
        for (i=0;i<ptr-s[0];i++) {
            script_object[i]=s[0][i];
            script_object[i+1]=0;
        }
        /* [script_method] will become the data after the equal sign */
        for (i=0;i<(strlen(s[0])-(ptr-s[0]));i++) {
            script_method[i]=s[0][i+1+ptr-s[0]];
            script_method[i+1]=0;
        }
        /* check whether object [script_object] was ever declared */
        if((objectno=findobject(script_object))!=MC_NOBJECT) {
            /* it was. */
            /* add a set variable opcode */
            cscript_addbyte(OPCODE_SETVAR);

            /* mark this position */
            addobjectchange(script_object,cscriptsize);

            /* write a phony object id (not known yet) */
            cscript_addword(MC_DEFAULT_OBJNO);

            /* and add the result */
            evaluateparam(script_method,lineno);

            /* now get outta here! */
            return 0;
        }
    }
    /* well, maybe this is an MC_COMMAND_IF or a MC_COMMAND_WHILE? */
    /* if it is a IF or WHILE make sure there's a space between the command and
       the ( */
    if(!strncmp(s[0],MC_COMMAND_IF,strlen(MC_COMMAND_IF)-1)) {
        /* it's an if. */
        if(s[0][strlen(MC_COMMAND_IF)]=='(') {
            /* insert a space */
            /* first seek where the spaces end */
            for(i=0;i<strlen(script_line);i++) {
                if(script_line[i]!=0x20) break;
            }
            /* now insert the space */
            insert(script_line+i,strlen(MC_COMMAND_IF),' ');
        }
        /* clear all s[] entries */
        memset(s,0,MC_MAX_SCRIPTLINE_LEN*MC_MAX_SCRIPTLINE_PAIRS);
        /* split line in space seperated variables */
        sscanf(script_line,"%s %s %s %s %s %s %s %s",&s[0],&s[1],&s[2],&s[3],&s[4],&s[5],&s[6],&s[7]);
    }

    j=1;
    if((!strcmp(s[0],MC_COMMAND_IF))||(!(j=strcmp(s[0],MC_COMMAND_WHILE)))) {
        /* yipee! it actually IS an if. take care of it */
        count=0;
        for(i=(strchr(script_line,')')-script_line)+1;i<strlen(script_line);i++) {
            script_method[count]=script_line[i];
            script_method[count+1]=0;
            count++;
        }
        /*
         * now [script_method] is whatever the user
         * wants call when this is true and
         * s[1] is the expression to be checked
         *
         */
        if(!j) {
            cscript_addbyte(OPCODE_WHILE);
        } else {
            cscript_addbyte(OPCODE_IF);
        }
        do_expression(s[1],lineno,&objectno);

        /*
         * [objectno] is the offset we need to modify
         *
         */

        /* make sure all spaces etc are nuked */
        sscanf(script_method,"%s",&script_temp);

        /* now check whether a single command was specified or a block. */
        if(!strcmp(script_temp,MC_START_PROC)) {
            /* it is a complete block. */

            i=lineno+1;
            do {
                /* get the new line */
                sscanf(ucscript[i],"%s",&script_temp);

                /* is this an MC_END_PROC string? */
                if(!strcmp(script_temp,MC_END_PROC)) {
                     /* yeah, break out */
                     break;
                }

                /* compile it */
                count=compile_line(ucscript[i],i);
                i+=count;

                i++;
            } while (1);
            /* correct the lines to skip variable (i) */
            i-=(lineno);
        } else {
            /* just a single line. take care of it */
            compile_line(script_method,lineno);
            i=0;
        }

        /* correct the offset */
        count=cscriptsize;
        memcpy((void*)(cscript+objectno),&count,4);

        /* get outta here! */
        return i;
    }
    /* first char a comment char or a blank line? */
    if((s[0][0]==MC_CHAR_COMMENT)||(s[0][0]=='\0')) {
        /* yeah, ignore the line */
        return 0;
    }
    /* is the first word a MC_DEFINE_OBJ? */
    if(!strcmp(s[0],MC_DEFINE_OBJ)) {
        /* yeah, add a active command */
        cscript_addbyte(OPCODE_OBJ_ACTIVATE);
        /* search the object */
        if((objectno=findobject(s[1]))==MC_NOBJECT) {
            /* it doesn't exists. die */
            printf("%s[%lu]: internal error: object '%s' could not be found when it was about to be activated\n",filename,lineno+1,s[1]);
            exit(3);
        }
        /* mark this position */
        addobjectchange(object[objectno].name,cscriptsize);

        /* write a phony object id (not known yet) */
        cscript_addword(MC_DEFAULT_OBJNO);

        /* add the object type */
        cscript_addbyte(object[objectno].type);

        /* add the source file name */
        cscript_addstring(object[objectno].sourcefile);

        /* and add the object name */
        cscript_addstring(object[objectno].name);

        /* get outta here! */
        return 0;
    }
    /* if this line denotes a label, leave it alone */
    if(get_labelno_from_line(lineno)!=MC_NOLABEL) {
        /* it does! get outta here! */
        return 0;
    }
    /* is there a + in the line? */
    if((ptr=strchr(s[0],'+'))!=NULL) {
        /* yeah, there is. */
        /* is the next char also a plus? */
        if(s[0][ptr-s[0]+1]=='+') {
            /* yeah. this is something in the form [value]++. set
               [script_object] to the value before the + */
            for (i=0;i<ptr-s[0];i++) {
                script_object[i]=s[0][i];
                script_object[i+1]=0;
            }
            /* try to find it */
            if((objectno=findobject(script_object))!=MC_NOBJECT) {
                /* found it. */
                /* add a set variable opcode */
                cscript_addbyte(OPCODE_SETVAR);

                /* mark this position */
                addobjectchange(script_object,cscriptsize);

                /* write a phony object id (not known yet) */
                cscript_addword(MC_DEFAULT_OBJNO);

                strcpy(script_method,script_object);
                strcat(script_method,"+1");

                /* and add the result */
                evaluateparam(script_method,lineno);

                /* get outta here! */
                return 0;
            }
        }
    }
    /* if it is a procedure defination, ignore it */
    if(!strcmp(s[1],MC_START_PROC)) {
         /* it is! ignore it */
         return 0;
    }
    /* if it is a procedure ending, ignore it */
    if(!strcmp(s[0],MC_END_PROC)) {
         /* it is! ignore it */
         return 0;
    }
    /*
     * add new special parsed lines here, like:
     *
     * // is it the blah function?
     * if(!strcmp(s[0],MC_BLAH)) {
     *    // yeah, take care of it
     *    ...code...
     *
     *    // get out of here!
     *    return 0;
     * }
     *
     *
     */
    /* we don't recognise this command. die */
    printf("%s[%lu]: cannot interprint '%s'\n",filename,lineno+1,script_line);
    exit(3);
}

/*
 * compile_proc(_ULONG procno)
 *
 * This will compile procedure number [procno].
 *
 */
void
compile_proc(_ULONG procno) {
    _ULONG line,i;

    proc[procno].startbyte=cscriptsize;
    for(line=proc[procno].startline+1;line<proc[procno].endline;line++) {
        i=compile_line(ucscript[line],line);

        line+=i;

        /* is this line used by a label? */
        if((i=get_labelno_from_line(line))!=MC_NOLABEL) {
            /* yeah, tweak the offset correct */
            label[i].offset=cscriptsize;
        }
    } /* line */
    proc[procno].endbyte=cscriptsize;
}

/*
 * fix_labels()
 *
 * This will fix all label entries.
 *
 */
void
fix_labels() {
    _ULONG i,j;

    for(i=0;i<nofixlabels;i++) {
        j=label[fixlabel[i].labelno].offset;

        memcpy((void*)(cscript+fixlabel[i].offset),(void*)&j,4);
    }
}

/*
 * writescript()
 *
 * This will write the compiled script to the disk
 *
 */
void
writescript() {
    struct MC_SCRIPTHEADER hdr;
    FILE *f;
    char tempfname[MC_MAX_FILENAME_LEN];
    _UINT i;

    /* first get the base filename of the filename */
    getbasename(filename,tempfname);

    /* and add the compiled script extension */
    strcat(tempfname,MC_EXT_CSCRIPT);

    /* create the file */
    if((f=fopen(tempfname,"wb"))==NULL) {
        /* this failed. die */
        printf("unable to create file '%s'\n",tempfname);
        exit(1);
    }

    /* set up script header */
    memset(&hdr,0,sizeof(MC_SCRIPTHEADER));
    hdr.magicno=MC_HEADER_MAGICNO;
    hdr.version=MC_HEADER_VERSIONO;
    hdr.script_size=cscriptsize;
    hdr.nofprocs=nofprocs;
    hdr.nofobjects=nofobjects;
/*    printf("sizeof(hdr.magicno) = %u\n",sizeof(hdr.magicno));
    printf("sizeof(hdr.version) = %u\n",sizeof(hdr.version));
    printf("sizeof(hdr.script_size) = %u\n",sizeof(hdr.script_size));
    printf("sizeof(hdr.nofprocs) = %u\n",sizeof(hdr.nofprocs));
    printf("sizeof(hdr.nofobjects) = %u\n",sizeof(hdr.nofobjects));
    printf("sizeof(hdr.reserved) = %u\n",sizeof(hdr.reserved));
    printf("sizeof(hdr) = %u\n",sizeof(hdr));
    printf("sizeof(MC_SCRIPTHEADER) = %u\n",sizeof(MC_SCRIPTHEADER));*/

    /* write the header */
    if(!fwrite(&hdr,sizeof(MC_SCRIPTHEADER),1,f)) {
        /* this failed. die */
        printf("unable to write file '%s'\n",tempfname);
        exit(1);
    }
    /* write the script data to it */
    if(!fwrite(cscript,cscriptsize,1,f)) {
        /* this failed. die */
        printf("unable to write file '%s'\n",tempfname);
        exit(1);
    }
    /* write all procedures to it */
    for(i=0;i<nofprocs;i++) {
        if(!fwrite(proc[i].name,MC_MAX_PROCNAME_LEN,1,f)) {
            /* this failed. die */
            printf("unable to write file '%s'\n",tempfname);
            exit(1);
        }
        if(!fwrite(&proc[i].startbyte,4,1,f)) {
            /* this failed. die */
            printf("unable to write file '%s'\n",tempfname);
            exit(1);
        }
        if(!fwrite(&proc[i].endbyte,4,1,f)) {
            /* this failed. die */
            printf("unable to write file '%s'\n",tempfname);
            exit(1);
        }
    }
    /* and dump the object info */
    for(i=0;i<nofobjects;i++) {
        if(!fwrite(object[i].name,MC_MAX_OBJECT_LEN,1,f)) {
            /* this failed. die */
            printf("unable to write file '%s'\n",tempfname);
            exit(1);
        }
        if(!fwrite(&object[i].type,1,1,f)) {
            /* this failed. die */
            printf("unable to write file '%s'\n",tempfname);
            exit(1);
        }
        if(!fwrite(&object[i].flags,4,1,f)) {
            /* this failed. die */
            printf("unable to write file '%s'\n",tempfname);
            exit(1);
        }
        if(!fwrite(&object[i].nofchanges,4,1,f)) {
            /* this failed. die */
            printf("unable to write file '%s'\n",tempfname);
            exit(1);
        }
        /* only write changes if there are changes to be made */
        if(object[i].nofchanges) {
            if(!fwrite(&object[i].change,4*object[i].nofchanges,1,f)) {
                /* this failed. die */
                printf("unable to write file '%s'\n",tempfname);
                exit(1);
            }
        }
    }
    /* neatly close the file */
    fclose(f);
}

/*
 * main(_UINT argc,char* argv[])
 *
 * This is the main procedure. This will be called first.
 *
 */
int
main(int argc,char* argv[]) {
    _ULONG i;

    /* parse the parameters */
    parseparms(argc,argv);

    /* load the script */
    loadscript(filename);

    /* initialize the system objects */
    initobjects();

    /* now the script file is loaded. parse it (remove all comments etc.) */
    if(verbose) {
        printf("> Parsing script\n");
        fflush(stdout);
    }
    parsescript();
    if(verbose) {
        printf("> Parsing done\n");
        printf("> Compiling procedures\n");
        fflush(stdout);
    }
    /* now compile all procedures */
    for(i=0;i<nofprocs;i++) {
        if(verbose) {
            printf("Compiling procedure '%s'...",proc[i].name);
            fflush(stdout);
        }
        compile_proc(i);
        if(verbose) {
            printf(" done\n");
            fflush(stdout);
        }
    }
    if(verbose) {
        printf("> Compiling done\n");
        printf("> Fixing labels...");
        fflush(stdout);
    }
    fix_labels();
    if(verbose) {
        printf(" done\n");
        printf("> Writing script...");
        fflush(stdout);
    }
    writescript();
    if(verbose) {
        printf(" done\n");
        fflush(stdout);
    }
    return 0;
}
