/*
 *                             The MAD Compiler
 *                                Version 4.0
 *
 *                       (c) 1999, 2000 The MAD Crew
 *
 *                             Main include file
 *
 */
#include "../../../source/include/types.h"
#include "../../../source/include/script.h"

#define MC_MAX_SCRIPT_PAIRS             8          /* Maximum number of splitted pairs */
#define MC_MAX_PAIR_LENGTH              128        /* Maximum length of a pair */

#define MC_MESSAGELEVEL_DEFAULT         0          /* Default message level */
#define MC_MESSAGELEVEL_SILENT          1          /* Silent mode */
#define MC_MESSAGELEVEL_VERBOSE         2          /* Verbose mode */

#define MC_BASE_SCRIPTCODE_SIZE         128        /* Default compiled code size */

#define MC_MAX_PROCNAME_LEN             16         /* Maximum length of a procedure name */
#define MC_MAX_LABELNAME_LEN            16         /* Maximum length of a label */
#define MC_MAX_OBJECT_LEN               16         /* Maximum length of an object name */
#define MC_MAX_SCRIPTNAME_LEN           128        /* Maximum length of the script's filename */
#define MC_MAX_SUPERCLASS_LEN           16         /* Maximum length of a superclass's name */
#define MC_MAX_METHOD_LEN               16         /* Maximum length of a method */
#define MC_MAX_FUNC_LEN                 16         /* Maximum length of a kernel function */
#define MC_MAX_PARAM_LEN                64         /* Maximum length of parameters */
#define MC_MAX_PROP_LEN                 16         /* Maximum length of a property name */

#define MC_EXT_CM                       ".cm"      /* extension of a compiled script */

#define MC_NOPROC                       0xffffffff /* No procedure */
#define MC_NOLABEL                      0xffffffff /* No label */
#define MC_NOLINE                       0L         /* No line */
#define MC_NOBJECT                      0xffffffff /* No object */
#define MC_NOENTRY                      0xffff     /* No entry */
#define MC_NOCLASS                      0xffffffff /* No superclass */
#define MC_NOMETHOD                     0xffffffff /* No method */
#define MC_NOFFSET                      0xffffffff /* No offset */
#define MC_NOFUNC                       0xffffffff /* No function */
#define MC_NOPROP                       0xffffffff /* No property */

#define MC_DEFINE_OBJECT                "object"   /* defines an object */
#define MC_DEFINE_CLASSDEF              "classdef" /* defines a class */
#define MC_DEFINE_STARTPROC             "{"        /* defines the beginning of a proc. */
#define MC_DEFINE_ENDPROC               "}"        /* defines the end of a proc. */
#define MC_DEFINE_FLAGS                 "flags"    /* defines flags */
#define MC_DEFINE_KERNELFUNC            "kernelfunc" /* defines the kernel function code to use */
#define MC_DEFINE_KERNELFUNCS           "kernelfuncs" /* defines the kernel functions */
#define MC_DEFINE_EXTERNAL              "external" /* defines something external */
#define MC_DEFINE_PROPERTY              "propno"   /* defines something external */
#define MC_DEFINE_SYSFUNCS              "sysfuncs" /* defines system functions */
#define MC_INCLUDE_FILE                 "#include" /* includes a file */

#define MC_PROPERTY_NULL                "NULL"     /* NULL property */

#define MC_IF                           "if"       /* if */
#define MC_WHILE                        "while"    /* while */

#define MC_TYPE_STRING      0                      /* ascii string */
#define MC_TYPE_ELONG       1                      /* evaluated unsigned integer */
#define MC_TYPE_QSTRING     2                      /* quoted string */
#define MC_TYPE_DIRECTION   3                      /* direction */
#define MC_TYPE_BOOLEAN     4                      /* boolean */
#define MC_TYPE_TEXTBLOCK   5                      /* text block */
#define MC_TYPE_LABEL       6                      /* label */
#define MC_TYPE_DWORD       7                      /* double word */
#define MC_TYPE_PROP        8                      /* property */
#define MC_TYPE_OBJPROP     9                      /* object+property */
#define MC_TYPE_UNKNOWN     0xffff                 /* unresolvable */

#define MC_FLAG_DEFAULT     0                      /* default flags */
#define MC_FLAG_EXTERNAL    1                      /* external object */

#define MC_FLAG_EXTERNALOBJ 32768                  /* bit of objname */

#define MC_TYPE_UNDEFINED   0xff                   /* undefined thing */
#define MC_TYPE_METHOD      0                      /* a normal method */
#define MC_TYPE_OPCODE      1                      /* opcode reference */

#define MC_STATUS_NORMAL    0                      /* nothing special */
#define MC_STATUS_OBJECT    1                      /* doing objectdefs */
#define MC_STATUS_CLASSDEF  2                      /* doing classdefs */
#define MC_STATUS_CODE      3                      /* doing code */
#define MC_STATUS_KERNELDEF 4                      /* doing kernel definitions */
#define MC_STATUS_SYSFUNCS  5                      /* doing system functions */

#define MC_OBJDEF_END       "}"                    /* end of an object declaration */
#define MC_PROC_END         "}"                    /* end of procedure */

#define MC_HEADER_MAGIC     0x52435343             /* magic! */
#define MC_HEADER_VERSIONO  400                    /* version number */

#define MC_BOOL_FALSE       0                      /* no! */
#define MC_BOOL_TRUE        1                      /* yes! */
#define MC_BOOL_UNKNOWN     0xff                   /* what the heck does the user mean? */

#define MC_FLAG_KERNELFUNC  0x8000                 /* indicates a kernel function */
#define MC_FLAG_PROPERTY    0x4000                 /* indicates a property */

#define MC_CTYPE_IF          0                     /* if */
#define MC_CTYPE_WHILE       1                     /* while */
#define MC_CTYPE_UNKNOWN     0xff                  /* ??? */

#define MC_COMP_EQUAL        0                     /* equal? */
#define MC_COMP_NOTEQUAL     1                     /* not equal? */
#define MC_COMP_GE           2                     /* greater/equal? */
#define MC_COMP_LE           3                     /* lesser/equal? */
#define MC_COMP_GT           4                     /* greater than? */
#define MC_COMP_LT           5                     /* lesser than? */
#define MC_COMP_UNKNOWN      0xff                  /* ??? */

struct MC_COMMAND {
    char*  name;                                   /* command name */
    _UCHAR opcode;                                 /* script opcode */
    char*  parms;                                  /* parameters, NULL if none */
};

struct MC_PROPERTY {
    char   name[MC_MAX_PROP_LEN];                  /* property name */
    _UCHAR code;                                   /* property code */
};

struct MC_SPECIAL {
    char*  name;                                   /* special name */
    _UCHAR code;                                   /* special code */
    char*  parms;                                  /* special parms */
};

struct MC_METHOD {
    char    name[MC_MAX_METHOD_LEN];               /* method name */
    char    params[MC_MAX_PARAM_LEN];              /* parameters */
    _ULONG  flags;                                 /* method flags */
    _ULONG  start_byte;                            /* start of code */
    _UCHAR  type;                                  /* type */
    _UCHAR  opcode;                                /* opcode */
};

struct MC_SUPERCLASS {
    char         name[MC_MAX_SUPERCLASS_LEN];      /* superclass name */
    _ULONG       nofmethods;                       /* number of methods */
    _ULONG       nofproperties;                    /* number of properties */
    MC_METHOD*   methods;                          /* methods */
    MC_PROPERTY* properties;                       /* properties */
};

struct MC_OBJECT {
    char       name[MC_MAX_OBJECT_LEN];            /* object name */
    _ULONG     nofmethods;                         /* number of methods */
    MC_METHOD* methods;                            /* methods */
    _ULONG     superclass;                         /* superclass */
    _ULONG     flags;                              /* flags */
};

struct MC_FILEHEADER {
    _ULONG     magic;                              /* magic */
    _UINT      versiono;                           /* version number */
    _ULONG     codesize;                           /* script code size */
    _ULONG     datasize;                           /* script data size */
    _UINT      nofobjects;                         /* number of objects */
    _ULONG     dataoffset;                         /* data area offset */
    _ULONG     init_offset;                        /* initialization offset */
    _ULONG     done_offset;                        /* deinitalization offset */
    _ULONG     run_offset;                         /* run offset */
    char       reserved[6];                       /* unused */
};

struct MC_FUNC {
    char    name[MC_MAX_FUNC_LEN];                 /* method name */
    char    params[MC_MAX_PARAM_LEN];              /* parameters */
    _UCHAR  funcno;                                /* function number */
    _ULONG  flags;                                 /* flags */
};

extern _UCHAR mc_message_level;

_ULONG cmp_compileline(char*,_ULONG,_UINT*);
