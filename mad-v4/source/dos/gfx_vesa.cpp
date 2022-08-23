/*
                            GFX_VESA.CPP

               VESA Graphix Engine for MAD - DOS version

                  (c) 1998, 1999, 2000 The MAD Crew

  Description:
  This will handle the actual low-level graphics functions.

  Todo: Nothing

  Portability notes: Needs to be fully rewritten for other platforms.
*/

#define  FARPTR

#include <dos.h>
#include <dpmi.h>
#include <go32.h>
#include <stdlib.h>
#include <string.h>
#include <sys/movedata.h>
#ifdef FARPTR
#include <sys/farptr.h>
#else
#include <sys/nearptr.h>
#endif
#include "archive.h"
#include "gfx.h"
#include "gfx_drv.h"
#include "mad.h"
#include "maderror.h"
#include "vbe.h"
#include "types.h"

#define   LINEAR_FRAME_FLAG       128
#define   USE_LINEAR_FRAME        16384
#define   _32BIT_SCANLINE         1280
#define   MODE_320x200_32         0x10F       /* 320x200x32 */
#define   MODE_320x200_24         0x10E       /* 320x200x24 */
#define   MODE_320x200_16         0x10D       /* 320x200x16 */

VESA_INFO vesa_info;
MODE_INFO mode_info;

_UCHAR *vesa_base;
_UCHAR *buffer;

__dpmi_meminfo mapping;
_ULONG selector;

#ifdef NEARPTR
char*        vesa_addr;
#endif

void
GFXDRIVER::init() {
	__dpmi_regs r;
    _ULONG dosbuf;
    _UINT  c;

    /* first of all, set the size of the linear buffer to zero, to ensure
       it's not being freed */
    buffer=NULL;

    memset(&r,0,sizeof(union REGS));
    r.x.ax=0x1130;              /* video bios: get font information */
    r.h.bh=0;
	__dpmi_int(0x10, &r);
    nofrows=r.h.dl;

    memset(&r,0,sizeof(union REGS));
    r.h.ah=0xf;                 /* video bios: get current video mode */
	__dpmi_int(0x10, &r);
    oldmode=r.h.al;

    /* retrieve VESA information */
    /* use the conventional memory transfer buffer */
    dosbuf = __tb & 0xFFFFF;

    /* initialize the buffer to zero */
    for (c=0; c<sizeof(VESA_INFO); c++) {
        _farpokeb(_dos_ds, dosbuf+c, 0);
    }

    /* use the magic VESA2 signature */
    dosmemput("VBE2", 4, dosbuf);

    memset(&r,0,sizeof(union REGS));

    /* call the VESA function */
    r.x.ax = 0x4F00;
    r.x.di = dosbuf & 0xF;
    r.x.es = (dosbuf>>4) & 0xFFFF;
    __dpmi_int(0x10, &r);

    /* was an error? */
    if (r.h.ah) {
        /* yes.die */
        die("Unable to get VESA version information");
    }

    /* copy the resulting data into our structure */
    dosmemget(dosbuf, sizeof(VESA_INFO), &vesa_info);

    /* check that we got the right magic marker value */
    if (strncmp((char *)vesa_info.VESASignature, "VESA", 4)) {
        /* this failed. die */
        die ("Error [???]: VESA returned unexpected information");
    }

    /* try to use 320x200x32bit */
    if (!set_mode(MODE_320x200_32)) {
        /* this failed. try 320x200x24bit */
        if(!set_mode(MODE_320x200_24)) {
            /* this failed. try 320x200x16 */
            if(!set_mode(MODE_320x200_16)) {
                /* this failed. die */
                die("Error [???]: Cannot find an appropriate VESA video mode to use");
            }
        }
    }
}

void
GFXDRIVER::done() {
    union REGS r;

    /* do we have a buffer? */
    if (buffer!=NULL) {
        /* yup. kill it */
        free(buffer);

        /* make sure we never do this twice */
        buffer=NULL;
    }

    #ifdef FARPTR
        __dpmi_free_ldt_descriptor(selector);
    #endif

    memset(&r,0,sizeof(union REGS));
    r.x.ax=oldmode;                 /* bios: set video mode */
    int86(0x10,&r,&r);

    if (nofrows!=0x18) {            /* was the user in 25 mode line? */
        /* no, so set the 8x8 font */
        memset(&r,0,sizeof(union REGS));
        r.x.ax=0x1112;           /* video bios: set 8x8 chars */
        r.h.bl=0;
        int86(0x10,&r,&r);
    }
}

void
GFXDRIVER::draw(void *data) {
    _ULONG i;
    char* ptr;
    _UCHAR* cdata = (_UCHAR*)data;
    _UINT ty, tx;
    _UINT color;

    /* what kind of mode to we have? */
    switch (mode_info.BitsPerPixel) {
        case 15: /* 15 bpp */
                 for(ty = 0; ty < mode_info.YResolution; ty++) {
                     for(tx = 0; tx < mode_info.XResolution; tx++) {
                         color = ((cdata[(ty*_32BIT_SCANLINE)+(tx<<2)  ] >> (8-mode_info.RedMaskSize)) << mode_info.RedMaskPos) +
                                 ((cdata[(ty*_32BIT_SCANLINE)+(tx<<2)+1] >> (8-mode_info.GreenMaskSize)) << mode_info.GreenMaskPos) +
                                 ((cdata[(ty*_32BIT_SCANLINE)+(tx<<2)+2] >> (8-mode_info.BlueMaskSize)) << mode_info.BlueMaskPos);

                         buffer[(ty*mode_info.BytesPerScanLine)+(tx<<1)  ] = color & 0x00FF;
                         buffer[(ty*mode_info.BytesPerScanLine)+(tx<<1)+1] = color >> 8;
                     }
                 }
                 break;
        case 16: /* 16 bpp */
                 for(ty = 0; ty < mode_info.YResolution; ty++) {
                     for(tx = 0; tx < mode_info.XResolution; tx++) {
                         color = ((cdata[(ty*_32BIT_SCANLINE)+(tx<<2)  ] >> (8-mode_info.RedMaskSize)) << mode_info.RedMaskPos) +
                                  ((cdata[(ty*_32BIT_SCANLINE)+(tx<<2)+1] >> (8-mode_info.GreenMaskSize)) << mode_info.GreenMaskPos) +
                                  ((cdata[(ty*_32BIT_SCANLINE)+(tx<<2)+2] >> (8-mode_info.BlueMaskSize)) << mode_info.BlueMaskPos);
                         buffer[(ty*mode_info.BytesPerScanLine)+(tx<<1)  ] = color & 0x00FF;
                         buffer[(ty*mode_info.BytesPerScanLine)+(tx<<1)+1] = color >> 8;
                     }
                 }
                 break;
        case 24: /* 24 bpp */
                 /* RGB or BGR? */
                 /* XXX! THIS DOESN'T WORK CORRECTLY!!! */
                 if(mode_info.BlueMaskPos) {
                     /* RGB */
                     for(ty = 0; ty < mode_info.YResolution; ty++) {
                         for(tx = 0; tx < mode_info.XResolution; tx++) {
                             buffer[(ty*mode_info.BytesPerScanLine)+(tx*3)] = cdata[(ty*_32BIT_SCANLINE)+(tx<<2)+2];
                             buffer[(ty*mode_info.BytesPerScanLine)+((tx*3)+1)] = cdata[(ty*_32BIT_SCANLINE)+(tx<<2)+1];
                             buffer[(ty*mode_info.BytesPerScanLine)+((tx*3)+2)] = cdata[(ty*_32BIT_SCANLINE)+(tx<<2)];
                         }
                     }
                 } else {
                     /* BGR */
                     for(ty = 0; ty < mode_info.YResolution; ty++) {
                         for(tx = 0; tx < mode_info.XResolution; tx++) {
                             buffer[(ty*mode_info.BytesPerScanLine)+(tx*3)] = cdata[(ty*_32BIT_SCANLINE)+(tx<<2)+2];
                             buffer[(ty*mode_info.BytesPerScanLine)+((tx*3)+1)] = cdata[(ty*_32BIT_SCANLINE)+(tx<<2)+1];
                             buffer[(ty*mode_info.BytesPerScanLine)+((tx*3)+2)] = cdata[(ty*_32BIT_SCANLINE)+(tx<<2)];
                         }
                     }
                 }
                 break;
        case 32: /* 32 bpp */
                 /* RGB or BGR? */
                 if(mode_info.BlueMaskPos) {
                     /* BGR */
                     for(ty = 0; ty < mode_info.YResolution; ty++) {
                         for(tx = 0; tx < mode_info.XResolution; tx++) {
                             buffer[(ty*mode_info.BytesPerScanLine)+(tx<<2)] = cdata[(ty*_32BIT_SCANLINE)+(tx<<2)+2];
                             buffer[(ty*mode_info.BytesPerScanLine)+((tx<<2)+1)] = cdata[(ty*_32BIT_SCANLINE)+(tx<<2)+1];
                             buffer[(ty*mode_info.BytesPerScanLine)+((tx<<2)+2)] = cdata[(ty*_32BIT_SCANLINE)+(tx<<2)];
                         }
                     }
                  } else {
                     /* RGB (just memcpy() because no conversation is needed now) */
                     memcpy(buffer,cdata,(mode_info.YResolution*mode_info.BytesPerScanLine));
                   }
                   break;
    }

    #ifdef FARPTR
        movedata(_my_ds(), (int)buffer, selector, 0, GFX_DRIVER_VRES * mode_info.BytesPerScanLine);
    #else
        __djgpp_nearptr_enable();
        memcpy(vesa_base, buffer, GFX_DRIVER_VRES * mode_info.BytesPerScanLine);
        __djgpp_nearptr_disable();
    #endif
}

_UINT
GFXDRIVER::get_hres() {
    return GFX_DRIVER_HRES;
}

_UINT
GFXDRIVER::get_vres() {
    return GFX_DRIVER_VRES;
}

_UINT
GFXDRIVER::get_nofcols() {
    return GFX_DRIVER_NOFCOLS;
}

void
GFXDRIVER::setpalette(char *ptr) {
}

void
GFXDRIVER::poll() {
    /* nothing to do! */
}

_UINT
GFXDRIVER::get_mode_info(_UINT mode) {
    __dpmi_regs r;
    _ULONG dosbuf;
    _UINT c;

    /* use the conventional memory transfer buffer */
    dosbuf = __tb & 0xFFFFF;

    /* initialize the buffer to zero */
    for (c=0; c<sizeof(MODE_INFO); c++) {
        _farpokeb(_dos_ds, dosbuf+c, 0);
    }

    /* call the VESA function */
    r.x.ax = 0x4F01;
    r.x.di = dosbuf & 0xF;
    r.x.es = (dosbuf>>4) & 0xFFFF;
    r.x.cx = mode;
    __dpmi_int(0x10, &r);

    /* quit if there was an error */
    if (r.h.ah) return 0;

    /* copy the resulting data into our structure */
    dosmemget(dosbuf, sizeof(MODE_INFO), &mode_info);

    /* it worked! */
    return 1;
}


_UINT
GFXDRIVER::set_mode(_UINT mode) {
    __dpmi_regs r;
    char tempstr[MAD_TEMPSTR_SIZE];
    _ULONG size;

    /* get mode information */
    if (!get_mode_info (mode)) {
        /* this failed. return */
        return 0;
    }

    /* calculate the size of the temp buffer */
    size=mode_info.BytesPerScanLine * mode_info.YResolution;

    /* did the allocation fail? */
    if ((buffer=(_UCHAR*)malloc(size))==NULL) {
        /* yup. complain (error 2: out of memory) */
        sprintf(tempstr,MAD_ERROR_2,size);
        die(tempstr);
    }

      /* linear frame buffer supported? */
      if(!(mode_info.ModeAttributes & LINEAR_FRAME_FLAG)) {
          /* no. return */
          return 0;
      }

      /* call the VESA mode set function */
      r.x.ax = 0x4F02;
      r.x.bx = mode + USE_LINEAR_FRAME;
      __dpmi_int(0x10, &r);

      /* did this work? */
      if (r.h.ah) {
          /* no. get out of here */
          return 0;
      }

      /* map into linear memory */
      mapping.address = mode_info.PhysBasePtr;
      mapping.size = vesa_info.TotalMemory << 16;

      if (__dpmi_physical_address_mapping(&mapping) != 0) {
          mapping.size = 0;
          die("Error [???]: Physical mapping of linear frame buffer failed");
          return 0;
      }

      #ifdef FARPTR
          /* allocate an LDT descriptor to access the linear region */
          selector = __dpmi_allocate_ldt_descriptors(1);

          /* did this work? */
          if (selector < 0) {
              /* no. complain */
              __dpmi_free_physical_address_mapping(&mapping);
              die("Error [???]: Unable allocate LDT descriptors");
          }

          /* set the descriptor location and size */
          __dpmi_set_segment_base_address(selector, mapping.address);
          __dpmi_set_segment_limit(selector, mapping.size-1);
       #else
          vesa_base = (_UCHAR *)(mapping.address + __djgpp_conventional_base);
       #endif
       /* it worked! */
       return 1;
}
