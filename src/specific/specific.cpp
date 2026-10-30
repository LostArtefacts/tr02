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

sint32 S_SoundPlaySampleLooped(int nSample, uint16 wVol, int nPitch, sint16 wPan)
{
    if (!sound_active)
        return ERR_SOUND_NOT_ENABLED;
    return DS_StartSample(nSample, CalcVolume(wVol), nPitch, CalcPan(wPan), DSBPLAY_LOOPING);
}

void S_SoundSetPanAndVolume(sint32 handle, sint16 wPan, uint16 wVolume)
{
    if (!sound_active)
        return;
    DS_AdjustVolumeAndPan(handle, CalcVolume(wVolume), CalcPan(wPan));
}

void S_SoundSetPitch(sint32 handle, int nNewPitch)
{
    if (!sound_active)
        return;
    DS_AdjustPitch(handle, nNewPitch);
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

sint32 S_SoundSampleIsPlaying(sint32 handle)
{
    if (!sound_active)
        return false;
    return DS_IsChannelPlaying(handle);
}
