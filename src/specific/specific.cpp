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

extern "C" bool CD_LoadAudioIndex(void);

#ifndef TR2_EXE
extern "C" bool CD_Init(void)
{
    if (CD_LoadAudioIndex() && !mciSendString("open audio\\cdaudio.mp3  type mpegvideo alias T2", NULL, 0, NULL))
        mciSendString("set T2 time format ms", NULL, 0, NULL);
    return true;
}
#endif

#ifndef TR2_EXE
extern "C" void CD_Cleanup(void)
{
    mciSendString("stop T2", NULL, 0, NULL);
    mciSendString("close T2", NULL, 0, NULL);
}
#endif

extern "C" char *CD_CommandBuffer;
extern "C" char *CD_AudioIndex;
extern "C" char *CD_LoopTrack;
extern "C" int CD_LoopCounter;

#ifndef TR2_EXE
extern "C" void S_CDLoop(void)
{
    if (!CD_LoopTrack)
        return;
    if (++CD_LoopCounter < 150)
        return;
    CD_LoopCounter = 0;
    if (mciSendString("status T2 mode", CD_CommandBuffer, 50, NULL))
        return;
    if (*(DWORD *)CD_CommandBuffer != 0x706f7473)
        return;
    wsprintf(CD_CommandBuffer, "play T2 from %s to %s", CD_LoopTrack, CD_LoopTrack + 9);
    mciSendString(CD_CommandBuffer, NULL, 0, NULL);
}
#endif

extern "C" sint16 cdtrack;
extern "C" sint16 Option_Music_Volume;
extern "C" int GetRealTrack(int track);

#ifndef TR2_EXE
extern "C" void S_CDPlay(sint16 track, int mode)
{
    char *entry;

    if (!Option_Music_Volume)
        return;
    cdtrack = track;
    entry = CD_AudioIndex + (GetRealTrack(track) - 1) * 18;
    wsprintf(CD_CommandBuffer, "play T2 from %s to %s", entry, entry + 9);
    mciSendString(CD_CommandBuffer, NULL, 0, NULL);
    if (mode)
    {
        CD_LoopTrack = entry;
        CD_LoopCounter = 120;
    }
}
#endif

#ifndef TR2_EXE
extern "C" void S_CDStop(void)
{
    if (cdtrack > 0)
    {
        mciSendString("stop T2", NULL, 0, NULL);
        CD_LoopTrack = NULL;
        cdtrack = 0;
    }
}
#endif

#ifndef TR2_EXE
extern "C" int StartSyncedAudio(int num)
{
    int track;
    char *entry;

    cdtrack = num;
    track = GetRealTrack(num);
    if (mciSendString("set T2 time format ms", NULL, 0, NULL))
        return 0;
    entry = CD_AudioIndex + (track - 1) * 18;
    wsprintf(CD_CommandBuffer, "play T2 from %s to %s", entry, entry + 9);
    return !mciSendString(CD_CommandBuffer, NULL, 0, NULL);
}
#endif

#ifndef TR2_EXE
extern "C" int S_CDGetLoc(void)
{
    long pos;

    if (mciSendString("status T2 position", CD_CommandBuffer, 50, NULL))
        return 0;
    pos = atol(CD_CommandBuffer);
    return (DWORD)(pos - atol(CD_AudioIndex + (GetRealTrack((uint16)cdtrack) - 1) * 18)) * 3 / 40;
}
#endif

#ifndef TR2_EXE
extern "C" void S_CDVolume(int volume)
{
    if (volume)
        volume = (volume - 5) * 4;
    wsprintf(CD_CommandBuffer, "setaudio T2 volume to %u", volume);
    mciSendString(CD_CommandBuffer, NULL, 0, NULL);
}
#endif

extern "C" bool T_IsNonCyrillicChar(uint32 c);

#ifndef TR2_EXE
extern "C" bool T_IsExtendedNonCyrillicChar(uint32 c)
{
    if (c <= 0x81)
        return false;
    return T_IsNonCyrillicChar(c);
}
#endif

#ifndef TR2_EXE
extern "C" bool T_IsNonCyrillicChar(uint32 c)
{
    if (c == 0xa8 || c == 0xb8)
        return false;
    if (c >= 0xc0 && c <= 0xff)
        return false;
    return true;
}
#endif

extern "C" uint8 T_remapASCII[];
extern "C" uint8 T_CyrillicASCIIMap[];
#ifdef TR2_EXE
uint8 T_CyrillicASCIIMap[88] = {
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

#ifndef TR2_EXE
extern "C" uint32 T_GetGlyphIndex(uint32 c)
{
    if (T_IsNonCyrillicChar(c))
        return T_remapASCII[c - 32];
    return T_CyrillicASCIIMap[c - 0xa8];
}
#endif

extern "C" uint8 T_textSpacing[];
extern "C" uint8 T_CyrillicSpacing[];
#ifdef TR2_EXE
uint8 T_CyrillicSpacing[66] = {
    0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
    0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
    0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
    0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
    0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
    0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
};
#endif

#ifndef TR2_EXE
extern "C" uint32 T_GetGlyphSpacing(uint32 c)
{
    if (c >= 0x6e)
        return T_CyrillicSpacing[c - 0x6e];
    return T_textSpacing[c];
}
#endif

#ifdef TR2_EXE
int LnMasterVolume;
char *CD_LoopTrack;
int CD_LoopCounter;
int sound_active;
char *CD_CommandBuffer;
#endif

#ifdef TR2_EXE
extern "C" {
char AudioDeviceType[] = "waveaudio";
}
#endif
