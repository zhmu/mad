/*
 *                          The MAD Compiler 4.0
 *
 *                      (c) 1999, 2000 The MAD Crew
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compile.h"
#include "gadgets.h"
#include "mc.h"
#include "../../../source/include/objman.h"
#include "../../../source/include/opcodes.h"
#include "../../../source/include/script.h"
#include "../../../source/include/types.h"

#define strnicmp strncasecmp

char*          cmp_cscript_code;
_ULONG         cmp_cscript_codesize;

char*          cmp_cscript_data;
_ULONG         cmp_cscript_datasize;

_ULONG*        cmp_linetab;
_ULONG         cmp_noflines;
char*          cmp_filebuf;
_ULONG         cmp_filebufsize;

MC_SUPERCLASS* cmp_superclass;
_ULONG         cmp_nofsuperclasses;

_ULONG         cmp_nofobjects;
MC_OBJECT*     cmp_object;

_ULONG         cmp_curobjid,cmp_curmethodid;

MC_FUNC*       cmp_kernelfuncs;
_ULONG         cmp_nofkernelfuncs;

_ULONG         cmp_initoffset,cmp_doneoffset,cmp_runoffset;

MC_FUNC*       cmp_sysfuncs;
_ULONG         cmp_nofsysfuncs;

char           cmp_scriptname[MC_MAX_SCRIPTNAME_LEN];
char           cmp_pair[MC_MAX_SCRIPT_PAIRS][MC_MAX_PAIR_LENGTH];

void cmp_evaluate_expr(char*,_ULONG);
void cmp_add_params(char*,char*,_ULONG,MC_OBJECT*);

/*
 * cmp_realloc(void* ptr,_ULONG oldsize,_ULONG newsize)
 *
 * This will resize [ptr] to be [newsize] instead of [oldsize].
 *
 */
void*
cmp_realloc(void* ptr,_ULONG oldsize,_ULONG newsize) {
    void* buf;

    /* allocate new memory for the buffer */
    if((buf=malloc(newsize))==NULL) {
        /* this failed. die */
        fprintf(stderr,"cmp_realloc(): couldn't allocate %lu bytes of memory\n",newsize);
        exit(1);
    }

    /* copy the old data to the new buffer */
    memcpy(buf,ptr,oldsize);

    /* free the old buffer (FIXME: this generates a SEGFAULT! WHY?) */
//    free(ptr);

    /* return the new stuff */
    return buf;
}

/*
 * cmp_cscript_addbyte(_UCHAR ch)
 *
 * This will add [ch] to the compiled script code. It'll resize the script
 * buffer if needed.
 *
 */
void
cmp_cscript_addbyte(_UCHAR ch) {
    /* expand the buffer */
    cmp_cscript_code=(char*)cmp_realloc(cmp_cscript_code,cmp_cscript_codesize,(cmp_cscript_codesize+1));

    /* did this fail? */
    if(cmp_cscript_code==NULL) {
        /* yes. die */
        fprintf(stderr,"cmp_cscript_addbyte(0x%x): unable to resize code buffer to %lu bytes\n",(_UINT)(_UCHAR)ch,cmp_cscript_codesize);
        exit(1);
    }

    /* add the byte */
    cmp_cscript_code[cmp_cscript_codesize]=ch;

    /* increment pointer */
    cmp_cscript_codesize++;
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
 * cmp_cscript_data_addbyte(_UCHAR ch)
 *
 * This will add [ch] to the compiled script data. It'll resize the script
 * buffer if needed.
 *
 */
void
cmp_cscript_data_addbyte(_UCHAR ch) {
    /* expand the buffer */
    cmp_cscript_data=(char*)cmp_realloc(cmp_cscript_data,cmp_cscript_datasize,(cmp_cscript_datasize+1));

    /* did this fail? */
    if(cmp_cscript_data==NULL) {
        /* yes. die */
        fprintf(stderr,"cmp_cscript_data_addbyte(0x%x): unable to resize code buffer to %lu bytes\n",(_UINT)(_UCHAR)ch,cmp_cscript_datasize);
        exit(1);
    }

    /* add the byte */
    cmp_cscript_data[cmp_cscript_datasize]=ch;

    /* increment pointer */
    cmp_cscript_datasize++;
}

/*
 * cmp_cscript_data_addword(_UINT w)
 *
 * This will add word [w] to the compiled script data. It'll resize the script
 * buffer if needed.
 *
 */
void
cmp_cscript_data_addword(_UINT w) {
    /* add the hi byte */
    cmp_cscript_data_addbyte((_UCHAR)(w&0xff));
    /* add the lo byte */
    cmp_cscript_data_addbyte((_UCHAR)(w>>8));
}

/*
 * cmp_cscript_data_addstring(char* s)
 *
 * This will add string [s] to the compiled script data. It'll resize the
 * script buffer if needed.
 *
 */
void
cmp_cscript_data_addstring(char* s) {
    _UINT i;

    /* add the chars (including the NUL) */
    for(i=0;i<=strlen(s);i++) {
        cmp_cscript_data_addbyte(s[i]);
    }
}


/*
 * cmp_cscript_pushimm(_ULONG i)
 *
 * This will add the code that will push [i] as an immediate long.
 *
 */
void
cmp_cscript_pushimm(_ULONG i) {
    /* does the immediate value fit in a word? */
    if (i<65536) {
        /* yes. add it as a push word */
        cmp_cscript_addbyte(OPCODE_PUSHW);
        /* add the value */
        cmp_cscript_addword(i);
    } else {
        /* no, add it as a push dword */
        cmp_cscript_addbyte(OPCODE_PUSH);
        /* add the value */
        cmp_cscript_addlong(i);
    }
}

/*
 * cmp_cscript_pusha()
 *
 * This will add the code that will push the accumulator.
 *
 */
void
cmp_cscript_pusha() {
    /* add the opcode */
    cmp_cscript_addbyte(OPCODE_PUSHA);
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
    cmp_cscript_codesize=0; cmp_cscript_code=NULL;

    /* set up the size and current position of the data thing */
    cmp_cscript_datasize=0; cmp_cscript_data=NULL;

    /* clean the line table */
    cmp_linetab=NULL; cmp_noflines=0;
    /* clean the file buffer */
    cmp_filebuf=NULL; cmp_filebufsize=0;
    /* clean the object buffer */
    cmp_object=NULL; cmp_nofobjects=0;
    /* clean the kernel functions buffer */
    cmp_kernelfuncs=NULL; cmp_nofkernelfuncs=0;
    /* clean the offsets */
    cmp_initoffset=MC_NOFFSET; cmp_doneoffset=MC_NOFFSET; cmp_runoffset=MC_NOFFSET;
    /* clean the system functions */
    cmp_sysfuncs=NULL;cmp_nofsysfuncs=0;
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
    _UINT i;

    /* first clean all splitted stuff */
    memset(cmp_pair,0,MC_MAX_SCRIPT_PAIRS*MC_MAX_PAIR_LENGTH);

    /* split them! */
    i = sscanf(s,"%s %s %s %s %s %s %s %s",&cmp_pair[0], &cmp_pair[1],
                                           &cmp_pair[2], &cmp_pair[3],
                                           &cmp_pair[4], &cmp_pair[5],
                                           &cmp_pair[6], &cmp_pair[7]);

    if (i==0xffff) return 0;
    return i;
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
 * cmp_find_kernelfunc(char* name)
 *
 * This will return the ID of the kernel function named [name], or MC_NOFUNC
 * if there isn't one.
 *
 */
_ULONG
cmp_find_kernelfunc(char* name) {
    _ULONG i;
    struct MC_FUNC* fc;

    /* scan 'em all! */
    for(i=0;i<cmp_nofkernelfuncs;i++) {
        /* is this The Chosen(tm)? */
        fc=(struct MC_FUNC*)&(cmp_kernelfuncs[i]);
        if(!strcmp(name,fc->name)) {
            /* yup. return it */
            return i;
        }
    }
    /* nothing was found. say no */
    return MC_NOFUNC;
}

/*
 * cmp_find_sysfunc(char* name)
 *
 * This will return the ID of the system function named [name], or
 * MC_NOFUNC if there isn't one.
 *
 */
_ULONG
cmp_find_sysfunc(char* name) {
    _ULONG i;
    struct MC_FUNC* fc;

    /* scan 'em all! */
    for(i=0;i<cmp_nofsysfuncs;i++) {
        /* is this The Chosen(tm)? */
        fc=(struct MC_FUNC*)&(cmp_sysfuncs[i]);
        if(!strcmp(name,fc->name)) {
            /* yup. return it */
            return i;
        }
    }
    /* nothing was found. say no */
    return MC_NOFUNC;
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
    if((cmp_superclass=(MC_SUPERCLASS*)cmp_realloc(cmp_superclass,(cmp_nofsuperclasses*sizeof(MC_SUPERCLASS)),(cmp_nofsuperclasses+1)*sizeof(MC_SUPERCLASS)))==NULL) {
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
    memset(sc,0,sizeof(MC_SUPERCLASS));
    strcpy(sc->name,name);
    sc->nofmethods=0;
    sc->methods=NULL;

    /* increment counter */
    cmp_nofsuperclasses++;

    /* return this one */
    return (cmp_nofsuperclasses-1);
}

/*
 * cmp_add_superclass_method(_ULONG classid,char* name,char* parms,_UCHAR type,_ULONG flags,_UCHAR opcode)
 *
 * This will add method [name] to the superclass with ID [classid]. The new
 * method will be given flags [flags]. The type will be set to [type]. The
 * method will get opcode [opcode].
 *
 */
void
cmp_add_superclass_method(_ULONG classid,char* name,char* params,_UCHAR type,_ULONG flags,_UCHAR opcode) {
    struct MC_SUPERCLASS* sc;
    struct MC_METHOD* mt;
    char* ptr;

    /* make a pointer to the superclass */
    sc=(struct MC_SUPERCLASS*)&(cmp_superclass[classid]);

    /* resize the method field */
    if((sc->methods=(MC_METHOD*)cmp_realloc(sc->methods,(sc->nofmethods*sizeof(MC_METHOD)),(sc->nofmethods+1)*sizeof(MC_METHOD)))==NULL) {
        /* this failed. die */
        fprintf(stderr,"cmp_add_superclass_method(): out of memory while adding '%s:%s'\n",sc->name,name);
        exit(1);
    }

    /* make a pointer to the method */
    mt=(struct MC_METHOD*)&(sc->methods[sc->nofmethods]);

    /* check the length of the name */
    if(strlen(name)>MC_MAX_METHOD_LEN) {
        /* this failed. die */
        fprintf(stderr,"cmp_add_superclass_method(): name of method '%s' is too long (superclas '%s')\n",name,sc->name);
        exit(1);
    }
    /* check the length of the params */
    if(strlen(params)>MC_MAX_PARAM_LEN) {
        /* this failed. die */
        fprintf(stderr,"cmp_add_superclass_method(): method '%s' has a too long parameter string '%s' (superclass '%s')\n",sc->name,params,sc->name);
        exit(1);
    }

    /* add it */
    memset(mt,0,sizeof(MC_METHOD));
    strcpy(mt->name,name);
    strcpy(mt->params,params);

    /* change all comma's in mt->params to spaces */
    while((ptr=strchr(mt->params,','))!=NULL) *ptr=' ';

    mt->type=type;
    mt->flags=flags;
    mt->opcode=opcode;

    /* increment the number of methods */
    sc->nofmethods++;
}

/*
 * cmp_add_superclass_property(_ULONG classid,char* name,_UINT code)
 *
 * This will add property [name] to the superclass with ID [classid]. The new
 * property will get property code [code].
 *
 */
void
cmp_add_superclass_property(_ULONG classid,char* name,_UINT code) {
    struct MC_SUPERCLASS* sc;
    struct MC_PROPERTY* pr;
 
    /* make a pointer to the superclass */
    sc=(struct MC_SUPERCLASS*)&(cmp_superclass[classid]);

    /* resize the method field */
    if((sc->properties=(MC_PROPERTY*)cmp_realloc(sc->properties,(sc->nofproperties*sizeof(MC_PROPERTY)),(sc->nofproperties+1)*sizeof(MC_PROPERTY)))==NULL) {
        /* this failed. die */
        fprintf(stderr,"cmp_add_superclass_property(): out of memory while adding '%s:%s'\n",sc->name,name);
        exit(1);
    }

    /* make a pointer to the method */
    pr=(struct MC_PROPERTY*)&(sc->properties[sc->nofproperties]);

    /* check the length of the name */
    if(strlen(name)>MC_MAX_PROP_LEN) {
        /* this failed. die */
        fprintf(stderr,"cmp_add_superclass_property(): name of property '%s' is too long (superclass '%s')\n",name,sc->name);
        exit(1);
    }

    /* add it */
    memset(pr,0,sizeof(MC_PROPERTY));
    strcpy(pr->name,name);
    pr->code=code;

    /* increment the number of methods */
    sc->nofproperties++;
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
 * cmp_scan_superclass_property(_ULONG classid,char* name)
 *
 * This will scan for property [name] in superclass [classid]. It will return
 * the property ID if it is found or MC_NOPROP if nothing was found.
 *
 */
_ULONG
cmp_scan_superclass_property(_ULONG classid,char* name) {
    struct MC_SUPERCLASS* sc;
    struct MC_PROPERTY* pt;
    _ULONG i;

    /* make a pointer to the superclass */
    sc=(struct MC_SUPERCLASS*)&(cmp_superclass[classid]);

    /* scan 'em all! */
    for(i=0;i<sc->nofproperties;i++) {
        /* make a pointer to the property */
        pt=(struct MC_PROPERTY*)&(sc->properties[i]);

        /* is this The Chosen One(tm)? */
        if(!strcmp(name,pt->name)) {
            /* yup! return the id */
            return i;
        }
    }
    /* nothing was found. return MC_NOPROP */
    return MC_NOPROP;
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
 * cmp_add_object(char* objname,_ULONG superclass,_ULONG flags)
 *
 * This will add object [objname] to the object list. It will return the
 * object ID of the new object. The new object will be given flags [flags].
 *
 */
_ULONG
cmp_add_object(char* objname,_ULONG superclass,_ULONG flags) {
    struct MC_OBJECT* obj;

    /* does the object already exists? */
    if(cmp_find_object(objname)!=MC_NOBJECT) {
        /* yup. die */
        fprintf(stderr,"cmp_add_object(): object '%s' already exists\n",objname);
        exit(1);
    }

    /* resize the buffer */
    if((cmp_object=(MC_OBJECT*)cmp_realloc(cmp_object,(cmp_nofobjects*sizeof(MC_OBJECT)),(cmp_nofobjects+1)*sizeof(MC_OBJECT)))==NULL) {
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
    obj->flags=flags;

    /* increment number of objects */
    cmp_nofobjects++;

    /* return the id */
    return (cmp_nofobjects-1);
}

/*
 * cmp_add_kernelfunc(char* name,char* params,_ULONG flags,_UCHAR funcno)
 *
 * This will add a kernel function named [name]. It will return the function
 * ID. It will get parameters [params], flags [flags] and function number
 * [funcno].
 *
 */
_ULONG
cmp_add_kernelfunc(char* name,char* params,_ULONG flags,_UCHAR funcno) {
    struct MC_FUNC* fc;
    char* ptr;

    /* does it already exists? */
    if(cmp_find_kernelfunc(name)!=MC_NOCLASS) {
        /* yes. die */
        fprintf(stderr,"cmp_add_kernelfunc(): kernel function '%s' already declared\n",name);
        exit(1);
    }

    /* add memory for the new function */
    if((cmp_kernelfuncs=(MC_FUNC*)cmp_realloc(cmp_kernelfuncs,(cmp_nofkernelfuncs*sizeof(MC_FUNC)),(cmp_nofkernelfuncs+1)*sizeof(MC_FUNC)))==NULL) {
        /* this failed. die */
        fprintf(stderr,"cmp_add_kernelfunc(): could not allocate memory for kernel function '%s'\n",name);
        exit(1);
    }

    /* add it */
    fc=(struct MC_FUNC*)&(cmp_kernelfuncs[cmp_nofkernelfuncs]);

    /* set the properties etc */
    if(strlen(name)>MC_MAX_FUNC_LEN) {
        /* the name is too long! die */
        fprintf(stderr,"cmp_add_kernelfunc(): kernel function name '%s' is too long, it may not exceed %lu chars\n",name,MC_MAX_FUNC_LEN);
        exit(1);
    }

    /* check the length of the params */
    if(strlen(params)>MC_MAX_PARAM_LEN) {
        /* this failed. die */
        fprintf(stderr,"cmp_add_kernelfunc(): kernel function '%s' has a too long parameter string '%s'\n",name,params);
        exit(1);
    }

    memset(fc,0,sizeof(MC_FUNC));
    strcpy(fc->name,name);
    strcpy(fc->params,params);
    fc->flags=flags;
    fc->funcno=funcno;

    /* change all comma's in mt->params to spaces */
    while((ptr=strchr(fc->params,','))!=NULL) *ptr=' ';

    /* increment counter */
    cmp_nofkernelfuncs++;

    /* return this one */
    return (cmp_nofkernelfuncs-1);
}

/*
 * cmp_add_sysfunc(char* name,char* params,_UCHAR funcno)
 *
 * This will add a system function named [name]. It will return the function
 * ID. It will get parameters [params] and function number [funcno].
 *
 */
_ULONG
cmp_add_sysfunc(char* name,char* params,_UCHAR funcno) {
    struct MC_FUNC* fc;
    char* ptr;

    /* does it already exists? */
    if(cmp_find_sysfunc(name)!=MC_NOCLASS) {
        /* yes. die */
        fprintf(stderr,"cmp_add_sysfunc(): system function '%s' already declared\n",name);
        exit(1);
    }

    /* add memory for the new function */
    if((cmp_sysfuncs=(MC_FUNC*)cmp_realloc(cmp_sysfuncs,(cmp_nofsysfuncs*sizeof(MC_FUNC)),(cmp_nofsysfuncs+1)*sizeof(MC_FUNC)))==NULL) {
        /* this failed. die */
        fprintf(stderr,"cmp_add_sysfunc(): could not allocate memory for system function '%s'\n",name);
        exit(1);
    }

    /* add it */
    fc=(struct MC_FUNC*)&(cmp_sysfuncs[cmp_nofsysfuncs]);

    /* set the properties etc */
    if(strlen(name)>MC_MAX_FUNC_LEN) {
        /* the name is too long! die */
        fprintf(stderr,"cmp_add_sysfunc(): system function name '%s' is too long, it may not exceed %lu chars\n",name,MC_MAX_FUNC_LEN);
        exit(1);
    }

    /* check the length of the params */
    if(strlen(params)>MC_MAX_PARAM_LEN) {
        /* this failed. die */
        fprintf(stderr,"cmp_add_sysfunc(): system function '%s' has a too long parameter string '%s'\n",name,params);
        exit(1);
    }

    memset(fc,0,sizeof(MC_FUNC));
    strcpy(fc->name,name);
    strcpy(fc->params,params);
    fc->funcno=funcno;

    /* change all comma's in mt->params to spaces */
    while((ptr=strchr(fc->params,','))!=NULL) *ptr=' ';

    /* increment counter */
    cmp_nofsysfuncs++;

    /* return this one */
    return (cmp_nofsysfuncs-1);
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
    char   cmp_params[MC_MAX_PAIR_LENGTH];
    char   cmp_obj_objname[MC_MAX_PAIR_LENGTH];
    char   cmp_obj_methodname[MC_MAX_PAIR_LENGTH];
    char*  ptr;
    char*  ptr2;
    _ULONG lineno;
    _ULONG cur_class_id,cur_method_id,flags;
    _ULONG i,j;
    _UCHAR status,ok,type,opcode;

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
            } else {
                /* well, it *might* be a kernel function reference */
                if(!strcmp(cmp_pair[0],MC_DEFINE_KERNELFUNCS)) {
                    /* yes! this is it! set the new status */
                    status=MC_STATUS_KERNELDEF;

                    /* line was successfully progressed */
                    ok=1;
                } else {
                    /* maybe it's a system function reference? */
                    if(!strcmp(cmp_pair[0],MC_DEFINE_SYSFUNCS)) {
                        /* yes! this is it! set the new status */
                        status=MC_STATUS_SYSFUNCS;

                        /* line was successfully progressed */
                        ok=1;
                    }
                }
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
                    if((ptr2=strchr(cmp_temp,')'))==NULL) {
                        /* nope. die */
                        fprintf(stderr,"%s[%lu]: ')' expected\n",fname,lineno);
                        exit(1);
                    }
                    /* get the parameters */
                    gad_copyuntil((char*)(ptr+1),cmp_params,')');

                    /* copy the unparathised name to [cmp_methodname] */
                    gad_copyuntil(cmp_temp,cmp_methodname,'(');

                    /* dump everything after the ) to [cmp_temp] */
                    strcpy(cmp_temp,(char*)(strchr(line,')')+1));

                    /* nuke spaces */
                    gad_striptrailingspaces(cmp_temp);

                    /* strip [cmp_temp] into cmp_pair[] pairs */
                    j=cmp_splitpairs(cmp_temp);

                    /* normally, it's just a normal function */
                    type=MC_TYPE_UNDEFINED;flags=0;

                    /* now, scan the stuff behind */
                    for(i=0;i<j;i++) {
                        /* does cmp_pair[0] indicate we have flags? */
                        if(!strcmp(cmp_pair[i],MC_DEFINE_FLAGS)) {
                            /* yup! are there still parameters? */
                            if((i+1)==j) {
                                /* nope. die */
                                fprintf(stderr,"%s[%lu]: parameter expected\n",fname,lineno);
                                exit(1);
                            }

                            /* set flag we are a kernel function */
                            type=MC_TYPE_METHOD;

                            /* yes! resolve them */
                            flags=strtol(cmp_pair[i+1],&ptr,0);

                            /* did this work */
                            if(ptr==cmp_pair[i+1]) {
                                /* nope. die */
                                fprintf(stderr,"%s[%lu]: cannot resolve number pair '%s'\n",fname,lineno,cmp_pair[i+1]);
                                exit(1);
                            }
                            /* skip the next param */
                            i++;
                        } else {
                            /* maybe a kernel function then? */
                            if(!strcmp(cmp_pair[i],MC_DEFINE_KERNELFUNC)) {
                               /* yup! are there still parameters? */
                               if((i+1)==j) {
                                    /* nope. die */
                                    fprintf(stderr,"%s[%lu]: parameter expected\n",fname,lineno);
                                    exit(1);
                                }
                                /* yes! resolve them */
                                opcode=strtol(cmp_pair[i+1],&ptr,0);

                                /* did this work */
                                if(ptr==cmp_pair[i+1]) {
                                    /* nope. die */
                                    fprintf(stderr,"%s[%lu]: cannot resolve number pair '%s'\n",fname,lineno,cmp_pair[i+1]);
                                    exit(1);
                                }

                                /* enforce kernel function flag */
                                flags|=MC_FLAG_KERNELFUNC;

                                /* set flag we are a kernel function */
                                type=MC_TYPE_METHOD;

                                /* skip the next param */
                                i++;
                            } else {
                                /* maybe a property declaration? */
                                if(!strcmp(cmp_pair[i],MC_DEFINE_PROPERTY)) {
                                    /* yup! are there still parameters? */
                                    if((i+1)==j) {
                                        /* nope. die */
                                        fprintf(stderr,"%s[%lu]: parameter expected\n",fname,lineno);
                                        exit(1);
                                    }
                                    /* yes! resolve them */
                                    opcode=strtol(cmp_pair[i+1],&ptr,0);

                                    /* did this work */
                                    if(ptr==cmp_pair[i+1]) {
                                        /* nope. die */
                                        fprintf(stderr,"%s[%lu]: cannot resolve number pair '%s'\n",fname,lineno,cmp_pair[i+1]);
                                        exit(1);
                                    }

                                    /* enforce property flag */
                                    flags|=MC_FLAG_PROPERTY;

                                    /* set flag we are a kernel function */
                                    type=MC_TYPE_METHOD;

                                    /* skip the next param */
                                    i++;
                                } else {
                                    /* was the string empty? */
                                    if(strlen(cmp_pair[i])) {
                                        /* no. this is an unknown parameter. die */
                                        fprintf(stderr,"%s[%lu]: unknown superclass keyword '%s'\n",fname,lineno,cmp_pair[i]);
                                        exit(1);
                                    }
                                }
                            }
                        }
                    }

                    /* if this is still an undefined thing, complain */
                    if(type==MC_TYPE_UNDEFINED) {
                        /* it is... die */
                        fprintf(stderr,"%s[%lu]: cannot determine type\n",fname,lineno);
                        exit(1);
                    }

                    /* add the method */
                    if(!(flags&MC_FLAG_PROPERTY)) {
                        cmp_add_superclass_method(cur_class_id,cmp_methodname,cmp_params,type,flags,opcode);
                    } else {
                        cmp_add_superclass_property(cur_class_id,cmp_methodname,opcode);
                    }
                }
                /* line was successfully progressed */
                ok=1;
            } else {
                /* are we currently doing a kernel function definition? */
                if(status==MC_STATUS_KERNELDEF) {
                    /* yup! nuke all spaces */
                    strcpy(cmp_temp,line);
                    gad_striptrailingspaces(cmp_temp);
                    /*  is there a closing definition thingy? */
                    if(!strcmp(cmp_temp,"}")) {
                        /* yes. get outta here */
                        status=MC_STATUS_NORMAL;
                        ok=1;
                    } else {
                        /* is there a () pair? */
                        if((ptr=strchr(cmp_temp,'('))==NULL) {
                            /* nope. die */
                            fprintf(stderr,"%s[%lu]: '(' expected\n",fname,lineno);
                            exit(1);
                        }
                        if((ptr2=strchr(cmp_temp,')'))==NULL) {
                            /* nope. die */
                            fprintf(stderr,"%s[%lu]: ')' expected\n",fname,lineno);
                            exit(1);
                        }
                        /* get the parameters */
                        gad_copyuntil((char*)(ptr+1),cmp_params,')');

                        /* copy the unparathised name to [cmp_methodname] */
                        gad_copyuntil(cmp_temp,cmp_methodname,'(');

                        /* dump everything after the ) to [cmp_temp] */
                        strcpy(cmp_temp,(char*)(strchr(line,')')+1));

                        /* nuke spaces */
                        gad_striptrailingspaces(cmp_temp);

                        /* strip [cmp_temp] into cmp_pair[] pairs */
                        j=cmp_splitpairs(cmp_temp);

                        /* normally, it's just a normal function */
                        type=MC_TYPE_METHOD;flags=0;

                        /* now, scan the stuff behind */
                        for(i=0;i<j;i++) {
                            /* does cmp_pair[0] indicate we have flags? */
                            if(!strcmp(cmp_pair[i],MC_DEFINE_FLAGS)) {
                                /* yup! are there still parameters? */
                                if((i+1)==j) {
                                    /* nope. die */
                                    fprintf(stderr,"%s[%lu]: parameter expected\n",fname,lineno);
                                    exit(1);
                                }
                                /* yes! resolve them */
                                flags=strtol(cmp_pair[i+1],&ptr,0);

                                /* did this work */
                                if(ptr==cmp_pair[i+1]) {
                                    /* nope. die */
                                    fprintf(stderr,"%s[%lu]: cannot resolve number pair '%s'\n",fname,lineno,cmp_pair[i+1]);
                                    exit(1);
                                }
                                /* skip the next param */
                                i++;
                            } else {
                                /* maybe a kernel function then? */
                                if(!strcmp(cmp_pair[i],MC_DEFINE_KERNELFUNC)) {
                                    /* yup! are there still parameters? */
                                    if((i+1)==j) {
                                        /* nope. die */
                                        fprintf(stderr,"%s[%lu]: parameter expected\n",fname,lineno);
                                        exit(1);
                                    }
                                    /* yes! resolve them */
                                    opcode=strtol(cmp_pair[i+1],&ptr,0);

                                    /* did this work */
                                    if(ptr==cmp_pair[i+1]) {
                                        /* nope. die */
                                        fprintf(stderr,"%s[%lu]: cannot resolve number pair '%s'\n",fname,lineno,cmp_pair[i+1]);
                                        exit(1);
                                    }
                                    /* skip the next param */
                                    i++;
                                } else {
                                    /* was the string empty? */
                                    if(strlen(cmp_pair[i])) {
                                        /* no. this is an unknown parameter. die */
                                        fprintf(stderr,"%s[%lu]: unknown superclass keyword '%s'\n",fname,lineno,cmp_pair[i]);
                                        exit(1);
                                    }
                                }
                            }
                        }
                        /* add the kernel function method */
                        cmp_add_kernelfunc(cmp_methodname,cmp_params,flags,opcode);
                    }
                    /* line was successfully progressed */
                    ok=1;
                }  else {
                    /* are we doing system functions? */
                    if (status==MC_STATUS_SYSFUNCS) {
                        /* yup! nuke all spaces */
                        strcpy(cmp_temp,line);
                        gad_striptrailingspaces(cmp_temp);
                        /*  is there a closing definition thingy? */
                        if(!strcmp(cmp_temp,"}")) {
                            /* yes. get outta here */
                            status=MC_STATUS_NORMAL;
                            ok=1;
                        } else {
                            /* is there a () pair? */
                            if((ptr=strchr(cmp_temp,'('))==NULL) {
                                /* nope. die */
                                fprintf(stderr,"%s[%lu]: '(' expected\n",fname,lineno);
                                exit(1);
                            }
                            if((ptr2=strchr(cmp_temp,')'))==NULL) {
                                /* nope. die */
                                fprintf(stderr,"%s[%lu]: ')' expected\n",fname,lineno);
                                exit(1);
                            }
                            /* get the parameters */
                            gad_copyuntil((char*)(ptr+1),cmp_params,')');

                            /* copy the unparathised name to [cmp_methodname] */
                            gad_copyuntil(cmp_temp,cmp_methodname,'(');

                            /* dump everything after the ) to [cmp_temp] */
                            strcpy(cmp_temp,(char*)(strchr(line,')')+1));

                            /* nuke spaces */
                            gad_striptrailingspaces(cmp_temp);

                            /* strip [cmp_temp] into cmp_pair[] pairs */
                            j=cmp_splitpairs(cmp_temp);

                            /* normally, it's just a normal function */
                            type=MC_TYPE_METHOD;flags=0;

                            /* now, scan the stuff behind */
                            for(i=0;i<j;i++) {
                                /* does cmp_pair[0] indicate we have a kernel function? */
                                if(!strcmp(cmp_pair[i],MC_DEFINE_KERNELFUNC)) {
                                     /* yup! are there still parameters? */
                                     if((i+1)==j) {
                                         /* nope. die */
                                        fprintf(stderr,"%s[%lu]: parameter expected\n",fname,lineno);
                                        exit(1);
                                    }
                                    /* yes! resolve them */
                                    opcode=strtol(cmp_pair[i+1],&ptr,0);

                                    /* did this work */
                                    if(ptr==cmp_pair[i+1]) {
                                        /* nope. die */
                                        fprintf(stderr,"%s[%lu]: cannot resolve number pair '%s'\n",fname,lineno,cmp_pair[i+1]);
                                        exit(1);
                                    }
                                    /* skip the next param */
                                    i++;
                                } else {
                                    /* was the string empty? */
                                    if(strlen(cmp_pair[i])) {
                                            /* no. this is an unknown parameter. die */
                                            fprintf(stderr,"%s[%lu]: unknown superclass keyword '%s'\n",fname,lineno,cmp_pair[i]);
                                            exit(1);
                                    }
                                }
                            }
                            /* add the system function method */
                            cmp_add_sysfunc(cmp_methodname,cmp_params,opcode);
                        }
                        /* line was successfully progressed */
                        ok=1;
                    }
                }
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

    /* close the file (FIXME: This causes a segfault... why?) */
//    fclose(f);
}

/*
 * cmp_scan_object_method(_ULONG objno,char* methodname)
 *
 * This will scan for method [methodname] in the object with id [objno]. It
 * will return the method ID if it is found or MC_NOMETHOD if not.
 *
 */
_ULONG
cmp_scan_object_method(_ULONG objno,char* methodname) {
    struct MC_OBJECT* obj;
    struct MC_METHOD* mt;
    _ULONG i;

    /* make a pointer to the object data */
    obj=(struct MC_OBJECT*)&(cmp_object[objno]);

    /* scan 'em all! */
    for(i=0;i<obj->nofmethods;i++) {
        /* make a pointer to the method */
        mt=(struct MC_METHOD*)&(obj->methods[i]);

        /* is this The Chosen One(tm)? */
        if(!strcmp(methodname,mt->name)) {
            /* yup! return the id */
            return i;
        }
    }
    /* nothing was found. return MC_NOMETHOD */
    return MC_NOMETHOD;
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
    struct MC_METHOD* mt;

    /* make a pointer to the object data */
    obj=(struct MC_OBJECT*)&(cmp_object[objno]);

    /* does it already exists? */
    if(cmp_scan_object_method(objno,methodname)!=MC_NOMETHOD) {
        /* yes. die */
        fprintf(stderr,"cmp_add_object_method(): method '%s' already exists in object '%s'\n",methodname,obj->name);
        exit(1);
    }

    /* resize the method field */
    if((obj->methods=(MC_METHOD*)cmp_realloc(obj->methods,(obj->nofmethods*sizeof(MC_METHOD)),(obj->nofmethods+1)*sizeof(MC_METHOD)))==NULL) {
        /* this failed. die */
        fprintf(stderr,"cmp_add_object_method(): out of memory while adding '%s:%s'\n",obj->name,methodname);
        exit(1);
    }

    /* make a pointer to the method */
    mt=(struct MC_METHOD*)&(obj->methods[obj->nofmethods]);

    /* check the length */
    if(strlen(methodname)>MC_MAX_METHOD_LEN) {
        /* this failed. die */
        fprintf(stderr,"cmp_add_object_method(): name of method '%s' is too long (superclas '%s')\n",methodname,obj->name);
        exit(1);
    }

    /* add it */
    memset(mt,0,sizeof(MC_METHOD));
    strcpy(mt->name,methodname);

    /* increment the number of methods */
    obj->nofmethods++;
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
                    fprintf(stderr,"%s[%lu]: superclass '%s' was never declared\n",cmp_scriptname,i,cmp_supertemp);
                    exit(1);
                }

                /* add it! */
                cur_obj_id=cmp_add_object(cmp_objtemp,class_id,MC_FLAG_DEFAULT);

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
                            exit(1);
                        }
                    }
                    /* handle the include file */
                    cmp_includefile(cmp_temp);
                } else {
                    /* maybe something external? */
                    if(!strcmp(cmp_pair[0],MC_DEFINE_EXTERNAL)) {
                        /* yup! is it an object? */
                        if(strcmp(cmp_pair[1],MC_DEFINE_OBJECT)) {
                            /* nope. die */
                            fprintf(stderr,"%s[%lu]: only objects can be external (at the time)\n",cmp_scriptname,i);
                            exit(1);
                        }
                        /* is there a colon? */
                        if(strchr(line,':')==NULL) {
                             /* there wasn't a colon! die */
                            fprintf(stderr,"%s[%lu]: ':' expected\n",cmp_scriptname,i);
                            exit(1);
                        }
                        /* copy everything before the colon to [cmp_temp] */
                        gad_copyuntil(gad_scanlastfrombegin(strchr(line,' '),' '),cmp_temp,':');
                        /* spaces? we *HATE* spaces. nuke 'em! */
                        gad_striptrailingspaces(cmp_temp);
                        /* copy everything after the first space to [cmp_objtemp] */
                        strcpy(cmp_objtemp,(char*)(strchr(cmp_temp,' ')+1));

                        /* copy everything after the colon to [cmp_methodtemp] */
                        strcpy(cmp_supertemp,(char*)(strchr(line,':')+1));

                        /* nuke all spaces */
                        gad_striptrailingspaces(cmp_objtemp);
                        gad_striptrailingspaces(cmp_supertemp);

                        /* scan for the superclass */
                        if((class_id=cmp_find_superclass(cmp_supertemp))==MC_NOCLASS) {
                            /* no luck either. die */
                            fprintf(stderr,"%s[%lu]: superclass '%s' was never declared\n",cmp_scriptname,i,cmp_supertemp);
                            exit(1);
                        }
                        cmp_add_object(cmp_objtemp,class_id,MC_FLAG_EXTERNAL);
                    }
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
                        fprintf(stderr,"%s[%lu]: ')' expected\n",cmp_scriptname,i);
                        exit(1);
                    }
                    /* copy the unparathised name to [cmp_methodname] */
                    gad_copyuntil(cmp_temp,cmp_methodtemp,'(');

                    /* does the superclass know this method? */
                    if(cmp_scan_superclass_method(class_id,cmp_methodtemp)==MC_NOMETHOD) {
                        /* nope. die */
                        fprintf(stderr,"%s[%lu]: method '%s' is not defined for superclass '%s'\n",cmp_scriptname,i,cmp_methodtemp,cmp_supertemp);
                        exit(1);
                    }

                    /* add the method to the object */
                    cmp_add_object_method(cur_obj_id,cmp_methodtemp);
                }
            }
        }
    }
}

/*
 * cmp_checkdeclarations(char* line,_ULONG lineno,_UINT* status)
 *
 * This will check line [line] for declarations. Line number [lineno] will be
 * used in error messages. [status] should be a pointer to an unsigned int
 * where the new status can be stored. It will return zero if nothing was
 * declared, or non-zero if something was declared.
 *
 */
_ULONG
cmp_checkdeclarations(char* line,_ULONG lineno,_UINT* status) {
    char*  ptr;
    char   cmp_temp[MC_MAX_PAIR_LENGTH];
    char   cmp_temptemp[MC_MAX_PAIR_LENGTH];
    char   cmp_objtemp[MC_MAX_PAIR_LENGTH];
    char   cmp_supertemp[MC_MAX_PAIR_LENGTH];
    char   cmp_methodtemp[MC_MAX_PAIR_LENGTH];
    _ULONG class_id,i,methodno;
    struct MC_OBJECT* obj;
    struct MC_METHOD* mt;
    struct MC_METHOD* smt;
    struct MC_SUPERCLASS* sc;

    /* is this an include line? */
    if(!strcmp(cmp_pair[0],MC_INCLUDE_FILE)) {
        /* yup. just ignore it */
        return 1;
    }

    /* are we inside an object declaration? */
    if(*status==MC_STATUS_OBJECT) {
        /* yes. is this the end of it? */
        if(!strcmp(line,MC_OBJDEF_END)) {
            /* yup. set the status back */
            *status=MC_STATUS_NORMAL;

            /* say we did something */
            return 1;
        }
        /* nope. tell it we did something so it'll ignore the line */
        return 1;
    }

    /* hmm... could this be an object declaration? */
    if(!strcmp(cmp_pair[0],MC_DEFINE_OBJECT)) {
        /* yes. let's just skip it */
        *status=MC_STATUS_OBJECT;

        /* we did something */
        return 1;
    }

    /* hmm... could this be an external declaration? */
    if(!strcmp(cmp_pair[0],MC_DEFINE_EXTERNAL)) {
        /* yes. let's just skip it */

        /* we did something */
        return 1;
    }

    /* let's see... is this the start of a procedure? */
    if(!strcmp(cmp_pair[1],MC_DEFINE_STARTPROC)) {
        /* yup */
        strcpy(cmp_temp,line);
        gad_striptrailingspaces(cmp_temp);

        /* is there a () pair? */
        if((ptr=strchr(cmp_temp,'('))==NULL) {
            /* nope. die */
            fprintf(stderr,"%s[%lu]: '(' expected\n",cmp_scriptname,lineno);
            exit(1);
        }
        if(strchr(cmp_temp,')')==NULL) {
            /* nope. die */
            fprintf(stderr,"%s[%lu]: ')' expected\n",cmp_scriptname,lineno);
            exit(1);
        }
        /* copy the unparathised name to [cmp_temptemp] */
        gad_copyuntil(cmp_temp,cmp_temptemp,'(');

        /* split the name into a [objname].[method] pair */
        if((ptr=strchr(cmp_temptemp,'.'))==NULL) {
            /* there wasn't a dot! die */
            fprintf(stderr,"%s[%lu]: '.' expected\n",cmp_scriptname,lineno);
            exit(1);
        }
        /* copy everything after the dot to [cmp_methodtemp] */
        strcpy(cmp_methodtemp,(char*)(ptr+1));

        /* copy everything before the dot to [cmp_objtemp] */
        gad_copyuntil(cmp_temptemp,cmp_objtemp,'.');

        /* are we already in a procedure? */
        if((cmp_curobjid!=MC_NOBJECT)||(cmp_curmethodid!=MC_NOMETHOD)) {
            /* yup! die */
            fprintf(stderr,"%s[%lu]: nested procedures are not allowed\n",cmp_scriptname,lineno);
            exit(1);
        }

        /* scan for the object */
        if((cmp_curobjid=cmp_find_object(cmp_objtemp))==MC_NOBJECT) {
            /* not found. complain */
            fprintf(stderr,"%s[%lu]: object '%s' never declared\n",cmp_scriptname,lineno,cmp_objtemp);
            exit(1);
        }
        /* scan for the method */
        if((cmp_curmethodid=cmp_scan_object_method(cmp_curobjid,cmp_methodtemp))==MC_NOMETHOD) {
            /* it isn't there! die */
            fprintf(stderr,"%s[%lu]: method '%s' of object '%s' never declared\n",cmp_scriptname,lineno,cmp_methodtemp,cmp_objtemp);
            exit(1);
        }
        /* make a pointer to the object data */
        obj=(struct MC_OBJECT*)&(cmp_object[cmp_curobjid]);

        /* make a pointer to the superclass data */
        sc=(struct MC_SUPERCLASS*)&(cmp_superclass[obj->superclass]);

        /* get the superclass method stuff */
        if((methodno=cmp_scan_superclass_method(obj->superclass,cmp_methodtemp))==MC_NOMETHOD) {
            /* is isn't there! die */
            fprintf(stderr,"%s[%lu]: method '%s' of object '%s' not found in superclass?!?\n",cmp_scriptname,lineno,cmp_methodtemp,cmp_objtemp);
            exit(1);
        }

        /* make a pointer to the superclass method */
        smt=(struct MC_METHOD*)&(sc->methods[methodno]);

        /* make a pointer to the method */
        mt=(struct MC_METHOD*)&(obj->methods[cmp_curmethodid]);

        /* set the start byte */
        mt->start_byte=cmp_cscript_codesize;

        /* do the flags indicate we are the init method? */
        if(smt->flags&SCRIPT_METHODFLAG_INIT) {
            /* yup. is this a superclass definition? */
            if(strcmp(cmp_objtemp,sc->name)) {
                /* no. was it already set? */
                if(cmp_initoffset!=MC_NOFFSET) {
                    /* yes. show error */
                    fprintf(stderr,"%s[%lu]: multiple methods declared as init methods\n",cmp_scriptname,lineno);
                    exit(1);
                }
                /* no. activate this one! */
                cmp_initoffset=cmp_cscript_codesize;
            }
        }

        /* do the flags indicate we are the done method? */
        if(smt->flags&SCRIPT_METHODFLAG_DONE) {
            /* yup. is this a superclass definition? */
            if(strcmp(cmp_objtemp,sc->name)) {
                /* no. was it already set? */
                if(cmp_doneoffset!=MC_NOFFSET) {
                    /* yes. show error */
                    fprintf(stderr,"%s[%lu]: multiple methods declared as done methods\n",cmp_scriptname,lineno);
                    exit(1);
                }
                /* no. activate this one! */
                cmp_doneoffset=cmp_cscript_codesize;
            }
        }

        /* do the flags indicate we are the run method? */
        if(smt->flags&SCRIPT_METHODFLAG_RUN) {
            /* yup. is this a superclass definition? */
            if(strcmp(cmp_objtemp,sc->name)) {
                /* no. was it already set? */
                if(cmp_runoffset!=MC_NOFFSET) {
                    /* yes. show error */
                    fprintf(stderr,"%s[%lu]: multiple methods declared as run methods\n",cmp_scriptname,lineno);
                    exit(1);
                }
                /* no. activate this one! */
                cmp_runoffset=cmp_cscript_codesize;
            }
        }

        /* scan for the method in the superclass */
        if((i=cmp_scan_superclass_method(obj->superclass,cmp_methodtemp))==MC_NOMETHOD) {
            /* it isn't there! die */
            fprintf(stderr,"%s[%lu]: previously found method '%s' of superclass '%s' not found?!?",cmp_scriptname,lineno,cmp_methodtemp,sc->name);
            exit(1);
        }

        /* make a pointer to the method */
        mt=(struct MC_METHOD*)&(sc->methods[i]);

        /* edit the status */
        *status=MC_STATUS_CODE;

        /* it was there! */
        return 1;
    }
    /* maybe the procedure was closed instead? */
    if(!strcmp(cmp_pair[0],MC_DEFINE_ENDPROC)) {
        /* yup */
        strcpy(cmp_temp,gad_scanlastfrombegin(line,'}'));
        gad_striptrailingspaces(cmp_temp);

        /* is there a () pair? */
        if((ptr=strchr(cmp_temp,'('))==NULL) {
            /* nope. die */
            fprintf(stderr,"%s[%lu]: '(' expected\n",cmp_scriptname,lineno);
            exit(1);
        }
        if(strchr(cmp_temp,')')==NULL) {
            /* nope. die */
            fprintf(stderr,"%s[%lu]: ')' expected\n",cmp_scriptname,lineno);
            exit(1);
        }
        /* copy the unparathised name to [cmp_temptemp] */
        gad_copyuntil(cmp_temp,cmp_temptemp,'(');

        /* split the name into a [objname].[method] pair */
        if((ptr=strchr(cmp_temptemp,'.'))==NULL) {
            /* there wasn't a dot! die */
            fprintf(stderr,"%s[%lu]: '.' expected\n",cmp_scriptname,lineno);
            exit(1);
        }
        /* copy everything after the dot to [cmp_methodtemp] */
        strcpy(cmp_methodtemp,(char*)(ptr+1));

        /* copy everything before the dot to [cmp_objtemp] */
        gad_copyuntil(cmp_temptemp,cmp_objtemp,'.');

        /* are we in a procedure? */
        if((cmp_curobjid==MC_NOBJECT)||(cmp_curmethodid==MC_NOMETHOD)) {
            /* no. complain */
            fprintf(stderr,"%s[%lu]: procedure '%s::%s' ended but never started\n",cmp_scriptname,lineno,cmp_objtemp,cmp_methodtemp);
            exit(1);
        }

        /* scan for the object */
        if(cmp_curobjid!=cmp_find_object(cmp_objtemp)) {
            /* not found. complain */
            fprintf(stderr,"%s[%lu]: you are not closing the currently opened procedure\n",cmp_scriptname,lineno);
            exit(1);
        }
        /* scan for the method */
        if(cmp_curmethodid!=cmp_scan_object_method(cmp_curobjid,cmp_methodtemp)) {
            fprintf(stderr,"%s[%lu]: you are not closing the currently opened procedure\n",cmp_scriptname,lineno);
            exit(1);
        }

        /* set variables indicating no procedure is being compiled (that we know of) */
        cmp_curobjid=MC_NOBJECT; cmp_curmethodid=MC_NOMETHOD;

        /* dump in the return opcode */
        cmp_cscript_addbyte(OPCODE_RET);

        /* edit the status */
        *status=MC_STATUS_CODE;

        /* say it's ok */
        return 1;
    }

    /* it wasn't there */
    return 0;
}

/*
 * cmp_evaluate_objexpr(char* value,_ULONG lineno)
 *
 * This will add object expression [value] to the script. It expects to be at
 * line [lineno]. It will add it directly to the script code. It will always
 * push the value on the stack.
 *
 */
void
cmp_evaluate_objexpr(char* value,_ULONG lineno) {
    char*  ptr;
    char   cmp_objnametmp[MC_MAX_PAIR_LENGTH];
    char   cmp_proptmp[MC_MAX_PAIR_LENGTH];
    struct MC_OBJECT* obj;
    struct MC_PROPERTY* pt;
    _ULONG propno;
    _ULONG objectno;

    /* make a pointer to the dot */
    if((ptr=strchr(value,'.'))==NULL) {
        /* there wasn't a dot! die */
        fprintf(stderr,"%s[%lu]: (INTERNAL ERROR) cmp_evaluate_objexpr() called without a object.method pair\n",cmp_scriptname,lineno);
        exit(0xfe);
    }

    /* copy everything before the dot to [cmp_objnametmp] */
    gad_copyuntil(value,cmp_objnametmp,'.');
    /* and copy everything after the dot to [cmp_proptmp] */
    gad_copyuntil((char*)(ptr+1),cmp_proptmp,'\0');
    /* nuke *all* spaces */
    gad_striptrailingspaces(cmp_objnametmp);
    gad_striptrailingspaces(cmp_proptmp);

    /* look up the object id */
    objectno=cmp_find_object(cmp_objnametmp);

    /* did this work? */
    if(objectno==MC_NOBJECT) {
        /* no. die */
        fprintf(stderr,"%s[%lu]: unknown object '%s'\n",cmp_scriptname,lineno,cmp_objnametmp);
        exit(1);
    }

    /* make a pointer to the object */
    obj=(struct MC_OBJECT*)&(cmp_object[objectno]);

    /* look up the method */
    propno=cmp_scan_superclass_property(obj->superclass,cmp_proptmp);

    /* did this work? */
    if(propno==MC_NOPROP) {
        /* no. die */
        fprintf(stderr,"%s[%lu]: unknown object property '%s'\n",cmp_scriptname,lineno,cmp_proptmp);
        exit(1);
    }

    /* make a pointer to the property */
    pt=(struct MC_PROPERTY*)&(cmp_superclass[obj->superclass].properties[propno]);

    /* first, add the opcode */
    cmp_cscript_addbyte(OPCODE_LAP);

    /* add the object number */
    cmp_cscript_addword(objectno);

    /* and add the property number */
    cmp_cscript_addword(pt->code);
}


/*
 * cmp_evaluate_specialexpr(char* value,_ULONG lineno)
 *
 * This will add special expression [value] to the script. It expects to be
 * at line [lineno]. It will add it directly to the script code. It will
 * always push the value on the stack.
 *
 */
void
cmp_evaluate_specialexpr(char* value,_ULONG lineno) {
    char* ptr;
    char  cmp_headtmp[MC_MAX_PAIR_LENGTH];
    char  cmp_tailtmp[MC_MAX_PAIR_LENGTH];
    _UCHAR code;
    _ULONG func_id;
    MC_FUNC* fc;

    /* make a pointer to the open accolade */
    if((ptr=strchr(value,'{'))==NULL) {
        /* there wasn't an open accolade! die */
        fprintf(stderr,"%s[%lu]: (INTERNAL ERROR) cmp_evaluate_specialexpr() called without a foo{bar} pair\n",cmp_scriptname,lineno);
        exit(0xfe);
    }

    /* check for a '}' */
    if(strchr(value,'}')==NULL) {
        /* it wasn't there! die */
        fprintf(stderr,"%s[%lu]: '}' expected\n",cmp_scriptname,lineno);
        exit(1);
    }

    /* dump everything before the '{' to [cmp_headtmp] */
    gad_copyuntil(value,cmp_headtmp,'{');

    /* and copy everything after the dot to [cmp_tailtmp] */
    gad_copyuntil((char*)(ptr+1),cmp_tailtmp,'}');

    /* scan for the system function */
    if ((func_id=cmp_find_sysfunc (cmp_headtmp))==MC_NOFUNC) {
        /* it isn't known! die */
        fprintf(stderr,"%s[%lu]: unknown special function '%s'\n",cmp_scriptname,lineno,cmp_headtmp);
        exit(1);
    }

    /* create a pointer to the structure */
    fc=(struct MC_FUNC*)&(cmp_sysfuncs[func_id]);

    /* push the tail to the stack */
    cmp_add_params(fc->params,cmp_tailtmp,lineno,NULL);

    /* add our kernel function */
    cmp_cscript_addbyte(OPCODE_CLK);
    cmp_cscript_addbyte(fc->funcno);
}

/*
 * cmp_evaluate_var(char* value,_ULONG lineno)
 *
 * This will add variable [value] to the script. It expects to be at line
 * [lineno]. It will add it directly to the script code. It will always push
 * the value on the stack.
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

    /* do we have a blah{q} thingy? */
    if(strchr(value,'{')!=NULL) {
        /* yup, pass it over to the special expression routine */
        cmp_evaluate_specialexpr(value,lineno);
        return;
    }

    /* does this number have a _ before it? */
    if(value[0]=='_') {
        /* yup, it's negative */
        value[0]='-';
    }

    /* it might be a static number then */
    no=strtoul(value,&ptr,0);
    if(ptr==value) {
        /* this failed. die */
        fprintf(stderr,"%s[%lu]: cannot parse '%s'\n",cmp_scriptname,lineno,value);
        cmp_cleanup();
        exit(1);
    }

    cmp_cscript_pushimm(no);
}

/*
 * cmp_add_expr_operator(_ULONG lineno,_UCHAR expr)
 *
 * This will add the valid expression comperisation opcode for [expr].
 * [lineno] should be the line number, which will be used in error messages.
 *
 */
void
cmp_add_expr_operator(_ULONG lineno,_UCHAR expr) {
    switch (expr) {
        case EXPR_ADD: /* + */
                       cmp_cscript_addbyte(OPCODE_ADD);
                       return;
        case EXPR_SUB: /* - */
                       cmp_cscript_addbyte(OPCODE_SUB);
                       return;
        case EXPR_MUL: /* * */
                       cmp_cscript_addbyte(OPCODE_MUL);
                       return;
        case EXPR_DIV: /* / */
                       cmp_cscript_addbyte(OPCODE_DIV);
                       return;
    }
    /* it's unknown! give an error! */
    fprintf(stderr,"%s[%lu]: (INTERNAL ERROR) cmp_add_expr_operator() called with illegal expression value %u\n",cmp_scriptname,lineno,expr);
    cmp_cleanup();
    exit(0xfe);
}


/*
 * cmp_evaluate_expr(char* value,_ULONG lineno)
 *
 * This will evaluate expression [value]. It expects to be at line [lineno].
 * It will add it directly to the script code. It will always push the
 * value on the stack.
 *
 */
void
cmp_evaluate_expr(char* value,_ULONG lineno) {
    char*  ptr;
    char*  temptr;
    char   cmp_temp[MC_MAX_PAIR_LENGTH];
    char   cmp_fullthing[MC_MAX_PAIR_LENGTH];
    char   cmp_objnametmp[MC_MAX_PAIR_LENGTH];
    char   cmp_methodtmp[MC_MAX_PAIR_LENGTH];
    _UINT  methodno,nofparms;
    _UCHAR expr,old_expr;
    _ULONG objectno;

    /* first put it all into [cmp_temp], ignoring spaces */
    gad_copynochar(value,cmp_temp,' ');
    temptr=cmp_temp;old_expr=MC_COMP_UNKNOWN;

    /* keep scanning for an operator and adding them */
    while (((ptr=gad_scanforoperator(temptr,&expr))!=NULL)) {
        /* there's an operator in the string. copy everything before it */
        strncpy(cmp_fullthing,temptr,(ptr-temptr));
        cmp_fullthing[(ptr-temptr)]=0;

        /* nuke spaces */
        gad_striptrailingspaces(cmp_fullthing);

        /* is the length zero? */
        if(strlen(cmp_fullthing)) {
            /* no, evaluate it */
            cmp_evaluate_var(cmp_fullthing,lineno);
        }

        /* do we have a valid old expression? */
        if (old_expr!=MC_COMP_UNKNOWN) {
            /* yup. add the operator */
            cmp_add_expr_operator(lineno,old_expr);
        }

        /* use this as the old expression */
        old_expr=expr;

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

    /* do we have a valid old expression? */
    if (old_expr!=MC_COMP_UNKNOWN) {
        /* yup. add the operator */
        cmp_add_expr_operator(lineno,old_expr);
    }
}

/*
 * cmp_add_param(_UINT paramtype,char* param,_ULONG lineno)
 *
 * This will add parameter [param] with type [param]. Line number [lineno]
 * will be used in error messages. [obj] should be a pointer to the object
 * that is currently used, or NULL if none if used.
 *
 */
void
cmp_add_param(_UINT paramtype,char* param,_ULONG lineno,MC_OBJECT* obj) {
    char temp[MC_MAX_PAIR_LENGTH];
    char temp2[MC_MAX_PAIR_LENGTH];
    MC_OBJECT* tmp_obj;
    char* ptr;
    _ULONG i,objid;
    _UCHAR c;

    /* figure out the parameter type */
    switch(paramtype) {
        case MC_TYPE_STRING: /* string */
                             cmp_cscript_pushimm(cmp_cscript_datasize);
                             cmp_cscript_data_addstring(param);
                             break;
       case MC_TYPE_QSTRING: /* string with quotes */
                             /* get the string without quotes */
                             gad_copynochar(param,temp,'"');

                             /* add the result */
                             cmp_cscript_pushimm(cmp_cscript_datasize);
                             cmp_cscript_data_addstring(temp);
                             break;
         case MC_TYPE_DWORD: /* unsigned integer */
                             i=strtoul(param,&ptr,0);

                             /* did it work? */
                             if(ptr==param) {
                                 /* no. give error */
                                 fprintf(stderr,"%s[%lu]: cannot parse integer '%s'\n",cmp_scriptname,lineno,param);
                                 cmp_cleanup();
                                 exit(1);
                             }

                             /* add it to the script */
                             cmp_cscript_pushimm(i);
                             break;
         case MC_TYPE_ELONG: /* evaluated */
                             cmp_evaluate_expr(param,lineno);
                             break;
     case MC_TYPE_DIRECTION: /* direction */
                             /* try to resolve the direction */
                             if((i=gad_resolvedirection(param))==DIR_UNKNOWN) {
                                 /* this failed. die */
                                 fprintf(stderr,"%s[%lu]: cannot parse direction '%s'\n",cmp_scriptname,lineno,param);
                                 cmp_cleanup();
                                 exit(1);
                             }

                             /* add it to the script */
                             cmp_cscript_pushimm(i);
                             break;
       case MC_TYPE_BOOLEAN: /* boolean */
                             /* try to resolve the boolean */
                             if((i=gad_resolveboolean(param))==MC_BOOL_UNKNOWN) {
                                 /* this failed. die */
                                 fprintf(stderr,"%s[%lu]: cannot parse boolean '%s'\n",cmp_scriptname,lineno,param);
                                 cmp_cleanup();
                                 exit(1);
                             }

                             /* add it to the script */
                             cmp_cscript_pushimm(i);
                             break;
          case MC_TYPE_PROP: /* property */
                             /* if the user gave a NULL property, do that */
                             if (!strcmp(param,MC_PROPERTY_NULL)) {
                                 /* push the NULL property value */
                                 i=SCRIPT_NULLPROPERTY;
                             } else {
                                 if(obj==NULL) {
                                     /* no object was given. die */
                                     fprintf(stderr,"%s[%lu]: property of a NULL object requested\n",cmp_scriptname,lineno);
                                     cmp_cleanup();
                                     exit(1);
                                 }
                                 /* scan the superclass */
                                 if((i=cmp_scan_superclass_method(obj->superclass,param))==MC_NOCLASS) {
                                     /* no property was found. die */
                                     fprintf(stderr,"%s[%lu]: property '%s' of object '%s' (superclass '%s') not found\n",cmp_scriptname,lineno,param,obj->name,cmp_superclass[obj->superclass].name);
                                     cmp_cleanup();
                                     exit(1);
                                 }
                             }
                             /* add it to the script */
                             cmp_cscript_pushimm(i);
                             break;
       case MC_TYPE_OBJPROP: /* object + property pair */
                             /* is it the NULL thing? */
                             if (!strcmp(param,MC_PROPERTY_NULL)) {
                                 /* yup. add the null stuff */
                                 cmp_cscript_pushimm(MC_NOBJECT);
                                 cmp_cscript_pushimm(SCRIPT_NULLPROPERTY);
                             } else {
                                 /* no. scan for the dot */
                                 if(strchr(param,'.')==NULL) {
                                     /* there isn't a dot in the expression! complain */
                                     fprintf(stderr,"%s[%lu]: '.' expected\n",cmp_scriptname,lineno);
                                     exit(1);
                                 }
                                 /* copy everything before the dot in [temp] */
                                 gad_copyuntil(param,temp,'.');
                                 /* and everything after the dot in [temp2] */
                                 strcpy(temp2,(char*)(strchr(param,'.')+1));

                                 /* scan for the object */
                                 if((objid=cmp_find_object(temp))==MC_NOBJECT) {
                                     /* it was not found. die */
                                     fprintf(stderr,"%s[%lu]: object '%s' not found\n",cmp_scriptname,lineno,temp);
                                     exit(1);
                                 }

                                 /* add the object id */
                                 cmp_cscript_pushimm(objid);

                                 /* make a pointer to the object */
                                 tmp_obj=(struct MC_OBJECT*)&(cmp_object[objid]);

                                 /* scan the superclass */
                                 if((i=cmp_scan_superclass_method(tmp_obj->superclass,temp2))==MC_NOCLASS) {
                                     /* no property was found. die */
                                     fprintf(stderr,"%s[%lu]: property '%s' of object '%s' (superclass '%s') not found\n",cmp_scriptname,lineno,temp2,tmp_obj->name,cmp_superclass[tmp_obj->superclass].name);
                                     cmp_cleanup();
                                     exit(1);
                                 }

                                 /* add it to the script */
                                 cmp_cscript_pushimm(i);
                             }
                             break;
                    default: /* we don't know this type. die */
                             fprintf(stderr,"%s[%lu]: INTERNAL ERROR: unknown parameter type %u\n",cmp_scriptname,lineno,paramtype);
                             exit(0xfe);
   }
}

/*
 * cmp_add_params(MC_OBJECT* obj,char* params,char* line,_ULONG lineno);
 *
 * This will add the parameters, following the declarations in [params]. It
 * uses line [line]. It expects to be at line [lineno]. [obj] should be a
 * pointer to the object that is currently used, or NULL if none if used.
 *
 */
void
cmp_add_params(char* params,char* line,_ULONG lineno,MC_OBJECT* obj) {
    char   cmp_temp[MC_MAX_PAIR_LENGTH];
    char   cmp_temptemp[MC_MAX_PAIR_LENGTH];
    char   cmp_paramtemp[MC_MAX_PAIR_LENGTH];
    char*  ptr;
    _ULONG paramno,i;
    _UINT  paramtype;

    /* if there are no parameters, just get out */
    if(!strlen(params)) return;

    /* copy the line into [cmp_temptemp], and nuke the spaces */
    strcpy(cmp_temptemp,line);
    gad_striptrailingspaces(cmp_temptemp);

    /* set [ptr] to the beginning of the params */
    ptr=strchr(cmp_temptemp,' ');
    if (ptr!=NULL) {
        while(*ptr==' ') ptr++;
    } else {
        ptr=cmp_temptemp;
    }

    /* do all parameters one by one */
    paramno=0;
    for(i=0;i<strlen(params);i++) {
        /* get the thingy */
        sscanf((char*)(params+i),"%s",&cmp_temp);
        i+=strlen(cmp_temp);

        /* copy the parameter to [cmp_paramtemp] */
        gad_copyuntil(ptr,cmp_paramtemp,',');

        /* increment the source pointer */
        ptr+=(strlen(cmp_paramtemp)+1);

        /* resolve the parameter string */
        paramtype=gad_resolveparamstring(cmp_temp);

        /* is the type known? */
        if(paramtype==MC_TYPE_UNKNOWN) {
            /* nope. die */
            fprintf(stderr,"%s[%lu]: (INTERNAL ERROR) parameter string '%s' is unresolvable\n",cmp_scriptname,lineno,cmp_temp);
            cmp_cleanup();
            exit(0xfe);
        }

        /* add the parameter */
        cmp_add_param(paramtype,cmp_paramtemp,lineno,obj);

        /* next parameter */
        paramno++;
    }
}

/*
 * cmp_dobjectassignement(char* line,_ULONG lineno,_UINT* status)
 *
 * This will compile object assignement line [line]. Line number [lineno]
 * will be used in error messages. [status] should be a pointer to a variable
 * that can be modified. It will return the number of lines to skip extra.
 *
 */
_ULONG
cmp_dobjectassignment(char* line,_ULONG lineno,_UINT* status) {
    char*  ptr;
    char*  ptr2;
    char   cmp_objtemp[MC_MAX_PAIR_LENGTH];
    char   cmp_methodtemp[MC_MAX_PAIR_LENGTH];
    char   cmp_value[MC_MAX_PAIR_LENGTH];
    struct MC_OBJECT* obj;
    struct MC_PROPERTY* pt;
    _ULONG objid,propno;

    /* check for the equal sign */
    if((ptr=strchr(line,'.'))==NULL) {
        /* there wasn't a dot! die */
        fprintf(stderr,"%s[%lu]: (INTERNAL ERROR)cmp_dobjectassignment() called without a object.method=qqq pair!\n",cmp_scriptname,lineno);
        exit(0xfe);
    }

    /* check for the equal sign */
    if((ptr2=strchr(line,'='))==NULL) {
        /* there wasn't a dot! die */
        fprintf(stderr,"%s[%lu]: (INTERNAL ERROR)cmp_dobjectassignment() called without a object.method=qqq pair!\n",cmp_scriptname,lineno);
        exit(0xfe);
    }

    /* dump all stuff before the dot into [cmp_objtemp] */
    gad_copyuntil(cmp_pair[0],cmp_objtemp,'.');
    gad_striptrailingspaces(cmp_objtemp);

    /* dump all stuff after the dot into [cmp_methodtemp] */
    gad_copyuntil((char*)(ptr+1),cmp_methodtemp,'=');
    gad_striptrailingspaces(cmp_methodtemp);

    /* dump all stuff after the = into [cmp_value] */
    strcpy(cmp_value,(char*)(ptr2+1));
    gad_striptrailingspaces(cmp_value);

    /* scan for the object */
    if((objid=cmp_find_object(cmp_objtemp))==MC_NOBJECT) {
        /* it was not found. die */
        fprintf(stderr,"%s[%lu]: object '%s' not found\n",cmp_scriptname,lineno,cmp_objtemp);
        exit(1);
    }

    /* make a pointer to the object */
    obj=(struct MC_OBJECT*)&(cmp_object[objid]);

    /* look up the method */
    propno=cmp_scan_superclass_property(obj->superclass,cmp_methodtemp);

    /* did this work? */
    if(propno==MC_NOPROP) {
        /* no. die */
        fprintf(stderr,"%s[%lu]: unknown object property '%s'\n",cmp_scriptname,lineno,cmp_methodtemp);
        exit(1);
    }

    /* make a pointer to the property */
    pt=(struct MC_PROPERTY*)&(cmp_superclass[obj->superclass].properties[propno]);

    cmp_evaluate_expr(cmp_value,lineno);

    /* add the opcodes */
    cmp_cscript_addbyte(OPCODE_LDP);

    /* add the object number */
    cmp_cscript_addword(objid);

    /* and add the property number */
    cmp_cscript_addword(pt->code);

    return 0;
}

/*
 * cmp_dobjectopcode(char* line,_ULONG lineno,_UINT* status)
 *
 * This will compile object line [line]. Line number [lineno] will be used in
 * error messages. It will return the number of lines to skip EXTRA! [status]
 * should be a pointer to a variable that can be modified.
 *
 */
_ULONG
cmp_dobjectopcode(char* line,_ULONG lineno,_UINT* status) {
    char*  ptr;
    char   cmp_objtemp[MC_MAX_PAIR_LENGTH];
    char   cmp_methodtemp[MC_MAX_PAIR_LENGTH];
    _ULONG objid;
    _UINT  methodno;
    struct MC_SUPERCLASS* sc;
    struct MC_METHOD* mt;
    struct MC_OBJECT* obj;

    /* check for the dot in the first pair */
    if((ptr=strchr(cmp_pair[0],'.'))==NULL) {
        /* there wasn't a dot! die */
        fprintf(stderr,"%s[%lu]: (INTERNAL ERROR)cmp_dobjectopcode() called without a object.method pair!\n",cmp_scriptname,lineno);
        exit(0xfe);
    }

    /* is there an equal sign in it? */
    if(strchr(line,'=')!=NULL) {
        /* no. handle it as a variable assignment */
        return cmp_dobjectassignment(line,lineno,status);
    }

    /* dump all stuff before the dot into [cmp_objtemp] */
    gad_copyuntil(cmp_pair[0],cmp_objtemp,'.');

    /* dump all stuff after the dot into [cmp_methodtemp] */
    strcpy(cmp_methodtemp,(char*)(ptr+1));

    /* nuke *all* spaces */
    gad_striptrailingspaces(cmp_objtemp);
    gad_striptrailingspaces(cmp_methodtemp);

    /* scan for the object */
    if((objid=cmp_find_object(cmp_objtemp))==MC_NOBJECT) {
        /* it was not found. die */
        fprintf(stderr,"%s[%lu]: object '%s' not found\n",cmp_scriptname,lineno,cmp_objtemp);
        exit(1);
    }

    /* make a pointer to the object */
    obj=(struct MC_OBJECT*)&(cmp_object[objid]);

    /* make a pointer to the superclass */
    sc=(struct MC_SUPERCLASS*)&(cmp_superclass[obj->superclass]);

    /* scan for the method */
    if((methodno=cmp_scan_superclass_method(obj->superclass,cmp_methodtemp))==MC_NOENTRY) {
        /* it was not found. die */
        fprintf(stderr,"%s[%lu]: unknown method '%s'\n",cmp_scriptname,lineno,cmp_methodtemp);
        exit(1);
    }

    /* make a pointer to the method */
    mt=(struct MC_METHOD*)&(sc->methods[methodno]);

    /* first add the parameters */
    cmp_add_params(mt->params,line,lineno,obj);

    /* add the opcode */
    cmp_cscript_addbyte(OPCODE_CLO);

    /* add the object id */
    cmp_cscript_addword(objid);

    /* add the function number */
    cmp_cscript_addbyte(mt->opcode);

    /* no extra skipping needed */
    return 0;
}

/*
 * cmp_checkcomments(char* line)
 *
 * This will check line [line] for comments. It will return zero if nothing
 * was commented, or non-zero if this line should be ignored.
 *
 */
_ULONG
cmp_checkcomments(char* line) {
    char   cmp_temp[MC_MAX_PAIR_LENGTH];

    /* copy the line to [cmp_temp] */
    strcpy(cmp_temp,line);

    /* nuke *all* trailing spaces */
    gad_striptrailingspaces(cmp_temp);

    /* does the line begin with a semicolon? */
    if(cmp_temp[0]==';') {
        /* yes, it's a comment */
        return 1;
    }

    /* does the line begin with two slashes? */
    if((cmp_temp[0]=='/')&&(cmp_temp[1]=='/')) {
        /* yes, it's a comment */
        return 1;
    }

    /* is the line empty? */
    if(!strlen(cmp_temp)) {
        /* yes. treat empty lines also as comments */
        return 1;
    }

    /* it's not a comment */
    return 0;
}

/*
 * cmp_dokernelfunc(char* line,_ULONG lineno,_UINT* status)
 *
 * This will compile object line [line]. Line number [lineno] will be used in
 * error messages. It will return the number of lines to skip, or zero if the
 * line could not be parsed. [status] should be a pointer to a variable that
 * can be modified.
 *
 */
_ULONG
cmp_dokernelfunc(char* line,_ULONG lineno,_UINT* status) {
    _ULONG funcno;
    struct MC_FUNC* fc;

    /* can we find the kernel function? */
    if((funcno=cmp_find_kernelfunc(cmp_pair[0]))==MC_NOFUNC) {
        /* nope. say it didn't parse */
        return 0;
    }

    /* make a pointer to the kernel function info */
    fc=(MC_FUNC*)&(cmp_kernelfuncs[funcno]);

    /* add the parameters */
    cmp_add_params(fc->params,line,lineno,NULL);

    /* add the opcode */
    cmp_cscript_addbyte(OPCODE_CLK);
    cmp_cscript_addbyte(fc->funcno);

    return 1;
}

/*
 * cmp_compilexpression(char* expr,_ULONG lineno,_ULONG start_offset)
 *
 * This will compile expression [expr], at line number [lineno]. It will
 * return zero if a JC (Jump flag Clear) should be used for a positive check,
 * otherwise non-zero, indicating a JS (Jump flag Set). [statype] should
 * indicate whether this is an 'if' or 'while' line. [fix_offsets] should be
 * a pointer to the fixed offsets, [nof_fixoffsets] should be a pointer to a
 * value.
 *
 */
void
cmp_compilexpression(char* expr,_ULONG lineno,_UCHAR statype,_ULONG** fix_offsets,_ULONG* nof_fixoffsets) {
    char* ptr;
    _UCHAR type,len,cond;
    char cmp_part1[MC_MAX_PAIR_LENGTH];
    char cmp_part2[MC_MAX_PAIR_LENGTH];

    /* scan for the operator */
    ptr=gad_scanforcomperator(expr,&type,&len);

    /* was anything found? */
    if(ptr==NULL) {
        /* no. default to != 0 */
        type=MC_COMP_NOTEQUAL;strcpy(cmp_part2,"0");
        gad_copyuntil(expr,cmp_part1,*ptr);
    } else {
        /* get the stuff */
        gad_copyuntil(expr,cmp_part1,*ptr);
        strcpy(cmp_part2,(char*)(ptr+len));
    }

    /* add the values! */
    cmp_evaluate_expr(cmp_part1,lineno);
    cmp_evaluate_expr(cmp_part2,lineno);

    switch (type) {
        case MC_COMP_EQUAL: /* equal */
                            cmp_cscript_addbyte(OPCODE_EQ); cond=1;
                            break;
     case MC_COMP_NOTEQUAL: /* equal */
                            cmp_cscript_addbyte(OPCODE_EQ); cond=0;
                            break;
           case MC_COMP_GE: /* greater/equal */
                            cmp_cscript_addbyte(OPCODE_GE); cond=1;
                            break;
           case MC_COMP_LE: /* greater/equal */
                            cmp_cscript_addbyte(OPCODE_LE); cond=1;
                            break;
           case MC_COMP_GT: /* greater than */
                            cmp_cscript_addbyte(OPCODE_GT); cond=1;
                            break;
           case MC_COMP_LT: /* less than */
                            cmp_cscript_addbyte(OPCODE_LT); cond=1;
                            break;
                   default: /* it was unknown! die */
                            fprintf(stderr,"%s[%lu]: (INTERNAL ERROR)no comperator found, even tho we default to !=0 ?!?\n",cmp_scriptname,lineno);
                            exit(0xfe);
    }

    /* resize memory buffer */
    if((*fix_offsets=(_ULONG*)cmp_realloc(*fix_offsets,sizeof(_ULONG)*(*nof_fixoffsets),sizeof(_ULONG)*(*nof_fixoffsets+1)))==NULL) {
        /* this failed. die */
        fprintf(stderr,"%s[%lu]: out of memory while trying to resize offset buffer\n",cmp_scriptname,lineno);
        exit(0xfe);
    }

    /* while line? */
    if(statype==MC_CTYPE_WHILE) {
        /* yup. need to do this positive or negative? */
        if(!cond) {
            /* positive. add a positive jump */
            cmp_cscript_addbyte(OPCODE_JS);
        } else {
            /* negative. add a negative jump */
            cmp_cscript_addbyte(OPCODE_JC);
        }
        /* handle offset */
        *fix_offsets[*nof_fixoffsets]=cmp_cscript_codesize;
        /* add a junk offset */
        cmp_cscript_addlong(MC_NOFFSET);
    } else {
        /* if line */
        /* need to do this positive or negative? */
        if(!cond) {
            /* negative. add a positive jump */
            cmp_cscript_addbyte(OPCODE_JS);
        } else {
            /* positive. add a negative jump */
            cmp_cscript_addbyte(OPCODE_JC);
        }
        /* handle offset */
        *fix_offsets[*nof_fixoffsets]=cmp_cscript_codesize;
        /* add a junk byte */
        cmp_cscript_addlong(MC_NOFFSET);
    }

    /* increment number */
    *nof_fixoffsets=*nof_fixoffsets+1;
}

/*
 * cmp_doifwhile(char* line,_ULONG lineno,_UINT* status)
 *
 * This will handle a 'if' and 'while' lines. It will need to set [status] to
 * a status code. It will return the number of lines to skip extra.
 *
 */
_ULONG
cmp_doifwhile(char* line,_ULONG lineno,_UINT* status) {
    char cmp_temp[MC_MAX_PAIR_LENGTH];
    char cmp_code[MC_MAX_PAIR_LENGTH];
    char cmp_expr[MC_MAX_PAIR_LENGTH];
    char cmp_oper[MC_MAX_PAIR_LENGTH];
    char* ptr;
    char* ptr2;
    char* mainptr;
    _UCHAR type,expr;
    _ULONG start_offset,i,k,j;
    _ULONG* fix_offsets;
    _ULONG  nof_fix_offsets,linebytes;
    _ULONG  lines_to_skip;

    /* copy the line to [cmp_temp] and zap all spaces */
    strcpy(cmp_temp,line);
    gad_striptrailingspaces(cmp_temp);

    /* set flag whether it's a 'if' or 'while' thingy */
    type=MC_CTYPE_UNKNOWN;

    /* is this an if thingy? */
    if(!strnicmp(cmp_temp,MC_IF,strlen(MC_IF))) type=MC_CTYPE_IF;
    if(!strnicmp(cmp_temp,MC_WHILE,strlen(MC_WHILE))) type=MC_CTYPE_WHILE;

    /* if nothing was found, die */
    if(type==MC_CTYPE_UNKNOWN) {
        fprintf(stderr,"%s[%lu]: (INTERNAL ERROR)cmp_doifwhile() called without an 'if' or 'while'!\n",cmp_scriptname,lineno);
        exit(0xfe);
    }

    /* scan for a '(' */
    if((ptr=strchr(cmp_temp,'('))==NULL) {
        /* nothing there... die */
        fprintf(stderr,"%s[%lu]: '(' expected\n",cmp_scriptname,lineno);
        exit(1);
    }

    /* zap everything before the '(' */
    strcpy(cmp_temp,(char*)(ptr));

    /* check for a ')' */
    if(strchr(cmp_temp,')')==NULL) {
        /* nothing there... die */
        fprintf(stderr,"%s[%lu]: ')' expected\n",cmp_scriptname,lineno);
        exit(1);
    }

    /* scan the last ) */
    ptr=cmp_temp;
    while((ptr2=strchr(ptr,')'))!=NULL) {
        ptr=(char*)(ptr2+1);
    }

    /* copy everything after the last ')' to [cmp_code] */
    *ptr=0;
    strcpy(cmp_code,(char*)(ptr+1));

    start_offset=cmp_cscript_codesize;

    /* zap the fix offset buffer */
    nof_fix_offsets=0; fix_offsets=NULL;

//    printf(">>");

    /* now, handle the entire line */
    mainptr=cmp_temp;
    while(*mainptr!=0) {
        /* get a (blah) thing from the list */
        ptr=mainptr;
        while(*ptr=='(') *ptr++;

        /* search for the ')' of it */
        if((ptr2=strchr(ptr,')'))==NULL) {
            /* there wasn't any! die */
            fprintf(stderr,"%s[%lu]: ')' expected\n",cmp_scriptname,lineno);
            exit(1);
        }
        gad_copyuntil(ptr,cmp_expr,')');

        /* if there is an expression, handle it */
        if(strlen(cmp_expr)) {
            /* handle the expression */
            cmp_compilexpression(cmp_expr,lineno,type,&fix_offsets,&nof_fix_offsets);
        }

        mainptr=(char*)ptr2+1;
        /* if there is still data at [mainptr], it must be an operator */
        if(*mainptr!=0) {
            /* figure out what it is */
            if(strchr(mainptr,'(')!=NULL) {
                /* this is data after it */
                gad_copyuntil(mainptr,cmp_oper,'(');
  //              printf("operator [%s] ",cmp_oper);
                mainptr+=strlen(cmp_oper);
            }
        }

//        printf("expr [%s] ",cmp_expr);
    }

    /* compile the code */
    j=cmp_cscript_codesize;
    /* is it a '{' indicating more code? */
    if(!strcmp(cmp_code,"{")) {
        /* yup. do it */
        i=lineno+1;
        while(1){
            /* if we are out of range, die */
            if(i>cmp_noflines) {
                fprintf(stderr,"%s[%lu]: {} block never terminated\n",cmp_scriptname,lineno);
                exit(1);
            }

            ptr=(char*)(cmp_filebuf+cmp_linetab[i]);
            strcpy(cmp_temp,ptr);
            gad_striptrailingspaces(cmp_temp);

            if (!strcmp(cmp_temp,"}")) break;

            i+=(cmp_compileline(ptr,i,status)+1);
        }

        lines_to_skip=i-lineno;
    } else {
        /* no, just a single line. phew! */
        lines_to_skip=cmp_compileline(cmp_code,lineno,status);
    }
    linebytes=cmp_cscript_codesize-j;

    /* fix the offset */
    for(i=0;i<nof_fix_offsets;i++) {
        /* get it */
        j=cmp_cscript_codesize;
        /* if this is a while, add skip the jump that will come after it */
        if(type==MC_CTYPE_WHILE) {
            j+=5;
        }
        /* fix it */
        cmp_cscript_code[fix_offsets[i]]=(j&0xffff)&0xff;
        cmp_cscript_code[fix_offsets[i]+1]=(j&0xffff)>>8;
        cmp_cscript_code[fix_offsets[i]+2]=(j>>16)&0xff;
        cmp_cscript_code[fix_offsets[i]+3]=(j>>16)>>8;
    }

    /* if this is a while, jump back */
    if(type==MC_CTYPE_WHILE) {
        cmp_cscript_addbyte(OPCODE_JMP);
        cmp_cscript_addlong(start_offset);
    }

    /* free the offsets */
    if (fix_offsets!=NULL) free (fix_offsets);

    /* return the number of lines */
    return lines_to_skip;
}


/*
 * cmp_compileline(char* line,_ULONG lineno)
 *
 * This will compile line [line]. Line number [lineno] will be used in error
 * messages. It will return the number of lines to skip EXTRA! [status]
 * should be a pointer to a variable that can be modified.
 *
 */
_ULONG
cmp_compileline(char* line,_ULONG lineno,_UINT *status) {
    /* split the string */
    cmp_splitpairs(line);

    /* first of all, check comments */
    if(cmp_checkcomments(line)) return 0;

    /* check declarations */
    if(cmp_checkdeclarations(line,lineno,status)) return 0;

    /* does the line start with an 'if' or 'while'? */
    if((!strnicmp(cmp_pair[0],MC_IF,strlen(MC_IF)))||
       (!strnicmp(cmp_pair[0],MC_WHILE,strlen(MC_WHILE)))) {
        /* yes. special handling for these ones! */
        return cmp_doifwhile(line,lineno,status);

    }

    /* does the first part contain a dot? */
    if(strchr(cmp_pair[0],'.')!=NULL) {
        /* yes. let the object thing do it */
        return cmp_dobjectopcode(line,lineno,status);
    }

    /* maybe it will do as a normal kernel function? */
    if(cmp_dokernelfunc(line,lineno,status)) return 0;

    /* we cannot parse this line! */
    fprintf(stderr,"%s[%lu]: could not parse line\n",cmp_scriptname,lineno);
    exit(1);

    return 0;
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
    status=MC_STATUS_NORMAL;cmp_curobjid=MC_NOBJECT;cmp_curmethodid=MC_NOMETHOD;
    for(i=1;i<=cmp_noflines;i++) {
        /* first, create a pointer to the current line */
        line=(char*)(cmp_filebuf+cmp_linetab[i]);

        /* split the string */
        cmp_splitpairs(line);

        i+=cmp_compileline(line,i,&status);
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

/*
 * cmp_writefile(char* fname)
 *
 * This will write the compiled script to [fname].
 *
 */
void
cmp_writefile(char* fname) {
    FILE *f;
    struct MC_FILEHEADER hdr;
    struct MC_SUPERCLASS* sc;
    _ULONG i,j,val,methodid;

    /* do we have an init method? */
    if(cmp_initoffset==MC_NOFFSET) {
        /* no. show error */
        fprintf(stderr,"cmp_writefile(): there is no init method declared\n");
        exit(1);
    }

    /* do we have a done method? */
    if(cmp_doneoffset==MC_NOFFSET) {
        /* no. show error */
        fprintf(stderr,"cmp_writefile(): there is no done method declared\n");
        exit(1);
    }

    /* do we have a run method? */
    if(cmp_runoffset==MC_NOFFSET) {
        /* no. show error */
        fprintf(stderr,"cmp_writefile(): there is no run method declared\n");
        exit(1);
    }

    /* create the file */
    if((f=fopen(fname,"wb"))==NULL) {
        /* this failed. die */
        fprintf(stderr,"cmp_writefile(): unable to create file '%s'\n",fname);
        exit(1);
    }

    /* construct the header */
    memset(&hdr,0,sizeof(hdr));
    hdr.magic=MC_HEADER_MAGIC;
    hdr.versiono=MC_HEADER_VERSIONO;
    hdr.codesize=cmp_cscript_codesize;
    hdr.datasize=cmp_cscript_datasize;
    hdr.dataoffset=sizeof(MC_FILEHEADER)+cmp_cscript_codesize;
    hdr.nofobjects=cmp_nofobjects;
    hdr.init_offset=cmp_initoffset;
    hdr.done_offset=cmp_doneoffset;
    hdr.run_offset=cmp_runoffset;

    /* write it */
    if(!fwrite(&hdr,sizeof(hdr),1,f)) {
        /* this failed. die */
        fprintf(stderr,"cmp_writefile(): unable to write to file '%s'\n",fname);
        exit(1);
    }
    /* write the code */
    if(!fwrite(cmp_cscript_code,cmp_cscript_codesize,1,f)) {
        /* this failed. die */
        fprintf(stderr,"cmp_writefile(): unable to write to file '%s'\n",fname);
        exit(1);
    }

    /* is there data? */
    if(cmp_cscript_datasize) {
        /* yup, write it */
        if(!fwrite(cmp_cscript_data,cmp_cscript_datasize,1,f)) {
            /* this failed. die */
            fprintf(stderr,"cmp_writefile(): unable to write to file '%s'\n",fname);
            exit(1);
        }
    }

    /* go through the objects one by one */
    for(i=0;i<cmp_nofobjects;i++) {
        if(!fwrite(cmp_object[i].name,MC_MAX_OBJECT_LEN,1,f)) {
            /* this failed. die */
            fprintf(stderr,"cmp_writefile(): unable to write to file '%s'\n",fname);
            exit(1);
        }
        if(!fwrite((void*)&cmp_object[i].flags,sizeof(_ULONG),1,f)) {
            /* this failed. die */
            fprintf(stderr,"cmp_writefile(): unable to write to file '%s'\n",fname);
            exit(1);
        }
        /* make a pointer to the superclass */
        sc=(struct MC_SUPERCLASS*)&(cmp_superclass[cmp_object[i].superclass]);

        if(!fwrite(sc->name,MC_MAX_SUPERCLASS_LEN,1,f)) {
            /* this failed. die */
            fprintf(stderr,"cmp_writefile(): unable to write to file '%s'\n",fname);
            exit(1);
        }
        if(!fwrite((void*)&sc->nofmethods,sizeof(_ULONG),1,f)) {
            /* this failed. die */
            fprintf(stderr,"cmp_writefile(): unable to write to file '%s'\n",fname);
            exit(1);
        }
        /* now dump all offsets and flags */
        for(j=0;j<sc->nofmethods;j++) {
            /* does it exists in the object? */
            if((methodid=cmp_scan_object_method(i,sc->methods[j].name))==MC_NOMETHOD) {
                /* it's not here. inherit it! */
                val=MC_NOFFSET;
            } else {
                val=cmp_object[i].methods[methodid].start_byte;
            }
            /* add the offset */
            if(!fwrite(&val,sizeof(_ULONG),1,f)) {
                /* this failed. die */
                fprintf(stderr,"cmp_writefile(): unable to write to file '%s'\n",fname);
                exit(1);
            }
            /* add the flags */
            if(!fwrite(&sc->methods[j].flags,sizeof(_ULONG),1,f)) {
                /* this failed. die */
                fprintf(stderr,"cmp_writefile(): unable to write to file '%s'\n",fname);
                exit(1);
            }
        }
    }

    /* close the file */
    fclose(f);
}
