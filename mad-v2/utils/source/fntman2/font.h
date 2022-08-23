/* are we already included ? */
#ifndef __FONT_INCLUDED__
/* no. Set flag we are included and define things */
#define __FONT_INCLUDED__

#include "../../../source/types.h"

struct FONTHEADER {
    _ULONG  magic;          /* magic value, should be MAGICID */
    _UINT   version;	    /* version number, 100 for 1.00, 201 for 2.01 etc. */
    _UCHAR  height_width;   /* font height and width */
    _UCHAR  reserved[57];   /* currently unused */
};

#define FONT_VERSION    "2.0"
#define FONT_VERSIONO   0x200
#define FONT_MAGICID    0x544e464d

#define FONT_ERROR_OK         0
#define FONT_ERROR_WRITERR    1
#define FONT_ERROR_READERR    2
#define FONT_ERROR_OPENERR    3
#define FONT_ERROR_VERSIONERR 4
#define FONT_ERROR_MAGICERR   5
#define FONT_ERROR_ALLOCERR   6
#define FONT_ERROR_CHARTOOBIG 7

#define FONT_NOFCHARS         256

#define x__FONT_DEBUG__

class FONT {
public:
       int   init(char*);
       int   create(char*);
       int   done();

       int   write();

       char* err2msg(int);

       struct FONTHEADER header;

       char  charsize[FONT_NOFCHARS];
       char* chardata[FONT_NOFCHARS];

       int   addchar(char*,_UCHAR,_UCHAR);

private:
       FILE  *fontfile;
};
#endif
