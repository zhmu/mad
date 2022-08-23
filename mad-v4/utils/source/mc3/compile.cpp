/*
 *                             The MAD Compiler
 *                             Compiler Routines
 *                               Version 3.0
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

struct     MC_COMMAND cmp_objcmd[] = {
  { "enable",            OPCODE_OBJ_ENABLE,       NULL,               0 },
  { "disable",           OPCODE_OBJ_DISABLE,      NULL,               0 },
  { "loadsprite",        OPCODE_OBJ_LOADSPR,      "%q",               0 },
  { "promptexit",        OPCODE_OBJ_PROMPTEXIT,   NULL,               0 },
  { "addicon",           OPCODE_OBJ_ADDICON,      "%q %q %q %q %u",   0 },
  { "setpriority",       OPCODE_OBJ_SETPRI,       "%e",               0 },
  { "move",              OPCODE_OBJ_MOVE,         "%e %e",            0 },
  { "sethalt",           OPCODE_OBJ_SETHALT,      "%q",               0 },
  { "setmove",           OPCODE_OBJ_SETMOVE,      "%q",               0 },
  { "setspeed",          OPCODE_OBJ_SETSPEED,     "%u %u %u",         0 },
  { "face",              OPCODE_OBJ_FACE,         "%d",               0 },
  { "setmoveobj",        OPCODE_OBJ_SETMOVEOBJ,   NULL,               0 },
  { "settext",           OPCODE_OBJ_SETTEXT,      "%t",               1 },
  { "setcolor",          OPCODE_OBJ_SETCOLOR,     "%e",               0 },
  { "show",              OPCODE_OBJ_SHOW,         NULL,               0 },
  { "hide",              OPCODE_OBJ_HIDE,         NULL,               0 },
  { "go",                OPCODE_OBJ_GO,           NULL,               0 },
  { "dispose",           OPCODE_OBJ_DISPOSE,      NULL,               0 },
  { "load",              OPCODE_OBJ_LOAD,         "%q",               0 },
  { "stop",              OPCODE_OBJ_STOP,         NULL,               0 },
  { "setanim",           OPCODE_OBJ_SETANIM,      "%q",               0 },
  { "setloop",           OPCODE_OBJ_SETLOOP,      "%u",               0 },
  { "animate",           OPCODE_OBJ_ANIMATE,      NULL,               0 },
  { "moveto",            OPCODE_OBJ_MOVETO,       "%e %e",            0 },
  { "unload",            OPCODE_OBJ_UNLOAD,       NULL,               0 },
  { "destroy",           OPCODE_OBJ_DESTROY,      NULL,               1 },
  { "setclip",           OPCODE_OBJ_SETCLIP,      "%b",               0 },
  { "setmask",           OPCODE_OBJ_SETMASK,      "%d",               0 },
  { "darkenpal",         OPCODE_OBJ_DARKENPAL,    "%i",               0 },
  { "restorepal",        OPCODE_OBJ_RESTOREPAL,   NULL,               0 },
  { "fadeout",           OPCODE_OBJ_FADEOUT,      NULL,               0 },
  { "fadein",            OPCODE_OBJ_FADEIN,       NULL,               0 },
  { "loadtext",          OPCODE_OBJ_LOADTEXT,     "%e",               0 },
  { "createtextdialog",  OPCODE_OBJ_CREATETXTDLG, "%u %u",            0 },
  { "loadbackdrop",      OPCODE_OBJ_LOADBCKDRP,   "%q",               0 },
  { "exit",              OPCODE_OBJ_EXIT,         NULL,               0 },
  { "loadborder",        OPCODE_OBJ_LOADBORDER,   "%q %q %q %q",      0 },
  { "dodialog",          OPCODE_OBJ_DODIALOG,     "%u",               0 },
  { "teleport",          OPCODE_OBJ_TELEPORT,     "%u",               0 },
  { "showmessage",       OPCODE_OBJ_SHOWMESSAGE,  "%e",               0 },
  { "converse",          OPCODE_OBJ_CONVERSE,     "%u %u %u %u %e",   0 } };

struct MC_COMMAND cmp_cmdtab[] = {
  { "loadscreen",        OPCODE_LOADSCREEN,       "%q",               0 },
  { "goto",              OPCODE_JUMP,             "%l",               0 },
  { "refresh",           OPCODE_REFRESH,          NULL,               0 },
  { "return",            OPCODE_RETURN,           NULL,               0 },
  { "delay",             OPCODE_DELAY,            "%u",               1 },
  { "leave",             OPCODE_LEAVE,            NULL,               0 },
  { "call",              OPCODE_CALL,             "%s",               0 } };

struct  MC_PROPERTY cmp_property[] = {
  { "x",                 TYPE_OBJECT_X,                                 },
  { "y",                 TYPE_OBJECT_Y,                                 },
  { "frameno",           TYPE_OBJECT_FRAMENO,                           },
  { "moving",            TYPE_OBJECT_MOVING,                            },
  { "animating",         TYPE_OBJECT_ANIMATING,                         },
  { "prioritycode",      TYPE_OBJECT_PRIORITY,                          },
  { "finished",          TYPE_OBJECT_FINISHED,                          },
  { "roomno",            TYPE_OBJECT_ROOMNO,                            },
  { "oldroomno",         TYPE_OBJECT_OLDROOMNO,                         },
  { "dialogchoice",      TYPE_OBJECT_CHOICE                             } };

struct  MC_SPECIAL cmp_special[] = {
  { "random",            TYPE_SPECIAL_RANDOM, "%e"                      } };

char*      cmp_codebuf;
_ULONG     cmp_codebuflen;

_ULONG*    cmp_linetab;
_ULONG     cmp_noflines;

MC_PROC*   cmp_proc;
_ULONG     cmp_nofprocs;

MC_LABEL*  cmp_label;
_ULONG     cmp_noflabels;

MC_OBJECT* cmp_object;
_ULONG     cmp_nofobjects;
_ULONG     cmp_objcount;

_ULONG     cmp_comp_script_bufsize;
_ULONG     cmp_comp_script_size;
char*      cmp_comp_script;

char       cmp_pair[MC_MAX_SCRIPT_PAIRS][MC_MAX_PAIR_LENGTH];
char       cmp_temp[MC_MAX_PAIR_LENGTH];
char       cmp_scriptname[MC_MAX_SCRIPTNAME_LEN];

/*
 * cmp_splitpairs(char* s)
 *
 * This will split [s] in pair [cmp_pair][i]. It will return the number of
 * pairs successfuly split.
 *
 */
_UINT
cmp_splitpairs(char* s) {
    /* first clean all splitted stuff */
    memset(cmp_pair,0,MC_MAX_SCRIPT_PAIRS*MC_MAX_PAIR_LENGTH);

    /* split them! */
    return (sscanf(s,"%s %s %s %s %s %s %s %s",&cmp_pair[0], &cmp_pair[1],
                                               &cmp_pair[2], &cmp_pair[3],
                                               &cmp_pair[4], &cmp_pair[5],
                                               &cmp_pair[6], &cmp_pair[7]));
}

/*
 * cmp_cleanup()
 *
 * This will do a complete cleanup of all variables and such.
 *
 */
void
cmp_cleanup() {
    _ULONG i;

    /* nuke the buffers */
    if(cmp_codebuf!=NULL) {
        /* there's still data in the code buffer. Free it */
        free(cmp_codebuf);
    }
    if(cmp_linetab!=NULL) {
        /* there's still data in the line table buffer. Free it */
        free(cmp_linetab);
    }
    if(cmp_proc!=NULL) {
        /* there's still data in the procedure table buffer. Free it */
        free(cmp_proc);
    }
    if(cmp_label!=NULL) {
        /* there's still data in the label table. First scan all objects for
           used data */
        for(i=0;i<cmp_noflabels;i++) {
            /* is there data for the change[] buffer? */
            if(cmp_label[i].fixes!=NULL) {
                /* yes, free it */
                free(cmp_label[i].fixes);
            }
        }
        /* nuke the buffer */
        free(cmp_label);
    }
    if(cmp_object!=NULL) {
        /* there's still data in the object table. First scan all objects for
           used data */
        for(i=0;i<cmp_nofobjects;i++) {
            /* is there data for the change[] buffer? */
            if(cmp_object[i].change!=NULL) {
                /* yes, free it */
                free(cmp_object[i].change);
            }
        }
        /* and nuke the buffer */
        free(cmp_object);
    }
    if(cmp_comp_script!=NULL) {
        /* there's still data in the compiled script buffer. Free it */
        free(cmp_comp_script);
    }
    /* make sure all pointers are null to prevent them from being freed
       AGAIN */
    cmp_codebuf=NULL; cmp_linetab=NULL; cmp_proc=NULL; cmp_label=NULL;
    cmp_object=NULL;cmp_comp_script=NULL;

    /* set all other variables to zero */
    cmp_noflines=0; cmp_codebuflen=0; cmp_nofprocs=0; cmp_noflabels=0;
    cmp_nofobjects=0; cmp_objcount=0; cmp_comp_script_bufsize=0;
    cmp_comp_script_size=0;
}

/*
 * cmp_init()
 *
 * This will initialize some compiler variables.
 *
 */
void
cmp_init() {
    /* make sure the buffers won't be freed */
    cmp_codebuf=NULL; cmp_linetab=NULL;cmp_proc=NULL;cmp_label=NULL;
    cmp_object=NULL;cmp_comp_script=NULL;

    /* let cmp_cleanup() clean the rest */
    cmp_cleanup();
}

/*
 * cmp_loadfile(char* fname,_ULONG* codebuflen,char** codebuf,
 *              _ULONG* noflines,_ULONG** linetab,
 *              _ULONG* comp_script_bufsize,char** comp_script)
 *
 * This will load file [fname]. It will quit on any errors. It'll dump the
 * size to [codebuflen], the buffer to [codebuf], the number of lines to
 * [noflines], the line table buffer to [linetab], the compiled script size
 * to [comp_script_bufsize], the compiled script to [comp_script].
 *
 */
void
cmp_loadfile(char* fname,_ULONG* codebuflen,char** codebuf,_ULONG* noflines,_ULONG** linetab, _ULONG* comp_script_bufsize,char** comp_script) {
    FILE *f;
    _ULONG i,j;

    /* make sure everything is tidy before we begin */
    cmp_cleanup();

    /* open the file */
    if((f=fopen(fname,"rb"))==NULL) {
        /* this failed. die */
        fprintf(stderr," unable to open file '%s'\n",fname);
        exit(1);
    }
    /* figure out the size */
    fseek(f,0,SEEK_END); *codebuflen=ftell(f); rewind(f);

    /* allocate memory for the buffer */
    if(((*codebuf)=(char*)malloc(*codebuflen))==NULL) {
        /* the allocation failed. die */
        fprintf(stderr," allocation of %lu bytes failed\n",*codebuflen);
        exit(1);
    }

    /* read the file contents */
    if(!fread((*codebuf),*codebuflen,1,f)) {
        /* this failed. die */
        fprintf(stderr," unable to read %lu bytes from file '%s'\n",*codebuflen,fname);
        cmp_cleanup();
        exit(1);
    }

    /* close the file */
    fclose(f);

    *noflines=0;
    /* now scan how much lines there are in the file */
    for(i=0;i<*codebuflen;i++) {
        /* is this a newline? */
        if((*codebuf)[i]=='\n') {
            /* Yes. Increment the line count */
            (*noflines)++;
        }
    }
    /* add two lines (we are using base 1 now) */
    (*noflines)+=2;

    /* allocate memory for the line offset buffer */
    if(((*linetab)=(_ULONG*)malloc((*noflines)*sizeof(_ULONG)))==NULL) {
        /* this failed. die */
        fprintf(stderr," allocation of %lu bytes failed\n",*noflines*sizeof(_ULONG));
        cmp_cleanup();
        exit(1);
    }

    /* build the line offset table */
    (*linetab)[1]=0; j=2;
    for(i=0;i<(*codebuflen);i++) {
        /* is this a newline? */
        if((*codebuf)[i]=='\n') {
            /* yes. Add the offset */
            (*linetab)[j]=i+1;
            /* turn the newline into a nul */
            (*codebuf)[i]=0;
            /* if the previous char was a linefeed, also make it null */
            if((*codebuf)[i-1]=='\r') {
                (*codebuf)[i-1]=0;
            }
            /* next line */
            j++;
            /* check for boundries (just to make sure) */
            if(j>(*noflines)) {
                /* out of boundries! show error */
                fprintf(stderr," (INTERNAL ERROR) calculation said there were %lu lines in file, yet we were told to process line %lu\n",(*noflines),j);
                cmp_cleanup();
                exit(0xfe);
            }
        }
    }

    /* the last line in a trash line, so remove it */
    *noflines--;

    /* allocate memory for the compiled script data */
    *comp_script_bufsize=MC_DEFAULT_SCRIPTSIZE;
    if((*comp_script=(char*)malloc(*comp_script_bufsize))==NULL) {
        /* this failed. die */
        fprintf(stderr," allocation of %lu bytes failed\n",*comp_script_bufsize);
        cmp_cleanup();
        exit(1);
    }
}

/*
 * cmp_findproc(char* name)
 *
 * This will scan for procedure [name]. When it is found, it will return the
 * procedure ID, otherwise MC_NOPROC.
 *
 */
_ULONG
cmp_findproc(char* name) {
    _ULONG i;

    /* scan all procedures */
    for(i=0;i<cmp_nofprocs;i++) {
        /* is this the one? */
        if(!strcmp(name,cmp_proc[i].name)) {
            /* yup. return its id */
            return i;
        }
    }
    /* the procedure was not found. return MC_NOPROC */
    return MC_NOPROC;
}

/*
 * cmp_findlabel(char* name)
 *
 * This will scan for label [name]. When it is found, it will return the label
 * ID, otherwise MC_NOLABEL.
 *
 */
_ULONG
cmp_findlabel(char* name) {
    _ULONG i;

    /* scan all procedures */
    for(i=0;i<cmp_noflabels;i++) {
        /* is this the one? */
        if(!strcmp(name,cmp_label[i].name)) {
            /* yup. return its id */
            return i;
        }
    }
    /* the label was not found. return MC_NOLABEL */
    return MC_NOLABEL;
}

/*
 * cmp_findobject(char* name)
 *
 * This will scan for object [name]. When it is found, it will return the
 * object ID, otherwise MC_NOBJECT.
 *
 */
_ULONG
cmp_findobject(char* name) {
    _ULONG i;

    /* scan all procedures */
    for(i=0;i<cmp_nofobjects;i++) {
        /* is this the one? */
        if(!strcmp(name,cmp_object[i].name)) {
            /* yup. return its id */
            return i;
        }
    }
    /* the procedure was not found. return MC_NOBJECT */
    return MC_NOBJECT;
}

/*
 * cmp_labelhere(_ULONG lineno)
 *
 * This will return the ID of the label that is at line [lineno], otherwise
 * MC_NOLABEL;
 *
 */
_ULONG
cmp_labelhere(_ULONG lineno) {
    _ULONG i;

    /* scan all procedures */
    for(i=0;i<cmp_noflabels;i++) {
        /* is this the one? */
        if(cmp_label[i].line==lineno) {
            /* yup. return its id */
            return i;
        }
    }
    /* the label was not found. return MC_NOLABEL */
    return MC_NOLABEL;
}

/*
 * cmp_addobject(char* objname,_UCHAR objtype,char* sourcefile,_ULONG flags)
 *
 * This will add object [objname] with object type [objtype], and with source
 * file [sourcefile]. It will get flags [flags].
 *
 */
void
cmp_addobject(char* objname,_UCHAR objtype,char* sourcefile,_ULONG flags) {
    MC_OBJECT* obj;

    /* check whether the name is short enough */
    if(strlen(objname)>MC_MAX_OBJECT_LEN) {
        /* it's not. die */
        fprintf(stderr," object name '%s' is too long, it can be maximal %lu chars\n",objname,MC_MAX_OBJECT_LEN);
        cmp_cleanup();
        exit(1);
    }

    /* add the object */
    obj=(MC_OBJECT*)&(cmp_object[cmp_objcount]);
    strcpy(obj->name,objname);

    /* if the sourcefile is not null, add it */
    if(sourcefile!=NULL) {
        /* check whether the source filename is short enough */
        if(strlen(sourcefile)>MC_MAX_FILENAME_LEN) {
            /* it's not. die */
            fprintf(stderr," filename '%s' is too long, it can be maximal %lu chars\n",sourcefile,MC_MAX_FILENAME_LEN);
            cmp_cleanup();
            exit(1);
        }
        strcpy(obj->sourcefile,sourcefile);
    }
    /* set the flags */
    obj->flags=flags;

    /* allocate memory for the change buffer */
    obj->maxchanges=MC_DEFAULT_CHANGESIZE;
    if((obj->change=(_ULONG*)malloc(obj->maxchanges*sizeof(_ULONG)))==NULL) {
        /* this failed. die */
        fprintf(stderr," allocation of %lu bytes failed\n",obj->maxchanges*sizeof(_ULONG));
        cmp_cleanup();
        exit(1);
    }

    /* if verbose, show it */
    if(mc_message_level==MC_MESSAGELEVEL_VERBOSE) {
        printf("obj $%02x: name '%s' file '%s' type %u flags $%x\n",cmp_objcount,objname,sourcefile,objtype,flags);
    }

    /* increment the number of objects */
    cmp_objcount++;

    /* check for boundries (just to make sure) */
    if(cmp_objcount>cmp_nofobjects) {
        /* out of boundries! show error */
        fprintf(stderr," (INTERNAL ERROR) calculation said there were %lu objects in the file, yet we were told to process object %lu\n",cmp_nofobjects,cmp_objcount);
        cmp_cleanup();
        exit(0xfe);
    }
}

/*
 * cmp_add_system_objects()
 *
 * This will add the system objects.
 *
 */
void
cmp_add_system_objects() {
    /* add the system objects */
    cmp_addobject(MC_MOUSE_OBJNAME,OBJMAN_TYPE_ANM,NULL,MC_FLAG_INTERNAL);
    cmp_addobject(MC_BAR_OBJNAME,OBJMAN_TYPE_ANM,NULL,MC_FLAG_INTERNAL);
    cmp_addobject(MC_CONTROLS_OBJNAME,OBJMAN_TYPE_ANM,NULL,MC_FLAG_INTERNAL);
    cmp_addobject(MC_SYSTEM_OBJNAME,OBJMAN_TYPE_ANM,NULL,MC_FLAG_INTERNAL);
    cmp_addobject(MC_SCRIPT_OBJNAME,OBJMAN_TYPE_ANM,NULL,MC_FLAG_INTERNAL);
    cmp_addobject(MC_PARSER_OBJNAME,OBJMAN_TYPE_ANM,NULL,MC_FLAG_INTERNAL);
    cmp_addobject(MC_DIALOG_OBJNAME,OBJMAN_TYPE_ANM,NULL,MC_FLAG_INTERNAL);
}

/*
 * cmp_preprocess(&cmp_codebuflen,&cmp_codebuf,&cmp_noflines,&cmp_linetab,&cmp_comp_script_bufsize,&cmp_comp_script)
 *
 * This will preprocess the correctly loaded file. It will die on any error.
 *
 */
void
cmp_preprocess(_ULONG* codebuflen,char** codebuf,_ULONG* noflines,_ULONG** linetab, _ULONG* comp_script_bufsize,char** comp_script) {
    _ULONG   i,j,n,proc,label;
    _UINT    nofpairs;
    _UCHAR   type;
    _UCHAR   stat;
    MC_PROC* procptr;
    char*    line;
    char*    ptr;
    char     cmp_objtemp[MC_MAX_PAIR_LENGTH];
    char     cmp_supertemp[MC_MAX_PAIR_LENGTH];
    char     cmp_filetemp[MC_MAX_PAIR_LENGTH];
    char     cmp_typetemp[MC_MAX_PAIR_LENGTH];

    /* scan all lines */
    for(i=1;i<cmp_noflines;i++) {
        /* create a pointer to the line */
        line=(char*)(cmp_codebuf+cmp_linetab[i]);

        /* split it */
        nofpairs=cmp_splitpairs(line);

        /* when there are 2 pairs, it might be a procedure definition */
        if(nofpairs==2) {
            /* is the last pair the magic string MC_PROCSTART? */
            if(!strcmp(cmp_pair[1],MC_PROCSTART)) {
                /* yup. we have a procedure start! increment the number of
                   procedures */
                cmp_nofprocs++;
            }
        } else { /* 2 pairs */
            /* is there just 1 pair? */
            if(nofpairs==1) {
                /* yup. check whether there's a MC_CHAR_LABEL at the end */
                if(line[strlen(line)-1]==MC_CHAR_LABEL) {
                    /* yup. increment the number of labels */
                    cmp_noflabels++;
                }
            } /* 1 pair */
        }

        /* check whether it is a variable definition */
        if(!strcmp(cmp_pair[0],MC_DEFINE_VAR)) {
            /* it is a variable definition! increment number of objects */
            cmp_nofobjects++;
        } /* variable definition */

        /* check whether it is an external definition */
        if(!strcmp(cmp_pair[0],MC_DEFINE_EXTERNAL)) {
            /* it is an external definition! increment number of objects */
            cmp_nofobjects++;
        } /* variable definition */

        /* check whether it is an object definition */
        if(!strcmp(cmp_pair[0],MC_DEFINE_OBJECT)) {
            /* it is an object definition! increment number of objects */
            cmp_nofobjects++;
        } /* variable definition */
    }

    /* are the procedures in the script file? */
    if(!cmp_nofprocs) {
        /* nope. die */
        fprintf(stderr," no procedures in script file defined\n");
        cmp_cleanup();
        exit(1);
    }

    /* allocate memory for the procedure buffer */
    if((cmp_proc=(MC_PROC*)malloc(cmp_nofprocs*sizeof(struct MC_PROC)))==NULL) {
        /* this failed. die */
        fprintf(stderr," unable to allocate %lu bytes\n",cmp_nofprocs*sizeof(struct MC_PROC));
        cmp_cleanup();
        exit(1);
    }
    /* zero it out */
    memset(cmp_proc,0,cmp_nofprocs*sizeof(struct MC_PROC));

    /* are there labels in the script file? */
    if(cmp_noflabels) {
        /* yup. initialize the label table */
        /* allocate memory for the label buffer */
        if((cmp_label=(MC_LABEL*)malloc(cmp_noflabels*sizeof(struct MC_LABEL)))==NULL) {
            /* this failed. die */
            fprintf(stderr," unable to allocate %lu bytes\n",cmp_noflabels*sizeof(struct MC_LABEL));
            cmp_cleanup();
            exit(1);
        }
        /* zero it out */
        memset(cmp_label,0,cmp_noflabels*sizeof(struct MC_LABEL));
    }

    /* allocate memory for the procedure buffer */
    /* add the internal objects */
    cmp_nofobjects+=MC_NOF_INTERNAL_OBJ;
    if((cmp_object=(MC_OBJECT*)malloc(cmp_nofobjects*sizeof(struct MC_OBJECT)))==NULL) {
        /* this failed. die */
        fprintf(stderr," unable to allocate %lu bytes\n",cmp_nofobjects*sizeof(struct MC_OBJECT));
        cmp_cleanup();
        exit(1);
    }
    /* zero it out */
    memset(cmp_object,0,cmp_nofobjects*sizeof(struct MC_OBJECT));

    /* first add *all* internal objects */
    cmp_add_system_objects();

    /* make the procedure and label tables */
    proc=0;label=0;stat=MC_STAT_NORMAL;
    /* scan all lines */
    for(i=1;i<cmp_noflines;i++) {
        /* create a pointer to the line */
        line=(char*)(cmp_codebuf+cmp_linetab[i]);

        /* split it */
        nofpairs=cmp_splitpairs(line);

        /* do we have normal status? */
        if(stat==MC_STAT_NORMAL) {
            /* yup. do all 'normal' thingies */
            /* when there are 2 pairs, it might be a procedure definition */
            if(nofpairs==2) {
                /* is the last pair the magic string MC_PROCSTART? */
                if(!strcmp(cmp_pair[1],MC_PROCSTART)) {
                    /* yup. we have a procedure start! is it already declared? */
                    if(cmp_findproc(cmp_pair[0])!=MC_NOPROC) {
                        /* yes. return error */
                        fprintf(stderr,"%s[%lu]: procedure '%s' already declared\n",cmp_scriptname,i,cmp_pair[0]);
                        cmp_cleanup();
                        exit(1);
                    }
                    /* is the name too long? */
                    if(strlen(cmp_pair[0])>MC_MAX_PROCNAME_LEN) {
                        /* yeh. die */
                        fprintf(stderr,"%s[%lu]: procedure name '%s' is too long, it may not exceed %lu chars\n",cmp_scriptname,i,cmp_pair[0],MC_MAX_PROCNAME_LEN);
                        cmp_cleanup();
                        exit(1);
                    }
                    /* add it! */
                    procptr=(MC_PROC*)&(cmp_proc[proc]);
                    strcpy (procptr->name,cmp_pair[0]);
                    procptr->startline=i;
                    procptr->endline=MC_NOLINE;

                    /* next procedure */
                    proc++;

                    /* check for boundries (just to make sure) */
                    if(proc>cmp_nofprocs) {
                        /* out of boundries! show error */
                        fprintf(stderr," (INTERNAL ERROR) calculation said there were %lu procedures in file, yet we were told to process procedure %lu\n",cmp_nofprocs,proc);
                        cmp_cleanup();
                        exit(0xfe);
                    }
                } else { /* is it MC_PROCSTART */
                    /* is the first pair the magic string MC_PROCEND? */
                    if(!strcmp(cmp_pair[0],MC_PROCEND)) {
                        /* yeh. find the procedure */
                        n=cmp_findproc(cmp_pair[1]);
                        /* was it ever declared? */
                        if(n==MC_NOPROC) {
                            /* nope. die */
                            fprintf(stderr," %s[%lu]: attempt to close never opened procedure '%s'\n",cmp_scriptname,i,cmp_pair[1]);
                            cmp_cleanup();
                            exit(1);
                        }
                        /* edit the procedure table */
                        cmp_proc[n].endline=i;
                    } /* is it MC_PROC_END */
                } /* is it MC_PROC_START */
            } else {
                /* is there just 1 pair? */
                if(nofpairs==1) {
                    /* yup. check whether there's a MC_CHAR_LABEL at the end */
                    if(line[strlen(line)-1]==MC_CHAR_LABEL) {
                        /* yup. get the label name */
                        for(n=0;n<strlen(line)-1;n++) {
                            cmp_temp[n]=line[n];
                        }
                        /* add the terminating nul */
                        cmp_temp[n]=0;
                        /* does it already exists? */
                        if(cmp_findlabel(cmp_temp)!=MC_NOLABEL) {
                            /* yeh. die */
                            fprintf(stderr," %s[%lu]: label '%s' already declared somewhere else\n",cmp_scriptname,i,cmp_temp);
                            cmp_cleanup();
                            exit(1);
                        }
                        /* is the name too long? */
                        if(strlen(cmp_pair[0])>MC_MAX_PROCNAME_LEN) {
                            /* yeh. die */
                            fprintf(stderr,"%s[%lu]: label name '%s' is too long, it may not exceed %lu chars\n",cmp_scriptname,i,cmp_temp,MC_MAX_LABELNAME_LEN);
                            cmp_cleanup();
                            exit(1);
                        }
                        /* add it! */
                        strcpy(cmp_label[label].name,cmp_temp);
                        cmp_label[label].line=i;
                        cmp_label[label].nofixes=0;
                        cmp_label[label].maxfixes=MC_DEFAULT_FIXESIZE;
                        if((cmp_label[label].fixes=(_ULONG*)malloc(cmp_label[label].maxfixes*sizeof(_ULONG)))==NULL) {
                            /* this failed. die */
                            fprintf(stderr," allocation of %lu bytes failed\n",cmp_label[label].maxfixes*sizeof(_ULONG));
                            cmp_cleanup();
                            exit(1);
                        }

                        /* next label */
                        label++;

                        /* check for boundries (just to make sure) */
                        if(label>cmp_noflabels) {
                            /* out of boundries! show error */
                            fprintf(stderr," (INTERNAL ERROR) calculation said there were %lu labels in the file, yet we were told to process label %lu\n",cmp_noflabels,label);
                            cmp_cleanup();
                            exit(0xfe);
                        }
                    } /* label */
                 } /* 1 pair */
            } /* 2 pairs */

            /* it might be an object declaration. is it one? */
            if(!strcmp(cmp_pair[0],MC_DEFINE_OBJECT)) {
                /* yes. handle it */
                /* copy everything before the colon to [cmp_objtemp] */
                gad_copyuntil(gad_scanlastfrombegin(strchr(line,' '),' '),cmp_objtemp,':');
                /* spaces? we *HATE* spaces. nuke 'em! */
                gad_striptrailingspaces(cmp_objtemp);

                /* figure out the superclass (the name between the : and {) */
                if((ptr=strchr(line,':'))==NULL) {
                    /* there wasn't a colon! die */
                    fprintf(stderr,"%s[%lu]: ':' expected\n",cmp_scriptname,i);
                    cmp_cleanup();
                    exit(1);
                }
                gad_copyuntil((char*)(ptr+1),cmp_supertemp,'{');
                /* spaces? we *HATE* spaces. nuke 'em! */
                gad_striptrailingspaces(cmp_supertemp);

                printf("OBJECT '%s' SUPERCLASS '%s'\n",cmp_objtemp,cmp_supertemp);

                /* change status */
                stat=MC_STAT_OBJECT;
            } /* object definition */
        } else {
            /* maybe we are in an object declaration? */
            if(stat==MC_STAT_OBJECT) {
                /* do we have an '}' ? */
                if((nofpairs==1)&&(cmp_pair[0][0]=='}')) {
                    /* yup. definition's over */
                    stat=MC_STAT_NORMAL;
                }
            } /* 'object' type */
        } /* 'normal' type */
    } /* for loop */

    /* scan all procedures to check for unclosed ones */
    for(i=0;i<cmp_nofprocs;i++) {
        /* was this one ever closed? */
        if(cmp_proc[i].endline==MC_NOLINE) {
            /* nope. die */
            fprintf(stderr,"%s[%lu]: script procedure '%s' never closed\n",cmp_scriptname,cmp_proc[i].startline,cmp_proc[i].name);
            cmp_cleanup();
            exit(1);
        }
    }

    /* check whether all procedures were processed */
    if(cmp_nofprocs!=proc) {
        /* they're not. warn the user */
        fprintf(stderr," WARNING: only %lu of the predicted %lu procedures were processed\n",proc,cmp_nofprocs);
        cmp_nofprocs=proc;
    }

    /* check whether all labels were processed */
    if(cmp_noflabels!=label) {
        /* they're not. warn the user */
        fprintf(stderr," WARNING: only %lu of the predicted %lu labels were processed\n",label,cmp_noflabels);
        cmp_noflabels=label;
    }

    /* check whether all objects were processed */
    if(cmp_nofobjects!=cmp_objcount) {
        /* they're not. warn the user */
        fprintf(stderr," WARNING: only %lu of the predicted %lu objects were processed\n",cmp_objcount,cmp_nofobjects);
        cmp_nofobjects=cmp_objcount;
    }
    if(mc_message_level==MC_MESSAGELEVEL_VERBOSE) {
        printf("> Found %lu procedure(s), %lu label(s) and %lu object(s)\n",cmp_nofprocs,cmp_noflabels,cmp_nofobjects);
    }
}

/*
 * cmp_scanobjectmethod(char* method)
 *
 * This will scan the cmp_objcmd[] table for method [method]. It will return
 * the ID of the command if it is found or MC_NOENTRY if not.
 *
 */
_UINT
cmp_scanobjectmethod(char* method) {
    _UINT i;

    /* scan them all */
    for(i=0;i<(sizeof(cmp_objcmd)/sizeof(MC_COMMAND));i++) {
        /* is this the one? */
        if(!strcmp(method,cmp_objcmd[i].name)) {
            /* yeh, return the ID */
            return i;
        }
    }
    /* it was not found. return MC_NOENTRY */
    return MC_NOENTRY;
}

/*
 * cmp_scancommand(char* command)
 *
 * This will scan the cmp_cmdtab[] table for command [method]. It will return
 * the ID of the command if it is found or MC_NOENTRY if not.
 *
 */
_UINT
cmp_scancommand(char* method) {
    _UINT i;

    /* scan them all */
    for(i=0;i<(sizeof(cmp_cmdtab)/sizeof(MC_COMMAND));i++) {
        /* is this the one? */
        if(!strcmp(method,cmp_cmdtab[i].name)) {
            /* yeh, return the ID */
            return i;
        }
    }
    /* it was not found. return MC_NOENTRY */
    return MC_NOENTRY;
}

/*
 * cmp_scanobjectproperty(char* property)
 *
 * This will scan the cmp_property[] table for property [property]. It will
 * return the ID of the property if it is found or TYPE_NORESOLV if not.
 *
 */
_UCHAR
cmp_scanobjectproperty(char* property) {
    _UINT i;

    /* scan them all */
    for(i=0;i<(sizeof(cmp_property)/sizeof(MC_PROPERTY));i++) {
        /* is this the one? */
        if(!strcmp(property,cmp_property[i].name)) {
            /* yeh, return the ID */
            return i;
        }
    }
    /* it was not found. return TYPE_NORESOLV */
    return TYPE_NORESOLV;
}

/*
 * cmp_scanspecial(char* property)
 *
 * This will scan the cmp_special[] table for special [special]. It will
 * return the ID of the special if it is found or TYPE_NORESOLV if not.
 *
 */
_UCHAR
cmp_scanspecial(char* special) {
    _UINT i;

    /* scan them all */
    for(i=0;i<(sizeof(cmp_special)/sizeof(MC_SPECIAL));i++) {
        /* is this the one? */
        if(!strcmp(special,cmp_special[i].name)) {
            /* yeh, return the ID */
            return i;
        }
    }
    /* it was not found. return TYPE_NORESOLV */
    return TYPE_NORESOLV;
}

/*
 * cmp_paramsplit(char* line,_ULONG lineno)
 *
 * This will split parameters from [line] to cmd_pair[] pairs. It will die on
 * any error. Line number [lineno] is used only for errors. It will return the
 * number of pairs split.
 *
 */
_UCHAR
cmd_paramsplit(char* line,_ULONG lineno) {
    _UINT i,n,c;

    /* if [line] has a zero size, there are no parameters */
    if(!strlen(line)) return 0;

    n=0;c=0;
    for(i=0;i<strlen(line);i++) {
        /* is it a comma? */
        if(line[i]==',') {
            /* yeh, use next pair */
            /* first terminate this pair with a nul */
            cmp_pair[n][c]=0;
            /* use next pair */
            n++;c=0;
            /* have we run out of pairs? */
            if(n>=MC_MAX_SCRIPT_PAIRS) {
                /* yeh. die */
                fprintf(stderr,"%s[%lu]: Too much parameters (%lu is the maximum)\n",cmp_scriptname,lineno,MC_MAX_SCRIPT_PAIRS);
                cmp_cleanup();
                exit(1);
            }
        } else {
            /* no, add this to the current pair */
            cmp_pair[n][c]=line[i];
            c++;
        }
    }
    /* add the nul */
    cmp_pair[n][c]=0;

    /* strip all spaces */
    for(i=0;i<=n;i++) {
        gad_striptrailingspaces(cmp_pair[i]);
    }
    return (n+1);
}

/*
 * cmp_add_cscript_byte(_UCHAR ch)
 *
 * This will add byte [ch] to the compiled script. It will resize the
 * compiled script buffer if needed.
 *
 */
void
cmp_add_cscript_byte(_UCHAR ch) {
    char* ptr;
    _ULONG i;

    /* add it */
    cmp_comp_script[cmp_comp_script_size]=ch;

    /* increment pointer */
    cmp_comp_script_size++;

    /* need to resize? */
    if(cmp_comp_script_size>=cmp_comp_script_bufsize) {
        /* yeah, do it */
        /* allocate a new buffer */
        i=cmp_comp_script_bufsize+MC_DEFAULT_SCRIPTSIZE;
        if((ptr=(char*)malloc(i))==NULL) {
            /* this failed. die */
            fprintf(stderr,"unable to allocate %lu bytes of memory for buffer change\n",i);
            cmp_cleanup();
            exit(0xfe);
        }
        /* copy the current buffer to the new one */
        memcpy(ptr,cmp_comp_script,cmp_comp_script_size);

        /* free the old buffer */
        free(cmp_comp_script);

        /* if debugging level is verbose, notify user of it */
        if(mc_message_level==MC_MESSAGELEVEL_VERBOSE) {
            printf("+++ Script buffer resized from %lu to %lu bytes +++\n",cmp_comp_script_bufsize,i);
        }

        /* select the new one */
        cmp_comp_script=ptr;
        cmp_comp_script_size=i;
    }
}

/*
 * cmp_add_cscript_word(_UINT ch)
 *
 * This will add word [ch] to the compiled script. It will resize the
 * compiled script buffer if needed.
 *
 */
void
cmp_add_cscript_word(_UINT ch) {
    /* first add the lo byte */
    cmp_add_cscript_byte((_UCHAR)(ch&0xff));
    /* first add the hi byte */
    cmp_add_cscript_byte((_UCHAR)(ch>>8));
}

/*
 * cmp_add_cscript_dword(_ULONG ch)
 *
 * This will add double word [ch] to the compiled script. It will resize the
 * compiled script buffer if needed.
 *
 */
void
cmp_add_cscript_dword(_ULONG ch) {
    /* first add the lo word */
    cmp_add_cscript_word((_UINT)(ch&0xffff));
    /* first add the hi byte */
    cmp_add_cscript_word((_UINT)(ch>>16));
}

/*
 * cmp_add_cscript_objref(_UINT objno)
 *
 * This will add the current offset as an object change offset for object
 * [objno]. It will also write a junk value to it, that will be replaced by
 * the MAD Linker.
 *
 */
void
cmp_add_cscript_objref(_UINT objno) {
    _ULONG*    ptr;
    _ULONG     i;
    MC_OBJECT* obj;

    /* add a mumbo-jumbo word */
    cmp_add_cscript_word((_UINT)MC_DEFAULT_OBJNO);

    /* get a pointer to the object, to improve readability :-) */
    obj=(MC_OBJECT*)&cmp_object[objno];

    /* add a nonsense number (will be changed by the linker) */
    obj->change[obj->curchangeofs]=cmp_comp_script_size;

    /* increment pointer */
    obj->curchangeofs++;

    /* need to resize? */
    if(obj->curchangeofs>=obj->maxchanges) {
        /* yeah, do it */
        /* allocate a new buffer */
        i=obj->curchangeofs+(MC_DEFAULT_CHANGESIZE*sizeof(_ULONG));
        if((ptr=(_ULONG*)malloc(i))==NULL) {
            /* this failed. die */
            fprintf(stderr,"unable to allocate %lu bytes of memory for buffer change\n",i);
            cmp_cleanup();
            exit(0xfe);
        }
        /* copy the current buffer to the new one */
        memcpy(ptr,obj->change,obj->curchangeofs);

        /* free the old buffer */
        free(obj->change);

        /* if debugging level is verbose, notify user of it */
        if(mc_message_level==MC_MESSAGELEVEL_VERBOSE) {
            printf("+++ Object change buffer resized from %lu to %lu bytes +++\n",obj->curchangeofs*sizeof(_ULONG),i);
        }

        /* select the new one */
        obj->change=ptr;
        obj->maxchanges=i;
    }
}

/*
 * cmp_add_cscript_labelref(_ULONG labelno)
 *
 * This will add the current offset as a label changed offset for label
 * [labelno]. It will also write a junk value to it, that will be replaced by
 * another step.
 *
 */
void
cmp_add_cscript_labelref(_UINT labelno) {
    _ULONG*    ptr;
    _ULONG     i;
    MC_LABEL*  label;

    /* add a mumbo-jumbo dword*/
    cmp_add_cscript_dword(MC_DEFAULT_LABELOFS);

    /* get a pointer to the label, to improve readability :-) */
    label=(MC_LABEL*)&cmp_label[labelno];

    /* add a nonsense number (will be changed by the linker) */
    label->fixes[label->nofixes]=cmp_comp_script_size;

    /* increment pointer */
    label->nofixes++;

    /* need to resize? */
    if(label->nofixes>=label->maxfixes) {
        /* yeah, do it */
        /* allocate a new buffer */
        i=label->nofixes+(MC_DEFAULT_FIXESIZE*sizeof(_ULONG));
        if((ptr=(_ULONG*)malloc(i))==NULL) {
            /* this failed. die */
            fprintf(stderr,"unable to allocate %lu bytes of memory for buffer change\n",i);
            cmp_cleanup();
            exit(0xfe);
        }
        /* copy the current buffer to the new one */
        memcpy(ptr,label->fixes,label->nofixes);

        /* free the old buffer */
        free(label->fixes);

        /* if debugging level is verbose, notify user of it */
        if(mc_message_level==MC_MESSAGELEVEL_VERBOSE) {
            printf("+++ Label change buffer resized from %lu to %lu bytes +++\n",label->nofixes*sizeof(_ULONG),i);
        }

        /* select the new one */
        label->fixes=ptr;
        label->maxfixes=i;
    }
}
/*
 * cmp_add_cscript_string(char* string)
 *
 * This will add string [string] to the compiled script. It will resize the
 * compiled script buffer if needed.
 *
 */
void
cmp_add_cscript_string(char* string) {
    _UINT i;

    /* get the length */
    i=strlen(string);

    /* if the string is longer than MC_MAX_STRINGLEN, die */
    if(i>MC_MAX_STRINGLEN) {
        /* it is. die */
        fprintf(stderr,"Cannot add strings which are longer than %lu chars\n",MC_MAX_STRINGLEN);
        cmp_cleanup();
        exit(1);
    }

    /* add the length byte */
    cmp_add_cscript_word(i);

    /* add the chars */
    for(i=0;i<strlen(string);i++) {
        /* add each byte */
        cmp_add_cscript_byte(string[i]);
    }
}

/*
 * cmp_evaluate_objexpr(char* value,_ULONG lineno)
 *
 * This will add object expression [value] to the script. It expects to be at
 * line [lineno]. It will add it directly to the script code.
 *
 */
void
cmp_evaluate_objexpr(char* value,_ULONG lineno) {
    char*  ptr;
    char   cmp_objnametmp[MC_MAX_PAIR_LENGTH];
    char   cmp_proptmp[MC_MAX_PAIR_LENGTH];
    _UCHAR propno;
    _ULONG objectno;

    /* make a pointer to the dot */
    if((ptr=strchr(value,'.'))==NULL) {
        /* there wasn't a dot! die */
        fprintf(stderr,"%s[%lu]: (INTERNAL ERROR) cmp_evaluate_objexpr() called without a object.method pair\n",cmp_scriptname,lineno);
        cmp_cleanup();
        exit(0xfe);
    }

    /* copy everything before the dot to [cmp_objnametmp] */
    gad_copyuntil(value,cmp_objnametmp,'.');
    /* and copy everything after the dot to [cmp_methodtmp] */
    gad_copyuntil((char*)(ptr+1),cmp_proptmp,'\0');
    /* nuke *all* spaces */
    gad_striptrailingspaces(cmp_objnametmp);
    gad_striptrailingspaces(cmp_proptmp);

    /* look up the object id */
    objectno=cmp_findobject(cmp_objnametmp);

    /* did this work? */
    if(objectno==MC_NOBJECT) {
        /* no. die */
        fprintf(stderr,"%s[%lu]: unknown object '%s'\n",cmp_scriptname,lineno,cmp_objnametmp);
        cmp_cleanup();
        exit(1);
    }

    /* look up the method */
    propno=cmp_scanobjectproperty(cmp_proptmp);
    /* did this work? */
    if(propno==TYPE_NORESOLV) {
        /* no. die */
        fprintf(stderr,"%s[%lu]: unknown object property '%s'\n",cmp_scriptname,lineno,cmp_proptmp);
        cmp_cleanup();
        exit(1);
    }

    /* add the object reference */
    cmp_add_cscript_objref(objectno);

    /* and add the property number */
    cmp_add_cscript_byte(cmp_property[propno].code);
}

/*
 * cmp_evaluate_special(char* value,_ULONG lineno)
 *
 * This will add special expression [value] to the script. It expects to be
 * at line [lineno]. It will add it directly to the script code. It must be
 * in the form xxx[yyy].
 *
 */
void
cmp_evaluate_special(char* value,_ULONG lineno) {
    char*  ptr;
    char*  ptr2;
    char   cmp_specialname[MC_MAX_PAIR_LENGTH];
    char   cmp_specialvalue[MC_MAX_PAIR_LENGTH];
    _UCHAR specialno;
    _UINT  paramtype;

    /* make a pointer to the open bracket */
    if((ptr=strchr(value,'['))==NULL) {
        /* there wasn't a bracket! die */
        fprintf(stderr,"%s[%lu]: (INTERNAL ERROR) cmp_evaluate_special() called without a method[value]] pair\n",cmp_scriptname,lineno);
        cmp_cleanup();
        exit(0xfe);
    }

    /* scan for the bracket close */
    if((ptr2=strchr(value,']'))==NULL) {
        /* it wasn't found. die */
        fprintf(stderr,"%s[%lu]: ']' expected\n",cmp_scriptname,lineno);
        cmp_cleanup();
        exit(1);
    }
    /* copy everything until the [ to [cmp_specialname] */
    gad_copyuntil(value,cmp_specialname,'[');
    /* nuke all spaces */
    gad_striptrailingspaces(cmp_specialname);

    /* copy everything between the []'s to [cmp_specialvalue] */
    gad_copyuntil((char*)(ptr+1),cmp_specialvalue,']');
    /* nuke all spaces */
    gad_striptrailingspaces(cmp_specialvalue);

    /* look up the special */
    specialno=cmp_scanspecial(cmp_specialname);
    /* did this work? */
    if(specialno==TYPE_NORESOLV) {
        /* no. die */
        fprintf(stderr,"%s[%lu]: unknown object special '%s'\n",cmp_scriptname,lineno,cmp_specialname);
        cmp_cleanup();
        exit(1);
    }

    /* add the special flag */
    cmp_add_cscript_byte(TYPE_SPECIAL_RANDOM);

    /* resolve the parameter string */
    paramtype=gad_resolveparamstring(cmp_special[specialno].parms);

    /* is the type known? */
    if(paramtype==MC_TYPE_UNKNOWN) {
        /* nope. die */
        fprintf(stderr,"%s[%lu]: (INTERNAL ERROR) special string '%s' not known for method '%s'\n",cmp_scriptname,lineno,cmp_special[specialno].parms,cmp_special[specialno].name);
        cmp_cleanup();
        exit(0xfe);
    }

    /* add the parameter */
    cmp_add_param(paramtype,cmp_specialvalue,lineno);
}

/*
 * cmp_evaluate_var(char* value,_ULONG lineno)
 *
 * This will add variable [value] to the script. It expects to be at line
 * [lineno]. It will add it directly to the script code.
 *
 */
void
cmp_evaluate_var(char* value,_ULONG lineno) {
    _ULONG objno,no;
    char*  ptr;

    /* is there a dot in the value? */
    if(strchr(value,'.')!=NULL) {
        /* yes, pass it over to the add object expression routine */
        cmp_evaluate_objexpr(value,lineno);
        return;
    }

    /* does it have a [? */
    if(strchr(value,'[')!=NULL) {
        /* yes, pass it over to the special routine */
        cmp_evaluate_special(value,lineno);
        return;
    }

    /* is it a variable? */
    if((objno=cmp_findobject(value))!=MC_NOBJECT) {
        /* well, it's an object. maybe a variable? */
        if(cmp_object[objno].type==OBJMAN_TYPE_VARIABLE) {
            /* yes! add it as a variable */
            cmp_add_cscript_byte(TYPE_VARIABLE);

            /* add the object reference */
            cmp_add_cscript_objref(objno);

            /* get outta here! */
            return;
        }
    }

    /* it might be a static number then */
    no=strtoul(value,&ptr,0);
    if(ptr==value) {
        /* this failed. die */
        fprintf(stderr,"%s[%lu]: cannot parse '%s'\n",cmp_scriptname,lineno,value);
        cmp_cleanup();
        exit(1);
    }

    /* add it as a static nymber */
    cmp_add_cscript_byte(TYPE_STATIC);

    /* and the value itself */
    cmp_add_cscript_dword(no);
}

/*
 * cmp_evaluate_expr(char* value,_ULONG lineno)
 *
 * This will evaluate expression [value]. It expects to be at line [lineno].
 * It will add it directly to the script code.
 *
 */
void
cmp_evaluate_expr(char* value,_ULONG lineno) {
    char*  ptr;
    char*  temptr;
    char   cmp_fullthing[MC_MAX_PAIR_LENGTH];
    char   cmp_objnametmp[MC_MAX_PAIR_LENGTH];
    char   cmp_methodtmp[MC_MAX_PAIR_LENGTH];
    _UINT  methodno,nofparms;
    _UCHAR expr;
    _ULONG objectno;

    /* first put it all into [cmp_temp], ignoring spaces */
    gad_copynochar(value,cmp_temp,' ');
    temptr=cmp_temp;

    /* keep scan for an operator and adding them */
    while (((ptr=gad_scanforoperator(temptr,&expr))!=NULL)) {
        /* there's an operator in the string. copy everything before it */
        strncpy(cmp_fullthing,temptr,(ptr-temptr));
        cmp_fullthing[(ptr-temptr)]=0;

        /* nuke spaces */
        gad_striptrailingspaces(cmp_fullthing);

        /* is the length zero? */
        if(!strlen(cmp_fullthing)) {
            /* yes. give error */
            fprintf(stderr,"%s[%lu]: parameter expected\n",cmp_scriptname,lineno);
            cmp_cleanup();
            exit(1);
        }

        /* evaluate the variable */
        cmp_evaluate_var(cmp_fullthing,lineno);

        /* next */
        temptr=(ptr+1);
    }

    /* is the length zero? */
    if(!strlen(temptr)) {
        /* yes. give error */
        fprintf(stderr,"%s[%lu]: parameter expected\n",cmp_scriptname,lineno);
        cmp_cleanup();
        exit(1);
    }

    /* do the last thingy */
    cmp_evaluate_var(temptr,lineno);
}

/*
 * cmp_add_param(_UINT parmtype,char* param,_ULONG lineno);
 *
 * This will add parameter [param], as type [paramtype]. It expects to be at
 * line [lineno].
 *
 */
void
cmp_add_param(_UINT parmtype,char* param,_ULONG lineno) {
    char temp[MC_MAX_PAIR_LENGTH];
    char* ptr;
    _ULONG i;
    _UCHAR c;

    /* figure out the parameter type */
    switch(parmtype) {
        case MC_TYPE_STRING: /* string */
                             cmp_add_cscript_string(param);
                             break;
       case MC_TYPE_QSTRING: /* string with quotes */
                             /* get the string without quotes */
                             gad_copynochar(temp,param,' ');

                             /* add the result */
                             cmp_add_cscript_string(temp);
                             break;
          case MC_TYPE_UINT: /* unsigned integer */
          case MC_TYPE_SINT: /* signed integer */
                             i=strtoul(param,&ptr,0);

                             /* did it work? */
                             if(ptr==param) {
                                 /* no. give error */
                                 fprintf(stderr,"%s[%lu]: cannot parse integer '%s'\n",cmp_scriptname,lineno,param);
                                 cmp_cleanup();
                                 exit(1);
                             }

                             /* add it to the script */
                             cmp_add_cscript_word((_UINT)i);
                             break;
         case MC_TYPE_EUINT: /* evaluated integer */
                             cmp_evaluate_expr(param,lineno);
                             break;
     case MC_TYPE_DIRECTION: /* direction */
                             /* try to resolve the direction */
                             if((c=gad_resolvedirection(param))==DIR_UNKNOWN) {
                                 /* this failed. die */
                                 fprintf(stderr,"%s[%lu]: cannot parse direction '%s'\n",cmp_scriptname,lineno,param);
                                 cmp_cleanup();
                                 exit(1);
                             }

                             /* add it to the script */
                             cmp_add_cscript_byte(c);
                             break;
         case MC_TYPE_LABEL: /* label */
                             /* find the label */
                             if((i=cmp_findlabel(param))==MC_NOLABEL) {
                                 /* this failed. die */
                                 fprintf(stderr,"%s[%lu]: undefined label '%s'\n",cmp_scriptname,lineno,param);
                                 cmp_cleanup();
                                 exit(1);
                             }

                             /* add it to the script */
                             cmp_add_cscript_labelref(i);
                             break;
                    default: /* we don't know this type. die */
                             fprintf(stderr,"%s[%lu]: INTERNAL ERROR: unknown parameter type %u\n",cmp_scriptname,lineno,parmtype);
                             cmp_cleanup();
                             exit(0xfe);
   }
}

/*
 * cmp_add_params(MC_COMMAND* cmd,_UINT nofparms)
 *
 * This will add parameters according to the information in [cmd]. It will use
 * cmp_pair[] as parameters. It expects to get the number of parameters given
 * in [nofparms]. [lineno] is expected to be the line number.
 *
 */
void
cmp_add_params(MC_COMMAND* cmd,_UINT nofparams,_ULONG lineno) {
    _ULONG i;
    _UINT paramno;
    _UINT paramtype;

    /* if there are no parameters, just get out */
    if(cmd->parms==NULL) return;

    /* do all parameters one by one */
    paramno=0;
    for(i=0;i<strlen(cmd->parms);i++) {
        /* get the thingy */
        sscanf((char*)(cmd->parms+i),"%s",&cmp_temp);
        i+=strlen(cmp_temp);

        /* does this parameter exist? */
        if(paramno>=nofparams) {
            /* nope. die */
            fprintf(stderr,"%s[%lu]: parameter expected\n",cmp_scriptname,lineno);
            cmp_cleanup();
            exit(1);
        }

        /* resolve the parameter string */
        paramtype=gad_resolveparamstring(cmp_temp);

        /* is the type known? */
        if(paramtype==MC_TYPE_UNKNOWN) {
            /* nope. die */
            fprintf(stderr,"%s[%lu]: (INTERNAL ERROR) parameter string '%s' not known for method '%s'\n",cmp_scriptname,lineno,cmp_temp,cmd->name);
            cmp_cleanup();
            exit(0xfe);
        }

        /* add the parameter */
        cmp_add_param(paramtype,cmp_pair[paramno],lineno);

        /* next parameter */
        paramno++;
    }
}

/*
 * cmp_dobjectopcode(char* line,_ULONG lineno)
 *
 * This will compile line [line]. It expects [lineno] to be the line's number,
 * of a line that is like <object>.<method>. It will return the number of
 * lines to skip. It will die on any error.
 */
_ULONG
cmp_dobjectopcode(char* line,_ULONG lineno) {
    char*  ptr;
    char   cmp_objnametmp[MC_MAX_PAIR_LENGTH];
    char   cmp_methodtmp[MC_MAX_PAIR_LENGTH];
    _UINT  methodno,nofparms;
    _ULONG objectno;

    /* split it */
    cmp_splitpairs(line);

    /* cmp_pair[0] is in the form <object>.<method>. Split it */
    if((ptr=strchr(cmp_pair[0],'.'))==NULL) {
        /* there wasn't a dot! die */
        fprintf(stderr,"%s[%lu]: (INTERNAL ERROR) cmp_dobjectopcode() called without a object.method pair\n",cmp_scriptname,lineno);
        cmp_cleanup();
        exit(0xfe);
    }
    /* copy everything before the dot to [cmp_objnametmp] */
    gad_copyuntil(line,cmp_objnametmp,'.');
    /* and copy everything after the dot to [cmp_methodtmp] */
    gad_copyuntil((char*)(ptr+1),cmp_methodtmp,'\0');
    /* nuke *all* spaces */
    gad_striptrailingspaces(cmp_objnametmp);
    gad_striptrailingspaces(cmp_methodtmp);

    /* look up the object id */
    objectno=cmp_findobject(cmp_objnametmp);
    /* did this work? */
    if(objectno==MC_NOBJECT) {
        /* no. die */
        fprintf(stderr,"%s[%lu]: unknown object '%s'\n",cmp_scriptname,lineno,cmp_objnametmp);
        cmp_cleanup();
        exit(1);
    }

    /* look up the method */
    methodno=cmp_scanobjectmethod(cmp_methodtmp);
    /* did this work? */
    if(methodno==MC_NOENTRY) {
        /* no. die */
        fprintf(stderr,"%s[%lu]: unknown object method '%s'\n",cmp_scriptname,lineno,cmp_methodtmp);
        cmp_cleanup();
        exit(1);
    }

    /* add the opcode */
    cmp_add_cscript_byte(OPCODE_OBJECT);

    /* add the object reference */
    cmp_add_cscript_objref(objectno);

    /* split it after the first space */
    strcpy(cmp_temp,line);
    gad_striptrailingspaces(cmp_temp);
    nofparms=cmd_paramsplit((char*)(gad_scanlastfrombegin(strchr(cmp_temp,' '),' ')),lineno);

//    printf("\n%s->%s => ",cmp_objnametmp,cmp_methodtmp);

    /* add the parameters */
    cmp_add_params(&cmp_objcmd[methodno],nofparms,lineno);

    return 0;
}

/*
 * cmp_dobjectlessopcode(char* line,_ULONG lineno)
 *
 * This will compile line [line]. It expects [lineno] to be the line's number,
 * of a line that is without anything special. It will return non-zero if it
 * was compiled, or non-zero if it wasn't.
 */
_UCHAR
cmp_dobjectlessopcode(char* line,_ULONG lineno) {
    char*  ptr;
    _UINT  cmdno,nofparms;
    _ULONG objectno;

    /* split it */
    cmp_splitpairs(line);

    /* look up the method */
    cmdno=cmp_scancommand(cmp_pair[0]);
    /* did this work? */
    if(cmdno==MC_NOENTRY) {
        /* no. leave */
        return 0;
    }

    /* add the opcode */
    cmp_add_cscript_byte(cmp_cmdtab[cmdno].opcode);

    /* split it after the first space */
    strcpy(cmp_temp,line);
    gad_striptrailingspaces(cmp_temp);
    nofparms=cmd_paramsplit((char*)(gad_scanlastfrombegin(strchr(cmp_temp,' '),' ')),lineno);

    /* add the parameters */
    cmp_add_params(&cmp_cmdtab[cmdno],nofparms,lineno);

    return 1;
}

/*
 * cmp_doassignment(char* line,_ULONG lineno)
 *
 * This will compile line [line]. It expects [lineno] to be an assigment line
 * in the form [foo]=[bar]. It will return non-zero if successful, otherwise
 * zero. It will die on any error.
 */
_UCHAR
cmp_doassignment(char* line,_ULONG lineno) {
    char*  ptr;
    char   cmp_objnametmp[MC_MAX_PAIR_LENGTH];
    char   cmp_methodtmp[MC_MAX_PAIR_LENGTH];
    _UINT  methodno,nofparms;
    _ULONG objectno;

    /* put it in [cmp_temp], nuking all spaces */
    gad_copynochar(line,cmp_temp,' ');

    /* [cmp_temp] is in the form [foo]=[bar]. Split it */
    if((ptr=strchr(cmp_temp,'='))==NULL) {
        /* there wasn't an equal sign! die */
        fprintf(stderr,"%s[%lu]: (INTERNAL ERROR) cmp_doassignment() called without a foo=bar pair\n",cmp_scriptname,lineno);
        cmp_cleanup();
        exit(0xfe);
    }
    /* copy everything before the equal sign to [cmp_objnametmp] */
    gad_copyuntil(cmp_temp,cmp_objnametmp,'=');
    /* and copy everything after the equal sign to [cmp_methodtmp] */
    gad_copyuntil((char*)(ptr+1),cmp_methodtmp,'\0');

    /* look up the object id */
    objectno=cmp_findobject(cmp_objnametmp);
    /* did this work? */
    if(objectno==MC_NOBJECT) {
        /* well, there's an object, but is it a variable? */
        if(cmp_object[objectno].type!=OBJMAN_TYPE_VARIABLE) {
            /* no. just get out */
            return 0;
        }
    }

    /* add the opcode */
    cmp_add_cscript_byte(OPCODE_SETVAR);

    /* add the object reference */
    cmp_add_cscript_objref(objectno);

    return 1;
}

/*
 * cmp_compileline(char* line,_ULONG lineno)
 *
 * This will compile line [line]. It expects [lineno] to be the line's number.
 * It will return the number of lines to skip. It will die on any error.
 *
 */
_ULONG
cmp_compileline(char* line,_ULONG lineno) {
    /* split it */
    cmp_splitpairs(line);

    /* if it begins with a semi-colon, just skip it */
    if(cmp_pair[0][0]==';') return 0;

    /* if there's a label here, ignore it */
    if(cmp_labelhere(lineno)!=MC_NOLABEL) return 0;

    /* if it's a blank line, ignore it */
    if(!strlen(cmp_pair[0])) return 0;

    /* if it's a declaration, ignore it */
    if(!strcmp(cmp_pair[0],MC_DEFINE_OBJECT)) return 0;
    if(!strcmp(cmp_pair[0],MC_DEFINE_VAR)) return 0;
    if(!strcmp(cmp_pair[0],MC_DEFINE_EXTERNAL)) return 0;

    /* is there a dot in the first pair? */
    if((strchr(cmp_pair[0],'.'))!=NULL) {
        /* yeh, it is an object opcode. take care of that */
        return (cmp_dobjectopcode(line,lineno));
    }

    /* is there an equal sign in it? */
    if((strchr(cmp_pair[0],'='))!=NULL) {
        /* yeh, it is an object assignment. take care of that */
        if (cmp_doassignment(line,lineno)) return 0;
    }

    /* try to compile it as a normal line */
    if(cmp_dobjectlessopcode(line,lineno)) {
       /* this worked! leave */
       return 0;
    }

    return 0;
}

/*
 * cmp_compileproc(_ULONG procno)
 *
 * This will compile procedure [procno].
 *
 */
void
cmp_compileproc(_ULONG procno) {
    _ULONG i;

    /* show messages if not in silent mode */
    if(mc_message_level!=MC_MESSAGELEVEL_SILENT) {
        printf("Compiling procedure '%s'...",cmp_proc[procno].name);
    }

    i=cmp_proc[procno].startline+1;
    while(i<cmp_proc[procno].endline) {
        /* compile it line by line and add the number of lines to skip */
        i+=(cmp_compileline((char*)(cmp_codebuf+cmp_linetab[i]),i)+1);
    }

    /* show messages if not in silent mode */
    if(mc_message_level!=MC_MESSAGELEVEL_SILENT) {
        printf(" done\n");
    }
}

/*
 * cmp_writescript(char* fname)
 *
 * This will write the compiled script to [fname].
 *
 */
void
cmp_writescript(char* fname) {
    FILE* f;

    /* open the destination file */
    if((f=fopen(fname,"wb"))==NULL) {
        /* nope, this failed. die */
        cmp_cleanup();
        fprintf(stderr,"unable to open script output file %s\n",fname);
        exit(1);
    }

    /* dump the script buffer to the file */
    if(!fwrite(cmp_comp_script,cmp_comp_script_size,1,f)) {
        /* this failed. die */
        cmp_cleanup();
        fprintf(stderr,"unable to write script output file %s\n",fname);
        exit(1);
    }

    /* close the file */
    fclose(f);
}

/*
 * cmp_compilefile(char* fname)
 *
 * This will compile file [fname]. It will quit on any error.
 *
 */
void
cmp_compilefile(char* fname) {
    _ULONG i;
    char destname[MC_MAX_SCRIPTNAME_LEN];

    /* is the script name too long? */
    if(strlen(fname)>MC_MAX_SCRIPTNAME_LEN) {
        /* yeh. die */
        fprintf(stderr,"script file name '%s' is too long, it may not exceed %lu chars\n",MC_MAX_SCRIPTNAME_LEN);
        exit(1);
    }
    /* copy it */
    strcpy(cmp_scriptname,fname);

    /* load the file */
    cmp_loadfile(fname,&cmp_codebuflen,&cmp_codebuf,&cmp_noflines,&cmp_linetab,&cmp_comp_script_bufsize,&cmp_comp_script);

    /* preprocess the script */
    cmp_preprocess(&cmp_codebuflen,&cmp_codebuf,&cmp_noflines,&cmp_linetab,&cmp_comp_script_bufsize,&cmp_comp_script);

    /* compile all procedures one by one */
    for(i=0;i<cmp_nofprocs;i++) {
        cmp_compileproc(i);
    }

    /* construct the output file */
    gad_getbasename(fname,destname);

    /* add the correct extension */
    strcat(destname,MC_EXT_CM);

    /* write the compiled script */
    cmp_writescript(destname);
}
