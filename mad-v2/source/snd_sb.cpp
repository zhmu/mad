/*
                              SND_SB.CPP

          Sound Blaster Sound Engine for MAD - DOS version

                     (c) 1998, 1999 The MAD Crew

  Description:
  This will handle the actual low-level sound functions.

  Todo: Nothing

  Portability notes: Needs to be fully rewritten for other platforms.
*/
#include <pc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/movedata.h>
#include "gadgets.h"
#include "snd_drv.h"
#include "types.h"

/* these are the port addresses of the DMA channels */
_UCHAR dma_page[8] = { 0x87, 0x83, 0x81, 0x82, 0x8f, 0x8b, 0x89, 0x8a };
_UCHAR dma_addr[8] = { 0x00, 0x02, 0x04, 0x06, 0xc0, 0xc4, 0xc8, 0xcc };
_UCHAR dma_size[8] = { 0x01, 0x03, 0x05, 0x07, 0xc2, 0xc6, 0xca, 0xce };

/*
 * new_sb_irq()
 *
 * This is the new SB irq
 *
 */
void
new_sb_irq() {
    /* acknowledge the sb by reading the data available port */
    if(snddriver->sb_16bit) {
        inportb(snddriver->sb_port+0xf);
    } else {
        inportb(snddriver->sb_port+0xe);
    }

    /* if in irq detection state, increment it */
    if (snddriver->irq_detect) snddriver->irq_detect++;

    /* acknowlege the pics */
    outportb(0xa0,0x20);
    outportb(0x20,0x20);
}

void new_sb_irq_end() { }

/*
 * SNDDRIVER::SNDDRIVER()
 *
 * This is the constructor of the sound driver object. It will set some values
 * to zero.
 *
 */
SNDDRIVER::SNDDRIVER() {
    /* set values to zero */
    irq_hooked=0;sb_port=0;sb_irq=0;sb_dma=0;irq_detect=0;playbufsel=0;
    playing=0;
}

/*
 * SNDDRIVER::init()
 *
 * This will initialize the sound driver. It will return a non-zero error code
 * on failure or zero on success.
 *
 */
_UCHAR
SNDDRIVER::init() {
    char* blaster;
    char* tmp;
    _UCHAR type;
    _UINT  value,i;
    _ULONG timeout;

    /* Set flag indication IRQ has not yet been hooked */
    irq_hooked=0;

    /* Retrieve the BLASTER= from the environment */
    blaster=getenv(SNDDRV_BLASTER_ENV);

    /* does it exists? */
    if(blaster==NULL) {
        /* nope. die */
        return SNDDRV_ERR_NOBLASTERENV;
    }

    /* parse it */
    /* do the address */
    tmp=strchr(blaster,'A');
    if(tmp==NULL) tmp=strchr(blaster,'a');
    if (tmp==NULL) {
        /* sound blaster environment variable not understood. die */
        return SNDDRV_ERR_BLASTERENVISJUNK;
    }
    /* grab the address */
    sscanf(tmp,"%c%x",&type,&value);
    /* save the port address */
    sb_port=value;

    /* do the interrupt */
    tmp=strchr(blaster,'I');
    if(tmp==NULL) tmp=strchr(blaster,'i');
    if (tmp==NULL) {
        /* sound blaster environment variable not understood. die */
        return SNDDRV_ERR_BLASTERENVISJUNK;
    }
    /* grab the irq */
    sscanf(tmp,"%c%u",&type,&value);
    /* save the irq number */
    sb_irq=value;

    /* do the drq */
    tmp=strchr(blaster,'D');
    if(tmp==NULL) tmp=strchr(blaster,'d');
    if (tmp==NULL) {
        /* sound blaster environment variable not understood. die */
        return SNDDRV_ERR_BLASTERENVISJUNK;
    }
    /* grab the drq */
    sscanf(tmp,"%c%u",&type,&value);
    /* save the drq number */
    sb_dma=value;

    /* do the 16 bit drq */
    tmp=strchr(blaster,'H');
    if(tmp==NULL) tmp=strchr(blaster,'h');
    if (tmp!=NULL) {
        /* it exists! grab the high drq */
        sscanf(tmp,"%c%u",&type,&value);
        /* save the drq number */
        sb_hdma=value;
    } else {
        sb_hdma=SNDDRIVER_NODMA;
    }

    /* now check if there is really a SB card there */
    /* output 1 to the sb reset port */
    outportb(sb_port+6,1);
    delay_ms(10);
    outportb(sb_port+6,0);
    delay_ms(10);

    /* is there data waiting for us? */
    timeout=SNDDRV_WAIT_CYCLES;
    while (--timeout) {
        /* is bit 7 of the data available port set? */
        if(inportb(sb_port+0xe)&0x80) {
            timeout=1;
            break;
        }
    }
    /* check whether it was the timeout that killed us */
    if(!timeout) {
        /* the sb didn't respond. die */
        return SNDDRV_ERR_DSPDOESNOTRESPOND;
    }

    /* now let's check whether the correct byte is available */
    if (inportb(sb_port+0xa)!=0xaa) {
        /* it is not the correct byte. die */
        return SNDDRV_ERR_DSPDOESNOTRESPOND;
    }

    /* lock the handler code */
    _go32_dpmi_lock_code(new_sb_irq,(_ULONG)((_ULONG)new_sb_irq_end-(_ULONG)new_sb_irq));

    /* lock all data */
    _go32_dpmi_lock_data(&this->sb_port,sizeof(sb_port));
    _go32_dpmi_lock_data(&this->sb_irq,sizeof(sb_irq));
    _go32_dpmi_lock_data(&this->sb_dma,sizeof(sb_dma));
    _go32_dpmi_lock_data(&this->irq_detect,sizeof(irq_detect));

    /* get the old sb irq vector */
    _go32_dpmi_get_protected_mode_interrupt_vector(sb_irq+8,&old_sb_irq);

    /* install the new one */
    wrapper.pm_offset=(_ULONG)new_sb_irq;

    /* allocate a iret wrapper */
    if (_go32_dpmi_allocate_iret_wrapper(&wrapper)) {
        /* this failed. die */
        return SNDDRV_ERR_IRQWRONG;
    }

    /* really set the new irq */
    _go32_dpmi_set_protected_mode_interrupt_vector(sb_irq+8,&wrapper);

    /* set the irq hooked flag (Thanks Zarath) */
    irq_hooked=1;

    /* enable the sb irq */
    outportb(0x21,(inportb(0x21)&(~(1<<sb_irq))));

    playbufsize=SNDDRV_PLAYBUF_SIZE;

    /* now we need to allocate some memory that doesn't cross the boundry.
       YUCK! (The code for this was based on The Sound Blaster Library for
       DJGPP 2.x by Joel H. Hunter, but now the Allegro source by Shawn
       Hargreaves is used) */
    playbufseg=__dpmi_allocate_dos_memory(((playbufsize<<1)+15)>>4,&playbufsel);

    /* Did anything fail? */
    if (playbufseg==-1) {
        /* Yeah, we were probably out of memory. Die */
        return SNDDRV_ERR_OUTOFMEMORY;
    }

    /* Set some variables */
    playbufaddr=(playbufseg<<4);

    /* Did we cross the border? */
    if((playbufaddr>>16)!=((playbufaddr+playbufsize)>>16)) {
        /* Yeah, use the second half of the buffer */
        playbufaddr+=playbufsize;
    }

    /* Let the SB keep playing the buffer */
    sb_samplerate=SNDDRV_SAMPLERATE;

    /* Allocate a buffer */
    if((tmp=(char*)malloc(playbufsize))==NULL) {
        /* we're out of memory. die */
        return SNDDRV_ERR_OUTOFMEMORY;
    }

    /* Silence the buffer */
//    memset(tmp,0x80,playbufsize);

    /* Put the buffer in the playing buffer */
//    _dosmemputl(tmp,playbufsize>>2,playbufseg<<4);

    /* Free the buffer */
    free(tmp);

    /* turn on speakers */
    dsp_write(0xd1);

    /* keep playing the buffer */
    play_8bit();

    /* All went OK. Return SNDSB_ERR_OK */
    return SNDDRV_ERR_OK;
}

/*
 * SNDDRIVER::transferdma(_UCHAR channel,_UINT count)
 *
 * This will dump the data at [playbufaddr] to DMA channel [channel]. It will
 * copy [count] bytes to it.
 *
 */
void
SNDDRIVER::transferdma(_UCHAR channel,_UINT count) {
    _UINT size,offset;
    _UCHAR page,picno;
    _UCHAR dma_channel;

    /* set up the variables */
    page=playbufaddr>>12;
    size=count-1;
    dma_channel=channel;
    picno=0;
    offset=playbufaddr&0xffff;

    if(dma_channel>4) {
        picno=1;
        dma_channel-=4;
        offset=(playbufaddr>>1)&0xffff;
    }

    /* Program the DMA controller */
    /* Tell the dma controller the channel */
    outportb((!picno) ? 0x0a : 0xd4,4+dma_channel);
    /* reset some dma flip/flop */
    outportb((!picno) ? 0x0c : 0xd8,0);
    /* tell dma controller write to channel */
    outportb((!picno) ? 0x0b : 0xd6,0x58+dma_channel);
    /* tell the dma controller the lo offset */
    outportb(dma_addr[channel],(offset&0xff));
    /* tell the dma controller the hi offset */
    outportb(dma_addr[channel],(offset>>8));
    /* tell the dma controller the lo size */
    outportb(dma_size[channel],size&0xff);
    /* tell the dma controller the hi size */
    outportb(dma_size[channel],size>>8);
    /* tell the dma controller the page */
    outportb(dma_page[channel],page);
    /* dma programming is done */
    outportb((!picno) ? 0x0a : 0xd4,dma_channel);
}

/*
 * SNDDRIVER::play_8bit()
 *
 * This will start playing 8 bit sound. It will play the memory specified by
 * [playbufaddr]. Please note that the speakers must be turned on BEFORE
 * calling this, since the DSP will not take commands while playing.
 *
 */
void
SNDDRIVER::play_8bit() {
    _UINT size,sr;
    _UCHAR page;

    /* do the dma transfer */
    transferdma(sb_dma,playbufsize);

    /* now program the sb */
    /* calculate the sample rate in dsp form */
    sr=256-((_ULONG)1000000/(_ULONG)sb_samplerate);
    /* divide the size by two */
    size=(playbufsize>>1)-1;
    /* tell the dsp we are about to send the sample rate */
    dsp_write(0x40);
    /*
     * tell the dsp the lo byte of the sr
     *
     * NOTE: The Sound Blaster Series Hardware Programming Guide by Creative
     * Labs (the document I use) says the low byte does NOT have to be sent,
     * but if I don't do this, it doesn't work on my AWE64. Documentation bug?
     *
     */
    dsp_write(sr&0xff);
    /* tell dsp the hi byte of the sr */
    dsp_write(sr>>8);
    /* tell the DSP we are about to send the size */
    dsp_write(0x48);
    /* tell dsp the lo byte of the size */
    dsp_write(size&0xff);
    /* tell dsp the hi byte of the size */
    dsp_write(size>>8);
    /* dsp autoinit 8bit mode */
    dsp_write(0x1c);

    /* set flags */
    playing=1; sb_16bit=0;
}

/*
 * SNDDRIVER::play_16bit()
 *
 * This will start playing 16 bit sound. It will play the memory specified by
 * [playbufaddr]. Please note that the speakers must be turned on BEFORE
 * calling this, since the DSP will not take commands while playing.
 *
 */
void
SNDDRIVER::play_16bit() {
    _UINT size,sr;
    _UCHAR page;

    /* set up some variables */
    size=playbufsize-1;

    /* do the dma transfer */
    transferdma(sb_hdma,playbufsize);

    /* now program the sb */
    /* tell the dsp we're about to send the sample rate */
    dsp_write(0x41);
    /* write the hi byte */
    dsp_write(sb_samplerate>>8);
    /* write the lo byte */
    dsp_write(sb_samplerate&0xff);
    /* and send the play command */
    dsp_write(0xb6);
    /* send the play mode */
    dsp_write((sb_stereo) ? 0x20 : 0x00);
    /* and finally, the length (lo byte first) */
    dsp_write(size&0xff);
    /* and the hi byte */
    dsp_write(size>>8);

    /* set flags */
    playing=1; sb_16bit=1;
}

/*
 * SNDDRIVER::dsp_write(_UCHAR value)
 *
 * This will write [value] to the DSP.
 *
 */
void
SNDDRIVER::dsp_write(_UCHAR value) {
    _ULONG timeout;

    timeout=SNDDRV_WAIT_CYCLES;
    /* wait until the DSP is ready */
    while (--timeout) {
        if(inp(sb_port+0xc)&0x80) break;
    }
    /* write the data */
    outportb(sb_port+0xc,value);
}

/*
 * SNDDRIVER::dsp_read()
 *
 * This will read a byte from the DSP.
 *
 */
_UCHAR
SNDDRIVER::dsp_read() {
    _ULONG timeout;

    timeout=SNDDRV_WAIT_CYCLES;
    /* wait until the DSP is ready */
    while (--timeout) {
        if(inp(sb_port+0xe)) break;
    }
    /* read the data */
    return (inportb(sb_port+0xa));
}

/*
 * SNDDRIVER::err2msg(_UCHAR errcode)
 *
 * This will resolve a sound driver error code into a human-readable string.
 *
 */
char*
SNDDRIVER::err2msg(_UCHAR errcode) {
    switch(errcode) {
                  case SNDDRV_ERR_OK: return SNDDRV_ERROR0;
        case SNDDRV_ERR_NOBLASTERENV: return SNDDRV_ERROR1;
   case SNDDRV_ERR_DSPDOESNOTRESPOND: return SNDDRV_ERROR2;
            case SNDDRV_ERR_IRQWRONG: return SNDDRV_ERROR3;
            case SNDDRV_ERR_DMAWRONG: return SNDDRV_ERROR4;
    case SNDDRV_ERR_BLASTERENVISJUNK: return SNDDRV_ERROR5;
         case SNDDRV_ERR_OUTOFMEMORY: return SNDDRV_ERROR6;
    }
    return SNDDRV_ERRORUNK;
}

/*
 * SNDDRIVER::done()
 *
 * This will deinitialize the sound driver.
 *
 */
void
SNDDRIVER::done() {
    /* are we playing? */
    if (playing) {
        /* yeah, shut it down */
        /* tell dsp to exit auto-init dma mode */
        dsp_write((!sb_16bit) ? 0xda : 0xd9);

        /* turn off speakers */
        dsp_write(0xd3);
        playing=0;
    }

    /* do we know a valid sb irq? */
    if (sb_irq) {
        /* yeah, disable the sb irq */
        outportb(0x21,(inportb(0x21)&((1<<sb_irq))));
    }

    /* did we allocate a playing buffer? */
    if (playbufsel) {
        /* yeah. free it */
        __dpmi_free_dos_memory(playbufsel);
        /* make sure we don't do it twice */
        playbufsel=0;
    }

    /* was the irq hooked? */
    if(irq_hooked) {
        /* yeah, restore it */
        _go32_dpmi_set_protected_mode_interrupt_vector(sb_irq+8,&old_sb_irq);
        /* free the iret wrapper */
        _go32_dpmi_free_iret_wrapper(&wrapper);
        /* make sure we don't do this twice */
        irq_hooked=0;
    }
}
