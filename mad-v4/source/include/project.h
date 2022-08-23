/* are we already included? */
#ifndef __PRJ_INCLUDED__
/* no, set flag we are, and define things */
#define __PRJ_INCLUDED__
#include <stdio.h>
#include "types.h"

#define PROJECT_OK				0			/* its ok */
#define PROJECT_OPENERR			1			/* unable to open/create file */
#define PROJECT_READERR			2			/* read error */
#define PROJECT_WRITERR			3			/* write error */
#define PROJECT_NOMADPROJECT	4			/* file is not a mad project file */
#define PROJECT_WRONGVERSION	5			/* project is wrong version */
#define PROJECT_CREATERR		6			/* unable to create project file */

#define PROJECT_FLAG_NOBAR    1        /* don't load a bar */
#define PROJECT_FLAG_NOINV    2        /* don't load inventory file */
#define PROJECT_FLAG_NODLG    4        /* don't load dialog stuff */
#define PROJECT_FLAG_NOPARSER 8        /* don't use parser */

struct PROJECTINFO {
	_ULONG  idcode;			            /* id code (should be MAGICVALUE) */
	_UINT	  versiono;	                	/* version number (100 = 1.0, 201 = 2.01, etc.) */

   char    scriptfile[12];             /* name of script file to load */
   char    scriptproc[32];             /* name of script procedure to start */
   char    palettefile[12];            /* palette file */
   char    sysfont[12];                /* system font file */
   char    textfile[12];               /* text file */
   _ULONG  flags;                      /* flags */
   char    projectid[8];               /* project id */
   char    invfile[12];                /* inventory file */
   char    backdrop[12];               /* dialog backdrop */
   char    border[12];                 /* dialog border filenames */
   char    sysmouse[12];               /* system mouse file */
   _UCHAR  bpp;                        /* bytes(!) per pixel */
};

#define PROJECT_VERSION "2.0"
#define PROJECT_VERSIONO 200
#define PROJECT_MAGICVALUE 0x4a52504d

class
PROJECT {
public:
	_UCHAR	init(char *fname);
	#ifdef PRJEDIT
	_UCHAR	initreadwrite(char *fname);
	_UCHAR	create(char *fname);
	#endif

	void	done();

	char	*err2msg(_UCHAR err);

	struct	PROJECTINFO info;
private:
	FILE	*projectfile;
};

#endif
