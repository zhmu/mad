/*
                             PROJECT.CPP

                MAD Project Engine - Handles project information

                     (c) 1998, 1999 The MAD Crew

  Description:
  This provides some MAD information so that the main script file name and
  procedure is known.

  Todo: Nothing

  Portability notes: Fully portable, no changes required
*/
#include <stdlib.h>
#include <string.h>
#include "archive.h"
#include "maderror.h"
#include "mad.h"
#include "project.h"
#include "types.h"

char
*PROJECT::err2msg(_UCHAR errno) {
   switch (errno) {
             case PROJECT_OK: return PROJECT_ERROR_0;
        case PROJECT_OPENERR: return PROJECT_ERROR_1;
        case PROJECT_READERR: return PROJECT_ERROR_2;
        case PROJECT_WRITERR: return PROJECT_ERROR_3;
   case PROJECT_NOMADPROJECT: return PROJECT_ERROR_4;
   case PROJECT_WRONGVERSION: return PROJECT_ERROR_5;
       case PROJECT_CREATERR: return PROJECT_ERROR_6;
   }
   return MAD_ERROR_UNKNOWN;
}

_UCHAR
PROJECT::init(char *fname) {
	char *ptr;

	/* open the file and read the information. we dont care about the
	   size */

   ptr=(char *)archive->readfile(fname,NULL);
   memcpy(&info,ptr,sizeof(struct PROJECTINFO));

   free(ptr);

	/* is this a mad project file? */
	if(info.idcode!=PROJECT_MAGICVALUE) {
		/* no, so return PROJECT_NOMADPROJECT */
		return PROJECT_NOMADPROJECT;
	}
	/* is the version ok? */
	if (info.versiono!=PROJECT_VERSIONO) {
		/* no, so return PROJECT_WRONGVERSION */
		return PROJECT_WRONGVERSION;
	}
   /* if the number of bits per pixel is zero, tweak it to one */
   if(info.bpp==0) info.bpp=1;
	/* everything went ok, so return PROJECT_OK */
	return PROJECT_OK;
}

void
PROJECT::done() {
}
