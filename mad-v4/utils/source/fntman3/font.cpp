/*
                Font Manager Version 2.0
                 (c) 1999 The MAD Crew

   This version only handles fonts with version 2.0
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "font.h"
#include "../../../source/include/gfx.h"

/*
 * pow(int a, int b)
 *
 * This will return [a] raised by the power [b].
 *
 */
int pow(int a, int b) {
    int i,c=a;

    if (b==0) return 1;
    if (a==0) return 0;

    for(i=1;i<b;i++) c*=a;

    return c;
}

/*
 * FONT::init(char* fname)
 *
 * This will try to open font file [fname]. It will return FONT_ERROR_OK on
 * success or any of the other FONT_ERROR_xxx if anything fails.
 *
 */
int
FONT::init(char* fname) {
    /* it all went ok. return FONT_ERROR_OK */
    return FONT_ERROR_OK;
}

/*
 * FONT::create(char* fname)
 *
 * This will try to create font file [fname]. It will return FONT_ERROR_OK on
 * success or any of the other FONT_ERROR_xxx if anything fails.
 *
 */
int
FONT::create(char* fname) {
    int i;

    /* try to create the file */
    if((fontfile=fopen(fname,"wb"))==NULL) {
        /* this failed. die */
        return FONT_ERROR_OPENERR;
    }

    /* zero out the header */
    memset(&header,0,sizeof(header));

    /* nuke the size array */
    memset(&charsize,0,FONT_NOFCHARS);

    /* nuke the font data pointers */
    for(i=0;i<FONT_NOFCHARS;i++) {
        chardata[i]=NULL;
    }

    /* build it */
    header.magic=FONT_MAGICID;                /* magic id code */
    header.version=FONT_VERSIONO;             /* version number */
    header.height_width=0;                    /* not yet known */

    /* it all went ok. return FONT_ERROR_OK */
    return FONT_ERROR_OK;
}

/*
 * FONT::done()
 *
 * This will close the font file, and write the data to it. It will return
 * FONT_ERROR_OK if successful or any of the FONT_ERROR_xxx constants.
 *
 */
int
FONT::done() {
    int i;

    /* write the file */
    if ((i=write())!=FONT_ERROR_OK) {
        /* this failed. close the file and forward the error code */
        fclose(fontfile);
        return i;
    }

    /* close the font file */
    fclose(fontfile);

    /* say it's all ok */
    return FONT_ERROR_OK;
}

/*
 * FONT::write()
 *
 * This will write the font file to the disk. It will return FONT_ERROR_OK if
 * successful or any of the FONT_ERROR_xxx constants.
 *
 */
int
FONT::write() {
    _UINT i,j;
    _UCHAR a,b;

    #ifdef __FONT_DEBUG__
        /* debugging stuff */
        printf("Writing header...");
    #endif
    /* write the header */
    if(!fwrite(&header,sizeof(header),1,fontfile)) {
        /* this failed. return error */
        return FONT_ERROR_WRITERR;
    }
    #ifdef __FONT_DEBUG__
        /* debugging stuff */
        printf(" done\nWriting sizes...");
    #endif
    /* write the char height/width table */
    if(!fwrite(charsize,FONT_NOFCHARS,1,fontfile)) {
        /* this failed. return error */
        return FONT_ERROR_WRITERR;
    }
    #ifdef __FONT_DEBUG__
        /* debugging stuff */
        printf(" done\nWriting chars...");
    #endif

    /* write all char data */
    for(i=0;i<FONT_NOFCHARS;i++) {
        /* data there? */
        if(chardata[i]!=NULL) {
            /* yeah, write it */
            a=charsize[i];
            a=(a>>4);
            b=charsize[i]&0xf;
            j=((_UINT)a*(_UINT)b);
            j+=(8-(j%8));
            j/=8;
            #ifdef __FONT_DEBUG__
                /* debugging stuff */
                printf("%u (%ux%u = %u bytes) ",(_UINT)i,(_UINT)(charsize[i]>>(_UINT)4),(_UINT)(charsize[i]&(_UINT)0xf),(_UINT)j);
            #endif
            if(!fwrite(chardata[i],j,1,fontfile)) {
                /* this failed. return error */
                return FONT_ERROR_WRITERR;
            }
        }
    }
    #ifdef __FONT_DEBUG__
        /* debugging stuff */
        printf(" done\n");
    #endif

    /* it all went ok. return FONT_ERROR_OK */
    return FONT_ERROR_OK;
}

/*
 * FONT::err2msg(int errno)
 *
 * This will return a readable string of error code [errno].
 *
 */
char*
FONT::err2msg(int errno) {
    switch (errno) {
             case FONT_ERROR_OK: return "everything's ok";
        case FONT_ERROR_WRITERR: return "write error";
        case FONT_ERROR_READERR: return "read error";
        case FONT_ERROR_OPENERR: return "open error";
     case FONT_ERROR_VERSIONERR: return "wrong version";
       case FONT_ERROR_MAGICERR: return "this is not a MAD font file";
       case FONT_ERROR_ALLOCERR: return "out of memory";
     case FONT_ERROR_CHARTOOBIG: return "char too big";
    }
    return "unknown error. help!";
}

/*
 * FONT::addchar(char* fname,_UCHAR charno,_UCHAR opaquecolor)
 *
 * This will add a char [charno] as sprite filename [fname], with opaque color
 * [opaquecolor]. It will return FONT_ERROR_OK if all went ok, or
 * FONT_ERROR_xxx if any error occours.
 *
 */
int
FONT::addchar(char* fname,_UCHAR charno,_UCHAR opaquecolor) {
    FILE*  f;
    _UINT  i,bitsize,bytesize,bitpos;
    _ULONG size;
    _UCHAR ch;
    char*  dataptr;
    struct SPRITEHEADER* sprhdr;
    char*  ptr;

    /* open the sprite file */
    if((f=fopen(fname,"rb"))==NULL) {
        /* this failed. return error */
        return FONT_ERROR_OPENERR;
    }

    /* get the file size */
    fseek(f,0,SEEK_END);size=ftell(f);rewind(f);

    /* allocate memory for the file */
    if((ptr=(char*)malloc(size))==NULL) {
        /* this failed. return error */
        return FONT_ERROR_ALLOCERR;
    }

    /* read the file's contents */
    if(!fread(ptr,size,1,f)) {
        /* this failed. die */
        fclose(f); free(ptr);
        return FONT_ERROR_READERR;
    }

    /* close the file */
    fclose(f);

    /* create a pointer to the header */
    sprhdr=(struct SPRITEHEADER*)ptr;

    /* verify the header */
    if(sprhdr->idcode!=GFX_SPR_MAGICNO) {
        /* this failed. die */
        return FONT_ERROR_MAGICERR;
    }
    if(sprhdr->version!=GFX_SPR_VERSIONO) {
        /* this failed. die */
        return FONT_ERROR_VERSIONERR;
    }
    #ifdef __FONT_DEBUG__
      /* show the sizes (debugging!) */
      printf("Char size: %ux%u\n",sprhdr->height,sprhdr->width);
    #endif

    /* is the char small enough? */
    if((sprhdr->height>15)||(sprhdr->width>15)) {
        /* nope. return error */
        fclose(f);free(ptr);
        return FONT_ERROR_CHARTOOBIG;
    }

    /* is there old char data? */
    if(chardata[charno]!=NULL) {
        /* yeah, nuke it */
        free(chardata[charno]);
        charsize[charno]=0;
    }

    /* calculate the bit and bytesize */
    bitsize=sprhdr->height*sprhdr->width;
    bytesize=(bitsize/8);
    if ((bitsize*8)!=bitsize) bytesize++;

    /* allocate memory for the char data */
    if((chardata[charno]=(char*)malloc(bytesize))==NULL) {
        /* this failed. return error */
        fclose(f);
        return FONT_ERROR_ALLOCERR;
    }

    /* fill it with zeroes */
    memset(chardata[charno],0,bytesize);

    #ifdef __FONT_DEBUG__
      /* show more sizes (debugging!) */
      printf("Bit size: %u\nByte size: %u\n",bitsize,bytesize);
    #endif

    /* iterate one byte for each pixel in the character image */
    bitpos=0;dataptr=(char*)(ptr+sizeof(struct SPRITEHEADER));
    for (i=0;i<bitsize;i++) {
        /* if color is not transparant, indicate this pixel should be drawn */
        if ((dataptr[0]!=sprhdr->transcol_r)||(dataptr[1]!=sprhdr->transcol_g)||(dataptr[2]!=sprhdr->transcol_b)) {
            chardata[charno][bitpos/(sizeof(char)*8)] |= pow(2,(bitpos%(sizeof(char)*8)));
        }
        /* move to next bit */
        bitpos++;
        /* and to the next pixel */
        dataptr+=3;
    }

    /* construct the size of the char */
    charsize[charno]=(sprhdr->height<<4)+sprhdr->width;

    #ifdef __FONT_DEBUG__
        printf("\nStored:\n");
        for (i=0;i<bitsize;i++) {
            printf("%c",((chardata[charno][i/(sizeof(char)*8)] & pow(2,(i%(sizeof(char)*8))))!=0)?219:32);
	    if ((i+1)%sprhdr->width==0)printf("  (0x%x)\n",(_UCHAR)chardata[charno][i/(sizeof(char)*8)]);
	}
    #endif

    /* close the file */
    fclose(f);

    /* it all went ok. return FONT_ERROR_OK */
    return FONT_ERROR_OK;
}
