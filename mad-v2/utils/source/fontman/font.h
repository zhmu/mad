
/* are we already included ? */
#ifndef __FONT_INCLUDED__
/* no. Set flag we are included and define things */
#define __FONT_INCLUDED__

#include "../../../source/types.h"

struct FONTHEADER {
    _ULONG  magic;          /* magic value, should be MAGICID */
	_UINT	version;	    /* version number, 100 for 1.00, 201 for 2.01 etc. */
	_UINT	height;		    /* font height */
	_UINT	width;		    /* font width */
    _UCHAR  type;           /* font type */
    _UCHAR  ocolor;         /* opaque color (for color fonts) */
	_UCHAR	entry[32];	    /* font entry describer */
	_UCHAR	reserved[24];	/* currently unused */
};

#define FONT_MONO       0
#define FONT_COLOR      1

#define FONT_VERSION    "1.0"
#define FONT_VERSIONNO  0x100
#define FONT_MAGICID    0x544e464d

#define FONT_DEFAULTCOLOR 12

#define FONT_OK         0
#define FONT_WRITEERR   1
#define FONT_READERR    2
#define FONT_OPENERR    3
#define FONT_VERSIONERR 4
#define FONT_MAGICERR   5
#define FONT_ALLOCERR   6
#define FONT_NOFONTDAT  7
#define FONT_SIZEERR    8


class FONT {
public:
       FONT();

       void init();
       void done();
       _UCHAR setchar(FILE*,_UCHAR,_UCHAR);
       _UCHAR deletechar(_UCHAR);
       _UCHAR getchr(FILE*,char,_UCHAR,_UCHAR);
       _UCHAR initreadwrite(char*);
       _UCHAR create(char*);
       _UCHAR save(char*);
       char* err2msg(_UINT);
       char charexists(_UCHAR);

       struct FONTHEADER *header;
       char *data[256];
       _ULONG datasize;
       _UCHAR color;
       _ULONG bitpos;
};

#endif  /* __FONT_INCLUDED__ */
