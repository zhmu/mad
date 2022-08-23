#include <stdio.h>
#include "types.h"

/* are we already included? */
#ifndef __ARCHIVE_INCLUDED__
/* no. set flag we are included, and define things */
#define __ARCHIVE_INCLUDED__

struct ARCHIVEFILE {
	UCHAR name[13];						/* name of the file */
	ULONG size;								/* file size */
};

class ARCHIVE {
public:
	void  init(char *fname);
	UCHAR read(void *buf, unsigned int size);
	UCHAR findfirst(struct ARCHIVEFILE *entry);
	UCHAR findnext(struct ARCHIVEFILE *entry);
	UCHAR findfile(char *fname,struct ARCHIVEFILE *entry);
	UCHAR readfile(char *fname,void *buf);
	ULONG filesize(char *fname);
	ULONG readfile(char *fname);

	void  done();

private:
	FILE *archivefile;
	struct ARCHIVEFILE archiventry;

	UINT		nofprocs;												/* number of procedures in the script */
	void		parseprocs();
};

char *archive_errmsg(unsigned char no);

#define ARCHIVE_ID				   0x2144414dL /* MAD! */
#define ARCHIVE_VERSION			         100 /* version number (1.00) */

/* NOTE: ARCHIVE_CHUNKSIZE MUST BE UNDER MM_PAGEBUFSIZE */
#define ARCHIVE_CHUNKSIZE						8192 /* size of chunks in which archive is read */

#endif