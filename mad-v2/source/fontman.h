/* are we already included ? */
#ifndef __FONT_INCLUDED__
/* no. Set flag we are included and define things */
#define __FONT_INCLUDED__

#include "types.h"

#define FONTMAN_MONO         0              /* type of a mono font */
#define FONTMAN_COLOR        1              /* type of a color font */

#define FONTMAN_VERSION      "2.0"          /* font manager version */
#define FONTMAN_VERSIONNO    0x200          /* font manager version */
#define FONTMAN_MAGICID      0x544e464d     /* font file magic number */

#define FONTMAN_MAXFONTS     8              /* maximum fonts that can be loaded */
#define FONTMAN_NOFCHARS     256            /* number of chars in font */
#define FONTMAN_UNKNOWNCHAR  0              /* char printed if char is unknown */
#define FONTMAN_MAXFILENAMELEN  16          /* max font filename length */

#define FONTMAN_SYSFONTFILENAME "<system>"  /* filename of system font */
#define FONTMAN_NOFONT       0xffff

struct FONTHEADER {
    _ULONG  magic;          /* magic value, should be MAGICID */
    _UINT   version;	       /* version number, 100 for 1.00, 201 for 2.01 etc. */
    _UCHAR  height_width;   /* font height and width */
    _UCHAR  reserved[57];   /* currently unused */

};

struct FONT {
    _UCHAR   inuse;              /* in use flag */
    struct   FONTHEADER *header; /* font header */

    _ULONG   datasize;           /* size of font data */
    char*    chardata;           /* data of the chars */
    char*    charsize;           /* size of the chars */

    char     filename[FONTMAN_MAXFILENAMELEN]; /* font filename */
};

class FONTMAN {
public:
       FONTMAN();
       void   init(char*);
       void   done();

       void   unloadfont(_UINT);
       void   draw(void*);

       _UINT  loadfont(char*);
       _UINT  getsysfontno();

       _UINT  getfontwidth(_UINT);
       _UINT  getfontheight(_UINT);
       _UINT  getextwidth(_UINT,char*);
       _UINT  getextheight(_UINT,char*);
       _UINT  getcharwidth(_UINT,_UCHAR);
       _UINT  getcharheight(_UINT,_UCHAR);

       void   textout(_UINT,_UCHAR,char *text,_SINT,_SINT,char*);

private:
       _UCHAR charout(char,_SINT,_SINT,char*);
       _UCHAR charexists(_UINT,_UCHAR);

       void   charout(_UINT,_UCHAR,_UCHAR,_SINT,_SINT,char*);

       _UINT  searchfreentry();
       _UINT  searchfont(char*);

       struct FONT font[FONTMAN_MAXFONTS];

       _UINT  sysfontno;
};
#endif  /* __FONT_INCLUDED__ */
