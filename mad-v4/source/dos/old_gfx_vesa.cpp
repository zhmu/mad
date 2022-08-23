/* TUOIC - The real way to call the draw function is (gfxdriver.*draw)(void *)
           The functions get_vres, get_hres, setpal, and poll are not present
           The head file you created for windows and dos is not implemented
           Sorry for this lack of goodies but I had to get it to work!


           TODO:
           -- find and delete all occurances of //XXX// in order to enable "die()"

*/


/* If FAR is defined the program used farpointers, otherwise the nearptr hack is used */
#define FAR
#include "types.h"
#include <dpmi.h>
#include <go32.h>
#include <sys/farptr.h>
#include <sys/movedata.h>
#include <sys/nearptr.h>
#include <string.h>
#include <stdio.h>
#include <conio.h>
#include <malloc.h>
#include <dos.h>
#include "vbe.h"
#include "gfx_drv.h"

#define GFXDRIVER_VRES          200
#define GFXDRIVER_HRES          320
#define LINEAR_FRAME_FLAG       128
#define USE_LINEAR_FRAME        16384
#define _32BIT_SCANLINE         1280
#define MODE_320x200_TRUE       0x10F
#define MODE_320x200_HIGH       0x10E

_UINT GFXDRIVER::init( void )
{
 buffer = NULL;

 if(!get_vesa_info())
  return 0;

 if(!set_mode(MODE_320x200_TRUE))
  return 0;

 return 1;
}

_UINT GFXDRIVER::done()
{

  /* All modes required a temp buffer, free it */

  if(buffer)
   free(buffer);

  /* There was a valid mapping, free it */
  if(mapping.size != 0)
   __dpmi_free_physical_address_mapping(&mapping);

  #ifdef FAR
  __dpmi_free_ldt_descriptor(selector);
  #endif


  /* Reset the previous mode */
  set_vga_mode(oldmode);

  return 1;
}

_UINT GFXDRIVER::draw32( void *data)
{
 _UINT ty, tx;
 _UCHAR *cdata = (_UCHAR *)data;

 /* Are we RGB or BGR? */

 if(mode_info.BlueMaskPos == 0)
 /* RGB */
  for(ty = 0; ty < mode_info.YResolution; ty++)
   for(tx = 0; tx < mode_info.XResolution; tx++)
   {
    buffer[(ty*mode_info.BytesPerScanLine)+(tx<<2)] = cdata[(ty*_32BIT_SCANLINE)+(tx<<2)+2];
    buffer[(ty*mode_info.BytesPerScanLine)+((tx<<2)+1)] = cdata[(ty*_32BIT_SCANLINE)+(tx<<2)+1];
    buffer[(ty*mode_info.BytesPerScanLine)+((tx<<2)+2)] = cdata[(ty*_32BIT_SCANLINE)+(tx<<2)];
   }
 else
 /* BGR */
  for(ty = 0; ty < mode_info.YResolution; ty++)
   for(tx = 0; tx < mode_info.XResolution; tx++)
   {
    buffer[(ty*mode_info.BytesPerScanLine)+(tx<<2)] = cdata[(ty*_32BIT_SCANLINE)+(tx<<2)];
    buffer[(ty*mode_info.BytesPerScanLine)+((tx<<2)+1)] = cdata[(ty*_32BIT_SCANLINE)+(tx<<2)+1];
    buffer[(ty*mode_info.BytesPerScanLine)+((tx<<2)+2)] = cdata[(ty*_32BIT_SCANLINE)+(tx<<2)+2];
   }


 /* Can we write without worrying about padded scanlines? Now we can*/
  #ifdef FAR
   movedata(_my_ds(), (int)data, selector, 0, GFXDRIVER_VRES * mode_info.BytesPerScanLine);
  #else
  {
   __djgpp_nearptr_enable();
   memcpy(vesa_base, data, GFXDRIVER_VRES * mode_info.BytesPerScanLine);
   __djgpp_nearptr_disable();
  }
  #endif

 return 32;

}

_UINT GFXDRIVER::draw24( void *data)
{
 _UINT ty, tx;
 _UCHAR *cdata = (_UCHAR *)data;

 /* Are we RGB or BGR? */
 if(mode_info.BlueMaskPos == 0)
 /*RGB*/
  for(ty = 0; ty < mode_info.YResolution; ty++)
   for(tx = 0; tx < mode_info.XResolution; tx++)
   {
    buffer[(ty*mode_info.BytesPerScanLine)+(tx*3)] = cdata[(ty*_32BIT_SCANLINE)+(tx<<2)+2];
    buffer[(ty*mode_info.BytesPerScanLine)+((tx*3)+1)] = cdata[(ty*_32BIT_SCANLINE)+(tx<<2)+1];
    buffer[(ty*mode_info.BytesPerScanLine)+((tx*3)+2)] = cdata[(ty*_32BIT_SCANLINE)+(tx<<2)];
   }
 else
  /*BGR*/
  for(ty = 0; ty < mode_info.YResolution; ty++)
   for(tx = 0; tx < mode_info.XResolution; tx++)
   {
    buffer[(ty*mode_info.BytesPerScanLine)+(tx*3)] = cdata[(ty*_32BIT_SCANLINE)+(tx<<2)];
    buffer[(ty*mode_info.BytesPerScanLine)+((tx*3)+1)] = cdata[(ty*_32BIT_SCANLINE)+(tx<<2)+1];
    buffer[(ty*mode_info.BytesPerScanLine)+((tx*3)+2)] = cdata[(ty*_32BIT_SCANLINE)+(tx<<2)+2];
   }

  /* Scanlines were padded by previous 32->24 conversion */

  #ifdef FAR
   movedata(_my_ds(), (int)buffer, selector, 0, GFXDRIVER_VRES * mode_info.BytesPerScanLine);
  #else
   __djgpp_nearptr_enable();
   memcpy(vesa_base, buffer, GFXDRIVER_VRES * mode_info.BytesPerScanLine);
   __djgpp_nearptr_disable();
  #endif

 return 24;
}
_UINT GFXDRIVER::draw16( void *data)
{
 _UINT ty, tx, color;
 _UCHAR *cdata = (_UCHAR *)data;

 for(ty = 0; ty < mode_info.YResolution; ty++)
   for(tx = 0; tx < mode_info.XResolution; tx++)
   {
    color = ((cdata[(ty*_32BIT_SCANLINE)+(tx<<2)  ] >> (8-mode_info.RedMaskSize)) << mode_info.RedMaskPos) +
            ((cdata[(ty*_32BIT_SCANLINE)+(tx<<2)+1] >> (8-mode_info.GreenMaskSize)) << mode_info.GreenMaskPos) +
            ((cdata[(ty*_32BIT_SCANLINE)+(tx<<2)+2] >> (8-mode_info.BlueMaskSize)) << mode_info.BlueMaskPos);

    buffer[(ty*mode_info.BytesPerScanLine)+(tx<<1)  ] = color & 0x00FF;
    buffer[(ty*mode_info.BytesPerScanLine)+(tx<<1)+1] = color >> 8;
   }


 #ifdef FAR
  movedata(_my_ds(), (int)buffer, selector, 0, GFXDRIVER_VRES * mode_info.BytesPerScanLine);
 #else
  __djgpp_nearptr_enable();
  memcpy(vesa_base, buffer, GFXDRIVER_VRES * mode_info.BytesPerScanLine);
  __djgpp_nearptr_disable();
 #endif

 return 16;
}
_UINT GFXDRIVER::draw15( void *data)
{
 _UINT ty, tx, color;
 _UCHAR *cdata = (_UCHAR *)data;

 for(ty = 0; ty < mode_info.YResolution; ty++)
   for(tx = 0; tx < mode_info.XResolution; tx++)
   {
    color = ((cdata[(ty*_32BIT_SCANLINE)+(tx<<2)  ] >> (8-mode_info.RedMaskSize)) << mode_info.RedMaskPos) +
            ((cdata[(ty*_32BIT_SCANLINE)+(tx<<2)+1] >> (8-mode_info.GreenMaskSize)) << mode_info.GreenMaskPos) +
            ((cdata[(ty*_32BIT_SCANLINE)+(tx<<2)+2] >> (8-mode_info.BlueMaskSize)) << mode_info.BlueMaskPos);

    buffer[(ty*mode_info.BytesPerScanLine)+(tx<<1)  ] = color & 0x00FF;
    buffer[(ty*mode_info.BytesPerScanLine)+(tx<<1)+1] = color >> 8;
   }


 #ifdef FAR
  movedata(_my_ds(), (int)buffer, selector, 0, GFXDRIVER_VRES * mode_info.BytesPerScanLine);
 #else
  __djgpp_nearptr_enable();
  memcpy(vesa_base, buffer, GFXDRIVER_VRES * mode_info.BytesPerScanLine);
  __djgpp_nearptr_disable();
 #endif

 return 15;
}

_UINT GFXDRIVER::get_vesa_info( void )
{
 __dpmi_regs r;
 _ULONG dosbuf;
 _UINT c;

 memset(&r,0,sizeof(union REGS));
 r.h.ah = 0xf;
 __dpmi_int(0x10, &r);
 oldmode = r.h.al;

 /* use the conventional memory transfer buffer */
 dosbuf = __tb & 0xFFFFF;

 /* initialize the buffer to zero */
 for (c=0; c<sizeof(VESA_INFO); c++)
  _farpokeb(_dos_ds, dosbuf+c, 0);

 dosmemput("VBE2", 4, dosbuf);


 memset(&r,0,sizeof(union REGS));

  /* call the VESA function */
 r.x.ax = 0x4F00;
 r.x.di = dosbuf & 0xF;
 r.x.es = (dosbuf>>4) & 0xFFFF;
 __dpmi_int(0x10, &r);

  /* quit if there was an error */
 if (r.h.ah)
 {
  //XXX//die("Unable to get VESA version information");
  return 0;
 }

   /* copy the resulting data into our structure */
 dosmemget(dosbuf, sizeof(VESA_INFO), &vesa_info);

   /* check that we got the right magic marker value */
 if (strncmp((char *)vesa_info.VESASignature, "VESA", 4) != 0)
 {
  //XXX//die("VESA returned unexpected information");
  return 0;
 }

  /* it worked! */


 return 1;
}

_UINT GFXDRIVER::get_mode_info(_UINT mode)
   {
      __dpmi_regs r;
      _ULONG dosbuf;
      _UINT c;

      /* use the conventional memory transfer buffer */
      dosbuf = __tb & 0xFFFFF;

      /* initialize the buffer to zero */
      for (c=0; c<sizeof(MODE_INFO); c++)
	 _farpokeb(_dos_ds, dosbuf+c, 0);

      /* call the VESA function */
      r.x.ax = 0x4F01;
      r.x.di = dosbuf & 0xF;
      r.x.es = (dosbuf>>4) & 0xFFFF;
      r.x.cx = mode;
      __dpmi_int(0x10, &r);

      /* quit if there was an error */
      if (r.h.ah)
	 return 0;

      /* copy the resulting data into our structure */
      dosmemget(dosbuf, sizeof(MODE_INFO), &mode_info);

      /* it worked! */

      return 1;
   }

_UINT GFXDRIVER::set_mode(_UINT mode)
   {
      __dpmi_regs r;
      _UINT mode_number = mode;

      /* First true 24/32 mode */
      if(!get_mode_info(mode_number))
      {
       /* No 24/32? Then try 15/16! */
       mode_number = MODE_320x200_HIGH;
       if(!get_mode_info(mode_number))
       {
        //XXX//die("Unable to get VESA Mode information");
        return 0; /* Mode info didn't work */
       }
      }

      switch(mode_info.BitsPerPixel)
      {
       case 15:
        buffer = (_UCHAR *)malloc(mode_info.BytesPerScanLine * mode_info.YResolution);
        draw = &GFXDRIVER::draw15;
        break;
       case 16:
        buffer = (_UCHAR *)malloc(mode_info.BytesPerScanLine * mode_info.YResolution);
        draw = &GFXDRIVER::draw16;
        break;
       case 24:
        buffer = (_UCHAR *)malloc(mode_info.BytesPerScanLine * mode_info.YResolution);
        draw = &GFXDRIVER::draw24;
        break;
       case 32:
        buffer = (_UCHAR *)malloc(mode_info.BytesPerScanLine * mode_info.YResolution);
        draw = &GFXDRIVER::draw32;
        break;
      }

      if(mode_info.ModeAttributes & LINEAR_FRAME_FLAG)
        mode_number += USE_LINEAR_FRAME;
      else
      {
       //XXX//die("Linear Frame not supported");
       return 0; /* Mode doesn't support linear frame buffer */
      }

      /* call the VESA mode set function */

      r.x.ax = 0x4F02;
      r.x.bx = mode_number;
      __dpmi_int(0x10, &r);

      if (r.h.ah)
      {
       //XXX//die("Unable to set VESA Mode");
       return 0;
      }

      /* map into linear memory */
      mapping.address = mode_info.PhysBasePtr;
      mapping.size = vesa_info.TotalMemory << 16;

      if (__dpmi_physical_address_mapping(&mapping) != 0)
      {
       mapping.size = 0;
       //XXX//die("Physical mapping of linear frame buffer failed");
       return 0;
      }
      #ifdef FAR
       /* allocate an LDT descriptor to access the linear region */
       selector = __dpmi_allocate_ldt_descriptors(1);

       if (selector < 0)
       {
        //XXX//die("Unable allocate ldt descriptors");
        __dpmi_free_physical_address_mapping(&mapping);
        return 0;
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

void GFXDRIVER::set_vga_mode(_UINT mode)
{
 __dpmi_regs r;
 r.x.ax = mode;
 __dpmi_int(0x10, &r);
}
