#include "standard.h"
#include "global.h"
#include "ds.h"
#include "../sound.h"
#include <math.h>
#include <process.h>
#include "winmain.h"
#include <windows.h>
#include <dsound.h>

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

extern "C" void S_CDVolume(int volume)
{
    if (volume)
        volume = (volume - 5) * 4;
    wsprintf(Var5275f8, "setaudio T2 volume to %u", volume);
    mciSendString(Var5275f8, NULL, 0, NULL);
}

extern "C" bool Func4558e1(uint32 c);

extern "C" bool Func4558d7(uint32 c)
{
    if (c <= 0x81)
        return false;
    return Func4558e1(c);
}

extern "C" bool Func4558e1(uint32 c)
{
    if (c == 0xa8 || c == 0xb8)
        return false;
    if (c >= 0xc0 && c <= 0xff)
        return false;
    return true;
}

extern "C" uint8 T_remapASCII[];
extern "C" uint8 Var44b226[];
#ifdef TR2_EXE
uint8 Var44b226[88] = {
    0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x95, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a,
    0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86,
    0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f, 0x90, 0x91, 0x92,
    0x93, 0x94, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab,
    0xac, 0xad, 0xae, 0xaf,
};
#endif

extern "C" uint32 Func455905(uint32 c)
{
    if (Func4558e1(c))
        return T_remapASCII[c - 32];
    return Var44b226[c - 0xa8];
}

extern "C" uint8 T_textSpacing[];
extern "C" uint8 Var44b27e[];
#ifdef TR2_EXE
uint8 Var44b27e[66] = {
    0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
    0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
    0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
    0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
    0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
    0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
};
#endif

extern "C" uint32 Func455922(uint32 c)
{
    if (c >= 0x6e)
        return Var44b27e[c - 0x6e];
    return T_textSpacing[c];
}

#ifdef TR2_EXE
sint16 cdtrack = -1;
uint8 T_textSpacing[112] = {
    14, 11, 11, 11, 11, 11, 11, 13, 8, 11, 12, 11, 13, 13, 12, 11, 12, 12, 11, 12, 13, 13,
    13, 12, 12, 11, 9, 9, 9, 9, 9, 9, 9, 9, 5, 9, 9, 5, 12, 10, 9, 9, 9, 8, 9, 8, 9, 9, 11,
    9, 9, 9, 12, 8, 10, 10, 10, 10, 10, 9, 10, 10, 5, 5, 5, 11, 9, 10, 8, 6, 6, 7, 7, 3,
    11, 8, 13, 16, 9, 4, 12, 12, 7, 5, 7, 7, 7, 7, 7, 7, 7, 7, 16, 14, 14, 14, 16, 16, 16,
    16, 16, 12, 14, 8, 8, 8, 8, 8, 8, 8, 0, 0,
};
uint8 T_remapASCII[96] = {
    0, 64, 66, 78, 77, 74, 78, 79, 69, 70, 92, 72, 63, 71, 62, 68, 52, 53, 54, 55, 56, 57,
    58, 59, 60, 61, 73, 73, 66, 74, 75, 65, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
    13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 80, 76, 81, 97, 98, 77, 26, 27, 28,
    29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
    51, 100, 101, 102, 67, 0,
};
int LnMasterVolume;
char *CD_LoopTrack;
int CD_LoopCounter;
int sound_active;
char *Var5275f8;
#endif
