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

extern "C" char DemoExtension[];
extern "C" void ChangeFileNameExtension(char *file_name, char *file_ext);

extern "C" void LoadDemoExternal(char *level_name)
{
    char file_name[80];
    HANDLE file;
    DWORD read;

    strcpy(file_name, level_name);
    ChangeFileNameExtension(file_name, DemoExtension);
    file = CreateFileA(file_name, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file != INVALID_HANDLE_VALUE) {
        MyReadFile(file, demoptr, 36000, &read, NULL);
        demo_loaded = read > 0;
        CloseHandle(file);
    }
}

typedef struct {
    uint16 tpage;
    uint16 offset;
    uint16 width;
    uint16 height;
    sint16 x1;
    sint16 y1;
    sint16 x2;
    sint16 y2;
} PHDSPRITESTRUCT;

typedef struct {
    sint16 mesh_number;
    uint16 flags;
    sint16 x_minp, x_maxp, y_minp, y_maxp, z_minp, z_maxp;
    sint16 x_minc, x_maxc, y_minc, y_maxc, z_minc, z_maxc;
} STATIC_INFO;

#define NUMBER_OBJECTS 265

extern "C" PHDSPRITESTRUCT PhdSpriteInfo[];
extern "C" STATIC_INFO StaticObjects[];

extern "C" int LoadSprites(HANDLE file)
{
    int i, j;
    int number;
    DWORD read;

    MyReadFile(file, &number, sizeof(sint32), &read, NULL);
    MyReadFile(file, PhdSpriteInfo, sizeof(PHDSPRITESTRUCT) * number, &read, NULL);
    MyReadFile(file, &number, sizeof(sint32), &read, NULL);
    for (i = 0; i < number; i++)
    {
        MyReadFile(file, &j, sizeof(sint32), &read, NULL);
        if (j < NUMBER_OBJECTS)
        {
            MyReadFile(file, &objects[j].nmeshes, sizeof(sint16), &read, NULL);
            MyReadFile(file, &objects[j].mesh_index, sizeof(sint16), &read, NULL);
            objects[j].loaded = 1;
        }
        else
        {
            j -= NUMBER_OBJECTS;
            SetFilePointer(file, sizeof(sint16), NULL, FILE_CURRENT);
            MyReadFile(file, &StaticObjects[j].mesh_number, sizeof(sint16), &read, NULL);
        }
    }
    return 1;
}

#define NUMBER_ITEMS 256

extern "C" char StringToShow[];
extern "C" char LoadItemsBadObject[];
extern "C" char LoadItemsNoMemory[];
extern "C" char LoadItemsTooMany[];
extern "C" int level_items;
extern "C" void InitialiseItemArray(int numitems);
extern "C" void InitialiseItem(sint16 item_num);

extern "C" int LoadItems(HANDLE file)
{
    sint16 i;
    sint32 number_items;
    ITEM_INFO *item;
    DWORD read;

    MyReadFile(file, &number_items, sizeof(sint32), &read, NULL);
    if (number_items == 0)
        return 1;
    if (number_items > NUMBER_ITEMS)
    {
        lstrcpy(StringToShow, LoadItemsTooMany);
        return 0;
    }
    items = (ITEM_INFO *)game_malloc(sizeof(ITEM_INFO) * NUMBER_ITEMS, ITEMS);
    if (items == NULL)
    {
        lstrcpy(StringToShow, LoadItemsNoMemory);
        return 0;
    }
    level_items = number_items;
    InitialiseItemArray(NUMBER_ITEMS);
    for (i = 0; i < number_items; i++)
    {
        item = &items[i];
        MyReadFile(file, &item->object_number, sizeof(sint16), &read, NULL);
        MyReadFile(file, &item->room_number, sizeof(sint16), &read, NULL);
        MyReadFile(file, &item->pos.x_pos, sizeof(sint32), &read, NULL);
        MyReadFile(file, &item->pos.y_pos, sizeof(sint32), &read, NULL);
        MyReadFile(file, &item->pos.z_pos, sizeof(sint32), &read, NULL);
        MyReadFile(file, &item->pos.y_rot, sizeof(sint16), &read, NULL);
        MyReadFile(file, &item->shade, sizeof(sint16), &read, NULL);
        MyReadFile(file, &item->shadeB, sizeof(sint16), &read, NULL);
        MyReadFile(file, &item->flags, sizeof(sint16), &read, NULL);
        if (item->object_number < 0 || item->object_number >= NUMBER_OBJECTS)
        {
            wsprintf(StringToShow, LoadItemsBadObject, item->object_number, i);
            return 0;
        }
        InitialiseItem(i);
    }
    return 1;
}

#define WORKER3 32
#define SPIDER 36
#define YETI 45
#define SKIDOO_ARMED 51

typedef struct box_info {
    uint8 left, right, top, bottom;
    sint16 height;
    sint16 overlap_index;
} BOX_INFO;

extern "C" char LoadBoxesFlyZone[];
extern "C" char LoadBoxesGroundZone[];
extern "C" char LoadBoxesOverlaps[];
extern "C" char LoadBoxesBoxes[];
extern "C" BOX_INFO *boxes;
extern "C" int number_boxes;
extern "C" uint16 *Overlaps;
extern "C" sint16 *ground_zone[4][2];
extern "C" sint16 *fly_zone[2];

extern "C" int LoadBoxes(HANDLE file)
{
    sint32 i, j, number_overlaps;
    DWORD read;

    MyReadFile(file, &number_boxes, sizeof(sint32), &read, NULL);
    boxes = (BOX_INFO *)game_malloc(sizeof(BOX_INFO) * number_boxes, BOXES);
    MyReadFile(file, boxes, sizeof(BOX_INFO) * number_boxes, &read, NULL);
    if (read != sizeof(BOX_INFO) * number_boxes)
    {
        lstrcpy(StringToShow, LoadBoxesBoxes);
        return 0;
    }
    MyReadFile(file, &number_overlaps, sizeof(sint32), &read, NULL);
    Overlaps = (uint16 *)game_malloc(sizeof(uint16) * number_overlaps, OVERLAPS);
    MyReadFile(file, Overlaps, sizeof(uint16) * number_overlaps, &read, NULL);
    if (read != sizeof(uint16) * number_overlaps)
    {
        lstrcpy(StringToShow, LoadBoxesOverlaps);
        return 0;
    }
    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < 4; j++)
        {
            if (j == 2 || (j == 1 && !objects[SPIDER].loaded && !objects[SKIDOO_ARMED].loaded) || (j == 3 && !objects[YETI].loaded && !objects[WORKER3].loaded))
            {
                SetFilePointer(file, sizeof(sint16) * number_boxes, NULL, FILE_CURRENT);
            }
            else
            {
                ground_zone[j][i] = (sint16 *)game_malloc(sizeof(sint16) * number_boxes, GROUNDZONE);
                MyReadFile(file, ground_zone[j][i], sizeof(sint16) * number_boxes, &read, NULL);
                if (read != sizeof(sint16) * number_boxes)
                {
                    lstrcpy(StringToShow, LoadBoxesGroundZone);
                    return 0;
                }
            }
        }
        fly_zone[i] = (sint16 *)game_malloc(sizeof(sint16) * number_boxes, FLYZONE);
        MyReadFile(file, fly_zone[i], sizeof(sint16) * number_boxes, &read, NULL);
        if (read != sizeof(sint16) * number_boxes)
        {
            lstrcpy(StringToShow, LoadBoxesFlyZone);
            return 0;
        }
    }
    return 1;
}
