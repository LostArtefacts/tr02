#include "include.h"
#include "global.h"

extern "C" {
extern bool BGND_PictureIsReady;
void BGND_Free(void);
extern int BGND_TexturePageIndexes[5];
extern DWORD BGND_PageHandles[5];
}

extern "C" DWORD GetTexturePageHandle(int nHandle);

extern "C" void BGND_GetPageHandles(void)
{
    DWORD i;

    for (i = 0; i < 5; i++)
    {
        if (BGND_TexturePageIndexes[i] >= 0)
            BGND_PageHandles[i] = GetTexturePageHandle(BGND_TexturePageIndexes[i]);
        else
            BGND_PageHandles[i] = 0;
    }
}

extern "C" void Func444560(void)
{
    BGND_Free();
    BGND_PictureIsReady = false;
}
