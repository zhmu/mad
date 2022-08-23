/*
 *                             The MAD Linker
 *                               Version 2.0
 *
 *                          (c) 1999 The MAD Crew
 *
 */
#include <stdlib.h>
#include <string.h>
#ifdef DJGPP
#include <dir.h>
#endif
#ifdef UNIX
#include <dirent.h>
#endif
#include "ml.h"
#include "../../../source/objman.h"
#include "../../../source/script.h"
#include "../../../source/types.h"

/* remove the x to activate debugging */
#define xML_DEBUG

_UCHAR verbose;
_ULONG nofiles,nofsharedobj,noflocalobjs;

char   filename[ML_MAX_FILES][ML_MAX_FILENAME_LEN];

struct ML_OBJECT shared_obj[ML_MAX_SHARED_OBJECTS];

/*
 * showbuildinfo()
 *
 * This will show information about the current build
 *
 */
void
showbuildinfo() {
    printf("The MAD Linker version 2.0\n");
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
    printf("Usuage: ML [switches]\n\n");
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
                if(!strcasecmp(ptr,MC_EXT_CSCRIPT)) {
                    /* this is a script! add it */
                    strcpy(filename[nofiles],entry->d_name);

                    nofiles++;
                }
            }
        }

        /* Close the directory */
        closedir(dir);
    #else
        #error Platform needed
    #endif
}

/*
 * find_shared_obj(char* objname)
 *
 * This will return the object id of [objname], or ML_NOBJECT if it doesn't
 * exists. This will only browse shared objects.
 *
 */
_ULONG
find_shared_obj(char* objname) {
    _ULONG i;

    /* scan all shared objects */
    for(i=0;i<nofsharedobj;i++) {
        /* is this the one? */
        if(!strcmp(shared_obj[i].name,objname)) {
            /* yeah, return its id */
            return i;
        }
    }
    /* we couldn't find it. return ML_NOBJECT */
    return ML_NOBJECT;
}

/*
 * add_shared_obj(char* objname)
 *
 * This will add shared object [objname] to the list. If it already exists,
 * this function will do nothing.
 *
 */
void
add_shared_obj(char* objname) {
    /* was it already declared? */
    if(find_shared_obj(objname)!=ML_NOBJECT) {
        /* yeah. just return */
        return;
    }
    /* copy the object name */
    strcpy(shared_obj[nofsharedobj].name,objname);

    nofsharedobj++;
}

/*
 * initsharedobj()
 *
 * Initialize the shared object table
 *
 */
void
initsharedobj() {
    nofsharedobj=0;

    add_shared_obj(MC_MOUSE_OBJNAME);
    add_shared_obj(MC_BAR_OBJNAME);
    add_shared_obj(MC_CONTROLS_OBJNAME);
    add_shared_obj(MC_SYSTEM_OBJNAME);
    add_shared_obj(MC_SCRIPT_OBJNAME);
    add_shared_obj(MC_PARSER_OBJNAME);
    add_shared_obj(MC_DIALOG_OBJNAME);
}

/*
 * getsharedobj()
 *
 * This will get all shared objects and put them into shared_obj[].
 *
 */
void
getsharedobj() {
    FILE *f;
    _ULONG i,j;
    struct MC_SCRIPTHEADER hdr;
    struct ML_OBJECT obj;

    i=0;
    for(i=0;i<nofiles;i++) {
        /* open the file */
        if((f=fopen(filename[i],"rb"))==NULL) {
            /* this failed. die */
            printf("ML: Unable to open file '%s'\n",filename[i]);
            exit(1);
        }
        if (verbose) printf("Processing %s... ",filename[i]);

        /* read the header */
        if(!fread(&hdr,sizeof(struct MC_SCRIPTHEADER),1,f)) {
            /* this failed. die */
            printf("ML: Unable to read file '%s'\n",filename[i]);
            exit(1);
        }

        /* verify the file type */
        if(hdr.magicno!=MC_HEADER_MAGICNO) {
            /* this failed. die */
            printf("ML: File '%s' is not a MAD compiled script\n",filename[i]);
            exit(1);
        }
        /* verify the version number */
        if(hdr.version!=MC_HEADER_VERSIONO) {
            /* this failed. die */
            printf("ML: File '%s' is of a wrong version\n",filename[i]);
            exit(1);
        }

        /* read the object name */
        fseek(f,hdr.script_size+sizeof(struct MC_SCRIPTHEADER)+((MC_MAX_PROCNAME_LEN+8)*hdr.nofprocs),SEEK_SET);

        for(j=0;j<hdr.nofobjects;j++) {
            /* read the object data */
            if(!fread(&obj.name,MC_MAX_OBJECT_LEN,1,f)) {
                /* this failed. die */
                printf("ML: Unable to read file '%s'\n",filename[i]);
                exit(1);
            }
            if(!fread(&obj.type,1,1,f)) {
                /* this failed. die */
                printf("ML: Unable to read file '%s'\n",filename[i]);
                exit(1);
            }
            if(!fread(&obj.flags,4,1,f)) {
                /* this failed. die */
                printf("ML: Unable to read file '%s'\n",filename[i]);
                exit(1);
            }
            if(!fread(&obj.nofchanges,4,1,f)) {
                /* this failed. die */
                printf("ML: Unable to read file '%s'\n",filename[i]);
                exit(1);
            }
            if(obj.flags&MC_FLAG_EXTERNAL) {
                /* boy, this object must be shared. was it already declared? */
                if(find_shared_obj(obj.name)==ML_NOBJECT) {
                    /* no, add it to the list */
                    add_shared_obj(obj.name);
                }
            }
            fseek(f,ftell(f)+4*obj.nofchanges,SEEK_SET);
        }

        if (verbose) printf("done\n");
        /* close the file */
        fclose(f);
    }
}

/*
 * change_object_no(char* data,_ULONG offset,_UINT newno)
 *
 * This will change the object number at offset [offset] of script [data] to
 * [newno]. [datalen] is the length of the buffer, and will be used to check
 * for invalid offsets. [fname] should be the filename.
 *
 */
void
change_object_no(char* data,_ULONG datalen,_ULONG offset,_UINT newno,char* fname) {
    /* is this offset valid? */
    if((offset+2)>datalen) {
        /* no. die */
        printf("ML: Offset beyond end of file '%s' (there ARE limits <g>)\n",fname);
        exit(1);
    }
    /* change it */
//    memcpy((void*)(data+offset),&newno,2);
    data[offset]=newno&0xff;
    data[offset+1]=newno>>8;
}

/*
 * processfiles()
 *
 * This will process all files.
 *
 */
void
processfiles() {
    FILE *f;
    _ULONG i,j,count,objno,no;
    struct SCRIPTHEADER lhdr;
    struct MC_SCRIPTHEADER hdr;
    struct ML_OBJECT obj;
    struct ML_PROC proc[ML_MAX_PROCS];
    _ULONG change[MC_MAX_CHANGES];
    char* cscript;
    char   tempfname[ML_MAX_FILENAME_LEN];

    i=0;
    for(i=0;i<nofiles;i++) {
        /* open the file */
        if((f=fopen(filename[i],"rb"))==NULL) {
            /* this failed. die */
            printf("ML: Unable to open file '%s'\n",filename[i]);
            exit(1);
        }
        if (verbose) printf("Processing %s... ",filename[i]);

        /* read the header */
        if(!fread(&hdr,sizeof(struct MC_SCRIPTHEADER),1,f)) {
            /* this failed. die */
            printf("ML: Unable to read file '%s'\n",filename[i]);
            exit(1);
        }

        /* no verifying needed, getsharedobj() already did this */

        /* allocate memory the entire script */
        if((cscript=(char*)malloc(hdr.script_size))==NULL) {
            /* this failed. die */
            printf("ML: Unable to allocate %lu bytes for the script data\n",hdr.script_size);
            exit(1);
        }
        /* read the script */
        if(!fread(cscript,hdr.script_size,1,f)) {
            /* this failed. die */
            printf("ML: Unable to read file '%s'\n",filename[i]);
            exit(1);
        }

        /* no verifying needed, getsharedobj() already did this */

        /* read the object name */
        fseek(f,hdr.script_size+sizeof(struct MC_SCRIPTHEADER)+((MC_MAX_PROCNAME_LEN+8)*hdr.nofprocs),SEEK_SET);

        for(j=0;j<hdr.nofobjects;j++) {
            /* read the object data */
            if(!fread(&obj.name,MC_MAX_OBJECT_LEN,1,f)) {
                /* this failed. die */
                printf("ML: Unable to read file '%s'\n",filename[i]);
                exit(1);
            }
            if(!fread(&obj.type,1,1,f)) {
                /* this failed. die */
                printf("ML: Unable to read file '%s'\n",filename[i]);
                exit(1);
            }
            if(!fread(&obj.flags,4,1,f)) {
                /* this failed. die */
                printf("ML: Unable to read file '%s'\n",filename[i]);
                exit(1);
            }
            if(!fread(&obj.nofchanges,4,1,f)) {
                /* this failed. die */
                printf("ML: Unable to read file '%s'\n",filename[i]);
                exit(1);
            }
            /* is this amount of changes supported? */
            if(obj.nofchanges>=MC_MAX_CHANGES) {
                /* no. die */
                printf("ML: Script object file '%s' needs too many changes\n",filename[i]);
                exit(1);
            }
            /* were there changes? */
            if(obj.nofchanges) {
                /* yeah, read them */
                if(!fread(&change,(4*obj.nofchanges),1,f)) {
                    /* this failed. die */
                    printf("ML: Unable to read file '%s'\n",filename[i]);
                    exit(1);
                }
            }

            /* apply all changes */
            for(count=0;count<obj.nofchanges;count++) {
                /* is this an external object? */
                /* yeah. is it an external object? */
                if((objno=find_shared_obj(obj.name))==ML_NOBJECT) {
                    /* no, it's a local object! take care of it */
                    #ifdef ML_DEBUG
                    printf("GAVE LOCAL OBJECT '%s' number %u\n",obj.name,(nofsharedobj+j+2));
                    #endif
                    change_object_no(cscript,hdr.script_size,change[count],(nofsharedobj+j+2),filename[i]);
                } else {
                    /* yeah, change the object number */
                    #ifdef ML_DEBUG
                    printf("GAVE SHARED OBJECT '%s' number %u\n",obj.name,objno);
                    #endif
                    change_object_no(cscript,hdr.script_size,change[count],objno,filename[i]);
               }
            }
        }
        /* load all procedure info */
        fseek(f,hdr.script_size+sizeof(MC_SCRIPTHEADER),SEEK_SET);

        for(count=0;count<hdr.nofprocs;count++) {
            /* handle procedures */
            if(!fread(proc[count].name,MC_MAX_PROCNAME_LEN,1,f)) {
                /* this failed. die */
                printf("unable to read file '%s'\n",filename[i]);
                exit(1);
            }
            if(!fread(&proc[count].startbyte,4,1,f)) {
                /* this failed. die */
                printf("unable to read file '%s'\n",filename[i]);
                exit(1);
            }
            if(!fread(&proc[count].endbyte,4,1,f)) {
                /* this failed. die */
                printf("unable to read file '%s'\n",filename[i]);
                exit(1);
            }
        }
        /* close the file */
        fclose(f);

        /* construct the correct file name */
        getbasename(filename[i],tempfname);
        strcat(tempfname,MC_EXT_LSCRIPT);

        /* write the new, linked script to the disk */
        if((f=fopen(tempfname,"wb"))==NULL) {
            /* this failed. die */
            printf("ML: Unable to create file '%s'\n",tempfname);
            exit(1);
        }

        /* construct the header */
        memset(&lhdr,0,sizeof(struct SCRIPTHEADER));
        lhdr.magicno=SCRIPT_MAGICNO;
        lhdr.version=ML_VERSIONO;
        lhdr.script_size=hdr.script_size;
        lhdr.nofprocs=hdr.nofprocs;

        /* write it to the file */
        if(!fwrite(&lhdr,sizeof(struct SCRIPTHEADER),1,f)) {
            /* this failed. die */
            printf("ML: Unable to write file '%s'\n",tempfname);
            exit(1);
        }
        /* write the script data */
        if(!fwrite(cscript,hdr.script_size,1,f)) {
            /* this failed. die */
            printf("unable to write file '%s'\n",tempfname);
            exit(1);
        }
        /* write the procs */
        for(count=0;count<hdr.nofprocs;count++) {
            /* handle procedures */
            if(!fwrite(&proc[count],sizeof(struct ML_PROC),1,f)) {
                /* this failed. die */
                printf("unable to write file '%s'\n",tempfname);
                exit(1);
            }
        }

        /* close the file */
        fclose(f);

        /* free script data */
        free(cscript);

        if (verbose) { printf("done\n"); fflush(stdout); }
    }
}

int
main(_UINT argc,char* argv[]) {
    /* parse the parameters */
    parseparms(argc,argv);

    /* get all filenames */
    getfilenames();

    /* initialize the shared object table */
    initsharedobj();

    /* get all shared objects from all files */
    if (verbose) printf("> Processing all shared objects\n");
    getsharedobj();
    if (verbose) printf("> Done processing all shared objects\n");

    /* now process all files */
    if (verbose) printf("> Processing all object files\n");
    processfiles();
    if (verbose) printf("> Done processing all object files\n");
    return 0;
}
