#include "Aica.h"

#include "MessageQueue.h"

#include "common.h"
#include "BasicFunctions.h"

#define dc_snd_base ((volatile unsigned char *)0x00800000)

/* Some convienence macros */
#define SNDREG32A(x) ((volatile unsigned long *)(dc_snd_base + (x)))
#define SNDREG32(x) (*SNDREG32A(x))
#define SNDREG8A(x) (dc_snd_base + (x))
#define SNDREG8(x) (*SNDREG8A(x))
#define CHNREG32A(chn, x) SNDREG32A(0x80*(chn) + (x))
#define CHNREG32(chn, x) (*CHNREG32A(chn, x))
#define CHNREG8A(chn, x) SNDREG8A(0x80*(chn) + (x))
#define CHNREG8(chn, x) (*CHNREG8A(chn, x))

static inline int calc_aica_pan(int x)
{
    if (x == 0x80)
        return 0;
    else if (x < 0x80) {
        return 0x10 | ((0x7f - x) >> 3);
    } else {
        return (x - 0x80) >> 3;
    }
}

void aica_init()
{
    int i, j;
    
    /* Initialize AICA channels */  
    SNDREG32(0x2800) = 0x0000;
    
    for (i=0; i<64; i++) {
        for (j=0; j<0x80; j+=4)
            CHNREG32(i, j) = 0;
        CHNREG32(i,0) = 0x8000;
        CHNREG32(i,20) = 0x1f;
    }

    SNDREG32(0x2800) = 0x000f;
}

/* Sets up a sound channel completely. This is generally good if you want
   a quick and dirty way to play notes. If you want a more comprehensive
   set of routines (more like PC wavetable cards) see below.
   
   ch is the channel to play on (0 - 63)
   smpptr is the pointer to the sound data; if you're running off the
     SH4, then this ought to be (ptr - 0xa0800000); otherwise it's just
     ptr. Basically, it's an offset into sound ram.
   mode is one of the mode constants (16 bit, 8 bit, ADPCM)
   nsamp is the number of samples to play (not number of bytes!)
   freq is the sampling rate of the sound
   vol is the volume, 0 to 0xff (0xff is louder)
   pan is a panning constant -- 0 is left, 128 is center, 255 is right.

   This routine (and the similar ones) owe a lot to Marcus' sound example -- 
   I hadn't gotten quite this far into dissecting the individual regs yet. */
void aica_play(int ch, unsigned long smpptr, int mode, int loopst, int loopend, int freq, int vol, int pan, int loopflag)
{
    unsigned long freq_lo, freq_base = 5644800;
    int freq_hi = 7;
    int i;
    uint32 playCont;
    int delay = 0;
/*
    SendMessage("ch:");
    SendMessage(GetStringValue(ch));
    SendMessage("smpptr:");
    SendMessage(GetStringValue((int)smpptr));
    SendMessage("Mode:");
    SendMessage(GetStringValue(mode));
    SendMessage("LoopSt:");
    SendMessage(GetStringValue(loopst));
    SendMessage("LoopEnd:");
    SendMessage(GetStringValue(loopend));
    SendMessage("Frequency:");
    SendMessage(GetStringValue(freq));
    SendMessage("Volume:");
    SendMessage(GetStringValue(vol));
    SendMessage("Pan:");
    SendMessage(GetStringValue(pan));
    SendMessage("LoopFlag:");
    SendMessage(GetStringValue(loopflag));
*/
    /* Stop the channel (if it's already playing) */
    aica_stop(ch);
	
	/* Need to convert frequency to floating point format
    (freq_hi is exponent, freq_lo is mantissa)
    Formula is freq = 44100*2^freq_hi*(1+freq_lo/1024) */
    while (freq < freq_base && freq_hi > -8) {
        freq_base >>= 1;
        --freq_hi;
    }
    freq_lo = (freq<<10) / freq_base;
	
	/* Envelope setup. The first of these is the loop point,
    e.g., where the sample starts over when it loops. The second
    is the loop end. This is the full length of the sample when
    you are not looping, or the loop end point when you are (though
    storing more than that is a waste of memory if you're not doing
    volume enveloping). */
    CHNREG32(ch, 8) = loopst & 0xffff;
    CHNREG32(ch, 12) = loopend & 0xffff;
	
    /* Write resulting values */
    CHNREG32(ch, 24) = (freq_hi << 11) | (freq_lo & 1023);
	
    /* Set volume, pan */
    CHNREG8(ch, 36) = calc_aica_pan(pan);
    CHNREG8(ch, 37) = 0xf;
    /* turn off Low Pass Filter (LPF) */
    CHNREG8(ch, 40) = 0x24;
    /* Convert the incoming volume and pan into hardware values */
    /* Vol starts at zero so we can ramp */
    CHNREG8(ch, 41) = 0xff;

	/* If we supported volume envelopes (which we don't yet) then
    this value would set that up. The top 4 bits determine the
    envelope speed. f is the fastest, 1 is the slowest, and 0
    seems to be an invalid value and does weird things). The
    default (below) sets it into normal mode (play and terminate/loop).
    CHNREG32(ch, 16) = 0xf010;
        */
    CHNREG32(ch, 16) = 0x1f;	/* No volume envelope */
	
	
	/* Set sample format, buffer address, and looping control. If
    0x0200 mask is set on reg 0, the sample loops infinitely. If
    it's not set, the sample plays once and terminates. We'll
    also set the bits to start playback here. */
    CHNREG32(ch, 4) = smpptr & 0xffff;
    playCont = (mode<<7) | (smpptr >> 16);
    vol = vol_to_log(vol);
	
    if (loopflag)
        playCont |= 0x0200;

    if (delay) {
        CHNREG32(ch, 0) = playCont; 		/* key off */
        CHNREG8(ch, 41) = vol;
    } else {
        CHNREG32(ch, 0) = 0xc000 | playCont;	/* key on */
	
        /* ramp up the volume */
        for (i=0xff; i>=vol; i--)
            CHNREG8(ch, 41) = i;
    }
}

/* Stop the sound on a given channel */
void aica_stop(int ch)
{
    CHNREG32(ch, 0) = (CHNREG32(ch, 0) & ~0x4000) | 0x8000;
}

static const int logs[] = {
    0, 15, 22, 27, 31, 35, 39, 42, 45, 47, 50, 52, 55, 57, 59, 61,
    63, 65, 67, 69, 71, 73, 74, 76, 78, 79, 81, 82, 84, 85, 87, 88,
    90, 91, 92, 94, 95, 96, 98, 99, 100, 102, 103, 104, 105, 106,
    108, 109, 110, 111, 112, 113, 114, 116, 117, 118, 119, 120, 121,
    122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134,
    135, 136, 137, 138, 138, 139, 140, 141, 142, 143, 144, 145, 146,
    146, 147, 148, 149, 150, 151, 152, 152, 153, 154, 155, 156, 156,
    157, 158, 159, 160, 160, 161, 162, 163, 164, 164, 165, 166, 167,
    167, 168, 169, 170, 170, 171, 172, 173, 173, 174, 175, 176, 176,
    177, 178, 178, 179, 180, 181, 181, 182, 183, 183, 184, 185, 185,
    186, 187, 187, 188, 189, 189, 190, 191, 191, 192, 193, 193, 194,
    195, 195, 196, 197, 197, 198, 199, 199, 200, 200, 201, 202, 202,
    203, 204, 204, 205, 205, 206, 207, 207, 208, 209, 209, 210, 210,
    211, 212, 212, 213, 213, 214, 215, 215, 216, 216, 217, 217, 218,
    219, 219, 220, 220, 221, 221, 222, 223, 223, 224, 224, 225, 225,
    226, 227, 227, 228, 228, 229, 229, 230, 230, 231, 232, 232, 233,
    233, 234, 234, 235, 235, 236, 236, 237, 237, 238, 239, 239, 240,
    240, 241, 241, 242, 242, 243, 243, 244, 244, 245, 245, 246, 246,
    247, 247, 248, 248, 249, 249, 250, 250, 251, 251, 252, 252, 253, 254
};

int vol_to_log(int vol)
{
    // Doesn't seem to work with 0xff
    if (vol >= 0xff)
        vol = 0xfd;
    vol = 0xff - logs[128 + ((vol & 0xff) / 2)];
    return vol;
}

void aica_vol(int ch, int vol)
{
    CHNREG8(ch, 41) = vol_to_log(vol);
}

void aica_freq(int ch, int freq)
{
    unsigned long freq_lo, freq_base = 5644800;
    int freq_hi = 7;

    while (freq < freq_base && freq_hi > -8) {
        freq_base >>= 1;
        freq_hi--;
    }
    freq_lo = (freq<<10) / freq_base;
    CHNREG32(ch, 24) = (freq_hi << 11) | (freq_lo & 1023);
}

void aica_pan(int ch, int pan)
{
    CHNREG8(ch, 36) = calc_aica_pan(pan);
}
