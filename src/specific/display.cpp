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
