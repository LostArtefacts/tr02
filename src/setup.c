#include "game.h"
#include "savegame.h"

#define NUMBER_OBJECTS 265
#define DEFAULT_RADIUS 10
#define DONT_TARGET (-16384)
#define NO_ROOM 0xff

#include "specific/init.h"

#define FINAL_LEVEL 262
#define GFT_TITLEFILE 0

void InitialiseGameFlags(void);
void InitialiseLara(int type);
void InitialiseFXArray(void);
void InitialiseLOTarray(void);
void InitColours(void);
void T_InitPrint(void);
void InitialisePickUpDisplay(void);
void S_InitialiseScreen(int type);
void ExtractSaveGameInfo(void);
void GF_ModifyInventory(int level, int type);
void InitialiseFinalLevel(void);
void S_CDPlay(sint16 track, int mode);
int S_LoadLevelFile(char *fname, int level_number, int type);
void GetCarriedItems(void);

extern int IsDemoLevelType;
extern int HealthBarTimer;
extern char **GF_levelfilenames;
extern char **GF_cutscenefilenames;
extern char **GF_titlefilenames;
extern sint16 GF_CDtracks[];

int InitialiseLevel(int level_number, int type)
{
    if (type != IL_TITLE && type != IL_CUTSCENE)
        CurrentLevel = level_number;
    IsDemoLevelType = type == IL_DEMO;
    InitialiseGameFlags();
    lara.item_number = NO_ITEM;
    title_loaded = 0;

    switch (type)
    {
    case IL_NORMAL:
    case IL_SAVED:
    case IL_DEMO:
    default:
        if (!S_LoadLevelFile(GF_levelfilenames[level_number], level_number, type))
            return 0;
        break;
    case IL_CUTSCENE:
        if (!S_LoadLevelFile(GF_cutscenefilenames[level_number], level_number, type))
            return 0;
        break;
    case IL_TITLE:
        if (!S_LoadLevelFile(GF_titlefilenames[GFT_TITLEFILE], level_number, type))
            return 0;
        break;
    }

    if (lara.item_number != NO_ITEM)
        InitialiseLara(type);
    if (type == IL_NORMAL || type == IL_SAVED || type == IL_DEMO)
        GetCarriedItems();

    effects = (FX_INFO *)game_malloc(NUM_EFFECTS * sizeof(FX_INFO), EFFECTS_ARRAY);
    InitialiseFXArray();
    InitialiseLOTarray();
    InitColours();
    T_InitPrint();
    InitialisePickUpDisplay();
    S_InitialiseScreen(type);
    HealthBarTimer = 100;
    SOUND_Stop();

    if (type == IL_SAVED)
        ExtractSaveGameInfo();
    else if (type == IL_NORMAL)
        GF_ModifyInventory(CurrentLevel, 0);

    if (objects[FINAL_LEVEL].loaded)
        InitialiseFinalLevel();

    if (type == IL_NORMAL || type == IL_SAVED || type == IL_DEMO)
    {
        if (GF_CDtracks[0])
            S_CDPlay(GF_CDtracks[0], 1);
    }

    assault_timer_active = 0;
    assault_timer_display = 0;
    camera.underwater = 0;
    return 1;
}

void InitialiseLevelFlags(void)
{
    savegame.secrets = 0;
    savegame.timer = 0;
    savegame.kills = 0;
    savegame.distance_travelled = 0;
    savegame.ammo_used = savegame.ammo_hit = 0;
    savegame.health_used = 0;
}

void BaddyObjects(void);
void TrapObjects(void);
void ObjectObjects(void);

void InitialiseObjects(void)
{
    int i;
    OBJECT_INFO *obj;

    for (i = 0, obj = objects; i < NUMBER_OBJECTS; i++, obj++)
    {
        obj->initialise = NULL;
        obj->collision = NULL;
        obj->control = NULL;
        obj->draw_routine = DrawAnimatingItem;
        obj->floor = obj->ceiling = NULL;
        obj->pivot_length = 0;
        obj->radius = DEFAULT_RADIUS;
        obj->shadow_size = 0;
        obj->hit_points = DONT_TARGET;
        obj->intelligent = obj->water_creature = 0;
        obj->save_position = obj->save_hitpoints = obj->save_flags = obj->save_anim = 0;
    }

    BaddyObjects();
    TrapObjects();
    ObjectObjects();

    InitialiseHair();
}

void PickUpCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void RemoveDrawnItem(sint16 item_num);
extern int level_items;

void GetCarriedItems(void)
{
    int i;
    sint16 pickup_number;
    ITEM_INFO *item, *pickup;

    for (i = 0, item = items; i < level_items; i++, item++)
    {
        if (objects[item->object_number].intelligent)
        {
            item->carried_item = NO_ITEM;

            pickup_number = room[item->room_number].item_number;
            do
            {
                pickup = &items[pickup_number];
                if (pickup->pos.x_pos == item->pos.x_pos && pickup->pos.y_pos == item->pos.y_pos
                    && pickup->pos.z_pos == item->pos.z_pos && objects[pickup->object_number].collision == PickUpCollision)
                {
                    pickup->carried_item = item->carried_item;
                    item->carried_item = pickup_number;
                    RemoveDrawnItem(pickup_number);
                    pickup->room_number = NO_ROOM;
                }

                pickup_number = pickup->next_item;
            }
            while (pickup_number != NO_ITEM);
        }
    }
}
