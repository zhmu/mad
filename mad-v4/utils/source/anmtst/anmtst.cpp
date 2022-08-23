#include <conio.h>
#include <ctype.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/movedata.h>
#include "gfx.h"
#include "anm.h"

#define MAX_SPRITES 128
#define MAX_FRAMES 128
#define SCREENSIZE 64000
#define SCREENCOLOR 1
#define TEXTCOL 0xf

struct ANMHEADER hdr;
struct ANMSPRITEDATA sprite[MAX_SPRITES];
struct ANMFRAME anims[MAX_SPRITES];

FILE *f;
_UCHAR nofrows,oldmode,curbackscr,mouse_installed;
_UINT mousex,mousey,buttonstat;
char *buf;
char *backscreen,*priorityscreen,*maskscreen;
char pal[768];

_UINT
readword() {
    _UCHAR i,j;

    if(!fread((void *)&i,1,1,f)) {
        printf("read error\n");
        exit(1);
    }
    if(!fread((void *)&j,1,1,f)) {
        printf("read error\n");
        exit(1);
    }
    return ((j<<8)+i);
}

void
putsprite(_SINT xpos,_SINT ypos,_UINT h,_UINT w,char *data,char *dest) {
    _UINT y,x;
    _UCHAR c;

    for(y=0;y<h;y++) {
        for(x=0;x<w;x++) {
            c=data[(w*y)+x];
            /* ignore color zero */
            if (c) { dest[(320*(ypos+y))+xpos+x]=c; }
        }
    }
}

void
RLEdecompress(char *inbuf,char *outbuf,_UINT destsize) {
        _UCHAR c,loop;
        _ULONG pos,offset;

        pos=0;offset=0;
        /* decompress it to [destsize] bytes */
        while (offset<destsize) {
                c=inbuf[pos];
                if ((c&192)==192) {     /* are bits 7 and 8 set? */
                        /* yes, its a loop */
                        loop=(c&63);    /* clear them, we have our loop value */
                        while (loop--) {
                                outbuf[offset++]=inbuf[pos+1];
                        }
                        pos++;
                } else {
                        /* just one byte */
                        outbuf[offset++]=inbuf[pos];
                }
                pos++;
        }

}

void
loadpic(char *fname) {
        char *ptr;
        struct PICHEADER *hdr;
        _ULONG l;
        FILE *f;

        /* first open the file */
        if((f=fopen(fname,"rb"))==NULL) {
            /* this failed. die */
            printf("unable to open '%s'\n",fname);
            exit(1);
        }
        /* figure out the size */
        fseek(f,0,SEEK_END);l=ftell(f);rewind(f);
        /* allocate memory */
        if((ptr=(char *)malloc(l))==NULL) {
            /* this failed. die */
            printf("out of memory when trying to load '%s'\n",fname);
            exit(1);
        }
        /* read the file */
        if(!fread(ptr,l,1,f)) {
            /* this failed. die */
            printf("unable to read '%s'\n",fname);
            exit(1);
        }
        /* close the file */
        fclose(f);

        /* create a pointer to the header */
        hdr=(struct PICHEADER *)ptr;

        /* magic number ok? */
        if(hdr->idcode!=GFX_PIC_MAGICNO) {
            /* no, die */
            printf("file '%s' is not a picture",fname);
            exit(1);
        }
        /* version number ok? */
        if(hdr->version!=GFX_PIC_VERSIONO) {
            /* no, die */
            printf("file '%s' has a wrong picture version",fname);
            exit(1);
        }
        /* decode the background picture */
        if(hdr->gfx_offset!=GFX_PIC_NOPIC) {
            /* and decompress it into [backscreen] */
            RLEdecompress((char *)(ptr+hdr->gfx_offset),backscreen,64000);
        } else {
            memset(backscreen,0,64000);
        }
        /* decode the priority screen */
        if(hdr->pri_offset!=GFX_PIC_NOPIC) {
            /* and decompress it into [backscreen] */
            RLEdecompress((char *)(ptr+hdr->pri_offset),priorityscreen,64000);
        } else {
            memset(priorityscreen,0,64000);
        }
        /* decode the priority screen */
        if(hdr->msk_offset!=GFX_PIC_NOPIC) {
            /* and decompress it into [backscreen] */
            RLEdecompress((char *)(ptr+hdr->msk_offset),maskscreen,64000);
        } else {
            memset(maskscreen,0,64000);
        }
        free(ptr);
}

void
initmouse() {
    union REGS r;

    memset(&r,0,sizeof(union REGS));
    int86(0x33,&r,&r);                  /* mouse driver: initialize */

    mousex=160;mousey=100;

    mouse_installed=(r.w.ax==0xffff);
}

void
pollmouse() {
    union REGS r;

    /* do nothing if no mouse installed */
    if(!mouse_installed) return;

    memset(&r,0,sizeof(union REGS));

    r.x.ax=0x3;                     /* mouse driver: return position and button status */
    int86(0x33,&r,&r);
    mousex=r.x.cx>>1;               /* store x position (must be divided by 2) */
    mousey=r.x.dx;                  /* store y position */
    buttonstat=r.x.bx;
}

void
loadpal(char* fname) {
    FILE *f;

    if((f=fopen(fname,"rb"))==NULL) {
        printf("Unable to open palette file '%s'\n",fname);
        exit(1);
    }
    if(!fread(pal,768,1,f)) {
        printf("Unable to read palette file '%s'\n",fname);
        exit(1);
    }
    fclose(f);
}

void
setpal() {
    _UINT i;

    for(i=0;i<256;i++) {
        outportb(0x3c8,i);
        outportb(0x3c9,pal[i*3]>>2);
        outportb(0x3c9,pal[i*3+1]>>2);
        outportb(0x3c9,pal[i*3+2]>>2);
    }
}

void
main(int argc,char *argv[]) {
    _UINT i,h,w,count,animno,dely,animate,dcount,posx,posy;
    _UCHAR ch;
    union REGS r;

    if(argc<2) {
        printf("usuage: anmtst file.anm [back.pic] [palfile.pal]\n");
        exit(1);
    }

    if((f=fopen(argv[1],"rb"))==NULL) {
        printf("unable to open animation file\n");
        exit(1);
    }
    if (!fread(&hdr,sizeof(struct ANMHEADER),1,f)) {
        printf("unable to read animation file\n");
        exit(1);
    }
    if(hdr.idcode!=ANM_MAGICNO) {
        printf("this is not a MAD animation file\n");
        exit(1);
    }
    if(hdr.version!=ANM_VERSION) {
        printf("version is incorrect\n");
        exit(1);
    }
    if((buf=(char *)malloc(64000))==NULL) {
        printf("out of memory for gfx buffer\n");
        exit(1);
    }
    printf("loading sprites");
    if(hdr.nofsprites>MAX_SPRITES) {
        printf("... failed, too much sprites in this file.\n");
        exit(1);
    }
    for(i=0;i<hdr.nofsprites;i++) {
        w=readword();h=readword();
        if((sprite[i].data=(char *)malloc(h*w))==NULL) {
            printf("out of memory\n");
            exit(1);
        }
        sprite[i].height=h;sprite[i].width=w;
        if(!fread(sprite[i].data,(h*w),1,f)) {
            printf("read error\n");
            exit(1);
        }
        printf(".");
    }
    printf(" done\n");
    /* now load the animations */
    if(hdr.nofanims>MAX_FRAMES) {
        printf("... failed, too much animations in this file.\n");
        exit(1);
    }
    printf("loading animations");
    for(i=0;i<hdr.nofanims;i++) {
        if(!fread(&anims[i].anm,sizeof(struct ANIMATION),1,f)) {
            printf("read error\n");
            exit(1);
        }
        if(!fread(&anims[i].frame,anims[i].anm.noframes,1,f)) {
            printf("read error\n");
            exit(1);
        }
        printf(".");
    }
    printf(" done\n");
    /* allocate memory for the background screen */
    if((backscreen=(char *)malloc(SCREENSIZE))==NULL) {
        /* this failed. die */
        printf("unable to allocate %lu bytes for background screen\n",SCREENSIZE);
        exit(1);
    }
    /* allocate memory for the mask screen */
    if((maskscreen=(char *)malloc(SCREENSIZE))==NULL) {
        /* this failed. die */
        printf("unable to allocate %lu bytes for mask screen\n",SCREENSIZE);
        exit(1);
    }
    /* allocate memory for the priority screen */
    if((priorityscreen=(char *)malloc(SCREENSIZE))==NULL) {
        /* this failed. die */
        printf("unable to allocate %lu bytes for priority screen\n",SCREENSIZE);
        exit(1);
    }
    /* check for background screen */
    if(argc>=3) {
        /* we need to load it. do it */
        printf("loading background picture...");
        loadpic(argv[2]);
        printf(" done\n");
    } else {
        /* just use solid-color screens */
        memset(backscreen,SCREENCOLOR,SCREENSIZE);
        memset(priorityscreen,SCREENCOLOR,SCREENSIZE);
        memset(maskscreen,SCREENCOLOR,SCREENSIZE);
    }
    /* check for palette screen */
    if(argc>=4) {
        /* we need to load it. do it */
        printf("loading palette...");
        loadpal(argv[3]);
        printf(" done\n");
    }
    initmouse();

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
    r.x.ax=0x13;                /* video bios: set video mode */
    int86(0x10,&r,&r);

    count=1;ch=0;animate=0;posx=0;posy=0;
    animno=0;i=anims[animno].frame[0]-1;dely=100;curbackscr=0;dcount=0;

    if(argc==4) {
        setpal();
    }

    i=0;
    while (ch!=0x1b) {
        switch(curbackscr) {
            case 0: memcpy(buf,backscreen,SCREENSIZE); break;
            case 1: memcpy(buf,priorityscreen,SCREENSIZE); break;
           default: memcpy(buf,maskscreen,SCREENSIZE); break;
        }
        i=anims[animno].frame[count]-1;
//        printf("%u  %u\n",sprite[i].height,sprite[i].width);
        putsprite (mousex,mousey,sprite[i].height,sprite[i].width,sprite[i].data,buf);
        if(animate) {
            if((dcount++==dely)||(!dely)) {
                dcount=0;
                count++;
                if(count==anims[animno].anm.noframes) { count=0; }
            }
        }
        ch=0;
        while(kbhit()) ch=toupper(getch());
        switch(ch) {
            case ']': /* next animation */
                      animno=(animno+1)%hdr.nofanims;
                      count=0;
                      break;
            case '[': /* previous animation */
                      if(animno) {
                          animno--;
                      } else {
                          animno=hdr.nofanims-1;
                      }
                      count=0;
                      break;
            case 'S': /* slower */
                      if(dely<1000) dely+=10;
                      dcount=0;
                      break;
            case 'F': /* faster */
                      if(dely>0) dely-=10;
                      dcount=0;
                      break;
            case ' ': /* toggle animate */
                      animate^=1;
                      break;
            case 'R': /* reset to frame 1 */
                      count=0;
                      break;
            case 'M': /* show mask screen */
                      curbackscr=2;
                      break;
            case 'P': /* show priority screen */
                      curbackscr=1;
                      break;
            case 'B': /* show back screen */
                      curbackscr=0;
                      break;
            case '=': /* next frame */
                      count++;
                      if(count==anims[animno].anm.noframes) { count=0; }
                      break;
            case '-': /* previous frame */
                      if(count) {
                          count--;
                      } else {
                          count=anims[animno].anm.noframes-1;
                      }
                      break;
        }
        if(buttonstat&1) {
            fprintf(stderr,"%u,%u\n",mousex,mousey);
            while (buttonstat&1) pollmouse();
        }
        if(buttonstat&2) {
            putsprite (mousex,mousey,sprite[i].height,sprite[i].width,sprite[i].data,backscreen);
            while (buttonstat&2) pollmouse();
        }
        dosmemput(buf,64000,0xa0000);
        delay(1);
        pollmouse();
    }

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
    free(buf);
    for(i=0;i<hdr.nofsprites;i++) {
        free(sprite[i].data);
    }
    free(backscreen);free(maskscreen);free(priorityscreen);
    printf("Animations:\n");
    for(i=0;i<hdr.nofanims;i++) {
        printf ("FRAMES for ANIMATION %u (%s): ",i,anims[i].anm.name);
        for(count=0;count<anims[i].anm.noframes;count++) {
            printf("%u ",anims[i].frame[count]);
        }
        printf("flags %u\n",anims[i].anm.flags);
    }
    exit(0);
}
