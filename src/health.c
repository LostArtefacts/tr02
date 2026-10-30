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

#define SECRET_ITEM1 190
#define SECRET_ITEM2 191
#define SECRET_ITEM3 192

void S_CDPlay(sint16 track, int mode);

void AddDisplayPickup(sint16 objnum)
{
    int i;

    if (objnum == SECRET_ITEM1 || objnum == SECRET_ITEM2 || objnum == SECRET_ITEM3)
        S_CDPlay(gameflow.secret_track, 0);

    for (i = 0; i < NUM_PU; i++)
    {
        if (pickups[i].duration <= 0)
        {
            pickups[i].duration = 75;
            pickups[i].sprnum = objects[objnum].mesh_index;
            return;
        }
    }
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

#define LARA_AIR 1800

void S_DrawAirBar(int percent);

void DrawAirBar(int flash_state)
{
    int air;

    if (lara.water_status != LARA_UNDERWATER && lara.water_status != LARA_SURFACE)
        return;

    air = lara.air;
    if (air < 0)
        air = 0;
    else if (air > LARA_AIR)
        air = LARA_AIR;

    if (air <= LARA_AIR / 4)
    {
        if (flash_state)
            S_DrawAirBar(air * 100 / LARA_AIR);
        else
            S_DrawAirBar(0);
    }
    else
        S_DrawAirBar(air * 100 / LARA_AIR);
}

extern TEXTSTRING *DisplayModeTextInfo;
extern int DisplayModeInfoTimer;

void DrawModeInfo(void)
{
    if (DisplayModeTextInfo && --DisplayModeInfoTimer == 0)
    {
        T_RemovePrint(DisplayModeTextInfo);
        DisplayModeTextInfo = NULL;
    }
}
