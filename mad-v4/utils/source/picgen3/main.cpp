/*
 *                    MAD Picture Generator Version 3.0
 *                      (c) 1999, 2000 the MAD Crew
 *
 */
#define  BANNER                 "MAD Picture Generator 3.0 - (c) 1999, 2000 The MAD Crew\n"
#define  MAX_FILENAME_LEN       64

#define  NO_MSK_SCR 1
#define  NO_PRI_SCR 2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../source/include/gfx.h"
#include "../../../source/include/types.h"

/* header structure */
struct PCXHEADER {
    _UCHAR manufacturer;         /* must be 0xa, for ZSoft PCX */
    _UCHAR version;              /* version number */
    _UCHAR encoding;             /* must be 1 for RLE */
    _UCHAR bpp;                  /* must be 32 for 16.8 million color */
    _UCHAR xmin,ymin,xmax,ymax;  /* picture dimensions */
    _UINT  hres;                 /* horizonal size for device */
    _UINT  vres;                 /* vertical size for device */
    _UCHAR colormap[48];         /* palette, ignored */
    _UCHAR reserved;             /* some reserved junk */
    _UINT  bytesperline;         /* bytes per line */
    _UINT  palinfo;              /* palette information */
    _UCHAR filler[62];           /* filler */
};

char       filename[MAX_FILENAME_LEN];
char       gfx_filename[MAX_FILENAME_LEN];
char       msk_filename[MAX_FILENAME_LEN];
char       pri_filename[MAX_FILENAME_LEN];
_UCHAR     flags;

/*
 * usuage()
 *
 * This will show the program usuage.
 *
 */
void
usuage() {
    fprintf(stderr,"%s\n",BANNER);
    fprintf(stderr,"Usuage: PICGEN [parameters] filename\n\n");
    printf("Parameters can be:\n");
    printf("    -G[filename]   use [filename] as graphics screen\n");
    printf("    -P[filename]   use [filename] as priority screen\n");
    printf("    -p             don't use a priority screen\n");
    printf("    -M[filename]   use [filename] as mask screen\n");
    printf("    -m             don't use a mask screen\n\n");
    printf("As you might have guessed, the parameters are cAsE sEnSiTiVe\n");
    exit(1);
}

/*
 * rle_decompress(char* in,char* out,_ULONG outsize)
 *
 * This will RLE-decompress [in] to [outsize] bytes in [out]. It will return
 * the number of bytes used from [in].
 *
 */
_ULONG
rle_decompress(char* in,char* out,_ULONG outsize) {
    _ULONG pos,offset;
    _UCHAR c,loop;

    pos=0;offset=0;
    while (offset<outsize) {
        c=in[pos];                /* get byte */
        if ((c&0xc0)==0xc0) {     /* are bits 7 and 8 set? */
            /* yes, its a loop */
            loop=(c&63);          /* strip the bits off, we have our loop value */
            while (loop--) {
                out[offset++]=in[pos+1];
            }
            pos++;
        } else {
            /* just one byte */
            out[offset++]=in[pos];
        }
        pos++;
    }

    /* return the number of bytes used */
    return pos;
}

/*
 * loadpcx(char* fname,_UCHAR is8bit,_UINT* hres,_UINT* vres,_ULONG* picsize)
 *
 * This will open PCX file [fname] and return the data in [RGB] pairs. If
 * [is8bit] is non-zero, it will read the file as a 8 bit PCX file, otherwise
 * as a true-color (24bit) PCX file. The resolution will be dumped into
 * [hres] and [vres], and the size in [picsize].
 *
 */
char*
loadpcx(char* fname,_UCHAR is8bit,_UINT* hres,_UINT* vres,_ULONG* picsize) {
    FILE* f;
    char* tmp;
    char* ptr;
    char* planetmp;
    char* imagedata;
    struct PCXHEADER hdr;
    _ULONG size,i,j,tsize;
    _ULONG h_res,v_res,bpp;

    /* open the file */
    if((f=fopen(fname,"rb"))==NULL) {
        /* this failed. die */
        fprintf(stderr,"PICGEN: couldn't open file '%s'\n",fname);
        exit(1);
    }

    /* get the filesize */
    fseek(f,0,SEEK_END);size=ftell(f);rewind(f);

    /* is the file large enough (>128 bytes)? */
    if(size<128) {
        /* no. this can never be a PCX file */
        fprintf(stderr,"PICGEN: file '%s' cannot be a PCX file, it's too small\n",fname);
        exit(1);
    }

    /* subtract all useless junk from the size */
    size-=128;

    /* try to read the header */
    if(!fread(&hdr,sizeof(PCXHEADER),1,f)) {
        /* this failed. die */
        fprintf(stderr,"PICGEN: couldn't read file '%s'\n",fname);
        exit(1);
    }

    /* is this a PCX file? */
    if(hdr.manufacturer!=0xa) {
        /* no. complain */
        fprintf(stderr,"PICGEN: file '%s' isn't a PCX file\n",fname);
        exit(1);
    }

    /* version ok? */
    if(hdr.version!=5) {
        /* no. complain */
        fprintf(stderr,"PICGEN: file '%s' is of the wrong PCX file version (only version 5 supported)\n",fname);
        exit(1);
    }

    /* how much bpp is this? (psp fills in 8, even for true-color images... hmm...) */
    if(hdr.bpp!=8) {
        fprintf(stderr,"PICGEN: file '%s' is isn't true-color (only true-color images supported)\n",fname);
        exit(1);
    }

    /* get the dimensions */
    h_res=(hdr.hres+1); v_res=(hdr.vres+1);
    if(hres!=NULL) *hres=h_res;
    if(vres!=NULL) *vres=v_res;

    /* 24 bit? */
    if(!is8bit) {
        /* yup */
        bpp=3;
    } else {
        /* no */
        bpp=1;
    }

    /* allocate memory for the actual image */
    if((imagedata=(char*)malloc(h_res*v_res*GFX_BPP))==NULL) {
        /* this failed. die */
        fprintf(stderr,"PICGEN: unable to allocate memory for the PCX image");
        exit(1);
    }

    /* allocate memory for our temponary buffer */
    if((tmp=(char*)malloc(size))==NULL) {
        /* this failed. die */
        fprintf(stderr,"PICGEN: unable to allocate temponary memory for the PCX image");
        exit(1);
    }

    /* allocate memory for the plane buffer */
    if((planetmp=(char*)malloc(h_res*v_res*GFX_BPP))==NULL) {
        /* this failed. die */
        fprintf(stderr,"PICGEN: unable to allocate temponary memory for the plane buffer");
        exit(1);
    }

    /* try to read the data */
    if(!fread(tmp,size,1,f)) {
        /* this failed. die */
        fprintf(stderr,"PICGEN: couldn't read file '%s'\n",fname);
        exit(1);
    }

    /* 24 bit PCX file? */
    if(!is8bit) {
        /* yup. merge this into the overall buffer */
        ptr=tmp;
        for(j=0;j<v_res;j++) {
            /* decompress this line in RGB pairs */
            size=rle_decompress(ptr,planetmp,(h_res*bpp));
            ptr+=size;
            for(i=0;i<h_res;i++) {
                imagedata[(h_res*j*GFX_BPP)+(i*GFX_BPP)]=planetmp[(i+h_res+h_res)];
                imagedata[(h_res*j*GFX_BPP)+(i*GFX_BPP)+1]=planetmp[(i+h_res)];
                imagedata[(h_res*j*GFX_BPP)+(i*GFX_BPP)+2]=planetmp[i];
            }
        }
        if(picsize!=NULL) *picsize=(h_res*v_res*GFX_BPP);
    } else {
        /* just decompress it */
        rle_decompress(tmp,imagedata,(h_res*v_res));
        if(picsize!=NULL) *picsize=(h_res*v_res);
    }

    /* zap the temp buffers */
    free(planetmp); free(tmp);

    /* close the file */
    fclose(f);

    /* return the new buf */
    return imagedata;
}

/*
 * createpic(char* fname)
 *
 * This will actually create the picture.
 *
 */
void
createpic(char* fname) {
    FILE* f;
    struct PICHEADER hdr;
    _UINT hsize,vsize,i,j;
    _ULONG size;
    char* ptr;

    /* open the file */
    if((f=fopen(fname,"wb"))==NULL) {
        /* this failed. die */
        fprintf(stderr,"PICGEN: Unable to create file '%s'\n",fname);
        exit(1);
    }

    /* create a bogus header (will be changed later) */
    memset(&hdr,0,sizeof(struct PICHEADER));
    hdr.idcode=GFX_PIC_MAGICNO;
    hdr.version=GFX_PIC_VERSIONO;
    /* and write it to the file */
    if(!fwrite(&hdr,sizeof(struct PICHEADER),1,f)) {
        /* failed. report and die */
        fprintf(stderr,"PICGEN: unable to write to '%s'\n",fname);
        exit(2);
    }
    /*
     * Add graphics screen
     *
     */

    /* fill in offset field */
    hdr.gfx_offset=ftell(f);
    /* get the picture */
    ptr=loadpcx(gfx_filename,0,&hsize,&vsize,&size);

    /* write it to the picture file */
    if(!fwrite(ptr,size,1,f)) {
        /* failed. report and die */
        free(ptr);
        fprintf(stderr,"PICGEN: unable to write to '%s'\n",fname);
        exit(2);
    }

    /* free the temp buffer */
    free(ptr);

    /*
     * Add priority screen
     *
     */
    /* need to do this? */
    if(!(flags&NO_PRI_SCR)) {
        /* yup. do it */
        /* fill in offset field */
        hdr.pri_offset=ftell(f);
        /* get the picture */
        ptr=loadpcx(pri_filename,1,&i,&j,&size);

        /* check the dimension */
        if((i!=hsize)||(j!=vsize)) {
            /* they are not correct. complain */
            free(ptr);
            fprintf(stderr,"PICGEN: file '%s' has wrong dimensions\n",fname);
            exit(2);
        }

        /* write it to the picture file */
        if(!fwrite(ptr,size,1,f)) {
            /* failed. report and die */
            free(ptr);
            fprintf(stderr,"PICGEN: unable to write to '%s'\n",fname);
            exit(2);
        }

        /* free the temp buffer */
        free(ptr);
    } else {
        /* no priority screen here */
        hdr.pri_offset=GFX_PIC_NOPIC;
    }

    /*
     * Add mask screen
     *
     */
    /* need to do this? */
    if(!(flags&NO_MSK_SCR)) {
        /* yup. do it */
        /* fill in offset field */
        hdr.msk_offset=ftell(f);
        /* get the picture */
        ptr=loadpcx(msk_filename,1,&i,&j,&size);

        /* check the dimension */
        if((i!=hsize)||(j!=vsize)) {
            /* they are not correct. complain */
            free(ptr);
            fprintf(stderr,"PICGEN: file '%s' has wrong dimensions\n",fname);
            exit(2);
        }

        /* write it to the picture file */
        if(!fwrite(ptr,size,1,f)) {
            /* failed. report and die */
            free(ptr);
            fprintf(stderr,"PICGEN: unable to write to '%s'\n",fname);
            exit(2);
        }

        /* free the temp buffer */
        free(ptr);
    } else {
        /* no priority screen here */
        hdr.msk_offset=GFX_PIC_NOPIC;
    }

    /* edit the header */
    hdr.picheight=vsize;
    hdr.picwidth=hsize;

    /* rewrite the header */
    rewind(f);
    if(!fwrite(&hdr,sizeof(struct PICHEADER),1,f)) {
        /* failed. report and die */
        fprintf(stderr,"PICGEN: unable to write to '%s'\n",fname);
        exit(2);
    }

    /* close the file */
    fclose(f);
}

/*
 * main(int argc,char* argv[])
 *
 * Geez, I wouldn't know...
 *
 */
int
main(int argc,char* argv[]) {
    _UINT i,j;
    char* ptr;

    /* parse the parameters */
    strcpy(filename,"");strcpy(gfx_filename,"");strcpy(msk_filename,"");strcpy(pri_filename,"");
    flags=0;
    for(i=1;i<argc;i++) {
        /* does it begin with a -? */
        if(argv[i][0]=='-') {
            /* yup. must be a parameter */
            switch(argv[i][1]) {
                case 'h':
                case '?': /* help */
                          usuage();
                          /* NOTREACHED */
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
                 default: /* unknown parameter */
                          fprintf(stderr,"PICGEN: Unknown parameter '%c'\n",argv[i][1]);
                          exit(1);
                          /* NOTREACHED */
            }
        } else {
            /* it must be the filename. do we already have one? */
            if(strlen(filename)) {
                /* yup. complain */
                fprintf(stderr,"PICGEN: filename already given\n");
                exit(1);
            }
            /* is the length ok? */
            if(strlen(argv[i])>MAX_FILENAME_LEN) {
                /* no. complain */
                fprintf(stderr,"PICGEN: filename length '%s' too long\n");
                exit(1);
            }
            /* all looks good. copy the filename */
            strcpy(filename,argv[i]);
        }
    }

    /* was a filename given? */
    if(!strlen(filename)) {
        /* no. complain */
        fprintf(stderr,"PICGEN: no filename given\n");
        usuage();
        /* NOTREACHED */
    }

    /* check for enough/too much options */
    if((flags&NO_PRI_SCR)&&(strlen(pri_filename))) {
        fprintf(stderr,"PICGEN: Why did you specify a priority filename and the -p flag?\n");
        exit(1);
    }
    if((flags&NO_MSK_SCR)&&(strlen(msk_filename))) {
        fprintf(stderr,"PICGEN: Why did you specify a mask filename and the -m flag?\n");
        exit(1);
    }
    if(!strlen(gfx_filename)) {
        fprintf(stderr,"PICGEN: You didn't specify the graphics screen to use!\n");
        exit(1);
    }
    if((!(flags&NO_PRI_SCR))&&(!strlen(pri_filename))) {
        fprintf(stderr,"PICGEN: You didn't use one of the -p or -P flags. I don't know what to do!\n");
        exit(1);
    }
    if((!(flags&NO_MSK_SCR))&&(!strlen(msk_filename))) {
        fprintf(stderr,"PICGEN: You didn't use one of the -m or -M flags. I don't know what to do!\n");
        exit(1);
    }

    /* create the picture */
    createpic(filename);
    return 0;
}
