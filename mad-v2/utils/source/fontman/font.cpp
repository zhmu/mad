#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "font.h"

#define __FONTMAN_DEBUG__

int pow(int a, int b) {
    int i,c=a;

    if (b==0) return 1;
    if (a==0) return 0;

    for(i=1;i<b;i++) c*=a;

    return c;
}

FONT::FONT() {
    /* initialize the variables */
    header = NULL;
    datasize = 0;
    color = FONT_DEFAULTCOLOR;
}

void
FONT::init() {

}

void
FONT::done() {
    int i;
    /* clean up stuff */
    if (header!=NULL) { delete header; }
    for (i=0;i<256;i++) { if (data[i]!=NULL) delete data[i]; }
}

char
FONT::charexists(_UCHAR c) {
    return (header->entry[c/(sizeof(char)*8)] & pow(2,c%(sizeof(char)*8)))!=0;
}

/*
 * _UCHAR FONT::save(char* filename)
 *
 * Saves the current font in the file filename
 */
_UCHAR
FONT::save(char* filename) {
    int i;										/* Counter variable */
    FILE *fil;									/* Font file to write to */
    _UCHAR shifts=0,nshifts=0;					/* Current shift state */
    _UINT bitsize=header->width*header->height;	/* Size of 1 char in bits */
    _UINT bpos;									/* Current position in bytes */
    _UCHAR curByte=0, curOByte=0, restByte=0;	/* Pointers */

    _UINT bytepos,bytesize,curbyte;

    /* open file and return if error */
    if ((fil=fopen(filename,"wb"))==NULL) { return FONT_OPENERR; }
    /* seek to beginning */
    fseek(fil,0,SEEK_SET);

    /* save header */
    if (fwrite(header,sizeof(struct FONTHEADER),1,fil)!=1) { fclose(fil); return FONT_WRITEERR; }

#ifdef __FONTMAN_DEBUG__
                printf("Writing: \n");
#endif

	/* Loop for each character in the font */
    for (i=0;i<256;i++) {
    	/* Only write character if this character is used */
         if (charexists(i)) {
          #ifdef __FONTMAN_DEBUG__
            printf("character %d:\r\n",i);
          #endif /* __FONTMAN_DEBUG__ */
          /* Check if the data is valid */
          if (data[i]==NULL) { fclose(fil); return FONT_NOFONTDAT; }
          bytesize = (int)(bitsize/8);
          if(bitsize%8) bytesize++;
          printf("bitsize: %u Bytesize: %u\n",bitsize,bytesize);
          /* Now write each byte of this character */
          for (bytepos=0;bytepos<bytesize;bytepos++) {
             /* get the current byte */
             curbyte = data[i][bytepos];
             if (fwrite(&curbyte,sizeof(char),1,fil)!=1) { fclose(fil); return FONT_WRITEERR; }
			 printf(" %x",(int)(char)curbyte);
          }
        }
    }

    fclose(fil);
	return FONT_OK;

	/*
     * OLD CODE
     */


    /* Loop for each character in the font */
    for (i=0;i<256;i++) {
        /* Only write charecter if this char is used */
        if (charexists(i)) {
            /* Check if the data is valid; otherwise return with error */
            if (data[i]==NULL) { fclose(fil); return FONT_NOFONTDAT; }
            /* iterate for each byte of this character */
            for (bpos=0;bpos<(bitsize/(sizeof(char)*8));bpos++) {
                /* Current byte in variable, shifted the apropriate times */
                curOByte = data[i][bpos] << shifts;
                /* Add the previous value to it */
                curByte = curOByte + restByte;
#ifdef __FONTMAN_DEBUG__
                printf("%.2x ",curByte);
#endif
                if (fwrite(&curByte,sizeof(char),1,fil)!=1) { fclose(fil); return FONT_WRITEERR; }
                /* restByte are the lowest bits of the byte, which have not
                   been written */
                restByte = (data[i][bpos]&0xff) >> (8*sizeof(char)-shifts);
            }
            /* If for this character not all bits have been written */
            if (bpos*8<bitsize) {
                /* put it in restByte and set the shifts variable */
                /* first set the shifts variable */
                nshifts+=bitsize-bpos*8;
                /* If #shifts exceeds a byte (7 bits), write one byte
                   and decrease shifts by 8 */
                if (nshifts>(8*sizeof(char)-1)) {
                    curOByte=data[i][bpos] << shifts;
                    curByte=curOByte + restByte;
                    if (fwrite(&restByte,sizeof(char),1,fil)!=1) { fclose(fil); return FONT_WRITEERR; }
	                restByte = (data[i][bpos]&0xff) >> (8*sizeof(char)-shifts);
                    nshifts-=8;
                }
                shifts=nshifts;
                restByte=(data[i][bpos]&0xff) >> (sizeof(char)*8-shifts);
            }
        }
    }
 	if (shifts!=0) {
        fwrite(&restByte,sizeof(char),1,fil);
        /* XXX: TODO: Error checking */
        printf("%.2x \n",restByte);
    }
    fclose(fil);
    return FONT_OK;
}

_UCHAR
FONT::deletechar(_UCHAR c) {
    /* clean up allocated memory */
    if (data[c]!=NULL) delete data[c];
    /* and indicate in header this char isn't used */
    header->entry[c/(sizeof(char)*8)] &= 0xff - pow(2,c%(sizeof(char)*8));
    /* this function cannot fail */
    return FONT_OK;
}

_UCHAR
FONT::setchar(FILE* f,_UCHAR c,_UCHAR o) {
    _UINT tmpint;
    _UINT bitsize=header->height*header->width;
    _UINT bitpos=0;
    _UINT w, h, i;
    _UCHAR ch;

    /* free if previously allocated memory */
    if (data[c]!=NULL) delete data[c];
    /* allocate memory for this character iamge, check for errs and clear it */
    tmpint=bitsize/(sizeof(char)*8)+1;
    data[c]=new char[tmpint];
    if (data[c]==NULL) return FONT_ALLOCERR;
    memset(data[c],0,tmpint);

    /* read sprite header and check sizes */
    if (fread(&w,sizeof(_UINT),1,f)!=1) { deletechar(c); return FONT_READERR; }
    if (fread(&h,sizeof(_UINT),1,f)!=1) { deletechar(c); return FONT_READERR; }
    if (w!=header->width || h!=header->height) { deletechar(c); return FONT_SIZEERR; }

    /* iterate one byte for each pixel in the character image */
    for (i=0;i<bitsize;i++) {
        /* read next byte of character image. If error, free and return */
        if (fread(&ch,sizeof(char),1,f)!=1) {
           deletechar(c);
           return FONT_READERR;
        }
        /* if color is not transparant, indicate this pixel should be drawn */
        if (ch!=o) {
              data[c][bitpos/(sizeof(char)*8)] |= pow(2,(bitpos%(sizeof(char)*8)));
              printf("data[%u][%u] |= $%x\n",c,bitpos/(sizeof(char)*8),(_UCHAR)pow(2,(bitpos%(sizeof(char)*8))));
        }
        /* move to next bit */;
        bitpos++;
    }

#ifdef __FONTMAN_DEBUG__
    printf("\nStored:\n");
    for (i=0;i<bitsize;i++) {
		printf("%d",(data[c][i/(sizeof(char)*8)] & pow(2,(i%(sizeof(char)*8))))!=0  );
		if ((i+1)%header->width==0)printf("  (0x%x)\n",(_UCHAR)data[c][i/(sizeof(char)*8)]);
	}
#endif

    /* indicate in header this char is used */
    header->entry[c/(sizeof(char)*8)] |= pow(2,(c%(sizeof(char)*8)));
    /* everything was ok */
    return FONT_OK;
}


_UCHAR
FONT::getchr(FILE *f,char c,_UCHAR bcol,_UCHAR fcol) {
}


_UCHAR
FONT::initreadwrite(char *fname) {
    FILE *f;
    int i, j;
    _UINT charsize;

    /* try to open the file as read/write. exit if failure */
    if ((f=fopen(fname,"r+n"))==NULL) {
       return FONT_OPENERR;
    }

    /* move to beginning of file */
    fseek(f, 0, SEEK_SET);

    /* allocate header. If error, return error */
    header = new struct FONTHEADER;
    if (header==NULL) { fclose(f); return FONT_ALLOCERR; }
    /* read header. If error, return error */
    if (fread(header,sizeof(struct FONTHEADER),1,f)!=1) { fclose(f); return FONT_READERR; }

    /* check header values */
    if (header->magic!=FONT_MAGICID) {fclose(f); return FONT_MAGICERR; }
    if (header->version<FONT_VERSIONNO) {fclose(f); return FONT_VERSIONERR; }

    /* compute size of character and round on bytes and convert to bytes */
    charsize = header->width*header->height;
    charsize += charsize%(sizeof(char)*8);
    charsize /= sizeof(char)*8;

    /* load all character images */
    for (i=0;i<256;i++) {
        if (charexists(i)) {
            data[i]=new char[charsize];
            /* read character data and if error, free memory and return */
            if (fread(data[i],charsize,1,f)!=1) {
                for (j=0;j<i;i++) delete data[j];
                delete header;
                header=NULL;
                return FONT_READERR;
            }
        } else {
            /* initialize item with NULL, so we know it isn't used */
            data[i]=NULL;
        }
        /* next data item */
    }

    /* its ok */
    return FONT_OK;
}

_UCHAR
FONT::create(char *fname) {
    FILE *fontfile;


    /* try too create the file. exit if failure */
    if ((fontfile=fopen(fname,"wb"))==NULL) {
       return FONT_OPENERR;
    }

    /* allocate header */
    header = new struct FONTHEADER;
    if (header==NULL) { fclose(fontfile); return FONT_ALLOCERR; }
    /* clear header */
    memset(header,0,sizeof(struct FONTHEADER));

    header->magic=FONT_MAGICID;
    header->version=FONT_VERSIONNO;
    header->width=0;
    header->height=0;

    /* write the header */
    if (!fwrite(header,sizeof(struct FONTHEADER),1,fontfile)) return FONT_WRITEERR;

    /* close the file */
    fclose(fontfile);

    /* clean up memory */
    if (header!=NULL) delete header;

    /* let initreadwrite handle it */
    return (initreadwrite(fname));
}


char*
FONT::err2msg(_UINT err) {
  switch(err) {
  case FONT_OK:         return "No error occured";
  case FONT_WRITEERR:   return "Error while writing file";
  case FONT_READERR:    return "Error while reading file";
  case FONT_OPENERR:    return "Error while opening file";
  case FONT_VERSIONERR: return "Incorrect version";
  case FONT_MAGICERR:   return "Incorrect file type (magicid)";
  case FONT_ALLOCERR:   return "Could not allocate memory";
  case FONT_NOFONTDAT:  return "Character not found in font";
  case FONT_SIZEERR:    return "Incorrect image size";
  }
  return "Unkown error";
}

