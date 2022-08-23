/*
                              ARCHIVE.CPP

                        MAD Archive Engine 2.0

                        (c) 1999 The MAD Crew

  Description:
  This will preform all the archive file functions. If a file is found in the
  current directory it will use that file instead.

  Todo: add compression of the archive using LZW or Hufmann

  Portability notes: Fully portable, no changes required
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "archive.h"
#include "maderror.h"
#include "mad.h"
#include "types.h"

_UCHAR
ARCHIVE::init(char *fname) {
    /* make sure archivefile is a NULL pointer. windows doesn't like fclose() calls
       with non-open files */
    archivefile=NULL;
    /* try to open the file as read only. exit if failure */
    if ((archivefile=fopen(fname,"rb"))==NULL) {
        return ARCHIVE_OPENERR;
    }
    /* read a header */
    if (!fread(&header,sizeof(header),1,archivefile)) return ARCHIVE_READERR;

    /* verify header */
    if (header.idcode!=ARCHIVE_MAGICNO) return ARCHIVE_NOMADARCHIVE;
    if (header.version!=ARCHIVE_VERSION) return ARCHIVE_WRONGVERSION;

    /* its ok */
    return ARCHIVE_OK;
}

void
ARCHIVE::done() {
    if(archivefile!=NULL) {
        fclose(archivefile);
    }
}

_UINT
ARCHIVE::getnofiles() {
    return (header.nofiles);
}

_UINT
ARCHIVE::getversion() {
    return (header.version);
}

_ULONG
ARCHIVE::getentrypos() {
    return (header.entrypos);
}

_UCHAR
ARCHIVE::findfirst(struct ARCHIVEFILE *a) {
    /* if no files in the archive, return ARCHIVE_NOFILES */
    if (!header.nofiles) return ARCHIVE_NOFILES;

    /* seek to the file entry table */
    fseek(archivefile,header.entrypos,SEEK_SET);
    /* and read the first file entry */
    if (!fread(a,sizeof(struct ARCHIVEFILE),1,archivefile)) {
        /* that failed, so return ARCHIVE_READERR */
        return ARCHIVE_READERR;
    }
    /* all went ok. return ARCHIVE_OK */
    return ARCHIVE_OK;
}

_UCHAR
ARCHIVE::findnext(struct ARCHIVEFILE *a) {
    /* try to read a next entry */
    if (!fread(a,sizeof(struct ARCHIVEFILE),1,archivefile)) {
        /* that didnt work. die with an ARCHIVE_READERR error */
        return ARCHIVE_READERR;
    }
    /* all went ok. return ARCHIVE_OK */
    return ARCHIVE_OK;
}

_UCHAR
ARCHIVE::findfile(char *fname,struct ARCHIVEFILE *fe) {
    _UCHAR result;

    /* seek to first entry */
    if ((result=findfirst(fe))!=ARCHIVE_OK) return result;

    /* keep searching. if any error, say it is not in the archive */
    while (strcmp(fname,fe->filename)) {
        if (findnext(fe)!=ARCHIVE_OK) return ARCHIVE_NOTINARCHIVE;
    }

    /* we found the file. return ARCHIVE_OK */
    return ARCHIVE_OK;
}

void
*ARCHIVE::readfile(char *fname,_ULONG *rsize) {
    struct ARCHIVEFILE entry;
    _ULONG size,pos;
    char chunk[ARCHIVE_CHUNKSIZE];
    void *ptr;
    FILE *f;

    /* first check whether the file exists in the current directory */
    if ((f=fopen(fname,"rb"))!=NULL) {
        /* yeah, it does. lets read that file instead of the archive */
        /* figure out the size */
        fseek(f,0,SEEK_END); size=ftell(f); rewind(f);

        /* allocate memory for the data */
        if ((ptr=malloc(size))==NULL) {
            /* error 2: out of memory */
            sprintf(chunk,MAD_ERROR_2,size);
            die(chunk);
        }
        /* read the file to the buffer */
        if (!fread(ptr,size,1,f)) {
            /* this failed. die (error 3: read error) */
            sprintf(chunk,MAD_ERROR_3,fname);
            die(chunk);
        }
        /* and close the file */
        fclose (f);

        /* if the user specified a valid pointer to copy the size to, copy it */
        if (rsize!=NULL) *rsize=size;

        /* return the pointer to the file */
        return ptr;
    }

    /* find the file */
    if (findfile(fname,&entry)!=ARCHIVE_OK) {
        /* the file was not found. die (error 4: file '%s' not found in archive) */
        sprintf(chunk,MAD_ERROR_4,fname);
        die(chunk);
    }

    /* store the amount of bytes to go */
    size=entry.size;
    pos=0L;
    /* allocate memory for the data */
    if ((ptr=malloc(size))==NULL) {
        /* error 2: out of memory */
        sprintf(chunk,MAD_ERROR_2,size);
        die(chunk);
    }

    /* seek to offset of file */
    fseek(archivefile,entry.offset,SEEK_SET);

    /* read the file completely into the memory buffer */
    if (!fread(ptr,size,1,archivefile)) {
        /* this failed. die (error 3: read error */
        sprintf(chunk,MAD_ERROR_3,fname);
        die(chunk);
    }
    /* if the user specified a valid pointer to copy the size to, copy it */
    if (rsize!=NULL) *rsize=size;

    /* return the pointer to the file */
    return ptr;
}

char
*ARCHIVE::err2msg(_UCHAR errno) {
    switch (errno) {
               case ARCHIVE_OK: return ARCHIVE_ERROR_0;
          case ARCHIVE_OPENERR: return ARCHIVE_ERROR_1;
          case ARCHIVE_READERR: return ARCHIVE_ERROR_2;
          case ARCHIVE_WRITERR: return ARCHIVE_ERROR_3;
     case ARCHIVE_NOMADARCHIVE: return ARCHIVE_ERROR_4;
     case ARCHIVE_WRONGVERSION: return ARCHIVE_ERROR_5;
      case ARCHIVE_OUTOFMEMORY: return ARCHIVE_ERROR_6;
          case ARCHIVE_NOFILES: return ARCHIVE_ERROR_7;
     case ARCHIVE_NOTINARCHIVE: return ARCHIVE_ERROR_8;
         case ARCHIVE_CREATERR: return ARCHIVE_ERROR_9;
    }
    return MAD_ERROR_UNKNOWN;
}
