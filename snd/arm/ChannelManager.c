#define CHANNEL_MANAGER_C

#include "common.h"

#include "ChannelManager.h"

#include "MessageQueue.h"

void SendChannelUsage()
{
    int iChannel;
    SendMessage("SendChannelUsage():");
    for (iChannel = 0; iChannel < AICAD_MAX_CHANNEL_COUNT; ++iChannel)
    {
        SendMessage(GetStringValue(g_pChannelBank[iChannel].eUsage + (g_pChannelBank[iChannel].iWaveStartTime << 8)));
    }
}

void InitAicaChannelUsageVector()
{
    // Set the global channel usage vector
    int iChannel;
    for (iChannel = 0; iChannel < AICAD_MAX_CHANNEL_COUNT; ++iChannel)
    {
        g_pChannelBank[iChannel].eUsage = channelUnused;
        g_pChannelBank[iChannel].iMusicID = 0xFF;
    }
}

int GetFreeChannelsCount()
{
    // Iterate through the channel usage vector
    int nResult = 0;
    int iChannel;
    ChannelUsage_t *pChanUsage = &g_pChannelBank[0];
    for (iChannel = 0; iChannel < AICAD_MAX_CHANNEL_COUNT; ++iChannel)
    {
        // Increment the result, depending on the channel usage
        switch ((pChanUsage++)->eUsage)
        {
            case channelUnused:
            case channelWaveTemp:
                ++nResult;
                break;
            default:
                break;
        }
    }
    return nResult;
}

// Release the channels used to play a music, when the music stops
void ReleaseMusicChannels(int iMusicID)
{
    // Iterate through the channel usage list, and free channels used by the given music
    int iChannel;
    ChannelUsage_t *pChanUsage = &g_pChannelBank[0];
    for (iChannel = 0; iChannel < AICAD_MAX_CHANNEL_COUNT; ++iChannel)
    {
        if (pChanUsage->eUsage == channelMusic
            && pChanUsage->iMusicID == (byte)iMusicID)
        {
            // This channel was being used by the given music
            pChanUsage->eUsage = channelUnused;
            pChanUsage->iMusicID = 0xFF;
        }
        ++pChanUsage;
    }
}

// Assign a channel to the given music
byte AssignChannelToMusic(int iMusicID)
{
    // Get the next free channel
    byte bChannelID = GetNextFreeChannel();

    // Mark the channel
    g_pChannelBank[bChannelID].eUsage = channelMusic;
    g_pChannelBank[bChannelID].iMusicID = iMusicID;

    return bChannelID;
}

// Assign a channel to a wave
byte AssignChannelToWave()
{
    // Get the next free channel
    byte bChannelID = GetNextFreeChannel();

    // Mark the channel
    g_pChannelBank[bChannelID].eUsage = channelWaveTemp;
    g_pChannelBank[bChannelID].iWaveStartTime = timer;

    return bChannelID;
}

// Get the next available channel
byte GetNextFreeChannel()
{
    // Iterate through the channel usage vector completely once to find the oldest
    // playing wave or a completely free channel
    int iChannel;
    ChannelUsage_t *pChanUsage = &g_pChannelBank[0];
    int iLatestTimeFound = 0X7FFFFFFF;
    int iNextAvailableChannel = -1;
    for (iChannel = 0; iChannel < AICAD_MAX_CHANNEL_COUNT; ++iChannel)
    {
        // Increment the result, depending on the channel usage
        switch (pChanUsage->eUsage)
        {
            case channelUnused:
                iNextAvailableChannel = iChannel;
                iLatestTimeFound = 0;
                break;
            case channelWaveTemp:
                if (pChanUsage->iWaveStartTime < iLatestTimeFound)
                {
                    iLatestTimeFound = pChanUsage->iWaveStartTime;
                    iNextAvailableChannel = iChannel;
                }
                break;
            default:
                break;
        }
        ++pChanUsage;
    }
    return (byte)iNextAvailableChannel;
}
