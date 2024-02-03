#include "common.h"

#include "MessageQueue.h"
#include "CommandQueue.h"
#include "ChannelManager.h"
#include "MusicManager.h"
#include "WaveManager.h"

#include "Aica.h"

extern int nDebug;
extern int start_count;

void arm_main()
{
    // Init message queue
    InitMessageQueue();

    // Detect driver crash
    if (start_count++ > 0)
    {
        SendMessage("Driver crashed");
        for (;;);
    }

    // Init command queue
    InitCommandQueue();
    // Init the Aica channel usage vector
    InitAicaChannelUsageVector();
    // Compute the music bank and the channel vector
    InitMusics();
    // Init the wave table
    InitWaves();
    // Init the channels
    aica_init();
    // Tell SH4 everything initialized correctly
    SendMessage("SPU initialized correctly");
    // Start reading message queue
    for (;;)
    {
        switch (ReadCommand())
        {
        case AICAD_COMMAND_ID_NOP:
            break;
        case AICAD_COMMAND_ID_ECHO:
            SendMessage(GetCommandParameters());
            break;
        case AICAD_COMMAND_ID_GET_SONG_NAME:
            SetCurrentMusic(*((int *)GetCommandParameters()));
            SendMessage(GetSongName());
            break;
        case AICAD_COMMAND_ID_GET_INSTRUMENTS_NAMES:
            SetCurrentMusic(*((int *)GetCommandParameters()));
            int nNbInstruments = GetInstrumentsCount();
            int iInstrumentID;
            for (iInstrumentID = 0; iInstrumentID < nNbInstruments; ++iInstrumentID)
            {
                SetCurrentInstrument(iInstrumentID);
                SendMessage(GetInstrumentName());
            }
            break;
        case AICAD_COMMAND_ID_GET_CLOCK_VALUE:
            SendMessage(GetStringValue(timer));
            break;
        case AICAD_COMMAND_ID_PLAY:
            SetCurrentMusic(*((int *)GetCommandParameters()));
            Play();
            break;
        case AICAD_COMMAND_ID_PAUSE:
            SetCurrentMusic(*((int *)GetCommandParameters()));
            Pause();
            break;
        case AICAD_COMMAND_ID_STOP:
            SetCurrentMusic(*((int *)GetCommandParameters()));
            Stop();
            break;
        case AICAD_COMMAND_ID_LOAD_WAVE_HEADER:
            LoadWaveHeader((AicaSendWAVEHeader_t *)GetCommandParameters());
            break;
        case AICAD_COMMAND_ID_LOAD_WAVE_CONTENT:
            LoadWaveContent((AicaSendWAVEContent_t *)GetCommandParameters());
            break;
        case AICAD_COMMAND_ID_DEBUG:
            SendMessage(MusicDebug(*((int *)GetCommandParameters())));
            break;
        case AICAD_COMMAND_ID_PLAY_WAVE:
            PlayWave(*((int *)GetCommandParameters()));
            break;
        case AICAD_COMMAND_ID_SOUND_VOLUME_WAVE:
            SetWaveVolume(*((uint *)GetCommandParameters()));
            break;
        case AICAD_COMMAND_ID_SOUND_VOLUME_MUSIC:
            SetMusicVolume(*((uint *)GetCommandParameters()));
            break;
        case AICAD_COMMAND_ID_PLAY_WITH_VOLUME_SLIDE:
        {
            AicaMusicVolumeModifiers_t * pModifiers = (AicaMusicVolumeModifiers_t *)GetCommandParameters();
            SetCurrentMusic(pModifiers->iSongID);
            SetVolumeModifiers((unsigned short)pModifiers->uVolumeUpModifier, 0);
            Play();
            break;
        }
        case AICAD_COMMAND_ID_STOP_WITH_VOLUME_SLIDE:
        {
            AicaMusicVolumeModifiers_t * pModifiers = (AicaMusicVolumeModifiers_t *)GetCommandParameters();
            SetCurrentMusic(pModifiers->iSongID);
            SetVolumeModifiers(0, (unsigned short)pModifiers->uVolumeDownModifier);
            break;
        }
        default:
            SendMessage("Received an unknown command");
        }
        // Process playing songs
        ProcessPlayingSongs();
    }
}
