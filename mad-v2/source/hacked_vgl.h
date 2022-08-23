/*-
 * Copyright (c) 1991-1997 Søren Schmidt
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in this position and unchanged.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software withough specific prior written permission
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $FreeBSD: src/lib/libvgl/vgl.h,v 1.2.2.1 1999/08/29 14:58:10 peter Exp $
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <machine/cpufunc.h>

typedef unsigned char byte;
typedef struct {
  byte 	Type;
  int  	Xsize, Ysize;
  byte 	*Bitmap;
} VGLBitmap;

/*
 * Defined Type's
 */
#define MEMBUF		0
#define VIDBUF4		1
#define VIDBUF8		2
#define VIDBUF8X	3
#define NOBUF		255

typedef struct VGLText {
  byte	Width, Height;
  byte	*BitmapArray;
} VGLText;

typedef struct VGLObject {
  int	  	Id;
  int	  	Type;
  int	  	Status;
  int	  	Xpos, Ypos;
  int	  	Xhot, Yhot;
  VGLBitmap 	*Image;
  VGLBitmap 	*Mask;
  int		(*CallBackFunction)();
} VGLObject;

#define MOUSE_IMG_SIZE		16
#define VGL_MOUSEHIDE		0
#define VGL_MOUSESHOW		1
#define VGL_MOUSEFREEZE		0
#define VGL_MOUSEUNFREEZE	1
#define VGL_DIR_RIGHT		0
#define VGL_DIR_UP		1
#define VGL_DIR_LEFT		2
#define VGL_DIR_DOWN		3
#define VGL_RAWKEYS		1
#define VGL_CODEKEYS		2
#define VGL_XLATEKEYS		3

extern VGLBitmap 	*VGLDisplay;

/*
 * Prototypes
 */
/* bitmap.c */
extern "C" int __VGLBitmapCopy(VGLBitmap *src, int srcx, int srcy, VGLBitmap *dst, int dstx, int dsty, int width, int hight);
extern "C" int VGLBitmapCopy(VGLBitmap *src, int srcx, int srcy, VGLBitmap *dst, int dstx, int dsty, int width, int hight);
/* keyboard.c */
extern "C" int VGLKeyboardInit(int mode);
extern "C" void VGLKeyboardEnd(void);
extern "C" int VGLKeyboardGetCh(void);
/* main.c */
extern "C" void VGLEnd(void);
extern "C" int VGLInit(int mode);
extern "C" void VGLCheckSwitch(void);
/* mouse.c */
extern "C" void VGLMousePointerShow(void);
extern "C" void VGLMousePointerHide(void);
extern "C" void VGLMouseMode(int mode);
extern "C" void VGLMouseAction(int dummy);
extern "C" void VGLMouseSetImage(VGLBitmap *AndMask, VGLBitmap *OrMask);
extern "C" void VGLMouseSetStdImage(void);
extern "C" int VGLMouseInit(int mode);
extern "C" int VGLMouseStatus(int *x, int *y, char *buttons);
extern "C" int VGLMouseFreeze(int x, int y, int width, int hight, byte color);
extern "C" void VGLMouseUnFreeze(void);
/* simple.c */
extern "C" void VGLSetXY(VGLBitmap *object, int x, int y, byte color);
extern "C" byte VGLGetXY(VGLBitmap *object, int x, int y);
extern "C" void VGLLine(VGLBitmap *object, int x1, int y1, int x2, int y2, byte color);
extern "C" void VGLBox(VGLBitmap *object, int x1, int y1, int x2, int y2, byte color);
extern "C" void VGLFilledBox(VGLBitmap *object, int x1, int y1, int x2, int y2, byte color);
extern "C" void VGLEllipse(VGLBitmap *object, int xc, int yc, int a, int b, byte color);
extern "C" void VGLFilledEllipse(VGLBitmap *object, int xc, int yc, int a, int b, byte color);
extern "C" void VGLClear(VGLBitmap *object, byte color);
extern "C" void VGLRestorePalette(void);
extern "C" void VGLSavePalette(void);
extern "C" void VGLSetPalette(byte *red, byte *green, byte *blue);
extern "C" void VGLSetPaletteIndex(byte color, byte red, byte green, byte blue);
extern "C" void VGLSetBorder(byte color);
extern "C" void VGLBlankDisplay(int blank);
/* text.c */
extern "C" int VGLTextSetFontFile(char *filename);
extern "C" void VGLBitmapPutChar(VGLBitmap *Object, int x, int y, byte ch, byte fgcol, byte bgcol, int fill, int dir);
extern "C" void VGLBitmapString(VGLBitmap *Object, int x, int y, char *str, byte fgcol, byte bgcol, int fill, int dir);
