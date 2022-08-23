#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include "../../../source/include/archive.h"
#include "../../../source/include/mad.h"
#include "../../../source/include/types.h"

_UCHAR
ARCHIVE::init(char *fname) {
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

_UCHAR
ARCHIVE::initreadwrite(char *fname) {
	/* try to open the file as read/write. exit if failure */
        if ((archivefile=fopen(fname,"r+b"))==NULL) {
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

_UCHAR
ARCHIVE::create(char *fname) {
	/* try to create the file. exit if failure */
	if ((archivefile=fopen(fname,"wb"))==NULL) {
		return ARCHIVE_OPENERR;
	}

	/* create a header */
	memset(&header,0,sizeof(header));

	header.idcode=ARCHIVE_MAGICNO;
	header.version=ARCHIVE_VERSION;
	header.nofiles=0;
	header.entrypos=sizeof(struct ARCHIVEHEADER);

	/* write the header */
	if (!fwrite(&header,sizeof(struct ARCHIVEHEADER),1,archivefile)) return ARCHIVE_WRITERR;

	/* close the file */
	fclose(archivefile);

	/* let initreadwrite handle it */
	return (initreadwrite(fname));
}

void
ARCHIVE::done() {
	fclose(archivefile);
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
ARCHIVE::addfile(char *filename,FILE *f) {
    void *buf;
    _ULONG done,fsize,archivelen,newpos;
    _UINT bsize;
    struct ARCHIVEFILE filentry;
    char* ptr;
    char* tmp;
    char* realname;
    uLongf destlen;

    /* get size of file */
    fseek(f,0,SEEK_END);
    fsize=ftell(f);
    rewind(f);

    /* first seek to the end of the file and store the position */
    fseek(archivefile,0,SEEK_END);
    archivelen=ftell(archivefile);

    printf("compressing file '%s' (%lu bytes)",filename,fsize);

    /* wipe and fill in the file entry buffer */
    memset(&filentry,0,sizeof(filentry));
    realname=strrchr(filename,'/');
    if (realname != NULL)
	realname++;
    else
	realname = filename;
    strcpy(filentry.filename,realname);
    filentry.offset=header.entrypos;
    filentry.real_size=fsize;

    if ((buf=malloc(header.nofiles*sizeof(struct ARCHIVEFILE)))==NULL) {
   	return ARCHIVE_OUTOFMEMORY;
    }

    /* move to entry position */
    fseek(archivefile,header.entrypos,SEEK_SET);

    /* do we need to save everthing? */
    if (header.nofiles) {
   	/* yes. put it in the buffer */
 	if (!fread(buf,(header.nofiles*sizeof(struct ARCHIVEFILE)),1,archivefile)) {
   	    return ARCHIVE_READERR;
	}
    }

    /* allocate memory for the destination file */
    if((ptr=(char*)malloc(fsize))==NULL) {
        /* this failed. die */
        return ARCHIVE_OUTOFMEMORY;
    }

    /* read the file */
    if(!fread(ptr,fsize,1,f)) {
        /* this failed. die */
        return ARCHIVE_READERR;
    }

    /* close the file */
    fclose(f);

    /* allocate memory for the compression buffer */
    destlen=(fsize*2);
    if((tmp=(char*)malloc(destlen))==NULL) {
        /* this failed. die */
        return ARCHIVE_OUTOFMEMORY;
    }

    /* compress the buffer */
    if (compress2((Bytef *)tmp,&destlen,(Bytef *)ptr,fsize,ARCHIVE_COMPRESSLEVEL)!=Z_OK) {
        /* this failed. die */
        return ARCHIVE_COMPRESSERR;
    }

    printf(" compressed to %lu bytes. adding...",destlen);

    /* move to entry position */
    fseek(archivefile,header.entrypos,SEEK_SET);

    /* add the new data (plus overwrite) */
    if(!fwrite(tmp,destlen,1,archivefile)) {
        /* this failed. die */
        return ARCHIVE_WRITERR;
    }

    printf (" success\n");

    /* get the new archive directory offset */
    newpos=ftell(archivefile);

    /* write new entries */
    if (header.nofiles) {
        if (!fwrite(buf,header.nofiles*sizeof(struct ARCHIVEFILE),1,archivefile)) {
            return ARCHIVE_READERR;
        }
    }

    /* add this one */
    filentry.size=destlen;
    if (!fwrite(&filentry,sizeof(struct ARCHIVEFILE),1,archivefile)) {
	return ARCHIVE_READERR;
    }

    header.nofiles++;
    header.entrypos=newpos;

    rewind(archivefile);

    if (!fwrite(&header,sizeof(struct ARCHIVEHEADER),1,archivefile)) {
 	return ARCHIVE_WRITERR;
    }

    free(buf); free(tmp); free(ptr);
    return ARCHIVE_OK;
}

_UCHAR
ARCHIVE::getfile(char *filename,FILE *f) {
    _UINT bsize;
    _UCHAR result;
    _ULONG fsize,done;
    uLongf real_size;
    char*  buf;
    char*  tmp;
    struct ARCHIVEFILE fe;

    /* seek to first entry */
    if ((result=findfirst(&fe))!=ARCHIVE_OK) return result;

    /* check where file is. if any error, say it is not in file */
    while (strcmp(filename,fe.filename)) {
        if (findnext(&fe)!=ARCHIVE_OK) return ARCHIVE_NOTINARCHIVE;
    }

    fsize=fe.size;
    real_size=fe.real_size;

    /* allocate memory for the data */
    if((tmp=(char*)malloc(fsize))==NULL) {
        /* this failed. die */
        return ARCHIVE_OUTOFMEMORY;
    }

    /* seek to the correct offset */
    fseek(archivefile,fe.offset,SEEK_SET);

    /* read this data */
    if(!fread(tmp,fsize,1,archivefile)) {
        /* this failed. die */
        return ARCHIVE_READERR;
    }

    /* get data for the uncompressed data */
    if((buf=(char*)malloc(real_size))==NULL) {
        /* this failed. die */
        return ARCHIVE_OUTOFMEMORY;
    }

    /* uncompress the data */
    if(uncompress((Bytef*)buf,&real_size,(Bytef*)tmp,fsize)!=Z_OK) {
        /* this failed. die */
        return ARCHIVE_COMPRESSERR;
    }

    /* write this to the destination file */
    if(!fwrite(buf,real_size,1,f)) {
        /* this failed. die */
        return ARCHIVE_WRITERR;
    }

    /* free the memory used */
    free(tmp);free(buf);

    return ARCHIVE_OK;
}

_UCHAR
ARCHIVE::findfirst(struct ARCHIVEFILE *a) {
	if (!header.nofiles) {
		return ARCHIVE_NOFILES;
	}

	fseek(archivefile,header.entrypos,SEEK_SET);
	if (!fread(a,sizeof(struct ARCHIVEFILE),1,archivefile)) {
		return ARCHIVE_READERR;
	}
	return ARCHIVE_OK;
}

_UCHAR
ARCHIVE::findnext(struct ARCHIVEFILE *a) {
	if (!fread(a,sizeof(struct ARCHIVEFILE),1,archivefile)) {
		return ARCHIVE_READERR;
	}
	return ARCHIVE_OK;
}

_UCHAR
ARCHIVE::findfile(char *fname,struct ARCHIVEFILE *fe) {
	_UCHAR result;

	/* seek to first entry */
	if ((result=findfirst(fe))!=ARCHIVE_OK) return result;

	/* check where file is. if any error, say it is not in file */
	while (strcmp(fname,fe->filename)) {
		if (findnext(fe)!=ARCHIVE_OK) return ARCHIVE_NOTINARCHIVE;
	}

	return ARCHIVE_OK;
}

char
*ARCHIVE::err2msg(_UCHAR errno) {
	switch (errno) {
			   case ARCHIVE_OK: return "everything is ok";
		  case ARCHIVE_OPENERR: return "unable to open archive file";
		  case ARCHIVE_READERR: return "read error";
		  case ARCHIVE_WRITERR: return "write error";
	 case ARCHIVE_NOMADARCHIVE: return "file is not a mad archive";
	 case ARCHIVE_WRONGVERSION: return "this is a wrong version";
	  case ARCHIVE_OUTOFMEMORY: return "out of memory";
		  case ARCHIVE_NOFILES: return "no files in archive";
	 case ARCHIVE_NOTINARCHIVE: return "file is not in archive";
		 case ARCHIVE_CREATERR: return "create error";
	     case ARCHIVE_FILETOOSMALL: return "file is too small";
             case ARCHIVE_COMPRESSERR: return "compression failure";
	}
	return "unknown error. help!";
}
