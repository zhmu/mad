#define MAD_COMPILER
#define RHIDE_COMPITABLE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "gadgets.h"
#include "../../../source/objman.h"
#include "../../../source/script.h"
#include "../../../source/types.h"

#define MAX_COMMANDS 63
#define MAX_FILE_NAME_LEN 64
#define MAX_SCRIPT_FILE_NAME_LEN 14

#define MAX_SCRIPT_STRINGS 6

#define MAX_SCRIPT_LINE_LEN 512
#define MAX_IDENT_LEN  32
#define MAX_PROCS      32
#define MAX_VARS       32
#define MAX_LABELS     32
#define MAX_OBJECTS    63
#define MAX_LABEL_TABS 63

#define TYPE_ANM      OBJMAN_TYPE_ANM
#define TYPE_VAR      0x80
#define TYPE_TEXT     OBJMAN_TYPE_TEXT

#define SCRIPT_MAX_COMPILED_SIZE 16384

#define SCRIPT_STARTPROC "{"
#define SCRIPT_ENDPROC "}"
#define SCRIPT_DEFINEVAR        "var"           /* define variable */
#define SCRIPT_JUMP "goto"                      /* jump in the script */

#define SCRIPT_NOIDENT 0xffff
#define SCRIPT_NOLINE 0xffff

#define MAX_GLOBAL_OBJ_COUNT    128

#define GFX_LOADANM "loadanm"
#define GFX_TEXT "text"

#define MOUSE_OBJECTNAME "mouse"         /* mouse object name */
#define BAR_OBJECTNAME   "bar"           /* bar object name */
#define CONTROLS_OBJECTNAME "controls"   /* controls object name */
#define SYSTEM_OBJECTNAME "system"       /* system object name */
#define SCRIPT_OBJECTNAME "script"       /* script object name */

#define SCRIPT_EXPR_X           "x"             /* x coordinate of object */
#define SCRIPT_EXPR_Y           "y"             /* y coordinate of object */
#define SCRIPT_EXPR_FRAMENO      "frameno"       /* get current frame number */
#define SCRIPT_EXPR_MOVING       "moving"        /* is animation currently moving? */
#define SCRIPT_EXPR_ANIMATING    "animating"     /* is animation currently animating? */
#define SCRIPT_EXPR_PRIORITYCODE "prioritycode"  /* get current priority code */

char *scriptdata;
_ULONG scriptsize;
char s[MAX_SCRIPT_LINE_LEN];
char script_filename[MAX_SCRIPT_FILE_NAME_LEN];

struct PROCEDURE {
    char name[MAX_IDENT_LEN];
    _UINT startline,endline;
    _UINT bytestart,bytend;
};

struct LABEL {
    char name[MAX_IDENT_LEN];
    _UINT line;
    _UINT offset;
};

struct COMMAND {
    char  *name;
    _UCHAR nofparms;
    _UCHAR opcode;
    _UCHAR type;
    _UCHAR strip_quotes;
};

struct OBJLOOKUP {
    char name[MAX_IDENT_LEN];
    _UCHAR offsetcount;
    _UCHAR type,internal,external;
    _UINT offset[MAX_GLOBAL_OBJ_COUNT];
    char filename[MAX_SCRIPT_FILE_NAME_LEN];
};

struct LABELTAB {
    _UINT labelno;
    _UINT offset;
};

#define COMMAND_ASSIGN        "set"
#define COMMAND_EXTERN        "extern"
#define COMMAND_IF            "if"
#define COMMAND_WHILE         "while"
#define EXTERN_OBJ            "object"

struct COMMANDTAB {
    _UINT nofcommands;
    struct COMMAND command[MAX_COMMANDS];
};

struct COMMANDTAB command[] = { 5, { "loadscreen",     1, OPCODE_LOADSCREEN,     0, 1,
                                     "refresh",        0, OPCODE_REFRESH,        0, 0,
                                     "return",         0, OPCODE_RETURN,         0, 0,
                                     "delay",          1, OPCODE_DELAY,          1, 0,
                                     "leave",          0, OPCODE_LEAVE,          0, 0 } };

struct COMMANDTAB objcmdtab[] = { 31, { "setanim",    1, OPCODE_OBJ_SETANIM,    0, 1,
                                        "setframe",   1, OPCODE_OBJ_SETFRAME,   1, 0,
                                        "setmove",    1, OPCODE_OBJ_SETMOVE,    0, 1,
                                        "moveto",     2, OPCODE_OBJ_MOVETO,     4, 0,
                                        "setspeed",   3, OPCODE_OBJ_SETSPEED,   1, 0,
                                        "sethalt",    1, OPCODE_OBJ_SETHALT,    0, 1,
                                        "setloop",    1, OPCODE_OBJ_SETLOOP,    1, 0,
                                        "setclip",    1, OPCODE_OBJ_SETCLIP,    3, 0,
                                        "stop",       0, OPCODE_OBJ_STOP,       0, 0,
                                        "move",       2, OPCODE_OBJ_MOVE,       4, 0,
                                        "loadsprite", 1, OPCODE_OBJ_LOADSPR,    0, 1,
                                        "enable",     0, OPCODE_OBJ_ENABLE,     0, 0,
                                        "disable",    0, OPCODE_OBJ_DISABLE,    0, 0,
                                        "face",       1, OPCODE_OBJ_FACE,       2, 0,
                                        "animate",    0, OPCODE_OBJ_ANIMATE,    0, 0,
                                        "show",       0, OPCODE_OBJ_SHOW,       0, 0,
                                        "hide",       0, OPCODE_OBJ_HIDE,       0, 0,
                                        "setmask",    1, OPCODE_OBJ_SETMASK,    2, 0,
                                        "unload",     0, OPCODE_OBJ_UNLOAD,     0, 0,
                                        "exit",       0, OPCODE_OBJ_EXIT,       0, 0,
                                        "addicon",    4, OPCODE_OBJ_ADDICON,    0, 1,
                                        "load",       1, OPCODE_OBJ_LOAD,       0, 1,
                                        "go",         0, OPCODE_OBJ_GO,         0, 0,
                                        "dispose",    0, OPCODE_OBJ_DISPOSE,    0, 0,
                                        "setmoveobj", 0, OPCODE_OBJ_SETMOVEOBJ, 0, 0,
                                        "setpriority",1, OPCODE_OBJ_SETPRI,     1, 0,
                                        "darkenpal",  1, OPCODE_OBJ_DARKENPAL,  1, 0,
                                        "restorepal", 0, OPCODE_OBJ_RESTOREPAL, 0, 0,
                                        "settext",    1, OPCODE_OBJ_SETTEXT,    5, 0,
                                        "setcolor",   1, OPCODE_OBJ_SETCOLOR,   4, 0,
                                        "promptexit", 0, OPCODE_OBJ_PROMPTEXIT, 0, 0 } };

struct PROCEDURE proc[MAX_PROCS];
struct LABEL label[MAX_LABELS];
struct OBJLOOKUP object[MAX_OBJECTS];
struct LABELTAB labeltab[MAX_LABEL_TABS];

char map_filename[MAX_FILE_NAME_LEN];
char out_filename[MAX_FILE_NAME_LEN];
char loc_filename[MAX_FILE_NAME_LEN];

char script[SCRIPT_MAX_COMPILED_SIZE];

_UINT nofprocs,nofvars,noflabels,nofobjs,noflabeltabs;
_UINT script_pointer;

_UINT
findproc(char *name) {
    _UINT i;

    /* trace all procedures */
    for(i=0;i<nofprocs;i++) {
        /* if they match, return the number */
        if (!strcmp(name,proc[i].name)) return i;
    }
    /* no matches found, so return SCRIPT_NOIDENT */
    return SCRIPT_NOIDENT;
}

_UINT
findlabel(char *name) {
    _UINT i;

    /* trace all labels */
    for(i=0;i<noflabels;i++) {
        /* if they match, return the number */
        if (!strcmp(name,label[i].name)) return i;
    }
    /* no matches found, so return SCRIPT_NOIDENT */
    return SCRIPT_NOIDENT;
}

_UINT
lookupcommand(char *name) {
    _UINT i;

    /* trace all commands */
    for(i=0;i<command->nofcommands;i++) {
        /* if they match, return the number */
        if (!strcmp(name,command->command[i].name)) return i;
    }
    /* no matches found, so return SCRIPT_NOIDENT */
    return SCRIPT_NOIDENT;
}

_UINT
lookup_object_command(char *name) {
    _UINT i;

    /* trace all object commands */
    for(i=0;i<objcmdtab->nofcommands;i++) {
        /* if they match, return the number */
        if (!strcmp(name,objcmdtab->command[i].name)) return i;
    }
    /* no matches found, so return SCRIPT_NOIDENT */
    return SCRIPT_NOIDENT;
}

_UINT
searchobject(char *name) {
    _UINT i;

    /* trace all objects */
    for(i=0;i<nofobjs;i++) {
        /* if they match, return the number */
        if (!strcmp(name,object[i].name)) return i;
    }
    /* no matches found, so return SCRIPT_NOIDENT */
    return SCRIPT_NOIDENT;
}

void
addobject(char *name,_UCHAR type,_UCHAR internal,_UCHAR external,char *basefile) {
    _UINT i;

    if(nofobjs>=MAX_OBJECTS) {
        /* too many objects declared. bomb out */
        printf("addobject(%s): too many objects declared. %u is the maximum\n",name,MAX_OBJECTS);
        exit(1);
    }
    strcpy(object[nofobjs].name,name);
    strcpy(object[nofobjs].filename,basefile);
    object[nofobjs].offsetcount=0;
    object[nofobjs].type=type;
    object[nofobjs].internal=internal;
    object[nofobjs].external=external;
    nofobjs++;
}


void
addlabeltab(char *name,_UINT pos) {
    if(noflabeltabs>=MAX_LABEL_TABS) {
        /* too many entries in the label tab. bomb out */
        printf("too many entries in the label table. %u is the maximum\n",MAX_LABEL_TABS);
        exit(1);
    }
    if((labeltab[noflabeltabs].labelno=findlabel(name))==SCRIPT_NOIDENT) {
        /* this label was never declared. bomb out */
        printf("undefined label '%s'\n",name);
        exit(1);
    }
    labeltab[noflabeltabs].offset=pos;
    noflabeltabs++;
}

_UINT
get_labelno_from_line(_UINT lineno) {
    _UINT i;

    for(i=0;i<noflabels;i++) {
        if(label[i].line==lineno) return i;
    }
    return SCRIPT_NOIDENT;
}

void
initobjects() {
    /* add all system objects */
    addobject(MOUSE_OBJECTNAME,TYPE_ANM,1,0,"/dev/null");
    addobject(BAR_OBJECTNAME,TYPE_ANM,1,0,"/dev/null");
    addobject(CONTROLS_OBJECTNAME,TYPE_ANM,1,0,"/dev/null");
    addobject(SYSTEM_OBJECTNAME,TYPE_ANM,1,0,"/dev/null");
    addobject(SCRIPT_OBJECTNAME,TYPE_ANM,1,0,"/dev/null");
}

void
add_object_instance(char *objname,_UINT offset) {
    _UINT objno;

    if ((objno=searchobject(objname))==SCRIPT_NOIDENT) {
        /* object was never declared. report and die */
        printf("object '%s' was never declared when adding instance\n",objname);
        exit(1);
    }
    if(object[objno].offsetcount>=MAX_GLOBAL_OBJ_COUNT) {
        /* cannot add anymore. maximum global object count exceeded */
        printf("cannot add instance to object '%s', it exceeded the maximum count of %u instances\n",object[objno].name,MAX_GLOBAL_OBJ_COUNT);
        exit(1);
    }
    object[objno].offset[object[objno].offsetcount]=offset;
    object[objno].offsetcount++;
}

_UINT
line_in_proc(_UINT line) {
    _UINT i;

    for(i=0;i<nofprocs;i++) {
       if((line>proc[i].startline)&&(line<proc[i].endline)) return i;
    }
    return SCRIPT_NOIDENT;
}

void
parse() {
    _UINT line,count;
    char script_line[MAX_SCRIPT_LINE_LEN];
    char s[MAX_SCRIPT_STRINGS][MAX_SCRIPT_LINE_LEN];
    char script_temp[MAX_IDENT_LEN];
    char *ptr;

    line=0; nofprocs=0; nofvars=0; noflabels=0; noflabeltabs=0;
    while(readline(line,scriptsize,script_line,scriptdata)) {
        /* zero out all previously read fields */
        for(count=0;count<MAX_SCRIPT_STRINGS;count++) {
            strcpy(s[count],"");
        }
        /* is sscanf able to fill in 2 variables? */
        if (sscanf(script_line,"%s %s",&s[0],&s[1])==2) {
            /* yes. check if s3 is the SCRIPT_STARTPROC */
	    if (!strcmp(s[1],SCRIPT_STARTPROC)) {
                /* yes. we found a procedure, let's check if it was already defined */
                if ((count=findproc(s[0]))!=SCRIPT_NOIDENT) {
                    /* yes, bomb out with an error message */
                    printf("%s(%lu): Error: procedure '%s' is duplicated in line %lu\n",script_filename,line+1,s[0],proc[count].name,proc[count].startline,line);
                    exit(1);
                }
                /* no, so add it to the list. is the name short enough? */
                if (strlen(s[0])>MAX_IDENT_LEN) {
                    /* no, bomb out with an error message (friendlier than a segmentation fault :-) */
                    printf("%s(%lu): Error: procedure name cannot exceed %u chars, and '%s' does\n",script_filename,line+1,MAX_IDENT_LEN,s[0]);
                    exit(1);
                }
                /* yes. let's go! */
                strcpy(proc[nofprocs].name,s[0]);
                proc[nofprocs].startline=line;
                proc[nofprocs].endline=SCRIPT_NOLINE;
                nofprocs++;
            }
            /* no, is s[0] a SCRIPT_ENDPROC? */
            if (!strcmp(s[0],SCRIPT_ENDPROC)) {
                /* yes. we found a procedure end. was it ever started? */
                if ((count=findproc(s[1]))==SCRIPT_NOIDENT) {
                    /* no, so bomb out with an error message */
                    printf("%s(%lu): Error: procedure '%s' never started\n",script_filename,line+1,s[0]);
                    exit(1);
                }
                /* no, so edit the endline field */
                proc[count].endline=line;
            }
        }
        line++;
    }
    /* now check the variables and labels */
    line=0;
    while(readline(line,scriptsize,script_line,scriptdata)) {
        /* zero out all previously read fields */
        for(count=0;count<MAX_SCRIPT_STRINGS;count++) {
            strcpy(s[count],"");
        }
        /* split script line into fields */
	sscanf(script_line,"%s %s %s %s %s",s[0],&s[1],&s[2],&s[3],&s[4]);
        /* is s[0] a SCRIPT_DEFINEVAR? */
        if (!strcmp(s[0],SCRIPT_DEFINEVAR)) {
            /* yes. we found a variable declarator. check if it was ever declared before */
            /* is this line part of a procedure? */
            if ((count=line_in_proc(line))!=SCRIPT_NOIDENT) {
                /* yeah. bomb out, since that is not allowed */
                printf("%s(%lu): Error: variable '%s' is declared in procedure '%s'. variables cannot be declared in a procedure\n",script_filename,line+1,s[1],proc[count].name);
                exit(1);
            }
            /* fill in the new variable info */
            addobject(s[1],TYPE_VAR,0,0,"/dev/bb");
        } else {
            /* no, but it might be a label. check for a colon */
            if ((ptr=strchr(s[0],':'))!=NULL) {
                /* it's a label, we found a colon. copy the name in s3 */
                for (count=0;count<(ptr-s[0]);count++) {
                    script_temp[count]=s[0][count];
                    script_temp[count+1]=0;
                }
                /* check if this label was already declared elsewhere */
                if ((count=findlabel(script_temp))!=SCRIPT_NOIDENT) {
                    /* yes, so bomb out with an error */
                    printf("%s(%lu): Error: label '%s' already defined in line %lu\n",script_filename,line+1,script_temp,label[count].line+1);
                    exit(1);
                }
                /* no, so add the label to the list */
                strcpy(label[noflabels].name,script_temp);
                label[noflabels].line=line;
                noflabels++;
             }
             /* well, maybe its a -hard to code- set command. check of it */
             if(!strcmp(s[0],COMMAND_ASSIGN)) {
                 /* oh joy! it actually IS one! */
                 /* check if it is an animation */
                 if(!strcmp(s[3],GFX_LOADANM)) {
                     /* it is one. oh boy! */
                     /* script_s[4] is with quotes. strip them off */
                     stripquotes(s[4],script_temp);
                     /* add it to The List(tm) */
                     addobject(s[1],TYPE_ANM,0,0,script_temp);
                 } else {
                     /* well, maybe a text object? */
                     if(!strcmp(s[3],GFX_TEXT)) {
                         /* it is one. oh boy! */
                         /* script_s[4] is with quotes. strip them off */
                         stripquotes(s[4],script_temp);
                         /* add it to The List(tm) */
                         addobject(s[1],TYPE_TEXT,0,0,script_temp);
                     } else {
                         /* not  a known type. die */
                         printf("%s(%lu): unsupported assignment type '%s'\n",script_filename,line+1,s[3]);
                         exit(1);
                     }
                 }
             }
             /* it something external declared? */
             if(!strcmp(s[0],COMMAND_EXTERN)) {
                 /* yeah. figure out what it is */
                 if(strcmp(s[1],EXTERN_OBJ)) {
                     /* check if it is a variable */
                     if(!strcmp(s[1],SCRIPT_DEFINEVAR)) {
                         /* it is an external variable. add it to the list */
                         addobject(s[2],TYPE_VAR,0,1,"/dev/bb");
                     } else {
                         /* also not a variable. this is unknown, so die */
                         printf("%s(%lu): what kind of external type is '%s'?\n",script_filename,line+1,s[1]);
                         exit(1);
                     }
                 } else {
                     /* add it as an object */
                     addobject(s[2],TYPE_ANM,0,1,"/dev/null");
                 }
             }
        }
        line++;
    }
}

void
addscript(_UCHAR c) {
    script[script_pointer]=c;
    script_pointer++;
    if(script_pointer>=SCRIPT_MAX_COMPILED_SIZE) {
        printf("%s(0) Error: a compiled script cannot exceed %u bytes\n",script_filename,SCRIPT_MAX_COMPILED_SIZE);
        exit(1);
    }
}

void
addscriptstr(char *s,_UINT len) {
    _UINT i,size;

    if(!len) {
        size=strlen(s);
        addscript(size);
    } else {
        size=len;
    }

    for(i=0;i<size;i++) {
        addscript(s[i]);
    }
}

void
addscriptword(_UINT no) {
    addscript(no&0xff);
    addscript(no>>8);
}

void
addscriptnum(char *s,_ULONG line) {
    _UINT i,j;
    char *ptr;

    i=(_UINT)strtoul(s,&ptr,0);
    if(ptr==s) {
        printf("%s(%lu): '%s' is not numeric\n",script_filename,line+1,s);
        exit(1);
    }
    addscriptword(i);
}

_UINT
resolvexpression(char *expr,_UCHAR *type) {
    char *ptr;
    _UINT no,i;
    char script_object[SCRIPT_MAXIDENTLEN];
    char script_value[SCRIPT_MAXIDENTLEN];

    /* check if there is an dot in it */
    if((ptr=strchr(expr,'.'))!=NULL) {
        /* it got a dot. copy everthing before the dot to [script_object] */
        for (i=0;i<ptr-expr;i++) {
            script_object[i]=expr[i];
            script_object[i+1]=0;
        }
        /* [script_value] is the method after the dot */
        for (i=0;i<(strlen(expr)-(ptr-expr))-1;i++) {
            script_value[i]=expr[i+1+ptr-expr];
            script_value[i+1]=0;
        }
        /* try to find the object */
        if((no=searchobject(script_object))!=SCRIPT_NOIDENT) {
            /* this is an object, so continue */
            *type=TYPE_NORESOLV;
            if(!strcmp(script_value,SCRIPT_EXPR_X)) {
                *type=TYPE_OBJECT_X;
            }
            if(!strcmp(script_value,SCRIPT_EXPR_Y)) {
                *type=TYPE_OBJECT_Y;
            }
            if(!strcmp(script_value,SCRIPT_EXPR_FRAMENO)) {
                *type=TYPE_OBJECT_FRAMENO;
            }
            if(!strcmp(script_value,SCRIPT_EXPR_MOVING)) {
                *type=TYPE_OBJECT_MOVING;
            }
            if(!strcmp(script_value,SCRIPT_EXPR_ANIMATING)) {
                *type=TYPE_OBJECT_ANIMATING;
            }
            if(!strcmp(script_value,SCRIPT_EXPR_PRIORITYCODE)) {
                *type=TYPE_OBJECT_PRIORITY;
            }
            return no;
        }
    }
    /* does a variable like this exist? */
    no=searchobject(expr);
    if((no!=SCRIPT_NOIDENT)&&(object[no].type==TYPE_VAR)) {
        /* it's a variable! */
        *type=TYPE_VARIABLE;
        return no;
    }
    /* try to convert it to a value */
    no=strtoul(expr,&ptr,0);
    if(ptr==expr) {
        /* we cannot resolv this. */
        *type=TYPE_NORESOLV;
        return 0;
    }
    *type=TYPE_STATIC;
    return no;
}

void
getobjname(char *out,char *in) {
    _UINT i;
    char *ptr;

    if((ptr=strchr(in,'.'))!=NULL) {
        /* it got a dot. copy everthing before the dot to [out] */
        for (i=0;i<ptr-in;i++) {
            out[i]=in[i];
            out[i+1]=0;
        }
        return;
    }
    strcpy(out,in);
}

void
doparams(char* scriptline,_UCHAR type,_UINT nofparms,char script_s[MAX_SCRIPT_STRINGS][MAX_IDENT_LEN],_ULONG line) {
    _UINT count,i;
    _UCHAR ttype;
    char tmp[SCRIPT_MAXIDENTLEN];
    char moretmp[MAX_SCRIPT_LINE_LEN];

    switch(type) {
        case 0: /* normal string */
                for(count=1;count<=nofparms;count++) {
                    addscriptstr(script_s[count],0);
                }
                break;
        case 1: /* numbers */
                for(count=1;count<=nofparms;count++) {
                    addscriptnum(script_s[count],line);
                }
                break;
        case 2: /* directions */
                for(count=1;count<=nofparms;count++) {
                    i=resolvedirection(script_s[count]);
                    if(i==DIR_UNKNOWN) {
                        /* direction didn't resolve. damn */
                        printf("%s(%lu): Error: unknown direction '%s'\n",script_filename,line+1,script_s[count]);
                        exit(1);
                    }
                    addscript(i);
                }
                break;
        case 3: /* boolean */
                for(count=1;count<=nofparms;count++) {
                    i=resolveboolean(script_s[count]);
                    if(i==BOOL_UNKNOWN) {
                        /* boolean didn't resolve. damn */
                        printf("%s(%lu): Error: unknown boolean value '%s'\n",script_filename,line+1,script_s[count]);
                        exit(1);
                    }
                    addscript(i);
                }
                break;
        case 4: /* numbers/variables */
                for(count=1;count<=nofparms;count++) {
                    i=resolvexpression(script_s[count],&ttype);
                    if(ttype==TYPE_NORESOLV) {
                        /* cannot resolve expression. die */
                        printf("%s(%lu): Error: what sort of expression is '%s'?\n",script_filename,line+1,script_s[count]);
                        exit(1);
                    }
                    /* add the type and value of the first expression to the script */
                    addscript(ttype);
                    if(ttype==TYPE_STATIC) {
                        addscriptword(i);
                    } else {
                        /* this has an object attached, so add also an instance */
                        getobjname(tmp,script_s[count]);
                        add_object_instance(tmp,script_pointer);
                        addscriptword(0xFFFF);
                    }
                }
                break;
        case 5: /* complete text (!!!CHANGE 512 FOR MAX_SCRIPT_LINE_LEN!!!) */
                memset(moretmp,0,MAX_SCRIPT_LINE_LEN);
                sscanf(scriptline,"%s %512c",&tmp,&moretmp);
                /* nuke all quotes */
                i=0;
                while(moretmp[i]) {
                    if(moretmp[i]=='"') {
                        memcpy((void *)(moretmp+i),(void *)(moretmp+i+1),strlen(moretmp)-i);
                    }
                    i++;
                }
                /* add the string */
                addscriptstr(moretmp,0);
                break;
       default: printf("%s(%lu): unknown type value %u\n",script_filename,line+1,type);
                exit(1);
    }
}

void compile_line(char *script_line,_ULONG line);

void
do_expression(char *expr,char *code,_ULONG line) {
    _UINT i,count,chek,offs,v,s1;
    char first_expr[SCRIPT_MAXIDENTLEN];
    char sec_expr[SCRIPT_MAXIDENTLEN];
    char tmp[SCRIPT_MAXIDENTLEN];
    char *ptr,*ptr2;
    _UCHAR type;

    /* split [expr] into usuable parts */
    ptr=strchr(expr,'(');
    if(ptr==NULL) {
        /* no parathese open found. die */
        printf("%s(%lu): '(' expected\n",script_filename,line+1);
        exit(1);
    }
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
                          printf("%s(%lu): Error: cannot resolve operator in string '%s'\n",script_filename,line+1,expr);
                          exit(1);
            }
        }
    }
    count=0;
    for(i=(ptr-expr)+1;i<(ptr2-expr)-v;i++) {
        first_expr[count]=expr[i];
        first_expr[count+1]=0;
        count++;
    }
    /* write it to the script */
    addscript(chek);
    /* get the next variable */
    count=0;
    for(i=(ptr2-expr)+2-v;i<(strchr(expr,')')-expr);i++) {
        sec_expr[count]=expr[i];
        sec_expr[count+1]=0;
        count++;
    }
    /* now figure out what [first_expr] is */
    i=resolvexpression(first_expr,&type);
    if(type==TYPE_NORESOLV) {
        /* cannot resolve expression. die */
        printf("%s(%lu): Error: what sort of expression is '%s'(1) out of '%s'?\n",script_filename,line+1,first_expr,expr);
        exit(1);
    }
    /* add the type and value of the first expression to the script */
    addscript(type);
    if(type==TYPE_STATIC) {
        addscriptword(i);
    } else {
        /* this has an object attached, so add also an instance */
        getobjname(tmp,first_expr);
        add_object_instance(tmp,script_pointer);
        addscriptword(0xFFFF);
    }
    if(s1) {
        /* now do the second expression */
        i=resolvexpression(sec_expr,&type);
        if(type==TYPE_NORESOLV) {
            /* cannot resolve expression. die */
            printf("%s(%lu): Error: what sort of expression is '%s'(2) out of '%s'?\n",script_filename,line+1,sec_expr,expr);
            exit(1);
        }
        /* add the type and value of the second expression to the script */
        addscript(type);
        if(type==TYPE_STATIC) {
            addscriptword(i);
        } else {
            /* this has an object attached, so add also an instance */
            getobjname(tmp,sec_expr);
            add_object_instance(tmp,script_pointer);
            addscriptword(0xffff);
        }
    } else {
        addscript(TYPE_STATIC);
        addscriptword(0);
    }
    /* now save the offset */
    offs=script_pointer;
    addscriptword(0xffff);              /* address to jump to if false (not known yet) */
    compile_line(code,0);
    /* and set address to jump to if false the correct value */
    script[offs]=script_pointer&0xff;
    script[offs+1]=script_pointer>>8;
}

void
compile_line(char *script_line,_ULONG line) {
    _UINT i,count;
    char script_object[MAX_IDENT_LEN];
    char script_method[MAX_IDENT_LEN];
    char script_temp[MAX_IDENT_LEN];
    char tmp[MAX_IDENT_LEN];
    char script_s[MAX_SCRIPT_STRINGS][MAX_IDENT_LEN];
    char *ptr;
    _UCHAR ok,ttype;

        /* empty previously read strings */
        for(i=0;i<MAX_SCRIPT_STRINGS;i++) {
            strcpy(script_s[i],"");
        }

        /* divide the script line into field script_s[number] */
        sscanf(script_line,"%s %s %s %s %s %s",&script_s[0],script_s[1],&script_s[2],&script_s[3],&script_s[4],&script_s[5]);

        /* process the line */
        if (((ptr=strchr(script_s[0],'.'))!=NULL)&&(strchr(script_s[0],'=')==NULL)) {
            /* script_object is going to be the object name before the dot */
            for (i=0;i<ptr-script_s[0];i++) {
                script_object[i]=script_s[0][i];
                script_object[i+1]=0;
            }
            /* script_method is the method after the dot */
            for (i=0;i<(strlen(script_s[0])-(ptr-script_s[0]))-1;i++) {
                script_method[i]=script_s[0][i+1+ptr-script_s[0]];
                script_method[i+1]=0;
            }
            addscript(OPCODE_OBJECT); ok=0;
            if ((i=searchobject(script_object))==SCRIPT_NOIDENT) {
                printf("%s(%lu): Error: script object '%s' never declared\n",script_filename,line+1,script_object);
                exit(1);
            }
            add_object_instance(script_object,script_pointer);
            addscriptword(0xFFFF);                              /* objectno (not yet known) */
            if ((i=lookup_object_command(script_method))!=SCRIPT_NOIDENT) {
                /* the method is in the table. good */
                addscript(objcmdtab->command[i].opcode);
                /* are there any parameters? */
                if (objcmdtab->command[i].nofparms) {
                    /* yeah, also jam in the parameters */
                    /* do we need to strip the quotes? */
                    if (objcmdtab->command[i].strip_quotes) {
                        /* yeah, lets do it! */
                        for(count=0;count<MAX_SCRIPT_STRINGS;count++) {
                            stripquotes(script_s[count],script_line);
                            strcpy(script_s[count],script_line);
                        }
                    }
                    doparams(script_line,objcmdtab->command[i].type,objcmdtab->command[i].nofparms,script_s,line);
                    if (objcmdtab->command[i].opcode==OPCODE_OBJ_ADDICON) {
                        /* also add a word */
                        addscriptnum(script_s[objcmdtab->command[i].nofparms+1],line);
                    }
              }
              ok=1;
            } else {
                /* ...handle special functions here... */
            }
            if(!ok) {
                /* method not found. aargh! */
                printf("%s(%lu): Error: script method '%s' is not known to me.\n",script_filename,line+1,script_method);
                exit(1);
            }
        } else {
            /* its not an object. look it up and process it */
            i=lookupcommand(script_s[0]);
            /* did we find it? */
            if(i!=SCRIPT_NOIDENT) {
                /* yeah, jam in the opcode */
                addscript(command->command[i].opcode);
                if (command->command[i].nofparms) {
                    /* also jam in the parameters */
                    /* do we need to strip the quotes? */
                    if (command->command[i].strip_quotes) {
                        /* yeah, lets do it! */
                        for(count=0;count<MAX_SCRIPT_STRINGS;count++) {
                            stripquotes(script_s[count],script_line);
                            strcpy(script_s[count],script_line);
                        }
                    }
                    /* are the parameters numeric? */
                    if (command->command[i].type) {
                        /* yeah, convert them to numbers */
                        for(count=1;count<command->command[i].nofparms+1;count++) {
                            addscriptnum(script_s[count],line);
                        }
                    } else {
                        /* no, just wipe them in */
                        for(count=1;count<command->command[i].nofparms+1;count++) {
                            addscriptstr(script_s[count],0);
                        }
                    }
                }
            } else {
                /* no, but this might be a variable assignment. check for it */
                if ((ptr=strchr(script_s[0],'='))!=NULL) {
                    /* script_object is the value name before the equal sign */
                    for (i=0;i<ptr-script_s[0];i++) {
                        script_object[i]=script_s[0][i];
                        script_object[i+1]=0;
                    }
                    /* script_temp is the number after the equal sign */
                    for (i=0;i<(strlen(script_s[0])-(ptr-script_s[0]));i++) {
                        script_temp[i]=script_s[0][i+1+ptr-script_s[0]];
                        script_temp[i+1]=0;
                    }
                    /* was this variable ever declared in this script? */
                    i=searchobject(script_object);
                    if((i==SCRIPT_NOIDENT)||(object[i].type!=TYPE_VAR)) {
                        /* no, so die */
                        printf("%s(%lu): variable '%s' was never declared\n",script_filename,line+1,script_object);
                        exit(1);
                    }
                    addscript(OPCODE_SETVAR);
                    add_object_instance(script_object,script_pointer);
                    addscriptword(0xffff);      /* variable number (not yet known) */

                    i=resolvexpression(script_temp,&ttype);
                    if(ttype==TYPE_NORESOLV) {
                        /* cannot resolve expression. die */
                        printf("%s(%lu): what sort of expression is '%s'?\n",script_filename,line+1,script_temp);
                        exit(1);
                    }
                    /* add the type and value of the first expression to the script */
                    addscript(ttype);
                    if(ttype==TYPE_STATIC) {
                        addscriptword(i);
                    } else {
                        /* this has an object attached, so add also an instance */
                        getobjname(tmp,script_temp);
                        add_object_instance(tmp,script_pointer);
                        addscriptword(0xFFFF);
                    }
                } else {
                    /* also not. was is then a jump? */
                    if (!strcmp(script_s[0],SCRIPT_JUMP)) {
                        /* yeah. add an entry */
                        addscript(OPCODE_JUMP);
                        /* and add it to The Table(tm) */
                        addlabeltab(script_s[1],script_pointer);
                        /* and dump in a mumbo-jumbo address */
                        addscriptword(0xffff);  /* label address */
                    } else {
                        /* well, maybe a set? */
                        if(!strcmp(script_s[0],COMMAND_ASSIGN)) {
                            /* yeah! jank it in */
                            addscript(OPCODE_OBJ_ACTIVATE);
                            if ((i=searchobject(script_s[1]))==SCRIPT_NOIDENT) {
                               printf("%s(%lu): Error: script object '%s' never declared when setting (probably bug in compiler)\n",script_filename,line+1,script_s[1]);
                               exit(1);
                            }
                            add_object_instance(script_s[1],script_pointer);
                            /* dump in the object number (will be changed by linker) */
                            addscriptword(0xFFFF);                              /* objectno (not yet known) */
                            /* add the object type */
                            addscript(object[i].type);
                            /* and add the filename */
                            addscriptstr(object[i].filename,0);
                            /* and add the object name */
                            addscriptstr(object[i].name,0);
                        } else {
                            /* maybe an if? */
                            if(!strcmp(script_s[0],COMMAND_IF)) {
                                /* yeah. let's do it */
                                /* first figure out what to do if true */
                                count=0;
                                for(i=(strchr(script_line,')')-script_line)+1;i<strlen(script_line);i++) {
                                    script_method[count]=script_line[i];
                                    script_method[count+1]=0;
                                    count++;
                                }
                                /*
                                 * now [script_method] is whatever the user
                                 * wants call when this is true and
                                 * script_s[1] is the expression to be checked
                                 *
                                 */
                                addscript(OPCODE_IF);
                                do_expression(script_s[1],script_method,line);
                            }
                            /* how about a while? */
                            if(!strcmp(script_s[0],COMMAND_WHILE)) {
                                /* yeah. let's do it */
                                /* first figure out what to do if true */
                                count=0;
                                for(i=(strchr(script_line,')')-script_line)+1;i<strlen(script_line);i++) {
                                    script_method[count]=script_line[i];
                                    script_method[count+1]=0;
                                    count++;
                                }
                                /*
                                 * now [script_method] is whatever the user
                                 * wants call when this is true and
                                 * script_s[1] is the expression to be checked
                                 *
                                 */
                                addscript(OPCODE_WHILE);
                                do_expression(script_s[1],script_method,line);
                            }
                        }
                    }
                }
            }
        }
}


void
compile_proc(_UINT procno) {
    char script_line[MAX_SCRIPT_LINE_LEN];
    _UINT line,i;

    proc[procno].bytestart=script_pointer;
    line=proc[procno].startline+1;
    while (1) {
        if (line>=proc[procno].endline) {
           break;
        }
        if (!readline(line,scriptsize,script_line,scriptdata)) {
            /* we were unable to read the line. bomb out with an error */
            printf("%s(%lu): unable to read script line\n",script_filename,line+1);
            exit(1);
        }
        compile_line(script_line,line);
        /* does this line denote a label */
        if ((i=get_labelno_from_line(line))!=SCRIPT_NOIDENT) {
            /* yeah. change the label structure to point it to the correct byte */
            label[i].offset=script_pointer;
        }
        /* next line */
        line++;
    }
    proc[procno].bytend=script_pointer;
}

void
dump_map(char *fname,char *script_name) {
    FILE *f;
    _UINT i;
    char s[128];

    if((f=fopen(fname,"wb"))==NULL) {
        printf(" unable to create file\n");
        exit(1);
    }
    fprintf(f,"MAD Compiler Map File for '%s'\n",script_name);
    fprintf(f,"-------------------------------------------\n\n");
    fprintf(f,"Procedure       Start  End  Size\n");
    for(i=0;i<nofprocs;i++) {
        strcpy(s,proc[i].name);
        lineout(s,16);
        fprintf(f,"%s %4u %4u %5u\n",s,proc[i].bytestart,proc[i].bytend,(proc[i].bytend-proc[i].bytestart));
    }
    fclose(f);
}

void
zerotabs() {
    _UINT i;

    for(i=0;i<MAX_OBJECTS;i++) {
        object[i].offsetcount=0;
    }
}

void
dumpinfile(_UCHAR *buf,_ULONG size,FILE *f) {
    if(!fwrite(buf,size,1,f)) {
        printf("unable to write file\n");
        exit(1);
    }
}

void
write_loc(char *fname) {
    _UCHAR i;
    FILE *f;

    if((f=fopen(fname,"wb"))==NULL) {
        printf("unable to create file '%s'\n",fname);
        exit(1);
    }

    for(i=0;i<nofobjs;i++) {
        dumpinfile((_UCHAR *)object[i].name,strlen(object[i].name)+1,f);
        dumpinfile(&object[i].offsetcount,1,f);
        if(object[i].offsetcount) {
            dumpinfile((_UCHAR *)object[i].offset,(object[i].offsetcount)*2,f);
        }
        dumpinfile(&object[i].type,1,f);
        dumpinfile(&object[i].internal,1,f);
        dumpinfile(&object[i].external,1,f);

        dumpinfile((_UCHAR *)&object[i].filename,strlen(object[i].filename)+1,f);
    }
    fclose(f);
}

void
fix_labels() {
    _UINT i;

    for(i=0;i<noflabeltabs;i++) {
        script[labeltab[i].offset]=(label[labeltab[i].labelno].offset&0xff);
        script[labeltab[i].offset+1]=(label[labeltab[i].labelno].offset>>8);
    }
}

void
writescript(char *fname) {
    FILE *f;
    _UINT i;
    struct SCRIPTHEADER hdr;
    struct PROCHEADER phdr;

    if((f=fopen(fname,"wb"))==NULL) {
        printf("unable to open '%s'\n",fname);
        exit(1);
    }
    memset(&hdr,0,sizeof(struct SCRIPTHEADER));
    /* build header. set version number to zero to indicate linking required */
    hdr.magicno=SCRIPT_MAGICNO;
    hdr.version=0;
    hdr.script_size=script_pointer;
    hdr.nofprocs=nofprocs;
    if(!fwrite(&hdr,sizeof(struct SCRIPTHEADER),1,f)) {
        printf("unable to write '%s'\n",fname);
        exit(1);
    }
    if(!fwrite(&script,script_pointer,1,f)) {
        printf("unable to write '%s'\n",fname);
        exit(1);
    }
    /* now dump in the procedure table */
    for(i=0;i<nofprocs;i++) {
        memset(&phdr,0,sizeof(struct PROCHEADER));
        memcpy(phdr.name,proc[i].name,MAX_IDENT_LEN);
        phdr.start_pos=proc[i].bytestart;
        phdr.end_pos=proc[i].bytend;
        if(!fwrite(&phdr,sizeof(struct PROCHEADER),1,f)) {
            printf("unable to write '%s'\n",fname);
            exit(1);
        }
    }
    fclose(f);
}

int
main(int argc,char *argv[]) {
    _UINT count;

    #ifndef RHIDE_COMPITABLE
        printf("The MAD Compiler Version 1.0 - (c) 1999 The MAD Crew\n\n");
    #endif
    if(argc!=2) {
        printf("The MAD Compiler Version 1.0 - (c) 1999 The MAD Crew\n\n");
        printf("Usuage: MC [file.scr]\n");
        exit(1);
    }
    constructfilenames(argv[1],out_filename,map_filename,loc_filename);
    strcpy(script_filename,argv[1]);
    script_pointer=0;nofobjs=0;
    zerotabs();
    initobjects();
    #ifndef RHIDE_COMPITABLE
        printf("> Compiling %s\n",argv[1]);
        printf("þ Reading file...");
    #endif
    scriptdata=readfile(argv[1],&scriptsize);
    if (scriptdata==NULL) exit(1);
    #ifndef RHIDE_COMPITABLE
        printf(" done\n");
        printf("þ Parsing script...");
    #endif
    parse();
    #ifndef RHIDE_COMPITABLE
        printf(" done\n");
        printf("> found %u variable(s), %u procedure(s) and %u label(s)\n",nofvars,nofprocs,noflabels);
    #endif
    for(count=0;count<nofprocs;count++) {
        #ifndef RHIDE_COMPITABLE
            printf("þ Compiling procedure '%s'...",proc[count].name);
        #endif
        compile_proc(count);
        #ifndef RHIDE_COMPITABLE
            printf(" done\n");
        #endif
    };
    free(scriptdata);
    #ifndef RHIDE_COMPITABLE
        printf("þ Fixing label entries...");
    #endif
    fix_labels();
    #ifndef RHIDE_COMPITABLE
        printf(" done\n");
        printf("> Writing destination files\n");
        printf("þ Writing compiled script to '%s'...",out_filename);
    #endif
    writescript(out_filename);
    #ifndef RHIDE_COMPITABLE
        printf(" done\n");
        printf("þ Writing map file to '%s'...",map_filename);
    #endif
    dump_map(map_filename,argv[1]);
    #ifndef RHIDE_COMPITABLE
        printf(" done\n");
        printf("þ Writing linker information file to '%s'...",loc_filename);
    #endif
    write_loc(loc_filename);
    #ifndef RHIDE_COMPITABLE
        printf(" done\n");
    #endif
    free(scriptdata);
    return 0;
}
