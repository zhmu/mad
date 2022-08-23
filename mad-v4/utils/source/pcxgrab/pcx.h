#include "../../../source/types.h"

struct PCXHEADER {
    _UCHAR manufacturer;         /* must be 0xA, for ZSoft PCX */
    _UCHAR version;              /* version number */
    _UCHAR encoding;             /* must be 1 for RLE */
    _UCHAR bpp;                  /* must be 8 for 256 color */
    _UCHAR xmin,ymin,xmax,ymax;  /* picture dimensions */
    _UINT  hres;                 /* horizonal size for device */
    _UINT  vres;                 /* vertical size for device */
    _UCHAR colormap[48];         /* palette, ignored */
    _UCHAR reserved;             /* some reserved junk */
    _UINT  bytesperline;         /* bytes per line */
    _UINT  palinfo;              /* palette information */
    _UCHAR filler[62];           /* filler */
};
