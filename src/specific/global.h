#ifndef GLOBAL_H
#define GLOBAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../types.h"

sint32 GetRandomDraw(void);
void SeedRandomDraw(sint32 seed);
sint32 GetRandomControl(void);
void SeedRandomControl(sint32 seed);

#ifdef __cplusplus
}
#endif

#endif
