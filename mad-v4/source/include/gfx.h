#include "anm.h"
#include "archive.h"
#include "gfx_drv.h"
#include "script.h"
#include "types.h"

/* are we already included? */
#ifndef __GFX_INCLUDED__
/* no. set flag we are included, and define things */
#define __GFX_INCLUDED__

#define  GFX_BPP        4               /* BYTES per pixel */

#define  GFX_COLOR_CONSOLETEXT  0xffff00

#define  GFX_MAXCONSOLETEXT 4           /* maximum number of strings at the console */
#define  GFX_MAXSPRITES 63              /* maximum number of sprites loaded at one time */
#define  GFX_MAXANIMS   63              /* maximum number of animations */

#define  GFX_SCREENWIDTH 320            /* screen width */
#define  GFX_SCREENHEIGHT 200           /* screen height */

#define  GFX_NOSPRITE 0xffff            /* returned if no sprite is available */
#define  GFX_NOBJECT  0xffff            /* returned if no object is available */
#define  GFX_NOCOORD  0x7fff            /* object has no coordinate */
#define  GFX_NOANIM   0xff              /* returned if no animation is available */

#define  GFX_PRIORITY_UNREACHABLE   0   /* unreachable */
#define  GFX_PRIORITY_REACHABLE     15  /* reachable */
#define  GFX_MAX_PRIORITIES 3           /* maximum levels of priorities */

#define  GFX_ALWAYS_MASK            1   /* always mask this color */
#define  GFX_MASK_TOP      DIR_NORTH    /* mask from the top number */
#define  GFX_MASK_BOTTOM   DIR_SOUTH    /* mask from the bottom number */

#define  GFX_TYPE_UNUSED        0       /* unused object */
#define  GFX_TYPE_SPRITE        1       /* sprite object */
#define  GFX_TYPE_ANM           2       /* animatable object */

#define  GFX_PIC_MAGICNO   0x21434950   /* magic number (PIC!) */
#define  GFX_PIC_VERSIONO  300          /* version number (2.0) */
#define  GFX_PIC_NOPIC     0xffffffff   /* no picture assigned */

#define  GFX_SPR_MAGICNO   0x21525053   /* magic number (SPR!) */
#define  GFX_SPR_VERSIONO  300          /* version number (3.0) */

struct SPRITEHEADER {
   _ULONG   idcode;                     /* id code */
   _UINT    version;                    /* version number (100 = 1.0, 210 = 2.1 etc) */
   _UINT    height,width;               /* height and width */
   _ULONG   flags;                      /* flags */
   _UCHAR   transcol_r;                 /* transperancy red value */
   _UCHAR   transcol_g;                 /* transperancy green value */
   _UCHAR   transcol_b;                 /* transperancy blue value */
   _UCHAR   reserved[10];               /* unused */
};

struct SPRITEDATA {
   struct SPRITEHEADER* hdr;           /* sprite header */
   char   filename[ARCHIVE_MAXFILENAMELEN];  /* file name */
   char*  data;                   /* actual sprite data */
   char*  buf;                     /* actual data buffer */
};

struct PICHEADER {
   _ULONG   idcode;		        /* id code */
   _UINT    version;                    /* version number (100 = 1.0, 201 = 2.01, etc.) */
   _UINT    picheight;                  /* picture height */
   _UINT    picwidth;                   /* picture width  */
   _ULONG   gfx_offset;                 /* background screen offset */
   _ULONG   pri_offset;                 /* priority screen offset */
   _ULONG   msk_offset;                 /* mask screen offset */
   _UCHAR   reserved[5];                /* unused */
};

struct RGB {
   _UCHAR   r;
   _UCHAR   g;
   _UCHAR   b;
};

class GFX {
public:
        void init();
        void done();
        void redraw(_UCHAR);
        void loadscreen(char *fname);

        void loadpalette(char *fname);
        void darkenpalette(_UCHAR step);
        void restorepalette();

        _UINT loadsprite(char *fname);
        _UINT findatasprite(char *name);

        _UINT findanm(char *name);
        _UINT loadanm(char *fname);
        _UINT findfreeanm();
        _UINT findanmsequence(_UINT animno,char *name);

        _UCHAR objectscollide(_UINT spr1,_UINT spr2);
        _UCHAR objectcollideswith(_UINT,_UINT,_UINT,_UINT,_UINT);

        void putsprite(_SINT,_SINT,_UINT,char*);
        void unloadsprite(_UINT no);
        void unloadanimation(_UINT no);

        _UCHAR animationhitmask(_UINT objectno);
        void drawanimationmask(_UINT objectno);

        _UINT getspriteheight(_UINT no);
        _UINT getspritewidth(_UINT no);
        char  *getspritedata(_UINT no);

        _UINT getcuranmsprite(_UINT objectno);
        _UINT getnoframes(_UINT objectno);

        _UINT getanmheight(_UINT objectno);
        _UINT getanmwidth(_UINT objectno);

        _UINT sethaltsequence(_UINT objectno,char *seqname);
        _UINT setmovesequence(_UINT objectno,char *seqname);

        void handlegomoves();

        void getgamescreen(char *dest);

        char  *virtualscreen;
        char  *priorityscreen;
        char  *maskscreen;
        char  *backscreen;

        void fadeout();
        void fadein();

        void  set_mouse_spriteno(_UINT);
        _UINT get_mouse_spriteno();

        void restorepiece(_SINT,_SINT,_UINT,_UINT,char*,char*);

        _ULONG linetable[GFX_DRIVER_VRES*GFX_BPP];

        char current_picture[ARCHIVE_MAXFILENAMELEN];

        char* get_main_pal();

        ANM* get_anim(_UINT);

        void drawdata(_SINT,_SINT,_UINT,_UINT,char*,char*,char*,char*);
        void drawrgbdata(_SINT,_SINT,_UINT,_UINT,_UCHAR,_UCHAR,_UCHAR,char*,char*,char*);

        void console_print(char*);

private:
        _UCHAR oldmode,nofrows,bpp;

        _UINT getfreedatasprite();
        _UCHAR hasbaseline(_UINT,_UINT);

        void drawpriorityline(_UINT,_SINT,_UCHAR);
        _UCHAR getpriority(_SINT x,_SINT y,_SINT x2,_SINT y2,_UCHAR wrap_coords);
        _UCHAR posok(_SINT,_SINT,_UINT);

        void getobjectsize(_UINT objno,_UINT*,_UINT*);

        class ANM animation[GFX_MAXANIMS];
        struct SPRITEDATA spritedata[GFX_MAXSPRITES];

        _UINT mouse_spriteno;

        char palette[768];

        char console_text[GFX_MAXCONSOLETEXT][MAD_TEMPSTR_SIZE];
};

#endif
