/*
 *                             The MAD Compiler
 *                                Version 2.0
 *
 *                          (c) 1999 The MAD Crew
 *
 *                             Main include file
 *
 */
#include "../../../source/types.h"
#include "../../../source/script.h"

#define MC_MAX_OBJECT_LEN    16                          /* maximum length of an object name */
#define MC_MAX_CHANGES       256                         /* maximum number of offset changes allowed */
#define MC_MAX_FILENAME_LEN  64                          /* maximum length of a filename */
#define MC_MAX_PROCNAME_LEN  32                          /* maximum length of a procedure name */
#define MC_MAX_LABELNAME_LEN 32                          /* maximum length of a label name */

#define  MC_MAX_SCRIPTLINE_LEN   128
#define  MC_MAX_SCRIPT_LINES     255
#define  MC_MAX_SCRIPTLINE_PAIRS 8
#define  MC_MAX_PARAMSTR_LEN     16

#define MC_FLAG_INTERNAL  1                              /* internal object */
#define MC_FLAG_EXTERNAL  2                              /* external object */
#define MC_FLAGS_DEFAULT  0                              /* default flags */

#define MC_NOBJECT        0xffffffff                     /* returned if there is no object */
#define MC_NOPROC         0xffffffff                     /* returned if there is no procedure */
#define MC_NOLABEL        0xffffffff                     /* returned if there is no label */
#define MC_NOFIXLABEL     0xffffffff                     /* returned if there is no fix label */

#define MC_MAX_OBJECTS    64                             /* maximum number of objects possible */
#define MC_MAX_PROCS      64                             /* maximum number of procedures possible */
#define MC_MAX_LABELS     64                             /* maximum number of labels possible */
#define MC_MAX_FIXLABELS  128                            /* maximum number of label uses possible */

#define MC_EXPR_X         "x"                            /* x coordinate of object */
#define MC_EXPR_Y         "y"                            /* y coordinate of object */
#define MC_EXPR_FRAMENO   "frameno"                      /* get current frame number */
#define MC_EXPR_MOVING    "moving"                       /* is animation currently moving? */
#define MC_EXPR_ANIMATING "animating"                    /* is animation currently animating? */
#define MC_EXPR_PRIOCODE  "prioritycode"                 /* object priority code */
#define MC_EXPR_FINISHED  "finished"                     /* finished flag */
#define MC_EXPR_ROOMNO    "roomno"                       /* room number */
#define MC_EXPR_OLDROOMNO "oldroomno"                    /* old room number */
#define MC_EXPR_CHOICE    "dialogchoice"                 /* dialog choice */

#define MC_HEADER_VERSIONO 200                           /* script version 2.0 */
#define MC_HEADER_MAGICNO  0x52435343                    /* CSCR */

struct MC_OBJECT {
    char    name[MC_MAX_OBJECT_LEN];                     /* object name */
    char    sourcefile[MC_MAX_FILENAME_LEN];
    _UCHAR  type;                                        /* object type */
    _ULONG  flags;                                       /* flags */
    _ULONG  nofchanges;                                  /* number of changes needed */
    _ULONG  change[MC_MAX_CHANGES];                      /* change offset */
};

struct MC_PROC {
    char    name[MC_MAX_PROCNAME_LEN];                   /* proc name */
    _ULONG  startline;                                   /* line where the proc starts */
    _ULONG  endline;                                     /* line where the proc ends */

    _ULONG  startbyte;                                   /* byte where the proc starts */
    _ULONG  endbyte;                                     /* byte where the proc ends */
};

struct MC_LABEL {
    char    name[MC_MAX_LABELNAME_LEN];                  /* name */
    _ULONG  line;                                        /* line where the label is */

    _ULONG  offset;                                      /* offset of the label in the script */
};

struct MC_FIXLABEL {
    _ULONG  labelno;                                     /* which label it is */
    _ULONG  offset;                                      /* offset in the script */
};

struct MC_SCRIPTHEADER {
    _ULONG magicno;                                      /* script id code */
    _UINT  version;                                      /* version number (100 = 1.00, 201 = 2.01 etc.) */
    _ULONG script_size;                                  /* size of the script code */
    _UINT  nofprocs;                                     /* number of procedures */
    _UINT  nofobjects;                                   /* number of objects */
    _UCHAR reserved[16];                                 /* unused */
};

#define MC_DEFINE_VAR     "var"                          /* define variable */
#define MC_DEFINE_OBJ     "object"                       /* define object */
#define MC_DEFINE_EXTERN  "extern"                       /* define external variable */

#define MC_START_PROC     "{"                            /* start procedure */
#define MC_END_PROC       "}"                            /* end procedure */

#define MC_TYPE_ANM       "animation"                    /* animation */
#define MC_TYPE_TEXT      "text"                         /* text */
#define MC_TYPE_VAR       "var"                          /* variable */

#define MC_MOUSE_OBJNAME    "mouse"                      /* name of mouse object */
#define MC_BAR_OBJNAME      "bar"                        /* name of bar object */
#define MC_CONTROLS_OBJNAME "controls"                   /* name of controls object */
#define MC_SYSTEM_OBJNAME   "system"                     /* name of system object */
#define MC_SCRIPT_OBJNAME   "script"                     /* name of script object */
#define MC_PARSER_OBJNAME   "parser"                     /* name of parser object */
#define MC_DIALOG_OBJNAME   "dialog"                     /* name of dialog object */

#define MC_MAX_CSCRIPT_LEN  65535                        /* maximum length of a compiled script */

#define MC_MAX_COMMANDS     64                           /* maxium number of commands in table */

#define MC_TYPE_STRING      0                            /* ascii string */
#define MC_TYPE_UINT        1                            /* unsigned integer */
#define MC_TYPE_SINT        2                            /* signed integer */
#define MC_TYPE_EUINT       3                            /* evaluated unsigned integer */
#define MC_TYPE_QSTRING     4                            /* quoted string */
#define MC_TYPE_DIRECTION   5                            /* direction */
#define MC_TYPE_BOOLEAN     6                            /* boolean */
#define MC_TYPE_TEXTBLOCK   7                            /* text block */
#define MC_TYPE_LABEL       8                            /* label */
#define MC_TYPE_DWORD       9                            /* double word */
#define MC_TYPE_UNKNOWN     0xffff                       /* unresolvable */

#define MC_DEFAULT_OBJNO    0xffff                       /* default object number */
#define MC_DEFAULT_LABELOFS 0xffffffff                   /* default label offset */

#define MC_EXT_CSCRIPT      ".cm"                        /* compiled script extension */
#define MC_EXT_LSCRIPT      ".lm"                        /* linked script extension */

#define MC_BOOL_FALSE       0                            /* no! */
#define MC_BOOL_TRUE        1                            /* yes! */
#define MC_BOOL_UNKNOWN     0xff                         /* what the heck does the user mean? */

#define MC_COMMAND_IF       "if"                         /* if command */
#define MC_COMMAND_WHILE    "while"                      /* while command */

#define MC_CHAR_COMMENT     ';'                          /* comment char */

struct COMMAND {
    char  *name;                                         /* command name */
    _UCHAR opcode;                                       /* script opcode */
    char  *parms;                                        /* parameters, NULL if none */
    _UCHAR depriciated;                                  /* non-zero if command is depriciated */
};

struct COMMANDTAB {
    _UINT nofcommands;                                   /* number of commands in table */
    struct COMMAND command[MC_MAX_COMMANDS];             /* all commmands */
};
