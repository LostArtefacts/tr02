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
