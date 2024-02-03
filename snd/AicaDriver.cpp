#include "../common/commun.h"

#include "AicaCommon.h"

#include "AicaDriver.h"

typedef struct
{
    char acRIFF[4];
    char acGarbage1[4];
    char acWAVE[4];
    char acGarbage2[8];
    short cPcmAdpcm;
    short sChannelsCount;
    uint uFrequency;
    char cGarbage3[6];
    short sDepth;
    char acGarbage4[4];
    uint uDataLength;
} WaveHeader_t;

CAicaDriver::CAicaDriver()
:m_bDriverLoaded(FALSE),
m_nMusicCount(0),
m_szDriverPath(NULL)
{
    // Clear the wave table
    memset(m_pbWaveTable, 0, AICAD_MAX_WAVE_COUNT * sizeof(BOOL));
}

CAicaDriver::~CAicaDriver()
{
    // Clear the music table
    for (int nMusic = 0; nMusic < m_nMusicCount; ++nMusic)
    {
        delete [] m_pszMusicTable[nMusic];
    }

    if (m_szDriverPath)
    {
        delete [] m_szDriverPath;
    }
}

void CAicaDriver::SetDriverPath(const char * szPath)
{
    assert_msg(m_szDriverPath == NULL, "The driver path is already set");
    m_szDriverPath = new char[strlen(szPath)+1];
    strcpy(m_szDriverPath, szPath);
}

// Adds the given music to the upload list
int CAicaDriver::AddMusic(const char * szPath)
{
    TRACE2("Adding music file \"%s\" to the music file list\n", szPath);

    assert_msg(m_nMusicCount < AICAD_MAX_MUSIC_COUNT, "The max music count was exceeded");
    assert_msg(!m_bDriverLoaded, "Cannot add a music on a running instance of the audio driver");

    m_pszMusicTable[m_nMusicCount] = new char[strlen(szPath)+1];
    strcpy(m_pszMusicTable[m_nMusicCount], szPath);
    return m_nMusicCount++;
}

// Load musics and driver in audio RAM and start the driver
void CAicaDriver::LoadInAudioRAMAndStart()
{
    // Ensure the driver was not already loaded
    assert_msg(!m_bDriverLoaded, "The driver was already loaded");

    // Ensure a driver path was specified
    assert_msg(m_szDriverPath, "A driver path should be specified before loading it in audio memory");

    TRACE("Audio RAM mapping:\n");
    TRACE2("Maximum music count:\t%d\n", AICAD_MAX_MUSIC_COUNT);
    TRACE2("RAM Start:\t\t0x%08x\n", AICAD_RAM_START);
    TRACE2("RAM Size:\t\t0x%08x\n", AICAD_RAM_SIZE);
    TRACE2("RAM End:\t\t0x%08x\n", AICAD_RAM_END);

    TRACE2("Driver:\t\t0x%08x\n", AICAD_DRIVER);
    TRACE2("Music state bank:\t0x%08x\n", AICAD_MUSIC_STATE_BANK);
    TRACE2("Command queue:\t0x%08x\n", AICAD_COMMAND_QUEUE);
    TRACE2("Message queue:\t0x%08x\n", AICAD_MESSAGE_QUEUE);
    TRACE2("Music vector:\t0x%08x\n", AICAD_MUSIC_VECTOR);
    TRACE2("Wave vector:\t0x%08x\n", AICAD_WAVE_VECTOR);
    TRACE2("Music RAM end:\t0x%08x\n", AICAD_MUSIC_RAM_END);

    TRACE2("Message queue size:\t%d messages\n", AICAD_MESSAGE_QUEUE_MESSAGE_COUNT);
    TRACE2("Message size:\t\t%d bytes\n", AICAD_MESSAGE_QUEUE_MESSAGE_LENGTH);
    
    // Disactivate the SPU
    TRACE("Disactivating the SPU\n");
    spu_disable();

    TRACE("Loading audio stuff in audio RAM...\n");

   // Define the music vector
    AicaMusicVector_t stMusicVector;
    memset(&stMusicVector, 0, sizeof(AicaMusicVector_t));

    // Define the wave vector
    AicaWAVEVector_t stWaveVector;
    memset(&stWaveVector, 0, sizeof(AicaWAVEVector_t));

    // Define the loading limit
    byte * pMusicEndLimit = (byte *)AICAD_MUSIC_RAM_END;

    for (int nMusicIndex = 0; nMusicIndex < m_nMusicCount; ++nMusicIndex)
    {
        // Load the file
        TRACE2("- \"%s\" in audio RAM...\n", m_pszMusicTable[nMusicIndex]);
        FILE *fIn = fopen(m_pszMusicTable[nMusicIndex], "rb");
        assert_msg(fIn, "Unable to find a music file");
        fseek(fIn, 0, SEEK_END);
        uint32 uFileLength = (uint32) ftell(fIn);
        rewind(fIn);
        uint uBufferLength = (uFileLength + 3) & ~0x3;
        byte * pBuffer = new byte[uBufferLength];
        TRACE("  loading in DC RAM...\n");
        fread(pBuffer, 1, uFileLength, fIn);
        fclose(fIn);
        byte * pFileStart = (pMusicEndLimit - uBufferLength);
        TRACE("  loading in AICA RAM...\n");
        spu_memload((uint)pFileStart, pBuffer, uBufferLength);
        delete [] pBuffer;
        TRACE3("  %d bytes sent (rounded to %d bytes)\n", (int)uFileLength, (int)uBufferLength);
        // Compute the new limit
        pMusicEndLimit = pFileStart;
        // Update the vector
        stMusicVector.pPointerArray[nMusicIndex] = (uint)(void *)pFileStart;
    }
    TRACE("- Music vector...\n");
#ifdef _DEBUG
    for (int nIndex = 0; nIndex < AICAD_MAX_MUSIC_COUNT; ++nIndex)
    {
        TRACE("     o 0x%08x\n", stMusicVector.pPointerArray[nIndex]);
    }
#endif
    spu_memload(AICAD_MUSIC_VECTOR, (void *)&stMusicVector, sizeof(AicaMusicVector_t));
    TRACE("- Empty wave vector...\n");
    spu_memload(AICAD_WAVE_VECTOR, (void *)&stWaveVector, sizeof(AicaWAVEVector_t));
    TRACE2("  Loaded (%d bytes)\n", sizeof(AicaWAVEVector_t));
    // Load the driver
    TRACE2("- Driver \"%s\"...", m_szDriverPath);
    FILE *fIn = fopen(m_szDriverPath, "rb");
    assert_msg(fIn, "Unable to find the driver file");
    fseek(fIn, 0, SEEK_END);
    uint uFileLength = (uint32) ftell(fIn);
    rewind(fIn);
    // Create a buffer, rounded to the upper 4 multiple
    uint uBufferLength = (uFileLength + 3) & ~0x3 ;
    byte * pBuffer = new byte[uBufferLength];
    fread(pBuffer, 1, uFileLength, fIn);
    fclose(fIn);
    spu_memload(AICAD_DRIVER, pBuffer, uBufferLength);
    delete [] pBuffer;
    TRACE3("%d bytes sent (rounded to %d bytes)\n", uFileLength, uBufferLength);
    // Send the RAM mapping
    TRACE(" - RAM Mapping...");
    AicaRAMMapping_t stRAMMapping;
    stRAMMapping.pWaveRAMStart = AICAD_DRIVER + AICAD_STACK_SIZE + uBufferLength;
    stRAMMapping.pWaveRAMEnd = (uint)pMusicEndLimit;
    stRAMMapping.pWaveRAMFree = stRAMMapping.pWaveRAMEnd;
    spu_memload(AICAD_RAM_MAPPING, &stRAMMapping, sizeof(AicaRAMMapping_t));

    // Display free Aica RAM
    TRACE2("Free Aica RAM for WAV files: %d bytes\n", GetFreeAicaRAM());

    // Everything is done
    TRACE("-------------------------\nEverything was loaded\n");

    // Start the driver
    TRACE("Starting the SPU program...");
    spu_enable();
    // Wait for the driver to make its initialisations
    timer_spin_sleep(10);
    TRACE("Started\n");

    // Mark the driver as loaded
    m_bDriverLoaded = TRUE;
}

// Reads a message from the message queue
// returns null if no message was read
char *CAicaDriver::GetMessageFromSpu()
{
    assert_msg(m_bDriverLoaded, "GetMessageFromSpu(): Driver must be loaded");
    // First get the pointers
    AicaQueuePtrs_t stQueuePointers;
    spu_memread(&stQueuePointers, AICAD_MESSAGE_QUEUE + offsetof(AicaMessageQueue_t, stQueuePtrs), sizeof(AicaQueuePtrs_t));
    // Check if there is anything to read
    if (stQueuePointers.nReadPtr != stQueuePointers.nWritePtr)
    {
        // There is at least one message to read
        // Read the first one
        spu_memread(
            m_szLastMessage,
            AICAD_MESSAGE_QUEUE
            + offsetof(AicaMessageQueue_t, pMessagesList)
            + stQueuePointers.nReadPtr * AICAD_MESSAGE_QUEUE_MESSAGE_LENGTH, AICAD_MESSAGE_QUEUE_MESSAGE_LENGTH);
        // Update the pointers
        if (++stQueuePointers.nReadPtr == AICAD_MESSAGE_QUEUE_MESSAGE_COUNT)
        {
            stQueuePointers.nReadPtr = 0;
        }
        spu_memload(
            AICAD_MESSAGE_QUEUE
            + offsetof(AicaMessageQueue_t, stQueuePtrs.nReadPtr), &stQueuePointers.nReadPtr,
            sizeof(stQueuePointers.nReadPtr));
        // Return the result
        return m_szLastMessage;
    }
    else
    {
        // There is nothing to read
        return NULL;
    }
}

// Sends a command to the SPU
void CAicaDriver::SendCommandToSpu(int nCommandID, int nParametersSize, void * pParameters)
{
    assert_msg(m_bDriverLoaded, "SendCommandToSpu(): Driver must be loaded");
    assert_msg(nParametersSize <= AICAD_COMMAND_PARAMETERS_LENGTH, "SendCommandToSpu: parameters too long");
    // First get the pointers
    AicaQueuePtrs_t stQueuePointers;
    // Loop until there is space in the queue
    do
    {
        spu_memread(&stQueuePointers, AICAD_COMMAND_QUEUE + offsetof(AicaCommandQueue_t, stQueuePtrs), sizeof(AicaQueuePtrs_t));
    }
    while ((stQueuePointers.nReadPtr == 0 && stQueuePointers.nWritePtr == AICAD_COMMAND_QUEUE_COMMAND_COUNT - 1 )
            || (stQueuePointers.nWritePtr == stQueuePointers.nReadPtr - 1));

    //TRACE("Envoi d'une commande au SPU. File d'attente: R: %d W: %d\n", stQueuePointers.nReadPtr, stQueuePointers.nWritePtr);
    
    // Build and send the command header
    uint uCommandHeader = AICAD_COMMAND_SET_COMMAND_HEADER(nCommandID, nParametersSize);
    int nRemoteAddr = AICAD_COMMAND_QUEUE
            + offsetof(AicaCommandQueue_t, pCommandsList)
            + stQueuePointers.nWritePtr * AICAD_COMMAND_QUEUE_COMMAND_LENGTH;
    spu_memload(nRemoteAddr, &uCommandHeader, 4);
    // Send the command parameters
    if (nParametersSize > 0)
    {
        spu_memload(nRemoteAddr + 4, pParameters, nParametersSize);
    }
    // Update the write pointer
    stQueuePointers.nWritePtr = (stQueuePointers.nWritePtr + 1) % AICAD_COMMAND_QUEUE_COMMAND_COUNT;
    spu_memload(
        AICAD_COMMAND_QUEUE
        + offsetof(AicaCommandQueue_t, stQueuePtrs.nWritePtr), &stQueuePointers.nWritePtr,
        sizeof(stQueuePointers.nWritePtr));
}

// Read all pending messages from the SPU
void CAicaDriver::FlushMessageQueue()
{
    assert_msg(m_bDriverLoaded, "FlushMessageQueue(): Driver must be loaded");
    while (GetMessageFromSpu())
    {
        TRACE2("Got SPU message while flushing queue: %s\n", m_szLastMessage);
    }
}

void CAicaDriver::GetSongName(int nSongID)
{
    assert_msg(m_bDriverLoaded, "GetSongName(): Driver must be loaded");
    assert_msg(nSongID >= 0 && nSongID < m_nMusicCount, "GetSongName(): Music ID is not valid");
    SendCommandToSpu(AICAD_COMMAND_ID_GET_SONG_NAME, sizeof(int), &nSongID);
}

void CAicaDriver::PlayMusic(int nSongID)
{
    assert_msg(m_bDriverLoaded, "PlayMusic(): Driver must be loaded");
    assert_msg(nSongID >= 0 && nSongID < m_nMusicCount, "PlayMusic(): Music ID is not valid");
    SendCommandToSpu(AICAD_COMMAND_ID_PLAY, sizeof(int), &nSongID);
}

void CAicaDriver::PauseMusic(int nSongID)
{
    assert_msg(m_bDriverLoaded, "PauseMusic(): Driver must be loaded");
    assert_msg(nSongID >= 0 && nSongID < m_nMusicCount, "PauseMusic(): Music ID is not valid");
    SendCommandToSpu(AICAD_COMMAND_ID_PAUSE, sizeof(int), &nSongID);
}

void CAicaDriver::StopMusic(int nSongID)
{
    assert_msg(m_bDriverLoaded, "StopMusic(): Driver must be loaded");
    assert_msg(nSongID >= 0 && nSongID < m_nMusicCount, "StopMusic(): Music ID is not valid");
    SendCommandToSpu(AICAD_COMMAND_ID_STOP, sizeof(int), &nSongID);
}

int CAicaDriver::AddWave(const char * szPath)
{
    TRACE2("AddWave(\"%s\")...\n", szPath);
    assert_msg(m_bDriverLoaded, "AddWave(): Driver must be loaded");
    // Find free space for the wave to send
    int iWaveID;
    for (iWaveID = 0; iWaveID < AICAD_MAX_WAVE_COUNT; ++iWaveID)
    {
        if (!m_pbWaveTable[iWaveID])
            break;
    }
    // Check the wave count
    assert_msg(iWaveID != AICAD_MAX_WAVE_COUNT, "AddWave(): the WAVE table is full");

    // Check the wave file
    FILE *fIn = fopen(szPath, "rb");
    assert_msg(fIn != NULL, "AddWave(): could not open wave file");

    // Load the wave header
    WaveHeader_t stWaveHeader;
    fread(&stWaveHeader, sizeof(WaveHeader_t), 1, fIn);
    assert_msg(!strncmp(stWaveHeader.acWAVE, "WAVE", 4), "AddWave(): incorrect wave header");

    // Build the AICA's WAVE header
    AicaWAVEHeader_t stAicaWAVEHeader;
    stAicaWAVEHeader.bStereo = (stWaveHeader.sChannelsCount == 2 ? TRUE : FALSE);
    stAicaWAVEHeader.bADPCM = (stWaveHeader.cPcmAdpcm == 20 ? TRUE : FALSE);
    stAicaWAVEHeader.bDepth = (byte)stWaveHeader.sDepth;
    stAicaWAVEHeader.uLength = stWaveHeader.uDataLength;
    stAicaWAVEHeader.uFrequency = stWaveHeader.uFrequency;

    // Check some parameters
    assert_msg(!stAicaWAVEHeader.bStereo, "Stereo samples not tested!");
    assert_msg(!stAicaWAVEHeader.bADPCM, "ADPCM samples not tested!");
    assert_msg(stAicaWAVEHeader.bDepth == 8 || stAicaWAVEHeader.bDepth == 16, "ADPCM samples not tested!");

    // Send the wave file
    // First send the header
    AicaSendWAVEHeader_t stAicaSendWAVEHeader;
    stAicaSendWAVEHeader.iWaveID = iWaveID;
    memcpy(&stAicaSendWAVEHeader.stWAVEHeader, &stAicaWAVEHeader, sizeof(AicaWAVEHeader_t));
    SendCommandToSpu(AICAD_COMMAND_ID_LOAD_WAVE_HEADER, sizeof(AicaSendWAVEHeader_t), &stAicaSendWAVEHeader);

    // Then send the WAVE content
    int iSizeToSend;
    int iOffset = 0;
    AicaSendWAVEContent_t stAicaSendWAVEContent;
    int iLoop = 0;
    while ( (iSizeToSend = fread(&stAicaSendWAVEContent.pContent, 1, sizeof(stAicaSendWAVEContent.pContent), fIn)) > 0)
    {
        //TRACE("Sending part %d of %d (%d bytes)\n", iLoop, stWaveHeader.uDataLength / sizeof(stAicaSendWAVEContent.pContent), iSizeToSend);
        ++iLoop;
        FlushMessageQueue();
        stAicaSendWAVEContent.iWaveID = iWaveID;
        stAicaSendWAVEContent.iContentOffset = iOffset;
        stAicaSendWAVEContent.iPacketSize = iSizeToSend;
        SendCommandToSpu(AICAD_COMMAND_ID_LOAD_WAVE_CONTENT, iSizeToSend + offsetof(AicaSendWAVEContent_t, pContent), &stAicaSendWAVEContent);
        iOffset += iSizeToSend;
    }

    // Set the room as "taken"
    m_pbWaveTable[iWaveID] = TRUE;

    TRACE2("Wave file added with ID %d\n", iWaveID);

    return iWaveID;
}

void CAicaDriver::PlayWave(int iWaveID)
{
    assert_msg(m_bDriverLoaded, "PlayWave(): Driver must be loaded");
    assert_msg(m_pbWaveTable[iWaveID], "PlayWave(): Wave ID is not valid");
    SendCommandToSpu(AICAD_COMMAND_ID_PLAY_WAVE, sizeof(int), &iWaveID);
}

void CAicaDriver::SetWaveVolume(byte bVolume)
{
    uint uVolume = (uint)bVolume;
    SendCommandToSpu(AICAD_COMMAND_ID_SOUND_VOLUME_WAVE, sizeof(uint), &uVolume);
}

void CAicaDriver::SetMusicVolume(byte bVolume)
{
    uint uVolume = (uint)bVolume;
    SendCommandToSpu(AICAD_COMMAND_ID_SOUND_VOLUME_MUSIC, sizeof(uint), &uVolume);
}

int CAicaDriver::GetFreeAicaRAM()
{
    AicaRAMMapping_t oRamMapping;
    spu_memread(&oRamMapping, AICAD_RAM_MAPPING, sizeof(oRamMapping));
    return oRamMapping.pWaveRAMFree - oRamMapping.pWaveRAMStart;
}

void CAicaDriver::PlayMusicWithVolumeSlideUp(int nSongID, unsigned short uSlideSpeed)
{
    AicaMusicVolumeModifiers_t oVolumeModifiers;
    oVolumeModifiers.iSongID = nSongID;
    oVolumeModifiers.uVolumeUpModifier = uSlideSpeed;
    oVolumeModifiers.uVolumeDownModifier = 0;
    SendCommandToSpu(AICAD_COMMAND_ID_PLAY_WITH_VOLUME_SLIDE, sizeof(AicaMusicVolumeModifiers_t), &oVolumeModifiers);
}

void CAicaDriver::StopMusicWithVolumeSlideDown(int nSongID, unsigned short uSlideSpeed)
{
    AicaMusicVolumeModifiers_t oVolumeModifiers;
    oVolumeModifiers.iSongID = nSongID;
    oVolumeModifiers.uVolumeUpModifier = 0;
    oVolumeModifiers.uVolumeDownModifier = uSlideSpeed;
    SendCommandToSpu(AICAD_COMMAND_ID_STOP_WITH_VOLUME_SLIDE, sizeof(AicaMusicVolumeModifiers_t), &oVolumeModifiers);
}
