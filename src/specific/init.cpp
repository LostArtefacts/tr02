#include "global.h"
#include "init.h"
#include "output.h"
#include <time.h>

void ShutdownGame(void)
{
    if (malloc_buffer)
        GlobalFree((HGLOBAL)malloc_buffer);
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
