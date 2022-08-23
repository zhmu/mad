#include "types.h"

/* are we already included? */
#ifndef __MM_INCLUDED__
/* no. set flag we are included, and define things */
#define __MM_INCLUDED__

#define MM_MINIMUM_XMS_AMOUNT		128			/* minimum amount in kb of xms block when initialized */
#define MM_MAXIMUM_XMS_AMOUNT		8192		/* we never allocate more xms memory than this */
#define MM_PAGEBUFSIZE					0xffff	/* memory manager page buffer size */
#define MM_NOFBLOCKS					  64			/* number of memory blocks */

#define MM_UNUSED     					0			  /* undefined/free block */
#define MM_BASE                 1				/* base memory */
#define	MM_XMS									2				/* xms memory */

#define MM_NOBLOCK							0xffffffff	/* returned if no block */

struct XMSMOVE {
	ULONG	size;														/* number of bytes to move (must be even) */
	UINT	shandle;												/* source handle, 0 if in base memory */
	ULONG soffset;                        /* source offset, offset:segment if handle is 0 */
	UINT	dhandle;                        /* dest handle, 0 if in base memory */
	ULONG	doffset;                        /* dest offset, offset:segment if handle is 0 */
};

struct MMBLOCK {
	UCHAR	location;												/* where is it? */
	ULONG address;												/* address of the memory block */
	ULONG	size;														/* size of block in bytes */
};

class MM {
public:
	void  init();
	void  done();
	ULONG getmem(ULONG size);
	void  freemem(ULONG no);
	ULONG getmemavail();
	UCHAR getxmsversion();
	void	debugdump();
	char  *pagein(ULONG no,ULONG pos,UINT size);
	void	pageout(ULONG no,ULONG pos,UINT size,void *buf);
	ULONG gethandlesize(ULONG no);

	MM();

private:
	UCHAR xms_available;
	char  *pagebuf;

	/* xms stuff */
	UINT	xms_handle;
	ULONG	xms_handlesize;
	ULONG	xms_memoryused;

	UINT  xms_getmem(UINT size);
	UCHAR xms_free(UINT handle);
	ULONG xms_getmemavail();
        UCHAR xms_movememory(struct XMSMOVE far *xmove);

	UCHAR xms_resizehandle(UINT handle,UINT newsize);

	UCHAR xms_version;

	struct MMBLOCK mmblock[MM_NOFBLOCKS];
	ULONG getfreeblock();
};

extern void far *xms_entry;

#endif
