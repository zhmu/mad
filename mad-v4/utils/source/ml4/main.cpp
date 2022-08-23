/*
 *                             The MAD Linker
 *                               Version 2.0
 *
 *                          (c) 1999 The MAD Crew
 *
 */
#include <stdlib.h>
#include <string.h>
#ifdef UNIX
#include <sys/types.h>
#include <dirent.h>
#endif
#include "ml.h"
#include "../../../source/include/objman.h"
#include "../../../source/include/script.h"
#include "../../../source/include/types.h"

_UCHAR verbose;
_ULONG nofiles,nofsharedobj,noflocalobjs;

char   filename[ML_MAX_FILES][ML_MAX_FILENAME_LEN];
char   rootscriptname[ML_MAX_FILENAME_LEN];

MC_OBJECT *object;
_ULONG    nofobjects;

ML_SUPERCLASS   *superclass;
_ULONG          nofsuperclasses;

/*
 * showbuildinfo()
 *
 * This will show information about the current build
 *
 */
void
showbuildinfo() {
    fprintf(stderr,"The MAD Linker version 4.0\n");
    fprintf(stderr,"(c) 1999, 2000 The MAD Crew\n");
}

/*
 * showhelp()
 *
 * This will show help about the switches
 *
 */
void
showhelp() {
    fprintf(stderr,"Usuage: ML [switches] [rootscript]\n\n");
    fprintf(stderr,"Switches:\n");
    fprintf(stderr,"    -? or -h    This help\n");
    fprintf(stderr,"    -b          Show build information\n");
    fprintf(stderr,"    -v          Verbose Mode\n");
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

    verbose=0; rootscriptname[0]=0;
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
                          fprintf(stderr,"ML: unknown parameter -- %c\n",argv[i][1]);
                          fprintf(stderr,"use -h or -? for help\n");
                          exit(0xff);
                          /* NOTREACHED */
            }
        } else {
            /* last parameter? */
            if(i!=(argc-1)) {
                /* nope. complain about the filename not being the last parameter */
                fprintf(stderr,"ML: root script file must be the last parameter\n");
                exit(1);
            }
        }
    }

    /* copy it */
    strcpy(rootscriptname,argv[argc-1]);

    /* was a root script specified? */
    if (!rootscriptname[0]) {
        /* no. show error */
        printf("You must specify a root script!\n");
        exit(1);
    }
}

/*
 * getfilenames()
 *
 * This will get the names of all compiled MAD files in the current directory.
 * They will be dumped into filename[].
 *
 */
void
getfilenames() {
    _UINT ok;
    char* ptr;

    #ifdef DJGPP
        struct ffblk ffblk;

        /* No files so far */
        nofiles=0;

        ok=findfirst(ML_NAME_CSCRIPT,&ffblk,0);
        while(!ok) {
            strcpy(filename[nofiles],ffblk.ff_name);

            nofiles++;

            ok=findnext(&ffblk);
        }

        /* did we find some files? */
        if(!nofiles) {
            /* no. die */
            printf("ML: No input files found\n");
            exit(1);
        }
    #elif UNIX
        DIR* dir;
        struct dirent *entry;
        /* UNIX does it differently -- thanks Christoph Reichenbach (FreeSCI) */

        /* No files so far */
        nofiles=0;

        /* Open the directory  */
        if((dir=opendir("."))==NULL) {
            /* This failed. Die */
            printf("ML: Unable to open current directory\n");
            exit(1);
        }

        /* Keep reading files */
        while((entry=readdir(dir))!=NULL) {
            /* It's the file! Is it a script file? */
            if((ptr=strchr(entry->d_name,'.'))!=NULL) {
                if(!strcasecmp(ptr,MC_EXT_CM)) {
                    /* this is a script! add it */
                    strcpy(filename[nofiles],entry->d_name);

                    nofiles++;
                }
            }
        }

        /* Close the directory */
        closedir(dir);
    #endif
}

/*
 * find_superclass(char* classname)
 *
 * This will scan the superclass chain for a superclass named [classname]. It
 * will return the superclass ID if it is found, or MC_NOCLASS if nohing was
 * found.
 *
 */
_ULONG
find_superclass(char* classname) {
    struct ML_SUPERCLASS* sc;
    _ULONG i;

    /* scan 'em all */
    for(i=0;i<nofsuperclasses;i++) {
        /* make a pointer to the superclass data */
        sc=(struct ML_SUPERCLASS*)&(superclass[i]);

        /* is this The Chosen One(tm)? */
        if(!strcmp(sc->name,classname)) {
            /* yup! return the id */
            return i;
        }
    }
    /* it was not found. return MC_NOCLASS */
    return MC_NOCLASS;
}

/*
 * add_superclass(char* classname,_ULONG nofmethods)
 *
 * This will add superclass [classname] to the superclass chain. It will also
 * allocate [nofmethods] methods for it. It will return the superclass ID.
 *
 */
_ULONG
add_superclass(char* classname,_ULONG nofmethods) {
    struct ML_SUPERCLASS* sc;

    /* does the object already exists? */
    if(find_superclass(classname)!=MC_NOCLASS) {
        /* yup. die */
        fprintf(stderr,"add_superclass(): superclass '%s' already exists\n",classname);
        exit(1);
    }

    /* resize the buffer */
    if((superclass=(ML_SUPERCLASS*)realloc(superclass,(nofsuperclasses+1)*sizeof(ML_SUPERCLASS)))==NULL) {
        /* this failed. die */
        fprintf(stderr,"add_superclass(): out of memory while trying to add superclass '%s'\n",classname);
        exit(1);
    }
    /* make a pointer to the superclass data */
    sc=(struct ML_SUPERCLASS*)&(superclass[nofsuperclasses]);

    /* check the length of the superclass name */
    if(strlen(classname)>ML_MAX_CLASSNAME_LEN) {
        /* the name is too long! die */
        fprintf(stderr,"add_superclass(): class name '%s' is too long, it may not exceed %lu chars\n",classname,ML_MAX_CLASSNAME_LEN);
        exit(1);
    }

    /* add it */
    memset(sc,0,sizeof(ML_SUPERCLASS));
    strcpy(sc->name,classname);
    sc->nofmethods=nofmethods;
    /* allocate memory for the methods */
    if ((sc->method=(ML_METHOD*)malloc(nofmethods*sizeof(ML_METHOD)))==NULL) {
        /* this failed. die */
        fprintf(stderr,"add_superclass(): cannot add superclass '%s', out of memory for the %lu methods it has\n",classname,nofmethods);
        exit(1);
    }

    /* increment number of objects */
    nofsuperclasses++;

    /* return the id */
    return (nofsuperclasses-1);
}

/*
 * find_object(char* objname)
 *
 * This will scan the object chain for an object named [objname]. It will
 * return the object ID if it is found, or MC_NOBJECT if nothing was found.
 *
 */
_ULONG
find_object(char* objname) {
    struct MC_OBJECT* obj;
    _ULONG i;

    /* scan 'em all */
    for(i=0;i<nofobjects;i++) {
        /* make a pointer to the object data */
        obj=(struct MC_OBJECT*)&(object[i]);

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
 * add_object(char* objname,_ULONG superclass,_ULONG flags)
 *
 * This will add object [objname] to the object list. It will return the
 * object ID of the new object. The new object will be given flags [flags].
 *
 */
_ULONG
add_object(char* objname,_ULONG superclass,_ULONG flags) {
    struct MC_OBJECT* obj;

    /* does the object already exists? */
    if(find_object(objname)!=MC_NOBJECT) {
        /* yup. die */
        fprintf(stderr,"add_object(): object '%s' already exists\n",objname);
        exit(1);
    }

    /* resize the buffer */
    if((object=(MC_OBJECT*)realloc(object,(nofobjects+1)*sizeof(MC_OBJECT)))==NULL) {
        /* this failed. die */
        fprintf(stderr,"add_object(): out of memory while trying to add object '%s'\n",objname);
        exit(1);
    }
    /* make a pointer to the object data */
    obj=(struct MC_OBJECT*)&(object[nofobjects]);

    /* check the length of the object name */
    if(strlen(objname)>MC_MAX_OBJECT_LEN) {
        /* the name is too long! die */
        fprintf(stderr,"add_object(): object name '%s' is too long, it may not exceed %lu chars\n",objname,MC_MAX_OBJECT_LEN);
        exit(1);
    }

    /* add it */
    memset(obj,0,sizeof(MC_OBJECT));
    strcpy(obj->name,objname);
    obj->superclass=superclass;
    obj->flags=flags;

    /* increment number of objects */
    nofobjects++;

    /* return the id */
    return (nofobjects-1);
}

/*
 * dorootscript()
 *
 * This will process the root script.
 *
 */
void
dorootscript() {
    char fname[ML_MAX_FILENAME_LEN];
    char foutname[ML_MAX_FILENAME_LEN];
    FILE* f;
    FILE* fout;
    struct MC_FILEHEADER hdr;
    _ULONG flags,nofmethods,classid,i,j;
    char objname[MC_MAX_OBJECT_LEN];
    char classname[MC_MAX_SUPERCLASS_LEN];
    struct ML_SUPERCLASS* sc;
    struct SCRIPTHEADER shdr;
    struct OBJECTHEADER ohdr;

    char* scriptcode;
    char* scriptdata;

    /* load the root script */
    strcpy(fname,rootscriptname);
    strcat(fname,ML_NAME_SCREXT);

    /* open the file */
    if((f=fopen(fname,"rb"))==NULL) {
        /* this failed. die */
        fprintf(stderr,"ML: Unable to open root script '%s'\n",fname);
        exit(1);
    }

    /* get the file name of the other thingy */
    strcpy(foutname,rootscriptname);
    strcat(foutname,ML_NAME_OBJEXT);

    /* create the object file */
    if((fout=fopen(foutname,"wb"))==NULL) {
        /* this failed. die */
        fprintf(stderr,"ML: Unable to create output file '%s'\n",foutname);
        exit(1);
    }

    /* read the header */
    if(!fread(&hdr,sizeof(hdr),1,f)) {
        /* this failed. die */
        fprintf(stderr,"ML: unable to read header of root script '%s'\n",fname);
        exit(1);
    }

    /* verify it */
    if(hdr.magic!=MC_HEADER_MAGIC) {
        /* this is not a mad compiler script. die */
        fprintf(stderr,"ML: root script '%s' is not a compiled mad file\n",fname);
        exit(1);
    }
    if(hdr.versiono!=MC_HEADER_VERSIONO) {
        /* this is a file that has a totally wrong version. die */
        fprintf(stderr,"ML: root script '%s' is of a wrong version\n",fname);
        exit(1);
    }

    /* set up the header for the output file */
    memset(&shdr,0,sizeof(SCRIPTHEADER));
    shdr.magicno=SCRIPT_MAGICNO;
    shdr.version=SCRIPT_VERSIONO;
    shdr.code_size=hdr.codesize;
    shdr.data_size=hdr.datasize;
    shdr.nofobjects=hdr.nofobjects;
    shdr.object_offset=sizeof(SCRIPTHEADER)+hdr.codesize+hdr.datasize;
    shdr.flags=SCRIPT_FLAG_ROOTSCRIPT;
    shdr.init_offset=hdr.init_offset;
    shdr.done_offset=hdr.done_offset;
    shdr.run_offset=hdr.run_offset;

    /* write the header */
    if(!fwrite(&shdr,sizeof(SCRIPTHEADER),1,fout)) {
        /* this failed. die */
        fprintf(stderr,"ML: Unable to write to output file '%s'\n",foutname);
        exit(1);
    }

    /* allocate memory for the code */
    if((scriptcode=(char*)malloc(hdr.codesize))==NULL) {
        /* this failed. die */
        fprintf(stderr,"ML: cannot allocate %lu bytes of memory\n",hdr.codesize);
        exit(1);
    }

    /* read the code */
    if(!fread(scriptcode,hdr.codesize,1,f)) {
        /* this failed. die */
        fprintf(stderr,"ML: cannot read file '%s'\n",fname);
        exit(1);
    }

    /* write the raw script code */
    if(!fwrite(scriptcode,hdr.codesize,1,fout)) {
        /* this failed. die */
        fprintf(stderr,"ML: Unable to write to output file '%s'\n",foutname);
        exit(1);
    }

    /* zap the script code buffer */
    free(scriptcode);

    /* if there is data, handle it */
    if(hdr.datasize) {
        /* allocate memory for the data */
        if((scriptdata=(char*)malloc(hdr.datasize))==NULL) {
            /* this failed. die */
            fprintf(stderr,"ML: cannot allocate %lu bytes of memory\n",hdr.codesize);
            exit(1);
        }

        /* read the data */
        if(!fread(scriptdata,hdr.datasize,1,f)) {
            /* this failed. die */
            fprintf(stderr,"ML: cannot read file '%s'\n",fname);
            exit(1);
        }

        /* write the raw script code */
        if(!fwrite(scriptdata,hdr.datasize,1,fout)) {
            /* this failed. die */
            fprintf(stderr,"ML: Unable to write to output file '%s'\n",foutname);
            exit(1);
        }

        /* zap the script data buffer */
        free(scriptdata);
    }

    /* get all objects */
    for(i=0;i<hdr.nofobjects;i++) {
        /* get the object name */
        if(!fread(objname,MC_MAX_OBJECT_LEN,1,f)) {
            /* this failed. die */
            fprintf(stderr,"ML: unable to read object entry in root script '%s'\n",fname);
            exit(1);
        }
        if(!fread(&flags,sizeof(_ULONG),1,f)) {
            /* this failed. die */
            fprintf(stderr,"ML: unable to read object entry in root script '%s'\n",fname);
            exit(1);
        }

        /* get the superclass info */
        if(!fread(classname,MC_MAX_SUPERCLASS_LEN,1,f)) {
            /* this failed. die */
            fprintf(stderr,"ML: unable to read superclass entry in root script '%s'\n",fname);
            exit(1);
        }
        if(!fread(&nofmethods,sizeof(_ULONG),1,f)) {
            /* this failed. die */
            fprintf(stderr,"ML: unable to read superclass entry in root script '%s'\n",fname);
            exit(1);
        }

        /* set up the object header */
        memset(&ohdr,0,sizeof(OBJECTHEADER));
        ohdr.nofmethods=nofmethods;
        ohdr.obj_mapcode=i;

        /* write it */
        if(!fwrite(&ohdr,sizeof(OBJECTHEADER),1,fout)) {
            /* this failed. die */
            fprintf(stderr,"ML: unable to write to output file '%s'\n",foutname);
            exit(1);
        }

        /* does it already exists? */
        if((classid=find_superclass(classname))==MC_NOCLASS) {
            /* no. add it */
            classid=add_superclass(classname,nofmethods);
        }

        sc=(struct ML_SUPERCLASS*)&(superclass[classid]);

        /* if we have methods, read them */
        if(nofmethods) {
            /* read all superclass methods */
            if(!fread(sc->method,sizeof(struct ML_METHOD)*nofmethods,1,f)) {
                /* this failed. die */
                fprintf(stderr,"ML: unable to read superclass entry in root script '%s' (superclass '%s')\n",fname,sc->name);
                exit(1);
            }
        }

        /* is this a superobject? */
        if(!strcmp(classname,objname)) {
            /* yes. check to make sure nothing will be inherited from the
               superobjects themselves */
            for(j=0;j<nofmethods;j++) {
                /* was it a kernel function? */
                if((!(sc->method[j].flags&MC_FLAG_KERNELFUNC))&&((!(sc->method[j].flags&MC_FLAG_PROPERTY)))) {
                    /* no. should this be inherited? */
                    if(sc->method[j].offset==MC_NOFFSET) {
                        /* yup. die */
                        fprintf(stderr,"ML: superobject '%s', superclass '%s' tried to inherit method %lu (root script '%s')\n",objname,classname,j,fname);
                        exit(1);
                    }
                }
            }
        }
        /* does it confirm with the one we have (looking at the number of
           properties) */
        sc=(struct ML_SUPERCLASS*)&(superclass[classid]);
        if(sc->nofmethods!=nofmethods) {
            /* it differs! die (this should not happend) */
            fprintf(stderr,"ML: object '%s' uses %lu methods of superclass '%s', instead of %lu (root script '%s')\n",objname,nofmethods,classname,sc->nofmethods,fname);
            exit(1);
        }

        /* add the object to the output file */
        for(j=0;j<nofmethods;j++) {
            /* write it */
            if(!fwrite(&sc->method[j].offset,sizeof(_ULONG),1,fout)) {
                /* this failed. die */
                fprintf(stderr,"ML: unable to write to output file '%s'\n",foutname);
                exit(1);
            }
        }

        /* add the object */
        add_object(objname,classid,flags);
    }

    /* close the files */
    fclose(f); fclose(fout);
}

/*
 * doscript(char* fname)
 *
 * This will process script [fname].
 *
 */
void
doscript(char* fname) {
    char foutname[ML_MAX_FILENAME_LEN];
    FILE* f;
    FILE* fout;
    struct MC_FILEHEADER hdr;
    struct ML_METHOD method;
    _ULONG flags,nofmethods,classid,i,j;
    char objname[MC_MAX_OBJECT_LEN];
    char classname[MC_MAX_SUPERCLASS_LEN];
    struct ML_SUPERCLASS* sc;
    struct SCRIPTHEADER shdr;
    struct OBJECTHEADER ohdr;

    char* scriptcode;
    char* scriptdata;

    /* get the output name */
    getbasename(fname,foutname);
    strcat(foutname,ML_NAME_OBJEXT);

    /* open the file */
    if((f=fopen(fname,"rb"))==NULL) {
        /* this failed. die */
        fprintf(stderr,"ML: Unable to open script '%s'\n",fname);
        exit(1);
    }

    /* create the object file */
    if((fout=fopen(foutname,"wb"))==NULL) {
        /* this failed. die */
        fprintf(stderr,"ML: Unable to create output file '%s'\n",foutname);
        exit(1);
    }

    /* read the header */
    if(!fread(&hdr,sizeof(hdr),1,f)) {
        /* this failed. die */
        fprintf(stderr,"ML: unable to read header of script '%s'\n",fname);
        exit(1);
    }

    /* verify it */
    if(hdr.magic!=MC_HEADER_MAGIC) {
        /* this is not a mad compiler script. die */
        fprintf(stderr,"ML: script '%s' is not a compiled mad file\n",fname);
        exit(1);
    }
    if(hdr.versiono!=MC_HEADER_VERSIONO) {
        /* this is a file that has a totally wrong version. die */
        fprintf(stderr,"ML: script '%s' is of a wrong version\n",fname);
        exit(1);
    }

    /* set up the header for the output file */
    memset(&shdr,0,sizeof(SCRIPTHEADER));
    shdr.magicno=SCRIPT_MAGICNO;
    shdr.version=SCRIPT_VERSIONO;
    shdr.code_size=hdr.codesize;
    shdr.data_size=hdr.datasize;
    shdr.nofobjects=hdr.nofobjects;
    shdr.object_offset=sizeof(SCRIPTHEADER)+hdr.codesize+hdr.datasize;
    shdr.flags=0;
    shdr.init_offset=hdr.init_offset;
    shdr.done_offset=hdr.done_offset;
    shdr.run_offset=hdr.run_offset;

    /* write the header */
    if(!fwrite(&shdr,sizeof(SCRIPTHEADER),1,fout)) {
        /* this failed. die */
        fprintf(stderr,"ML: Unable to write to output file '%s'\n",foutname);
        exit(1);
    }

    /* allocate memory for the code */
    if((scriptcode=(char*)malloc(hdr.codesize))==NULL) {
        /* this failed. die */
        fprintf(stderr,"ML: cannot allocate %lu bytes of memory\n",hdr.codesize);
        exit(1);
    }

    /* read the code */
    if(!fread(scriptcode,hdr.codesize,1,f)) {
        /* this failed. die */
        fprintf(stderr,"ML: cannot read file '%s'\n",fname);
        exit(1);
    }

    /* write the raw script code */
    if(!fwrite(scriptcode,hdr.codesize,1,fout)) {
        /* this failed. die */
        fprintf(stderr,"ML: Unable to write to output file '%s'\n",foutname);
        exit(1);
    }

    /* zap the script code buffer */
    free(scriptcode);

    /* if there's data, read it */
    if(hdr.datasize) {
        /* allocate memory for the data */
        if((scriptdata=(char*)malloc(hdr.datasize))==NULL) {
            /* this failed. die */
            fprintf(stderr,"ML: cannot allocate %lu bytes of memory\n",hdr.codesize);
            exit(1);
        }

        /* read the data */
        if(!fread(scriptdata,hdr.datasize,1,f)) {
            /* this failed. die */
            fprintf(stderr,"ML: cannot read file '%s'\n",fname);
            exit(1);
        }

        /* write the raw script data */
        if(!fwrite(scriptdata,hdr.datasize,1,fout)) {
            /* this failed. die */
            fprintf(stderr,"ML: Unable to write to output file '%s'\n",foutname);
            exit(1);
        }

    }

    /* zap the script data buffer */
    free(scriptdata);

    /* get all objects */
    for(i=0;i<hdr.nofobjects;i++) {
        /* get the object name */
        if(!fread(objname,MC_MAX_OBJECT_LEN,1,f)) {
            /* this failed. die */
            fprintf(stderr,"ML: unable to read object entry in script '%s'\n",fname);
            exit(1);
        }
        if(!fread(&flags,sizeof(_ULONG),1,f)) {
            /* this failed. die */
            fprintf(stderr,"ML: unable to read object entry in script '%s'\n",fname);
            exit(1);
        }

        /* get the superclass info */
        if(!fread(classname,MC_MAX_SUPERCLASS_LEN,1,f)) {
            /* this failed. die */
            fprintf(stderr,"ML: unable to read class entry in root script '%s'\n",fname);
            exit(1);
        }
        if(!fread(&nofmethods,sizeof(_ULONG),1,f)) {
            /* this failed. die */
            fprintf(stderr,"ML: unable to read class entry in root script '%s'\n",fname);
            exit(1);
        }

        /* set up the object header */
        memset(&ohdr,0,sizeof(OBJECTHEADER));
        ohdr.nofmethods=nofmethods;

        /* external object? */
        if(flags&MC_FLAG_EXTERNAL) {
            /* yup, find it */
            if((j=find_object(objname))==MC_NOBJECT) {
                /* it's not there! die */
                fprintf(stderr,"ML: externally declared object '%s' not found in root script '%s'\n",objname,fname);
                exit(1);
            }
            ohdr.obj_mapcode=j;
        } else {
            /* no */
            ohdr.obj_mapcode=i+nofobjects;
        }

        /* write it */
        if(!fwrite(&ohdr,sizeof(OBJECTHEADER),1,fout)) {
            /* this failed. die */
            fprintf(stderr,"ML: unable to write to output file '%s'\n",foutname);
            exit(1);
        }

        /* does it already exists? */
        if((classid=find_superclass(classname))==MC_NOCLASS) {
            /* no. die */
            fprintf(stderr,"ML: superclass '%s' from script '%s' not found in the root script\n",classname,fname);
            exit(1);
        }

        /* does it have a superobject? */
        if(find_object(classname)==MC_NOBJECT) {
            /* no. die */
            fprintf(stderr,"ML: superclass '%s' from script '%s' doesn't have a superobject in the root script\n",superclass,fname);
            exit(1);
        }

        /* add the object to the output file */
        for(j=0;j<nofmethods;j++) {
            /* read all the method */
            if(!fread(&method,sizeof(struct ML_METHOD),1,f)) {
                /* this failed. die */
                fprintf(stderr,"ML: unable to read superclass entry in root script '%s'\n",fname);
                exit(1);
            }

            /* write it */
            if(!fwrite(&method.offset,sizeof(_ULONG),1,fout)) {
                /* this failed. die */
                fprintf(stderr,"ML: unable to write to output file '%s'\n",foutname);
                exit(1);
            }
        }
    }

    /* close the files */
    fclose(f); fclose(fout);
}

int
main(int argc,char* argv[]) {
    _ULONG i;

    /* any parameters? */
    if(argc<2) {
        /* nope. print usuage and leave */
        showhelp();
        exit(1);
    }

    /* parse the parameters */
    parseparms(argc,argv);

    /* get all filenames */
    getfilenames();

    /* zap the object and superclass buffers */
    object=NULL; nofobjects=0;
    superclass=NULL; nofsuperclasses=0;

    /* get all shared objects from all files */
    if (verbose) printf("> Processing root script\n");
    dorootscript();
    if (verbose) printf("> Done processing root script\n");

    /* process the rest */
    for(i=0;i<nofiles;i++) {
        if (verbose) printf("> Processing script '%s'\n",filename[i]);
        if(!strncasecmp(filename[i],rootscriptname,strlen(rootscriptname))) {
            /* this is the root script. skip it */
            if (verbose) printf("> Skipped root script '%s'\n",filename[i]);
        } else {
            doscript(filename[i]);
            if (verbose) printf("> Done processing script '%s'\n",filename[i]);
        }
    }
    return 0;
}
