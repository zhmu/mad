#ifndef __SNDDRV_DEFINED__
#define __SNDDRV_DEFINED__

#ifdef DOS
#include <dpmi.h>
#endif
#include "mad.h"
#include "types.h"

#define SNDDRV_BLASTER_ENV "BLASTER"

#define SNDDRIVER_MAX_MEMTRIES              16

#define SNDDRIVER_NODMA                     0xff   /* no DMA channel */

#define SNDDRV_WAIT_CYCLES                  10000
#define SNDDRV_BUFFER_SIZE                  4096
#define SNDDRV_PLAYBUF_SIZE                 2 * SNDDRV_BUFFER_SIZE
#define SNDDRV_SAMPLERATE                   22100

#define SNDDRV_ERR_OK                       0      /* All OK */
#define SNDDRV_ERR_NOBLASTERENV             1      /* BLASTER not in environment */
#define SNDDRV_ERR_DSPDOESNOTRESPOND        2      /* DSP does not respond */
#define SNDDRV_ERR_IRQWRONG                 3      /* IRQ is wrong */
#define SNDDRV_ERR_DMAWRONG                 4      /* DMA is wrong */
#define SNDDRV_ERR_BLASTERENVISJUNK         5      /* BLASTER environment variable not understood */
#define SNDDRV_ERR_OUTOFMEMORY              6      /* The hated out of memory error */

#define SNDDRV_ERROR0                       "All OK"
#define SNDDRV_ERROR1                       "BLASTER environment variable not set"
#define SNDDRV_ERROR2                       "DSP does not respond"
#define SNDDRV_ERROR3                       "IRQ is wrong"
#define SNDDRV_ERROR4                       "DMA is wrong"
#define SNDDRV_ERROR5                       "BLASTER environment variable not understood"
#define SNDDRV_ERROR6                       "Out of memory (can also be BASE memory)"
#define SNDDRV_ERRORUNK                     "Unknown error. Help!"

class SNDDRIVER {
public:
        SNDDRIVER();

        _UCHAR init();
        void   done();

        void   setplaydata(char*);

        char*  err2msg(_UCHAR);

        _ULONG playbufsize;

        #ifdef DOS
        _UINT  sb_port,sb_samplerate;
        _UCHAR sb_irq,sb_dma,sb_hdma,irq_detect,sb_16bit,sb_stereo;

        _UCHAR buf_no;

        int    playbufseg,playbufsel;
        _UINT  playbufaddr;
        #endif

private:
        _UCHAR playing;

        #ifdef DOS
        void   play_8bit();
        void   play_16bit();
        void   dsp_write(_UCHAR);
        _UCHAR dsp_read();
        void   transferdma(_UCHAR,_UINT);

        _UCHAR irq_hooked;

        _go32_dpmi_seginfo old_sb_irq,wrapper;
        #endif
};
#endif /* __SNDDRV_DEFINED__ */
