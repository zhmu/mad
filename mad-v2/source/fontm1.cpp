/*
                             FONTMAN.CPP

                           MAD Font Manager

                       (c) 1999 The MAD Crew

  Description:
  This will manage all MAD fonts.

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
        font[i].data=NULL;
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
    return (font[fontno].header->entry[c/(sizeof(char)*8)] & pow(2,c%(sizeof(char)*8)))!=0;
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
    font[i].header = (struct FONTHEADER*)archive->readfile(fname,&font[i].datasize);
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
    /* set data pointer at end of header, pointing to the font data */
    font[i].data = (char*)font[i].header+sizeof(struct FONTHEADER);

    /* store the file name */
    strcpy(font[i].filename,fname);

    /* check some fields */
    if (font[i].header->magic!=FONTMAN_MAGICID || font[i].header->version!=FONTMAN_VERSIONNO ||
       font[i].header->height==0 || font[i].header->width==0) {
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
 * FONTMAN::draw(void* buffer)
 *
 * This will draw all text objects the object manager knows to [buffer].
 *
 */
void
FONTMAN::draw(void* buffer) {
    _UINT i;

    /* put the text on the screen */
    for(i=0;i<OBJMAN_MAXOBJECTS;i++) {
        /* only visible texts */
        if ((objman->getype(i)==OBJMAN_TYPE_TEXT)&&(objman->isvisible(i))) {
            /* write it */
            textout(objman->getsourceno(i),objman->getcolor(i),objman->getext(i),objman->getobjectx(i),objman->getobjecty(i),(char*)buffer);
        }
    }
}

void
FONTMAN::textout(_UINT fontno,_UCHAR color,char *text, _SINT x, _SINT y, char *buffer) {
    _SINT cx,cy;
    _UCHAR result;
    _UINT i;

    /* set [cx] to the x coordinate and [cy] to the y coordinate */
    cx=x;cy=y;
    /* loop for each character */
    for (i=0;i<strlen(text);i++) {
        /* newline? */
        if((text[i]=='\n')||((text[i]=='\\')&&(text[i+1]=='n'))) {
            /* yeah, handle it */
            cx=x;
            cy+=font[fontno].header->height;
            /* make sure we will not show the 'n' */
            if(text[i]!='\n') i++;
        } else {
            /* no, output the current character */
            charout(fontno,text[i],color,cx, cy, buffer);
            /* increase the x-position for each character */
            cx += font[fontno].header->width;
        }
    }
}

void
FONTMAN::charout(_UINT fontno,_UCHAR c,_UCHAR color,_SINT x,_SINT y,char* buffer) {
    _SINT cx, cy;
    _UINT dx, dy;
    _ULONG dataorg;
    _UINT bytepos=0, btpos=0, tmp;
    _UINT i,t;
    _UCHAR ch;

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
            t = font[fontno].header->width*font[fontno].header->height;
            t = t + (8 - (t % 8));
            dataorg += t;
        }
    }
    /* loop for each column of pixels */
    for (dy=0;dy<font[fontno].header->height;dy++) {
        /* loop for each row of pixels */
        t=(y+dy)*GFX_DRIVER_HRES+x;
        for (dx=0;dx<font[fontno].header->width;dx++) {
            tmp = dataorg+dy*font[fontno].header->width+dx;
            bytepos = tmp/(sizeof(char)*8);
            btpos  = tmp%(sizeof(char)*8);
            /* if pixel to be drawn */
            if ((font[fontno].data[bytepos] & pow(2,btpos)) !=0) {
                /* check clipping */
                if(((x+dx)>=0)&&((x+dx)<GFX_DRIVER_HRES)&&((y+dy)>=0)&&((y+dy)<=GFX_DRIVER_VRES)) {
                    /* make this pixel the specified color */
                    buffer[t+dx]=color;
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
//        if(((text[i]=='\\')&&(text[i+1]=='n'))||(text[i]!='\n')) {
        if((text[i]=='\\')&&(text[i+1]=='n')) {
            /* yeah, handle it */
            /* is this [j] greater than previous? */
            if(j>w) {
                /* yeah, use this as new [w] */
                w=j;
            }
            j=0;
            if(text[i]!='\n') i++;
        } else {
            /* no, just add the size */
            j+=font[fontno].header->width;
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
    return(font[fontno].header->height);
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
    i=font[fontno].header->height;

    /* keep scanning for newlines */
    for(j=0;j<strlen(text);j++) {
        /* Newline? */
        if((text[j]=='\\')&&(text[j+1]=='n')) {
            /* Yeah, add to height */
            i+=font[fontno].header->height;
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
    return(font[fontno].header->width);
}
