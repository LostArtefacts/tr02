#include "file.h"

extern "C" int LnTextureInfos;

void AdjustTextureUVs(bool tNew);
int LoadLevel(char *filename, int req_level);

int S_LoadLevelFile(char *fname, int level_number, int type)
{
    S_UnloadLevelFile();
    return LoadLevel(fname, level_number);
}

void S_AdjustTexelCoordinates(void)
{
    if (LnTextureInfos)
        AdjustTextureUVs(false);
}
