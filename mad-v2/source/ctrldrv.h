/* are we already included? */
#ifndef __CTRLDRV_INCLUDED__
/* no. set flag we are included, and define things */
#define __CTRLDRV_INCLUDED__

#ifdef DOS
#include <dpmi.h>
#endif

#include "mad.h"
#include "types.h"

class CTRLDRV {
public:
        void init();
        void done();
        void poll();

        _UINT getxpos();
        _UINT getypos();
        _UCHAR getbuttonstat();

private:
        _UINT x,y;
        _UCHAR buttonstat;
        _UCHAR mouse_installed;
};

#ifndef DOS
extern _UCHAR ctrldrv_keydown[128];
#else
extern volatile _UCHAR ctrldrv_keydown[128];
#endif

#ifdef UNIX
extern int x_mouse_x,x_mouse_y;
extern char x_mouse_buttonstat;
#endif

#endif /* __CTRLDRV_INCLUDED__ */
