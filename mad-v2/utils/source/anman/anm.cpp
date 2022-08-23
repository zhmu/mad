#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "anm.h"

_UCHAR
ANM::createanm(char *name) {
	if((anm_file=fopen(name,"wb"))==NULL) return ANM_OPENERR;

	/* zero out the header */
	memset(&anm_header,0,sizeof(struct ANMHEADER));
	/* fill in the essential fields */
	anm_header.idcode=ANM_MAGICNO;
	anm_header.version=ANM_VERSION;
		/* write the header to the file */
	if (!fwrite(&anm_header,sizeof(struct ANMHEADER),1,anm_file)) {
		return ANM_WRITERR;
	}
	return ANM_OK;
}

void
ANM::done() {
	/* write the header to the disk */
	rewind(anm_file);
	fwrite(&anm_header,sizeof(struct ANMHEADER),1,anm_file);
	fclose(anm_file);
}

char
*ANM::err2msg(_UCHAR errno) {
	switch (errno) {
					   case ANM_OK: return "everything is ok";
				  case ANM_OPENERR: return "unable to open animation file";
				  case ANM_READERR: return "read error";
				  case ANM_WRITERR: return "write error";
				case ANM_NOANMFILE: return "file is not an anm file";
			 case ANM_WRONGVERSION: return "this is a wrong version";
			  case ANM_OUTOFMEMORY: return "out of memory";
				 case ANM_CREATERR: return "create error";
	}
	return "unknown error. help!";
}

_UCHAR
ANM::addsprite(char *name) {
	FILE *f;
	void *ptr;
	_ULONG size;

	/* try the open the file, return ANM_OPENERR if failure */
	if((f=fopen(name,"rb"))==NULL) return ANM_OPENERR;

	/* read the entire file to memory */
	fseek(f,0,SEEK_END);size=ftell(f);rewind(f);

	if((ptr=malloc(size))==NULL) {
		/* couldnt allocate memory, so return ANM_OUTOFMEMORY */
		return ANM_OUTOFMEMORY;
	}
	if(!fread(ptr,size,1,f)) {
		/* couldnt read the file, so return ANM_READERR */
		return ANM_READERR;
	}

	/* be a neat program and close the file */
	fclose(f);

	/* seek to the end of the anm file and add the sprite */
	fseek(anm_file,0,SEEK_END);
	if(!fwrite(ptr,size,1,anm_file)) {
		/* couldnt write the file, so return ANM_WRITERR */
		return ANM_WRITERR;
	}
	/* increment the number of sprites */
	anm_header.nofsprites++;

        free(ptr);

	/* yipee, all went ok, so return ANM_OK */
	return ANM_OK;
}

_UCHAR
ANM::addanm(char *name,char *frame,_UCHAR noframes,_UINT flags) {
	struct ANIMATION anm;

	/* fill in header */
	memset(&anm,0,sizeof(struct ANIMATION));
	strcpy(anm.name,name);
	anm.noframes=noframes;
	anm.flags=flags;

	/* seek to the end of the anm file and add the animation */
	fseek(anm_file,0,SEEK_END);
	/* write header */
	if(!fwrite(&anm,sizeof(struct ANIMATION),1,anm_file)) {
		/* couldnt write the file, so return ANM_WRITERR */
		return ANM_WRITERR;
	}
	/* write the frame table */
	if(!fwrite(frame,noframes,1,anm_file)) {
		/* couldnt write the file, so return ANM_WRITERR */
		return ANM_WRITERR;
	}
	/* increment the number of animations */
	anm_header.nofanims++;

	/* yipee, all went ok, so return ANM_OK */
	return ANM_OK;
}
