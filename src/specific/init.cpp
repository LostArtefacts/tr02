#include "global.h"
#include "init.h"
#include "output.h"
#include <time.h>

extern "C" char *malloc_ptr;
extern "C" int malloc_size;
extern "C" int malloc_free, malloc_used;
extern "C" void init_game_malloc(void);

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
