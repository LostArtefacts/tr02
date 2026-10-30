#include "game.h"
#include "larafire.h"

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

void MakeAmmoString(char *string)
{
    char *c;

    for (c = string; *c != 0; c++)
    {
        if (*c == 32)
            continue;
        else if (*c - 'A' >= 0)
            *c += 12 - 'A';
        else
            *c += 1 - '0';
    }
}

#define NUM_PU 12

typedef struct {
    sint16 duration;
    sint16 sprnum;
} DISPLAYPU;

static DISPLAYPU pickups[NUM_PU];

void InitialisePickUpDisplay(void)
{
    int i;

    for (i = 0; i < NUM_PU; i++)
        pickups[i].duration = 0;
}

#define LARA_HITPOINTS 1000

extern int HealthBarTimer;
static int old_hitpoints;

void S_DrawHealthBar(int percent);

void DrawHealthBar(int flash_state)
{
    int hitpoints;

    hitpoints = lara_item->hit_points;
    if (hitpoints < 0)
        hitpoints = 0;
    else if (hitpoints > LARA_HITPOINTS)
        hitpoints = LARA_HITPOINTS;

    if (old_hitpoints != hitpoints)
    {
        HealthBarTimer = 40;
        old_hitpoints = hitpoints;
    }

    if (HealthBarTimer < 0)
        HealthBarTimer = 0;

    if (hitpoints <= LARA_HITPOINTS / 4)
    {
        if (flash_state)
            S_DrawHealthBar(hitpoints / 10);
        else
            S_DrawHealthBar(0);
    }
    else if (HealthBarTimer > 0 || hitpoints <= 0 || lara.gun_status == LG_READY)
        S_DrawHealthBar(hitpoints / 10);
}
