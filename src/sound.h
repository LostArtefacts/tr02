#ifndef SOUND_H
#define SOUND_H

#ifdef __cplusplus
extern "C" {
#endif

#include "3dglodef.h"

typedef struct {
    sint16 number;
    unsigned char volume;
    char radius;
    char randomness;
    signed char pitch;
    sint16 flags;
} SAMPLE_INFO;

extern SAMPLE_INFO *sample_infos;
extern sint16 sample_lut[];

int GetRealTrack(int track);
void StopSoundEffect(int nSFX);
void SOUND_EndScene(void);
void SOUND_Stop(void);
void SOUND_Init(void);
int SoundEffect(int nSFX, PHD_3DPOS *pPos, int flags);

#ifdef __cplusplus
}
#endif

#endif
