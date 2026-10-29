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

bool DX_UpdateFrame(bool tSpinMessageLoop = true, RECT *prcWindow = 0);
#endif

extern int sound_active;
extern char *malloc_buffer;

sint32 GetRandomDraw(void);
void SeedRandomDraw(sint32 seed);
sint32 GetRandomControl(void);
void SeedRandomControl(sint32 seed);

void S_SoundStopAllSamples(void);
void S_SoundStopSample(sint32 handle);
void S_SoundSetMasterVolume(int nVolume);

#ifdef __cplusplus
}
#endif

#endif
