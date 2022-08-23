#ifndef __ARCHIVE_DEFINED__
#define __ARCHIVE_DEFINED__

#include <stdio.h>
#include "mad.h"
#include "types.h"

#define ARCHIVE_MAGICNO			0x2144414d		/* MAD! */
#define ARCHIVE_VERSION			200			/* means 2.0 */

#define ARCHIVE_OK			0			/* its ok */
#define	ARCHIVE_OPENERR			1			/* unable to open/create file */
#define ARCHIVE_READERR			2			/* read error */
#define ARCHIVE_WRITERR			3			/* write error */
#define ARCHIVE_NOMADARCHIVE		4			/* file is not a mad archive */
#define ARCHIVE_WRONGVERSION		5			/* archive is wrong version */
#define ARCHIVE_OUTOFMEMORY		6			/* out of memory */
#define ARCHIVE_NOFILES			7			/* no files in archive */
#define ARCHIVE_NOTINARCHIVE		8			/* file is not in archive */
#define ARCHIVE_CREATERR		9			/* create error */
#define ARCHIVE_FILETOOSMALL		0xa			/* file is too small */
#define ARCHIVE_CHUNKSIZE               2048                    /* size of chunks to read/write in */
#define ARCHIVE_MAXFILENAMELEN          14                      /* maximum size of file name, including NUL */

struct ARCHIVEHEADER {
	_ULONG	idcode;			/* id code */
	_UINT	version;        	/* version number (100 = 1.0, 201 = 2.01, etc.) */
	_UINT 	nofiles;		/* number of files in mad archive */
	_ULONG	entrypos;		/* position of file entry data */
	_UCHAR	reserved[18];		/* unused */
};

struct ARCHIVEFILE {
        char    filename[ARCHIVE_MAXFILENAMELEN];       /* file name */
        _ULONG   offset;                 /* offset in mad archive */
	_ULONG	size;			/* size */
	_UCHAR	reserved[10];		/* unused */
};

class ARCHIVE {
public:
	_UCHAR init(char *fname);

	void  done();
        char  *err2msg(_UCHAR errno);

	#ifdef ARCHIVE_MFILE
	_UCHAR addfile(char *filename,FILE *f);
	_UCHAR getfile(char *filename,FILE *f);

	_UCHAR initreadwrite(char *fname);
	_UCHAR create(char *fname);
	#endif

	_UINT  getnofiles();
	_UINT  getversion();
	_ULONG getentrypos();

	_UCHAR findfirst(struct ARCHIVEFILE *a);
	_UCHAR findnext(struct ARCHIVEFILE *a);
	_UCHAR findfile(char *fname,struct ARCHIVEFILE *fe);
	void  *readfile(char *fname,_ULONG *rsize);

private:
	FILE *archivefile;

	struct ARCHIVEHEADER header;
};

#endif
