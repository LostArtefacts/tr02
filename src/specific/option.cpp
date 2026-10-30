#include "../3dglodef.h"
#include "../invdata.h"
#include "input.h"

void DefaultConflict(void)
{
    int i, j;
    sint16 key;

    for (i = 0; i < 13; i++)
    {
        key = layout[0][i];
        conflict[i] = 0;
        for (j = 0; j < 13; j++)
        {
            if (key == layout[1][j])
            {
                conflict[i] = 1;
                break;
            }
        }
    }
}

int GetRenderHeight(void)
{
    return phd_winheight;
}

int GetRenderWidth(void)
{
    return phd_winwidth;
}
