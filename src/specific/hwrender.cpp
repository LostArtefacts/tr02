#include "include.h"
#include "global.h"
#include "hwrender.h"

void HWR_BeginScene()
{
    HWR_GetAllTextureHandles();
    DX_DoFlipWait();
    G_Direct3DDevice->BeginScene();
}
