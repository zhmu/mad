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
#include "gadgets.h"
#include "mc.h"
#include "../../../source/objman.h"
#include "../../../source/script.h"
#include "../../../source/types.h"

char*          cmp_cscript_code;
_ULONG         cmp_cscript_code_bufsize;
_ULONG         cmp_cscript_code_curpos;

_ULONG*        cmp_linetab;
_ULONG         cmp_noflines;
char*          cmp_filebuf;
_ULONG         cmp_filebufsize;

MC_SUPERCLASS* cmp_superclass;
_ULONG         cmp_nofsuperclasses;

_ULONG         cmp_nofobjects;
MC_OBJECT*     cmp_object;

char           cmp_scriptname[MC_MAX_SCRIPTNAME_LEN];
char           cmp_pair[MC_MAX_SCRIPT_PAIRS][MC_MAX_PAIR_LENGTH];

/*
 * cmp_cscript_addbyte(_UCHAR ch)
 *
 * This will add [ch] to the compiled script code. It'll resize the script
 * buffer if needed.
 *
 */
void
cmp_cscript_addbyte(_UCHAR ch) {
    /* add the byte */
    cmp_cscript_code[cmp_cscript_code_curpos]=ch;

    /* increment pointer */
    cmp_cscript_code_curpos++;

    /* will the next call exceed the buffer space? */
    if(cmp_cscript_code_curpos==cmp_cscript_code_bufsize) {
        /* yes. resize the buffer */
        cmp_cscript_code_bufsize+=MC_BASE_SCRIPTCODE_SIZE;
        cmp_cscript_code=(char*)realloc(cmp_cscript_code,cmp_cscript_code_bufsize);

        /* did this fail? */
        if(cmp_cscript_code==NULL) {
            /* yes. die */
            fprintf(stderr,"cmp_cscript_addbyte(0x%x): unable to resize code buffer to %lu bytes\n",(_UINT)(_UCHAR)ch,cmp_cscript_code_bufsize);
            exit(1);
        }
    }
}

/*
 * cmp_cscript_addword(_UINT w)
 *
 * This will add word [w] to the compiled script code. It'll resize the script
 * buffer if needed.
 *
 */
void
cmp_cscript_addword(_UINT w) {
    /* add the hi byte */
    cmp_cscript_addbyte((_UCHAR)(w&0xff));
    /* add the lo byte */
    cmp_cscript_addbyte((_UCHAR)(w>>8));
}

/*
 * cmp_cscript_addlong(_ULONG l)
 *
 * This will add long [l] to the compiled script code. It'll resize the script
 * buffer if needed.
 *
 */
void
cmp_cscript_addlong(_ULONG l) {
    /* add the hi byte */
    cmp_cscript_addword((_UINT)(l&0xffff));
    /* add the lo byte */
    cmp_cscript_addword((_UINT)(l>>16));
}

/*
 * cmp_init()
 *
 * This will initialize the MAD compiler routines.
 *
 */
void
cmp_init() {
    /* set up the size and current position of the code thing */
    cmp_cscript_code_bufsize=MC_BASE_SCRIPTCODE_SIZE; cmp_cscript_code_curpos=0;

    /* allocate a buffer */
    if((cmp_cscript_code=(char*)malloc(cmp_cscript_code_bufsize))==NULL) {
        /* this failed. die */
        cmp_cleanup();
        fprintf(stderr,"cmp_init(): cannot allocate %lu bytes, exiting\n",cmp_cscript_code_bufsize);
        exit(1);
    }

    /* clean the line table */
    cmp_linetab=NULL; cmp_noflines=0;
    /* clean the file buffer */
    cmp_filebuf=NULL; cmp_filebufsize=0;
    /* clean the object buffer */
    cmp_object=NULL; cmp_nofobjects=0;
}

/*
 * cmp_readfile(char* fname)
 *
 * This will read file [fname]. It will die on any error.
 *
 */
void
cmp_readfile(char* fname) {
    FILE* f;
    _ULONG i,j;

    /* open the file */
    if((f=fopen(fname,"rb"))==NULL) {
        /* this failed. die */
        fprintf(stderr,"cmp_readfile(): unable to open file '%s'\n",fname);
        exit(1);
    }
    /* figure out the file size */
    fseek(f,0,SEEK_END); cmp_filebufsize=ftell(f); rewind(f);

    /* allocate memory for the buffer */
    if((cmp_filebuf=(char*)malloc(cmp_filebufsize))==NULL) {
        /* this failed. die */
        fprintf(stderr,"cmp_readfile(): cannot allocate %lu bytes for file data (file '%s')\n",cmp_filebufsize,fname);
        exit(1);
    }

    /* read the file into the buffer */
    if(!fread(cmp_filebuf,cmp_filebufsize,1,f)) {
        /* this failed. die */
        fprintf(stderr,"cmp_readfile(): cannot read %lu bytes of file '%s'\n",cmp_filebufsize,fname);
        exit(1);
    }

    /* count the number of lines */
    cmp_noflines=1;
    for(i=0;i<cmp_filebufsize;i++) {
        /* if we have a newline, increment the counter */
        if(cmp_filebuf[i]=='\n') cmp_noflines++;
    }

    /* allocate memory for the line offset buffer */
    if((cmp_linetab=(_ULONG*)malloc(cmp_noflines*sizeof(_ULONG)))==NULL) {
        /* this failed. die */
        fprintf(stderr,"cmp_readfile(): could not allocate %lu bytes for line offset buffer of file '%s'\n",cmp_noflines*sizeof(_ULONG),fname);
        exit(1);
    }

    /* add the line offsets */
    j=2; cmp_linetab[1]=0;
    for(i=0;i<cmp_filebufsize;i++) {
        /* newline? */
        if(cmp_filebuf[i]=='\n') {
            /* yes. is it within boundries? */
            if(j>cmp_noflines) {
                /* no. die */
                fprintf(stderr,"cmp_readfile(): we counted %lu lines in file '%s', but we were asked to add line %lu\n",cmp_noflines,fname,j);
                exit(1);
            }
            /* add it */
            cmp_linetab[j]=i+1;

            /* strip off the newline */
            cmp_filebuf[i]=0;

            /* if the previous char was a linefeed, also make it null */
            if(cmp_filebuf[i-1]=='\r') {
                cmp_filebuf[i-1]=0;
            }

            /* increment counter */
            j++;
        }
    }

    /* last line contains random junk, so remove it */
    cmp_noflines--;

    /* close the file */
    fclose(f);
}

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
 * cmp_find_superclass(char* name)
 *
 * This will return the ID of the superclass named [name], or MC_NOCLASS if
 * there isn't one.
 *
 */
_ULONG
cmp_find_superclass(char* name) {
    _ULONG i;
    struct MC_SUPERCLASS* sc;

    /* scan 'em all! */
    for(i=0;i<cmp_nofsuperclasses;i++) {
        /* is this The Chosen(tm)? */
        sc=(struct MC_SUPERCLASS*)&(cmp_superclass[i]);
        if(!strcmp(name,sc->name)) {
            /* yup. return it */
            return i;
        }
    }
    /* nothing was found. say no */
    return MC_NOCLASS;
}

/*
 * cmp_add_superclass(char* name)
 *
 * This will add a superclass named [name]. It will return the superclass ID.
 *
 */
_ULONG
cmp_add_superclass(char* name) {
    struct MC_SUPERCLASS* sc;

    /* does it already exists? */
    if(cmp_find_superclass(name)!=MC_NOCLASS) {
        /* yes. die */
        fprintf(stderr,"cmp_add_superclass(): superclass '%s' already declared\n",name);
        exit(1);
    }

    /* add memory for the new superclass */
    if((cmp_superclass=(MC_SUPERCLASS*)realloc(cmp_superclass,(cmp_nofsuperclasses+1)*sizeof(MC_SUPERCLASS)))==NULL) {
        /* this failed. die */
        fprintf(stderr,"cmp_addsuperclass(): could not allocate memory for superclass '%s'\n",name);
        exit(1);
    }

    /* add it */
    sc=(struct MC_SUPERCLASS*)&(cmp_superclass[cmp_nofsuperclasses]);

    /* set the properties etc */
    if(strlen(name)>MC_MAX_SUPERCLASS_LEN) {
        /* the name is too long! die */
        fprintf(stderr,"cmp_addsuperclass(): superclass name '%s' is too long, it may not exceed %lu chars\n",name,MC_MAX_SUPERCLASS_LEN);
        exit(1);
    }
    strcpy(sc->name,name);
    sc->nofmethods=0;
    sc->methods=NULL;

    /* increment counter */
    cmp_nofsuperclasses++;

    /* return this one */
    return (cmp_nofsuperclasses-1);
}

/*
 * cmp_add_superclass_method(_ULONG classid,char* name,_ULONG flags)
 *
 * This will add method [name] to the superclass with ID [classid]. The new
 * method will be given flags [flags].
 *
 */
void
cmp_add_superclass_method(_ULONG classid,char* name,_ULONG flags) {
    struct MC_SUPERCLASS* sc;
    struct MC_METHOD* mt;

    /* make a pointer to the superclass */
    sc=(struct MC_SUPERCLASS*)&(cmp_superclass[classid]);

    /* resize the method field */
    if((sc->methods=(MC_METHOD*)realloc(sc->methods,(sc->nofmethods+1)*sizeof(MC_METHOD)))==NULL) {
        /* this failed. die */
        fprintf(stderr,"cmp_add_superclass_method(): out of memory while adding '%s:%s'\n",sc->name,name);
        exit(1);
    }

    /* make a pointer to the method */
    mt=(struct MC_METHOD*)&(sc->methods[sc->nofmethods]);

    /* check the length */
    if(strlen(name)>MC_MAX_METHOD_LEN) {
        /* this failed. die */
        fprintf(stderr,"cmp_add_superclass_method(): name of method '%s' is too long (superclas '%s')\n",name,sc->name);
        exit(1);
    }

    /* add it */
    memset(mt,0,sizeof(MC_METHOD));
    strcpy(mt->name,name);
    mt->flags=flags;

    /* increment the number of methods */
    sc->nofmethods++;
}

/*
 * cmp_scan_superclass_method(_ULONG classid,char* name)
 *
 * This will scan for method [name] in superclass [classid]. It will return
 * the method ID if it is found or MC_NOMETHOD if nothing was found.
 *
 */
_ULONG
cmp_scan_superclass_method(_ULONG classid,char* name) {
    struct MC_SUPERCLASS* sc;
    struct MC_METHOD* mt;
    _ULONG i;

    /* make a pointer to the superclass */
    sc=(struct MC_SUPERCLASS*)&(cmp_superclass[classid]);

    /* scan 'em all! */
    for(i=0;i<sc->nofmethods;i++) {
        /* make a pointer to the method */
        mt=(struct MC_METHOD*)&(sc->methods[i]);

        /* is this The Chosen One(tm)? */
        if(!strcmp(name,mt->name)) {
            /* yup! return the id */
            return i;
        }
    }
    /* nothing was found. return MC_NOMETHOD */
    return MC_NOMETHOD;
}

/*
 * cmp_includefile(char* fname)
 *
 * This will include file [fname]. Include files are only allowed to have
 * declaration into them.
 *
 */
void
cmp_includefile(char* fname) {
    struct MC_SUPERCLASS* sc;
    struct MC_METHOD* mt;
    FILE*  f;
    char   line[MC_MAX_PAIR_LENGTH];
    char   cmp_classname[MC_MAX_PAIR_LENGTH];
    char   cmp_temp[MC_MAX_PAIR_LENGTH];
    char   cmp_methodname[MC_MAX_PAIR_LENGTH];
    char   cmp_obj_objname[MC_MAX_PAIR_LENGTH];
    char   cmp_obj_methodname[MC_MAX_PAIR_LENGTH];
    char*  ptr;
    _ULONG lineno;
    _ULONG cur_class_id,cur_method_id;
    _ULONG class_id,method_id;
    _UCHAR status,ok;

    /* open the file */
    if((f=fopen(fname,"rt"))==NULL) {
        /* this failed. die */
        fprintf(stderr,"cmp_includefile(): unable to open file '%s'\n",fname);
        cmp_cleanup();
        exit(1);
    }

    /* keep reading lines */
    lineno=1; status=MC_STATUS_NORMAL;
    while (fgets(line,MC_MAX_PAIR_LENGTH,f)) {
        /* nuke the newline at the end */
        if(line[strlen(line)-1]=='\n') {
            line[strlen(line)-1]=0;
        }

        /* split the line */
        cmp_splitpairs(line); ok=0;

        /* if we are just parsing normally, continue */
        if(status==MC_STATUS_NORMAL) {
            /* do we have a class definition? */
            if(!strcmp(cmp_pair[0],MC_DEFINE_CLASSDEF)) {
                /* yes! figure out the class name in [cmp_classname] */
                /* copy everything before the accolade to [cmp_objtemp] */
                gad_copyuntil(gad_scanlastfrombegin(strchr(line,' '),' '),cmp_classname,'{');
                /* spaces? we *HATE* spaces. nuke 'em! */
                gad_striptrailingspaces(cmp_classname);

                /* add it */
                cur_class_id=cmp_add_superclass(cmp_classname);

                /* set new status */
                status=MC_STATUS_CLASSDEF;

                /* line was successfully progressed */
                ok=1;
            }
        } else {
            /* are we inside a class definition? */
            if(status==MC_STATUS_CLASSDEF) {
                /* yup! if there are spaces, nuke 'em! */
                strcpy(cmp_temp,line);
                gad_striptrailingspaces(cmp_temp);

                /* is there a closing definition thingy? */
                if(!strcmp(cmp_temp,"}")) {
                    /* yes. get outta here */
                    status=MC_STATUS_NORMAL;
                } else {
                    /* is there a () pair? */
                    if((ptr=strchr(cmp_temp,'('))==NULL) {
                        /* nope. die */
                        fprintf(stderr,"%s[%lu]: '(' expected\n",fname,lineno);
                        exit(1);
                    }
                    if(strchr(cmp_temp,')')==NULL) {
                        /* nope. die */
                        fprintf(stderr,"%s[%lu]: ')' expected\n",fname,lineno);
                        exit(1);
                    }
                    /* copy the unparathised name to [cmp_methodname] */
                    gad_copyuntil(cmp_temp,cmp_methodname,'(');

                    /* add the method */
                    cmp_add_superclass_method(cur_class_id,cmp_methodname,MC_SCLASS_PROC);
                }
                /* line was successfully progressed */
                ok=1;
            }
        }

        /* if we have comments or blank lines, it's ok */
        if((!strlen(line))||(line[0]=='#')||(((line[0]=='/')&&(line[1]=='/')))) ok=1;

        /* was the line ok? */
        if(!ok) {
            /* no. die */
            fprintf(stderr,"%s[%lu]: unable to parse line (remind that this is an include file!)\n",fname,lineno);
            exit(1);
        }

        /* increment line counter */
        lineno++;
    }

    /* close the file */
    fclose(f);
}

/*
 * cmp_find_object(char* objname)
 *
 * This will scan the object chain for an object named [objname]. It will
 * return the object ID if it is found, or MC_NOBJECT if nothing was found.
 *
 */
_ULONG
cmp_find_object(char* objname) {
    struct MC_OBJECT* obj;
    _ULONG i;

    /* scan 'em all */
    for(i=0;i<cmp_nofobjects;i++) {
        /* make a pointer to the object data */
        obj=(struct MC_OBJECT*)&(cmp_object[i]);

        /* is this The Chosen One(tm)? */
        if(!strcmp(obj->name,objname)) {
            /* yup! return the id */
            return i;
        }
    }
    /* it was not found. return MC_NOBJECT */
    return MC_NOBJECT;
}

/*
 * cmp_add_object(char* objname,_ULONG superclass)
 *
 * This will add object [objname] to the object list. It will return the
 * object ID of the new object.
 *
 */
_ULONG
cmp_add_object(char* objname,_ULONG superclass) {
    struct MC_OBJECT* obj;

    /* does the object already exists? */
    if(cmp_find_object(objname)!=MC_NOBJECT) {
        /* yup. die */
        fprintf(stderr,"cmp_add_object(): object '%s' already exists\n",objname);
        exit(1);
    }

    /* resize the buffer */
    if((cmp_object=(MC_OBJECT*)realloc(cmp_object,(cmp_nofobjects+1)*sizeof(MC_OBJECT)))==NULL) {
        /* this failed. die */
        fprintf(stderr,"cmp_add_object(): out of memory while trying to add object '%s'\n",objname);
        exit(1);
    }
    /* make a pointer to the object data */
    obj=(struct MC_OBJECT*)&(cmp_object[cmp_nofobjects]);

    /* check the length of the object name */
    if(strlen(objname)>MC_MAX_OBJECT_LEN) {
        /* the name is too long! die */
        fprintf(stderr,"cmp_add_object(): object name '%s' is too long, it may not exceed %lu chars\n",objname,MC_MAX_OBJECT_LEN);
        exit(1);
    }

    /* add it */
    memset(obj,0,sizeof(MC_OBJECT));
    strcpy(obj->name,objname);
    obj->superclass=superclass;

    /* increment number of objects */
    cmp_nofobjects++;

    /* return the id */
    return (cmp_nofobjects-1);
}

/*
 * cmp_add_object_method(_ULONG objno,char* methodname)
 *
 * This will add method [methodname] to the object with id [objno]. It will
 * die on any error.
 *
 */
void
cmp_add_object_method(_ULONG objno,char* methodname) {
    struct MC_OBJECT* obj;

    /* make a pointer to the object data */
    obj=(struct MC_OBJECT*)&(cmp_object[cmp_nofobjects]);
}

/*
 * cmp_preprocess()
 *
 * This will preprocess the currently loaded script.
 *
 */
void
cmp_preprocess() {
    _ULONG i;
    _UINT  status;
    char*  line;
    char*  ptr;
    char   cmp_temp[MC_MAX_PAIR_LENGTH];
    char   cmp_objtemp[MC_MAX_PAIR_LENGTH];
    char   cmp_supertemp[MC_MAX_PAIR_LENGTH];
    char   cmp_methodtemp[MC_MAX_PAIR_LENGTH];
    _ULONG class_id;
    _ULONG cur_obj_id;

    /* make sure no info used */
    cmp_superclass=NULL;
    cmp_nofsuperclasses=0;

    /* scan all lines */
    status=MC_STATUS_NORMAL;
    for(i=1;i<cmp_noflines;i++) {
        /* first, create a pointer to the current line */
        line=(char*)(cmp_filebuf+cmp_linetab[i]);
        /* split the string */
        cmp_splitpairs(line);
        /* if we are in the normal status, continue */
        if(status==MC_STATUS_NORMAL) {
            /* check for the magic word that defines an object */
            if(!strcmp(cmp_pair[0],MC_DEFINE_OBJECT)) {
                /* this is an object definition! figure out the name and superclass */
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

                /* scan for the superclass */
                if((class_id=cmp_find_superclass(cmp_supertemp))==MC_NOCLASS) {
                    /* no luck either. die */
                    fprintf(stderr,"%s[%lu]: unable to find superclass '%s'\n",cmp_scriptname,i,cmp_supertemp);
                    cmp_cleanup();
                    exit(1);
                }

                /* add it! */
                cur_obj_id=cmp_add_object(cmp_objtemp,class_id);

                /* we are now checking objects! */
                status=MC_STATUS_OBJECT;
            } else {
                /* maybe include a file? */
                if(!strcmp(cmp_pair[0],MC_INCLUDE_FILE)) {
                    /* yes! if there's a quota, get the name between them in
                       [cmp_temp] */
                    if(!gad_getnamebetween(line,cmp_temp,'"','"')) {
                        /* this failed. maybe < and > then? */
                        if(!gad_getnamebetween(line,cmp_temp,'<','>')) {
                            /* no luck either. die */
                            fprintf(stderr,"%s[%lu]: cannot find filename\n",cmp_scriptname,i);
                            cmp_cleanup();
                            exit(1);
                        }
                    }
                    /* handle the include file */
                    cmp_includefile(cmp_temp);
                }
            }
        } else {
            /* are we inside an object declaration? */
            if(status==MC_STATUS_OBJECT) {
                /* yup. is this the stop thingy? */
                if(!strcmp(line,MC_OBJDEF_END)) {
                    /* yes. end it */
                    status=MC_STATUS_NORMAL;
                } else {
                    strcpy(cmp_temp,line);
                    gad_striptrailingspaces(cmp_temp);

                    /* is there a () pair? */
                    if((ptr=strchr(cmp_temp,'('))==NULL) {
                        /* nope. die */
                        fprintf(stderr,"%s[%lu]: '(' expected\n",cmp_scriptname,i);
                        exit(1);
                    }
                    if(strchr(cmp_temp,')')==NULL) {
                        /* nope. die */
                        fprintf(stderr,"%s[%lu]: ')' expected\n",cmp_scriptname,i);
                        exit(1);
                    }
                    /* copy the unparathised name to [cmp_methodname] */
                    gad_copyuntil(cmp_temp,cmp_methodtemp,'(');

                    printf(">%s<\n",cmp_methodtemp);
                }
            }
        }
    }
}

/*
 * cmp_compile()
 *
 * This will do the actual compiling. It will die on any error.
 *
 */
void
cmp_compile() {
    _ULONG i;
    _UINT  status;
    char*  line;

    /* scan all lines */
    status=MC_STATUS_NORMAL;
    for(i=1;i<cmp_noflines;i++) {
        /* first, create a pointer to the current line */
        line=(char*)(cmp_filebuf+cmp_linetab[i]);
        /* split the string */
        cmp_splitpairs(line);
    }
}

/*
 * cmp_compilefile(char* fname)
 *
 * This will read file [fname] and compile it. It will die on any error.
 *
 */
void
cmp_compilefile(char* fname) {
    /* save the filename */
    strcpy(cmp_scriptname,fname);

    /* read the file */
    cmp_readfile(fname);

    /* preprocess it */
    cmp_preprocess();

    /* compile it */
    cmp_compile();
}

/*
 * cmp_cleanup()
 *
 * This will free variables and such.
 *
 */
void
cmp_cleanup() {
    /* is there a line nymber table? */
    if (cmp_linetab!=NULL) {
        /* yes. free it */
        free(cmp_linetab);
        /* make sure we don't do this twice */
        cmp_linetab=NULL;
    }
    /* is there file data? */
    if (cmp_filebuf!=NULL) {
        /* yes. free it */
        free(cmp_filebuf);
        /* make sure we don't do this twice */
        cmp_filebuf=NULL;
    }
}
