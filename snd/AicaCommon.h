#ifndef AICACOMMON_H
#define AICACOMMON_H

/*
+------------------------------------------+ AICAD_RAM_START / AICAD_DRIVER
| Driver + Stack                           |
+------------------------------------------+
|                                          |
| ...                                      |
|                                          |
+------------------------------------------+ ...
| WAVE bank                                |
+------------------------------------------+ ...
| Music bank                               |
+------------------------------------------+ AICAD_WAVE_VECTOR / AICAD_MUSIC_RAM_END
| WAVE vector                              |
+------------------------------------------+ AICAD_MUSIC_VECTOR
| Music vector                             |
+------------------------------------------+ AICAD_MESSAGE_QUEUE
| Message queue                            |
+------------------------------------------+ AICAD_COMMAND_QUEUE
| Command queue                            |
+------------------------------------------+ AICAD_MUSIC_STATE_BANK
| Music state bank                         |
+------------------------------------------+ AICAD_RAM_MAPPING
| RAM mapping                              |
+------------------------------------------+ AICAD_RAM_END
*/

// General
#define AICAD_MAX_MUSIC_COUNT   4
#define AICAD_MAX_WAVE_COUNT    32
#define AICAD_MAX_CHANNEL_COUNT 64
#define AICAD_STACK_SIZE        (64 * 1024)

// RAM Offsets
#define AICAD_RAM_START 0x00000000
#define AICAD_RAM_SIZE 0x00200000
#define AICAD_RAM_END (AICAD_RAM_START + AICAD_RAM_SIZE)

#define AICAD_DRIVER                AICAD_RAM_START
#define AICAD_RAM_MAPPING           (AICAD_RAM_END - sizeof(AicaRAMMapping_t))
#define AICAD_MUSIC_STATE_BANK      (AICAD_RAM_MAPPING - sizeof(AicaMusicStateBank_t))
#define AICAD_COMMAND_QUEUE         (AICAD_MUSIC_STATE_BANK - sizeof(AicaCommandQueue_t))
#define AICAD_MESSAGE_QUEUE         (AICAD_COMMAND_QUEUE - sizeof(AicaMessageQueue_t))
#define AICAD_MUSIC_VECTOR          (AICAD_MESSAGE_QUEUE - sizeof(AicaMusicVector_t))
#define AICAD_WAVE_VECTOR           (AICAD_MUSIC_VECTOR - sizeof(AicaWAVEVector_t))
#define AICAD_MUSIC_RAM_END         AICAD_WAVE_VECTOR

#define AICAD_MESSAGE_QUEUE_MESSAGE_COUNT       16
#define AICAD_MESSAGE_QUEUE_MESSAGE_LENGTH      256

#define AICAD_COMMAND_QUEUE_COMMAND_COUNT       16
#define AICAD_COMMAND_PARAMETERS_LENGTH         (AICAD_COMMAND_QUEUE_COMMAND_LENGTH - 4)
#define AICAD_COMMAND_QUEUE_COMMAND_LENGTH      512

// Aica Commands
#define AICAD_COMMAND_GET_PARAMETERS_SIZE(W32)          ((W32)&0x0000FFFF)
#define AICAD_COMMAND_GET_COMMAND_ID(W32)               ((W32)>>16)
#define AICAD_COMMAND_SET_COMMAND_HEADER(ID, SIZE)      (((ID)<<16)|((SIZE)&0x0000FFFF))

#define AICAD_COMMAND_ID_NOP                        0
#define AICAD_COMMAND_ID_ECHO                       1
#define AICAD_COMMAND_ID_GET_SONG_NAME              2
#define AICAD_COMMAND_ID_GET_INSTRUMENTS_NAMES      3
#define AICAD_COMMAND_ID_GET_CLOCK_VALUE            4
#define AICAD_COMMAND_ID_PLAY                       5
#define AICAD_COMMAND_ID_PAUSE                      6
#define AICAD_COMMAND_ID_STOP                       7
#define AICAD_COMMAND_ID_LOAD_WAVE_HEADER           8
#define AICAD_COMMAND_ID_LOAD_WAVE_CONTENT          9
#define AICAD_COMMAND_ID_PLAY_WAVE                  10
#define AICAD_COMMAND_ID_SOUND_VOLUME_WAVE          11
#define AICAD_COMMAND_ID_SOUND_VOLUME_MUSIC         12
#define AICAD_COMMAND_ID_PLAY_WITH_VOLUME_SLIDE     13
#define AICAD_COMMAND_ID_STOP_WITH_VOLUME_SLIDE     14
#define AICAD_COMMAND_ID_DEBUG                      1234

typedef struct
{
    uint pWaveRAMStart;
    uint pWaveRAMEnd;
    uint pWaveRAMFree;
} AicaRAMMapping_t;

typedef struct
{
    uint pPointerArray[AICAD_MAX_MUSIC_COUNT];
} AicaMusicVector_t;

typedef struct
{
    uint nReadPtr;
    uint nWritePtr;
} AicaQueuePtrs_t;

typedef struct
{
    char pMessagesList[AICAD_MESSAGE_QUEUE_MESSAGE_COUNT][AICAD_MESSAGE_QUEUE_MESSAGE_LENGTH];
    AicaQueuePtrs_t stQueuePtrs;
} AicaMessageQueue_t;

typedef struct
{
    char pCommandsList[AICAD_COMMAND_QUEUE_COMMAND_COUNT][AICAD_COMMAND_QUEUE_COMMAND_LENGTH];
    AicaQueuePtrs_t stQueuePtrs;
} AicaCommandQueue_t;

typedef struct
{
  byte bNote;
  byte bInstrument;
  byte bVolume;
  byte bSpecialCommand;
  byte bCommandInfo;
  byte padding[3];
} ChanAction_t;

typedef struct
{
  ChanAction_t pChanInfoArray[32];
} PatternRow_t;

typedef struct
{
    int iVolume;
    int nNoteFrequency;
    int nVibPosition;
    int nLastVibrato;
    uint uCurrentNoteFrequency;
    uint uPreviousNoteFrequency;
    int nPortamento;
    int iPanning;
} ChannelState_t;

typedef struct
{
    byte *pOrdersArray;
    short *pInstrumentsParaPtrArray;
    short *pPatternsParaPtrArray;
    char *pChannelPanPositionsArray;
    int nCurrentOrderIndex;
    unsigned char * pCurrentPackedCommandPtr;
    uint nCurrentTempo;
    uint nCurrentSpeed;
    uint nCurrentRowIndex;
    uint nCurrentPeriod;
    uint nNextTick;
    int nPlaying;
    int nCurrentRowIncrement;
    int nPatternBreakRowToReach;
    unsigned short nCumulativeVolumeModifier;
    unsigned short nVolumeUpModifier;
    unsigned short nVolumeDownModifier;
    PatternRow_t stCurrentPatternRow;
    byte abChanMapping[32];
    ChannelState_t astChannelState[32];
 } AicaS3MState_t;

typedef struct
{
    AicaS3MState_t pBankArray[AICAD_MAX_MUSIC_COUNT];
} AicaMusicStateBank_t;

typedef struct
{
    byte bStereo;
    byte bADPCM;
    byte bDepth;
    uint uLength;
    uint uFrequency;
} AicaWAVEHeader_t;

typedef struct
{
    byte bUsed;
    AicaWAVEHeader_t stWAVEHeader;
    void *pWAVEData;
} AicaWAVEItem_t;

typedef struct
{
    AicaWAVEItem_t pBankArray[AICAD_MAX_WAVE_COUNT];
} AicaWAVEVector_t;

typedef struct
{
    int iWaveID;
    AicaWAVEHeader_t stWAVEHeader;
} AicaSendWAVEHeader_t;

typedef struct
{
    int iWaveID;
    int iContentOffset;
    int iPacketSize;
    byte pContent[AICAD_COMMAND_PARAMETERS_LENGTH - (sizeof(int) + sizeof(int) + sizeof(int))];
} AicaSendWAVEContent_t;

typedef struct
{
    int iSongID;
    unsigned int uVolumeUpModifier;
    unsigned int uVolumeDownModifier;
} AicaMusicVolumeModifiers_t;

#endif
