#include "script.h"
#include "types.h"

/* are we already included? */
#ifndef __BAR_INCLUDED__
/* no. set flag we are included, and define things */
#define __BAR_INCLUDED__

#define BAR_MAINSPRITENAME "bar.spr"

#define BAR_NOFIELD        "-"

#define BAR_HIDDEN         4                    /* hidden icon */
#define BAR_MAXICONS       16                   /* maximum bar icons */
#define BAR_NOCYCLE        1                    /* no cycle is bit 1 */
#define BAR_MOVETO         2                    /* move ego object to mouse position */
#define BAR_NOICON         "none"               /* no icon name */
#define BAR_USE_NOWAY_PROC "use"                /* generic noway use procedure */

#define BAR_INV_NOINVITEM  0xffff               /* no inventory item selected */

#define BAR_EGONAME        "ego"                /* name of ego object */

struct BARICON {
        char  name[SCRIPT_MAXIDENTLEN];         /* name of icon */
        _UINT spriteno;                         /* sprite number */
        _UINT x,y;                              /* coordinates */
        _UINT flags;                            /* flags */
        _UINT  objid;                           /* object id for procedure */
        _ULONG method;                          /* method id for procedure */
        _UINT  def_objid;                       /* default object id */
        _ULONG def_method;                      /* default method */
};

class BAR {
public:
        void init();
        void done();
        void draw();
        void handlevent();

        void addicon(char*,char*,_UINT,_UINT,_UINT,_UINT,_ULONG);
        void bindefault(_UINT,_UINT,_ULONG);

        char *getname(_UINT icono);
        void setobjtomove(_UINT objno);

        void set_inv_item(_UINT);

        _UINT getflags(_UINT icono);
        _UINT getselno();

        _UCHAR mouseinreach();

        _UCHAR isenabled();
        void   setenabled(_UCHAR);

private:
        _SCHAR bar_ypos;

        _UINT  bar_spriteno;
        _UINT  bar_selno;
        _UINT  inv_curitem;

        _UINT  old_aim_spriteno;

        _UCHAR noficons;
        struct BARICON icon[BAR_MAXICONS];

        _UCHAR bar_enabled;
};

#endif
