#include "game.h"

static int flash_state = 0;
static int flash_count = 0;

int FlashIt(void)
{
    if (flash_count)
        flash_count--;
    else
    {
        flash_count = 5;
        flash_state ^= 1;
    }
    return flash_state;
}
