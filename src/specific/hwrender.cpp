#include "include.h"
#include "global.h"
#include "hwrender.h"

#define MAX_TLVERTICES 8192

extern "C" D3DTLVERTEX VertexBuffer[MAX_TLVERTICES];
extern "C" int LanTextureHandle[32];

void HWR_BeginScene()
{
    HWR_GetAllTextureHandles();
    DX_DoFlipWait();
    G_Direct3DDevice->BeginScene();
}

bool HWR_Init()
{
    Zero(VertexBuffer);
    for (int nTexture = 0; nTexture < ArraySize(LanTextureHandle); ++nTexture)
        LanTextureHandle[nTexture] = -1;

    return true;
}
