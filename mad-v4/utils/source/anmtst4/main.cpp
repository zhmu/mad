/*
 *                  MAD Animation Test Utility Version 4.0
 *                       (c) 1999, 2000 the MAD Crew
 *
 */
#define  BANNER                 "MAD Animation Test Utility Version 4.0\n(c) 1999, 2000 The MAD Crew\n"
#define  MAX_FILENAME_LEN       64

#include <stdio.h>
#ifdef DOS
#include <conio.h>
#endif
#include "../../../source/include/anm.h"
#include "../../../source/include/controls.h"
#include "../../../source/include/gfx_drv.h"
#include "../../../source/include/ctrldrv.h"
#include "../../../source/include/gfx.h"
#include "../../../source/include/types.h"

/* it's bad karma to include c-files, but still... */
#ifdef DOS
#include "../../../source/dos/ctrldrv.cpp"
#include "../../../source/dos/gfx_vesa.cpp"
#elif defined(UNIX)
#include "../../../source/unix/gfx_x.cpp"
#include "../../../source/unix/ctrl_x.cpp"
#endif

GFXDRIVER* gfxdriver;
CTRLDRV*   ctrl_drv;
char*      vscreen;
char*      anmdata;

_UINT      cur_anm_no,cur_frame_no;

struct     ANMHEADER* hdr;

char*      backscreen;
char       filename[MAX_FILENAME_LEN];
char       pic_filename[MAX_FILENAME_LEN];

struct  ANMSPRITEDATA sprite[ANM_MAX_SPRITES];
struct  ANMFRAME anim[ANM_MAX_ANIMS];

/*
 * die(char* s)
 *
 * This will cause the program to die with error [s]. If [s] is NULL, no error
 * will be shown.
 *
 */
void
die(char* s) {
    /* do we have anaimtion data? */
    if(anmdata!=NULL) {
        /* yup. zap it */
        free(anmdata);
        /* make sure we will never do this again */
        anmdata=NULL;
    }
    /* do we have a virtual screen? */
    if(vscreen!=NULL) {
        /* yup. zap it */
        free(vscreen);
        /* make sure we will never do this again */
        vscreen=NULL;
    }
    /* do we have a background screen? */
    if(backscreen!=NULL) {
        /* yup. zap it */
        free(backscreen);
        /* make sure we will never do this again */
        backscreen=NULL;
    }

    /* was the controls driver initialized? */
    if(ctrl_drv!=NULL) {
        /* yup. deinitialize it */
        ctrl_drv->done();
        /* zap the class */
        delete ctrl_drv;
        /* make sure we don't do this twice */
        ctrl_drv=NULL;
    }

    /* was the graphics driver initialized? */
    if(gfxdriver!=NULL) {
        /* yup. deinitialize it */
        gfxdriver->done();
        /* zap the class */
        delete gfxdriver;
        /* make sure we don't do this twice */
        gfxdriver=NULL;
    }

    /* show our nice banner */
    printf("%s",BANNER);
    /* was there an error given? */
    if(s!=NULL) {
        /* yup. print it */
        fprintf(stderr,"%s",s);
    }
    /* die, with the correct error code. goodbye, blue sky! :D */
    exit((s==NULL) ? 1 : 0);
}

/*
 * usuage()
 *
 * This will show the program usuage.
 *
 */
void
usuage() {
    fprintf(stderr,"%s\n",BANNER);
    fprintf(stderr,"Usuage: ANMTST [parameters] filename\n\n");
    fprintf(stderr,"Parameters allowed:\n");
    fprintf(stderr,"-h, -?   this help\n\n");
    exit(1);
}

/*
 * build_vscreen()
 *
 * This will build the virtual screen.
 *
 */
void
build_vscreen() {
    _ULONG i,j,mx,my;
    _ULONG sprno;
    _ULONG ofs,sofs;
    _UCHAR* ptr;

    /* copy the back screen to the virtual screen */
    memcpy(vscreen,backscreen,GFX_DRIVER_HRES*GFX_DRIVER_HRES*GFX_BPP);

    /* add the current sprite */
    sprno=anim[cur_anm_no].frame[cur_frame_no];
    ptr=(_UCHAR*)sprite[sprno].data;
    for(j=0;j<sprite[sprno].hdr->heigth;j++) {
        for(i=0;i<sprite[sprno].hdr->width;i++) {
            /* plot the pixels! */
            ofs=(j*sprite[sprno].hdr->width*3)+(i*3);
            /* if this is still at the screen, plot it */
            mx=ctrl_drv->getxpos()+i;my=ctrl_drv->getypos()+j;
            if((mx<GFX_DRIVER_HRES)&&(my<GFX_DRIVER_VRES)) {
                sofs=(my*GFX_DRIVER_HRES*GFX_BPP)+(mx*GFX_BPP);

                /* need to draw the pixel? */
                if (!((ptr[ofs]==sprite[sprno].hdr->transcol_r)&&(ptr[ofs+1]==sprite[sprno].hdr->transcol_g)&&(ptr[ofs+2]==sprite[sprno].hdr->transcol_b))) {
                    vscreen[sofs]=ptr[ofs];
                    vscreen[sofs+1]=ptr[ofs+1];
                    vscreen[sofs+2]=ptr[ofs+2];
                }
            }
        }
    }
}

/*
 * load_anm(char* fname)
 *
 * This will load animation file [fname].
 *
 */
void
load_anm(char* fname) {
    _ULONG size,pos,count;
    FILE* f;

    /* open the file */
    if((f=fopen(fname,"rb"))==NULL) {
        /* this failed. die */
        die("Unable to open animation file");
    }
    /* figure out the size */
    fseek(f,0,SEEK_END);size=ftell(f);rewind(f);

    /* allocate memory */
    if((anmdata=(char*)malloc(size))==NULL) {
        /* this failed. die */
        die("Unable to allocate memory for animation file");
    }

    /* read the entire darn file into a buffer */
    if(!fread(anmdata,size,1,f)) {
        /* this failed. die */
        die("Unable to read animation file");
    }

    /* close the file */
    fclose(f);

    /* create a pointer to the header */
    hdr=(struct ANMHEADER*)anmdata;

    /* check the header */
    if(hdr->idcode!=ANM_MAGICNO) {
        die("This is not a MAD animation file");
    }
    if(hdr->version!=ANM_VERSION) {
        die("This is a file of a wrong version");
    }
    if(hdr->nofsprites>=ANM_MAX_SPRITES) {
        die("Too many sprites in file");
    }
    if(hdr->nofanims>=ANM_MAX_FRAMES) {
        die("Too many animations in file");
    }

    /* set pos just after header */
    pos=sizeof(struct ANMHEADER);
              
    /* load the sprites */
    for(count=0;count<hdr->nofsprites;count++) {
        /* create a pointer to this sprite's header */
        sprite[count].hdr=(struct ANMSPRITEHEADER*)(anmdata+pos);

        /* increment position */
        pos+=sizeof(ANMSPRITEHEADER);

        /* create a pointer to the sprite data */
        sprite[count].data=(char*)(anmdata+pos);

        /* increment position */
        pos+=(sprite[count].hdr->heigth*sprite[count].hdr->width)*3;
    }

    /* now load the animations */
    for(count=0;count<hdr->nofanims;count++) {
        memcpy(&anim[count].anm,(char *)(anmdata+pos),sizeof(struct ANIMATION));
        pos+=sizeof(struct ANIMATION);
        memcpy(&anim[count].frame,(char *)(anmdata+pos),anim[count].anm.noframes*sizeof(_ULONG));
        pos+=(anim[count].anm.noframes*sizeof(_ULONG));
    }

    /* reset some stuff */
    cur_anm_no=0;cur_frame_no=0;
}

/*
 * load_pic(char* fname)
 *
 * This will load picture [fname].
 *
 */
void
load_pic(char* fname) {
    char tempstr[MAD_TEMPSTR_SIZE];
    char *ptr;
    struct PICHEADER hdr;
    char *pal;
    _ULONG size;
    FILE *f;

    /* first load the file */
    if((f=fopen(fname,"rb"))==NULL) {
        die("ANMTST: Cannot open picture file");
    }

    /* read the file */
    if(!fread(&hdr,sizeof(PICHEADER),1,f)) {
        die("ANMTST: Cannot read picture file");
    }

    /* magic number ok? */
    if(hdr.idcode!=GFX_PIC_MAGICNO) {
        /* no. complain */
        die("ANMTST: Picture file isn't a MAD picture file");

    }
    /* version number ok? */
    if(hdr.version!=GFX_PIC_VERSIONO) {
        /* no. complain */
        die("ANMTST: Picture file is of a wrong version");
    }
    /* read the file */
    if(!fread(backscreen,(hdr.picheight*hdr.picwidth*4),1,f)) {
        die("ANMTST: Cannot read picture file");
    }

    /* close the file */
    fclose(f);
}

/*
 * main(int argc,char* argv[])
 *
 * Geez, I wouldn't know...
 *
 */
int
main(int argc,char* argv[]) {
    _UINT i;
    char* ptr;

    /* zap everything */
    gfxdriver=NULL;vscreen=NULL;ctrl_drv=NULL;anmdata=NULL;backscreen=NULL;

    /* parse the parameters */
    strcpy(filename,"");strcpy(pic_filename,"");
    for(i=1;i<argc;i++) {
        /* does it begin with a -? */
        if(argv[i][0]=='-') {
            /* yup. must be a parameter */
            switch(argv[i][1]) {
                case 'h':
                case '?': /* help */
                          usuage();
                          /* NOTREACHED */
                case 'p': /* load picture */
                          strcpy(pic_filename,(char*)(argv[i]+2));
                          break;
                 default: /* unknown parameter */
                          fprintf(stderr,"PCXGRAB: Unknown parameter '%c'\n",argv[i][1]);
                          exit(1);
                          /* NOTREACHED */
            }
        } else {
            /* it must be the filename. do we already have one? */
            if(strlen(filename)) {
                /* yup. complain */
                fprintf(stderr,"PCXGRAB: filename already given\n");
                exit(1);
            }
            /* is the length ok? */
            if(strlen(argv[i])>MAX_FILENAME_LEN) {
                /* no. complain */
                fprintf(stderr,"PCXGRAB: filename length '%s' too long\n");
                exit(1);
            }
            /* all looks good. copy the filename */
            strcpy(filename,argv[i]);
        }
    }

    /* was a filename given? */
    if(!strlen(filename)) {
        /* no. complain */
        fprintf(stderr,"ANMTST: no filename given\n");
        usuage();
        /* NOTREACHED */
    }

    /* open the animation file */
    load_anm(filename);


    /* allocate memory for the virtual screen */
    if((vscreen=(char*)malloc(GFX_DRIVER_HRES*GFX_DRIVER_HRES*GFX_BPP))==NULL) {
        /* this failed. die */
        die("ANMTST: unable to allocate memory for the virtual screen");
    }

    /* allocate memory for the background screen */
    if((backscreen=(char*)malloc(GFX_DRIVER_HRES*GFX_DRIVER_HRES*GFX_BPP))==NULL) {
        /* this failed. die */
        die("ANMTST: unable to allocate memory for the background screen");
    }
    /* zap the contents */
    memset(backscreen,0,GFX_DRIVER_HRES*GFX_DRIVER_HRES*GFX_BPP);

    /* if we have a pic, load it! */
    if(strlen(pic_filename)) {
        /* load it */
        load_pic(pic_filename);
    }

    /* initialize the graphics driver */
    if((gfxdriver=new GFXDRIVER)==NULL) {
        /* this failed. die */
        die("ANMTST: unable to allocate memory for the graphics driver");
    }

    /* initialize the controls driver */
    if((ctrl_drv=new CTRLDRV)==NULL) {
        /* this failed. die */
        die("ANMTST: unable to allocate memory for the controls driver");
    }
    /* initialize the graphics driver */
    gfxdriver->init();

    /* initialize the controls driver */
    ctrl_drv->init();

    while(1) {
        ctrl_drv->poll();

        /* build the virtual screen */
        build_vscreen();

        gfxdriver->draw(vscreen);

        /* if the + is down, increment the frame number */
        if(ctrldrv_keydown[CONTROLS_KEY_EQUAL]) {
            cur_frame_no++;
            if(cur_frame_no==(anim[cur_anm_no].anm.noframes-1)) {
                cur_frame_no=0;
            }
            ctrldrv_keydown[CONTROLS_KEY_EQUAL]=0;
        }

        /* if the ] is down, increment the animation number */
        if(ctrldrv_keydown[CONTROLS_KEY_RBRACKET]) {
            cur_anm_no++;
            if(cur_anm_no==hdr->nofanims) {
                cur_anm_no=0;
            }
            cur_frame_no=0;
            ctrldrv_keydown[CONTROLS_KEY_RBRACKET]=0;
        }

        /* if the mouse button is down, dump the coordinates */
        if(ctrl_drv->getbuttonstat()&1) {
            printf("%lu,%lu\n",ctrl_drv->getxpos(),ctrl_drv->getypos());
            while((ctrl_drv->getbuttonstat()&1)) ctrl_drv->poll();
        }
    }

    /* die (without error message) */
    die(NULL);
    return 0;
}
