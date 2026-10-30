#include <windows.h>
#include <dsound.h>
#include "global.h"
#include "ds.h"

extern "C" int LnMasterVolume;

int CalcVolume(int nGameVolume);
int CalcPan(sint16 angle);

sint32 S_SoundPlaySample(int nSample, uint16 wVol, int nPitch, sint16 wPan)
{
    if (!sound_active)
        return ERR_SOUND_NOT_ENABLED;
    return DS_StartSample(nSample, CalcVolume(wVol), nPitch, CalcPan(wPan), 0);
}

void S_SoundSetMasterVolume(int nVolume)
{
    LnMasterVolume = nVolume;
}

void S_SoundStopSample(sint32 handle)
{
    if (!sound_active)
        return;
    DS_StopSample(handle);
}

void S_SoundStopAllSamples(void)
{
    if (!sound_active)
        return;
    DS_StopAllSamples();
}
