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

#define MAX_FLIPMAPS 10
#define MAX_CD_TRACKS 64

extern int flipmap[];
extern sint16 cd_flags[];
extern int SunsetTimer;
extern TEXTSTRING *AmmoTextInfo;
extern sint32 detonate_all_mines;
extern int monks_attack_lara;

void InitialiseGameFlags(void)
{
    int i;

    flip_status = 0;
    for (i = 0; i < MAX_FLIPMAPS; i++)
        flipmap[i] = 0;
    for (i = 0; i < MAX_CD_TRACKS; i++)
        cd_flags[i] = 0;
    for (i = 0; i < NUMBER_OBJECTS; i++)
        objects[i].loaded = 0;
    SunsetTimer = 0;
    AmmoTextInfo = NULL;
    level_complete = 0;
    flipeffect = -1;
    detonate_all_mines = 0;
    monks_attack_lara = 0;
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

#define GONDOLA 24
#define FALLING_BLOCK 55
#define FALLING_BLOCK2 56
#define FALLING_PLANK 57
#define PENDULUM 58
#define SPIKES 59
#define ROLLING_BALL 60
#define DARTS 61
#define DART_EMITTER 62
#define TEETH_TRAP 64
#define MOVING_BAR 66
#define MOVABLE_BLOCK 67
#define MOVABLE_BLOCK2 68
#define MOVABLE_BLOCK3 69
#define MOVABLE_BLOCK4 70
#define PROPELLER1 76
#define PROPELLER2 77
#define HOOK 78
#define FALLING_CEILING1 79
#define SPINNING_BLADE 80
#define BLADE 81
#define KILLER_STATUE 82
#define OILDRUMS 83
#define ICICLES 84
#define SPIKE_WALL 85
#define SPRING_BOARD 86
#define CEILING_SPIKES 87
#define PROPELLER3 94
#define SMALL_FAN 95
#define SWING_BOX 96
#define BIG_ROLLING_BALL 101
#define DEATH_SLIDE 102
#define MINES 222
#define COPTER 228
#define DART_EFFECT 234
#define LAVA 250
#define LAVA_EMITTER 251
#define FLAME 252
#define FLAME_EMITTER 253
#define DYING_MONK 256
#define MINI_COPTER 259
#define UNIT_SHADOW 256

void BaddyObjects(void);
void TrapObjects(void);
void ObjectObjects(void);

void InitialiseDyingMonk(sint16 item_number);
void DyingMonk(sint16 item_number);
void GondolaControl(sint16 item_number);
void CopterControl(sint16 item_number);
void MiniCopterControl(sint16 item_number);
void GeneralControl(sint16 item_number);
void MineControl(sint16 item_number);
void InitialiseRollingBall(sint16 item_number);
void DeathSlideCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void ControlDeathSlide(sint16 item_number);
void ControlSpikeWall(sint16 item_number);
void ControlCeilingSpikes(sint16 item_number);
void HookControl(sint16 item_number);
void PropellerControl(sint16 item_number);
void InitialiseKillerStatue(sint16 item_number);
void SpinningBlade(sint16 item_number);
void IcicleControl(sint16 item_number);
void KillerStatueControl(sint16 item_number);
void InitialiseBlade(sint16 item_number);
void BladeControl(sint16 item_number);
void SpringBoardControl(sint16 item_number);
void FallingBlock(sint16 item_number);
void FallingBlockFloor(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height);
void FallingBlockCeiling(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height);
void Pendulum(sint16 item_number);
void TeethTrap(sint16 item_number);
void RollingBallCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void RollingBallControl(sint16 item_number);
void SpikeCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void FallingCeiling(sint16 item_number);
void InitialiseMovingBlock(sint16 item_number);
void MovableBlock(sint16 item_number);
void MovableBlockCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void DrawMovableBlock(ITEM_INFO *item);
void DartEmitterControl(sint16 item_number);
void DartsControl(sint16 item_number);
void DartEffectControl(sint16 fx_num);
void DrawSpriteItem(ITEM_INFO *item);
void FlameEmitterControl(sint16 item_number);
void FlameControl(sint16 fx_num);
void LavaSpray(sint16 item_number);
void ControlLavaBlob(sint16 fx_num);
void DrawDummyItem(ITEM_INFO *item);
void TrapCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void CreatureCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void ObjectCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);

#define INIT_MOVBLOCK(a) \
    obj = &objects[a]; \
    obj->initialise = InitialiseMovingBlock; \
    obj->control = MovableBlock; \
    obj->collision = MovableBlockCollision; \
    obj->draw_routine = DrawMovableBlock; \
    obj->save_position = obj->save_anim = obj->save_flags = 1

#define INIT_ROLLBALL(a) \
    obj = &objects[a]; \
    obj->collision = RollingBallCollision; \
    obj->control = RollingBallControl; \
    obj->initialise = InitialiseRollingBall; \
    obj->save_position = obj->save_flags = 1; \
    obj->save_anim = 1

#define INIT_FALLBLOCK(a) \
    obj = &objects[a]; \
    obj->control = FallingBlock; \
    obj->floor = FallingBlockFloor; \
    obj->ceiling = FallingBlockCeiling; \
    obj->save_position = obj->save_anim = 1; \
    obj->save_flags = 1

void TrapObjects(void)
{
    OBJECT_INFO *obj;

    obj = &objects[DYING_MONK];
    obj->initialise = InitialiseDyingMonk;
    obj->control = DyingMonk;
    obj->collision = ObjectCollision;
    obj->save_flags = 1;

    obj = &objects[GONDOLA];
    obj->control = GondolaControl;
    obj->collision = ObjectCollision;
    obj->save_anim = obj->save_flags = 1;

    obj = &objects[COPTER];
    obj->control = CopterControl;
    obj->save_anim = obj->save_flags = 1;

    obj = &objects[MINI_COPTER];
    obj->control = MiniCopterControl;
    obj->save_flags = obj->save_position = 1;

    obj = &objects[MOVING_BAR];
    obj->collision = ObjectCollision;
    obj->control = GeneralControl;
    obj->save_anim = obj->save_flags = 1;
    obj->water_creature = 1;

    obj = &objects[MINES];
    obj->collision = ObjectCollision;
    obj->control = MineControl;
    obj->save_flags = 1;

    obj = &objects[DEATH_SLIDE];
    obj->initialise = InitialiseRollingBall;
    obj->collision = DeathSlideCollision;
    obj->control = ControlDeathSlide;
    obj->save_position = obj->save_anim = obj->save_flags = 1;

    obj = &objects[SPIKE_WALL];
    obj->control = ControlSpikeWall;
    obj->collision = ObjectCollision;
    obj->save_position = obj->save_flags = 1;

    obj = &objects[CEILING_SPIKES];
    obj->control = ControlCeilingSpikes;
    obj->collision = TrapCollision;
    obj->save_position = obj->save_flags = 1;

    obj = &objects[HOOK];
    obj->control = HookControl;
    obj->collision = CreatureCollision;
    obj->save_flags = obj->save_anim = 1;

    obj = &objects[PROPELLER2];
    obj->control = PropellerControl;
    obj->collision = ObjectCollision;
    obj->save_flags = obj->save_anim = 1;

    obj = &objects[PROPELLER3];
    obj->water_creature = 1;
    obj->control = PropellerControl;
    obj->collision = TrapCollision;
    obj->save_flags = obj->save_anim = 1;

    obj = &objects[SMALL_FAN];
    obj->control = PropellerControl;
    obj->collision = TrapCollision;
    obj->save_flags = obj->save_anim = 1;

    obj = &objects[PROPELLER1];
    obj->control = PropellerControl;
    obj->collision = TrapCollision;
    obj->save_flags = obj->save_anim = 1;

    obj = &objects[SPINNING_BLADE];
    obj->initialise = InitialiseKillerStatue;
    obj->control = SpinningBlade;
    obj->collision = ObjectCollision;
    obj->save_position = obj->save_anim = obj->save_flags = 1;

    obj = &objects[ICICLES];
    obj->control = IcicleControl;
    obj->collision = TrapCollision;
    obj->save_position = obj->save_anim = obj->save_flags = 1;

    obj = &objects[KILLER_STATUE];
    obj->initialise = InitialiseKillerStatue;
    obj->control = KillerStatueControl;
    obj->collision = TrapCollision;
    obj->save_anim = obj->save_flags = 1;

    obj = &objects[BLADE];
    obj->initialise = InitialiseBlade;
    obj->control = BladeControl;
    obj->collision = TrapCollision;
    obj->save_anim = obj->save_flags = 1;

    obj = &objects[SPRING_BOARD];
    obj->control = SpringBoardControl;
    obj->save_anim = obj->save_flags = 1;

    INIT_FALLBLOCK(FALLING_BLOCK);
    INIT_FALLBLOCK(FALLING_BLOCK2);
    INIT_FALLBLOCK(FALLING_PLANK);

    obj = &objects[PENDULUM];
    obj->shadow_size = UNIT_SHADOW / 2;
    obj->control = Pendulum;
    obj->collision = ObjectCollision;
    obj->save_flags = 1;
    obj->save_anim = 1;

    obj = &objects[SWING_BOX];
    obj->shadow_size = UNIT_SHADOW / 2;
    obj->control = Pendulum;
    obj->collision = ObjectCollision;
    obj->save_flags = 1;
    obj->save_anim = 1;

    obj = &objects[TEETH_TRAP];
    obj->collision = TrapCollision;
    obj->control = TeethTrap;
    obj->save_flags = 1;
    obj->save_anim = 1;

    INIT_ROLLBALL(OILDRUMS);
    INIT_ROLLBALL(BIG_ROLLING_BALL);
    INIT_ROLLBALL(ROLLING_BALL);

    obj = &objects[SPIKES];
    obj->collision = SpikeCollision;

    obj = &objects[FALLING_CEILING1];
    obj->control = FallingCeiling;
    obj->collision = TrapCollision;
    obj->save_position = obj->save_anim = 1;
    obj->save_flags = 1;

    INIT_MOVBLOCK(MOVABLE_BLOCK);
    INIT_MOVBLOCK(MOVABLE_BLOCK2);
    INIT_MOVBLOCK(MOVABLE_BLOCK3);
    INIT_MOVBLOCK(MOVABLE_BLOCK4);

    obj = &objects[DART_EMITTER];
    obj->control = DartEmitterControl;
    obj->save_flags = 1;

    obj = &objects[DARTS];
    obj->control = DartsControl;
    obj->collision = ObjectCollision;
    obj->shadow_size = UNIT_SHADOW / 2;

    obj = &objects[DART_EFFECT];
    obj->control = DartEffectControl;
    obj->draw_routine = DrawSpriteItem;
    obj->semi_transparent = 1;

    obj = &objects[FLAME_EMITTER];
    obj->control = FlameEmitterControl;
    obj->draw_routine = DrawDummyItem;
    obj->save_flags = 1;

    objects[FLAME].control = FlameControl;
    objects[FLAME].semi_transparent = 1;

    objects[LAVA_EMITTER].control = LavaSpray;
    objects[LAVA_EMITTER].collision = ObjectCollision;
    objects[LAVA_EMITTER].draw_routine = DrawDummyItem;
    objects[LAVA_EMITTER].save_flags = 1;

    objects[LAVA].control = ControlLavaBlob;
    objects[LAVA].semi_transparent = 1;
}

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
