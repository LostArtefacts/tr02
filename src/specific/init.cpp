#include "standard.h"
#include "global.h"
#include "init.h"
#include "output.h"
#include "../3dsystem/3d_gen.h"
#include <time.h>
#include "winmain.h"
#include <math.h>

extern "C" char *malloc_ptr;
extern "C" int malloc_size;
extern "C" int malloc_free, malloc_used;
extern "C" void init_game_malloc(void);
extern "C" extern "C" char *game_malloc_strings[];
extern "C" char GameMallocOutOfMemory[];
extern "C" char StringToShow[];
extern "C" void S_ExitSystem(char *message);

extern "C" void game_free(int size);

extern "C" sint16 DumpX, DumpY, DumpWidth, DumpHeight;
extern "C" int GameVidWidth, GameVidHeight;

extern "C" int S_InitialiseSystem(void)
{
    S_SeedRandom();
    DumpX = 0;
    DumpY = 0;
    DumpWidth = GameVidWidth;
    DumpHeight = GameVidHeight;
    CalculateWibbleTable();
    malloc_size = 0x380000;
    return 1;
}

void ShutdownGame(void)
{
    if (malloc_buffer)
        GlobalFree((HGLOBAL)malloc_buffer);
}

void init_game_malloc(void)
{
    malloc_ptr = malloc_buffer;
    malloc_free = malloc_size;
    malloc_used = 0;
}

void *game_malloc(int size, int type)
{
    char *ptr;

    size = (size + 3) & ~3;
    if (size <= malloc_free) {
        ptr = malloc_ptr;
        malloc_free -= size;
        malloc_used += size;
        malloc_ptr += size;
        return ptr;
    }
    wsprintfA(StringToShow, GameMallocOutOfMemory, game_malloc_strings[type], size);
    S_ExitSystem(StringToShow);
    return NULL;
}

extern "C" void game_free(int size)
{
    size = (size + 3) & ~3;
    malloc_ptr -= size;
    malloc_free += size;
    malloc_used -= size;
}

void CalculateWibbleTable(void)
{
    int i, j, sn;
    PHD_ANGLE angle;

    for (i = 0; i < WIBBLE_SIZE; i++)
    {
        angle = (i * 65536) / WIBBLE_SIZE;
        sn = phd_sin(angle);
        wibble_table[i] = (float)(sn * MAX_WIBBLE >> W2V_SHIFT);
        shade_table[i] = sn * MAX_SHADE >> W2V_SHIFT;
        rand_table[i] = (GetRandomDraw() >> 5) - 0x01ff;
        for (j = 0; j < 31; j++)
            wibble_light[i][j] = ((j - 16) * i << 9) / (WIBBLE_SIZE - 1);
    }
}

void S_SeedRandom(void)
{
    struct tm *tptr;
    time_t lt;

    lt = time(NULL);
    tptr = localtime(&lt);
    SeedRandomControl(tptr->tm_sec + (tptr->tm_min * 57) + (tptr->tm_hour * 3543));
    SeedRandomDraw(tptr->tm_sec + (tptr->tm_min * 43) + (tptr->tm_hour * 3477));
}

#ifdef TR2_EXE
sint16 DumpX = 25;
sint16 DumpY = 25;
sint16 DumpWidth = 50;
sint16 DumpHeight;
char *game_malloc_strings[47] = {
    "Temp Alloc",
    "Texture Pages",
    "Mesh Pointers",
    "Meshes",
    "Anims",
    "Structs",
    "Ranges",
    "Commands",
    "Bones",
    "Frames",
    "Room Textures",
    "Room Infos",
    "Room Mesh",
    "Room Door",
    "Room Floor",
    "Room Lights",
    "Room Static Mesh Infos",
    "Floor Data",
    "ITEMS!!",
    "Cameras",
    "Sound FX",
    "Boxes",
    "Overlaps",
    "GroundZone",
    "FlyZone",
    "Animating Texture Ranges",
    "Cinematic Frames",
    "LoadDemo Buffer",
    "SaveDemo Buffer",
    "Cinematic Effects",
    "Mummy Head Turn",
    "Extra Door stuff",
    "Effects_Array",
    "Creature Data",
    "Creature LOT",
    "Sample Infos",
    "Samples",
    "Sample Offsets",
    "Rolling Ball Stuff",
    "Skidoo Stuff",
    "Load Piccy Buffer",
    "FMV Buffers",
    "Polygon Buffers",
    "Order Tables",
    "CLUTs",
    "Texture Infos",
    "Sprite Infos",
};
char GameMallocOutOfMemory[36] = "game_malloc(): OUT OF MEMORY %s %d";
int malloc_size;
char *malloc_ptr;
int malloc_used;
int malloc_free;
int wibble_light[WIBBLE_SIZE][32];
#endif

#ifdef TR2_EXE
extern "C" {
sint16 GameSetupScreenSize[2] = {320, 200};
}
#endif
