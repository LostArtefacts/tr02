#ifndef GLOBAL_H
#define GLOBAL_H

#ifdef __cplusplus
#include <windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "../types.h"

#ifdef __cplusplus
extern RECT GrcDumpWindow;

void DX_DoFlipWait();

bool DX_UpdateFrame(bool tSpinMessageLoop = true, RECT *prcWindow = 0);
#endif

#define ERR_SOUND_NOT_ENABLED (-3)

extern int sound_active;
extern char *malloc_buffer;

sint32 GetRandomDraw(void);
void SeedRandomDraw(sint32 seed);
sint32 GetRandomControl(void);
void SeedRandomControl(sint32 seed);

sint32 S_SoundPlaySample(int nSample, uint16 wVol, int nPitch, sint16 wPan);
sint32 S_SoundPlaySampleLooped(int nSample, uint16 wVol, int nPitch, sint16 wPan);
void S_SoundStopAllSamples(void);
void S_SoundStopSample(sint32 handle);
void S_SoundSetMasterVolume(int nVolume);
sint32 S_SoundSampleIsPlaying(sint32 handle);
void S_SoundSetPanAndVolume(sint32 handle, sint16 wPan, uint16 newvol);
void S_SoundSetPitch(sint32 handle, int nPitch);

#ifdef __cplusplus
}
#endif

#endif
