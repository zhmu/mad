/*
                            GFX_DRV.CPP

                  Graphix Engine for MAD - DOS version

                   (c) 1998, 1999 RiNK SPRiNGER
                           The MAD Crew

*/
#include <dos.h>
#include <dpmi.h>
#include <stdlib.h>
#include <string.h>
#include "archive.h"
#include "gfx_drv.h"
#include "mad.h"
#include "types.h"

void
GFXDRIVER::init() {
        FILE *f;
        char tempstr[MAD_TEMPSTR_SIZE];
        char driver[GFX_DRIVER_MAX_SIZE];
        ULONG l;
        _go32_dpmi_registers r;

        driver_sel=-1;swap_sel=-1;
        /* try to open the driver */
        if((f=fopen(GFX_DRIVER_NAME,"rb"))==NULL) {
            /* we cant open it, so die */
            sprintf(tempstr,"unable to open driver '%s'\n",GFX_DRIVER_NAME);
            die(tempstr);
        }
        /* figure out the file size */
        fseek(f,0,SEEK_END); l=ftell(f); rewind(f);

        /* check if driver is small enough */
        if (l>GFX_DRIVER_MAX_SIZE) {
            /* driver is too big. report and die */
            sprintf(tempstr,"driver is too big, it may not exceed %u bytes\n",GFX_DRIVER_MAX_SIZE);
            die(tempstr);
        }

        /* try to allocate some real mode memory for the driver */
        if ((driver_seg=__dpmi_allocate_dos_memory((l+15)>>4,&driver_sel))==-1) {
            /* this failed, so die */
            sprintf(tempstr,"unable to allocate %u bytes of memory for driver\n",l);
            die(tempstr);
        }
        /* try to allocate a swap buffer */
        if ((swap_seg=__dpmi_allocate_dos_memory(4000,&swap_sel))==-1) {
            /* this failed, so die */
            sprintf(tempstr,"unable to allocate memory for swap buffer\n");
            die(tempstr);
        }

        /* try to read the driver */
        if(!fread(driver,l,1,f)) {
            /* this failed, so die */
            sprintf(tempstr,"unable to read driver '%s'\n",GFX_DRIVER_NAME);
            die(tempstr);
        }
        /* copy the driver to the dos memory */
        dosmemput(driver, l, driver_seg<<4);

        fclose(f);

        /* let the driver initialize itself */
        memset(&r,0,sizeof(_go32_dpmi_registers));
        r.x.bp = 0;
        r.x.cs = driver_seg;
        r.x.ip = 0;
        r.x.ss = r.x.sp = 0;
        
        _go32_dpmi_simulate_fcall(&r);
}

void
GFXDRIVER::done() {
    _go32_dpmi_registers r;

    if(swap_sel!=-1) {
        __dpmi_free_dos_memory(swap_sel);
        swap_sel=-1;
    }
    if(driver_sel!=-1) {
        /* let the driver deinitialize itself */
        memset(&r,0,sizeof(_go32_dpmi_registers));
            
        r.x.bp = 1;
        r.x.cs = driver_seg;
        
        _go32_dpmi_simulate_fcall(&r);
        __dpmi_free_dos_memory(driver_sel);
        driver_sel=-1;
    }
}

void
GFXDRIVER::draw(void *data) {
    _go32_dpmi_registers r;

    dosmemput(data, 64000, swap_seg<<4);

    memset(&r,0,sizeof(_go32_dpmi_registers));
    r.x.bp = 2;
    r.x.cs = driver_seg;
    r.x.ds = swap_seg;

    _go32_dpmi_simulate_fcall(&r);
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
