/*
                            GFX_VGA.CPP

               VGA Graphix Engine for MAD - DOS version

                   (c) 1998, 1999 The MAD Crew

  Description:
  This will handle the actual low-level graphics functions.

  Todo: Nothing

  Portability notes: Needs to be fully rewritten for other platforms.
*/
#include <dos.h>
#include <stdlib.h>
#include <string.h>
#include <sys/movedata.h>
#include "archive.h"
#include "gfx_drv.h"
#include "mad.h"
#include "types.h"

void
GFXDRIVER::init() {
    union REGS r;

    memset(&r,0,sizeof(union REGS));
    r.x.ax=0x1130;              /* video bios: get font information */
    r.h.bh=0;
    int86(0x10,&r,&r);
    nofrows=r.h.dl;

    memset(&r,0,sizeof(union REGS));
    r.h.ah=0xf;                 /* video bios: get current video mode */
    int86(0x10,&r,&r);
    oldmode=r.h.al;

    memset(&r,0,sizeof(union REGS));
    r.x.ax=0x13;                /* bios: set video mode */
    int86(0x10,&r,&r);
}

void
GFXDRIVER::done() {
    union REGS r;

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
    /* move the data buffer to the video memory */
    dosmemput(data,64000,0xa0000);
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
    _UINT i;

    /* send the palette to the dac */
    for(i=0;i<256;i++) {
        outportb(0x3c8,i);             /* tell dac the color value */
        outportb(0x3c9,ptr[i*3]);      /* tell dac the red value */
        outportb(0x3c9,ptr[i*3+1]);    /* tell dac the green value */
        outportb(0x3c9,ptr[i*3+2]);    /* tell dac the blue value */
    }
}

void
GFXDRIVER::poll() {
    /* nothing to do! */
}
