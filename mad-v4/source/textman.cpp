/*
                               TEXTMAN.CPP

                           MAD Text Manager

                      (c) 1999, 2000 The MAD Crew

  Description:
  This will manage all MAD text.

  Todo: Nothing

  Portability notes: Fully portable, no changes required
*/
#include <stdlib.h>
#include <string.h>
#include "archive.h"
#include "mad.h"
#include "maderror.h"
#include "textman.h"
#include "types.h"

/*
 * TEXTMAN::TEXTMAN()
 *
 * This is the constructor of the text manager object. It will set most fields
 * to zero.
 *
 */
TEXTMAN::TEXTMAN() {
    /* set all pointers to NULL to make sure they don't get freed */
    textdata=NULL;offsetdata=NULL;
}

/*
 * TEXTMAN::init(char* fname)
 *
 * This will initialize the text manager. It will load text file [fname].
 *
 */
void
TEXTMAN::init(char* fname) {
    char *ptr;
    char tempstr[MAD_TEMPSTR_SIZE];
    _ULONG textlen;

    /* read the file */
    ptr=(char*)archive_readfile(fname,&textlen);

    /* copy the text header to [hdr] */
    memcpy(&hdr,(void*)ptr,sizeof(hdr));

    /* verify the header */
    if(hdr.magic!=TEXTMAN_MAGICID) {
        /* this failed. die (error 600: is not a mad text file) */
        free(ptr);
        sprintf(tempstr,MAD_ERROR_600,fname);
        die(tempstr);
    }
    /* check the version number */
    if(hdr.version!=TEXTMAN_VERSIONO) {
        /* this failed. die (error 601: is a mad text of a wrong version) */
        free(ptr);
        sprintf(tempstr,MAD_ERROR_601,fname);
        die(tempstr);
    }

    /* allocate memory for the actual text data */
    if((textdata=(char*)malloc(textlen-sizeof(hdr)-(hdr.nofstrings*4)))==NULL) {
        /* this failed. die (error 2: out of memory (allocation of bytes failed)) */
        free(ptr);
        sprintf(tempstr,MAD_ERROR_2,textlen-sizeof(hdr)-(hdr.nofstrings*4));
        die(tempstr);
    }
    /* copy the data to [textdata] */
    memcpy(textdata,(void*)(ptr+sizeof(hdr)),textlen-sizeof(hdr)-(4*hdr.nofstrings));

    /* allocate memory for the offset table */
    if((offsetdata=(char*)malloc((hdr.nofstrings*4)))==NULL) {
        /* this failed. die (error 2: out of memory (allocation of bytes failed)) */
        free(ptr);
        sprintf(tempstr,MAD_ERROR_2,(hdr.nofstrings*4));
        die(tempstr);
    }
    /* copy the offsets to [offsetdata] */
    memcpy(offsetdata,(void*)(ptr+sizeof(hdr)+hdr.offset),(4*hdr.nofstrings));

    /* and free the memory not longer neccesary */
    free(ptr);
}

/*
 * TEXTMAN::getstring(_ULONG stringno,char* dest,_ULONG destlen)
 *
 * This will retrieve string [string] from our text file. It will be dumped
 * in [dest]. The size of dest should be specified in [destlen]. This will
 * die if [stringno] is not in the file.
 *
 */
void
TEXTMAN::getstring(_ULONG stringno,char* dest,_ULONG destlen) {
    char tempstr[MAD_TEMPSTR_SIZE];
    _ULONG ofs,i;
    _UINT len;

    /* does this string actually exists? */
    if(stringno>=hdr.nofstrings) {
        /* no. die (error 602: string number does not exists in the MAD text file) */
        sprintf(tempstr,MAD_ERROR_602,stringno);
        die(tempstr);
    }

    /* figure out the offset (XXX: TODO: fix this (doesn't work correct for offsets > 16 bit) */
    ofs=(_UCHAR)offsetdata[4*stringno]+(_UINT)(offsetdata[4*stringno+1]<<8);
    len=0;
    /* figure out the length */
    len=(_UINT)((_UCHAR)textdata[ofs]+((_UCHAR)textdata[ofs+1]<<8));
    len=~len;

    if(len>destlen) len=destlen;

    memset(dest,0,destlen);

    strcpy(tempstr,TEXTMAN_ENCRYPTSTRING);

    /* get the data and decrypt it to [dest] */
    for(i=0;i<len;i++) {
        dest[i]=(char)textdata[ofs+i+2]^tempstr[i%strlen(tempstr)];
    }
}

/*
 * TEXTMAN::done()
 *
 * This will deinitialize the text manager. It will free any memory in use.
 *
 */
void
TEXTMAN::done() {
    /* was there memory allocated for the text data? */
    if(textdata!=NULL) {
        /* yeah. free it */
        free(textdata);
        /* make sure we don't do this twice */
        textdata=NULL;
    }
    /* was there memory allocated for the offset data? */
    if(offsetdata!=NULL) {
        /* yeah. free it */
        free(offsetdata);
        /* make sure we don't do this twice */
        offsetdata=NULL;
    }
}
