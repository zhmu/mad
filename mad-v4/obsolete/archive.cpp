#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "archive.h"
#include "mad.h"
#include "mm.h"
#include "types.h"

void
ARCHIVE::init(char *fname) {
	ULONG id;
	UCHAR result;
	UINT i;

	/* try to open file file. exit if impossible */
	if ((archivefile=fopen(fname,"rb"))==NULL) {
		shutdown();
		printf("unable to open archive file.\n");
		exit(1);
	}

	/* try to read the magic value indicating this is a mad archive */
	if (read(&id,4)) {
		shutdown();
		printf("unable to read archive file.\n");
		exit(1);
	}
	/* and check it for the magic value */
	if (id!=ARCHIVE_ID) {
		shutdown();
		printf("file is not a mad archive file.\n");
		exit(1);
	}
	/* also read the version number */
	if (read(&i,2)) {
		shutdown();
		printf("unable to read archive file.\n");
		exit(1);
	}
	/* and ensure the mad archive is the right version */
	if (i!=ARCHIVE_VERSION) {
		shutdown();
		printf("archive is a wrong version.\n");
		exit(1);
	}
}

UCHAR
ARCHIVE::findfirst(struct ARCHIVEFILE *entry) {
	UCHAR result;

	/* seek just after header */
	fseek(archivefile,6,SEEK_SET);

	/* read the first entry */
	if (read(&archiventry,sizeof(struct ARCHIVEFILE))) {
		shutdown();
		printf("unable to read archive file.\n");
		exit(1);
	}

	/* copy it to the parameter */
	memcpy(entry,&archiventry,sizeof(struct ARCHIVEFILE));

	/* and seek after the file data to the next entry */
	fseek(archivefile,ftell(archivefile)+archiventry.size,SEEK_SET);

	/* say it's ok */
	return 0;
}

UCHAR
ARCHIVE::findnext(struct ARCHIVEFILE *entry) {
	/* try to read an archive header */
	if (read(&archiventry,sizeof(struct ARCHIVEFILE))) {
		/* return 1 if it couldn't be read (we are at the eof) */
		return 1;
	}

	/* copy the data we read to the entry structure */
	memcpy(entry,&archiventry,sizeof(struct ARCHIVEFILE));

	/* and seek after the file data to the next entry */
	fseek(archivefile,ftell(archivefile)+archiventry.size,SEEK_SET);

	/* say it's ok */
	return 0;
}

void
ARCHIVE::done() {
	fclose(archivefile);
}

UCHAR
ARCHIVE::read(void *buf, unsigned int size) {
	/* this just reads some bytes to the buffer and returns 0 if ok or 1 if
		 any error */
	if (fread(buf,size,1,archivefile)!=1) {
		return 1;
	}
	return 0;
}

UCHAR
ARCHIVE::findfile(char *fname,struct ARCHIVEFILE *entry) {
	UCHAR result;

	/* first seek just after header */
	fseek(archivefile,6,SEEK_SET);

	/* and read the entry */
	result=read(&archiventry,sizeof(struct ARCHIVEFILE));

	while (!result) {
		/* is it the entry we need? */
		if (!strcmp(fname,archiventry.name)) {
			/* yes, so return its data and say its ok */
			memcpy(entry,&archiventry,sizeof(struct ARCHIVEFILE));
			return 0;
		}
		/* seek to next entry */
		fseek(archivefile,ftell(archivefile)+archiventry.size,SEEK_SET);
		result=read(&archiventry,sizeof(struct ARCHIVEFILE));
	}
	return 1;
}

ULONG
ARCHIVE::filesize(char *fname) {
	UCHAR result;
	struct ARCHIVEFILE entry;

	/* find the file */
	result=findfile(fname,&entry);
	if (result) return 0L;

	/* return its size */
	return entry.size;
}

ULONG
ARCHIVE::readfile(char *fname) {
	struct ARCHIVEFILE entry;
	ULONG no,size,run,pos;
	char chunk[ARCHIVE_CHUNKSIZE];

	/* find the file */
	if (findfile(fname,&entry)) {
		shutdown();
		printf("archive_readfile(%s): file not found in archive\n",fname);
		exit(1);
	}

	/* store the amount of bytes to go */
	size=entry.size;
	pos=0L;
	/* allocate memory for the data */
	no=mm->getmem(size);

	while(size) {
		if (size>ARCHIVE_CHUNKSIZE) {
			run=ARCHIVE_CHUNKSIZE;
		} else {
			run=size;
		}
		if (!run) break;
		if (read(chunk,run)) {
			shutdown();
			printf("archive_readfile(%s): read error\n");
			exit(1);
		}
		mm->pageout(no,pos,run,chunk);
		size-=run;
		pos+=run;
	}
	return no;
}