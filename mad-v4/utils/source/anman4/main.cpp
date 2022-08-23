/*
 *                     MAD Animation Manager Version 4.0
 *
 *                        (c) 1999, 2000 The MAD Crew
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "anm.h"
#include "gfx.h"
#include "types.h"

_UINT flagtable[16] = {   1,     2,     4,     8,    16,    32,    64,   128,
                        256,   512,  1024,  2048,  4096,  8192, 16384, 32768 };

#define BANNER           "MAD Animation Manager 4.0\n(c) 1999, 2000 The MAD Crew\n"
#define MAX_FILENAME_LEN 64
#define MAX_LINE_LEN     128
#define MAX_SEQ_LEN      256

#define STATUS_NORMAL    0
#define STATUS_FILES     1
#define STATUS_ANIMS     2

char*   anman_infile;
char*   anman_outfile;

FILE*   afile;
struct  ANMHEADER ahdr;

/*
 * usuage()
 *
 * This will show the correct usuage
 *
 */
void
usuage() {
    printf("%s\n",BANNER);
    fprintf(stderr,"Usuage: ANMAN [options] infile.asr outfile.anm\n\n");
    fprintf(stderr,"Flags can be:    -h, -? this help\n");
    exit(0xfe);
}

/*
 * parse_params(int argc,char* argv[])
 *
 * This will take care of the parameters. [argc] and [argv] should be passed
 * on from main().
 *
 */
void
parse_params(int argc,char* argv[]) {
    _UINT i;

    /* check for sufficient parameters */
    if (argc<3) {
        usuage();
        /* NOTREACHED */
    }

    /* setup defaults */
    /* scan all parameters */
    for(i=0;i<argc;i++) {
        /* does it begin with a dash or slash? */
        if((argv[i][0]=='-')||(argv[i][0]=='/')) {
            /* yes. check it */
            switch(argv[i][1]) {
                case 'h':
                case '?': /* help */
                          usuage();
                          /* NOTREACHED */
                 default: /* unknown! */
                          usuage();
                          /* NOTREACHED */
            }
        }
    }
    /* get the filenames */
    anman_infile=strdup(argv[argc-2]);
    anman_outfile=strdup(argv[argc-1]);
    /* if they start with a dash or slash, complain */
    if((anman_outfile[0]=='-')||(anman_outfile[0]=='/')||(anman_infile[0]=='-')||(anman_infile[0]=='/')) {
        /* they do. complain */
        usuage();
        /* NOTREACHED */
    }
}

/*
 * init_anm(char* fname)
 *
 * This will initialize the animation file.
 *
 */
void
init_anm(char* fname) {
    /* create the file */
    if((afile=fopen(fname,"wb"))==NULL) {
        /* this failed. die */
        fprintf(stderr,"ANMAN: Unable to create '%s'\n",fname);
        exit(1);
    }

    /* zero out the header and fill in some fields */
    memset(&ahdr,0,sizeof(ANMHEADER));
    ahdr.idcode=ANM_MAGICNO;
    ahdr.version=ANM_VERSION;

    /* write this bogus header (will be changed later) */
    if(!fwrite(&ahdr,sizeof(ANMHEADER),1,afile)) {
        /* this failed. die */
        fprintf(stderr,"ANMAN: Unable to write '%s'\n",fname);
        exit(1);
    }
}

/*
 * anm_addsprite(char* fname)
 *
 * This will add sprite [fname] to the animation file.
 *
 */
void
anm_addsprite(char* fname) {
    FILE* f;
    struct SPRITEHEADER shdr;
    struct ANMSPRITEHEADER ashdr;
    char*  buf;
    _ULONG i;

    /* open the file */
    if((f=fopen(fname,"rb"))==NULL) {
        /* this failed. die */
        fprintf(stderr,"ANMAN: Unable to open '%s'\n",fname);
        exit(1);
    }

    /* read the header */
    if(!fread(&shdr,sizeof(SPRITEHEADER),1,f)) {
        /* this failed. die */
        fprintf(stderr,"ANMAN: Unable to read '%s'\n",fname);
        exit(1);
    }

    /* verify the header */
    if(shdr.idcode!=GFX_SPR_MAGICNO) {
        /* this failed. die */
        fprintf(stderr,"ANMAN: '%s' is not a MAD sprite file\n",fname);
        exit(1);
    }
    if(shdr.version!=GFX_SPR_VERSIONO) {
        /* this failed. die */
        fprintf(stderr,"ANMAN: '%s' is of a wrong version\n",fname);
        exit(1);
    }

    /* build the animation sprite header */
    memset(&ashdr,0,sizeof(ANMSPRITEHEADER));
    ashdr.heigth=shdr.height;
    ashdr.width=shdr.width;
    ashdr.transcol_r=shdr.transcol_r;
    ashdr.transcol_g=shdr.transcol_g;
    ashdr.transcol_b=shdr.transcol_b;

    /* write it to the animation file */
    if(!fwrite(&ashdr,sizeof(ANMSPRITEHEADER),1,afile)) {
        /* this failed. die */
        fprintf(stderr,"ANMAN: Unable to write '%s'\n",anman_outfile);
        exit(1);
    }

    /* allocate memory for the sprite data */
    if((buf=(char*)malloc(shdr.height*shdr.width*3))==NULL) {
        /* this failed. die */
        fprintf(stderr,"ANMAN: unable to allocate %lu bytes for data of '%s'\n",(shdr.height*shdr.width*3),fname);
        exit(1);
    }

    /* read the sprite data */
    if(!fread(buf,(shdr.height*shdr.width*3),1,f)) {
        /* this failed. die */
        fprintf(stderr,"ANMAN: Unable to read '%s'\n",fname);
        exit(1);
    }

    /* write the sprite data itself */
    if(!fwrite(buf,(shdr.height*shdr.width*3),1,afile)) {
        /* this failed. die */
        fprintf(stderr,"ANMAN: Unable to write '%s'\n",anman_outfile);
        exit(1);
    }

    /* free the buffer */
    free(buf);

    /* increment the number of sprites in the header */
    ahdr.nofsprites++;

    /* close the file */
    fclose(f);
}

/*
 * anm_addanim(char* line)
 *
 * This will add an animaton to the file. It'll expect [line] to be the
 * line to add.
 *
 */
void
anm_addanim(char* line) {
    struct ANIMATION anm;
    char animname[ANM_MAXANMLEN];
    _ULONG pos,frameno,flags,count;
    _ULONG temp[MAX_SEQ_LEN];

    /* get the animation name */
    sscanf(line,"%s",animname);

    pos=strlen(animname)+1;flags=0;
    /* check the length */
    if(strlen(animname)>(ANM_MAXANMLEN-1)) {
        /* too long. die */
        fprintf(stderr,"ANMAN: Animation line '%s' is too long\n",line);
        exit(1);
    }
    count=0;
    while(pos<=strlen(line)) {
        frameno=atol((char *)(line+pos));
        if (!strncasecmp((char *)(line+pos),"f",1)) {
            flags+=flagtable[atoi((char *)(line+pos+1))];
            pos+=3;
        } else {
            if (frameno>99) {
               pos+=4;
            } else {
               if(frameno>9) {
                  pos+=3;
               } else {
                   pos+=2;
               }
            }
            temp[count]=frameno-1;
            count++;
        }
    }

    /* fill in header */
    memset(&anm,0,sizeof(struct ANIMATION));
    strcpy(anm.name,animname);
    anm.noframes=count;
    anm.flags=flags;

    /* write this header */
    if(!fwrite(&anm,sizeof(ANIMATION),1,afile)) {
        /* this failed. die */
        fprintf(stderr,"ANMAN: Unable to write to '%s'\n",anman_outfile);
        exit(1);
    }

    /* write the frame table */
    if(!fwrite(&temp,count*sizeof(_ULONG),1,afile)) {
        /* this failed. die */
        fprintf(stderr,"ANMAN: Unable to write to '%s'\n",anman_outfile);
        exit(1);
    }

    /* increment number of animations */
    ahdr.nofanims++;
}

/*
 * done_anm()
 *
 * This will deinitialize the animation file.
 *
 */
void
done_anm() {
    /* seek back the the beginning of the file and write the header */
    fseek(afile,0,SEEK_SET);
    if(!fwrite(&ahdr,sizeof(ANMHEADER),1,afile)) {
        /* this failed. die */
        fprintf(stderr,"ANMAN: Unable to rewrite header of '%s'\n",anman_outfile);
        exit(1);
    }

    /* close the file */
    fclose(afile);
}

/*
 * do_script(char* fname)
 *
 * This will handle script [fname].
 *
 */
void
do_script(char* fname) {
    FILE*  f;
    char   line[MAX_LINE_LEN];
    char   templine[MAX_LINE_LEN];
    char*  ptr;
    _UCHAR status;
    _UINT  i;

    /* open the script */
    if((f=fopen(fname,"rt"))==NULL) {
        /* this failed. die */
        fprintf(stderr,"ANMAN: Unable to open script file '%s'\n",fname);
        exit(1);
    }

    /* read the entire file */
    status=STATUS_NORMAL;
    while(fgets(line,MAX_LINE_LEN,f)) {
        /* zap the last char */
        line[strlen(line)-1]=0;

	/* handle DOS newlines too */
	if (strlen(line) > 0 && line[strlen(line) - 1] == '\r')
        	line[strlen(line)-1]=0;

        /* is it in the form of [xxx]? */
        if(line[0]=='[') {
            /* guess so... a ']' in the string? */
            if((ptr=strchr(line,']'))!=NULL) {
                /* yup. dump everything before the ']' to [templine] */
                for(i=1;i<(ptr-line);i++) {
                    templine[i-1]=line[i];
                }
                templine[i-1]=0;

                /* check what it is */
                if(!strcmp(templine,"files")) {
                    /* user wants to do the files. is it ok? */
                    if(status>STATUS_NORMAL) {
                        /* no. */
                        fprintf(stderr,"ANMAN: Processing files is not allowed anymore\n");
                        exit(1);
                    }
                    /* it's OK. set it */
                    status=STATUS_FILES;
                } else {
                    /* maybe animations? */
                    if((!strcmp(templine,"anims"))||(!strcmp(templine,"animations"))) {
                        /* yup. is this ok? */
                       if(status>STATUS_FILES) {
                            /* no. */
                            fprintf(stderr,"ANMAN: Processing animations is not allowed anymore\n");
                            exit(1);
                        }
                        /* it's OK. set it */
                        status=STATUS_ANIMS;
                    }
                }
            }
        } else {
            /* are we currently processing files? */
            if(status==STATUS_FILES) {
                /* yup. does it begin with a # or //, or is it a blank line? */
                if((!strlen(line))||(line[0]=='#')||(((line[0]=='/')&&(line[1]=='/')))) {
                    /* yes. ignore the comment */
                } else {
                    /* add the file */
                    anm_addsprite(line);
                }
            } else {
                /* should we be processing animations? */
                if(status==STATUS_ANIMS) {
                    /* yup. does it begin with a # or //, or is it a blank line? */
                    if((!strlen(line))||(line[0]=='#')||(((line[0]=='/')&&(line[1]=='/')))) {
                        /* yes. ignore the comment */
                    } else {
                        /* add the animation */
                        anm_addanim(line);
                    }
                }
            }
        }

        /* print the line */
    }

    /* close the file */
    fclose(f);
}

/*
 * main(int argc,char* argv[])
 *
 * Guess...
 *
 */
int
main(int argc,char* argv[]) {
    /* parse the parameters */
    parse_params(argc,argv);

    /* initialize the animation file */
    init_anm(anman_outfile);

    /* handle the script */
    do_script(anman_infile);

    /* deinitialize the animation file */
    done_anm();
    return 0;
}
