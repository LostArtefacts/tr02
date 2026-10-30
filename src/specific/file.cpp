#include "global.h"
#include "init.h"
#include "file.h"
#include "../cinema.h"
#include "../savegame.h"
#include "../camera.h"
#include "../effects.h"
#include "../gameflow.h"

extern "C" int LnTextureInfos;
extern "C" int democount;
extern "C" uint32 *demoptr;
extern "C" int demo_loaded;

#define DEMOCOUNT_MAX (30 * 60 * 5)

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

int LoadSoundEffects(HANDLE file)
{
    DWORD read;

    MyReadFile(file, &number_sound_effects, sizeof(sint32), &read, NULL);
    if (number_sound_effects == 0)
        return 1;
    sound_effects = (OBJECT_VECTOR *)game_malloc(sizeof(OBJECT_VECTOR) * number_sound_effects, SOUND_FX);
    if (sound_effects == NULL)
        return 0;
    MyReadFile(file, sound_effects, sizeof(OBJECT_VECTOR) * number_sound_effects, &read, NULL);
    return 1;
}

int LoadCinematic(HANDLE file)
{
    DWORD read;

    MyReadFile(file, &num_cine_frames, sizeof(sint16), &read, NULL);
    if (num_cine_frames)
    {
        cine = (sint16 *)game_malloc(sizeof(sint16) * 8 * num_cine_frames, CINEMATIC_FRAMES);
        MyReadFile(file, cine, sizeof(sint16) * 8 * num_cine_frames, &read, NULL);
        cine_loaded = 1;
    }
    else
        cine_loaded = 0;
    return 1;
}

int LoadDemo(HANDLE file)
{
    sint16 length;
    DWORD read;

    democount = 0;
    demoptr = (uint32 *)game_malloc(DEMOCOUNT_MAX * sizeof(uint32), LOADDEMO_BUFFER);

    MyReadFile(file, &length, sizeof(sint16), &read, NULL);
    if (length)
    {
        MyReadFile(file, demoptr, length, &read, NULL);
        demo_loaded = 1;
    }
    else
        demo_loaded = 0;

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

int Read_Strings(int num_strings, char **strings, char **buffer, DWORD *read, HANDLE file)
{
    int i;
    sint16 size;
    char *strptr;

    MyReadFile(file, LGF_offsets, num_strings * sizeof(sint16), read, NULL);
    MyReadFile(file, &size, sizeof(sint16), read, NULL);
    *buffer = (char *)GlobalAlloc(GMEM_FIXED, size);
    if (!buffer)
        return 0;
    MyReadFile(file, *buffer, size, read, NULL);
    if (gameflow.cyphered_strings)
    {
        strptr = *buffer;
        for (i = 0; i < size; i++)
            *(strptr + i) ^= gameflow.cypher_code;
    }
    for (i = 0; i < num_strings; i++)
        strings[i] = *buffer + LGF_offsets[i];
    return 1;
}
