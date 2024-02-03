#include "../common/commun.h"

#include "../snd/AicaDriver.h"

#include "../common/Controleur.h"

#ifdef _ROMDISK
extern uint8 romdisk[];
KOS_INIT_ROMDISK(romdisk);
#endif

int main(int argc, char **argv)
{
    TRACE("Music bank offset from SPU RAM end: %d\n",AICAD_RAM_END - AICAD_MUSIC_STATE_BANK);
    CAicaDriver driver;
    driver.SetDriverPath(PATH_RACINE "AicaD.drv");
    driver.AddMusic("/rd/Lick It -TMMs Mad Bass Mix.S3M");
    driver.LoadInAudioRAMAndStart();

    BOOL bAPressed = FALSE;
    BOOL bBPressed = FALSE;
    BOOL bXPressed = FALSE;
    BOOL bYPressed = FALSE;
    BOOL bLeftPressed = FALSE;
    BOOL bRightPressed = FALSE;
    BOOL bTopPressed = FALSE;
    BOOL bBottomPressed = FALSE;
    CControleur controleur(CONTROLEUR_DREAMCAST_1);
    uint8 uSound1 = 0xFFFFFFFF;
    uint8 uSound2 = 0xFFFFFFFF;
    uint8 uSoundTmp;

    do
    {
        // Inteactions with controllers
        controleur.Actualiser();
        if (controleur.IsBoutonA())
        {
            if (!bAPressed)
            {
                bAPressed = TRUE;
                driver.PlayMusic(0);
            }
        }
        else
        {
            bAPressed = FALSE;
        }
        if (controleur.IsBoutonB())
        {
            if (!bBPressed)
            {
                bBPressed = TRUE;
                driver.StopMusic(0);
            }
        }
        else
        {
            bBPressed = FALSE;
        }
        if (controleur.IsBoutonX())
        {
            if (!bXPressed)
            {
                bXPressed = TRUE;
                //vid_screen_shot(PATH_UPLOAD "cap.ppm");
            }
        }
        else
        {
            bXPressed = FALSE;
        }
        if (controleur.IsBoutonY())
        {
            if (!bYPressed)
            {
                bYPressed = TRUE;
                //driver.SendCommandToSpu(AICAD_COMMAND_ID_DEBUG, 0, NULL);
            }
        }
        else
        {
            bYPressed = FALSE;
        }
        if (controleur.IsBoutonLeft())
        {
            if (!bLeftPressed)
            {
                bLeftPressed = TRUE;
                //driver.PlayWave(0);
            }
        }
        else
        {
            bLeftPressed = FALSE;
        }
        if (controleur.IsBoutonRight())
        {
            if (!bRightPressed)
            {
                bRightPressed = TRUE;
                //driver.PlayWave(1);
            }
        }
        else
        {
            bRightPressed = FALSE;
        }

        if (controleur.IsBoutonUp())
        {
            if (!bTopPressed)
            {
                bTopPressed = TRUE;
                //driver.PlayWave(2);
            }
        }
        else
        {
            bTopPressed = FALSE;
        }

        if (controleur.IsBoutonDown())
        {
            if (!bBottomPressed)
            {
                bBottomPressed = TRUE;
                //driver.PlayWave(3);
            }
        }
        else
        {
            bBottomPressed = FALSE;
        }

        uSoundTmp = controleur.GetGachetteGauche();
        if (uSoundTmp != uSound1)
        {
            uSound1 = uSoundTmp;
            driver.SetWaveVolume(255 - uSound1);
        }

        uSoundTmp = controleur.GetGachetteDroite();
        if (uSoundTmp != uSound2)
        {
            uSound2 = uSoundTmp;
            driver.SetMusicVolume(255 - uSound2);
        }

        static int iMessage = 0;
        // Try to get a message from the driver
        char *pMessage = driver.GetMessageFromSpu();
        if (pMessage)
        {
            TRACE("SPU(%d): \"%s\"\n", iMessage++, pMessage);
        }
    }
    while (!controleur.IsBoutonStart());
}
