/*
                              ANM.CPP

                     Animation Engine for MAD

                   (c) 1999, 2000 The MAD Crew

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
#include "gfx.h"
#include "maderror.h"
#include "mad.h"
#include "types.h"

ANM::ANM() {
    /* make sure we have no data */
    data=NULL;
}

_UCHAR
ANM::init(char *name) {
    _UINT h,w,count;
    _UCHAR i,j;
    _ULONG pos,size;

    /* load the file from the archive */
    data=(char *)archive_readfile(name,&size);

    /* create a pointer to the header */
    header=(struct ANMHEADER*)data;

    /* check if the magic number is correct. if not, return ANM_NOANMFILE */
    if(header->idcode!=ANM_MAGICNO) {
        return ANM_NOANMFILE;
    }
    /* check if the version number is correct. if not, return ANM_WRONGVERSION */
    if(header->version!=ANM_VERSION) {
        return ANM_WRONGVERSION;
    }
    /* now check how much sprites there are */
    if(header->nofsprites>=ANM_MAX_SPRITES) {
        /* damn, too many! die */
        return ANM_TOOMANYSPRITES;
    }
    /* now check how much frames there are */
    if(header->nofanims>=ANM_MAX_FRAMES) {
        /* damn, too many! die */
        return ANM_TOOMANYFRAMES;
    }
    /* set pos just after header */
    pos=sizeof(struct ANMHEADER);
              
    /* load the sprites */
    for(count=0;count<header->nofsprites;count++) {
        /* create a pointer to this sprite's header */
        sprite[count].hdr=(struct ANMSPRITEHEADER*)(data+pos);

        /* increment position */
        pos+=sizeof(ANMSPRITEHEADER);

        /* create a pointer to the sprite data */
        sprite[count].data=(char*)(data+pos);

        /* increment position */
        pos+=(sprite[count].hdr->heigth*sprite[count].hdr->width)*3;
    }

    /* now load the animations */
    for(count=0;count<header->nofanims;count++) {
        memcpy(&anim[count].anm,(char *)(data+pos),sizeof(struct ANIMATION));
        pos+=sizeof(struct ANIMATION);
        memcpy(&anim[count].frame,(char *)(data+pos),anim[count].anm.noframes*sizeof(_ULONG));
        pos+=(anim[count].anm.noframes*sizeof(_ULONG));
    }

    /* set some flags */
    strcpy(filename,name);
    loaded=1;

    /* all went ok, so return ANM_OK */
    return ANM_OK;
}

void
ANM::done() {
    _UINT count;

    /* do we have data? */
    if(data!=NULL) {
        /* yup. zap it */
        free(data);
        /* make sure we don't do this twice */
        data=NULL;
    }

    loaded=0;
}

char
*ANM::err2msg(_UCHAR err) {
    switch (err) {
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
