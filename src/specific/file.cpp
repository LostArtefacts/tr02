#include "file.h"

extern "C" int LnTextureInfos;

void AdjustTextureUVs(bool tNew);

void S_AdjustTexelCoordinates(void)
{
    if (LnTextureInfos)
        AdjustTextureUVs(false);
}
