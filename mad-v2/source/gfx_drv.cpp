/*
                            GFX_DRV.CPP

                  Graphix Engine for MAD - DOS version

                   (c) 1998, 1999 RiNK SPRiNGER
                           The MAD Crew

*/
#include <dos.h>
#include <dpmi.h>
#include <io.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dxe.h>
#include "archive.h"
#include "gfx_drv.h"
#include "mad.h"
#include "types.h"

static int (*driver)(UINT function, char *buffer);

void *_dxe_load(char *name)
{
  dxe_header dh;
  char *data;
  int h;

  h = _open(name, 0);
  if (h < 0)
    return 0;
  _read(h, &dh, sizeof(dh));
  if (dh.magic != DXE_MAGIC)
  {
    _close(h);
    return 0;
  }

  data = (char *)malloc(dh.element_size);
  if (data == 0)
  {
    _close(h);
    return 0;
  }

  _read(h, data, dh.element_size);

  {
    long relocs[dh.nrelocs];
    int i;
    _read(h, relocs, sizeof(long)*dh.nrelocs);
    _close(h);

    for (i=0; i<dh.nrelocs; i++)
      *(long *)(data + relocs[i]) += (int)data;
  }

  return (void *)(data + dh.symbol_offset);
}

void
GFXDRIVER::init() {
        FILE *f;
        char tempstr[MAD_TEMPSTR_SIZE];
        ULONG l;
        _go32_dpmi_registers r;

       driver=0;
       
       driver=(void *)_dxe_load(GFX_DRIVER_NAME);
       if (driver==0) {
           sprintf(tempstr,"Unable to load graphics driver '%s'\n",GFX_DRIVER_NAME);
           die(tempstr);
       }

       driver(GFX_DRIVER_INIT, NULL);
}

void
GFXDRIVER::done() {
    if (driver!=0) {
        driver(GFX_DRIVER_DEINIT, NULL);
    }
}

void
GFXDRIVER::draw(void *data) {
    if (driver!=0) {
        driver(GFX_DRIVER_DRAW, data);
    }
}

UINT
GFXDRIVER::get_hres() {
        return GFX_DRIVER_HRES;
}

UINT
GFXDRIVER::get_vres() {
        return GFX_DRIVER_VRES;
}

UINT
GFXDRIVER::get_nofcols() {
        return GFX_DRIVER_NOFCOLS;
}

void
GFXDRIVER::loadpalette(char *fname) {
}
