/*
 *                             The MAD Compiler
 *                                Version 3.0
 *
 *                          (c) 1999 The MAD Crew
 *
 *                             Main include file
 *
 */
#include "../../../source/types.h"
#include "../../../source/script.h"

#define MC_MESSAGELEVEL_DEFAULT         0          /* Default message level */
#define MC_MESSAGELEVEL_SILENT          1          /* Silent mode */
#define MC_MESSAGELEVEL_VERBOSE         2          /* Verbose mode */

#define MC_MAX_SCRIPT_PAIRS             8          /* Maximum number of splitted pairs */
#define MC_MAX_PAIR_LENGTH              128        /* Maximum length of a pair */
#define MC_MAX_SCRIPTNAME_LEN           128        /* Maximum length of the script's name */
#define MC_MAX_FILENAME_LEN             64         /* Maximum length of a filename */
#define MC_MAX_STRINGLEN                512        /* Maximum length of a string */

#define MC_DEFAULT_CHANGESIZE           64         /* Default number of changes buffer size */
#define MC_DEFAULT_SCRIPTSIZE           512        /* Number of script size increments */
#define MC_DEFAULT_FIXESIZE             64         /* Default number of line fixes */
#define MC_DEFAULT_OBJNO                MC_NOBJECT /* Default object number */
#define MC_DEFAULT_LABELOFS             0xffffffff /* Default label offset */

#define MC_NOF_INTERNAL_OBJ             7          /* Number of internal objects */

#define MC_PROCSTART                    "{"        /* Procedure start char */
#define MC_PROCEND                      "}"        /* Procedure end char */
#define MC_DEFINE_VAR                   "var"      /* Variable definition */
#define MC_DEFINE_ANM                   "animation" /* Animation definition */
#define MC_DEFINE_EXTERNAL              "external" /* External definition */
#define MC_DEFINE_OBJECT                "object"   /* Object definition */
#define MC_DEFINE_TEXT                  "text"     /* Text definition */

#define MC_MOUSE_OBJNAME                "mouse"    /* name of mouse object */
#define MC_BAR_OBJNAME                  "bar"      /* name of bar object */
#define MC_CONTROLS_OBJNAME             "controls" /* name of controls object */
#define MC_SYSTEM_OBJNAME               "system"   /* name of system object */
#define MC_SCRIPT_OBJNAME               "script"   /* name of script object */
#define MC_PARSER_OBJNAME               "parser"   /* name of parser object */
#define MC_DIALOG_OBJNAME               "dialog"   /* name of dialog object */

#define MC_CHAR_LABEL                   ':'        /* Char to indicate a label */

#define MC_MAX_PROCNAME_LEN             16         /* Maximum length of a procedure name */
#define MC_MAX_LABELNAME_LEN            16         /* Maximum length of a label */
#define MC_MAX_OBJECT_LEN               16         /* Maximum length of an object name */

#define MC_FLAG_INTERNAL                1          /* internal object */
#define MC_FLAG_EXTERNAL                2          /* external object */
#define MC_FLAGS_DEFAULT                0          /* default flags */

#define MC_EXT_CM                       ".cm"      /* extension of a compiled script */

#define MC_NOPROC                       0xffffffff /* No procedure */
#define MC_NOLABEL                      0xffffffff /* No label */
#define MC_NOLINE                       0L         /* No line */
#define MC_NOBJECT                      0xffffffff /* No object */
#define MC_NOENTRY                      0xffff     /* No entry */

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

#define MC_STAT_NORMAL      0                      /* nothing special */
#define MC_STAT_OBJECT      1                      /* doing object */

struct MC_PROC {
    char    name[MC_MAX_PROCNAME_LEN];  /* proc name */
    _ULONG  startline;                  /* line where the proc starts */
    _ULONG  endline;                    /* line where the proc ends */

    _ULONG  startbyte;                  /* byte where the proc starts */
    _ULONG  endbyte;                    /* byte where the proc ends */
};

struct MC_LABEL {
    char    name[MC_MAX_LABELNAME_LEN]; /* name */
    _ULONG  line;                       /* line where the label is */

    _ULONG  offset;                     /* offset of the label in the script */

    _ULONG  maxfixes;                   /* maximum number of fixed allowed by [fixes] */
    _ULONG  nofixes;                    /* number of offsets that need to be fixed */
    _ULONG* fixes;                      /* offsets that need to be fixed */
};

struct MC_OBJECT {
    char    name[MC_MAX_OBJECT_LEN];           /* object name */
    char    sourcefile[MC_MAX_FILENAME_LEN];   /* source data file */

    _UCHAR  type;                              /* object type */
    _ULONG  flags;                             /* flags */
    _ULONG  curchangeofs;                      /* number of changes needed */
    _ULONG  maxchanges;                        /* change buffer size */
    _ULONG* change;                            /* change offset */
};

struct MC_COMMAND {
    char  *name;                               /* command name */
    _UCHAR opcode;                             /* script opcode */
    char  *parms;                              /* parameters, NULL if none */
    _ULONG flags;                              /* command flags */
};

struct MC_PROPERTY {
    char  *name;                               /* property name */
    _UCHAR code;                               /* property code */
};

struct MC_SPECIAL {
    char  *name;                               /* special name */
    _UCHAR code;                               /* special code */
    char *parms;                               /* special parms */
};

extern _UCHAR mc_message_level;

void cmp_add_param(_UINT,char*,_ULONG);
