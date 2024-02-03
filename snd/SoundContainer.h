#ifndef SOUNDCONTAINER_H
#define SOUNDCONTAINER_H

#include "../common/Liste.h"

class CSoundContainer
{
    public:
        CSoundContainer(class CAicaDriver *poAicaDriver);
        void AjouterSon(int iWaveID);
        void JouerSonAleatoire();
    protected:
        class CAicaDriver *mpoAicaDriver;
        CListe moListe;
};

#endif
