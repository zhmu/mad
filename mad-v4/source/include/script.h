#include "debugmad.h"
#include "mad.h"
#include "types.h"

/* are we already included? */
#ifndef __SCRIPT_INCLUDED__
/* no. set flag we are included, and define things */
#define __SCRIPT_INCLUDED__

#define SCRIPT_MAGICNO  0x21524353              /* SCR! */
#define SCRIPT_VERSIONO 400                     /* version 4.0 */

#define SCRIPT_MAXPROCS		63		/* maximum number of procedures */
#define SCRIPT_MAXIDENTLEN	31		/* maximum size of identifier */
#define SCRIPT_MAX_FILENAME_LEN 16             	/* maximum length of a filename */

#define SCRIPT_NOIDENT          0xffff          /* no identifier found */

#define SCRIPT_NOADDRESS    0xffffffff          /* no address */
#define SCRIPT_NULLPROPERTY     0xffffffff      /* NULL property */

/* internal object names */
#define SCRIPT_MOUSE_OBJECTNAME "mouse"         /* mouse object name */
#define SCRIPT_BAR_OBJECTNAME   "bar"           /* bar object name */
#define SCRIPT_CONTROLS_OBJECTNAME "controls"   /* controls object name */
#define SCRIPT_SYSTEM_OBJECTNAME "system"       /* system object name */
#define SCRIPT_SCRIPT_OBJECTNAME "script"       /* script object name */
#define SCRIPT_DIALOG_OBJECTNAME "dialog"       /* dialog object name */

#define SCRIPT_PROC_DODIALOG_DONE "dialog_done"

#define SCRIPT_PROC_INIT        "init"          /* called when script must initialize itself */
#define SCRIPT_PROC_RUN         "run"           /* called as a script main loop */
#define SCRIPT_PROC_DONE        "done"          /* called when script must de-initialize itself */

#define CHECK_NOCHECK   0xff
#define CHECK_EQUAL     0
#define CHECK_NOTEQUAL  1
#define CHECK_GREATEREQ 2
#define CHECK_SMALLEREQ 3
#define CHECK_GREATER   4
#define CHECK_SMALLER   5

#define TYPE_NORESOLV         0xff
#define TYPE_STATIC           0
#define TYPE_VARIABLE         1
#define TYPE_OBJECT_X         2
#define TYPE_OBJECT_Y         3
#define TYPE_OBJECT_FRAMENO   4
#define TYPE_OBJECT_MOVING    5
#define TYPE_OBJECT_ANIMATING 6
#define TYPE_OBJECT_PRIORITY  7
#define TYPE_SPECIAL_RANDOM   8
#define TYPE_OBJECT_FINISHED  9
#define TYPE_OBJECT_ROOMNO    10
#define TYPE_OBJECT_OLDROOMNO 11
#define TYPE_OBJECT_CHOICE    12

#define EXPR_NONE             0
#define EXPR_ADD              1
#define EXPR_SUB              2
#define EXPR_MUL              3
#define EXPR_DIV              4

#define OBJTYPE_OBJ     0
#define OBJTYPE_VAR     1

#define SCRIPT_MOVE_SEPARATOR   "_"             /* char to add to separator move strings */
#define SCRIPT_MOVE_NORTH       "north"         /* name to add if moves north */
#define SCRIPT_MOVE_EAST        "east"          /* name to add if moves east */
#define SCRIPT_MOVE_SOUTH       "south"         /* name to add if moves south */
#define SCRIPT_MOVE_WEST        "west"          /* name to add if moves west */

#define SCRIPT_MOVE_NORTHEAST   "ne"            /* name to add if moves north */
#define SCRIPT_MOVE_NORTHWEST   "nw"            /* name to add if moves east */
#define SCRIPT_MOVE_SOUTHEAST   "se"            /* name to add if moves south */
#define SCRIPT_MOVE_SOUTHWEST   "sw"            /* name to add if moves west */

#define DIR_WEST        0
#define DIR_EAST        1
#define DIR_NORTH       2
#define DIR_SOUTH       3
#define DIR_NORTHEAST   4
#define DIR_NORTHWEST   5
#define DIR_SOUTHEAST   6
#define DIR_SOUTHWEST   7
#define DIR_HALT        8
#define DIR_NONE        0xfe
#define DIR_UNKNOWN     0xff

#define SCRIPT_NOBREAKPOINT 0xff                /* there's no breakpoint */
#define SCRIPT_NOBREAKPOINTADDRESS 0xffffffff   /* no breakpoint address */

#define SCRIPT_FLAG_ROOTSCRIPT  1               /* set if script is the root script */

#define SCRIPT_METHODFLAG_INIT  1               /* set if proc. is called upon initalization */
#define SCRIPT_METHODFLAG_DONE  2               /* set if proc. is called upon deinitalization */
#define SCRIPT_METHODFLAG_RUN   4               /* set if proc. is called every cycle */

#define SCRIPT_STACK_SIZE       32768           /* stack size (MUST be a multiple of 4!) */

#define SCRIPT_KFUNC_LEAVE      0               /* leave the script loop */
#define SCRIPT_KFUNC_INVOKE     1               /* invoke a script */
#define SCRIPT_KFUNC_EXIT       2               /* exit */
#define SCRIPT_KFUNC_LOADSCREEN 3               /* load a screen */
#define SCRIPT_KFUNC_REFRESH    4               /* refresh the screen */
#define SCRIPT_KFUNC_DELAY      5               /* delay */
#define SCRIPT_KFUNC_LOADPAL    6               /* load palette */
#define SCRIPT_KFUNC_RETURN     7               /* return */
#define SCRIPT_KFUNC_ADDICON    0x20            /* add bar icon */
#define SCRIPT_KFUNC_SHOWBAR    0x21            /* show icon bar */
#define SCRIPT_KFUNC_HIDEBAR    0x22            /* hide icon bar */
#define SCRIPT_KFUNC_BINDBAR    0x23            /* bind icon bar */
#define SCRIPT_KFUNC_SHOWTEXT   0x30            /* show text */
#define SCRIPT_KFUNC_CONVERSE   0x31            /* converse */
#define SCRIPT_KFUNC_EMPTYCHOICE 0x32           /* empty dialog choice */
#define SCRIPT_KFUNC_ALERT      0x33           /* alert */
#define SCRIPT_KFUNC_INV_ENABLE 0x40            /* enable inventory */
#define SCRIPT_KFUNC_INV_DISABLE 0x41           /* disable inventory */
#define SCRIPT_KFUNC_INV_GIVE   0x42            /* give inventory item */
#define SCRIPT_KFUNC_INV_DROP   0x43            /* drop inventory item */
#define SCRIPT_KFUNC_INV_SETQ   0x44            /* set inventory item quantity */
#define SCRIPT_KFUNC_INV_OPEN   0x45            /* open inventory window */
#define SCRIPT_KFUNC_INV_GETQ   0x46            /* get inventory item quantity */
#define SCRIPT_KFUNC_LOADCURSOR 0x50            /* load mouse cursor */
#define SCRIPT_KFUNC_RANDOM     0x80            /* random */
#define SCRIPT_KFUNC_DLGCHOICE  0x81            /* dialog choice */
#define SCRIPT_KFUNC_TOUCHED    0x82            /* touched by cursor */
#define SCRIPT_KFUNC_BREAK      0xff            /* debug break */

#define SCRIPT_OFUNC_SETANIM    1               /* set animation */
#define SCRIPT_OFUNC_SETFRAME   2               /* set frame */
#define SCRIPT_OFUNC_SETMOVE    3               /* set move sequence */
#define SCRIPT_OFUNC_MOVETO     4               /* move to a coordinate */
#define SCRIPT_OFUNC_SETSPEED   5               /* set movement speed */
#define SCRIPT_OFUNC_SETHALT    6               /* set halt sequence */
#define SCRIPT_OFUNC_FACE       7               /* face */
#define SCRIPT_OFUNC_SETLOOP    8               /* set looping value */
#define SCRIPT_OFUNC_SETMASK    9               /* set masking value */
#define SCRIPT_OFUNC_SETCLIP    0xa             /* set clipping value */
#define SCRIPT_OFUNC_STOP       0xb             /* stop */
#define SCRIPT_OFUNC_ANIMATE    0xc             /* animate */
#define SCRIPT_OFUNC_MOVE       0xd             /* move to a coordinate */
#define SCRIPT_OFUNC_SHOW       0xe             /* show */
#define SCRIPT_OFUNC_HIDE       0xf             /* hide */
#define SCRIPT_OFUNC_DESTROY    0x10            /* destroy */
#define SCRIPT_OFUNC_UNLOAD     0x11            /* unload */
#define SCRIPT_OFUNC_ENABLE     0x12            /* enable */
#define SCRIPT_OFUNC_DISABLE    0x13            /* disable */
#define SCRIPT_OFUNC_SETPRIO    0x14            /* set priority value */
#define SCRIPT_OFUNC_TELEPORT   0x15            /* teleport */
#define SCRIPT_OFUNC_LOADANM    0x16            /* load animation */
#define SCRIPT_OFUNC_SETMOVEOBJ 0x17            /* set move object */
#define SCRIPT_OFUNC_BINDACTION 0x18            /* bind an action */
#define SCRIPT_OFUNC_SETCOLOR   0x19            /* set color */
#define SCRIPT_OFUNC_LOADTEXT   0x1a            /* load text */
#define SCRIPT_OFUNC_SETALIGN   0x1b            /* set alignment */
#define SCRIPT_OFUNC_INITEXT    0x1c            /* init text */
#define SCRIPT_OFUNC_DLG_BINDRES 0x1d           /* bind dialog manager result */

struct SCRIPTHEADER {
    _ULONG magicno;                             /* should be SCRIPT_MAGICNO */
    _UINT  version;                             /* version number (100 = 1.00, 123 = 1.23 etc.) */
    _ULONG code_size;                           /* script code size */
    _ULONG data_size;                           /* script data size */
    _ULONG nofobjects;                          /* number of objects */
    _ULONG object_offset;                       /* offset of object info */
    _ULONG flags;                               /* flags */
    _ULONG init_offset;                         /* offset of init code */
    _ULONG done_offset;                         /* offset of done code */
    _ULONG run_offset;                          /* offset of run code */
    _UCHAR reserved[26];                        /* unused */
};

struct OBJECTHEADER {
    _ULONG nofmethods;                          /* number of methods */
    _UINT  obj_mapcode;                         /* object map code */
    _ULONG flags;                               /* object flags */
    char   reserved[4];                         /* unused */
};

struct SCRIPT_OBJECT {
    struct  OBJECTHEADER* hdr;                  /* object header */
    _ULONG* method;                             /* method offsets */
};

class SCRIPT {
public:
    SCRIPT();

    void init(char*,_UCHAR);                    /* initialize script */
    void go();                                  /* run the script */
    void go_offset(_ULONG);                     /* run a certain procedure */
    void go_method(_ULONG,_ULONG);              /* run a certain method */
    void done();                                /* deinitialize script */

    void setactive(_UCHAR);                     /* set active flag */
    _UCHAR isactive();                          /* get active flag */

    char* getfilename();                        /* get script filename or NULL */

    _ULONG get_a();                             /* get script a */
    _ULONG get_ip();                            /* get script ip */
    _UINT  get_sp();                            /* get script sp */
    _UCHAR get_flag();                          /* get flap */

    _ULONG get_code_size();                     /* get code size */

    _UINT  get_fake_objid(_UINT);               /* get the fake object id */

    #ifdef DEBUG_AVAILABLE
    _ULONG disassemble(_ULONG,char*);
    _ULONG get_tos_value();                     /* get top of stack value */
    void   forceleave();
    #endif /* DEBUG_AVAILABLE */

    _UINT  getdialogno();                       /* get dialog number */
    void   setdialogno(_UINT);                  /* set dialog number */    

private:
    _ULONG a;                                   /* accumulator register */
    _ULONG ip;                                  /* instruction pointer */
    _UINT  sp;                                  /* stack pointer */
    _UCHAR flag;                                /* flag */

    _UCHAR running;                             /* zero if running */

    _UCHAR active;                              /* non-zero if active */

    _UCHAR left;                                /* leave() used flag */

    _UCHAR force_left;                          /* if left enforced, it's non-zero */

    char*  stack;                               /* stack itself */

    struct SCRIPTHEADER* hdr;                   /* script header */
    char*  code;                                /* script code */
    char*  data;                                /* script data */

    char*  buffer;                              /* raw data */

    SCRIPT_OBJECT*  object;                     /* script objects */

    char   filename[SCRIPT_MAX_FILENAME_LEN];   /* script filename */

    void   push(_ULONG);                        /* push a value */
    _ULONG pop();                               /* pop a value */

    _UCHAR code_getbyte();                      /* grab the next code byte */
    _UINT  code_getword();                      /* grab the next code word */
    _ULONG code_getlong();                      /* grab the next code long */

    char*  data_getstring(_ULONG);              /* grab a data string */

    _UINT  get_real_objid(_UINT);               /* get the real object id */

    _ULONG get_property(_UINT,_UINT);           /* get an object property */
    void   set_property(_UINT,_UINT,_ULONG);    /* set an object property */

    void   call_kernel(_UCHAR);                 /* call kernel function */
    void   call_object(_UINT,_UCHAR);           /* call object function */

    void   run_instruction();                   /* handle the instruction at [ip] */

    _UINT  dialogno;                            /* dialog number */
    _ULONG dialog_result;                       /* dialog result */

    _UINT  dialog_objid;                        /* dialog object id */
    _ULONG dialog_resulthandler;                /* dialog result handler */
};
#endif
