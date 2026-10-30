#include "global.h"
#include "init.h"
#include "file.h"
#include "../cinema.h"
#include "../savegame.h"
#include "../camera.h"
#include "../effects.h"
#include "../gameflow.h"

extern "C" int LnTextureInfos;

void AdjustTextureUVs(bool tNew);
int LoadLevel(char *filename, int req_level);
BOOL MyReadFile(HANDLE hFile, void *pBuffer, DWORD nNumberOfBytesToRead, DWORD *pNumberOfBytesRead, OVERLAPPED *pOverlapped);

int LoadCameras(HANDLE file)
{
    DWORD read;

    MyReadFile(file, &number_cameras, sizeof(sint32), &read, NULL);
    if (number_cameras == 0)
        return 1;
    camera.fixed = (OBJECT_VECTOR *)game_malloc(sizeof(OBJECT_VECTOR) * number_cameras, CAMERAS);
    if (camera.fixed == NULL)
        return 0;
    MyReadFile(file, camera.fixed, sizeof(OBJECT_VECTOR) * number_cameras, &read, NULL);
    return 1;
}

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

extern sint16 LGF_offsets[200];
