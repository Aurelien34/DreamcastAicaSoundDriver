#ifndef CHANNEL_MANAGER_H
#define CHANNEL_MANAGER_H

#ifdef CHANNEL_MANAGER_C
#define CHANNEL_MANAGER_VARIABLE_MODIFIER
#else
#define CHANNEL_MANAGER_VARIABLE_MODIFIER extern
#endif

enum ChannelUsage_e
{
    channelUnused,
    channelMusic,
    channelWaveTemp,
    channelWavePersistant
};

typedef struct
{
    enum ChannelUsage_e eUsage;
    byte iMusicID;
    int iWaveStartTime;
} ChannelUsage_t;

// The global channel usage vector
CHANNEL_MANAGER_VARIABLE_MODIFIER ChannelUsage_t g_pChannelBank[AICAD_MAX_CHANNEL_COUNT];

void InitAicaChannelUsageVector();
int GetFreeChannelsCount();
void ReleaseMusicChannels(int iMusicID);
byte AssignChannelToMusic(int iMusicID);
byte AssignChannelToWave();
byte GetNextFreeChannel();
void SendChannelUsage();

#endif
