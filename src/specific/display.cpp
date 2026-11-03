#include "standard.h"
#include "global.h"
#include "winmain.h"
#include "output.h"
#include "display.h"
#include "../camera.h"
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

extern "C" sint16 DumpX, DumpY, DumpWidth, DumpHeight;
extern "C" int GameVidWidth, GameVidHeight, GameVidBufWidth, GameVidBufHeight;
extern "C" bool WinVidNeedToResetBuffers;
extern "C" void phd_InitWindow(int x, int y, int width, int height, int nearz, int farz, int view_angle, int scrwidth, int scrheight);

void setup_screen_size(void)
{
    int width;
    int height;
    int xoff;
    int yoff;

    width = (int)(GameVidWidth * screen_sizer);
    height = (int)(GameVidHeight * screen_sizer);
    if (width > GameVidWidth)
        width = GameVidWidth;
    if (height > GameVidHeight)
        height = GameVidHeight;
    xoff = (GameVidWidth - width) / 2;
    yoff = (GameVidHeight - height) / 2;
    phd_InitWindow(xoff, yoff, width, height, 20, 0x5000, 80, GameVidBufWidth, GameVidBufHeight);
    DumpX = xoff;
    DumpY = yoff;
    DumpWidth = width;
    DumpHeight = height;
    WinVidNeedToResetBuffers = true;
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

void S_FadeInInventory(int fade)
{
    if (Inventory_Mode != INV_TITLE_MODE)
        S_CopyScreenToBuffer();
    if (fade)
    {
        fade_value = 16 << 16;
        fade_limit = 24 << 16;
        fade_adder = 32768;
    }
}

void S_FadeOutInventory(int fade)
{
    if (fade)
    {
        fade_value = 24 << 16;
        fade_limit = 16 << 16;
        fade_adder = -32768;
    }
}

#ifdef TR2_EXE
double screen_sizer = 1.0;
double game_sizer = 1.0;
int fade_value = 1048576;
int fade_limit = 1048576;
int fade_adder = 32768;
int mode_lock;
#endif
