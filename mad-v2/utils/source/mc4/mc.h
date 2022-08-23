/*
 *                             The MAD Compiler
 *                                Version 4.0
 *
 *                          (c) 1999 The MAD Crew
 *
 *                             Main include file
 *
 */
#include "../../../source/types.h"
#include "../../../source/script.h"

#define MC_MAX_SCRIPT_PAIRS             8          /* Maximum number of splitted pairs */
#define MC_MAX_PAIR_LENGTH              128        /* Maximum length of a pair */

#define MC_MESSAGELEVEL_DEFAULT         0          /* Default message level */
#define MC_MESSAGELEVEL_SILENT          1          /* Silent mode */
#define MC_MESSAGELEVEL_VERBOSE         2          /* Verbose mode */

#define MC_BASE_SCRIPTCODE_SIZE         128        /* Default compiled code size */

#define MC_MAX_PROCNAME_LEN             16         /* Maximum length of a procedure name */
#define MC_MAX_LABELNAME_LEN            16         /* Maximum length of a label */
#define MC_MAX_OBJECT_LEN               16         /* Maximum length of an object name */
#define MC_MAX_SCRIPTNAME_LEN           128        /* Maximum length of the script's name */
#define MC_MAX_SUPERCLASS_LEN           16         /* Maximum length of a superclass's name */
#define MC_MAX_METHOD_LEN               16         /* Maximum length of a method */

#define MC_FLAG_INTERNAL                1          /* internal object */
#define MC_FLAG_EXTERNAL                2          /* external object */
#define MC_FLAGS_DEFAULT                0          /* default flags */

#define MC_METHOD_FLAG_HASCODE          8192       /* set if the method has code */

#define MC_EXT_CM                       ".cm"      /* extension of a compiled script */

#define MC_NOPROC                       0xffffffff /* No procedure */
#define MC_NOLABEL                      0xffffffff /* No label */
#define MC_NOLINE                       0L         /* No line */
#define MC_NOBJECT                      0xffffffff /* No object */
#define MC_NOENTRY                      0xffff     /* No entry */
#define MC_NOCLASS                      0xffffffff /* No superclass */
#define MC_NOMETHOD                     0xffffffff /* No method */
#define MC_NOFFSET                      0xffffffff /* No offset */

#define MC_DEFINE_OBJECT                "object"   /* defines an object */
#define MC_DEFINE_CLASSDEF              "classdef" /* defines a class */
#define MC_DEFINE_STARTPROC             "{"        /* defines the beginning of a proc. */
#define MC_DEFINE_ENDPROC               "}"        /* defines the end of a proc. */
#define MC_INCLUDE_FILE                 "#include" /* includes a file */

#define MC_TYPE_STRING      0                      /* ascii string */
#define MC_TYPE_UINT        1                      /* unsigned integer */
#define MC_TYPE_SINT        2                      /* signed integer */
#define MC_TYPE_EUINT       3                      /* evaluated unsigned integer */
#define MC_TYPE_QSTRING     4                      /* quoted string */
#define MC_TYPE_DIRECTION   5                      /* direction */
#define MC_TYPE_BOOLEAN     6                      /* boolean */
#define MC_TYPE_TEXTBLOCK   7                      /* text block */
#define MC_TYPE_LABEL       8                      /* label */
#define MC_TYPE_DWORD       9                      /* double word */
#define MC_TYPE_UNKNOWN     0xffff                 /* unresolvable */

#define MC_STATUS_NORMAL    0                      /* nothing special */
#define MC_STATUS_OBJECT    1                      /* doing objectdefs */
#define MC_STATUS_CLASSDEF  2                      /* doing classdefs */
#define MC_STATUS_CODE      3                      /* doing code */

#define MC_OBJDEF_END       "}"                    /* end of an object declaration */
#define MC_PROC_END         "}"                    /* end of procedure */

#define MC_SCLASS_PROC      0                      /* it's a procedure! */

struct MC_COMMAND {
    char*  name;                                   /* command name */
    _UCHAR opcode;                                 /* script opcode */
    char*  parms;                                  /* parameters, NULL if none */
    _ULONG flags;                                  /* command flags */
};

struct MC_PROPERTY {
    char*  name;                                   /* property name */
    _UCHAR code;                                   /* property code */
};

struct MC_SPECIAL {
    char*  name;                                   /* special name */
    _UCHAR code;                                   /* special code */
    char*  parms;                                  /* special parms */
};

struct MC_METHOD {
    char    name[MC_MAX_METHOD_LEN];               /* method name */
    _ULONG  flags;                                 /* method flags */
    _ULONG  start_byte;                            /* start of code */
    _ULONG  end_byte;                              /* end of code */
    _UCHAR  in_use;                                /* is code in use? */
};

struct MC_SUPERCLASS {
    char       name[MC_MAX_SUPERCLASS_LEN];        /* superclass name */
    _ULONG     nofmethods;                         /* number of methods */
    MC_METHOD* methods;                            /* methods */
};

struct MC_OBJECT {
    char    name[MC_MAX_OBJECT_LEN];               /* object name */
    _ULONG  nofmethods;                            /* number of methods */
    char*   methods;                               /* methods */
    _ULONG  superclass;                            /* superclass */
};

extern _UCHAR mc_message_level;
