#include "anm.h"
#include "archive.h"
#include "script.h"
#include "types.h"

#define  OBJMAN_MAXOBJECTS     63       /* maximum number of objects */

#define  OBJMAN_MOUSE_OBJNO    0        /* internal mouse object */
#define  OBJMAN_BAR_OBJNO      1        /* internal bar object */
#define  OBJMAN_CONTROLS_OBJNO 2        /* internal control object */
#define  OBJMAN_SYSTEM_OBJNO   3        /* internal system object */
#define  OBJMAN_SCRIPT_OBJNO   4        /* internal script object */
#define  OBJMAN_PARSER_OBJNO   5        /* internal parser object */
#define  OBJMAN_DIALOG_OBJNO   6        /* internal dialog object */

#define  OBJMAN_NOBJECT        0xffff   /* returned if no object is available */

#define  OBJMAN_TYPE_UNUSED        0    /* unused object */
#define  OBJMAN_TYPE_SPRITE        1    /* sprite object */
#define  OBJMAN_TYPE_ANM           2    /* animatable object */
#define  OBJMAN_TYPE_TEXT          3    /* text object */
#define  OBJMAN_TYPE_VARIABLE      4    /* variable object */

/* internal object names */
#define  OBJMAN_MOUSE_OBJECTNAME    "mouse"     /* mouse object name */
#define  OBJMAN_BAR_OBJECTNAME      "bar"       /* bar object name */
#define  OBJMAN_CONTROLS_OBJECTNAME "controls"  /* controls object name */
#define  OBJMAN_SYSTEM_OBJECTNAME   "system"    /* system object name */
#define  OBJMAN_SCRIPT_OBJECTNAME   "script"    /* script object name */
#define  OBJMAN_PARSER_OBJECTNAME   "parser"    /* parser object name */
#define  OBJMAN_DIALOG_OBJECTNAME   "dialog"    /* dialog object name */

#define  OBJMAN_MAX_TEXT_LEN        64          /* maximum length of a text */

struct OBJECT {
        /* general variables */
        _UCHAR type;                     /* object type */
        _UCHAR visible;                  /* nonzero if visible */
        _UCHAR internal;                 /* nonzero if an internal object */
        _UCHAR enabled;                  /* nonzero if enabled */
        char name[SCRIPT_MAXIDENTLEN];   /* object name */
        _SINT  x,y;                      /* x and y coordinate */
        _ULONG sourceno;                 /* sprite/animation number or value */
        _UCHAR prioritycode;             /* current priority code */

        _UCHAR base_priority;            /* base priority */

        /* variables only applicable for animations */
        _UCHAR animno;                   /* animation number */
        _UCHAR frameno;                  /* current frame number */
        _UINT  oldframeno;               /* old frame no (automagically set) */
        _UINT  oldanimno;                /* old animation no (automagically set) */
        _UINT  move_sequence[5];         /* numbers of move sequences */
        _SINT  destx,desty;              /* destination x and y */
        _UCHAR move;                     /* nonzero if the object must move */
        _UCHAR animating;                /* nonzero if the object is animating */

        _UINT  speed;                    /* speed of the animation */
        _UINT  cur_wait;                 /* wait value */
        _UINT  h_speed,v_speed;          /* increment values */
        _UCHAR last_direction;
        _UCHAR looping;                  /* nonzero if object must loop */

        _UCHAR move_dir_h;               /* zero if no move horizontal, otherwise non zero */
        _UCHAR move_dir_v;               /* zero if no move vertical, otherwise non zero */

        _UCHAR mask_dir;                 /* direction to check masking */
        _UCHAR clip;                     /* nonzero if object must be clipped */

        _SINT  yank_x,yank_y;            /* used by refresh() */
        _SINT  ego_x,ego_y;
        _SINT  old_x,old_y;              /* old coordinates */

        char   datafile[ARCHIVE_MAXFILENAMELEN];   /* file containing the data */
        char   text[OBJMAN_MAX_TEXT_LEN];/* text */
        _UCHAR color;                    /* color of the text */

        _UCHAR dirty;                    /* dirty flag */
};

class OBJMAN {
public:
        void   init();
        void   done();

        void   destroyobject(_UINT);
        void   unloadobject(_UINT);

        void   setvisible(_UINT,_UCHAR);
        void   setenabled(_UINT,_UCHAR);
        void   setmove(_UINT,_UCHAR);
        void   setcoords(_UINT,_SINT,_SINT);
        void   setdestcoords(_UINT,_SINT,_SINT);
        void   setprioritycode(_UINT,_UCHAR);
        void   setlastdirection(_UINT,_UCHAR);
        void   setcurwait(_UINT,_UINT);
        void   setspeed(_UINT,_UINT);
        void   setmovesequence(_UINT,_UCHAR,_UINT);
        void   setanimno(_UINT,_UCHAR);
        void   setframeno(_UINT,_UCHAR);
        void   setyankcoords(_UINT,_SINT,_SINT);
        void   setanimating(_UINT,_UCHAR);
        void   setclipping(_UINT,_UCHAR);
        void   setmaskdir(_UINT,_UCHAR);
        void   setobjectspeed(_UINT,_UINT,_UINT,_UINT);
        void   setsourceno(_UINT,_ULONG);
        void   setbasepriority(_UINT,_UCHAR);
        void   setlooping(_UINT,_UCHAR);
        void   setanmobject(_UINT,char*,char*);
        void   setdirection(_UINT,_UCHAR);
        void   setcolor(_UINT,_UCHAR);
        void   setextobject(_UINT objno,char*,char*);
        void   unloadtext(_UINT);
        void   setdirty(_UINT,_UCHAR);
        void   setoldxy(_UINT,_SINT,_SINT);

        _UCHAR isenabled(_UINT);
        _UCHAR isvisible(_UINT);
        _UCHAR isdirty(_UINT);
        _UCHAR isinternal(_UINT);
        _UCHAR getype(_UINT);
        _UCHAR getframeno(_UINT);
        _UCHAR getanimno(_UINT);
        _UCHAR objmustmove(_UINT);
        _UCHAR getlastdirection(_UINT);
        _UCHAR getbasepriority(_UINT);
        _UCHAR isanimating(_UINT);
        _UCHAR islooping(_UINT);
        _UCHAR isclipping(_UINT);
        _UCHAR getmaskdir(_UINT);
        _UCHAR getprioritycode(_UINT);
        _UCHAR setext(_UINT,char*);
        _UCHAR getcolor(_UINT);
        _UCHAR getoldframeno(_UINT);
        _UCHAR getoldanimno(_UINT);

        _UINT  getobjecthspeed(_UINT);
        _UINT  getobjectvspeed(_UINT);
        _UINT  getcurwait(_UINT);
        _UINT  getspeed(_UINT);
        _UINT  getmovesequence(_UINT,_UCHAR);

        _SINT  getobjectx(_UINT);
        _SINT  getobjecty(_UINT);
        _SINT  getobjectdestx(_UINT);
        _SINT  getobjectdesty(_UINT);
        _SINT  getobjectyankx(_UINT);
        _SINT  getobjectyanky(_UINT);
        _SINT  getobjectoldx(_UINT);
        _SINT  getobjectoldy(_UINT);

        _ULONG getsourceno(_UINT);

        char*  getobjectname(_UINT);
        char*  getext(_UINT);
        char*  getdatafile(_UINT);

        _UINT  findobject(char*);
        _UINT  findfreeobject();

        OBJECT* getobjectptr(_ULONG);

private:
        struct OBJECT object[OBJMAN_MAXOBJECTS];
};
