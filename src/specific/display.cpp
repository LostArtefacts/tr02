#include "global.h"
#include "output.h"
#include "display.h"
#include "../inventry.h"

extern int fade_value;
extern int fade_limit;
extern int fade_adder;

void IncreaseScreenSize(void)
{
    if (screen_sizer == 1.0)
        return;
    screen_sizer += 0.08;
    if (screen_sizer > 1.0)
        screen_sizer = 1.0;
    game_sizer = screen_sizer;
    setup_screen_size();
}

void DecreaseScreenSize(void)
{
    if (screen_sizer == 0.44)
        return;
    screen_sizer -= 0.08;
    if (screen_sizer < 0.44)
        screen_sizer = 0.44;
    game_sizer = screen_sizer;
    setup_screen_size();
}

void TempVideoAdjust(int hires, double sizer)
{
    mode_lock = 1;
    if (sizer == screen_sizer)
        return;
    screen_sizer = sizer;
    setup_screen_size();
}

void TempVideoRemove(void)
{
    mode_lock = 0;
    if (screen_sizer == game_sizer)
        return;
    screen_sizer = game_sizer;
    setup_screen_size();
}
