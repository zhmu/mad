#include "anm.h"
#include "archive.h"
#include "gfx_drv.h"
#include "script.h"
#include "types.h"

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
#define  GFX_PIC_VERSIONO  200          /* version number (2.0) */
#define  GFX_PIC_NOPIC     0xffffffff   /* no picture assigned */

struct SPRITEDATA {
        _UCHAR inuse;                   /* nonzero if used */
        char   filename[ARCHIVE_MAXFILENAMELEN];  /* file name */
        _UINT  height,width;            /* height and width of sprite */
        _UCHAR bpp;                     /* bytes(!) per pixel */
        char   *data;                   /* actual sprite data */
};

struct PICHEADER {
	_ULONG	idcode;		                /* id code */
	_UINT	version;                       /* version number (100 = 1.0, 201 = 2.01, etc.) */
   _UINT    picheight;                  /* picture height */
   _UINT    picwidth;                   /* picture width  */
   _ULONG   gfx_offset;                 /* background screen offset */
   _ULONG   pri_offset;                 /* priority screen offset */
   _ULONG   msk_offset;                 /* mask screen offset */
   _UCHAR   bpp;                        /* bytes(!) per pixel */
   _UCHAR   reserved[5];                /* unused */
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

        void restorepiece(_SINT,_SINT,_UINT,_UINT,char*,char*);

        _UINT linetable[GFX_DRIVER_VRES];

        char current_picture[ARCHIVE_MAXFILENAMELEN];

        #ifdef PROFILE_FPS
        float fps;
        #endif

private:
        _UCHAR oldmode,nofrows,bpp;

        _UINT getfreedatasprite();
        _UCHAR hasbaseline(_UINT,_UINT);

        void drawdata(_SINT,_SINT,_UINT,_UINT,char*,char*,char*);

        void drawpriorityline(_UINT,_SINT,_UCHAR);
        _UCHAR getpriority(_SINT x,_SINT y,_SINT x2,_SINT y2,_UCHAR wrap_coords);
        _UCHAR posok(_SINT,_SINT,_UINT);

        class ANM animation[GFX_MAXANIMS];
        struct SPRITEDATA spritedata[GFX_MAXSPRITES];

        char palette[786];
};
