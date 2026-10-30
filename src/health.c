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

extern int OverlayStatus;

void DrawAssaultTimer(void);
void DrawHealthBar(int flash_state);
void DrawAirBar(int flash_state);
void DrawAmmoInfo(void);
void DrawPickups(int timed);
void DrawModeInfo(void);
void T_DrawText(void);

void DrawGameInfo(int timed)
{
    int flash_state;

    DrawAmmoInfo();
    DrawModeInfo();
    if (OverlayStatus > 0)
    {
        flash_state = FlashIt();
        DrawHealthBar(flash_state);
        DrawAirBar(flash_state);
        DrawPickups(timed);
        DrawAssaultTimer();
    }
    T_DrawText();
}
