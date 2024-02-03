#ifndef AICADRIVER_H
#define AICADRIVER_H

#include "AicaCommon.h"

class CAicaDriver
{
public:
    CAicaDriver();

    // Prepare data
    void SetDriverPath(const char * szPath);
    int AddMusic(const char * szPath);
    void LoadInAudioRAMAndStart();

    // Music control
    void PlayMusic(int nSongID);
    void PlayMusicWithVolumeSlideUp(int nSongID, unsigned short uSlideSpeed);
    void PauseMusic(int nSongID);
    void StopMusic(int nSongID);
    void StopMusicWithVolumeSlideDown(int nSongID, unsigned short uSlideSpeed);
    void GetSongName(int nSongID);

    // WAVE management
    int AddWave(const char * szPath);
    void PlayWave(int iWaveID);
    int GetFreeAicaRAM();

    // Volume level
    void SetWaveVolume(byte bVolume);
    void SetMusicVolume(byte bVolume);

    // Low level commands
    char *GetMessageFromSpu();
    void SendCommandToSpu(int nCommandID, int nParametersSize, void * pParameters);
    void FlushMessageQueue();
    ~CAicaDriver();

private:
    BOOL m_bDriverLoaded;
    int m_nMusicCount;
    char * m_pszMusicTable[AICAD_MAX_MUSIC_COUNT];
    BOOL m_pbWaveTable[AICAD_MAX_WAVE_COUNT];
    char * m_szDriverPath;
    char m_szLastMessage[AICAD_MESSAGE_QUEUE_MESSAGE_LENGTH];
};

#endif
