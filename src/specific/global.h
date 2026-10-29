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

sint32 GetRandomDraw(void);
void SeedRandomDraw(sint32 seed);
sint32 GetRandomControl(void);
void SeedRandomControl(sint32 seed);

#ifdef __cplusplus
}
#endif

#endif
