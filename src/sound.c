#include "game.h"

#define MAX_SLOTS 32

enum sound_flags {
    NORMAL_SOUND,
    WAIT_SOUND,
    RESTART_SOUND,
    LOOPED_SOUND
};

typedef struct {
    int nVolume;
    int nPan;
    int nSampleInfo;
    int nPitch;
} SoundSlot;

extern SoundSlot LaSlot[MAX_SLOTS];

#define ArraySize(a) (sizeof(a) / sizeof((a)[0]))

extern uint8 tracksRemoved[];

int GetRealTrack(int track)
{
    int i, real, tr = 0;

    for (i = 2, real = 2; i < track; i++)
    {
        if (tr >= 0 && i == tracksRemoved[tr])
        {
            tr++;
            continue;
        }
        real++;
    }
    return real;
}

#define UNDERWATER 1
#define PITCH_SHIFT 4
#define NO_PAN 0x1000
#define PITCH_WIBBLE 0x2000
#define VOLUME_WIBBLE 0x4000
#define SOUND_RADIUS 10240
#define SOUND_RADIUS_SQRD (SOUND_RADIUS * SOUND_RADIUS)
#define SOUND_MAXVOL_RADIUS 1024
#define SOUND_MAXVOL_RADIUS_SQRD (SOUND_MAXVOL_RADIUS * SOUND_MAXVOL_RADIUS)
#define MAX_VOLUME_CHANGE 0x1000
#define MAX_PITCH_CHANGE 6000
#define ERR_SOUND_NO_FREE_SLOTS -1

int SoundEffect(int nSFX, PHD_3DPOS *pPos, int flags)
{
    SAMPLE_INFO *pSI;
    uint32 distance;
    uint16 pan;
    sint32 x, y, z;
    int pitch;
    int nVolume;
    int nSampleInfo, nSample, nSfxSamples, nSlot, nMode, nHandle;
    int nMinVolume, nMinSlot, nAttenuation;

    if (!sound_active || (flags != SFX_ALWAYS && (flags & UNDERWATER) != (room[camera.pos.room_number].flags & UNDERWATER)))
        return 0;

    nSampleInfo = sample_lut[nSFX];
    if (nSampleInfo == -1) {
        sample_lut[nSFX] = -2;
        return 0;
    }
    if (nSampleInfo == -2)
        return 0;

    pSI = sample_infos + nSampleInfo;
    if (*(sint16 *)&pSI->randomness && GetRandomDraw() > *(sint16 *)&pSI->randomness)
        return 0;

    pan = 0;
    if (pPos) {
        x = pPos->x_pos - camera.mike_pos.x;
        y = pPos->y_pos - camera.mike_pos.y;
        z = pPos->z_pos - camera.mike_pos.z;
        if (x < -SOUND_RADIUS || x > SOUND_RADIUS || y < -SOUND_RADIUS || y > SOUND_RADIUS || z < -SOUND_RADIUS || z > SOUND_RADIUS)
            return 0;

        distance = x * x + y * y + z * z;
        if (distance > SOUND_RADIUS_SQRD)
            return 0;
        if (distance < SOUND_MAXVOL_RADIUS_SQRD)
            distance = 0;
        else
            distance = phd_sqrt(distance) - SOUND_MAXVOL_RADIUS;

        if (!(pSI->flags & NO_PAN))
            pan = phd_atan(z, x) - camera.actual_angle;
    } else
        distance = 0;

    nVolume = *(sint16 *)&pSI->volume;
    if (pSI->flags & VOLUME_WIBBLE)
        nVolume -= (GetRandomDraw() * MAX_VOLUME_CHANGE) >> 15;

    nAttenuation = distance * distance / (SOUND_RADIUS_SQRD / 0x10000);
    nVolume = ((0x10000 - nAttenuation) * nVolume) >> 16;
    if (nVolume <= 0)
        return 0;
    if (nVolume > 0x7fff)
        nVolume = 0x7fff;

    if (flags & PITCH_SHIFT)
        pitch = (flags >> 8) & 0xffffff;
    else
        pitch = 0x10000;

    if (pSI->flags & PITCH_WIBBLE)
        pitch += ((GetRandomDraw() * MAX_PITCH_CHANGE) >> 14) - MAX_PITCH_CHANGE;

    if (pSI->number < 0)
        return 0;

    nSfxSamples = (pSI->flags >> 2) & 15;
    if (nSfxSamples == 1)
        nSample = pSI->number;
    else
        nSample = pSI->number + ((GetRandomDraw() * nSfxSamples) >> 15);

    nMode = pSI->flags & 3;
    switch (nMode) {
    case WAIT_SOUND:
        for (nSlot = 0; nSlot < ArraySize(LaSlot); ++nSlot) {
            if (LaSlot[nSlot].nSampleInfo == nSampleInfo) {
                if (S_SoundSampleIsPlaying(nSlot))
                    return 0;
                LaSlot[nSlot].nSampleInfo = -1;
            }
        }
        break;

    case RESTART_SOUND:
        for (nSlot = 0; nSlot < ArraySize(LaSlot); ++nSlot) {
            if (LaSlot[nSlot].nSampleInfo == nSampleInfo) {
                S_SoundStopSample(nSlot);
                LaSlot[nSlot].nSampleInfo = -1;
                break;
            }
        }
        break;

    case LOOPED_SOUND:
        for (nSlot = 0; nSlot < ArraySize(LaSlot); ++nSlot) {
            if (LaSlot[nSlot].nSampleInfo == nSampleInfo) {
                if (nVolume > LaSlot[nSlot].nVolume) {
                    LaSlot[nSlot].nVolume = nVolume;
                    LaSlot[nSlot].nPan = pan;
                    LaSlot[nSlot].nPitch = pitch;
                    return 1;
                }
                return 0;
            }
        }
        break;

    case NORMAL_SOUND:
        break;
    }

    if (nMode == LOOPED_SOUND)
        nHandle = S_SoundPlaySampleLooped(nSample, (uint16)nVolume, pitch, pan);
    else
        nHandle = S_SoundPlaySample(nSample, (uint16)nVolume, pitch, pan);

    if (nHandle >= 0) {
        LaSlot[nHandle].nVolume = nVolume;
        LaSlot[nHandle].nPan = pan;
        LaSlot[nHandle].nPitch = pitch;
        LaSlot[nHandle].nSampleInfo = nSampleInfo;
        return 1;
    }

    if (nHandle != ERR_SOUND_NO_FREE_SLOTS) {
        pSI->number = -1;
        return 0;
    }

    nMinVolume = 0x8000;
    nMinSlot = -1;
    for (nSlot = 1; nSlot < ArraySize(LaSlot); ++nSlot) {
        if (LaSlot[nSlot].nSampleInfo >= 0 && LaSlot[nSlot].nVolume < nMinVolume) {
            nMinVolume = LaSlot[nSlot].nVolume;
            nMinSlot = nSlot;
        }
    }
    if (nMinSlot < 0 || nVolume < nMinVolume)
        return 0;

    S_SoundStopSample(nMinSlot);
    LaSlot[nMinSlot].nSampleInfo = -1;

    if (nMode == LOOPED_SOUND)
        nHandle = S_SoundPlaySampleLooped(nSample, (uint16)nVolume, pitch, pan);
    else
        nHandle = S_SoundPlaySample(nSample, (uint16)nVolume, pitch, pan);

    if (nHandle >= 0) {
        LaSlot[nHandle].nVolume = nVolume;
        LaSlot[nHandle].nPan = pan;
        LaSlot[nHandle].nPitch = pitch;
        LaSlot[nHandle].nSampleInfo = nSampleInfo;
        return 1;
    }
    return 0;
}

void StopSoundEffect(int nSFX)
{
    int nSlot;
    int nStartSampleInfo, nEndSampleInfo;

    if (!sound_active)
        return;
    nStartSampleInfo = sample_lut[nSFX];
    nEndSampleInfo = nStartSampleInfo + ((sample_infos[nStartSampleInfo].flags >> 2) & 15);
    for (nSlot = 0; nSlot < ArraySize(LaSlot); nSlot++)
    {
        if (LaSlot[nSlot].nSampleInfo >= nStartSampleInfo && LaSlot[nSlot].nSampleInfo < nEndSampleInfo)
        {
            S_SoundStopSample(nSlot);
            LaSlot[nSlot].nSampleInfo = -1;
        }
    }
}

void SOUND_EndScene(void)
{
    int nSlot;

    if (!sound_active)
        return;
    for (nSlot = 0; nSlot < ArraySize(LaSlot); nSlot++)
    {
        if (LaSlot[nSlot].nSampleInfo >= 0)
        {
            if ((sample_infos[LaSlot[nSlot].nSampleInfo].flags & 3) == LOOPED_SOUND)
            {
                if (LaSlot[nSlot].nVolume)
                {
                    S_SoundSetPanAndVolume(nSlot, (sint16)LaSlot[nSlot].nPan, (uint16)LaSlot[nSlot].nVolume);
                    S_SoundSetPitch(nSlot, LaSlot[nSlot].nPitch);
                    LaSlot[nSlot].nVolume = 0;
                }
                else
                {
                    S_SoundStopSample(nSlot);
                    LaSlot[nSlot].nSampleInfo = -1;
                }
            }
            else if (!S_SoundSampleIsPlaying(nSlot))
                LaSlot[nSlot].nSampleInfo = -1;
        }
    }
}

void SOUND_Stop(void)
{
    int nSlot;

    if (!sound_active)
        return;
    S_SoundStopAllSamples();
    for (nSlot = 0; nSlot < ArraySize(LaSlot); ++nSlot)
        LaSlot[nSlot].nSampleInfo = -1;
}

void SOUND_Init(void)
{
    int nSlot;

    S_SoundSetMasterVolume(32);
    for (nSlot = 0; nSlot < ArraySize(LaSlot); ++nSlot)
        LaSlot[nSlot].nSampleInfo = -1;
    sound_active = 1;
}

#ifdef TR2_EXE
SoundSlot LaSlot[MAX_SLOTS];
sint16 sample_lut[370];
SAMPLE_INFO *sample_infos;
#endif
