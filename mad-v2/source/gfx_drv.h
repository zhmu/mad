#ifndef __GFXDRV_DEFINED__
#define __GFXDRV_DEFINED__

#include "types.h"

#define GFX_DRIVER_HRES    320
#define GFX_DRIVER_VRES    200
#define GFX_DRIVER_NOFCOLS 256

#define GFX_DRIVER_PALSIZE (3*GFX_DRIVER_NOFCOLS)

/* Capability flags */
#define GFX_DRIVER_CAPS_DEFAULT 0		/* No special capabilities */
#define GFX_DRIVER_CAPS_ALSOKBD 1		/* This driver will also preform keyboard handling */
#define GFX_DRIVER_CAPS_ALSODEV 2		/* This driver will also preform device handling */
#define GFX_DRIVER_CAPS_CFG     4               /* This driver supports configuration options */

class GFXDRIVER {
public:
        void init();
        void done();
        void draw(void *data);

        void setpalette(char *ptr);

        _UINT    get_hres();
        _UINT    get_vres();
        _UINT    get_nofcols();

        void	poll();

        #ifdef WINDOWS
        _UINT   get_caps();
        void	get_device_stat(_UINT *x,_UINT *y,_UINT *stat);
        void	get_keyboard_status(void *dest);
        void	set_quit_handler(void *dest);
        #endif

private:
        #ifdef DOS
        _UCHAR oldmode,nofrows;
        #endif

        #ifdef WINDOWS
        _UCHAR got_focus;
        _UINT gfx_caps;
        #endif
};
#endif /* __GFXDRV_DEFINED__ */
