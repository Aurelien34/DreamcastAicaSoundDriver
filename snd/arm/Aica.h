#ifndef AICA_H
#define AICA_H

#define AICA_SM_8BIT    1
#define AICA_SM_16BIT   0
#define AICA_SM_ADPCM   2


void aica_init();
void aica_play(int ch, unsigned long smpptr, int mode, int loopst, int loopend, int freq, int vol, int pan, int loopflag);
void aica_stop(int ch);
int vol_to_log(int vol);
void aica_vol(int ch, int vol);
void aica_freq(int ch, int freq);
void aica_pan(int ch, int pan);

#endif
