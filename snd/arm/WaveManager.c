#define WAVE_MANAGER_C

#include "common.h"
#include "Aica.h"
#include "ChannelManager.h"
#include "WaveManager.h"

#include "MessageQueue.h"

// The wave vector
AicaWAVEVector_t * g_pWaveVector = (AicaWAVEVector_t *) AICAD_WAVE_VECTOR;

// The RAM mapping
AicaRAMMapping_t * g_pRAMMapping = (AicaRAMMapping_t *) AICAD_RAM_MAPPING;

void InitWaves()
{
    // Set WAV volume
    g_uWaveVolume = 255;
    // Clear the wave table
    memset(g_pWaveVector, 0, sizeof(AicaWAVEVector_t));
}

void LoadWaveHeader(AicaSendWAVEHeader_t *pSentHeader)
{
    // Get the wave vector entry
    AicaWAVEItem_t * pWaveItem = &g_pWaveVector->pBankArray[pSentHeader->iWaveID];
    // Mark the wave entry as used
    pWaveItem->bUsed = TRUE;
    // Copy the header in the WAVE vector
    memcpy(&pWaveItem->stWAVEHeader, &pSentHeader->stWAVEHeader, sizeof(AicaWAVEHeader_t));
    // Recompute free RAM pointer
    g_pRAMMapping->pWaveRAMFree -= pWaveItem->stWAVEHeader.uLength;
    // Round the address to an even one (only needed for 16 bits samples, but faster to do it anytime)
    g_pRAMMapping->pWaveRAMFree &= 0xFFFFFFFE;
    // Copy the wave content in the wave RAM
    pWaveItem->pWAVEData = (void *)g_pRAMMapping->pWaveRAMFree;
    // Send some information about the WAV file
    //SendMessage("Received WAV header:");
    //SendMessage("Stereo:");
    //SendMessage(GetStringValue((unsigned int)pWaveItem->stWAVEHeader.bStereo));
    //SendMessage("ADPCM:");
    //SendMessage(GetStringValue((unsigned int)pWaveItem->stWAVEHeader.bADPCM));
    //SendMessage("Depth:");
    //SendMessage(GetStringValue((unsigned int)pWaveItem->stWAVEHeader.bDepth));
    //SendMessage("Length:");
    //SendMessage(GetStringValue(pWaveItem->stWAVEHeader.uLength));
    //SendMessage("Frequency:");
    //SendMessage(GetStringValue(pWaveItem->stWAVEHeader.uFrequency));
    //SendMessage("Copied to the following address:");
    //SendMessage(GetStringValue(pWaveItem->pWAVEData));
}

void LoadWaveContent(AicaSendWAVEContent_t *pSentContent)
{
    // Get the wave vector entry
    AicaWAVEItem_t * pWaveItem = &g_pWaveVector->pBankArray[pSentContent->iWaveID];
    // Get the target address
    void *pTarget = pWaveItem->pWAVEData + pSentContent->iContentOffset;
    // Compute the byte count to copy
    int iBytesToCopy = pSentContent->iPacketSize;
    // Copy the wave content
    memcpy(pTarget, pSentContent->pContent, iBytesToCopy);
    // Sign the wave content
    if (!pWaveItem->stWAVEHeader.bADPCM)
    {
        if (pWaveItem->stWAVEHeader.bDepth == 16)
        {
            short *pCurr;
            pCurr = pTarget;
            uint uSamp;
            for (uSamp = (iBytesToCopy >> 1); uSamp > 0 ; --uSamp)
            {
                //*pCurr = *pCurr ^ 0x8000;
                ++pCurr;
            }
        }
        else
        {
            unsigned char *pCurr;
            pCurr = (unsigned char *)pTarget;
            uint uSamp;
            for (uSamp = iBytesToCopy; uSamp != 0; --uSamp)
            {
                *pCurr ^= 0x80;
                ++pCurr;
            }
        }
    }
}

void PlayWave(int iWaveID)
{
    // Get a free channel to play the wave
    byte bFreeChannel = AssignChannelToWave();

    // Get the wave header
    AicaWAVEItem_t * pWaveItem = &g_pWaveVector->pBankArray[iWaveID];

    int iMode = pWaveItem->stWAVEHeader.bADPCM ? AICA_SM_ADPCM : (pWaveItem->stWAVEHeader.bDepth == 8 ? AICA_SM_8BIT : AICA_SM_16BIT);
    
    // Play the wave file
    aica_play(
    bFreeChannel,               // Channel
    (unsigned long)pWaveItem->pWAVEData,       // Sample data
    iMode,                      // Sample mode
    0,                          // Loop begin
    (iMode == AICA_SM_16BIT) ? (pWaveItem->stWAVEHeader.uLength >> 1) : (pWaveItem->stWAVEHeader.uLength),         // Loop end
    pWaveItem->stWAVEHeader.uFrequency,      // Sample frequency
    g_uWaveVolume,                       // Volume
    128,                        // Panning (center)
    FALSE);                     // Loop enabled
}

void SetWaveVolume(uint uVolume)
{
    g_uWaveVolume = uVolume;
}
