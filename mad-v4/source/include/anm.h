#ifndef __ANM_DEFINED__
#define __ANM_DEFINED__

#include <stdio.h>
#include "mad.h"
#include "types.h"

#define ANM_MAGICNO				0x214d4e41 		/* ANM! */
#define ANM_VERSION				400				/* means 3.0 */
#define ANM_MAX_SPRITES 	512              /* maximum number of sprites loadable */
#define ANM_MAX_FRAMES          64              /* maximum number of frames in one animation */
#define ANM_MAX_ANIMS                   64              /* maximum number of animations in one file */

#define ANM_MAXANMLEN			16              /* maximum animation name length */
#define ANM_MAX_FILE_NAME_LEN           32              /* maximum size of my filename */

#define ANM_FLAG_PAL                    1               /* file doesn't include palette */

#define ANM_OK					0				/* its ok */
#define	ANM_OPENERR				1				/* unable to open/create file */
#define ANM_READERR				2				/* read error */
#define ANM_WRITERR				3				/* write error */
#define ANM_NOANMFILE    		4				/* file is not a mad archive */
#define ANM_WRONGVERSION		5				/* archive is wrong version */
#define ANM_OUTOFMEMORY			6				/* out of memory */
#define ANM_CREATERR		    7				/* create error */
#define ANM_TOOMANYSPRITES          8                   /* too many sprites in this file */
#define ANM_TOOMANYFRAMES           9                   /* too many frames in this file */

struct ANMHEADER {
	_ULONG	idcode;				/* id code */
	_UINT	version;        	/* version number (100 = 1.0, 201 = 2.01, etc.) */
	_ULONG	nofsprites;			/* number of files in library */
	_ULONG	nofanims;			/* number of animations in library */
        _ULONG  flags;                          /* flags */
	_UCHAR	reserved[46];	 	/* unused */
};

struct ANIMATION {
	char	name[ANM_MAXANMLEN];	/* name of animation */
	_UCHAR	noframes;				/* number of frames */
	_UCHAR	flags;					/* flags */
	_UCHAR	reserved[12];			/* unused */
};

struct ANMFRAME {
	struct	ANIMATION anm;			/* animation info */
	_ULONG 	frame[ANM_MAX_FRAMES];  /* frame number */
};

struct ANMSPRITEHEADER {
    _UINT heigth,width;                          /* dimensions */
    _UCHAR transcol_r,transcol_g,transcol_b;     /* transparant values */
    _UCHAR reserved[25];                         /* unused */
};

struct ANMSPRITEDATA {
    struct ANMSPRITEHEADER* hdr;
    char    *data;                  /* sprite data */
};

class ANM {
public:
    ANM();

    void    done();
    char    *err2msg(_UCHAR err);

    _UCHAR  init(char *name);

    _UCHAR  loaded;
    char    filename[ANM_MAX_FILE_NAME_LEN];

    struct  ANMSPRITEDATA sprite[ANM_MAX_SPRITES];
    struct  ANMFRAME anim[ANM_MAX_ANIMS];

    struct  ANMHEADER* header;
private:
    char*   data;
};
#endif
