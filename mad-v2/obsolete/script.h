#include "types.h"

/* are we already included? */
#ifndef __SCRIPT_INCLUDED__
/* no. set flag we are included, and define things */
#define __SCRIPT_INCLUDED__

#define SCRIPT_MAXLINESIZE      512             /* maximum size of a single line */
#define SCRIPT_MAXPROCS		63		/* maximum number of procedures */
#define SCRIPT_MAXLABELS        63              /* maximum number of labels */
#define SCRIPT_MAXVARS          63              /* maximum number of variables */
#define SCRIPT_MAXIDENTLEN	31		/* maximum size of identifier */

#define SCRIPT_MAINPROC         "main"          /* procedure which will be started first */

#define SCRIPT_LOADBACKGROUND   "loadbackground" /* load background picture */
#define SCRIPT_JUMP             "goto"          /* jump to another line */
#define SCRIPT_REDRAW           "refresh"       /* redraw the screen */
#define SCRIPT_ASSIGN           "set"           /* assign something */
#define SCRIPT_LOADSPR          "loadsprite"    /* loadsprite object */
#define SCRIPT_DEFINEVAR        "var"           /* define variable */
#define SCRIPT_IF               "if"            /* check expression */
#define SCRIPT_DELAY            "delay"         /* delay (in microseconds) */
#define SCRIPT_LOADANM          "loadanm"       /* loadanm object */
#define SCRIPT_LOADPRIORITY     "loadpriority"  /* load priority screen */
#define SCRIPT_LOADMASK         "loadmask"      /* load mask screen */
#define SCRIPT_RETURN           "return"        /* return from this script */
#define SCRIPT_LEAVE            "leave"         /* leave this script */
#define SCRIPT_WHILE            "while"         /* do something while something is true */

/* next are the animation methods */
#define SCRIPT_SETANIM          "setanim"       /* set animation sequence */
#define SCRIPT_SETFRAME         "setframe"      /* set animation frame */
#define SCRIPT_SETMOVE          "setmove"       /* set animation sequence for move */
#define SCRIPT_MOVETO           "moveto"        /* let object move to somewhere */
#define SCRIPT_SETSPEED         "setspeed"      /* set the speed of the animation */
#define SCRIPT_SETHALT          "sethalt"       /* set animation sequence for halt */
#define SCRIPT_FACE             "face"          /* face a direction */
#define SCRIPT_STARTANIM        "animate"       /* start animating */
#define SCRIPT_STOPANIM         "stopanim"      /* stop animating */
#define SCRIPT_SETLOOP          "setloop"       /* set looping flag */
#define SCRIPT_SETMASK          "setmask"       /* set where mask should begin */
#define SCRIPT_SETCLIP          "setclip"       /* set whether object must clip */
#define SCRIPT_STOP             "stop"          /* make object stop moving */

/* next are the sprite methods */
#define SCRIPT_MOVE             "move"          /* move the sprite to a [x,y] position */
#define SCRIPT_SHOW             "show"          /* show the sprite */
#define SCRIPT_HIDE             "hide"          /* hide the sprite */
#define SCRIPT_DESTROY          "destroy"       /* destroy a sprite */
#define SCRIPT_UNLOADSPR        "unload"        /* unload a sprite and destroy it */
#define SCRIPT_ENABLE           "enable"        /* turn it on/enable events */
#define SCRIPT_DISABLE          "disable"       /* turn it off/disable events */

/* the followings methods apply only for internal objects */
/* the following methods apply only for the bar object */
#define SCRIPT_BAR_ADDICON      "addicon"       /* add a icon to the bar */

/* the following methods apply only for the controls object */
#define SCRIPT_CONTROLS_WAITACTION "waitevent"  /* wait until user does something */
#define SCRIPT_CONTROLS_WAITIDLE "waitidle"  /* wait until user does nothing */

/* the following methods apply only for the system object */
#define SCRIPT_EXIT             "exit"          /* exit the interprinter */

/* the following methods apply only for the script object */
#define SCRIPT_LOADSCRIPT       "load"          /* load script */
#define SCRIPT_RUNSCRIPT        "go"            /* run script */
#define SCRIPT_DISPOSESCRIPT    "dispose"       /* dispose script */

/* now come the internal object names */
#define SCRIPT_MOUSE_OBJECTNAME "mouse"         /* mouse object name */
#define SCRIPT_BAR_OBJECTNAME   "bar"           /* bar object name */
#define SCRIPT_CONTROLS_OBJECTNAME "controls"   /* controls object name */
#define SCRIPT_SYSTEM_OBJECTNAME "system"       /* system object name */
#define SCRIPT_SCRIPT_OBJECTNAME "script"       /* script object name */

#define SCRIPT_NOIDENT          0xffff          /* no identifier found */
#define SCRIPT_NOLINE           0xffffffff      /* no line */

#define SCRIPT_STARTPROC	"{"		/* string to define a proc start */
#define SCRIPT_ENDPROC          "}"             /* string to define a proc end */
#define SCRIPT_NOFIELD          "-"             /* nothing actually filled in in field */
#define SCRIPT_EVENTSEPERATOR   "_"             /* event seperator */
#define SCRIPT_COMMENTCHAR      ';'             /* comment something */

#define SCRIPT_NOCHECK          0               /* no checks found */
#define SCRIPT_CHECKEQUAL       1               /* check if it is equal */
#define SCRIPT_CHECKNOTEQUAL    2               /* check if it is not equal */
#define SCRIPT_CHECKGREATER     3               /* check if it is greater */
#define SCRIPT_CHECKSMALLER     4               /* check if it is smaller */

#define SCRIPT_DELAYTIME        50              /* delay this number of ms */
#define SCRIPT_STACKSIZE        512             /* script stack size */

#define SCRIPT_MOVE_SEPARATOR   "_"             /* char to add to separator move strings */
#define SCRIPT_MOVE_NORTH       "north"         /* name to add if moves north */
#define SCRIPT_MOVE_EAST        "east"          /* name to add if moves east */
#define SCRIPT_MOVE_SOUTH       "south"         /* name to add if moves south */
#define SCRIPT_MOVE_WEST        "west"          /* name to add if moves west */

#define SCRIPT_MOVE_WEST_NO     0               /* array index when with west move */
#define SCRIPT_MOVE_EAST_NO     1               /* array index when with east move */
#define SCRIPT_MOVE_NORTH_NO    2               /* array index when with north move */
#define SCRIPT_MOVE_SOUTH_NO    3               /* array index when with south move */
#define SCRIPT_MOVE_HALT        4               /* array idnex when halted */

#define SCRIPT_MASK_TOP         "top"           /* mask from the top */
#define SCRIPT_MASK_BOTTOM      "bottom"        /* mask from the bottom */

#define SCRIPT_TRUE             "yes"           /* its true */
#define SCRIPT_FALSE            "no"            /* its false */

#define SCRIPT_NORESOLV         0xffff          /* returned if unresolvable */

/* next are all expresions understood */
#define SCRIPT_EXPR_X           "x"             /* x coordinate of object */
#define SCRIPT_EXPR_Y           "y"             /* y coordinate of object */

#define SCRIPT_EXPR_FRAMENO      "frameno"       /* get current frame number */
#define SCRIPT_EXPR_MOVING       "moving"        /* is animation currently moving? */
#define SCRIPT_EXPR_ANIMATING    "animating"     /* is animation currently animating? */
#define SCRIPT_EXPR_PRIORITYCODE "prioritycode"  /* get current priority code */

#define SCRIPT_PROC_INIT        "init"          /* called when script must initialize itself */
#define SCRIPT_PROC_RUN         "run"           /* called as a script main loop */
#define SCRIPT_PROC_DONE        "done"          /* called when script must de-initialize itself */

/* next are the return values for the process line function */
#define SCRIPT_OK               0               /* line processed ok */
#define SCRIPT_ERROR            1               /* error processing script line */
#define SCRIPT_SETLINE          2               /* procedure must set line */
#define SCRIPT_QUIT             3               /* signal interprinter to quit */
#define SCRIPT_LEFT             4               /* script wanted to end */

struct SCRIPTHEADER {
    ULONG magic;                                /* should be SCRIPT_MAGICNO */
    UINT  version;                              /* version number (100 = 1.00, 123 = 1.23 etc.) */
    UINT  script_size;                          /* size of the script code */
    UINT  nofprocs;                             /* number of procedures */
    UCHAR reserved[22];                         /* unused */
};

struct PROCHEADER {
    char  name[MAX_IDENT_LEN];                  /* identifier name */
    UINT  start_pos;
    UINT  end_pos;
    UCHAR reserved[28];                         /* unused */
};

class SCRIPT {
public:
    void init(char *fname);
    void done();
    void parse();

    void goproc(char *procname);
    UCHAR goline(char *script_line,char *script_errmsg,ULONG *line);

private:
    char *scriptdata;			/* pointer to script data */

    UCHAR readline(ULONG lineno,char *s);
    UINT findproc(char *name);
    UINT findlabel(char *name);
    UINT findvariable(char *name);
    UINT labelonline(ULONG line);

    UCHAR processexpression(char *expression);

    UINT resolvexpression(char *expr);

    UINT nofprocs;				/* number of procs found */
    UINT noflabels;                         /* number of labels found */
    UINT nofvars;                           /* number of variables found */
    struct PROC proc[SCRIPT_MAXPROCS];	/* procedure data */
    struct LABEL label[SCRIPT_MAXLABELS];   /* label data */
    struct VARIABLE variable[SCRIPT_MAXVARS]; /* variable data */
    ULONG scriptsize;

    UCHAR script_stack[SCRIPT_STACKSIZE];
    ULONG sp;                               /* script stack pointer */

    UCHAR running;                          /* running flag */

    void pushl(ULONG value);
    void popl(ULONG *value);
};
#endif
