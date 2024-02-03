#include "../common/commun.h"

#include "../snd/SoundContainer.h"

#include "../snd/AicaDriver.h"
#include "../common/MathPerso.h"

CSoundContainer::CSoundContainer(CAicaDriver *poAicaDriver)
    : mpoAicaDriver(poAicaDriver)
{
}

void CSoundContainer::AjouterSon(int iWaveID)
{
    moListe.AjouterItem((void *)iWaveID);
}

void CSoundContainer::JouerSonAleatoire()
{
    // Existe-t-il au moins un son?
    if (moListe.GetNbItems() > 0)
    {
        // Tirage au sort d'un fichier son
        int iWaveID = (int)moListe.GetItem(g_math.Rand()%moListe.GetNbItems());
        // Lancement du son
        //TRACE("Envoi du son %d\n", iWaveID);
        mpoAicaDriver->PlayWave(iWaveID);
    }
}
