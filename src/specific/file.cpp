#include "global.h"
#include "init.h"
#include "file.h"
#include "../cinema.h"
#include "../savegame.h"
#include "../camera.h"
#include "../effects.h"
#include "../gameflow.h"
#include "hwrender.h"
#include "winmain.h"

extern "C" int LnTextureInfos;
extern "C" int democount;
extern "C" uint32 *demoptr;
extern "C" int demo_loaded;

#define DEMOCOUNT_MAX (30 * 60 * 5)
#define MAX_TEXTPAGES 32

extern "C" char LevelFileName[];
extern "C" char *texture_page_ptrs[];

void AdjustTextureUVs(bool tNew);
int LoadLevel(char *filename, int req_level);
BOOL MyReadFile(HANDLE hFile, void *pBuffer, DWORD nNumberOfBytesToRead, DWORD *pNumberOfBytesRead, OVERLAPPED *pOverlapped);

extern "C" DWORD ReadFileBytesCounter;
extern "C" void WinVidSpinMessageLoop(bool tWait);

BOOL MyReadFile(HANDLE hFile, void *pBuffer, DWORD nNumberOfBytesToRead, DWORD *pNumberOfBytesRead, OVERLAPPED *pOverlapped)
{
    ReadFileBytesCounter += nNumberOfBytesToRead;
    if (ReadFileBytesCounter > 16384)
    {
        ReadFileBytesCounter = 0;
        WinVidSpinMessageLoop(false);
    }
    return ReadFile(hFile, pBuffer, nNumberOfBytesToRead, pNumberOfBytesRead, pOverlapped);
}

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

extern "C" int LoadAnimatedTextures(HANDLE file)
{
    int size;
    DWORD read;

    MyReadFile(file, &size, sizeof(sint32), &read, NULL);
    aranges = (sint16 *)game_malloc(size * sizeof(sint16), ANIMATING_TEXTURE_RANGES);
    MyReadFile(file, aranges, size * sizeof(sint16), &read, NULL);
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

void S_UnloadLevelFile(void)
{
    if (SavedAppSettings.RenderMode == RM_Hardware)
        HWR_FreeTexturePages();
    for (int i = 0; i < MAX_TEXTPAGES; ++i)
        texture_page_ptrs[i] = 0;
    LevelFileName[0] = 0;
    LnTextureInfos = 0;
}

void S_AdjustTexelCoordinates(void)
{
    if (LnTextureInfos)
        AdjustTextureUVs(false);
}

extern "C" DWORD LevelFilePalettesOffset, LevelFileTexPagesOffset, LevelFileDepthQOffset;
extern "C" int LoadPalettes(HANDLE file);
extern "C" int LoadDepthQ(HANDLE file);
extern "C" int LoadTexturePages(HANDLE file);
extern "C" void InitColours(void);

extern "C" int S_ReloadLevelGraphics(int tPalDPQ, int tTPages)
{
    HANDLE file;

    if (LevelFileName[0])
    {
        file = CreateFile(LevelFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (file == INVALID_HANDLE_VALUE)
            return 0;
        if (tPalDPQ && SavedAppSettings.RenderMode == RM_Software)
        {
            SetFilePointer(file, LevelFilePalettesOffset, 0, FILE_BEGIN);
            LoadPalettes(file);
            SetFilePointer(file, LevelFileDepthQOffset, 0, FILE_BEGIN);
            LoadDepthQ(file);
        }
        if (tTPages)
        {
            if (SavedAppSettings.RenderMode == RM_Hardware)
                HWR_FreeTexturePages();
            SetFilePointer(file, LevelFileTexPagesOffset, 0, FILE_BEGIN);
            LoadTexturePages(file);
        }
        CloseHandle(file);
    }
    if (tPalDPQ)
        InitColours();
    return 1;
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

extern "C" void ChangeFileNameExtension(char *file_name, char *file_ext)
{
    while (*file_name != '.' && *file_name)
        file_name++;
    *file_name++ = '.';
    *file_name++ = *file_ext++;
    *file_name++ = *file_ext++;
    *file_name = *file_ext;
    file_name[1] = 0;
}

extern "C" uint8 GamePalette16[];

extern "C" int LoadPalettes(HANDLE file)
{
    DWORD read;
    int i;

    MyReadFile(file, game_palette, 768, &read, NULL);
    game_palette[0] = game_palette[1] = game_palette[2] = 0;
    for (i = 3; i < 768; i++)
        game_palette[i] <<= 2;
    MyReadFile(file, GamePalette16, 1024, &read, NULL);
    return 1;
}
