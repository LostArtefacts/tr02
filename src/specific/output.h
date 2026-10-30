#ifndef OUTPUT_H
#define OUTPUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../3dglodef.h"

extern int wibble_light[WIBBLE_SIZE][32];

void S_CopyScreenToBuffer(void);
void ScreenDump(void);
void ScreenPartialDump();
void FadeWait(void);
void S_ClearScreen();
void S_SetupBelowWater(int underwater);
void S_SetupAboveWater(int underwater);

#ifdef __cplusplus
void ScreenClear(bool tWindow = false);
#endif

#ifdef __cplusplus
}
#endif

#endif
