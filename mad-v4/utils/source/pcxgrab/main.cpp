/*
                       MAD Sprite Grabber Version 2.0
                    (c) 1999 RiNK SPRiNGER - The MAD Crew
*/
#include <conio.h>
#include <dos.h>
#include <pc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pcx.h"
#include "../../../source/types.h"

#define MOUSE_SELECTION_COLOR 14

#define STATE_DRAGGING  0
#define STATE_SELECTING 1

char *picture;
char *vscreen;
char palette[768];

_UINT mousex,mousey,buttonstat;
_UINT topx,topy,grabno;
_UCHAR nofrows,oldmode,state;

char
*loadpcx(char *fname) {
    FILE *f;
    _ULONG l;
    _UCHAR c,loop;
    _ULONG pos,offset;
    struct PCXHEADER hdr;
    char *ptr;
    char *tmp;

    /* try to open the file */
    if ((f=fopen(fname,"rb"))==NULL) {
        /* that didnt work. die */
	printf(" unable to open file\n");
        exit(1);
    }
    /* figure out the file size (minus header and palette) */
    fseek(f,0,SEEK_END); l=ftell(f)-128-768; rewind(f);
    /* read the header */
    if(!fread(&hdr,sizeof(struct PCXHEADER),1,f)) {
        /* unable to read it. die */
        printf(" unable to read file\n");
        exit(1);
    }
    /* check whether the header is ok */
    if((hdr.manufacturer!=0xa)||(hdr.encoding!=1)) {
        /* something was wrong in the header. die */
        printf("this is not a pcx file\n");
        exit(1);
    }
    if ((hdr.hres!=319)||(hdr.vres!=199)||(hdr.bpp!=8)) {
        /* dimensions were not ok. die */
        printf(" only 320x200x256 pcx files can be used\n");
        exit(1);
    }
    /* allocate memory for the temp buffer */
    if((tmp=(char *)malloc(l))==NULL) {
        /* that didnt work. die */
        printf(" out of memory when trying to allocate %lu bytes\n",l);
        exit(1);
    }
    /* read the file data into a temp buffer */
    if(!fread(tmp,l,1,f)) {
        /* it failed. die */
        printf(" unable to read file\n");
        exit(1);
    }
    /* allocate memory for the (uncompressed) picture */
    if((ptr=(char *)malloc(64000))==NULL) {
        /* this failed. die */
        printf(" out of memory when trying to allocate 64000 bytes\n");
        exit(1);
    }

    pos=0;offset=0;
    /* decompress it (RLE decompression) */
    while (pos<l) {
        c=tmp[pos];               /* get byte */
        if ((c&0xc0)==0xc0) {     /* are bits 7 and 8 set? */
            /* yes, its a loop */
            loop=(c&63);          /* strip the bits off, we have our loop value */
            while (loop--) {
                ptr[offset++]=tmp[pos+1];
            }
            pos++;
        } else {
            /* just one byte */
            ptr[offset++]=tmp[pos];
        }
        pos++;
    }
    /* free the temponary memory */
    free(tmp);
    /* seek to the right offset where the palette resides */
    fseek(f,l+128,SEEK_SET);
    /* read the palette */
    if(!fread(palette,768,1,f)) {
        /* this failed. die */
        printf(" unable to read file\n");
        exit(1);
    }
    /* close the file */
    fclose(f);

    /* and return the pointer to the uncompressed chunk */
    return ptr;
}

void
draw() {
    /* whack the virtual screen to the video memory */
    dosmemput(vscreen,64000,0xa0000);
}

void
setpalette(char *pal) {
    _UINT i;

    /* send the palette to the dac */
    for(i=0;i<256;i++) {
        outportb(0x3c8,i);             /* tell dac the color number */
        outportb(0x3c9,pal[i*3]>>2);   /* tell dac the red value */
        outportb(0x3c9,pal[i*3+1]>>2); /* tell dac the green value */
        outportb(0x3c9,pal[i*3+2]>>2); /* tell dac the blue value */
    }
}

void
initmouse() {
    union REGS r;

    memset(&r,0,sizeof(union REGS));
    int86(0x33,&r,&r);                  /* mouse driver: initialize */

    /* it ax the magic number 0xffff? */
    if(r.x.ax!=0xffff) {
        /* no, no mouse installed. die */
        printf(" no mouse driver installed\n");
        exit(2);
    }
}

void
pollmouse() {
    union REGS r;

    memset(&r,0,sizeof(union REGS));

    r.x.ax=0x3;                     /* mouse driver: return position and button status */
    int86(0x33,&r,&r);
    mousex=r.x.cx>>1;               /* store x position (must be divided by 2) */
    mousey=r.x.dx;                  /* store y position */
    buttonstat=r.x.bx;
}

void
initgfx() {
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
donegfx() {
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
h_line(_UINT x,_UINT x2,_UINT y,_UCHAR color) {
    _UINT i;

    /* draw a line from [x:y] to [x2:y], color [color] */
    for(i=x;i<x2;i++) {
        vscreen[320*y+i]=color;
    }
}

void
v_line(_UINT y,_UINT y2,_UINT x,_UCHAR color) {
    _UINT i;

    /* draw a line from [x:y] to [x:y2], color [color] */
     for(i=y;i<y2;i++) {
         vscreen[320*i+x]=color;
    }
}

void
updatevscreen() {
    /* first copy the background picture to the virtual screen */
    memcpy(vscreen,picture,64000);
    /* if user is currently dragging, create a huge cross */
    if(state==STATE_DRAGGING) {
        h_line(0,320,mousey,MOUSE_SELECTION_COLOR);
        v_line(0,200,mousex,MOUSE_SELECTION_COLOR);
        return;
    }
    /* if user is selecting something, create a rectangle around it */
    if(state==STATE_SELECTING) {
        h_line(topx,mousex,topy,MOUSE_SELECTION_COLOR);
        v_line(topy,mousey,topx,MOUSE_SELECTION_COLOR);
        h_line(topx,mousex,mousey,MOUSE_SELECTION_COLOR);
        v_line(topy,mousey,mousex,MOUSE_SELECTION_COLOR);
    }
}

void
savespr(_UINT x1,_UINT y1,_UINT x2,_UINT y2,char *fname) {
    FILE *f;
    _UINT i,j;
    char thefile[64];

    /* construct a filename */
    sprintf(thefile,"%s.%03u",fname,grabno);

    /* try to create the file */
    if ((f=fopen(thefile,"wb"))==NULL) {
        /* this failed. print message and die */
        donegfx();
        printf("Unable to create %s\n",thefile);
        exit(2);
    }
    /* figure out the coordinates */
    i=(x2-x1);j=(y2-y1);
    /* write the dimensions to the file */
    if (((fwrite(&i,2,1,f))!=1)||
        ((fwrite(&j,2,1,f))!=1)) {
        /* this failed. print message and die */
        donegfx();
        printf("Unable to write to %s\n",thefile);
        exit(2);
    }
    /* now write the actual image data */
    for(i=y1;i<y2;i++) {
        if ((fwrite((char *)picture+320*i+x1,(x2-x1),1,f))!=1) {
            /* this failed. print message and die */
            donegfx();
            printf("Unable to write %s\n",thefile);
            exit(2);
        }
    }
    fclose(f);
}

void
main(int argc,char *argv[]) {
    char basename[64];
    char *ptr;

    printf("MAD Sprite Grabber Version 2.0 - (c) 1999 The MAD Crew\n\n");
    /* is one parameter given? */
    if(argc!=2) {
        /* no, so print error message and die */
        printf("Usuage: PCXGRAB [filename.pcx]\n");
        exit(1);
    }
    printf("Loading %s...",argv[1]);
    picture=loadpcx(argv[1]);
    printf(" done\n");
    printf("Initializing...");
    fflush(stdout);                     /* to make last line visible */
    /* zero out basename */
    memset(basename,0,64);
    /* strip extension of argv[1] and dump this into basename */
    ptr=strchr(argv[1],'.');
    /* is there an extension? */
    if (ptr!=NULL) {
        /* yeah, copy everything before the dot to basename */
        strncpy(basename,argv[1],(strlen(argv[1])-1-(ptr-argv[1])));
    } else {
        /* just copy the whole name into basename */
        strcpy(basename,argv[1]);
    }
    /* try to allocate a virtual screen */
    if((vscreen=(char *)malloc(64000))==NULL) {
        /* it failed, so die */
        printf(" unable to allocate 64000 bytes for virtual screen\n");
        exit(2);
    }
    initmouse();
    grabno=0;
    initgfx();
    setpalette(palette);
    state=STATE_DRAGGING;
    while(!kbhit()) {
        pollmouse();
        updatevscreen();
        if (buttonstat&1) {
           topx=mousex;topy=mousey;
           while (buttonstat&1) {
               pollmouse();
               state=STATE_SELECTING;
               updatevscreen();
               draw();
           }
           /* if current mouse x and y are greater than the old ones,
              grab the sprite */
           if((mousex>=topx)&&(mousey>=topy)) {
               /* grab it */
               savespr(topx,topy,(mousex+1),(mousey+1),basename);
               /* sound to notify user */
               sound(500); delay(100); nosound();
               /* next grab number */
               grabno++;
           } else {
               /* sound to notify user */
               sound(100); delay(100); nosound();
           }
           state=STATE_DRAGGING;
        }
        draw();
    }
    donegfx();

    free(vscreen);
    free(picture);
    printf("MAD Sprite Grabber Version 2.0 - (c) 1999 The MAD Crew\n\n");
}
