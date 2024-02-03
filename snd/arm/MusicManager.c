#define MUSIC_MANAGER_C

#include "common.h"
#include "Aica.h"
#include "ChannelManager.h"
#include "MusicManager.h"

#include "MessageQueue.h"

// The note period table
static const uint auPeriods[] = { 1712, 1616, 1524, 1440, 1356, 1280, 1208, 1140, 1076, 1016, 960, 906 };

// Sinusoidal vibration table for vibrato effect
int aiVibrationTable[64] =
{
       0,  24,  49,  74,  97, 120, 141, 161,
     180, 197, 212, 224, 235, 244, 250, 253,
     255, 253, 250, 244, 235, 224, 212, 197,
     180, 161, 141, 120,  97,  74,  49,  24,
       0, -24, -49, -74, -97,-120,-141,-161,
    -180,-197,-212,-224,-235,-244,-250,-253,
    -255,-253,-250,-244,-235,-224,-212,-197,
    -180,-161,-141,-120, -97, -74, -49, -24
};

int nDebug = 0;

// The music vector
AicaMusicVector_t * g_pMusicVector = (AicaMusicVector_t *) AICAD_MUSIC_VECTOR;
// The music state bank
AicaMusicStateBank_t * g_pMusicStateBank = (AicaMusicStateBank_t *) AICAD_MUSIC_STATE_BANK;


// Channel reservation before starting a music
void MapMusicToChannels();
// Channel release when stopping a music
void ReleaseMusicChannels();


void SetCurrentMusic(int nMusicID)
{
    bCurrentMusicID = (byte)nMusicID;
    g_pCurrentS3MHeader = (S3MHeader_t *)g_pMusicVector->pPointerArray[nMusicID];
    g_pCurrentS3MState = &g_pMusicStateBank->pBankArray[nMusicID];
}

char * GetSongName()
{
    return g_pCurrentS3MHeader->szSongName;
}

char * MusicDebug(int nParam)
{
    SetCurrentMusic(0);
    g_pCurrentS3MHeader->cGlobalVolume = (g_pCurrentS3MHeader->cGlobalVolume + 0x0F) % 0x40;
    return GetStringValue(g_pCurrentS3MHeader->cGlobalVolume);
    //SendMessage("Pan positions:");
    //return GetStringValue(((int)g_pCurrentS3MState->pChannelPanPositionsArray) - ((int)g_pCurrentS3MHeader));
/*    int nInstrument = nParam;
    if (nInstrument >= GetInstrumentsCount())
    {
        SendMessage("Instrument index too high. Instrument count is:");
        return GetStringValue(GetInstrumentsCount());
    }
    
    SetCurrentMusic(0);
    SetCurrentInstrument(nInstrument);
    // Get the note frequency
    uint uSampleFrequency = g_pCurrentInstrument->nC2SpdLo;
    SendMessage("Frequency:");
    SendMessage(GetStringValue(uSampleFrequency));
    SendMessage("Sample data ptr");
    SendMessage(GetStringValue((int)(g_pCurrentInstrument->pSampleData)));
    // Get the AICA channel for this column
    byte bAicaChannel = 0;
    int b16Bits = ((g_pCurrentInstrument->cFlags & 4) != 0);
    int bADPCM = (g_pCurrentInstrument->cPacking != 0);
    int bLoop = g_pCurrentInstrument->cFlags & 1;
    int nMode;
    if (bADPCM)
    {
        nMode = AICA_SM_ADPCM;
        SendMessage("Playing ADPCM");
    }
    else if (b16Bits)
    {
        nMode = AICA_SM_16BIT;
        SendMessage("Playing 16 bits");
    }
    else
    {
        nMode = AICA_SM_8BIT;
        SendMessage("Playing 8 bits");
    }
    if (bLoop)
    {
        SendMessage("Looping (begin, end):");
        SendMessage(GetStringValue(g_pCurrentInstrument->uLoopBegin));
        SendMessage(GetStringValue(g_pCurrentInstrument->uLoopEnd));
    }

    SendMessage("SampleData");
    int iSample;
    for (iSample = 0; iSample < 10; ++iSample)
    {
        SendMessage(GetStringValue(((int *)g_pCurrentInstrument->pSampleData)[iSample]));
    }
    
    aica_play(
        bAicaChannel,                               // Channel
        (uint)g_pCurrentInstrument->pSampleData,    // Sample data
        nMode,                                      // Compression mode
        g_pCurrentInstrument->uLoopBegin,           // Loop begin
         bLoop
         ?g_pCurrentInstrument->uLoopEnd            // Loop end
//          :g_pCurrentInstrument->uLength,            // Sample size
        (int)uSampleFrequency,                      // Sample frequency
        0xD0,                                       // Volume
        0xFF,                                       // Pan
        bLoop);                                     // Loop enabled
    return GetStringValue(nInstrument);
    */
}

void InitMusics()
{
    // Set music volume
    g_uMusicVolume = 255;
    // Iterate through all potential musics
    int iMusic;
    for (iMusic = 0; iMusic < AICAD_MAX_MUSIC_COUNT; ++iMusic)
    {
        // Check the music vector to see if the next music exists
        if (g_pMusicVector->pPointerArray[iMusic] != 0)
        {
            // Select the current music
            SetCurrentMusic(iMusic);
            // Compute the music bank
            g_pCurrentS3MState->pOrdersArray = (byte *)(((char *)g_pCurrentS3MHeader) + 0x0060);
            g_pCurrentS3MState->pInstrumentsParaPtrArray = (short *)(((char *)g_pCurrentS3MState->pOrdersArray) + g_pCurrentS3MHeader->nOrdNum);
            g_pCurrentS3MState->pPatternsParaPtrArray = (short *)(((char *)g_pCurrentS3MState->pInstrumentsParaPtrArray) + g_pCurrentS3MHeader->nInsNum * 2);
            g_pCurrentS3MState->pChannelPanPositionsArray = (((char *)g_pCurrentS3MState->pPatternsParaPtrArray) + g_pCurrentS3MHeader->nPatNum * 2);
            ResetVolumeModifiers();
            // Compute the samples' pointers
            int nNbInstruments = GetInstrumentsCount();
            int iInstrumentID;
            for (iInstrumentID = 0; iInstrumentID < nNbInstruments; ++iInstrumentID)
            {
                SetCurrentInstrument(iInstrumentID);
                g_pCurrentInstrument->pSampleData = (void *)((
                (((uint)g_pCurrentInstrument->nParaMemSeg) + (((uint)g_pCurrentInstrument->cParaMemsetHi) << 16))
                << 4) + (uint)g_pCurrentS3MHeader);
                if (g_pCurrentS3MHeader->nFfi == 2)
                {
                    // The sample is unsigned, we should sign it
                    SignInstrument();
                }
            }
            // Set the default values
            g_pCurrentS3MState->nPlaying = FALSE;

            // Point to the first order in the order list
            SetOrderPosition(0, 0);
        }
        else
        {
            break;
        }
    }
}

int GetInstrumentsCount()
{
    return g_pCurrentS3MHeader->nInsNum;
}

void SetCurrentInstrument(int nInstrumentID)
{
    g_pCurrentInstrument = (ST3SampleHeader_t *)(((int)g_pCurrentS3MHeader) + (((int)g_pCurrentS3MState->pInstrumentsParaPtrArray[nInstrumentID]) << 4));
}

char * GetInstrumentName()
{
    return g_pCurrentInstrument->szSampleName;
}

void SetOrderPosition(int nOrderIndex, int nRow)
{
    if (nOrderIndex < g_pCurrentS3MHeader->nOrdNum)
    {
        if (g_pCurrentS3MState->pOrdersArray[nOrderIndex] == 0xff)
        {
            SetOrderPosition(nOrderIndex + 1, nRow);
        }
        else
        {
            g_pCurrentS3MState->nCurrentOrderIndex = nOrderIndex;
            g_pCurrentS3MState->nPatternBreakRowToReach = nRow;
            g_pCurrentS3MState->nCurrentRowIndex = 0;
            g_pCurrentS3MState->nCurrentRowIncrement = 0;
            g_pCurrentS3MState->pCurrentPackedCommandPtr =
                (void *)(((int)g_pCurrentS3MHeader)
                + (((int)g_pCurrentS3MState->pPatternsParaPtrArray[g_pCurrentS3MState->pOrdersArray[nOrderIndex]]) << 4)
                + 2); // ( +2 = skip the packet length)
    
            // First index, we get default values
            if (nOrderIndex == 0)
            {
                // Initial tempo and speed
                SetTempoAndSpeed(g_pCurrentS3MHeader->cInitialTempo, g_pCurrentS3MHeader->cInitialSpeed);
                // Channels volumes and panning
                int iChannel;
                for (iChannel = 0; iChannel < MUSIC_MANAGER_S3M_MAX_CHANNEL_COUNT; ++iChannel)
                {
                    g_pCurrentS3MState->astChannelState[iChannel].iVolume = 0x3f;
                    int iPanning = 0x80;
                    // Is there a default pan information?
                    if (g_pCurrentS3MHeader->cDefaultPan == 252 && (g_pCurrentS3MState->pChannelPanPositionsArray[iChannel] & 0x20))
                    {
                        // There is a default panning section with panning information
                        iPanning = (g_pCurrentS3MState->pChannelPanPositionsArray[iChannel] & 0x0F) << 4;
                    }
                    else
                    {
                        // There no default panning information, compute it using the channel information
                        iPanning = (((g_pCurrentS3MHeader->cChannelSettings[iChannel] & 0x0F) < 8) ? 0 : 255);
                    }
                    // Set panning value
                    g_pCurrentS3MState->astChannelState[iChannel].iPanning = iPanning;
                }
            }
        }

    }
    else
    {
        SetOrderPosition(0, 0);
    }
}

void ProcessOneActionRow()
{
    // Create a local pointer on the working row
    PatternRow_t * pWorkingRow = &g_pCurrentS3MState->stCurrentPatternRow;
    // Clean the working row
    memset(pWorkingRow, 0, sizeof(PatternRow_t));

    // Unpack the line
    // First create a local reading pointer
    unsigned char * pRead = g_pCurrentS3MState->pCurrentPackedCommandPtr;
    unsigned char bCurrentAction = 0xff;

    int nChannelID;
    ChanAction_t * pCurrentAction;
    for(;;)
    {
        // Read one byte
        bCurrentAction = *(pRead++);

        // Break if EOL
        if (bCurrentAction == 0x00)
        {
            // The row is over
            break;
        }
        else
        {
            // Get the channel id
            nChannelID = bCurrentAction & 0x1F;
            // Get the corresponding action
            pCurrentAction = &(pWorkingRow->pChanInfoArray[nChannelID]);
            if (bCurrentAction & 0x20)
            {
                // Read note and instrument
                pCurrentAction->bNote = *(pRead++);
                pCurrentAction->bInstrument = *(pRead++);
            }
            if (bCurrentAction & 0x40)
            {
                pCurrentAction->bVolume = *(pRead++);
            }
            if (bCurrentAction & 0X80)
            {
                pCurrentAction->bSpecialCommand = *(pRead++);
                pCurrentAction->bCommandInfo = * (pRead++);
            }
        }
    }

    // Set the command pointer for the next row processing
    g_pCurrentS3MState->pCurrentPackedCommandPtr = pRead;

    

/*    uint uTmp = 0;
    pCurrentAction = &(pWorkingRow->pChanInfoArray[0]);
    uTmp += (((int)pWorkingRow->pChanInfoArray[4].bNote) << 24);
    uTmp += (((int)pWorkingRow->pChanInfoArray[5].bNote) << 16);
    uTmp += (((int)pWorkingRow->pChanInfoArray[6].bNote) << 8);
    uTmp += ((int)pWorkingRow->pChanInfoArray[7].bNote);
    SendMessage(GetStringValue(uTmp));*/
//     uDebug += (((int)pWorkingRow->pChanInfoArray[0].bVolume) << 24);
//     uDebug += (((int)pWorkingRow->pChanInfoArray[1].bVolume) << 16);
//     uDebug += (((int)pWorkingRow->pChanInfoArray[2].bVolume) << 8);
//     uDebug += ((int)pWorkingRow->pChanInfoArray[3].bVolume);
//     SendMessage(GetStringValue(uDebug));

    
    // Process the working row
    pCurrentAction = &(pWorkingRow->pChanInfoArray[0]);
    int nColumn;
    // Iterate through the pannel row
    
    for (nColumn = 0; nColumn < MUSIC_MANAGER_S3M_MAX_CHANNEL_COUNT; ++nColumn, ++pCurrentAction)
    {
        if (pCurrentAction->bNote)
        {
            if (pCurrentAction->bNote == 254)
            {
                // Stop the given channel
                aica_stop(g_pCurrentS3MState->abChanMapping[nColumn]);
            }
            else
            {
                // A note is defined, play it!
                // Get instrument
                SetCurrentInstrument(((int)pCurrentAction->bInstrument)-1);
                // Get the note frequency
                uint uSampleFrequency = ComputeSampleFrequency(pCurrentAction->bNote, g_pCurrentInstrument->nC2SpdLo);
                // Store note parameters in the channel state
                g_pCurrentS3MState->astChannelState[nColumn].nNoteFrequency = uSampleFrequency;
                g_pCurrentS3MState->astChannelState[nColumn].nVibPosition = 0;
                g_pCurrentS3MState->astChannelState[nColumn].uPreviousNoteFrequency = g_pCurrentS3MState->astChannelState[nColumn].uCurrentNoteFrequency;
                g_pCurrentS3MState->astChannelState[nColumn].uCurrentNoteFrequency = uSampleFrequency;
                // Get the AICA channel for this column
                byte bAicaChannel = g_pCurrentS3MState->abChanMapping[nColumn];
                int b16Bits = ((g_pCurrentInstrument->cFlags & 4) != 0);
                int bADPCM = (g_pCurrentInstrument->cPacking != 0);
                int bLoop = g_pCurrentInstrument->cFlags & 1;
                // A volume is declared
                if (pCurrentAction->bVolume)
                    g_pCurrentS3MState->astChannelState[nColumn].iVolume = (uint)pCurrentAction->bVolume;
                else
                    g_pCurrentS3MState->astChannelState[nColumn].iVolume = 0x3F;
                uint uVolume = (((g_pCurrentS3MState->astChannelState[nColumn].iVolume) * ((uint)g_pCurrentS3MHeader->cGlobalVolume) * ((uint)g_pCurrentInstrument->cVolume) * g_uMusicVolume) >> 18);
                int nMode = AICA_SM_8BIT;
                if (bADPCM)
                    nMode = AICA_SM_ADPCM;
                else if (b16Bits)
                    nMode = AICA_SM_16BIT;
                aica_play(
                    bAicaChannel,                               // Channel
                    (uint)g_pCurrentInstrument->pSampleData,    // Sample data
                    nMode,                                      // Compression mode
                    g_pCurrentInstrument->uLoopBegin,           // Loop begin
                    bLoop
                    ?g_pCurrentInstrument->uLoopEnd             // Loop end
                    :g_pCurrentInstrument->uLength,             // Sample size
                    (int)uSampleFrequency,                      // Sample frequency
                    uVolume,                                    // Volume
                    g_pCurrentS3MState->astChannelState[nColumn].iPanning,                                       // Panning
                    bLoop);                                     // Loop enabled
            }
        }
    }
}

void ProcessActionRowEffects()
{
    // Apply filters
    ChanAction_t * pCurrentAction = g_pCurrentS3MState->stCurrentPatternRow.pChanInfoArray;
    int nColumn;
    for (nColumn = 0; nColumn < MUSIC_MANAGER_S3M_MAX_CHANNEL_COUNT; ++nColumn, ++pCurrentAction)
    {
        switch (pCurrentAction->bSpecialCommand)
        {
        case 0:
            // No command
            break;
        case 'A' - 'A' + 1:
            // Set speed
            if (g_pCurrentS3MState->nCurrentRowIncrement == 0)
                SetSpeed(pCurrentAction->bCommandInfo);
            break;
        case 'C' - 'A' + 1:
            // Pattern break
            if (g_pCurrentS3MState->nCurrentRowIncrement == 0)
                PatternBreak(pCurrentAction->bCommandInfo);
            break;
        case 'D' - 'A' + 1:
            // Volume slide
            EffectVolumeSlide(pCurrentAction->bCommandInfo, nColumn);
            break;
        case 'E' - 'A' + 1:
            // Portameto Down
            EffectPortamentoDown(pCurrentAction->bCommandInfo, nColumn);
            break;
        case 'F' - 'A' + 1:
            // Portameto Up
            EffectPortamentoUp(pCurrentAction->bCommandInfo, nColumn);
            break;
        case 'G' -'A' + 1:
            // Tone portamento
            EffectTonePortamento(pCurrentAction->bCommandInfo, nColumn);
            break;
        case 'H' - 'A' + 1:
            // Vibrato
            g_pCurrentS3MState->astChannelState[nColumn].nLastVibrato = pCurrentAction->bCommandInfo;
            EffectVibrato(pCurrentAction->bCommandInfo, nColumn);
            break;
        case 'K' -'A' + 1:
            // Vibrato
            EffectVibrato((byte)g_pCurrentS3MState->astChannelState[nColumn].nLastVibrato, nColumn);
            // Volume slide
            EffectVolumeSlide(pCurrentAction->bCommandInfo, nColumn);
            break;
        case 'T' - 'A' + 1:
            // Set tempo
            EffectSetTempo(pCurrentAction->bCommandInfo);
            break;
        default:
            //SendMessage("Unknown command:");
            //SendMessage(GetStringValue(pCurrentAction->bSpecialCommand));
            break;
        }
    }
}

void MoveToNextRow()
{
    // Increment the row index
    if (g_pCurrentS3MState->nCurrentRowIndex < 63)
    {
        ++g_pCurrentS3MState->nCurrentRowIndex;
    }
    else
    {
        // The pattern is complete, step to the next one
        SetOrderPosition(g_pCurrentS3MState->nCurrentOrderIndex + 1, 0);
    }
}

void SetTempo(byte bTempo)
{
    g_pCurrentS3MState->nCurrentTempo = (int)bTempo;
    ComputeNewPeriod();
}

void SetSpeed(byte bSpeed)
{
    g_pCurrentS3MState->nCurrentSpeed = (int)bSpeed;
    ComputeNewPeriod();
}

void SetTempoAndSpeed(byte bTempo, byte bSpeed)
{
    g_pCurrentS3MState->nCurrentTempo = (uint)bTempo;
    g_pCurrentS3MState->nCurrentSpeed = (uint)bSpeed;
    ComputeNewPeriod();
}

void ComputeNewPeriod()
{
    g_pCurrentS3MState->nCurrentPeriod =
            (125 * 20) /
            g_pCurrentS3MState->nCurrentTempo;
}

// Start playing immediately
void Play()
{
    // First ensure the music is not already playing
    if (!g_pCurrentS3MState->nPlaying)
    {
        // Then ensure there are enough available channels to play the song
        if (GetFreeChannelsCount() >= GetNeededChannelsCount())
        {
            MapMusicToChannels();
            g_pCurrentS3MState->nNextTick = timer;
            g_pCurrentS3MState->nPlaying = TRUE;
        }
    }
}

// Suspend play
void Pause()
{
    if (g_pCurrentS3MState->nPlaying)
    {
        g_pCurrentS3MState->nPlaying = FALSE;
        StopAllChannelsForCurrentMusic();
        ReleaseMusicChannels(bCurrentMusicID);
        // Reinitialize volume modifierss
        ResetVolumeModifiers();
    }
}

// Stop playing and rewind the song
void Stop()
{
    if (g_pCurrentS3MState->nPlaying)
    {
        SetOrderPosition(0, 0);
        g_pCurrentS3MState->nPlaying = FALSE;
        StopAllChannelsForCurrentMusic();
        ReleaseMusicChannels(bCurrentMusicID);
        // Reinitialize volume modifiers
        ResetVolumeModifiers();
    }
}

// Process all playing songs
void ProcessPlayingSongs()
{
    // Iterate through all potential musics
    int iMusic;
    for (iMusic = 0; iMusic < AICAD_MAX_MUSIC_COUNT; ++iMusic)
    {
        // Check the music vector to see if the next music exists
        if (g_pMusicVector->pPointerArray[iMusic] != 0)
        {
            // Select the current music
            SetCurrentMusic(iMusic);
            // Is the music playing?
            if (g_pCurrentS3MState->nPlaying)
            {
                // Process volume modifiers
                // Is it the time to process the next row?
                if (timer >= g_pCurrentS3MState->nNextTick)
                {
                    // Process volume modifiers
                    if (ProcessS3MMusicVolumeModifiers())
                    {
                        if (g_pCurrentS3MState->nCurrentRowIncrement == 0)
                            ProcessOneActionRow();

                        // Process row effects
                        ProcessActionRowEffects();
                        // Compute next tick
                        g_pCurrentS3MState->nNextTick += g_pCurrentS3MState->nCurrentPeriod;

                        // Check for new row processing
                        ++(g_pCurrentS3MState->nCurrentRowIncrement);
                        if (g_pCurrentS3MState->nCurrentRowIncrement >= g_pCurrentS3MState->nCurrentSpeed)
                        {
                            // One new row should be processed
                            MoveToNextRow();
                            g_pCurrentS3MState->nCurrentRowIncrement = 0;
                        }
                    }
                }
            }
        }
    }
}

// Stop all channels for the current music
void StopAllChannelsForCurrentMusic()
{
    int iChannelCount = GetNeededChannelsCount();
    int iChannel;
    for (iChannel = 0; iChannel < iChannelCount; ++iChannel)
        aica_stop(g_pCurrentS3MState->abChanMapping[iChannel]);
}

int GetNeededChannelsCount()
{
    // Iterate through the music's channel list and count used channels
    int nResult = 0;
    int iChannel;
    byte *pChanPtr = &(g_pCurrentS3MHeader->cChannelSettings[0]);
    for (iChannel = 0; iChannel < MUSIC_MANAGER_S3M_MAX_CHANNEL_COUNT; ++iChannel)
    {
        if (!(*pChanPtr & 0x80))
            ++nResult;
        ++pChanPtr;
    }
    return nResult;
}

// Maps the channels before playing a song
// We supppose the available channel count was already checked
void MapMusicToChannels()
{
    // Iterate through the music's channel list and count used channels
    int iMusicChannel;
    byte *pMusicChanPtr = &(g_pCurrentS3MHeader->cChannelSettings[0]);
    for (iMusicChannel = 0; iMusicChannel < MUSIC_MANAGER_S3M_MAX_CHANNEL_COUNT; ++iMusicChannel)
    {
        if (!(*pMusicChanPtr & 0x80))
        {
            // A channel is needed, let's find one in the global channel list
            g_pCurrentS3MState->abChanMapping[iMusicChannel] = AssignChannelToMusic(bCurrentMusicID);
            ++pMusicChanPtr;
        }
    }
}

// Compute the sample frequency, given the note to play and the base sample frequency (for C4)
uint ComputeSampleFrequency(byte bNote, unsigned short nC4Frequency)
{
    uint uFrequency, uPeriod;
    uint uNote = (uint) bNote;
    uPeriod = ((8363 * 16 * auPeriods[uNote & 0xF])  >> ( uNote >> 4)) / ((uint) nC4Frequency);
    uFrequency = 14317056 / uPeriod;
    if ( ! uFrequency )
        uFrequency = 1;
    return uFrequency;
}

void SignInstrument()
{
    int bLoop = g_pCurrentInstrument->cFlags & 1;
    uint uSampleCount = bLoop ? g_pCurrentInstrument->uLoopEnd : g_pCurrentInstrument->uLength;

    if (uSampleCount > 0)
    {
        int b16Bits = ((g_pCurrentInstrument->cFlags & 4) != 0);
        if (b16Bits)
        {
            short *pCurr;
            pCurr = g_pCurrentInstrument->pSampleData;
            uint uSamp;
            for (uSamp = uSampleCount; uSamp != 0 ; --uSamp)
            {
                *pCurr = *pCurr ^ 0x8000;
                ++pCurr;
            }
        }
        else
        {
            unsigned char *pCurr;
            pCurr = (unsigned char *)g_pCurrentInstrument->pSampleData;
            uint uSamp;
            for (uSamp = uSampleCount; uSamp != 0; --uSamp)
            {
                *pCurr ^= 0x80;
                ++pCurr;
            }
        }
    }
}

// Effects
void EffectVolumeSlide(byte bCommandInfo, int nColumn)
{
    if ((bCommandInfo & 0xf0) == 0xf0)
    {
        // Fine volume slide down
        if (g_pCurrentS3MState->nCurrentRowIncrement == 0)
        {
            g_pCurrentS3MState->astChannelState[nColumn].iVolume -= ( bCommandInfo & 0x0f );
            if (g_pCurrentS3MState->astChannelState[nColumn].iVolume < 0)
                g_pCurrentS3MState->astChannelState[nColumn].iVolume = 0;
        }
    }
    else
        if ((bCommandInfo & 0x0f) == 0x0f)
        {
            // Fine volume slide up
            if (g_pCurrentS3MState->nCurrentRowIncrement == 0)
            {
                g_pCurrentS3MState->astChannelState[nColumn].iVolume += ( bCommandInfo >> 4 );
                if (g_pCurrentS3MState->astChannelState[nColumn].iVolume > 63)
                    g_pCurrentS3MState->astChannelState[nColumn].iVolume = 63;
            }
        }
        else
            if (bCommandInfo & 0x0f)
            {
                // Volume slide down
                g_pCurrentS3MState->astChannelState[nColumn].iVolume -= ( bCommandInfo & 0x0f );
                if (g_pCurrentS3MState->astChannelState[nColumn].iVolume < 0)
                    g_pCurrentS3MState->astChannelState[nColumn].iVolume = 0;
            }
            else
            {
                // Volume slide up
                g_pCurrentS3MState->astChannelState[nColumn].iVolume += ( bCommandInfo >> 4 );
                if (g_pCurrentS3MState->astChannelState[nColumn].iVolume > 63)
                    g_pCurrentS3MState->astChannelState[nColumn].iVolume = 63;
            }
    
    uint uTmp = ((((uint)g_pCurrentS3MState->astChannelState[nColumn].iVolume) * ((uint)g_pCurrentS3MHeader->cGlobalVolume) * ((uint)g_pCurrentInstrument->cVolume)) >> 10);
    
    aica_vol(g_pCurrentS3MState->abChanMapping[nColumn], uTmp);
}

void EffectVibrato(byte bCommandInfo, int nColumn)
{
    int nTemp = aiVibrationTable[(g_pCurrentS3MState->astChannelState[nColumn].nVibPosition >> 2) & 0x3f];
    nTemp *= (int)(bCommandInfo & 0xf);
    nTemp >>= 4;
    aica_freq(g_pCurrentS3MState->abChanMapping[nColumn], g_pCurrentS3MState->astChannelState[nColumn].nNoteFrequency + nTemp);
    g_pCurrentS3MState->astChannelState[nColumn].nVibPosition += (bCommandInfo & 0xF0) >> 2;
}

void EffectTonePortamento(byte bCommandInfo, int nColumn)
{
    ChannelState_t *pChanState = & g_pCurrentS3MState->astChannelState[nColumn];
    if (pChanState-> uPreviousNoteFrequency < pChanState->uCurrentNoteFrequency)
    {
        // Do an UP portamento
        pChanState-> uPreviousNoteFrequency += ((uint)bCommandInfo)<<4;
        if (pChanState-> uPreviousNoteFrequency > pChanState->uCurrentNoteFrequency)
            pChanState-> uPreviousNoteFrequency = pChanState->uCurrentNoteFrequency;
        aica_freq(g_pCurrentS3MState->abChanMapping[nColumn], pChanState->uPreviousNoteFrequency);
    }
    else
        if (pChanState-> uPreviousNoteFrequency > pChanState->uCurrentNoteFrequency)
        {
            // Do a DOWN portamento
            pChanState-> uPreviousNoteFrequency -= ((uint)bCommandInfo)<<4;
            if (pChanState-> uPreviousNoteFrequency < pChanState->uCurrentNoteFrequency)
                pChanState-> uPreviousNoteFrequency = pChanState->uCurrentNoteFrequency;
            aica_freq(g_pCurrentS3MState->abChanMapping[nColumn], pChanState->uPreviousNoteFrequency);
        }
}

void EffectSetTempo(byte bCommandInfo)
{
    if (g_pCurrentS3MState->nCurrentRowIncrement == 0)
    {
        if ((bCommandInfo >> 4) == 0)
        {
            int nTempo = g_pCurrentS3MState->nCurrentTempo;
            nTempo -= (int)(uint)bCommandInfo;
            if (nTempo <= 0)
            {
                nTempo = 1;
            }
            SetTempo((byte)nTempo);
        }
        else if ((bCommandInfo >> 4) == 1)
        {
            int nTempo = g_pCurrentS3MState->nCurrentTempo;
            nTempo += (int)(uint)(bCommandInfo & 0x0f);
            SetTempo((byte)nTempo);
        }
        else
        {
            SetTempo(bCommandInfo);
        }
    }
}

void EffectPortamentoDown(byte bCommandInfo, int nColumn)
{
    ChannelState_t *pChanState = & g_pCurrentS3MState->astChannelState[nColumn];
    if (g_pCurrentS3MState->nCurrentRowIncrement == 0)
    {
        switch (bCommandInfo & 0xF0)
        {
        case 0xF0:
            // Fine slide down
            pChanState->nPortamento = ((int)(uint)(bCommandInfo & 0x0F)) << 2;
            break;
        case 0xE0:
            // Extra fine slide down
            pChanState->nPortamento = (int)(uint)(bCommandInfo & 0x0F);
            break;
        default:
            // Slide down
            pChanState->nPortamento = (int)(uint)bCommandInfo << 2;
            break;
        }
    }
    pChanState->uCurrentNoteFrequency -= pChanState->nPortamento;
    aica_freq(g_pCurrentS3MState->abChanMapping[nColumn], pChanState->uPreviousNoteFrequency);
}

void EffectPortamentoUp(byte bCommandInfo, int nColumn)
{
    ChannelState_t *pChanState = & g_pCurrentS3MState->astChannelState[nColumn];
    if (g_pCurrentS3MState->nCurrentRowIncrement == 0)
    {
        switch (bCommandInfo & 0xF0)
        {
        case 0xF0:
            // Fine slide down
            pChanState->nPortamento = ((int)(uint)(bCommandInfo & 0x0F)) << 2;
            break;
        case 0xE0:
            // Extra fine slide down
            pChanState->nPortamento = (int)(uint)(bCommandInfo & 0x0F);
            break;
        default:
            // Slide down
            pChanState->nPortamento = (int)(uint)bCommandInfo << 2;
            break;
        }
    }
    pChanState->uCurrentNoteFrequency += pChanState->nPortamento;
    aica_freq(g_pCurrentS3MState->abChanMapping[nColumn], pChanState->uPreviousNoteFrequency);
}

void PatternBreak(byte bCommandInfo)
{
    SetOrderPosition(g_pCurrentS3MState->nCurrentOrderIndex + 1, (int)(uint)bCommandInfo);
    // TODO: position on the correct row
    // For now, position on the beginning of the row
}

void SetMusicVolume(uint uVolume)
{
    g_uMusicVolume = uVolume;
}

void SetVolumeModifiers(unsigned short uVolumeUp, unsigned short uVolumeDown)
{
    // Do nothing if a volume slide down was asked and the music was not playing
    if (uVolumeDown && !g_pCurrentS3MState->nPlaying)
    {
        return;
    }

    // Check if volume slide was switched (up=>down or down=>up)
    if ( (uVolumeUp && g_pCurrentS3MState->nVolumeDownModifier)
        || (uVolumeDown && g_pCurrentS3MState->nVolumeUpModifier) )
    {
        // Volume slide was switched, invert the accumulator
        g_pCurrentS3MState->nCumulativeVolumeModifier = 0x4000 - g_pCurrentS3MState->nCumulativeVolumeModifier;
    }
    else
    {
        // Volume slide was not switched, reset the accumulator
        g_pCurrentS3MState->nCumulativeVolumeModifier = 0;
    }
    g_pCurrentS3MState->nVolumeUpModifier = uVolumeUp;
    g_pCurrentS3MState->nVolumeDownModifier = uVolumeDown;
}

void ResetVolumeModifiers()
{
    g_pCurrentS3MState->nVolumeUpModifier = 0;
    g_pCurrentS3MState->nVolumeDownModifier = 0;
    g_pCurrentS3MState->nCumulativeVolumeModifier = 0;
    g_pCurrentS3MHeader->cGlobalVolume = 0x40;
}

int ProcessS3MMusicVolumeModifiers()
{
    int bContinuePlaying = TRUE;
    if (g_pCurrentS3MState->nVolumeUpModifier != 0)
    {
        // Volume Up
        g_pCurrentS3MState->nCumulativeVolumeModifier += g_pCurrentS3MState->nVolumeUpModifier;
        if (g_pCurrentS3MState->nCumulativeVolumeModifier >= 0x4000)
        {
            // Volum Up modifier is finished
            ResetVolumeModifiers();
        }
        else
        {
            g_pCurrentS3MHeader->cGlobalVolume = (g_pCurrentS3MState->nCumulativeVolumeModifier >> 8);
        }
    }
    else if (g_pCurrentS3MState->nVolumeDownModifier != 0)
    {
        // Volume Down
        g_pCurrentS3MState->nCumulativeVolumeModifier += g_pCurrentS3MState->nVolumeDownModifier;
        if (g_pCurrentS3MState->nCumulativeVolumeModifier >= 0x4000)
        {
            // Volum down modifier is finished
            ResetVolumeModifiers();
            // Stop the music
            Stop();
            bContinuePlaying = FALSE;
        }
        else
        {
            g_pCurrentS3MHeader->cGlobalVolume = 0x40 - (g_pCurrentS3MState->nCumulativeVolumeModifier >> 8);
        }
    }

    return bContinuePlaying;
}
