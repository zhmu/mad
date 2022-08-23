/*
                             FONTMAN.CPP

                           MAD Font Manager

                      (c) 1999, 2000 The MAD Crew

  Description:
  This will manage all MAD fonts. It will only handle fonts with version 2.0.

  Todo: Nothing

  Portability notes: Fully portable, no changes required
*/
#include <stdlib.h>
#include <string.h>
#include "archive.h"
#include "fontman.h"
#include "gadgets.h"
#include "gfx.h"
#include "gfx_drv.h"
#include "maderror.h"
#include "objman.h"

/*
 * FONTMAN_SYSFONTFILENAME
 *
 * This is the constructor of the font manager object. It will reset most
 * fields to zero.
 *
 */
FONTMAN::FONTMAN() {
    _UINT i;

    /* initialize the variables */
    for(i=0;i<FONTMAN_MAXFONTS;i++) {
        font[i].inuse=0;
        font[i].header=NULL;
        font[i].datasize=0;
    }
}

/*
 * FONTMAN::init(char* sysfntfile)
 *
 * This will initialize the font manager. It will load [sysfntfile] as the
 * system font.
 *
 */
void
FONTMAN::init(char* sysfntfile) {
    sysfontno=loadfont(sysfntfile);
    strcpy(font[sysfontno].filename,FONTMAN_SYSFONTFILENAME);
}

/*
 * FONTMAN::charexists(_UINT fontno,_UCHAR c)
 *
 * This will return zero if char [c] doesn't exists in font [fontno],
 * otherwise a non-zero value.
 *
 */
_UCHAR
FONTMAN::charexists(_UINT fontno,_UCHAR c) {
    _UCHAR a;

    /* set [a] to the size data */
    a=font[fontno].charsize[c];

    /* if there's no height, say there's no char */
    if(!(a>>4)) return 0;

    /* if there's no width, say there's no char */
    if(!(a&0xf)) return 0;

    /* there's a char! */
    return 1;
}

/*
 * FONTMAN::done()
 *
 * This will unload all loaded fonts
 *
 */
void
FONTMAN::done() {
    _UINT i;

    /* clean up stuff */
    for(i=0;i<FONTMAN_MAXFONTS;i++) {
        /* unload the font */
        unloadfont(i);
    }

}

/*
 * FONTMAN::searchfreentry()
 *
 * This will search the font database for a free entry. If it is found, this
 * function will return its number, otherwise FONTMAN_NOFONT.
 *
 */
_UINT
FONTMAN::searchfreentry() {
    _UINT i;

    for(i=0;i<FONTMAN_MAXFONTS;i++) {
        /* is this entry free? */
        if(!font[i].inuse) {
            /* yeah, let's return this one */
            return i;
        }
    }
    /* no free entries were found. return FONTMAN_NOFONT */
    return FONTMAN_NOFONT;
}

/*
 * FONTMAN::searchfont(char* fontname)
 *
 * This will search the font database for a font [fontname]. If it is found,
 * this function will return its number, otherwise FONTMAN_NOFONT.
 *
 */
_UINT
FONTMAN::searchfont(char* fontname) {
    _UINT i;

    for(i=0;i<FONTMAN_MAXFONTS;i++) {
        /* is this entry free? */
        if(font[i].inuse) {
            /* yeah, does the name match? */
            if(!strcmp(font[i].filename,fontname)) {
                /* yeah, this is the one. return its number */
                return i;
            }
            /* no, keep looking */
        }
    }
    /* the font was not found. return FONTMAN_NOFONT */
    return FONTMAN_NOFONT;
}

/*
 * FONTMAN::loadfont(char* fname)
 *
 * This will cause the font manager to load font [fname]. If it is already
 * loaded, this will not reload it, but return the number of the font. If
 * all goes ok, it will return the number of the font, otherwise it will
 * quit.
 *
 */
_UINT
FONTMAN::loadfont(char* fname) {
    char tempstr[MAD_TEMPSTR_SIZE];
    _UINT i;

    /* was the font already loaded? */
    if((i=searchfont(fname))!=FONTMAN_NOFONT) {
        /* yeah. return its number */
        return i;
    }

    /* find a free entry for our font */
    i=searchfreentry();
    /* did that work? */
    if(i==FONTMAN_NOFONT) {
        /* no. we're out of entries! die (error 400: out of font manager entries) */
        die(MAD_ERROR_400);
    }

    /* load fontheader */
    font[i].header = (struct FONTHEADER*)archive_readfile(fname,&font[i].datasize);
    /* set the inuse flag */
    font[i].inuse=1;
    /* make size the size of the data-part, excluding the header */
    font[i].datasize -= sizeof(struct FONTHEADER);
    /* check if data available */
    if (font[i].datasize<0) {
       /* no. die (error 401: font file doesn't contain any data) */
       sprintf(tempstr,MAD_ERROR_401,fname);
       die(tempstr);
    }
    /* set pointer */
    font[i].charsize=(char*)font[i].header+sizeof(struct FONTHEADER);
    font[i].chardata=(char*)font[i].header+sizeof(struct FONTHEADER)+FONTMAN_NOFCHARS;

    /* store the file name */
    strcpy(font[i].filename,fname);

    /* check some fields */
    if (font[i].header->magic!=FONTMAN_MAGICID || font[i].header->version!=FONTMAN_VERSIONNO ||
       font[i].header->height_width==0) {
        /* some were wrong. die (error 402: font file is not a mad font file) */
        sprintf(tempstr,MAD_ERROR_402,fname);
        die(tempstr);
    }
    /* oh boy, all went ok. return the font's number */
    return i;
}

/*
 * FONTMAN::unloadfont(_UINT fontno)
 *
 * This will unload font [fontno]
 *
 */
void
FONTMAN::unloadfont(_UINT fontno) {
    /* was the font loaded? */
    if(!font[fontno].inuse) {
        /* no, just return */
        return;
    }

    /* was the data ever loaded? */
    if (font[fontno].header!=NULL) {
        /* yeah, free it */
        free((void*)font[fontno].header);
        /* make sure we don't do this a second time */
        font[fontno].header=NULL;
    }

    /* clear the inuse flag */
    font[fontno].inuse=0;
}

/*
 * FONTMAN::draw(void* buffer,_UCHAR)
 *
 * This will draw all text objects the object manager knows to [buffer]. If
 * [update] is non-zero, all frames will be updated as well.
 *
 */
void
FONTMAN::draw(void* buffer,_UCHAR update) {
    _UINT i;
    _SLONG x,y,dx,dy;

    /* put the text on the screen */
    for(i=0;i<OBJMAN_MAXOBJECTS;i++) {
        /* only visible texts */
        if ((objman->getype(i)==OBJMAN_TYPE_TEXT)&&(objman->isvisible(i))) {
            /* write it */
            textout(objman->getsourceno(i),objman->getcolor(i),objman->getext(i),x,objman->getobjecty(i),objman->getalign(i),(char*)buffer);

            /* does this text have to move? */
            if(objman->objmustmove(i)&&(update)) {
                /* yeah. check whether the speed is ok */
                objman->setcurwait(i,objman->getcurwait(i)+1);
                if((objman->getcurwait(i)==objman->getspeed(i))||(!objman->getspeed(i))) {
                    /* the time has come! */
                    objman->setcurwait(i,0);

                    /* do it */
                    x=objman->getobjectx(i);y=objman->getobjecty(i);
                    dx=objman->getobjectdestx(i);dy=objman->getobjectdesty(i);
                    /* horizontal movement? */
                    if(x!=dx) {
                        /* yes. need to move right? */
                        if(dx>x) {
                            /* yup. do it */
                            x+=(objman->getobjecthspeed(i));
                        } else {
                            /* no, move left */
                            x-=(objman->getobjecthspeed(i));
                        }
                    }
                    /* vertical movement? */
                    if(y!=dy) {
                        /* yes. need to move down? */
                        if(dy>y) {
                            /* yup. do it */
                            y+=(objman->getobjectvspeed(i));
                        } else {
                            /* no, move up */
                            y-=(objman->getobjectvspeed(i));
                        }
                    } else {
                        y=0;
                    }
                    /* activate new coordinates */
                    objman->setcoords(i,x,y);

                    /* are they equal now? */
                    if((x==dx)&&(y==dy)) {
                        /* yup. zap the moving flag */
                        objman->setmove(i,0);
                    }
                }
            }
        }
    }
}

void
FONTMAN::textout(_UINT fontno,_ULONG color,char *text, _SINT x, _SINT y, _UCHAR align, char *buffer) {
    _SINT cx,cy;
    _UCHAR result,a;
    _UINT i;

    /* set [cx] to the x coordinate and [cy] to the y coordinate */
    cx=x;cy=y;
    switch (align) {
        case FONTMAN_ALIGN_LEFT: /* left alignment */
                                 cx=x;
                                 break;
      case FONTMAN_ALIGN_CENTER: /* center aligning */
                                 cx=(GFX_DRIVER_HRES/2)-(getextwidth(fontno,text)/2);
                                 break;
    }

    /* loop for each character */
    for (i=0;i<strlen(text);i++) {
        /* newline? */
        if((text[i]=='\n')||((text[i]=='\\')&&(text[i+1]=='n'))) {
            /* yeah, handle it */
            cx=x;
            switch (align) {
        case FONTMAN_ALIGN_LEFT: /* left alignment */
                                 cx=x;
                                 break;
      case FONTMAN_ALIGN_CENTER: /* center aligning */
                                 cx=(GFX_DRIVER_HRES/2)-(getsentencewidth(fontno,(char*)(text+i+1))/2);
                                 break;
            }
            cy+=(font[fontno].header->height_width>>4);
            /* make sure we will not show the 'n' */
            if(text[i]!='\n') i++;
        } else {
            /* no. is the charachter a slash? */
            if(text[i]=='\\') {
                /* yup. skip the next char too */
                i++;
            } else {
                /* no. output it */
                charout(fontno,text[i],color,cx, cy, buffer);
                /* increase the x-position for each character */
                cx += getcharwidth(fontno,text[i])+1;
            }
        }
    }
}

void
FONTMAN::charout(_UINT fontno,_UCHAR c,_ULONG color,_SINT x,_SINT y,char* buffer) {
    _SINT cx, cy;
    _UINT dx, dy;
    _ULONG dataorg,rdx,t;
    _UINT bytepos=0, btpos=0, tmp;
    _UINT i;
    _UCHAR ch,a,w,h;
    char* pal;

    /* get the main palette */
    pal=gfx->get_main_pal();

    /* set [ch] to the char to draw */
    ch=c;

    /* if character not in font, use 'unknown' charachter */
    if (!charexists(fontno,ch)) {
       ch=FONTMAN_UNKNOWNCHAR;
    }

    /* count the offset in the data for this character */
    dataorg=0;
    for (i=0;i<ch;i++) {
        if (charexists(fontno,i)) {
            a = font[fontno].charsize[i];
            w = (a&0xf); h = (a>>4);
            t = w*h;
            t = t + (8 - (t % 8));
            dataorg += t;
        }
    }
    a = font[fontno].charsize[ch];
    w = (a&0xf); h = (a>>4);
    /* loop for each column of pixels */
    for (dy=0;dy<h;dy++) {
        /* loop for each row of pixels */
        for (dx=0;dx<w;dx++) {
            tmp = dataorg+(dy*w)+dx;
            bytepos = tmp/8;
            btpos  = tmp%8;
            rdx=dx*GFX_BPP;
            /* if pixel to be drawn */
            if ((font[fontno].chardata[bytepos] & pow(2,btpos)) !=0) {
                /* check clipping */
                if(((x+dx)>=0)&&((x+dx)<GFX_DRIVER_HRES)&&((y+dy)>=0)&&((y+dy)<GFX_DRIVER_VRES)&&((t+rdx+3)<(GFX_DRIVER_HRES*GFX_DRIVER_VRES*4))) {
                    /* make this pixel the specified color */
                    t=gfx->linetable[(y+dy)]+(x*GFX_BPP);
                    buffer[t+rdx+2]=(color>>16);
                    buffer[t+rdx+1]=((color>>8)&0xff);
                    buffer[t+rdx]=(color&0xff);
                }
            }
        }
    }
}

/*
 * FONTMAN::getsysfontno()
 *
 * This will return the font number of the system font
 *
 */
_UINT
FONTMAN::getsysfontno() {
    return sysfontno;
}

/*
 * FONTMAN::getextwidth(_UINT fontno,char* text)
 *
 * This will return the width of [text] in pixels, using font [fontno].
 *
 */
_UINT
FONTMAN::getextwidth(_UINT fontno,char* text) {
    _UINT i,j,w;

    /* was this font ever loaded? */
    if(!font[fontno].inuse) {
        /* no, just return zero */
        return 0;
    }
    /* figure out the size */
    j=0;w=0;
    for(i=0;i<strlen(text);i++) {
        /* newline? */
        if((text[i]=='\\')&&(text[i+1]=='n')) {
            /* yeah, handle it */
            /* is this [j] greater than previous? */
            if(j>w) {
                /* yeah, use this as new [w] */
                w=j;
            }
            j=0;
        } else {
            /* no, just add the size */
            j+=(getcharwidth(fontno,text[i]))+1;
        }
    }
    /* is there a max size? */
    if ((!w)||(j>w)) {
        /* no, return the calculated size */
        return j;
    }
    /* return the max size */
    return w;
}

/*
 * FONTMAN::getfontheight(_UINT fontno)
 *
 * This will return the height of font [fontno] in pixels.
 *
 */
_UINT
FONTMAN::getfontheight(_UINT fontno) {
    return((font[fontno].header->height_width)>>4);
}

/*
 * FONTMAN::getextheight(_UINT fontno,char* text)
 *
 * This will return the height of [text] in pixels, using font [fontno].
 *
 */
_UINT
FONTMAN::getextheight(_UINT fontno,char* text) {
    _UINT i,j;

    /* was this font ever loaded? */
    if(!font[fontno].inuse) {
        /* no, just return zero */
        return 0;
    }
    i=getfontheight(fontno);

    /* keep scanning for newlines */
    for(j=0;j<strlen(text);j++) {
        /* Newline? */
        if((text[j]=='\\')&&(text[j+1]=='n')) {
            /* Yeah, add to height */
            i+=getfontheight(fontno);
            if(text[j]!='\n') j++;
        }
    }

    /* Return the height */
    return i;
}

/*
 * FONTMAN::getfontwidth(_UINT fontno)
 *
 * This will return the width of font [fontno] in pixels.
 *
 */
_UINT
FONTMAN::getfontwidth(_UINT fontno) {
    return((font[fontno].header->height_width)&0xf);
}

/*
 * FONTMAN::getcharwidth(_UINT fontno,_UCHAR ch)
 *
 * This will return the width of char [ch] in font [fontno].
 *
 */
_UINT
FONTMAN::getcharwidth(_UINT fontno,_UCHAR ch) {
    _UCHAR a;

    /* set [a] to the size data */
    a=font[fontno].charsize[ch];

    /* return the width */
    return (a&0xf);
}

/*
 * FONTMAN::getcharheight(_UINT fontno,_UCHAR ch)
 *
 * This will return the height of char [ch] in font [fontno].
 *
 */
_UINT
FONTMAN::getcharheight(_UINT fontno,_UCHAR ch) {
    _UCHAR a;

    /* set [a] to the size data */
    a=font[fontno].charsize[ch];

    /* return the width */
    return (a>>4);
}

/*
 * FONTMAN::getsentencewidth(_UINT fontno,char* s)
 *
 * This will return the width of string [s], but it will stop when a \
 * operator is found, or when a nul is found.
 *
 */
_UINT
FONTMAN::getsentencewidth(_UINT fontno,char* text) {
    _UINT i,j;

    /* was this font ever loaded? */
    if(!font[fontno].inuse) {
        /* no, just return zero */
        return 0;
    }

    /* figure out the size */
    j=0;
    for(i=0;i<strlen(text);i++) {
        /* slash? */
        if(text[i]=='\\') {
            /* yeah, get outta here! */
            return j;
        } else {
            /* no, just add the size */
            j+=(getcharwidth(fontno,text[i]))+1;
        }
    }

    /* return the size */
    return j;
}
