/* are we already included? */
#ifndef __DLGMAN_INCLUDED__
/* no. set flag we are included, and define things */
#define __DLGMAN_INCLUDED__

#include "types.h"

#define DLGMAN_DEFAULTITLE              "MAD"

#define DLGMAN_MAX_CONTROLS             32      /* maximum number of controls */
#define DLGMAN_MAX_DIALOGS              8       /* maximum number of dialogs */
#define DLGMAN_NODIALOG                 0xffff  /* there is no dialog */

#define DLGMAN_CONTROL_MAXCAPTIONLEN    16384   /* maximum length of a caption */
#define DLGMAN_CONTROL_DEFAULTCAPTION   "creative title"     /* default caption */
#define MAD_SAVEGAME_MAX_FILENAME_LEN   16

#define DLGMAN_DIALOG_NOFBORDERS        4

#define DLGMAN_CONTROL_CURSOR           "_"

#define DLGMAN_CONTROL_TYPE_LABEL       0       /* label */
#define DLGMAN_CONTROL_TYPE_BUTTON      1       /* button */
#define DLGMAN_CONTROL_TYPE_TEXTINPUT   2       /* text input */
#define DLGMAN_CONTROL_TYPE_LIST        3       /* text list */
#define DLGMAN_CONTROL_TYPE_SPRITE      4       /* sprite */
#define DLGMAN_CONTROL_TYPE_ANM         5       /* animation */

#define DLGMAN_CONTROL_FLAG_VISIBLE     1       /* you can see me! */
#define DLGMAN_CONTROL_FLAG_ENABLED     2       /* you can click me! */

#define DLGMAN_CONTROL_LIST_FLAG_UP     1       /* up is hit */
#define DLGMAN_CONTROL_LIST_FLAG_DOWN   2       /* down is hit */
#define DLGMAN_CONTROL_LIST_NOENTRY     0xffff  /* no entry */

#define DLGMAN_CONTROL_SPRITE_BORDERCOL 0x000000 /* sprite: focus border color */

#define DLGMAN_LIST_SELCOLOR            0x000000 /* selected color */
#define DLGMAN_LIST_BACKSELCOLOR        0xffffff /* background color of selection */

#define DLGMAN_DIALOG_BACKCOLOR         0xc0c0c0 /* dialog background color */
#define DLGMAN_DIALOG_BORDERCOL         0xffffff /* upper-left border color */
#define DLGMAN_DIALOG_BORDERCOL2        0x808080 /* lower-right border color */
#define DLGMAN_DIALOG_TITLEBARCOLOR     0x0000ff /* color of the title bar */
#define DLGMAN_DIALOG_TITLECOLOR        0xffff00 /* title color */

#define DLGMAN_BORDER_HEIGHT            2       /* dialog border height */
#define DLGMAN_BORDER_WIDTH             2       /* dialog border width */
#define DLGMAN_DIALOG_SPARETITLEBARPIXELS 0     /* spare pixels on title bar (vertical) */

#define DLGMAN_DIALOG_FLAG_MOVEABLE     1       /* dialog can be moved */
#define DLGMAN_DIALOG_FLAG_TITLEBAR     2       /* dialog has a title bar */
#define DLGMAN_DIALOG_FLAG_NOMOUSE      4       /* hide mouse cursor */
#define DLGMAN_DIALOG_FLAG_NODEFAULT    8       /* ignore all defaults */
#define DLGMAN_DIALOG_FLAG_GRAYBACK     16      /* 'gray out' background */

#define DLGMAN_DIALOG_NOHIT             0xfffe  /* nothing is touched */

#define DLGMAN_BUTTON_BACKCOLOR         0xc0c0c0 /* button background color */
#define DLGMAN_BUTTON_BORDERCOL         0xffffff /* upper-left border color */
#define DLGMAN_BUTTON_BORDERCOL2        0x808080 /* lower-right border color */

#define DLGMAN_FOCUSCOL                 0x000000 /* color of focus indicating box */

#define DLGMAN_TEXT_OKBUTTON            "OK"    /* ok button text */
#define DLGMAN_TEXT_YESBUTTON           "Yes"   /* yes button text */
#define DLGMAN_TEXT_NOBUTTON            "No"    /* no button text */

#define DLGMAN_LIST_UPBUTTON            "Up"    /* up button text */
#define DLGMAN_LIST_DNBUTTON            "Down"  /* down button text */

#define DLGMAN_TEXTINPUT_BORDERCOL      0x000000 /* text input box border color */
#define DLGMAN_TEXTINPUT_BACKCOL        0xc0c0c0 /* text input box background color */
#define DLGMAN_TEXTINPUT_TEXTCOL        0x000000 /* text input box text color */

#define DLGMAN_DIALOG_DEFAULTITLE       "Title bar" /* default title */

#define DLGMAN_DIALOG_ASKEXIT "Do you really wish to quit?"
#define DLGMAN_DIALOG_ASKEXITITLE "MAD"

#define DLGMAN_DEFAULT_SPR0EXT          "0.spr"
#define DLGMAN_DEFAULT_SPR1EXT          "1.spr"
#define DLGMAN_DEFAULT_SPR2EXT          "2.spr"
#define DLGMAN_DEFAULT_SPR3EXT          "3.spr"

#define DLGMAN_DEFAULT_BACKEXT          ".spr"

class CONTROL {
public:
    CONTROL();

    void   init(_UCHAR);              /* initialize the control */
    void   done();                    /* deinitialize the control */

    void   move(_SINT,_SINT);         /* move the control */
    void   resize(_UINT,_UINT);       /* resize the control */
    void   setcaption(char*);         /* set the caption */
    void   setflags(_UINT);           /* set the flags */
    void   setstate(_UCHAR);          /* set new state */
    void   setfocus(_UCHAR);          /* give it the focus */
    void   setcolor(_ULONG);          /* set color of the control */

    void   additem(char*);            /* add an item to the list */

    void   draw(char*,_SINT,_SINT);   /* draw the control */

    char*  getcaption();              /* return the caption of the control */
    void   getitem(_UINT,char*);      /* list: get item name */

    void   handlevent(_UINT,_UINT,_UINT);   /* take care of events */

    void   loadsprite(char*);         /* sprite: load a sprite to use */
    void   loadanim(char*);           /* animation: load an animation to use */

    void   setspriteno(_ULONG);       /* animation: set sprite number */

    _UINT finditem(char*);            /* list: find an item */
    _UINT getcuritem();               /* list: get currently selected item */

    _UINT  getflags();                /* get the flags */
    _UCHAR getype();                  /* get the control type */
    _UCHAR istouched(_UINT,_UINT);    /* is control touched? */

    _UCHAR getstate();                /* get the state */
    _UCHAR hasfocus();                /* do we have focus? */

private:
    _UCHAR type;                      /* type of the control */

    _UINT  flags;                     /* my flags */

    _SINT  x,y;                       /* coordinates */
    _UINT  height,width;              /* height and width */
    _UCHAR state;                     /* state of the control */
    _UCHAR focus;                     /* do we have the focus? */
    _ULONG color;

    _UCHAR curitem;                   /* list: currently selected item */
    _UINT  nofitems;                  /* list: number of items */
    _UINT  list_flags;                /* list: flags */
    _UINT  baseno;                    /* list: first item listed */

    _UINT  spriteno;                  /* sprite: sprite number (or animation sprite no) */
    _UINT  animno;                    /* animation: animation no to use */

    char   caption[DLGMAN_CONTROL_MAXCAPTIONLEN];
};

class DIALOG {
public:
    DIALOG();

    void   init(_UINT,_SINT,_SINT,_UINT,_UINT);
    void   done();

    void   draw(char*);
    void   centerdialog();

    _UINT  createcontrol(_UCHAR);

    _SINT  getx();
    _SINT  gety();
    _UINT  getheight();
    _UINT  getwidth();

    _UINT  getflags();
    _UINT  getcontroltouched(_UINT,_UINT);

    _UINT  cyclefocus(_UINT);

    void   setitle(char*);
    char*  getitle();

    void   move(_SINT,_SINT);
    void   resize(_UINT,_UINT);

    void   loadbackdrop(char*);
    void   loadborder(char*,char*,char*,char*);

    class  CONTROL *control[DLGMAN_MAX_CONTROLS];

    _UINT  backdrop;                  /* backdrop sprite or GFX_NOSPRITE */

    _UINT  border[DLGMAN_DIALOG_NOFBORDERS];

private:
    void   drawbackdrop(char*);
    void   drawborders(char*);

    _SINT  x,y;                       /* coordinates */
    _UINT  height,width;              /* height and width */
    _UINT  flags;                     /* flags */

    char*  title;
};

class DLGMAN {
public:
    DLGMAN();

    void   init();
    void   done();

    _UINT  createdialog(_UINT,_SINT,_SINT,_UINT,_UINT);
    void   destroydialog(_UINT);

    void   drawdialog(_UINT,_SINT,_SINT,char*);

    _UCHAR dodialog(_UINT);

    void   messagebox(char*,char*);
    _UCHAR askyesno(char*,char*);
    void   showmessage(char*);
    void   alert(_ULONG);

    class  DIALOG *dialog[DLGMAN_MAX_DIALOGS];

    _UINT  default_border[DLGMAN_DIALOG_NOFBORDERS];
    _UINT  default_backdrop;

private:
    _UINT  flags;
};
#endif /* __DLGMAN_INCLUDED__ */
