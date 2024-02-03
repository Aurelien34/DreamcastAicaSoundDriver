#ifndef WAVE_MANAGER_H
#define WAVE_MANAGER_H

#ifdef WAVE_MANAGER_C
#define WAVE_MANAGER_VARIABLE_MODIFIER
#else
#define WAVE_MANAGER_VARIABLE_MODIFIER extern
#endif

// The wave vector
WAVE_MANAGER_VARIABLE_MODIFIER AicaWAVEVector_t *g_pWaveVector;

// Wave volume
WAVE_MANAGER_VARIABLE_MODIFIER uint g_uWaveVolume;

void InitWaves();

void LoadWaveHeader(AicaSendWAVEHeader_t *pSentHeader);

void LoadWaveContent(AicaSendWAVEContent_t *pSentContent);

void PlayWave(int iWaveID);

void SetWaveVolume(uint uVolume);

#endif // WAVE_MANAGER_H
