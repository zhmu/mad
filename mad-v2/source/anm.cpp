/*
                              ANM.CPP

                     Animation Engine for MAD

                       (c) 1999 The MAD Crew

  Description:
  This will the loading of the animations. The actual drawing will be done by
  GFX.CPP

  Todo: Nothing

  Portability notes: Fully portable, no changes required
*/
#include <stdlib.h>
#include <string.h>
#include "anm.h"
#include "archive.h"
#include "maderror.h"
#include "mad.h"
#include "types.h"

_UCHAR
ANM::init(char *name) {
        char *ptr;
        _UINT h,w,count;
        _ULONG pos;

        /* load the file from the archive */
        ptr=(char *)archive->readfile(name,NULL);

        memcpy(&header,ptr,sizeof(struct ANMHEADER));

        /* check if the magic number is correct. if not, return ANM_NOANMFILE */
        if(header.idcode!=ANM_MAGICNO) {
            free(ptr);
            return ANM_NOANMFILE;
        }
        /* check if the version number is correct. if not, return ANM_WRONGVERSION */
        if(header.version!=ANM_VERSION) {
            free(ptr);
            return ANM_WRONGVERSION;
        }
        /* now check how much sprites there are */
        if(header.nofsprites>=ANM_MAX_SPRITES) {
            /* damn, too many! die */
            free(ptr);
            return ANM_TOOMANYSPRITES;
        }
        /* now check how much frames there are */
        if(header.nofanims>=ANM_MAX_FRAMES) {
            /* damn, too many! die */
            free(ptr);
            return ANM_TOOMANYFRAMES;
        }
        /* set pos just after header */
        pos=sizeof(struct ANMHEADER);

        /* load the sprites */
        for(count=0;count<header.nofsprites;count++) {
            /* set the width and height fields */
            w=(_UINT)ptr[pos]+((_UINT)ptr[pos+1]<<(_UINT)8);
            h=(_UINT)ptr[pos+2]+((_UINT)ptr[pos+3]<<(_UINT)8);

            sprite[count].height=h;sprite[count].width=w;

            /* allocate memory for the sprite */
            if ((sprite[count].data=(char *)malloc(h*w))==NULL) {
                /* not enough memory, so return ANM_OUTOFMEMORY */
                free(ptr);

                return ANM_OUTOFMEMORY;
            }
            memcpy(sprite[count].data,(char *)(ptr+pos+4),(h*w));

            pos+=(4+(h*w));
        }

        /* now load the animations */
        for(count=0;count<header.nofanims;count++) {
                memcpy(&anim[count].anm,(char *)(ptr+pos),sizeof(struct ANIMATION));
                pos+=sizeof(struct ANIMATION);
                memcpy(&anim[count].frame,(char *)(ptr+pos),anim[count].anm.noframes);
                pos+=anim[count].anm.noframes;
        }
        /* set some flags */
        strcpy(filename,name);
        loaded=1;

        free(ptr);

        /* all went ok, so return ANM_OK */
        return ANM_OK;
}

void
ANM::done() {
        _UINT count;

        /* free all sprite buffers */
        for(count=0;count<header.nofsprites;count++) {
             free(sprite[count].data);
        }
        loaded=0;
}

char
*ANM::err2msg(_UCHAR errno) {
    switch (errno) {
            case ANM_OK: return ANM_ERROR_0;
       case ANM_OPENERR: return ANM_ERROR_1;
       case ANM_READERR: return ANM_ERROR_2;
       case ANM_WRITERR: return ANM_ERROR_3;
     case ANM_NOANMFILE: return ANM_ERROR_4;
  case ANM_WRONGVERSION: return ANM_ERROR_5;
   case ANM_OUTOFMEMORY: return ANM_ERROR_6;
      case ANM_CREATERR: return ANM_ERROR_7;
case ANM_TOOMANYSPRITES: return ANM_ERROR_8;
 case ANM_TOOMANYFRAMES: return ANM_ERROR_9;
    }
    return MAD_ERROR_UNKNOWN;
}
