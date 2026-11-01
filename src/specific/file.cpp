#include "global.h"
#include "init.h"
#include "file.h"
#include "../cinema.h"
#include "../savegame.h"
#include "../camera.h"
#include "../effects.h"
#include "../gameflow.h"
#include "../sound.h"
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

void AdjustTextureUVs(bool reset);
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

extern "C" char StringToShow[];
extern "C" {
#include "../room.h"
}

extern "C" char LoadRoomsTooMany[];
extern "C" char LoadRoomsNoMemory[];
extern "C" sint16 number_rooms;
extern "C" sint16 *floor_data;

typedef struct {
    sint32 x, y, z;
    sint16 intensity1, intensity2;
    sint32 fade1, fade2;
} LIGHT_INFO;

typedef struct {
    sint32 x, y, z;
    sint16 y_rot;
    sint16 shade1, shade2;
    sint16 static_number;
} MESH_INFO;

extern "C" int LoadRooms(HANDLE file)
{
    int i;
    sint16 ndoors;
    sint32 size;
    ROOM_INFO *r;
    DWORD read;

    MyReadFile(file, &number_rooms, sizeof(number_rooms), &read, NULL);
    if (number_rooms < 0 || number_rooms > 1024)
    {
        lstrcpy(StringToShow, LoadRoomsTooMany);
        return 0;
    }
    room = (ROOM_INFO *)game_malloc(number_rooms * sizeof(ROOM_INFO), ROOM_INFOS);
    if (room == NULL)
    {
        lstrcpy(StringToShow, LoadRoomsNoMemory);
        return 0;
    }
    for (i = 0, r = room; i < number_rooms; i++, r++)
    {
        MyReadFile(file, &r->x, sizeof(sint32), &read, NULL);
        r->y = 0;
        MyReadFile(file, &r->z, sizeof(sint32), &read, NULL);
        MyReadFile(file, &r->minfloor, sizeof(sint32), &read, NULL);
        MyReadFile(file, &r->maxceiling, sizeof(sint32), &read, NULL);
        MyReadFile(file, &size, sizeof(sint32), &read, NULL);
        r->data = (sint16 *)game_malloc(size * sizeof(sint16), ROOM_MESH);
        MyReadFile(file, r->data, sizeof(sint16) * size, &read, NULL);
        MyReadFile(file, &ndoors, sizeof(sint16), &read, NULL);
        if (ndoors)
        {
            r->door = (sint16 *)game_malloc((ndoors * 16 + 1) * sizeof(sint16), ROOM_DOOR);
            r->door[0] = ndoors;
            MyReadFile(file, &r->door[1], sizeof(sint16) * ndoors * 16, &read, NULL);
        }
        else
            r->door = NULL;
        MyReadFile(file, &r->x_size, sizeof(sint16), &read, NULL);
        MyReadFile(file, &r->y_size, sizeof(sint16), &read, NULL);
        size = (int)r->x_size * r->y_size;
        r->floor = (FLOOR_INFO *)game_malloc(size * sizeof(FLOOR_INFO), ROOM_FLOOR);
        MyReadFile(file, r->floor, sizeof(FLOOR_INFO) * size, &read, NULL);
        MyReadFile(file, &r->ambient, sizeof(sint16), &read, NULL);
        MyReadFile(file, &r->ambient2, sizeof(sint16), &read, NULL);
        MyReadFile(file, &r->lighting, sizeof(sint16), &read, NULL);
        MyReadFile(file, &r->num_lights, sizeof(sint16), &read, NULL);
        if (r->num_lights)
        {
            r->light = game_malloc(r->num_lights * sizeof(LIGHT_INFO), ROOM_LIGHTS);
            MyReadFile(file, r->light, sizeof(LIGHT_INFO) * r->num_lights, &read, NULL);
        }
        else
            r->light = NULL;
        MyReadFile(file, &r->num_meshes, sizeof(sint16), &read, NULL);
        if (r->num_meshes)
        {
            r->mesh = game_malloc(r->num_meshes * sizeof(MESH_INFO), ROOM_STATIC_MESH_INFOS);
            MyReadFile(file, r->mesh, sizeof(MESH_INFO) * r->num_meshes, &read, NULL);
        }
        else
            r->mesh = NULL;
        MyReadFile(file, &r->flipped_room, sizeof(sint16), &read, NULL);
        MyReadFile(file, &r->flags, sizeof(uint16), &read, NULL);
        r->bound_active = 0;
        r->left = phd_winxmax;
        r->top = phd_winymax;
        r->right = r->bottom = 0;
        r->item_number = NO_ITEM;
        r->fx_number = NO_ITEM;
    }
    MyReadFile(file, &size, sizeof(sint32), &read, NULL);
    floor_data = (sint16 *)game_malloc(size * sizeof(sint16), FLOOR_DATA);
    MyReadFile(file, floor_data, sizeof(sint16) * size, &read, NULL);
    return 1;
}
extern "C" int UvAdd;
extern "C" uint8 LabTextureUVFlags[];

#define TexelAdjustMode (*(int *)((char *)&SavedAppSettings + 0x2d))
#define BilinearFiltering (*((bool *)&SavedAppSettings + 0x23))
#define NearestAdjustment (*(int *)((char *)&SavedAppSettings + 0x31))
#define LinearAdjustment (*(int *)((char *)&SavedAppSettings + 0x35))

void AdjustTextureUVs(bool reset)
{
    DWORD i;
    int j, adjust;
    uint8 flags;
    uint8 *flag_ptr;
    uint16 *uv;

    if (reset)
        UvAdd = 0;
    adjust = NearestAdjustment;
    if (SavedAppSettings.RenderMode == RM_Hardware && (TexelAdjustMode == 2 || (TexelAdjustMode == 1 && BilinearFiltering)))
        adjust = LinearAdjustment;
    adjust -= UvAdd;
    flag_ptr = LabTextureUVFlags;
    for (i = 0; i != LnTextureInfos; i++)
    {
        uv = &phdtextinfo[i].u1;
        flags = *flag_ptr++;
        for (j = 0; j < 8; j++)
        {
            if (flags & 1)
                uv[j] -= adjust;
            else
                uv[j] += adjust;
            flags >>= 1;
        }
    }
    UvAdd += adjust;
}


typedef struct {
    sint16 mesh_number;
    uint16 flags;
    sint16 x_minp, x_maxp, y_minp, y_maxp, z_minp, z_maxp;
    sint16 x_minc, x_maxc, y_minc, y_maxc, z_minc, z_maxc;
} STATIC_INFO;

extern "C" STATIC_INFO StaticObjects[];

typedef struct {
    sint16 goal_anim_state;
    sint16 number_ranges;
    sint16 range_index;
} CHANGE_STRUCT;

typedef struct {
    sint16 start_frame;
    sint16 end_frame;
    sint16 link_anim_num;
    sint16 link_frame_num;
} RANGE_STRUCT;

extern "C" sint16 *Meshes;
extern "C" CHANGE_STRUCT *changes;
extern "C" RANGE_STRUCT *ranges;
extern "C" sint16 *commands;
extern "C" sint16 *AnimFrames;
extern "C" char LoadObjectsTooManyTextures[];
extern "C" void InitialiseObjects(void);

extern "C" int LoadObjects(HANDLE file)
{
    int i, j, k;
    sint32 number, size, number_anims;
    DWORD read;
    uint16 *uv;
    char flags;

    MyReadFile(file, &size, sizeof(sint32), &read, NULL);
    Meshes = (sint16 *)game_malloc(size * sizeof(sint16), MESHES);
    MyReadFile(file, Meshes, sizeof(sint16) * size, &read, NULL);
    MyReadFile(file, &number, sizeof(sint32), &read, NULL);
    meshes = (sint16 **)game_malloc(number * sizeof(sint16 *), MESH_POINTERS);
    MyReadFile(file, meshes, sizeof(sint16 *) * number, &read, NULL);
    for (i = 0; i < number; i++)
        meshes[i] = Meshes + (int)meshes[i] / 2;
    MyReadFile(file, &number_anims, sizeof(sint32), &read, NULL);
    anims = (ANIM_STRUCT *)game_malloc(number_anims * sizeof(ANIM_STRUCT), ANIMS);
    MyReadFile(file, anims, sizeof(ANIM_STRUCT) * number_anims, &read, NULL);
    MyReadFile(file, &number, sizeof(sint32), &read, NULL);
    changes = (CHANGE_STRUCT *)game_malloc(number * sizeof(CHANGE_STRUCT), STRUCTS);
    MyReadFile(file, changes, sizeof(CHANGE_STRUCT) * number, &read, NULL);
    MyReadFile(file, &number, sizeof(sint32), &read, NULL);
    ranges = (RANGE_STRUCT *)game_malloc(number * sizeof(RANGE_STRUCT), RANGES);
    MyReadFile(file, ranges, sizeof(RANGE_STRUCT) * number, &read, NULL);
    MyReadFile(file, &size, sizeof(sint32), &read, NULL);
    commands = (sint16 *)game_malloc(size * sizeof(sint16), COMMANDS);
    MyReadFile(file, commands, sizeof(sint16) * size, &read, NULL);
    MyReadFile(file, &size, sizeof(sint32), &read, NULL);
    bones = (sint32 *)game_malloc(size * sizeof(sint32), BONES);
    MyReadFile(file, bones, sizeof(sint32) * size, &read, NULL);
    MyReadFile(file, &size, sizeof(sint32), &read, NULL);
    AnimFrames = (sint16 *)game_malloc(size * sizeof(sint16), FRAMES);
    MyReadFile(file, AnimFrames, sizeof(sint16) * size, &read, NULL);
    for (i = 0; i < number_anims; i++)
        anims[i].frame_ptr = (sint16 *)((long)anims[i].frame_ptr + (long)AnimFrames);
    MyReadFile(file, &number, sizeof(sint32), &read, NULL);
    for (i = 0; i < number; i++)
    {
        MyReadFile(file, &j, sizeof(sint32), &read, NULL);
        MyReadFile(file, &objects[j].nmeshes, sizeof(sint16), &read, NULL);
        MyReadFile(file, &objects[j].mesh_index, sizeof(sint16), &read, NULL);
        MyReadFile(file, &objects[j].bone_index, sizeof(sint32), &read, NULL);
        MyReadFile(file, &size, sizeof(sint32), &read, NULL);
        objects[j].frame_base = (sint16 *)((long)size + (long)AnimFrames);
        MyReadFile(file, &objects[j].anim_index, sizeof(sint16), &read, NULL);
        objects[j].loaded = 1;
    }
    InitialiseObjects();
    MyReadFile(file, &number, sizeof(sint32), &read, NULL);
    for (i = 0; i < number; i++)
    {
        MyReadFile(file, &j, sizeof(sint32), &read, NULL);
        MyReadFile(file, &StaticObjects[j].mesh_number, sizeof(sint16), &read, NULL);
        MyReadFile(file, &StaticObjects[j].x_minp, sizeof(sint16) * 6, &read, NULL);
        MyReadFile(file, &StaticObjects[j].x_minc, sizeof(sint16) * 6, &read, NULL);
        MyReadFile(file, &StaticObjects[j].flags, sizeof(uint16), &read, NULL);
    }
    MyReadFile(file, &number, sizeof(sint32), &read, NULL);
    if (number > 2048)
    {
        lstrcpy(StringToShow, LoadObjectsTooManyTextures);
        return 0;
    }
    MyReadFile(file, phdtextinfo, sizeof(PHDTEXTURESTRUCT) * number, &read, NULL);
    LnTextureInfos = number;
    for (number = 0; number < LnTextureInfos; number++)
    {
        uv = &phdtextinfo[number].u1;
        flags = 0;
        for (k = 0; k < 8; k++)
        {
            if (*uv & 0x80)
            {
                *uv |= 0xff;
                flags |= 1 << k;
            }
            else
                *uv &= 0xff00;
            uv++;
        }
        LabTextureUVFlags[number] = flags;
    }
    AdjustTextureUVs(true);
    return 1;
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

extern "C" int sound_active;
extern "C" int SampleInfoCount;
extern "C" char StringToShow[];
extern "C" char MainSfxFileName[];
extern "C" char MainSfxOpenFailed[];
extern "C" bool WinSndIsSoundEnabled(void);
extern "C" void WinSndFreeAllSamples(void);
extern "C" bool WinSndMakeSample(int nSample, WAVEFORMATEX *pWF, void *pWaveData, DWORD dwWaveLength);
extern "C" char *GetFullPath(char *path);
extern "C" void game_free(int size, int type);

extern "C" int LoadSamples(HANDLE file)
{
    DWORD read;
    int nSamples;
    int required_samples[500];
    int required;
    int sample;
    char *pWaveData;
    DWORD dwHeader[11];
    DWORD dwWaveLength;
    DWORD dwStoredLength;
    WAVEFORMATEX *pWF;
    HANDLE sfx;

    sound_active = 0;
    if (!WinSndIsSoundEnabled())
        return 1;

    WinSndFreeAllSamples();

    MyReadFile(file, sample_lut, sizeof(sint16) * 370, &read, NULL);
    MyReadFile(file, &SampleInfoCount, sizeof(int), &read, NULL);
    if (!SampleInfoCount)
        return 0;

    sample_infos = (SAMPLE_INFO *)game_malloc(sizeof(SAMPLE_INFO) * SampleInfoCount, 35);
    MyReadFile(file, sample_infos, sizeof(SAMPLE_INFO) * SampleInfoCount, &read, NULL);

    MyReadFile(file, &nSamples, sizeof(int), &read, NULL);
    if (!nSamples)
        return 0;

    MyReadFile(file, required_samples, sizeof(int) * nSamples, &read, NULL);

    sfx = CreateFile(GetFullPath(MainSfxFileName), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (sfx == INVALID_HANDLE_VALUE)
    {
        wsprintf(StringToShow, MainSfxOpenFailed);
        return 0;
    }

    required = 0;
    sample = 0;
    pWF = (WAVEFORMATEX *)(dwHeader + 5);
    while (required < nSamples)
    {
        MyReadFile(sfx, dwHeader, sizeof(dwHeader), &read, NULL);
        if (dwHeader[0] != MAKEFOURCC('R', 'I', 'F', 'F') || dwHeader[2] != MAKEFOURCC('W', 'A', 'V', 'E') || dwHeader[9] != MAKEFOURCC('d', 'a', 't', 'a'))
            return 0;

        pWF->cbSize = 0;
        dwWaveLength = dwHeader[10];
        dwStoredLength = (dwWaveLength + 1) & ~1;

        if (required_samples[required] != sample)
            SetFilePointer(sfx, dwStoredLength, 0, FILE_CURRENT);
        else
        {
            pWaveData = (char *)game_malloc(dwStoredLength, 36);
            MyReadFile(sfx, pWaveData, dwStoredLength, &read, NULL);
            if (!WinSndMakeSample(required, pWF, pWaveData, dwWaveLength))
                return 0;
            game_free(dwStoredLength, 36);
            required++;
        }

        sample++;
    }

    CloseHandle(sfx);
    sound_active = 1;
    return 1;
}

extern "C" DWORD LevelFilePalettesOffset, LevelFileTexPagesOffset, LevelFileDepthQOffset;
extern "C" char LoadLevelOpenFailed[];
extern "C" char LoadLevelTooNew[];
extern "C" char LoadLevelOutOfDate[];
extern "C" void init_game_malloc(void);
extern "C" int LoadPalettes(HANDLE file);
extern "C" int LoadTexturePages(HANDLE file);
extern "C" int LoadRooms(HANDLE file);
extern "C" int LoadObjects(HANDLE file);
extern "C" int LoadSprites(HANDLE file);
extern "C" int LoadBoxes(HANDLE file);
extern "C" int LoadItems(HANDLE file);
extern "C" int LoadDepthQ(HANDLE file);
extern "C" void LoadDemoExternal(char *level_name);

int LoadLevel(char *filename, int req_level)
{
    DWORD read;
    HANDLE file;
    int version;
    int level;

    filename = GetFullPath(filename);
    strcpy(LevelFileName, filename);

    init_game_malloc();

    file = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (file == INVALID_HANDLE_VALUE)
    {
        wsprintf(StringToShow, LoadLevelOpenFailed, filename, req_level);
        return 0;
    }

    MyReadFile(file, &version, sizeof(int), &read, NULL);
    if (version != 45)
    {
        if (version < 45)
            wsprintf(StringToShow, LoadLevelOutOfDate, req_level, filename, version);
        else
            wsprintf(StringToShow, LoadLevelTooNew, req_level, filename, version);
        return 0;
    }

    LevelFilePalettesOffset = SetFilePointer(file, 0, 0, FILE_CURRENT);
    if (!LoadPalettes(file))
        return 0;

    LevelFileTexPagesOffset = SetFilePointer(file, 0, 0, FILE_CURRENT);
    if (!LoadTexturePages(file))
        return 0;

    MyReadFile(file, &level, sizeof(int), &read, NULL);

    if (!LoadRooms(file))
        return 0;
    if (!LoadObjects(file))
        return 0;
    if (!LoadSprites(file))
        return 0;
    if (!LoadCameras(file))
        return 0;
    if (!LoadSoundEffects(file))
        return 0;
    if (!LoadBoxes(file))
        return 0;
    if (!LoadAnimatedTextures(file))
        return 0;
    if (!LoadItems(file))
        return 0;

    LevelFileDepthQOffset = SetFilePointer(file, 0, 0, FILE_CURRENT);
    if (!LoadDepthQ(file))
        return 0;
    if (!LoadCinematic(file))
        return 0;
    if (!LoadDemo(file))
        return 0;
    if (!LoadSamples(file))
        return 0;

    LoadDemoExternal(filename);
    CloseHandle(file);
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

extern "C" uint32 GF_ScriptVersion;
extern "C" char GF_Description[];
extern "C" sint16 *GF_sequence_buffer;
extern "C" char **GF_PictureFilesStringTable, **GF_titlefilenames, **GF_FmvFilesStringTable;
extern "C" char **GF_levelfilenames, **GF_cutscenefilenames;
extern "C" char *GF_LevelNamesStringBuffer, *GF_PictureFilesStringBuffer, *GF_TitleFilesStringBuffer;
extern "C" char *GF_FmvFilesStringBuffer, *GF_LevelFilesStringBuffer, *GF_CutsFilesStringBuffer;
extern "C" char *GF_GameStringBuffer, *GF_SpecificStringBuffer;
extern "C" char **GF_Puzzle1Strings, **GF_Puzzle2Strings, **GF_Puzzle3Strings, **GF_Puzzle4Strings;
extern "C" char **GF_Pickup1Strings, **GF_Pickup2Strings;
extern "C" char **GF_Key1Strings, **GF_Key2Strings, **GF_Key3Strings, **GF_Key4Strings;
extern "C" char *GF_Puzzle1StringBuffer, *GF_Puzzle2StringBuffer, *GF_Puzzle3StringBuffer, *GF_Puzzle4StringBuffer;
extern "C" char *GF_Pickup1StringBuffer, *GF_Pickup2StringBuffer;
extern "C" char *GF_Key1StringBuffer, *GF_Key2StringBuffer, *GF_Key3StringBuffer, *GF_Key4StringBuffer;

extern "C" int S_LoadGameFlow(char *filename)
{
    DWORD read;
    HANDLE file;
    sint16 number, size;
    int i;

    filename = GetFullPath(filename);
    file = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE)
        return 0;

    MyReadFile(file, &GF_ScriptVersion, sizeof(uint32), &read, NULL);
    if (GF_ScriptVersion != 3)
        return 0;
    MyReadFile(file, GF_Description, 256, &read, NULL);
    MyReadFile(file, &size, sizeof(sint16), &read, NULL);
    if (size != sizeof(GAMEFLOW_INFO))
        return 0;
    MyReadFile(file, &gameflow, sizeof(GAMEFLOW_INFO), &read, NULL);

    GF_Level_Names = (char **)GlobalAlloc(GMEM_FIXED, gameflow.num_levels * sizeof(char *));
    if (!GF_Level_Names)
        return 0;
    if (!Read_Strings(gameflow.num_levels, GF_Level_Names, &GF_LevelNamesStringBuffer, &read, file))
        return 0;

    GF_PictureFilesStringTable = (char **)GlobalAlloc(GMEM_FIXED, gameflow.num_picfiles * sizeof(char *));
    if (!GF_PictureFilesStringTable)
        return 0;
    if (!Read_Strings(gameflow.num_picfiles, GF_PictureFilesStringTable, &GF_PictureFilesStringBuffer, &read, file))
        return 0;

    GF_titlefilenames = (char **)GlobalAlloc(GMEM_FIXED, gameflow.num_titlefiles * sizeof(char *));
    if (!GF_titlefilenames)
        return 0;
    if (!Read_Strings(gameflow.num_titlefiles, GF_titlefilenames, &GF_TitleFilesStringBuffer, &read, file))
        return 0;

    GF_FmvFilesStringTable = (char **)GlobalAlloc(GMEM_FIXED, gameflow.num_fmvfiles * sizeof(char *));
    if (!GF_FmvFilesStringTable)
        return 0;
    if (!Read_Strings(gameflow.num_fmvfiles, GF_FmvFilesStringTable, &GF_FmvFilesStringBuffer, &read, file))
        return 0;

    GF_levelfilenames = (char **)GlobalAlloc(GMEM_FIXED, gameflow.num_levels * sizeof(char *));
    if (!GF_levelfilenames)
        return 0;
    if (!Read_Strings(gameflow.num_levels, GF_levelfilenames, &GF_LevelFilesStringBuffer, &read, file))
        return 0;

    GF_cutscenefilenames = (char **)GlobalAlloc(GMEM_FIXED, gameflow.num_cutfiles * sizeof(char *));
    if (!GF_cutscenefilenames)
        return 0;
    if (!Read_Strings(gameflow.num_cutfiles, GF_cutscenefilenames, &GF_CutsFilesStringBuffer, &read, file))
        return 0;

    MyReadFile(file, LGF_offsets, sizeof(sint16) * (gameflow.num_levels + 1), &read, NULL);
    MyReadFile(file, &size, sizeof(sint16), &read, NULL);
    GF_sequence_buffer = (sint16 *)GlobalAlloc(GMEM_FIXED, size);
    if (!GF_sequence_buffer)
        return 0;
    MyReadFile(file, GF_sequence_buffer, size, &read, NULL);
    GF_frontendSequence = GF_sequence_buffer;
    for (i = 0; i < gameflow.num_levels; i++)
        GF_level_sequence_list[i] = GF_sequence_buffer + LGF_offsets[i + 1] / 2;

    if (gameflow.num_demos)
        MyReadFile(file, GF_valid_demos, sizeof(sint16) * gameflow.num_demos, &read, NULL);

    MyReadFile(file, &number, sizeof(sint16), &read, NULL);
    if (number != 89)
        return 0;

    GF_GameStrings = (char **)GlobalAlloc(GMEM_FIXED, 89 * sizeof(char *));
    if (!GF_GameStrings)
        return 0;
    if (!Read_Strings(89, GF_GameStrings, &GF_GameStringBuffer, &read, file))
        return 0;

    GF_PCStrings = (char **)GlobalAlloc(GMEM_FIXED, 41 * sizeof(char *));
    if (!GF_PCStrings)
        return 0;
    if (!Read_Strings(41, GF_PCStrings, &GF_SpecificStringBuffer, &read, file))
        return 0;

    GF_Puzzle1Strings = (char **)GlobalAlloc(GMEM_FIXED, gameflow.num_levels * sizeof(char *));
    if (!GF_Puzzle1Strings)
        return 0;
    if (!Read_Strings(gameflow.num_levels, GF_Puzzle1Strings, &GF_Puzzle1StringBuffer, &read, file))
        return 0;

    GF_Puzzle2Strings = (char **)GlobalAlloc(GMEM_FIXED, gameflow.num_levels * sizeof(char *));
    if (!GF_Puzzle2Strings)
        return 0;
    if (!Read_Strings(gameflow.num_levels, GF_Puzzle2Strings, &GF_Puzzle2StringBuffer, &read, file))
        return 0;

    GF_Puzzle3Strings = (char **)GlobalAlloc(GMEM_FIXED, gameflow.num_levels * sizeof(char *));
    if (!GF_Puzzle3Strings)
        return 0;
    if (!Read_Strings(gameflow.num_levels, GF_Puzzle3Strings, &GF_Puzzle3StringBuffer, &read, file))
        return 0;

    GF_Puzzle4Strings = (char **)GlobalAlloc(GMEM_FIXED, gameflow.num_levels * sizeof(char *));
    if (!GF_Puzzle4Strings)
        return 0;
    if (!Read_Strings(gameflow.num_levels, GF_Puzzle4Strings, &GF_Puzzle4StringBuffer, &read, file))
        return 0;

    GF_Pickup1Strings = (char **)GlobalAlloc(GMEM_FIXED, gameflow.num_levels * sizeof(char *));
    if (!GF_Pickup1Strings)
        return 0;
    if (!Read_Strings(gameflow.num_levels, GF_Pickup1Strings, &GF_Pickup1StringBuffer, &read, file))
        return 0;

    GF_Pickup2Strings = (char **)GlobalAlloc(GMEM_FIXED, gameflow.num_levels * sizeof(char *));
    if (!GF_Pickup2Strings)
        return 0;
    if (!Read_Strings(gameflow.num_levels, GF_Pickup2Strings, &GF_Pickup2StringBuffer, &read, file))
        return 0;

    GF_Key1Strings = (char **)GlobalAlloc(GMEM_FIXED, gameflow.num_levels * sizeof(char *));
    if (!GF_Key1Strings)
        return 0;
    if (!Read_Strings(gameflow.num_levels, GF_Key1Strings, &GF_Key1StringBuffer, &read, file))
        return 0;

    GF_Key2Strings = (char **)GlobalAlloc(GMEM_FIXED, gameflow.num_levels * sizeof(char *));
    if (!GF_Key2Strings)
        return 0;
    if (!Read_Strings(gameflow.num_levels, GF_Key2Strings, &GF_Key2StringBuffer, &read, file))
        return 0;

    GF_Key3Strings = (char **)GlobalAlloc(GMEM_FIXED, gameflow.num_levels * sizeof(char *));
    if (!GF_Key3Strings)
        return 0;
    if (!Read_Strings(gameflow.num_levels, GF_Key3Strings, &GF_Key3StringBuffer, &read, file))
        return 0;

    GF_Key4Strings = (char **)GlobalAlloc(GMEM_FIXED, gameflow.num_levels * sizeof(char *));
    if (!GF_Key4Strings)
        return 0;
    if (!Read_Strings(gameflow.num_levels, GF_Key4Strings, &GF_Key4StringBuffer, &read, file))
        return 0;

    CloseHandle(file);
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

extern "C" char Var4d9358[];

extern "C" char *GetFullPath(char *path)
{
    wsprintf(Var4d9358, "%s", path);
    return Var4d9358;
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

#define NUMBER_OBJECTS 265

extern "C" PHDSPRITESTRUCT PhdSpriteInfo[];

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

typedef struct {
    uint8 red;
    uint8 green;
    uint8 blue;
} PAL_RGB;

extern "C" uint8 DepthQTable[32][256];
extern "C" uint8 DepthQIndex[256];
extern "C" uint8 GouraudTable[256][32];
extern "C" uint8 water_palette[];
extern "C" int wet;
extern "C" bool GameVid_IsWindowedVga;
extern "C" void CopyBitmapPalette(uint8 *src_palette, uint8 *bitmap, int size, uint8 *dst_palette);
extern "C" void SyncSurfacePalettes(uint8 *src, uint32 width, uint32 height, int srcPitch, uint8 *srcPalette, uint8 *dst, int dstPitch, uint8 *dstPalette, bool preserveSysPalette);
extern "C" uint8 S_COLOUR(int r, int g, int b);

extern "C" int LoadDepthQ(HANDLE file)
{
    int i;
    int j;
    DWORD read;
    uint8 palette[768];

    MyReadFile(file, DepthQTable, 32 * 256, &read, NULL);
    for (i = 0; i < 33; i++)
        DepthQTable[i][0] = 0;
    if (GameVid_IsWindowedVga) {
        CopyBitmapPalette(game_palette, DepthQTable[0], 32 * 256, palette);
        SyncSurfacePalettes(DepthQTable[0], 256, 32, 256, game_palette, DepthQTable[0], 256, palette, true);
        memcpy(game_palette, palette, 768);
        for (i = 0, j = 0; i < 256; i++, j += 3)
            DepthQIndex[i] = S_COLOUR(game_palette[j] >> 1, game_palette[j + 1] >> 1, game_palette[j + 2] >> 1);
    } else {
        memcpy(DepthQIndex, DepthQTable[24], 256);
    }
    for (i = 0; i < 32; i++) {
        for (j = 0; j < 256; j++)
            GouraudTable[j][i] = DepthQTable[i][j];
    }
    wet = 0;
    for (i = 0; i < 768; i += 3) {
        water_palette[i] = (int)(game_palette[i] * 2) / 3;
        water_palette[i + 1] = (int)(game_palette[i + 1] * 2) / 3;
        water_palette[i + 2] = game_palette[i + 2];
    }
    return 1;
}

typedef struct {
    BYTE pixelFormat[32];
    BYTE unknown[48];
    int bpp;
} TEXTURE_FORMAT;

void HWR_LoadTexturePages(int nPages, uint8 *pImage, uint8 *pPalette);

extern "C" TEXTURE_FORMAT TextureFormat;
extern "C" int HwrTexturePagesCount;

extern "C" int LoadTexturePages(HANDLE file)
{
    DWORD read;
    char *base;
    int i;
    int num_pages;
    bool t16BitTextures;
    int nBytesPerPage;

    MyReadFile(file, &num_pages, sizeof(int), &read, NULL);
    if (SavedAppSettings.RenderMode == RM_Software) {
        for (i = 0; i < num_pages; i++) {
            if (!texture_page_ptrs[i])
                texture_page_ptrs[i] = (char *)game_malloc(65536, TEXTURE_PAGES);
            MyReadFile(file, texture_page_ptrs[i], 65536, &read, NULL);
        }
        SetFilePointer(file, num_pages * 131072, NULL, FILE_CURRENT);
        return 1;
    }
    t16BitTextures = TextureFormat.bpp >= 16;
    nBytesPerPage = t16BitTextures ? 0x20000 : 0x10000;
    base = (char *)GlobalAlloc(GMEM_FIXED, num_pages * nBytesPerPage);
    if (!base)
        return 0;
    if (t16BitTextures) {
        SetFilePointer(file, num_pages * 65536, NULL, FILE_CURRENT);
        for (i = 0; i < num_pages; i++)
            MyReadFile(file, base + i * nBytesPerPage, nBytesPerPage, &read, NULL);
        HWR_LoadTexturePages(num_pages, (uint8 *)base, NULL);
    } else {
        for (i = 0; i < num_pages; i++)
            MyReadFile(file, base + i * nBytesPerPage, nBytesPerPage, &read, NULL);
        SetFilePointer(file, num_pages * 131072, NULL, FILE_CURRENT);
        HWR_LoadTexturePages(num_pages, (uint8 *)base, game_palette);
    }
    HwrTexturePagesCount = num_pages;
    GlobalFree(base);
    return 1;
}

#ifdef TR2_EXE
char LoadRoomsTooMany[] = "LoadRoom(): Too many rooms";
char LoadRoomsNoMemory[] = "LoadRoom(): Could not allocate memory for rooms";
char LoadObjectsTooManyTextures[] = "Too many Textures in level";
char LoadItemsBadObject[] = "LoadItems(): Bad Object number (%d) on Item %d";
char LoadItemsNoMemory[] = "LoadItems(): Unable to allocate memory for 'items'";
char LoadItemsTooMany[] = "LoadItems(): Too Many Items being Loaded!!";
char LoadBoxesFlyZone[] = "LoadBoxes(): Unable to load 'fly_zone'";
char LoadBoxesGroundZone[] = "LoadBoxes(): Unable to load 'ground_zone'";
char LoadBoxesOverlaps[] = "LoadBoxes(): Unable to load box overlaps";
char LoadBoxesBoxes[] = "LoadBoxes(): Unable to load boxes";
char DemoExtension[] = "DEM";
char MainSfxOpenFailed[] = "Could not open MAIN.SFX file";
char MainSfxFileName[] = "data\\main.sfx";
char LoadLevelTooNew[] = "FATAL: Level %d (%s) requires a new TOMB2.EXE (version %d) to run";
char LoadLevelOutOfDate[] = "FATAL: Level %d (%s) is OUT OF DATE (version %d). COPY NEW EDITORS AND REMAKE LEVEL";
char LoadLevelOpenFailed[] = "LoadLevel(): Could not open %s (level %d)";
uint8 GouraudTable[256][32];
uint8 DepthQIndex[256];
int demo_loaded;
uint32 GF_ScriptVersion;
char GF_Description[256];
int number_cameras;
int UvAdd;
int HwrTexturePagesCount;
char Var4d9358[136];
uint8 LabTextureUVFlags[2048];
DWORD ReadFileBytesCounter;
DWORD LevelFilePalettesOffset;
DWORD LevelFileTexPagesOffset;
sint16 LGF_offsets[200];
char LevelFileName[256];
int LnTextureInfos;
DWORD LevelFileDepthQOffset;
uint8 water_palette[768];
sint16 *aranges;
uint8 game_palette[768];
int SampleInfoCount;
char **GF_Puzzle1Strings;
char **GF_cutscenefilenames;
GAMEFLOW_INFO gameflow;
char **GF_Puzzle4Strings;
char *GF_Pickup1StringBuffer;
char *GF_FmvFilesStringBuffer;
char *GF_Key1StringBuffer;
sint16 *GF_frontendSequence;
char **GF_Key2Strings;
char *GF_CutsFilesStringBuffer;
char *GF_Key4StringBuffer;
sint16 *GF_sequence_buffer;
char *GF_Key2StringBuffer;
char *GF_Pickup2StringBuffer;
char *GF_PictureFilesStringBuffer;
char **GF_Key4Strings;
char **GF_Pickup1Strings;
char **GF_Puzzle2Strings;
char *GF_LevelFilesStringBuffer;
char **GF_PictureFilesStringTable;
char **GF_Key1Strings;
char *GF_Puzzle1StringBuffer;
char *GF_LevelNamesStringBuffer;
char **GF_GameStrings;
char *GF_SpecificStringBuffer;
char *GF_GameStringBuffer;
char **GF_Key3Strings;
char **GF_Puzzle3Strings;
char **GF_Level_Names;
sint16 *GF_level_sequence_list[24];
char *GF_Puzzle2StringBuffer;
char **GF_Pickup2Strings;
char **GF_titlefilenames;
char *GF_TitleFilesStringBuffer;
char **GF_PCStrings;
char **GF_levelfilenames;
sint16 GF_valid_demos[24];
char *GF_Puzzle4StringBuffer;
char **GF_FmvFilesStringTable;
char *GF_Puzzle3StringBuffer;
char *GF_Key3StringBuffer;
int number_sound_effects;
OBJECT_VECTOR *sound_effects;
RANGE_STRUCT *ranges;
STATIC_INFO StaticObjects[50];
CHANGE_STRUCT *changes;
sint16 number_rooms;
sint16 cine_loaded;
sint16 num_cine_frames;
BOX_INFO *boxes;
int number_boxes;
#endif
