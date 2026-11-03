#include "game.h"
#include "savegame.h"
#include "specific/init.h"

#define NUMBER_OBJECTS 265
#define DEFAULT_RADIUS 10
#define DONT_TARGET (-16384)
#define NO_ROOM 0xff


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

#define LARA_EXTRA 12
#define DOG 15
#define CULT1 16
#define CULT1A 17
#define CULT1B 18
#define CULT2 19
#define CULT3 20
#define MOUSE 21
#define DRAGON_FRONT 22
#define DRAGON_BACK 23
#define SHARK 25
#define EEL 26
#define BIG_EEL 27
#define BARRACUDA 28
#define DIVER 29
#define WORKER1 30
#define WORKER2 31
#define WORKER3 32
#define WORKER4 33
#define WORKER5 34
#define JELLY 35
#define SPIDER 36
#define BIG_SPIDER 37
#define CROW 38
#define TIGER 39
#define BARTOLI 40
#define XIAN_LORD 41
#define CHINESE2 42
#define WARRIOR 43
#define CHINESE4 44
#define YETI 45
#define GIANT_YETI 46
#define EAGLE 47
#define BANDIT1 48
#define BANDIT2 49
#define BANDIT2B 50
#define SKIDOO_ARMED 51
#define SKIDMAN 52
#define MONK1 53
#define MONK2 54
#define DINO 214
#define WINSTON 260
#define ROT_Y 8
#define ROT_Z 16

void S_ExitSystem(char *message);
void InitialiseLaraLoad(sint16 item_number);
void ControlLaraExtra(sint16 item_number);
void DogControl(sint16 item_number);
void MouseControl(sint16 item_number);
void InitialiseCult1(sint16 item_number);
void Cult1Control(sint16 item_number);
void Cult2Control(sint16 item_number);
void SharkControl(sint16 item_number);
void DrawUnclippedItem(ITEM_INFO *item);
void TigerControl(sint16 item_number);
void BaracuddaControl(sint16 item_number);
void SpiderControl(sint16 item_number);
void BigSpiderControl(sint16 item_number);
void YetiControl(sint16 item_number);
void JellyControl(sint16 item_number);
void DiverControl(sint16 item_number);
void Worker1Control(sint16 item_number);
void Worker2Control(sint16 item_number);
void Worker3Control(sint16 item_number);
void InitialiseCult3(sint16 item_number);
void Cult3Control(sint16 item_number);
void MonkControl(sint16 item_number);
void InitialiseEagle(sint16 item_number);
void EagleControl(sint16 item_number);
void BigEelControl(sint16 item_number);
void EelControl(sint16 item_number);
void BanditControl(sint16 item_number);
void Bandit2Control(sint16 item_number);
void DrawSkidoo(ITEM_INFO *item);
void SkidmanCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void InitialiseSkidman(sint16 item_number);
void SkidManControl(sint16 item_number);
void InitialiseXianLord(sint16 item_number);
void DrawXianLord(ITEM_INFO *item);
void XianLordControl(sint16 item_number);
void WarriorControl(sint16 item_number);
void DragonControl(sint16 item_number);
void DragonCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void InitialiseBartoli(sint16 item_number);
void BartoliControl(sint16 item_number);
void GiantYetiControl(sint16 item_number);
void DinoControl(sint16 item_number);
void WinstonControl(sint16 item_number);
void DrawDummyItem(ITEM_INFO *item);
void CreatureCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);

#define SAVE_CREATURE(o) \
    o->save_position = o->save_hitpoints = o->save_anim = o->save_flags = 1

void BaddyObjects(void)
{
    OBJECT_INFO *obj;

    obj = &objects[LARA];
    obj->initialise = InitialiseLaraLoad;
    obj->shadow_size = (UNIT_SHADOW / 16) * 10;
    obj->hit_points = 1000;
    obj->draw_routine = DrawDummyItem;
    obj->save_position = obj->save_hitpoints = 1;
    obj->save_anim = obj->save_flags = 1;

    obj = &objects[LARA_EXTRA];
    obj->control = ControlLaraExtra;

    obj = &objects[DOG];
    if (obj->loaded)
    {
        obj->control = DogControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = UNIT_SHADOW / 2;
        obj->hit_points = 10;
        obj->pivot_length = 300;
        obj->radius = 341;
        obj->intelligent = 1;
        SAVE_CREATURE(obj);
        bones[obj->bone_index + 19 * 4] |= ROT_Y;
    }

    obj = &objects[MOUSE];
    if (obj->loaded)
    {
        obj->control = MouseControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = UNIT_SHADOW / 2;
        obj->hit_points = 4;
        obj->pivot_length = 50;
        obj->radius = 102;
        obj->intelligent = 1;
        SAVE_CREATURE(obj);
        bones[obj->bone_index + 3 * 4] |= ROT_Y;
    }

    obj = &objects[CULT1];
    if (obj->loaded)
    {
        obj->initialise = InitialiseCult1;
        obj->control = Cult1Control;
        obj->collision = CreatureCollision;
        obj->shadow_size = UNIT_SHADOW / 2;
        obj->hit_points = 25;
        obj->pivot_length = 50;
        obj->radius = 102;
        obj->intelligent = 1;
        SAVE_CREATURE(obj);
        bones[obj->bone_index + 0 * 4] |= ROT_Y;
    }

    obj = &objects[CULT1A];
    if (obj->loaded)
    {
        if (!objects[CULT1].loaded)
            S_ExitSystem("FATAL: CULT1B requires CULT1");
        obj->frame_base = objects[CULT1].frame_base;
        obj->anim_index = objects[CULT1].anim_index;
        obj->initialise = InitialiseCult1;
        obj->control = Cult1Control;
        obj->collision = CreatureCollision;
        obj->shadow_size = UNIT_SHADOW / 2;
        obj->hit_points = 25;
        obj->pivot_length = 50;
        obj->radius = 102;
        obj->intelligent = 1;
        SAVE_CREATURE(obj);
        bones[obj->bone_index + 0 * 4] |= ROT_Y;
    }

    obj = &objects[CULT1B];
    if (obj->loaded)
    {
        if (!objects[CULT1].loaded)
            S_ExitSystem("FATAL: CULT1B requires CULT1");
        obj->frame_base = objects[CULT1].frame_base;
        obj->anim_index = objects[CULT1].anim_index;
        obj->initialise = InitialiseCult1;
        obj->control = Cult1Control;
        obj->collision = CreatureCollision;
        obj->shadow_size = UNIT_SHADOW / 2;
        obj->hit_points = 25;
        obj->pivot_length = 50;
        obj->radius = 102;
        obj->intelligent = 1;
        SAVE_CREATURE(obj);
        bones[obj->bone_index + 0 * 4] |= ROT_Y;
    }

    obj = &objects[CULT2];
    if (obj->loaded)
    {
        obj->control = Cult2Control;
        obj->collision = CreatureCollision;
        obj->shadow_size = UNIT_SHADOW / 2;
        obj->hit_points = 60;
        obj->pivot_length = 50;
        obj->radius = 102;
        obj->intelligent = 1;
        SAVE_CREATURE(obj);
        bones[obj->bone_index + 0 * 4] |= ROT_Y;
        bones[obj->bone_index + 8 * 4] |= ROT_Y;
    }

    obj = &objects[SHARK];
    if (obj->loaded)
    {
        obj->control = SharkControl;
        obj->draw_routine = DrawUnclippedItem;
        obj->collision = CreatureCollision;
        obj->shadow_size = UNIT_SHADOW / 2;
        obj->hit_points = 30;
        obj->pivot_length = 200;
        obj->radius = 341;
        obj->intelligent = 1;
        obj->water_creature = 1;
        SAVE_CREATURE(obj);
        bones[obj->bone_index + 9 * 4] |= ROT_Y;
    }

    obj = &objects[TIGER];
    if (obj->loaded)
    {
        obj->control = TigerControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = UNIT_SHADOW / 2;
        obj->hit_points = 20;
        obj->pivot_length = 200;
        obj->radius = 341;
        obj->intelligent = 1;
        SAVE_CREATURE(obj);
        bones[obj->bone_index + 21 * 4] |= ROT_Y;
    }

    obj = &objects[BARRACUDA];
    if (obj->loaded)
    {
        obj->control = BaracuddaControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = UNIT_SHADOW / 2;
        obj->hit_points = 12;
        obj->pivot_length = 200;
        obj->radius = 204;
        obj->intelligent = 1;
        obj->water_creature = 1;
        SAVE_CREATURE(obj);
        bones[obj->bone_index + 6 * 4] |= ROT_Y;
    }

    obj = &objects[SPIDER];
    if (obj->loaded)
    {
        obj->control = SpiderControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = UNIT_SHADOW / 2;
        obj->hit_points = 5;
        obj->radius = 102;
        obj->intelligent = 1;
        SAVE_CREATURE(obj);
    }

    obj = &objects[BIG_SPIDER];
    if (obj->loaded)
    {
        obj->control = BigSpiderControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = UNIT_SHADOW / 2;
        obj->hit_points = 40;
        obj->radius = 341;
        obj->intelligent = 1;
        SAVE_CREATURE(obj);
    }

    obj = &objects[YETI];
    if (obj->loaded)
    {
        obj->control = YetiControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = UNIT_SHADOW / 2;
        obj->hit_points = 30;
        obj->radius = 128;
        obj->intelligent = 1;
        SAVE_CREATURE(obj);
        obj->pivot_length = 100;
        bones[obj->bone_index + 6 * 4] |= ROT_Y;
        bones[obj->bone_index + 14 * 4] |= ROT_Y;
    }

    obj = &objects[JELLY];
    if (obj->loaded)
    {
        obj->control = JellyControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = UNIT_SHADOW / 2;
        obj->hit_points = 10;
        obj->radius = 102;
        obj->intelligent = 1;
        obj->water_creature = 1;
        SAVE_CREATURE(obj);
    }

    obj = &objects[DIVER];
    if (obj->loaded)
    {
        obj->control = DiverControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = UNIT_SHADOW / 2;
        obj->hit_points = 20;
        obj->radius = 341;
        obj->pivot_length = 50;
        obj->intelligent = 1;
        obj->water_creature = 1;
        SAVE_CREATURE(obj);
        bones[obj->bone_index + 10 * 4] |= ROT_Y;
        bones[obj->bone_index + 14 * 4] |= ROT_Z;
    }

    obj = &objects[WORKER1];
    if (obj->loaded)
    {
        obj->control = Worker1Control;
        obj->collision = CreatureCollision;
        obj->shadow_size = UNIT_SHADOW / 2;
        obj->hit_points = 25;
        obj->radius = 102;
        obj->intelligent = 1;
        SAVE_CREATURE(obj);
        obj->pivot_length = 0;
        bones[obj->bone_index + 4 * 4] |= ROT_Y;
        bones[obj->bone_index + 13 * 4] |= ROT_Y;
    }

    obj = &objects[WORKER2];
    if (obj->loaded)
    {
        obj->control = Worker2Control;
        obj->collision = CreatureCollision;
        obj->shadow_size = UNIT_SHADOW / 2;
        obj->hit_points = 20;
        obj->radius = 102;
        obj->intelligent = 1;
        SAVE_CREATURE(obj);
        obj->pivot_length = 0;
        bones[obj->bone_index + 4 * 4] |= ROT_Y;
        bones[obj->bone_index + 13 * 4] |= ROT_Y;
    }

    obj = &objects[WORKER3];
    if (obj->loaded)
    {
        obj->control = Worker3Control;
        obj->collision = CreatureCollision;
        obj->shadow_size = UNIT_SHADOW / 2;
        obj->hit_points = 27;
        obj->radius = 102;
        obj->intelligent = 1;
        SAVE_CREATURE(obj);
        obj->pivot_length = 0;
        bones[obj->bone_index + 0 * 4] |= ROT_Y;
        bones[obj->bone_index + 4 * 4] |= ROT_Y;
    }

    obj = &objects[WORKER4];
    if (obj->loaded)
    {
        obj->hit_points = 27;
        obj->control = Worker3Control;
        obj->collision = CreatureCollision;
        obj->shadow_size = UNIT_SHADOW / 2;
        obj->intelligent = 1;
        SAVE_CREATURE(obj);
        obj->radius = 102;
        obj->pivot_length = 0;
        bones[obj->bone_index + 0 * 4] |= ROT_Y;
        bones[obj->bone_index + 4 * 4] |= ROT_Y;
    }

    obj = &objects[WORKER5];
    if (obj->loaded)
    {
        obj->control = Worker2Control;
        obj->collision = CreatureCollision;
        obj->shadow_size = UNIT_SHADOW / 2;
        obj->hit_points = 20;
        obj->radius = 102;
        obj->intelligent = 1;
        SAVE_CREATURE(obj);
        obj->pivot_length = 0;
        bones[obj->bone_index + 4 * 4] |= ROT_Y;
        bones[obj->bone_index + 13 * 4] |= ROT_Y;
    }

    obj = &objects[CULT3];
    if (obj->loaded)
    {
        obj->initialise = InitialiseCult3;
        obj->control = Cult3Control;
        obj->collision = CreatureCollision;
        obj->shadow_size = UNIT_SHADOW / 2;
        obj->hit_points = 150;
        obj->radius = 102;
        obj->intelligent = 1;
        SAVE_CREATURE(obj);
        obj->pivot_length = 0;
    }

    obj = &objects[MONK1];
    if (obj->loaded)
    {
        obj->control = MonkControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = UNIT_SHADOW / 2;
        obj->hit_points = 30;
        obj->radius = 102;
        obj->intelligent = 1;
        SAVE_CREATURE(obj);
        obj->pivot_length = 0;
        bones[obj->bone_index + 6 * 4] |= ROT_Y;
    }

    obj = &objects[MONK2];
    if (obj->loaded)
    {
        obj->control = MonkControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = UNIT_SHADOW / 2;
        obj->hit_points = 30;
        obj->radius = 102;
        obj->intelligent = 1;
        SAVE_CREATURE(obj);
        bones[obj->bone_index + 6 * 4] |= ROT_Y;
    }

    obj = &objects[EAGLE];
    if (obj->loaded)
    {
        obj->initialise = InitialiseEagle;
        obj->control = EagleControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = UNIT_SHADOW / 2;
        obj->hit_points = 20;
        obj->radius = 204;
        obj->intelligent = 1;
        SAVE_CREATURE(obj);
        obj->pivot_length = 0;
    }

    obj = &objects[CROW];
    if (obj->loaded)
    {
        obj->initialise = InitialiseEagle;
        obj->control = EagleControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = UNIT_SHADOW / 2;
        obj->hit_points = 15;
        obj->radius = 204;
        obj->intelligent = 1;
        SAVE_CREATURE(obj);
        obj->pivot_length = 0;
    }

    obj = &objects[BIG_EEL];
    if (obj->loaded)
    {
        obj->control = BigEelControl;
        obj->collision = CreatureCollision;
        obj->hit_points = 20;
        obj->water_creature = 1;
        obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
    }

    obj = &objects[EEL];
    if (obj->loaded)
    {
        obj->control = EelControl;
        obj->collision = CreatureCollision;
        obj->hit_points = 5;
        obj->water_creature = 1;
        obj->save_hitpoints = obj->save_anim = obj->save_flags = 1;
    }

    obj = &objects[BANDIT1];
    if (obj->loaded)
    {
        obj->control = BanditControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = UNIT_SHADOW / 2;
        obj->hit_points = 45;
        obj->radius = 102;
        obj->intelligent = 1;
        SAVE_CREATURE(obj);
        obj->pivot_length = 0;
        bones[obj->bone_index + 6 * 4] |= ROT_Y;
        bones[obj->bone_index + 8 * 4] |= ROT_Y;
    }

    obj = &objects[BANDIT2];
    if (obj->loaded)
    {
        obj->control = Bandit2Control;
        obj->collision = CreatureCollision;
        obj->shadow_size = UNIT_SHADOW / 2;
        obj->hit_points = 50;
        obj->radius = 102;
        obj->intelligent = 1;
        SAVE_CREATURE(obj);
        obj->pivot_length = 0;
        bones[obj->bone_index + 6 * 4] |= ROT_Y;
        bones[obj->bone_index + 8 * 4] |= ROT_Y;
    }

    obj = &objects[BANDIT2B];
    if (obj->loaded)
    {
        if (!objects[BANDIT2].loaded)
            S_ExitSystem("FATAL: BANDIT2B requires BANDIT2");
        obj->frame_base = objects[BANDIT2].frame_base;
        obj->anim_index = objects[BANDIT2].anim_index;
        obj->control = Bandit2Control;
        obj->collision = CreatureCollision;
        obj->shadow_size = UNIT_SHADOW / 2;
        obj->hit_points = 50;
        obj->radius = 102;
        obj->intelligent = 1;
        SAVE_CREATURE(obj);
        obj->pivot_length = 0;
        bones[obj->bone_index + 6 * 4] |= ROT_Y;
        bones[obj->bone_index + 8 * 4] |= ROT_Y;
    }

    obj = &objects[SKIDOO_ARMED];
    if (obj->loaded)
    {
        obj->draw_routine = DrawSkidoo;
        obj->collision = SkidmanCollision;
        obj->shadow_size = UNIT_SHADOW / 2;
        obj->hit_points = 100;
        obj->radius = 341;
        obj->pivot_length = 0;
        obj->intelligent = 1;
        SAVE_CREATURE(obj);
    }

    obj = &objects[SKIDMAN];
    if (obj->loaded)
    {
        obj->initialise = InitialiseSkidman;
        obj->control = SkidManControl;
        obj->hit_points = 1;
        obj->save_position = obj->save_anim = obj->save_flags = 1;
    }

    obj = &objects[XIAN_LORD];
    if (obj->loaded)
    {
        if (!objects[CHINESE2].loaded)
            S_ExitSystem("FATAL: XianLord requires CHINESE2 (statue)");
        obj->initialise = InitialiseXianLord;
        obj->draw_routine = DrawXianLord;
        obj->control = XianLordControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = UNIT_SHADOW / 2;
        obj->intelligent = 1;
        SAVE_CREATURE(obj);
        obj->hit_points = 100;
        obj->radius = 204;
        obj->pivot_length = 0;
        bones[obj->bone_index + 6 * 4] |= ROT_Y;
        bones[obj->bone_index + 12 * 4] |= ROT_Y;
    }

    obj = &objects[WARRIOR];
    if (obj->loaded)
    {
        if (!objects[CHINESE4].loaded)
            S_ExitSystem("FATAL: Warrior requires CHINESE4 (statue)");
        obj->draw_routine = DrawXianLord;
        obj->initialise = InitialiseXianLord;
        obj->control = WarriorControl;
        obj->collision = CreatureCollision;
        obj->shadow_size = UNIT_SHADOW / 2;
        obj->intelligent = 1;
        SAVE_CREATURE(obj);
        obj->hit_points = 80;
        obj->radius = 204;
        obj->pivot_length = 0;
        bones[obj->bone_index + 6 * 4] |= ROT_Y;
        bones[obj->bone_index + 16 * 4] |= ROT_Y;
    }

    obj = &objects[DRAGON_FRONT];
    if (obj->loaded)
    {
        if (!objects[DRAGON_BACK].loaded)
            S_ExitSystem("FATAL: Dragon front needs back");
        obj->control = DragonControl;
        obj->collision = DragonCollision;
        obj->hit_points = 300;
        obj->radius = 341;
        obj->intelligent = 1;
        SAVE_CREATURE(obj);
        obj->pivot_length = 300;
        bones[obj->bone_index + 10 * 4] |= ROT_Z;
    }

    obj = &objects[DRAGON_BACK];
    if (obj->loaded)
    {
        obj->control = DragonControl;
        obj->collision = DragonCollision;
        obj->radius = 341;
        obj->save_position = obj->save_anim = obj->save_flags = 1;
    }

    obj = &objects[BARTOLI];
    if (obj->loaded)
    {
        obj->initialise = InitialiseBartoli;
        obj->control = BartoliControl;
        obj->save_anim = obj->save_flags = 1;
    }

    obj = &objects[GIANT_YETI];
    if (obj->loaded)
    {
        obj->control = GiantYetiControl;
        obj->collision = CreatureCollision;
        obj->hit_points = 200;
        obj->radius = 341;
        obj->intelligent = 1;
        SAVE_CREATURE(obj);
        bones[obj->bone_index + 14 * 4] |= ROT_Y;
    }

    obj = &objects[DINO];
    if (obj->loaded)
    {
        obj->control = DinoControl;
        obj->collision = CreatureCollision;
        obj->hit_points = 100;
        obj->shadow_size = UNIT_SHADOW / 4;
        obj->pivot_length = 1800;
        obj->intelligent = 1;
        SAVE_CREATURE(obj);
        obj->radius = 341;
        bones[obj->bone_index + 10 * 4] |= ROT_Y;
        bones[obj->bone_index + 11 * 4] |= ROT_Y;
    }

    obj = &objects[WINSTON];
    if (obj->loaded)
    {
        obj->control = WinstonControl;
        obj->collision = ObjectCollision;
        obj->hit_points = DONT_TARGET;
        obj->shadow_size = UNIT_SHADOW / 4;
        obj->radius = 102;
        obj->intelligent = 1;
        obj->save_position = obj->save_anim = obj->save_flags = 1;
    }
}

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

#define SKIDOO_FAST 13
#define BOAT 14
#define DRAW_BRIDGE 63
#define LIFT 65
#define BIG_BOWL 71
#define WINDOW1 72
#define WINDOW2 73
#define BELL 88
#define WATER_SPRITE 89
#define SNOW_SPRITE 90
#define SWITCH_TYPE1 92
#define SWITCH_TYPE2 93
#define SWITCH_TYPE3 103
#define SWITCH_TYPE4 104
#define SWITCH_TYPE5 105
#define DOOR_TYPE1 106
#define DOOR_TYPE2 107
#define DOOR_TYPE3 108
#define DOOR_TYPE4 109
#define DOOR_TYPE5 110
#define DOOR_TYPE6 111
#define DOOR_TYPE7 112
#define DOOR_TYPE8 113
#define TRAPDOOR 114
#define TRAPDOOR2 115
#define BRIDGE_FLAT 117
#define BRIDGE_TILT1 118
#define BRIDGE_TILT2 119
#define PLAYER_1 123
#define PLAYER_2 124
#define PLAYER_3 125
#define PLAYER_4 126
#define PLAYER_5 127
#define PLAYER_6 128
#define PLAYER_7 129
#define PLAYER_8 130
#define PLAYER_9 131
#define PLAYER_10 132
#define PISTOL_ITEM 135
#define SHOTGUN_ITEM 136
#define MAGNUM_ITEM 137
#define UZI_ITEM 138
#define HARPOON_ITEM 139
#define M16_ITEM 140
#define ROCKET_GUN_ITEM 141
#define PISTOL_AMMO_ITEM 142
#define SHOTGUN_AMMO_ITEM 143
#define MAGNUM_AMMO_ITEM 144
#define UZI_AMMO_ITEM 145
#define HARPOON_AMMO_ITEM 146
#define M16_AMMO_ITEM 147
#define ROCKET_AMMO_ITEM 148
#define MEDI_ITEM 149
#define BIGMEDI_ITEM 150
#define FLARES_ITEM 151
#define FLARE_ITEM 152
#define PUZZLE_ITEM1 174
#define PUZZLE_ITEM2 175
#define PUZZLE_ITEM3 176
#define PUZZLE_ITEM4 177
#define PUZZLE_HOLE1 182
#define PUZZLE_HOLE2 183
#define PUZZLE_HOLE3 184
#define PUZZLE_HOLE4 185
#define PUZZLE_DONE1 186
#define PUZZLE_DONE2 187
#define PUZZLE_DONE3 188
#define PUZZLE_DONE4 189
#define SECRET1 190
#define SECRET2 191
#define SECRET3 192
#define KEY_ITEM1 193
#define KEY_ITEM2 194
#define KEY_ITEM3 195
#define KEY_ITEM4 196
#define KEY_HOLE1 201
#define KEY_HOLE2 202
#define KEY_HOLE3 203
#define KEY_HOLE4 204
#define PICKUP_ITEM1 205
#define PICKUP_ITEM2 206
#define SPHERE_OF_DOOM 209
#define SPHERE_OF_DOOM2 210
#define SPHERE_OF_DOOM3 211
#define ALARM_SOUND 212
#define WATER_DRIP 213
#define BIRD_TWEETER 215
#define CLOCK_CHIMES 216
#define HOT_LIQUID 220
#define GONG_BONGER 225
#define DETONATOR1 226
#define DETONATOR2 227
#define EXPLOSION1 229
#define SPLASH1 230
#define BUBBLES1 231
#define BLOOD1 233
#define GLOW 236
#define RICOCHET1 238
#define TWINKLE 239
#define GUN_FLASH 240
#define BODY_PART 242
#define CAMERA_TARGET 243
#define WATERFALL 244
#define DIVER_HARPOON 245
#define DRAGON_FIRE 246
#define KNIFE 247
#define ROCKET 248
#define HARPOON_BOLT 249
#define DING_DONG 257
#define LARA_ALARM 258
#define FINAL_LEVEL 262
#define CUT_SHOTGUN 263
#define EARTHQUAKE 264

void SphereOfDoom(sint16 item_number);
void DrawSphereOfDoom(ITEM_INFO *item);
void SphereOfDoomCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void ControlRocket(sint16 item_number);
void ControlHarpoonBolt(sint16 item_number);
void ControlMissile(sint16 fx_num);
void InitialiseSkidoo(sint16 item_number);
void SkidooCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void DrawSkidoo(ITEM_INFO *item);
void InitialiseBoat(sint16 item_number);
void BoatControl(sint16 item_number);
void BoatCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void BigBowlControl(sint16 item_number);
void BellControl(sint16 item_number);
void FlareControl(sint16 item_number);
void DrawFlareInAir(ITEM_INFO *item);
void InitialiseWindow(sint16 item_number);
void WindowControl(sint16 item_number);
void SmashIceControl(sint16 item_number);
void InitialiseLift(sint16 item_number);
void LiftControl(sint16 item_number);
void LiftFloor(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height);
void BridgeFlatFloor(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height);
void BridgeFlatCeiling(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height);
void BridgeTilt1Floor(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height);
void BridgeTilt1Ceiling(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height);
void BridgeTilt2Floor(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height);
void BridgeTilt2Ceiling(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height);
void DrawBridgeCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void DrawBridgeFloor(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height);
void DrawBridgeCeiling(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height);
void SwitchControl(sint16 item_number);
void SwitchCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void SwitchCollision2(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void InitialiseDoor(sint16 item_number);
void DoorControl(sint16 item_number);
void DoorCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void TrapDoorControl(sint16 item_number);
void TrapDoorFloor(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height);
void TrapDoorCeiling(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height);
void ControlGongBonger(sint16 item_number);
void SecretControl(sint16 item_number);
void KeyHoleCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void DetonatorCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void DetonatorControl(sint16 item_number);
void PuzzleHoleCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void InitialiseGenPlayer(sint16 item_number);
void ControlCinematicPlayer(sint16 item_number);
void ControlBlood1(sint16 fx_num);
void ControlBubble1(sint16 fx_num);
void ControlExplosion1(sint16 fx_num);
void ControlRichochet1(sint16 fx_num);
void ControlTwinkle(sint16 fx_num);
void ControlSplash1(sint16 fx_num);
void ControlSnowSprite(sint16 fx_num);
void ControlWaterSprite(sint16 fx_num);
void WaterFall(sint16 item_number);
void ControlBodyPart(sint16 fx_num);
void ControlGunShot(sint16 fx_num);
void ControlGlow(sint16 fx_num);
void ControlHotLiquid(sint16 fx_num);
void ControlBirdTweeter(sint16 item_number);
void ControlAlarmSound(sint16 item_number);
void ControlDingDong(sint16 item_number);
void ControlLaraAlarm(sint16 item_number);
void ControlClockChimes(sint16 item_number);
void FinalLevelCounter(sint16 item_number);
void ControlCutShotgun(sint16 item_number);
void EarthQuake(sint16 item_number);
void PickUpCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);

#define INIT_PICKUP(a) \
    obj = &objects[a]; \
    obj->draw_routine = DrawSpriteItem; \
    obj->collision = PickUpCollision; \
    obj->save_flags = obj->save_position = 1

#define INIT_KEYHOLE(a) \
    obj = &objects[a]; \
    obj->collision = KeyHoleCollision; \
    obj->save_flags = 1

#define INIT_PUZZLEHOLE(a) \
    obj = &objects[a]; \
    obj->collision = PuzzleHoleCollision; \
    obj->save_flags = 1

#define INIT_DOOR(a) \
    obj = &objects[a]; \
    obj->initialise = InitialiseDoor; \
    obj->draw_routine = DrawUnclippedItem; \
    obj->control = DoorControl; \
    obj->collision = DoorCollision; \
    obj->save_flags = obj->save_anim = 1

#define INIT_SWITCH(a, b) \
    obj = &objects[a]; \
    obj->control = SwitchControl; \
    obj->collision = b; \
    obj->save_anim = obj->save_flags = 1

#define INIT_PLAYER(a) \
    obj = &objects[a]; \
    obj->initialise = InitialiseGenPlayer; \
    obj->control = ControlCinematicPlayer; \
    obj->hit_points = 1

void ObjectObjects(void)
{
    OBJECT_INFO *obj;

    objects[CAMERA_TARGET].draw_routine = DrawDummyItem;

    obj = &objects[SPHERE_OF_DOOM];
    obj->control = SphereOfDoom;
    obj->draw_routine = DrawSphereOfDoom;
    obj->collision = SphereOfDoomCollision;
    obj->save_flags = obj->save_position = 1;
    obj->semi_transparent = 1;

    obj = &objects[SPHERE_OF_DOOM2];
    obj->control = SphereOfDoom;
    obj->draw_routine = DrawSphereOfDoom;
    obj->collision = SphereOfDoomCollision;
    obj->save_flags = obj->save_position = 1;
    obj->semi_transparent = 1;

    obj = &objects[SPHERE_OF_DOOM3];
    obj->control = SphereOfDoom;
    obj->draw_routine = DrawSphereOfDoom;
    obj->collision = SphereOfDoomCollision;
    obj->save_flags = obj->save_position = 1;

    obj = &objects[BIG_BOWL];
    obj->control = BigBowlControl;
    obj->save_flags = obj->save_anim = 1;

    obj = &objects[BELL];
    obj->control = BellControl;
    obj->collision = ObjectCollision;
    obj->save_anim = obj->save_flags = 1;

    obj = &objects[ROCKET];
    obj->control = ControlRocket;
    obj->save_position = 1;

    obj = &objects[HARPOON_BOLT];
    obj->control = ControlHarpoonBolt;
    obj->save_position = 1;

    obj = &objects[KNIFE];
    obj->control = ControlMissile;
    obj->save_position = 1;

    obj = &objects[DIVER_HARPOON];
    obj->control = ControlMissile;
    obj->save_position = 1;

    obj = &objects[SKIDOO_FAST];
    obj->initialise = InitialiseSkidoo;
    obj->collision = SkidooCollision;
    obj->draw_routine = DrawSkidoo;
    obj->save_position = obj->save_flags = obj->save_anim = 1;

    obj = &objects[BOAT];
    obj->initialise = InitialiseBoat;
    obj->control = BoatControl;
    obj->collision = BoatCollision;
    obj->save_position = obj->save_flags = obj->save_anim = 1;

    obj = &objects[FLARE_ITEM];
    obj->control = FlareControl;
    obj->collision = PickUpCollision;
    obj->draw_routine = DrawFlareInAir;
    obj->save_position = obj->save_flags = 1;

    obj = &objects[WINDOW1];
    obj->initialise = InitialiseWindow;
    obj->control = WindowControl;
    obj->collision = ObjectCollision;
    obj->save_anim = obj->save_flags = 1;

    obj = &objects[WINDOW2];
    obj->initialise = InitialiseWindow;
    obj->control = SmashIceControl;
    obj->collision = ObjectCollision;
    obj->save_anim = obj->save_flags = 1;

    obj = &objects[LIFT];
    obj->initialise = InitialiseLift;
    obj->control = LiftControl;
    obj->floor = LiftFloor;
    obj->ceiling = LiftCeiling;
    obj->save_anim = obj->save_flags = obj->save_position = 1;

    obj = &objects[BRIDGE_FLAT];
    obj->floor = BridgeFlatFloor;
    obj->ceiling = BridgeFlatCeiling;

    obj = &objects[BRIDGE_TILT1];
    obj->floor = BridgeTilt1Floor;
    obj->ceiling = BridgeTilt1Ceiling;

    obj = &objects[BRIDGE_TILT2];
    obj->floor = BridgeTilt2Floor;
    obj->ceiling = BridgeTilt2Ceiling;

    obj = &objects[DRAW_BRIDGE];
    if (obj->loaded)
    {
        obj->control = GeneralControl;
        obj->collision = DrawBridgeCollision;
        obj->floor = DrawBridgeFloor;
        obj->ceiling = DrawBridgeCeiling;
        obj->save_anim = obj->save_flags = 1;
    }

    INIT_SWITCH(SWITCH_TYPE2, SwitchCollision);
    INIT_SWITCH(SWITCH_TYPE3, SwitchCollision);
    INIT_SWITCH(SWITCH_TYPE1, SwitchCollision);
    INIT_SWITCH(SWITCH_TYPE4, SwitchCollision);
    INIT_SWITCH(SWITCH_TYPE5, SwitchCollision2);

    INIT_DOOR(DOOR_TYPE1);
    INIT_DOOR(DOOR_TYPE2);
    INIT_DOOR(DOOR_TYPE3);
    INIT_DOOR(DOOR_TYPE4);
    INIT_DOOR(DOOR_TYPE5);
    INIT_DOOR(DOOR_TYPE6);
    INIT_DOOR(DOOR_TYPE7);
    INIT_DOOR(DOOR_TYPE8);

    obj = &objects[TRAPDOOR];
    obj->control = TrapDoorControl;
    obj->floor = TrapDoorFloor;
    obj->ceiling = TrapDoorCeiling;
    obj->save_anim = obj->save_flags = 1;

    obj = &objects[TRAPDOOR2];
    obj->control = TrapDoorControl;
    obj->floor = TrapDoorFloor;
    obj->ceiling = TrapDoorCeiling;
    obj->save_anim = obj->save_flags = 1;

    obj = &objects[GONG_BONGER];
    obj->control = ControlGongBonger;
    obj->save_anim = obj->save_flags = 1;

    INIT_PICKUP(PICKUP_ITEM1);
    INIT_PICKUP(PICKUP_ITEM2);
    INIT_PICKUP(KEY_ITEM1);
    INIT_PICKUP(KEY_ITEM2);
    INIT_PICKUP(KEY_ITEM3);
    INIT_PICKUP(KEY_ITEM4);
    INIT_PICKUP(PUZZLE_ITEM1);
    INIT_PICKUP(PUZZLE_ITEM2);
    INIT_PICKUP(PUZZLE_ITEM3);
    INIT_PICKUP(PUZZLE_ITEM4);
    INIT_PICKUP(SECRET1);
    INIT_PICKUP(SECRET2);
    objects[SECRET2].control = SecretControl;
    INIT_PICKUP(SECRET3);
    INIT_PICKUP(PISTOL_ITEM);
    INIT_PICKUP(SHOTGUN_ITEM);
    INIT_PICKUP(HARPOON_ITEM);
    INIT_PICKUP(ROCKET_GUN_ITEM);
    INIT_PICKUP(M16_ITEM);
    INIT_PICKUP(MAGNUM_ITEM);
    INIT_PICKUP(UZI_ITEM);
    INIT_PICKUP(FLARES_ITEM);
    INIT_PICKUP(PISTOL_AMMO_ITEM);
    INIT_PICKUP(SHOTGUN_AMMO_ITEM);
    INIT_PICKUP(MAGNUM_AMMO_ITEM);
    INIT_PICKUP(UZI_AMMO_ITEM);
    INIT_PICKUP(HARPOON_AMMO_ITEM);
    INIT_PICKUP(M16_AMMO_ITEM);
    INIT_PICKUP(ROCKET_AMMO_ITEM);
    INIT_PICKUP(MEDI_ITEM);
    INIT_PICKUP(BIGMEDI_ITEM);

    INIT_KEYHOLE(KEY_HOLE1);
    INIT_KEYHOLE(KEY_HOLE2);
    INIT_KEYHOLE(KEY_HOLE3);
    INIT_KEYHOLE(KEY_HOLE4);

    obj = &objects[DETONATOR2];
    obj->collision = DetonatorCollision;
    obj->control = DetonatorControl;
    obj->save_flags = obj->save_anim = 1;

    objects[DETONATOR1].collision = DetonatorCollision;

    INIT_PUZZLEHOLE(PUZZLE_HOLE1);
    INIT_PUZZLEHOLE(PUZZLE_HOLE2);
    INIT_PUZZLEHOLE(PUZZLE_HOLE3);
    INIT_PUZZLEHOLE(PUZZLE_HOLE4);

    objects[PUZZLE_DONE1].save_flags = 1;
    objects[PUZZLE_DONE2].save_flags = 1;
    objects[PUZZLE_DONE3].save_flags = 1;
    objects[PUZZLE_DONE4].save_flags = 1;

    INIT_PLAYER(PLAYER_1);
    INIT_PLAYER(PLAYER_2);
    INIT_PLAYER(PLAYER_3);
    INIT_PLAYER(PLAYER_4);
    INIT_PLAYER(PLAYER_5);
    INIT_PLAYER(PLAYER_6);
    INIT_PLAYER(PLAYER_7);
    INIT_PLAYER(PLAYER_8);
    INIT_PLAYER(PLAYER_9);
    INIT_PLAYER(PLAYER_10);

    objects[BLOOD1].control = ControlBlood1;
    objects[BLOOD1].semi_transparent = 1;

    objects[BUBBLES1].control = ControlBubble1;

    objects[EXPLOSION1].control = ControlExplosion1;
    objects[EXPLOSION1].semi_transparent = 1;

    objects[DRAGON_FIRE].control = ControlMissile;
    objects[DRAGON_FIRE].semi_transparent = 1;

    objects[RICOCHET1].control = ControlRichochet1;

    objects[TWINKLE].control = ControlTwinkle;

    objects[SPLASH1].control = ControlSplash1;
    objects[SPLASH1].semi_transparent = 1;

    objects[SNOW_SPRITE].control = ControlSnowSprite;

    objects[WATER_SPRITE].control = ControlWaterSprite;
    objects[WATER_SPRITE].semi_transparent = 1;

    objects[WATERFALL].control = WaterFall;
    objects[WATERFALL].draw_routine = DrawDummyItem;

    objects[BODY_PART].control = ControlBodyPart;
    objects[BODY_PART].nmeshes = 0;
    objects[BODY_PART].loaded = 1;

    objects[GUN_FLASH].control = ControlGunShot;

    objects[GLOW].control = ControlGlow;

    objects[HOT_LIQUID].control = ControlHotLiquid;
    objects[HOT_LIQUID].semi_transparent = 1;

    objects[BIRD_TWEETER].control = ControlBirdTweeter;
    objects[BIRD_TWEETER].draw_routine = DrawDummyItem;

    objects[WATER_DRIP].control = ControlBirdTweeter;
    objects[WATER_DRIP].draw_routine = DrawDummyItem;

    objects[ALARM_SOUND].control = ControlAlarmSound;
    objects[ALARM_SOUND].save_flags = 1;

    objects[DING_DONG].control = ControlDingDong;
    objects[DING_DONG].draw_routine = DrawDummyItem;

    objects[LARA_ALARM].control = ControlLaraAlarm;
    objects[LARA_ALARM].draw_routine = DrawDummyItem;
    objects[LARA_ALARM].save_flags = 1;

    objects[CLOCK_CHIMES].control = ControlClockChimes;
    objects[CLOCK_CHIMES].draw_routine = DrawDummyItem;
    objects[CLOCK_CHIMES].save_flags = 1;

    objects[FINAL_LEVEL].draw_routine = DrawDummyItem;
    objects[FINAL_LEVEL].control = FinalLevelCounter;
    objects[FINAL_LEVEL].save_flags = 1;

    objects[CUT_SHOTGUN].control = ControlCutShotgun;
    objects[CUT_SHOTGUN].save_flags = objects[CUT_SHOTGUN].save_anim = 1;

    objects[EARTHQUAKE].draw_routine = DrawDummyItem;
    objects[EARTHQUAKE].control = EarthQuake;
    objects[EARTHQUAKE].save_flags = 1;
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

#ifdef TR2_EXE
OBJECT_INFO objects[265];
sint32 *bones;
#endif
