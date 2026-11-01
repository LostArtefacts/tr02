#include "game.h"
#include "larafire.h"
#include "savegame.h"

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

#define ASSAULT_NUMBERS 261

int sprintf(char *buffer, const char *format, ...);
void S_DrawScreenSprite2d(sint32 sx, sint32 sy, sint32 z, sint32 scaleH, sint32 scaleV, sint16 sprnum, sint16 shade, uint16 flags);

void DrawAssaultTimer(void)
{
    char buffer[8];
    char *c;
    int x;
    int seconds;

    if (CurrentLevel != LV_GYM || !assault_timer_display)
        return;
    seconds = savegame.timer / 30;
    sprintf(buffer, "%d:%02d.%d", seconds / 60, seconds % 60, (int)(savegame.timer % 30 / 3));
    x = (phd_winxmax >> 1) - 50;
    for (c = buffer; *c; c++) {
        if (*c == ':') {
            x -= 6;
            S_DrawScreenSprite2d(x, 36, 0, 0x10000, 0x10000, (sint16)(objects[ASSAULT_NUMBERS].mesh_index + 10), 0x1000, 0);
            x += 14;
        } else if (*c == '.') {
            x -= 6;
            S_DrawScreenSprite2d(x, 36, 0, 0x10000, 0x10000, (sint16)(objects[ASSAULT_NUMBERS].mesh_index + 11), 0x1000, 0);
            x += 14;
        } else {
            S_DrawScreenSprite2d(x, 36, 0, 0x10000, 0x10000, (sint16)(*c + objects[ASSAULT_NUMBERS].mesh_index - '0'), 0x1000, 0);
            x += 20;
        }
    }
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

__declspec(dllimport) int __cdecl wsprintfA(char *, const char *, ...);
void T_RightAlign(TEXTSTRING *textString, sint16 ok);
extern TEXTSTRING *AmmoTextInfo;

void DrawAmmoInfo(void)
{
    char ammostring[80];

    ammostring[0] = 0;
    if (lara.gun_status != LG_READY || OverlayStatus <= 0 || savegame.bonus_flag)
    {
        if (AmmoTextInfo)
        {
            T_RemovePrint(AmmoTextInfo);
            AmmoTextInfo = NULL;
        }
        return;
    }
    switch (lara.gun_type)
    {
    case LG_PISTOLS:
        return;
    case LG_MAGNUMS:
        wsprintfA(ammostring, "%5d", lara.magnums.ammo);
        break;
    case LG_UZIS:
        wsprintfA(ammostring, "%5d", lara.uzis.ammo);
        break;
    case LG_SHOTGUN:
        wsprintfA(ammostring, "%5d", lara.shotgun.ammo / 6);
        break;
    case LG_HARPOON:
        wsprintfA(ammostring, "%5d", lara.harpoon.ammo);
        break;
    case LG_M16:
        wsprintfA(ammostring, "%5d", lara.m16.ammo);
        break;
    case LG_GRENADE:
        wsprintfA(ammostring, "%5d", lara.grenade.ammo);
        break;
    default:
        return;
    }
    MakeAmmoString(ammostring);
    if (AmmoTextInfo)
        T_ChangeText(AmmoTextInfo, ammostring);
    else
    {
        AmmoTextInfo = T_Print(-10, 35, 0, ammostring);
        T_RightAlign(AmmoTextInfo, 1);
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

static int old_game_timer;

void S_DrawPickup(int sx, int sy, int scale, sint16 sprnum, sint16 shade, uint16 flags);

void DrawPickups(int timed)
{
    sint32 sprite_width, time, x, y;
    DISPLAYPU *pu;
    int i, count;

    time = savegame.timer - old_game_timer;
    old_game_timer = savegame.timer;

    if (time > 0 && time < 60) {
        sprite_width = phd_winwidth / 10;
        y = phd_winheight - sprite_width;
        x = phd_winwidth - sprite_width;
        pu = pickups;
        count = 0;
        for (i = 0; i < NUM_PU; i++, pu++, x -= sprite_width) {
            if (timed)
                pu->duration -= (sint16)time;

            if (pu->duration > 0) {
                if (count == 4 || count == 8) {
                    y += -sprite_width * 2 / 3;
                    x = phd_winwidth - sprite_width;
                }
                S_DrawPickup(x, y, 12288, pu->sprnum, 4096, 0);
                count++;
            } else {
                pu->duration = 0;
            }
        }
    }
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

extern TEXTSTRING *DisplayModeTextInfo;
extern int DisplayModeInfoTimer;

void T_RightAlign(TEXTSTRING *textString, sint16 ok);
void T_BottomAlign(TEXTSTRING *textString, sint16 ok);

void DisplayModeInfo(char *string)
{
    if (!string)
    {
        T_RemovePrint(DisplayModeTextInfo);
        DisplayModeTextInfo = NULL;
        return;
    }
    if (DisplayModeTextInfo)
        T_ChangeText(DisplayModeTextInfo, string);
    else
    {
        DisplayModeTextInfo = T_Print(-16, -16, 0, string);
        T_RightAlign(DisplayModeTextInfo, 1);
        T_BottomAlign(DisplayModeTextInfo, 1);
    }
    DisplayModeInfoTimer = 75;
}

void DrawModeInfo(void)
{
    if (DisplayModeTextInfo && --DisplayModeInfoTimer == 0)
    {
        T_RemovePrint(DisplayModeTextInfo);
        DisplayModeTextInfo = NULL;
    }
}

#ifdef TR2_EXE
TEXTSTRING *AmmoTextInfo;
TEXTSTRING *DisplayModeTextInfo;
int DisplayModeInfoTimer;
int HealthBarTimer;
#endif
