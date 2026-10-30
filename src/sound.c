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
