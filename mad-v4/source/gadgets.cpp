/*
                               GADGETS.CPP

                   MAD Gadgets - Some small helper functions

                    (c) 1998, 1999, 2000 The MAD Crew

  Description:
  This just includes some useful functions.

  Todo: Nothing

  Portability notes: Use #ifdef's when code must differ for other platforms.
*/
#ifdef DOS
#include <dos.h>
#endif
#ifdef WINDOWS
#include <windows.h>
#endif
#include <string.h>
#ifdef UNIX
#include <unistd.h>
#endif
#include "gfx.h"
#include "gfx_drv.h"
#include "mad.h"
#include "types.h"

/*

        stripquotes(char *source,char *dest)

        this will remove the quotes from a string, e.g. "this" will
        be come this. (without the dot of course)

*/
void
stripquotes (char *instr,char *fname) {
        _UINT i,pos;
        _UCHAR active,ch;

	active=0; pos=0; strcpy(fname,"");
	for(i=0;i<strlen(instr);i++) {
		ch=instr[i];
		if (ch=='"') {
			if (active) {
			 fname[pos]=0;
			 return;
			} else {
				active=1;
			}
		}
		if ((active)&&(ch!='"')) {
			fname[pos]=ch;
			pos++;
		}
	}
	fname[pos]=0;
}

/*
        delay_ms(UINT timeout)

        this will delay [timeout] ms.
*/
void
delay_ms(_UINT timeout) {
    #ifdef DOS
        delay(timeout);
    #endif
    #ifdef WINDOWS
        Sleep(timeout);
    #endif
    #ifdef UNIX
        usleep(timeout);
    #endif
}

/*

   int pow(int a, int b)

   Returns the power of a to b. (a^b)

*/
int pow(int a, int b) {
    int i,c=a;

    if (b==0) return 1;
    if (a==0) return 0;

    for(i=1;i<b;i++) c*=a;

    return c;
}

/*
 * box(_SINT x,_SINT y,_UINT h,_UINT w,_UCHAR col,char* dest)
 *
 * This will draw a rectangle from ([x],[y]) to ([x+w],[y+h]) in color [col]
 * to [dest].
 *
 *
 */
void
box(_SINT x,_SINT y,_UINT h,_UINT w,_ULONG col,char* dest) {
    _SINT i,j;

    for(j=y;j<=(y+h);j++) {
        for(i=x;i<=(x+w);i++) {
            if((i>=0)&&(i<GFX_DRIVER_HRES)&&(j>=0)&&(j<GFX_DRIVER_VRES)) {
                dest[gfx->linetable[j]+(i*GFX_BPP)+2]=(col>>16);
                dest[gfx->linetable[j]+(i*GFX_BPP)+1]=((col>>8)&0xff);
                dest[gfx->linetable[j]+(i*GFX_BPP)]=(col&0xff);
            }
        } /* width */
    } /* height */
}

/*
 * grayoutbox(_SINT x,_SINT y,_UINT h,_UINT w,char* dest)
 *
 * This will 'gray out' the rectangle from ([x],[y]) to ([x+w],[y+h])
 *
 */
void
grayoutbox(_SINT x,_SINT y,_UINT h,_UINT w,char* dest) {
    _SINT i,j;
    _UCHAR c;

    for(j=y;j<=(y+h);j++) {
        for(i=x;i<=(x+w);i++) {
            if((i>=0)&&(i<GFX_DRIVER_HRES)&&(j>=0)&&(j<GFX_DRIVER_VRES)) {
                c=dest[gfx->linetable[j]+(i*GFX_BPP)];
                dest[gfx->linetable[j]+(i*GFX_BPP)]=192+(64-c);
                dest[gfx->linetable[j]+(i*GFX_BPP)+1]=192+(64-c);
                dest[gfx->linetable[j]+(i*GFX_BPP)+2]=192+(64-c);
            }
        } /* width */
    } /* height */
}

/*
 * hline(_SINT x,_SINT y,_SINT x2,_UCHAR col,char* dest)
 *
 * This will draw a horizontal line from ([x],[y]) to ([x2],[y]) in color
 * [col] to [dest].
 *
 */
void
hline(_SINT x,_SINT y,_SINT x2,_ULONG col,char* dest) {
    _SINT i,z;

    for(i=x;i<=x2;i++) {
        if((i>=0)&&(i<GFX_DRIVER_HRES)&&(y>=0)&&(y<GFX_DRIVER_VRES)) {
                dest[gfx->linetable[y]+(i*GFX_BPP)+2]=(col>>16);
                dest[gfx->linetable[y]+(i*GFX_BPP)+1]=((col>>8)&0xff);
                dest[gfx->linetable[y]+(i*GFX_BPP)]=(col&0xff);
        }
    } /* width */
}

/*
 * vline(_SINT x,_SINT y,_SINT y2,_UCHAR col,char* dest)
 *
 * This will draw a vertical line from ([x],[y]) to ([x],[y2]) in color
 * [col] to [dest].
 *
 */
void
vline(_SINT x,_SINT y,_SINT y2,_ULONG col,char* dest) {
    _SINT j;

    for(j=y;j<=y2;j++) {
        if((x>=0)&&(x<GFX_DRIVER_HRES)&&(j>=0)&&(j<GFX_DRIVER_VRES)) {
            dest[gfx->linetable[j]+(x*GFX_BPP)+2]=(col>>16);
            dest[gfx->linetable[j]+(x*GFX_BPP)+1]=((col>>8)&0xff);
            dest[gfx->linetable[j]+(x*GFX_BPP)]=(col&0xff);
        }
    } /* height */
}
