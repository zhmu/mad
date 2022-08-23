#include <alloc.h>
#include <dos.h>
#include <mem.h>
#include <stdio.h>
#include <stdlib.h>
#include "mad.h"
#include "mm.h"
#include "types.h"

void far *xms_entry;

MM::MM() {
	ULONG i;

	/* wipe all blocks empty */
	for(i=0;i<MM_NOFBLOCKS;i++) {
		mmblock[i].location=MM_UNUSED;
	}
}

void
MM::init() {
	ULONG i;

	/* check for a xms memory manager */
	asm {
		mov	ax,0x4300						/* xms v2+: installation check */
		int	0x2f
	}
	if (_AL==0x80) {
		/* xms memory manager is installed. now ask it for its entry point */
		asm {
			mov ax,0x4310					/* xms v2+: get entry point */
			int	0x2f
		}
		xms_entry=MK_FP(_ES,_BX);
		/* lets ask the xms manager which version it is */
		asm {
			xor	 ah,ah						/* xms v2+: get xms version number */
			call [xms_entry]
		}
		xms_version=_AX;
		xms_available=1;
		xms_memoryused=0;

		/* now we allocate 'MM_MINIMUM_XMS_AMOUNT' kb of xms memory */
		if (!(xms_handle=xms_getmem(MM_MINIMUM_XMS_AMOUNT))) {
			/* unable to allocate xms memory, so disable it */
			xms_available=0;
		} else {
			xms_handlesize=(MM_MINIMUM_XMS_AMOUNT*1024L);
		}
	}
	/* now we have to allocate a page buffer */
	if ((pagebuf=(char *)malloc(MM_PAGEBUFSIZE))==NULL) {
		printf("failed, unable to allocate page buffer\n");
		exit(1);
	}
}

UCHAR
MM::getxmsversion() {
	return xms_version;
}

UINT
MM::xms_getmem(UINT size) { asm {
	mov	 ah,9									/* xms v2+: allocate extended memory block */
	mov	 dx,size
	call dword ptr [xms_entry]
	}
	if (_AX!=1) return 0;
	return _DX;
}

UCHAR
MM::xms_free(UINT handle) { asm {
	mov	 ah,0xa								/* xms v2+: free extended memory block */
	mov	 dx,handle
	call dword ptr [xms_entry]
	}
	return _AL;
}

ULONG
MM::xms_getmemavail() { asm {
	mov	 ah,8									/* xms v2+: query free extended memory */
	call dword ptr [xms_entry]
	}
	return (ULONG)_AX;
}

ULONG
MM::getmemavail() {
	ULONG l;

	/* at least some base memory is available */
	l=(coreleft()/1024L);

	/* check whether xms is available */
	if (xms_available) {
		/* yes, so add xms memory amount to result */
		l+=xms_getmemavail();
		/* and add the size of the xms memory already allocated */
		l+=(ULONG)(xms_handlesize/1024L);
		/* and subtrace the size of the xms memory used */
		l-=(ULONG)xms_memoryused;
	}
	return l;
}

UCHAR
MM::xms_movememory(struct XMSMOVE far *xmove) { asm {
	mov	 	ah,0xb							/* xms v2+: move memory */

	push	ds									/* save ds and es */
	push	es

	push	ds
	pop		es									/* es = ds */

	lds		si,[xmove]
	call	dword ptr es:[xms_entry]

	pop		es									/* restore es and ds */
	pop		ds
	}
	return _AL;
}

UCHAR
MM::xms_resizehandle(UINT handle,UINT size) { asm {
	mov		ah,0fh				/* xms v2+: reallocte extended memory block */
	mov		bx,size
	mov		dx,handle
	call	dword ptr [xms_entry]
	}
	return _BL;
}

ULONG
MM::getfreeblock() {
	ULONG i;

	for(i=0;i<MM_NOFBLOCKS;i++) {
		if (mmblock[i].location==MM_UNUSED) return i;
	}
	return MM_NOBLOCK;
}

/* NOTE: MM::getmem EXPECTS size TO BE IN BYTES !!! */
ULONG
MM::getmem(ULONG size) {
	ULONG no;
	UCHAR usexms;

	if ((no=getfreeblock())==MM_NOBLOCK) {
		shutdown();
		printf("mm_getmem(%lu): out of memory blocks\n",size);
		exit(1);
	}

	/* is there xms memory? if so, use it */
	if (xms_available) {
		/* say the next routines to use the xms */
		usexms=1;
		/* is our current block large enough to handle the extra bytes? */
		if ((xms_memoryused+size)>xms_handlesize) {
			/* no, we'd better resize it, but is that allowed? */
			if ((xms_memoryused+size)<(MM_MAXIMUM_XMS_AMOUNT*1024L)) {
				/* it's allowed. let's resize it */
				if (!xms_resizehandle(xms_handle,(size/1024)+1+xms_handlesize)) {
					shutdown();
					printf("unable to resize xms memory\n");
					exit(1);
				}
			} else {
				/* say the next routines not to use the xms, because our handle is too small */
				usexms=0;
			}
		}
		if (usexms) {
			/* change the xms pointers and fill in the block info */
			xms_handlesize+=(size/1024)+1;
			mmblock[no].location=MM_XMS;
			mmblock[no].address=xms_memoryused;
			mmblock[no].size=size;
			xms_memoryused+=size;
			return no;
		}
	}
	/* only memory type left is base memory, so let's use that */
	/* check if program tries to allocate more that 64kb */
	if (size>0xffffL) {
		shutdown();
		printf("mm_getmem(%lu): out of memory\n",size);
		exit(1);
	}
	mmblock[no].location=MM_BASE;
	if ((mmblock[no].address=(ULONG)malloc(size))==NULL) {
		shutdown();
		printf("mm_getmem(%lu): out of memory\n",size);
		exit(1);
	}
	mmblock[no].size=size;
	return no;
}

void
MM::debugdump() {
	/* this will dump the entire memory block table to a file
		 is only available if DEBUGMODE_AVAILABLE has been defined */
	#ifdef DEBUGMODE_AVAILABLE
	FILE *f;
	ULONG i;

	if ((f=fopen(DEBUGFILENAME,"at"))==NULL) {
		printf("unable to create %s\n",DEBUGFILENAME);
		exit(1);
	}

	fprintf(f,"-Memory Manager debugging report START----\n");
	for(i=0;i<MM_NOFBLOCKS;i++) {
		/* skip unused blocks */
		if (mmblock[i].location!=MM_UNUSED) {
			switch (mmblock[i].location) {
				case MM_BASE: fprintf (f,"Block %4lu: in BASE addres %p, size %lu\n",i,(void *)mmblock[i].address,mmblock[i].size); break;
				 case MM_XMS: fprintf (f,"Block %4lu: in XMS address %lu, size %lu\n",i,mmblock[i].address,mmblock[i].size);
			}
		}
	}
	fprintf(f,"-Memory Manager debugging report END----\n");
	#endif
}

#pragma warn -rvl
/* above #pragma tells the compiler not to display 'function should return a value'
	 warning, because the memory manager will quit if there is an error, and return
	 would be nonsense
*/
char
*MM::pagein(ULONG no,ULONG pos,UINT size) {
	struct XMSMOVE xmove;

	if (mmblock[no].location==MM_UNUSED) {
		shutdown();
		printf("mm_pagein(%lu,%u,%u): memory is not marked as used\n",no,pos,size);
		exit(1);
	}
	if (pos>mmblock[no].size) {
		shutdown();
		printf("mm_pagein(%lu,%u,%u): position to page in from is greater than memory block lengh\n",no,pos,size);
		exit(1);
	}
	if ((pos+(ULONG)size)>mmblock[no].size) {
		shutdown();
		printf("mm_pagein(%lu,%u,%u): tried to page in more bytes than memory block has\n",no,pos,size);
		exit(1);
	}
	switch (mmblock[no].location) {
		case MM_BASE:	memcpy(pagebuf,(void *)(mmblock[no].address+pos),size);
									return pagebuf;
		 case MM_XMS: xmove.size=size;
									/* xms move size must be even */
									if (xmove.size%2) xmove.size++;
									xmove.shandle=xms_handle;
									xmove.soffset=mmblock[no].address+pos;
									xmove.dhandle=0;
									xmove.doffset=(ULONG)&pagebuf;
									if (!xms_movememory(&xmove)) {
										shutdown();
										printf("mm_pagein(%lu,%u,%u): xms move returned error\n",no,pos,size,mmblock[no].location);
										exit(1);
									}
									return pagebuf;
	}
	shutdown();
	printf("mm_pagein(%lu,%u,%u): memory block corrupted, unknown type %u\n",no,pos,size,mmblock[no].location);
	exit(1);
}
#pragma warn +rvl
/* above #pragma turns on the 'function should return a value' warning message
	 see comments above
*/

void
MM::pageout(ULONG no,ULONG pos,UINT size,void *buf) {
	struct XMSMOVE xmove;

	if (mmblock[no].location==MM_UNUSED) {
		shutdown();
		printf("mm_pageout(%lu,%u,%u): memory is not marked as used\n",no,pos,size);
		exit(1);
	}
	if (pos>mmblock[no].size) {
		shutdown();
		printf("mm_pageout(%lu,%u,%u): position to page out to is greater than memory block lengh\n",no,pos,size);
		exit(1);
	}
	if ((pos+(ULONG)size)>mmblock[no].size) {
		shutdown();
		printf("mm_pageout(%lu,%u,%u): tried to page out more bytes than memory block has\n",no,pos,size);
		exit(1);
	}
	switch (mmblock[no].location) {
		case MM_BASE:	memcpy((void *)(mmblock[no].address+pos),buf,size);
									return;
		 case MM_XMS: xmove.size=size;
									/* xms move size must be even */
									if (xmove.size%2) xmove.size++;
									xmove.shandle=0;
									xmove.soffset=(ULONG)&buf;
									xmove.dhandle=xms_handle;
									xmove.doffset=mmblock[no].address+pos;
									if (!xms_movememory(&xmove)) {
										shutdown();
										printf("mm_pageout(%lu,%u,%u): xms move returned error\n",no,pos,size);
										exit(1);
									}
									return;
	}
	shutdown();
	printf("mm_pageout(%lu,%u,%u): memory block corrupted, unknown type %u\n",no,pos,size,mmblock[no].location);
	exit(1);
	return;
}

ULONG
MM::gethandlesize(ULONG no) {
	if (mmblock[no].location==MM_UNUSED) {
		shutdown();
		printf("mm_gethandlesize(%lu): tried to get the size of an unused handle\n",no);
		exit(1);
	}
	return mmblock[no].size;
}

void
MM::done() {
	ULONG i;
	/* free all the blocks that are in base memory */
	for(i=0;i<MM_NOFBLOCKS;i++) {
		if (mmblock[i].location==MM_BASE) {
			mmblock[i].location=MM_UNUSED;
			free((void *)mmblock[i].address);
		}
	}

	/* is xms memory used? if so, free the memory */
	if (xms_available) {
		/* set it free */
		xms_free(xms_handle);
	}
	/* free the page buffer */
	free(pagebuf);
}