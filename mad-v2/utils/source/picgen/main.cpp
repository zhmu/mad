#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "gfx.h"

#define MAX_FILE_NAME_LEN 64

#define NO_GFX_SCR 1
#define NO_MSK_SCR 2
#define NO_PRI_SCR 4

#define NO_FILE_NAME "/dev/null desktop->trashcan"

_UCHAR flags;
char filename[MAX_FILE_NAME_LEN];
char gfx_filename[MAX_FILE_NAME_LEN];
char msk_filename[MAX_FILE_NAME_LEN];
char pri_filename[MAX_FILE_NAME_LEN];

struct PCXHEADER {
    _UCHAR manufacturer;         /* must be 0xA, for ZSoft PCX */
    _UCHAR version;              /* version number */
    _UCHAR encoding;             /* must be 1 for RLE */
    _UCHAR bpp;                  /* must be 8 for 256 color */
    _UCHAR xmin,ymin,xmax,ymax;  /* picture dimensions */
    _UINT  hres;                 /* horizonal size for device */
    _UINT  vres;                 /* vertical size for device */
    _UCHAR colormap[48];         /* palette, ignored */
    _UCHAR reserved;             /* some reserved junk */
    _UINT  bytesperline;         /* bytes per line */
    _UINT  palinfo;              /* palette information */
    _UCHAR filler[58];           /* filler */
};

void
usuage() {
    printf("Usuage: PICGEN [picture_file] [...switches...]\n\n");
    printf("Switches can be:\n");
    printf("    -G[filename]   use [filename] as graphics screen\n");
    printf("    -g             don't use a graphics screen\n");
    printf("    -P[filename]   use [filename] as priority screen\n");
    printf("    -p             don't use a priority screen\n");
    printf("    -M[filename]   use [filename] as mask screen\n");
    printf("    -m             don't use a mask screen\n\n");
    printf("As you might have guessed, the parameters are cAsE sEnSiTiVe\n");
}


void
parseparams(char *argv[], _UINT argc) {
    _UINT i,j;

    flags=0;
    strcpy(filename,NO_FILE_NAME);
    strcpy(gfx_filename,NO_FILE_NAME);
    strcpy(pri_filename,NO_FILE_NAME);
    strcpy(msk_filename,NO_FILE_NAME);
    for (i=1;i<argc;i++) {
        if ((argv[i][0]=='-')||(argv[i][0]=='/')) {
            switch (argv[i][1]) {
                case 'g': flags|=NO_GFX_SCR; break;
                case 'p': flags|=NO_PRI_SCR; break;
                case 'm': flags|=NO_MSK_SCR; break;
                case 'G': for(j=2;j<strlen(argv[i]);j++) {
                              gfx_filename[j-2]=argv[i][j];
                              gfx_filename[j-1]=0;
                          }
                          break;
                case 'P': for(j=2;j<strlen(argv[i]);j++) {
                              pri_filename[j-2]=argv[i][j];
                              pri_filename[j-1]=0;
                          }
                          break;
                case 'M': for(j=2;j<strlen(argv[i]);j++) {
                              msk_filename[j-2]=argv[i][j];
                              msk_filename[j-1]=0;
                          }
                          break;
                 default: printf("unknown option -- %c\n",argv[i][1]);
                          exit(1);
            }
        } else {
            if(strcmp(filename,"/dev/null desktop->trashcan")) {
                printf("Filename already given!\n");
                exit(1);
            }
            strcpy(filename,argv[i]);
	}
    }
    if((flags&NO_GFX_SCR)&&strcmp(gfx_filename,NO_FILE_NAME)) {
        printf("Why did you specify a graphics filename and the -g flag?\n");
        exit(1);
    }
    if((flags&NO_PRI_SCR)&&strcmp(pri_filename,NO_FILE_NAME)) {
        printf("Why did you specify a priority filename and the -p flag?\n");
        exit(1);
    }
    if((flags&NO_MSK_SCR)&&strcmp(msk_filename,NO_FILE_NAME)) {
        printf("Why did you specify a mask filename and the -m flag?\n");
        exit(1);
    }
    if((!(flags&NO_GFX_SCR))&&!strcmp(gfx_filename,NO_FILE_NAME)) {
        printf("You didn't use one of the -g or -G flags. I don't know what to do!\n");
        exit(1);
    }
    if((!(flags&NO_PRI_SCR))&&!strcmp(pri_filename,NO_FILE_NAME)) {
        printf("You didn't use one of the -p or -P flags. I don't know what to do!\n");
        exit(1);
    }
    if((!(flags&NO_MSK_SCR))&&!strcmp(msk_filename,NO_FILE_NAME)) {
        printf("You didn't use one of the -m or -M flags. I don't know what to do!\n");
        exit(1);
    }
}

char
*loadfile(char *fname,_ULONG *length) {
    FILE *f;
    char *ptr;

    /* try to open the file */
    if((f=fopen(fname,"rb"))==NULL) {
        /* this failed. report and die */
        printf(" unable to open '%s'\n",fname);
        exit(3);
    }
    /* figure out file length */
    fseek(f,0,SEEK_END); *length=ftell(f); rewind(f);
    /* allocate memory */
    if((ptr=(char *)malloc(*length))==NULL) {
        /* this failed. report and die */
        fclose(f);
        printf(" out of memory. I couldn't allocate %lu bytes.\n",*length);
        exit(3);
    }
    /* read the file into the buffer */
    if(!fread(ptr,*length,1,f)) {
        /* this failed. aaargh!!! */
        fclose(f);
        printf(" unable to read '%s'\n",fname);
        exit(3);
    }
    fclose(f);
    /* yipee, we read the file ok. return its pointer */
    return ptr;
}

_UCHAR
checkpcx(struct PCXHEADER *hdr) {
    if(hdr->manufacturer!=0xa) return 0;
    if(hdr->encoding!=1) return 0;
    if(hdr->bpp!=8) return 0;

    return 1;
}

void
createpic(char *fname) {
    struct PICHEADER hdr;
    FILE *f;
    char *pic;
    _ULONG l;
    _UINT demx,demy;
    _UINT i,j;
    struct PCXHEADER *pcxhdr;

    if((f=fopen(fname,"wb"))==NULL) {
        printf(" unable to create '%s'\n",fname);
        exit(2);
    }

    memset(&hdr,0,sizeof(struct PICHEADER));
    hdr.idcode=GFX_PIC_MAGICNO;
    hdr.version=GFX_PIC_VERSIONO;
    /* write a bullshit header, which will be updated when we're done */
    if(!fwrite(&hdr,sizeof(struct PICHEADER),1,f)) {
        /* failed. report and die */
        printf(" unable to write '%s'\n",fname);
        exit(2);
    }
    demx=0;demy=0;

    /* add the MAIN picture */
    if(!(flags&NO_GFX_SCR)) {
        /* fill in offset field */
        hdr.gfx_offset=ftell(f);
        /* load the picture */
        pic=loadfile(gfx_filename,&l);
        pcxhdr=(struct PCXHEADER *)pic;
        /* check if it is a pcx file we are reading */
        if (!checkpcx(pcxhdr)) {
            /* no, so die */
            printf(" file '%s' is not a pcx file\n",gfx_filename);
            exit(4);
        }
        /* save dimensions */
        demx=pcxhdr->hres+1;
        demy=pcxhdr->vres+1;

        /* dump the file data into the picture file */
        if (!fwrite((char *)(pic+128),(l-128-768),1,f)) {
            /* write error. aargh! */
            printf(" unable to write file '%s'\n",fname);
        }
    } else {
        hdr.gfx_offset=GFX_PIC_NOPIC;
    }
    /* add the PRIORITY picture */
    if(!(flags&NO_PRI_SCR)) {
        /* fill in offset field */
        hdr.pri_offset=ftell(f);
        /* load the picture */
        pic=loadfile(pri_filename,&l);
        pcxhdr=(struct PCXHEADER *)pic;
        /* check if it is a pcx file we are reading */
        if (!checkpcx(pcxhdr)) {
            /* no, so die */
            printf(" file '%s' is not a pcx file\n",pri_filename);
            exit(4);
        }
        /* save dimensions */
        demx=pcxhdr->hres+1;
        demy=pcxhdr->vres+1;

        /* dump the file data into the picture file */
        if (!fwrite((char *)(pic+128),(l-128-768),1,f)) {
            /* write error. aargh! */
            printf(" unable to write file '%s'\n",fname);
        }
        /* and free the previously read picture */
        free(pic);
    } else {
        hdr.pri_offset=GFX_PIC_NOPIC;
    }
    /* and last but not least, add the MASK picture */
    if(!(flags&NO_MSK_SCR)) {
        /* fill in offset field */
        hdr.msk_offset=ftell(f);
        /* load the picture */
        pic=loadfile(msk_filename,&l);
        pcxhdr=(struct PCXHEADER *)pic;
        /* check if it is a pcx file we are reading */
        if (!checkpcx(pcxhdr)) {
            /* no, so die */
            printf(" file '%s' is not a pcx file\n",msk_filename);
            exit(4);
        }
        /* save dimensions if not done already */
        if(!(demx+demy)) {
            demx=pcxhdr->hres+1;
            demy=pcxhdr->vres+1;
        } else {
            /* check the dimensions */
            if((demx!=pcxhdr->hres+1)||
               (demy!=pcxhdr->vres+1)) {
                printf(" dimensions of pcx file '%s' are not correct (%ux%u against %ux%u)\n",msk_filename,pcxhdr->hres+1,pcxhdr->vres+1,demx,demy);
                exit(4);
            }

        }
        /* dump the file data into the picture file */
        if (!fwrite((char *)(pic+128),(l-128-768),1,f)) {
            /* write error. aargh! */
            printf(" unable to write file '%s'\n",fname);
        }
        /* and free the previously read picture */
        free(pic);
    } else {
        hdr.msk_offset=GFX_PIC_NOPIC;
    }

    /* now jump to the beginning and write the correct header */
    rewind(f);
    hdr.picheight=demy;hdr.picwidth=demx;
    if(!fwrite(&hdr,sizeof(struct PICHEADER),1,f)) {
        /* failed. report and die */
        printf(" unable to write '%s'\n",fname);
        exit(2);
    }

    fclose(f);
}

int
main(int argc,char *argv[]) {
    _UINT i;

    printf("MAD Picture Generator Version 1.0 - (c) 1999 The MAD Crew\n\n");
    if (argc<3) {
        usuage();
        exit(1);
    }
    parseparams(argv,argc);
    printf("Creating picture file '%s'...",filename);
    createpic(filename);
    printf(" done\n");
    return 0;
}
