#include "anm.h"
#include "archive.h"
#include "script.h"
#include "types.h"

#define  OBJMAN_MAXOBJECTS     63       /* maximum number of objects */
#define  OBJMAN_MAX_ACTIONS    8        /* maximum actions bound */
#define  OBJMAN_MAX_MOVESEQS   9        /* maximum number of movement sequences */

#define  OBJMAN_NO_ACTION      0xfffffffe /* no action */
#define  OBJMAN_ACTION_INHERIT 0xffffffff /* inherit action */

#define  OBJMAN_NOBJECT        0xffff   /* returned if no object is available */

#define  OBJMAN_TYPE_UNUSED        0    /* unused object */
#define  OBJMAN_TYPE_SPRITE        1    /* sprite object */
#define  OBJMAN_TYPE_ANM           2    /* animatable object */
#define  OBJMAN_TYPE_TEXT          3    /* text object */
#define  OBJMAN_TYPE_VARIABLE      4    /* variable object */

#define  OBJMAN_MAX_TEXT_LEN       1024 /* maximum length of a text */

#define  OBJMAN_PROP_X             0    /* x */
#define  OBJMAN_PROP_Y             1    /* y */
#define  OBJMAN_PROP_FRAMENO       2    /* frame number */
#define  OBJMAN_PROP_MOVING        3    /* is moving flag */
#define  OBJMAN_PROP_ANIMATING     4    /* is animating flag */
#define  OBJMAN_PROP_PRIOCODE      5    /* priority code */
#define  OBJMAN_PROP_ROOMNO        6    /* room number */
#define  OBJMAN_PROP_OLDROOMNO     7    /* old room number */
#define  OBJMAN_PROP_DIALOGCHOICE  8    /* dialog choice */

#define  OBJMAN_PROP_USERDEFINED   0x80 /* user defined properties */

struct OBJECT {
    _SINT  x,y;                         /* x and y coordinates */
    _UINT  sourceno;                    /* data source number */
    _UINT  frameno;                     /* current frame number */
    _UCHAR move;                        /* nonzero if the object must move */
    _UCHAR animating;                   /* nonzero if the object is animating */
    _UCHAR base_priority;               /* base priority */
    _ULONG roomno;                      /* room number */
    _ULONG oldroomno;                   /* old room number */

    /* general variables */
    _UCHAR visible;                     /* nonzero if visible */
    _UCHAR internal;                    /* nonzero if an internal object */
    _UCHAR enabled;                     /* nonzero if enabled */
    _UCHAR prioritycode;                /* current priority code */

    /* variables only applicable for animations */
    _UCHAR animno;                      /* animation number */
    _UINT  oldframeno;                  /* old frame no (automagically set) */
    _UINT  oldanimno;                   /* old animation no (automagically set) */
    _UINT  move_sequence[OBJMAN_MAX_MOVESEQS];            /* numbers of move sequences */
    _SINT  destx,desty;                 /* destination x and y */

    _UINT  speed;                       /* speed of the animation */
    _UINT  cur_wait;                    /* wait value */
    _UINT  h_speed,v_speed;             /* increment values */
    _UCHAR last_direction;
    _UCHAR looping;                     /* nonzero if object must loop */

    _UCHAR move_dir_h;                  /* zero if no move horizontal, otherwise non zero */
    _UCHAR move_dir_v;                  /* zero if no move vertical, otherwise non zero */

    _UCHAR mask_dir;                    /* direction to check masking */
    _UCHAR clip;                        /* nonzero if object must be clipped */

    _SINT  yank_x,yank_y;               /* used by refresh() */
    _SINT  ego_x,ego_y;
    _SINT  old_x,old_y;                 /* old coordinates */

    _ULONG color;                       /* color of the text */

    _UCHAR dirty;                       /* dirty flag */

    char   name[SCRIPT_MAXIDENTLEN];    /* object name */
    char   datafile[ARCHIVE_MAXFILENAMELEN];   /* file containing the data */
    char   text[OBJMAN_MAX_TEXT_LEN];   /* text */
    _UCHAR type;                        /* object type */

    _UCHAR alignment;                   /* text alignment */

    _ULONG action[OBJMAN_MAX_ACTIONS];  /* actions */

    _ULONG userprop[0xff-OBJMAN_PROP_USERDEFINED]; /* user defined property */

    _UCHAR nonotify;                    /* if non-zero, it will never be sent to other players */
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
        void   setcolor(_UINT,_ULONG);
        void   setextobject(_UINT objno,char*,char*);
        void   unloadtext(_UINT);
        void   setdirty(_UINT,_UCHAR);
        void   setoldxy(_UINT,_SINT,_SINT);
        void   setroomno(_UINT,_ULONG);
        void   setalign(_UINT,_UCHAR);

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
        _UCHAR getoldframeno(_UINT);
        _UCHAR getoldanimno(_UINT);
        _UCHAR getalign(_UINT);

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
        _ULONG getroomno(_UINT);
        _ULONG getoldroomno(_UINT);
        _ULONG getaction(_UINT,_UINT);
        _ULONG getcolor(_UINT);

        char*  getobjectname(_UINT);
        char*  getext(_UINT);
        char*  getdatafile(_UINT);

        _UINT  findobject(char*);
        _UINT  findfreeobject();

        void   set_property(_UINT,_UINT,_ULONG);
        _ULONG get_property(_UINT,_UINT);

        void   bindaction(_UINT,_UINT,_ULONG);

        void   setnonotify(_UINT,_UCHAR);

        OBJECT* getobjectptr(_ULONG);

private:
        OBJECT object[OBJMAN_MAXOBJECTS];
};
