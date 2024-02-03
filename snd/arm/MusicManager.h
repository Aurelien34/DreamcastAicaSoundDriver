#ifndef MUSIC_MANAGER_H
#define MUSIC_MANAGER_H

#ifdef MUSIC_MANAGER_C
#define MUSIC_MANAGER_VARIABLE_MODIFIER
#else
#define MUSIC_MANAGER_VARIABLE_MODIFIER extern
#endif

#define MUSIC_MANAGER_S3M_MAX_CHANNEL_COUNT 32

// S3M Header
typedef struct
{
    // 0000
    char szSongName[28];
    char c1A;
    char cType;
    char xx [2];
    // 0020
    unsigned short nOrdNum;
    unsigned short nInsNum;
    unsigned short nPatNum;
    unsigned short nFlags;
    unsigned short nCwtV;
    unsigned short nFfi;
    char sSCRM[4];
    // 0030
    unsigned char cGlobalVolume;
    unsigned char cInitialSpeed;
    unsigned char cInitialTempo;
    unsigned char cMasterVolume;
    unsigned char cUltraClick;
    unsigned char cDefaultPan;
    char xxxxxxxx[8];
    short pSpecial;
    // 0040
    unsigned char cChannelSettings[32];
} S3MHeader_t;

// ST3 Sample header
typedef struct
{
    // 0000
    char cType;
    char sDosFileName[12];
    unsigned char cParaMemsetHi; // Unused
    unsigned short nParaMemSeg;
    // 0010
    uint uLength;
    uint uLoopBegin;
    uint uLoopEnd;
    char cVolume;
    char x; // Unused
    char cPacking;
    char cFlags;
    // 0020
    unsigned short nC2SpdLo;
    unsigned short nC2SpdHi; // Unused
    void *pSampleData; // Calculated, not in S3M specs
    short nGp; // Unused
    short nSbLE; // Unused
    int nLastUsed; // Unused
    // 0030
    char szSampleName[28];
    char sSCRS[4]; // Unused
} ST3SampleHeader_t;

// The music vector
MUSIC_MANAGER_VARIABLE_MODIFIER AicaMusicVector_t *g_pMusicVector;
// The music state bank
MUSIC_MANAGER_VARIABLE_MODIFIER AicaMusicStateBank_t *g_pMusicStateBank;

// The current S3M header
MUSIC_MANAGER_VARIABLE_MODIFIER S3MHeader_t * g_pCurrentS3MHeader;
// The current S3M state
MUSIC_MANAGER_VARIABLE_MODIFIER AicaS3MState_t * g_pCurrentS3MState;
// The current instrument
MUSIC_MANAGER_VARIABLE_MODIFIER ST3SampleHeader_t * g_pCurrentInstrument;
// The current music ID
MUSIC_MANAGER_VARIABLE_MODIFIER byte bCurrentMusicID;
// Music volume
MUSIC_MANAGER_VARIABLE_MODIFIER uint g_uMusicVolume;

// Initialization
void InitMusics();
// Select the music for next operations
void SetCurrentMusic(int nMusicID);
// Select the intrument for next operations
void SetCurrentInstrument(int nInstrumentID);
// Signs the current instrument data
void SignInstrument();

// Song processing
void ProcessPlayingSongs();
void ProcessOneActionRow();
void ReProcessOneActionRow();
void MoveToNextRow();
void SetOrderPosition(int nOrderIndex, int nRow);

// Song information
char * GetSongName();
int GetInstrumentsCount();
char * GetInstrumentName();

// Tempo and speed
void SetTempo(byte bTempo);
void SetSpeed(byte bSpeed);
void SetTempoAndSpeed(byte bTempo, byte bSpeed);
void ComputeNewPeriod();
uint ComputeSampleFrequency(byte bNote, unsigned short nC4Frequency);

// Music control
void Play();
void Pause();
void Stop();
void StopAllChannelsForCurrentMusic();
void SetMusicVolume(uint uVolume);
void SetVolumeModifiers(unsigned short uVolumeUp, unsigned short uVolumeDown);
void ResetVolumeModifiers();

// Channels mapping
int GetNeededChannelsCount();

// Special effects
void EffectVolumeSlide(byte bCommandInfo, int nColumn);
void EffectVibrato(byte bCommandInfo, int nColumn);
void EffectTonePortamento(byte bCommandInfo, int nColumn);
void EffectSetTempo(byte bCommandInfo);
void EffectPortamentoDown(byte bCommandInfo, int nColumn);
void EffectPortamentoUp(byte bCommandInfo, int nColumn);
void PatternBreak(byte bCommandInfo);
int ProcessS3MMusicVolumeModifiers();

// Debug
char * MusicDebug();

#endif // MUSIC_MANAGER_H
