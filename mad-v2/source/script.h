#include "debugmad.h"
#include "mad.h"
#include "types.h"

/* are we already included? */
#ifndef __SCRIPT_INCLUDED__
/* no. set flag we are included, and define things */
#define __SCRIPT_INCLUDED__

#define SCRIPT_MAGICNO  0x21524353              /* SCR! */
#define SCRIPT_VERSIONO 200                     /* version 2.0 */

#define SCRIPT_MAXPROCS		63		/* maximum number of procedures */
#define SCRIPT_MAXIDENTLEN	31		/* maximum size of identifier */
#define SCRIPT_MAX_FILENAME_LEN 16             	/* maximum length of a filename */

#define SCRIPT_NOIDENT          0xffff          /* no identifier found */

/* internal object names */
#define SCRIPT_MOUSE_OBJECTNAME "mouse"         /* mouse object name */
#define SCRIPT_BAR_OBJECTNAME   "bar"           /* bar object name */
#define SCRIPT_CONTROLS_OBJECTNAME "controls"   /* controls object name */
#define SCRIPT_SYSTEM_OBJECTNAME "system"       /* system object name */
#define SCRIPT_SCRIPT_OBJECTNAME "script"       /* script object name */
#define SCRIPT_DIALOG_OBJECTNAME "dialog"       /* dialog object name */

#define SCRIPT_PROC_DODIALOG_DONE "dialog_done"

/* array indexes */
#define SCRIPT_MOVE_WEST_NO     0               /* array index when with west move */
#define SCRIPT_MOVE_EAST_NO     1               /* array index when with east move */
#define SCRIPT_MOVE_NORTH_NO    2               /* array index when with north move */
#define SCRIPT_MOVE_SOUTH_NO    3               /* array index when with south move */
#define SCRIPT_MOVE_HALT        4               /* array idnex when halted */

/* opcodes */
#define OPCODE_LOADSCREEN     0x10
#define OPCODE_REFRESH        0x11
#define OPCODE_RETURN         0x12
#define OPCODE_DELAY          0x13
#define OPCODE_JUMP           0x14
#define OPCODE_IF             0x15
#define OPCODE_WHILE          0x16
#define OPCODE_LEAVE          0x17
#define OPCODE_NOP            0x18
#define OPCODE_CALL           0x19

#define OPCODE_SETVAR         0x20

#define OPCODE_OBJECT         0x30

#define OPCODE_OBJ_SETANIM    0x10
#define OPCODE_OBJ_SETFRAME   0x11
#define OPCODE_OBJ_SETMOVE    0x12
#define OPCODE_OBJ_MOVETO     0x13
#define OPCODE_OBJ_SETSPEED   0x14
#define OPCODE_OBJ_SETHALT    0x15
#define OPCODE_OBJ_FACE       0x16
#define OPCODE_OBJ_SETLOOP    0x17
#define OPCODE_OBJ_SETMASK    0x18
#define OPCODE_OBJ_SETCLIP    0x19
#define OPCODE_OBJ_STOP       0x1a
#define OPCODE_OBJ_ANIMATE    0x1b

#define OPCODE_OBJ_MOVE       0x20
#define OPCODE_OBJ_SHOW       0x21
#define OPCODE_OBJ_HIDE       0x22
#define OPCODE_OBJ_DESTROY    0x23
#define OPCODE_OBJ_UNLOAD     0x24
#define OPCODE_OBJ_ENABLE     0x25
#define OPCODE_OBJ_DISABLE    0x26
#define OPCODE_OBJ_LOADSPR    0x27
#define OPCODE_OBJ_ACTIVATE   0x28
#define OPCODE_OBJ_SETPRI     0x29
#define OPCODE_OBJ_TELEPORT   0x2a

#define OPCODE_OBJ_EXIT       0x30
#define OPCODE_OBJ_ADDICON    0x31
#define OPCODE_OBJ_LOAD       0x32
#define OPCODE_OBJ_GO         0x33
#define OPCODE_OBJ_DISPOSE    0x34
#define OPCODE_OBJ_SETMOVEOBJ 0x35
#define OPCODE_OBJ_DARKENPAL  0x36
#define OPCODE_OBJ_RESTOREPAL 0x37

#define OPCODE_OBJ_SETTEXT    0x38
#define OPCODE_OBJ_SETCOLOR   0x39

#define OPCODE_OBJ_PROMPTEXIT 0x3a
#define OPCODE_OBJ_FADEOUT    0x3b
#define OPCODE_OBJ_FADEIN     0x3c

#define OPCODE_OBJ_LOADTEXT   0x3d

#define OPCODE_OBJ_CREATETXTDLG 0x40
#define OPCODE_OBJ_LOADBCKDRP   0x41
#define OPCODE_OBJ_LOADBORDER   0x42
#define OPCODE_OBJ_DODIALOG     0x43
#define OPCODE_OBJ_SHOWMESSAGE  0x44
#define OPCODE_OBJ_CONVERSE     0x45

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

#define DIR_WEST        0
#define DIR_EAST        1
#define DIR_NORTH       2
#define DIR_SOUTH       3
#define DIR_NONE        0xfe
#define DIR_UNKNOWN     0xff

#define SCRIPT_NOBREAKPOINT 0xff                /* there's no breakpoint */
#define SCRIPT_NOBREAKPOINTADDRESS 0xffffffff   /* no breakpoint address */

#define SCRIPT_EVENTSEPERATOR   "_"             /* event seperator */

struct SCRIPTHEADER {
    _ULONG magicno;                              /* should be SCRIPT_MAGICNO */
    _UINT  version;                              /* version number (100 = 1.00, 123 = 1.23 etc.) */
    _ULONG script_size;                          /* size of the script code */
    _UINT  nofprocs;                             /* number of procedures */
    _UCHAR reserved[16];                         /* unused */
};

struct PROCHEADER {
    char   name[SCRIPT_MAXIDENTLEN];             /* identifier name */
    _ULONG start_pos;
    _ULONG end_pos;
};

class SCRIPT {
public:
    SCRIPT();									/* constructor */

    void init(char *fname);
    void done();
    void parse();

    char* getfilename();
    void goproc(char *procname);
    _UCHAR proc_exists(char *procname);

    _ULONG geteip();
    _UCHAR mainscriptactive();
    void   seteip(_ULONG);

    _UCHAR mainactive;                           /* is main script active? */

    #ifdef DEBUG_AVAILABLE
    _ULONG disassemble(_ULONG,char*,_UCHAR*);
    _ULONG getscriptsize();
    void   changescriptcode(_ULONG,_UCHAR);

    _UCHAR addbreakpoint(_ULONG);
    _UCHAR isbreakpoint(_ULONG);
    _ULONG getbreakpoint(_UCHAR);
    void   clearbreakpoint(_UCHAR);

    _ULONG getnofprocs();
    PROCHEADER* getprocinfo(_ULONG);
    #endif

    _UINT  dialogno;                             /* dialog for the script */

private:

    _UCHAR handleopcode();                       /* do current instruction */
    void handleobjectopcode();                  /* do current object instruction */

    struct SCRIPTHEADER header;                 /* header */
    char *scriptcode;			        /* pointer to script code */

    _UINT findproc(char *name);
    _UCHAR getbyte();
    _UINT getword();
    _ULONG getdword();
    void getstring(char *dest,_UCHAR maxlen);

    _ULONG getval(_UCHAR valtype,_ULONG valno,_UCHAR *error);
    _ULONG getevalue();
    _UCHAR expressiontrue(_UCHAR,_UINT,_UINT);

    struct PROCHEADER proc[SCRIPT_MAXPROCS];	/* procedure data */
    _ULONG scriptcodesize;                       /* size of the script code */

    #ifdef DEBUG_AVAILABLE
        void   resolvevalue(char*);

        char*  resolvecondition(_UCHAR condition);
        void   resolvevar(_UCHAR,_UINT,char*);
        void   resolvexpression(char*);
        void   disassembleobj(char*,_UINT);

        _ULONG breakpoint[DEBUG_MAX_BREAKPOINTS];       /* breakpoint addresses */
    #endif

    char   filename[SCRIPT_MAX_FILENAME_LEN];

    _UCHAR running;                              /* running flag */
    _ULONG eip;                                  /* ip of this script */
};
#endif
