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

int CalcVolume(int nGameVolume)
{
    nGameVolume *= LnMasterVolume;
    return int(5000.0 * (double(nGameVolume) / 2097152.0 - 1.0));
}

int CalcPan(sint16 angle)
{
    angle += 0x4000;
    if (angle < 0)
    {
        angle = -angle;
        if (angle < 0)
            angle = 0x7fff;
    }
    angle -= 0x4000;
    return angle >> 4;
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

extern "C" bool Func456972(void);

extern "C" bool CD_Init(void)
{
    if (Func456972() && !mciSendString("open audio\\cdaudio.mp3  type mpegvideo alias T2", NULL, 0, NULL))
        mciSendString("set T2 time format ms", NULL, 0, NULL);
    return true;
}

extern "C" void CD_Cleanup(void)
{
    mciSendString("stop T2", NULL, 0, NULL);
    mciSendString("close T2", NULL, 0, NULL);
}

extern "C" char *Var5275f8;
extern "C" char *Var5275fc;
extern "C" char *CD_LoopTrack;
extern "C" int CD_LoopCounter;

extern "C" void S_CDLoop(void)
{
    if (!CD_LoopTrack)
        return;
    if (++CD_LoopCounter < 150)
        return;
    CD_LoopCounter = 0;
    if (mciSendString("status T2 mode", Var5275f8, 50, NULL))
        return;
    if (*(DWORD *)Var5275f8 != 0x706f7473)
        return;
    wsprintf(Var5275f8, "play T2 from %s to %s", CD_LoopTrack, CD_LoopTrack + 9);
    mciSendString(Var5275f8, NULL, 0, NULL);
}

extern "C" sint16 cdtrack;
extern "C" sint16 Option_Music_Volume;
extern "C" int GetRealTrack(int track);

extern "C" void S_CDPlay(sint16 track, int mode)
{
    char *entry;

    if (!Option_Music_Volume)
        return;
    cdtrack = track;
    entry = Var5275fc + (GetRealTrack(track) - 1) * 18;
    wsprintf(Var5275f8, "play T2 from %s to %s", entry, entry + 9);
    mciSendString(Var5275f8, NULL, 0, NULL);
    if (mode)
    {
        CD_LoopTrack = entry;
        CD_LoopCounter = 120;
    }
}

extern "C" void S_CDStop(void)
{
    if (cdtrack > 0)
    {
        mciSendString("stop T2", NULL, 0, NULL);
        CD_LoopTrack = NULL;
        cdtrack = 0;
    }
}

extern "C" int StartSyncedAudio(int num)
{
    int track;
    char *entry;

    cdtrack = num;
    track = GetRealTrack(num);
    if (mciSendString("set T2 time format ms", NULL, 0, NULL))
        return 0;
    entry = Var5275fc + (track - 1) * 18;
    wsprintf(Var5275f8, "play T2 from %s to %s", entry, entry + 9);
    return !mciSendString(Var5275f8, NULL, 0, NULL);
}

extern "C" int S_CDGetLoc(void)
{
    long pos;

    if (mciSendString("status T2 position", Var5275f8, 50, NULL))
        return 0;
    pos = atol(Var5275f8);
    return (DWORD)(pos - atol(Var5275fc + (GetRealTrack((uint16)cdtrack) - 1) * 18)) * 3 / 40;
}
