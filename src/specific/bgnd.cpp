#include "include.h"
#include "global.h"

extern "C" {
extern bool BGND_PictureIsReady;
void BGND_Free(void);
}

extern "C" void Func444560(void)
{
    BGND_Free();
    BGND_PictureIsReady = false;
}
