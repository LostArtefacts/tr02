#include "global.h"
#include "ds.h"

extern "C" int LnMasterVolume;

void S_SoundSetMasterVolume(int nVolume)
{
    LnMasterVolume = nVolume;
}
