#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "archive.h"
#include "mad.h"
#include "types.h"

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
	char chunk[ARCHIVE_CHUNKSIZE];
	_ULONG done,fsize,archivelen,newpos;
	_UINT bsize;
	struct ARCHIVEFILE filentry;
	char* destname;

	/* get size of file */
	fseek(f,0,SEEK_END);
	fsize=ftell(f);
	rewind(f);

	if (fsize<sizeof(struct ARCHIVEFILE)) {
		return ARCHIVE_FILETOOSMALL;
	}

	/* first seek to the end of the file and store the position */
	fseek(archivefile,0,SEEK_END);
	archivelen=ftell(archivefile);

	/* wipe and fill in the file entry buffer */
	memset(&filentry,0,sizeof(filentry));
	destname = strrchr(filename,'/');
	if (destname != NULL)
		destname++;
	else
		destname = filename;
	strcpy(filentry.filename,destname);
	filentry.offset=header.entrypos;
	filentry.size=fsize;

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

	printf("adding %s, 0/%lu bytes\r",filename,fsize);
        fflush(stdout);
	done=0;

	/* overwrite the entry data with the file data */
	fseek(archivefile,header.entrypos,SEEK_SET);
	while (fsize!=done) {
		if ((fsize-done)>ARCHIVE_CHUNKSIZE) { bsize=ARCHIVE_CHUNKSIZE; } else { bsize=fsize-done; }
		if (!fread(chunk,bsize,1,f)) return ARCHIVE_READERR;
		if (!fwrite(chunk,bsize,1,archivefile)) return ARCHIVE_WRITERR;
		done+=bsize;
		printf("adding %s, %lu/%lu bytes\r",filename,done,fsize);
        fflush(stdout);
        }
	newpos=ftell(archivefile);

	/* write new entries */
	if (header.nofiles) {
		/* FOR DEBUGGING
		   printf("name: %s pos %lu size %lu\n",((struct ARCHIVEFILE *)buf)->filename,((struct ARCHIVEFILE *)buf)->offset,((struct ARCHIVEFILE *)buf)->size);
		*/
		if (!fwrite(buf,header.nofiles*sizeof(struct ARCHIVEFILE),1,archivefile)) {
			return ARCHIVE_READERR;
		}
	}

	if (!fwrite(&filentry,sizeof(struct ARCHIVEFILE),1,archivefile)) {
		return ARCHIVE_READERR;
	}

	header.nofiles++;
	header.entrypos=header.entrypos+fsize;

	rewind(archivefile);

	if (!fwrite(&header,sizeof(struct ARCHIVEHEADER),1,archivefile)) {
		return ARCHIVE_WRITERR;
	}

	printf("file '%s' was added correctly.           \n",filename);

	free(buf);
	return ARCHIVE_OK;
}

_UCHAR
ARCHIVE::getfile(char *filename,FILE *f) {
	char chunk[ARCHIVE_CHUNKSIZE];
	_UINT bsize;
	_UCHAR result;
	_ULONG fsize,done;
	struct ARCHIVEFILE fe;

	/* seek to first entry */
	if ((result=findfirst(&fe))!=ARCHIVE_OK) return result;

	/* check where file is. if any error, say it is not in file */
	while (strcmp(filename,fe.filename)) {
		if (findnext(&fe)!=ARCHIVE_OK) return ARCHIVE_NOTINARCHIVE;
	}

	fsize=fe.size;

	printf("retrieving %s, 0/%lu bytes\r",filename,fsize);
	done=0;

	/* seek to the position of the file data and keep reading */
	fseek(archivefile,fe.offset,SEEK_SET);
	while (fsize!=done) {
		if ((fsize-done)>ARCHIVE_CHUNKSIZE) { bsize=ARCHIVE_CHUNKSIZE; } else { bsize=fsize-done; }
		if (!fread(chunk,bsize,1,archivefile)) return ARCHIVE_READERR;
		if (!fwrite(chunk,bsize,1,f)) return ARCHIVE_WRITERR;
		done+=bsize;
		printf("retrieving %s, %lu/%lu bytes\r",filename,done,fsize);
	}
	printf("file '%s' was retrieved correctly.           \n",filename);

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
	}
	return "unknown error. help!";
}
