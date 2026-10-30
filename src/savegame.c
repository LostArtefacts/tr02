#include "game.h"
#include "savegame.h"

extern int SG_Count;
extern char *SG_Point;

void ModifyStartInfo(int level_number);

void InitialiseStartInfo(void)
{
    int i;

    if (savegame.bonus_flag)
        return;

    for (i = 0; i < MAXIMUM_LEVELS; i++)
    {
        ModifyStartInfo(i);
        savegame.start[i].available = 0;
        savegame.start[i].timer = 0;
        savegame.start[i].ammo_used = 0;
        savegame.start[i].ammo_hit = 0;
        savegame.start[i].distance_travelled = 0;
        savegame.start[i].kills = 0;
        savegame.start[i].secrets_found = 0;
        savegame.start[i].health_used = 0;
    }
    savegame.start[0].available = 1;
    savegame.start[1].available = 1;
    savegame.bonus_flag = 0;
}

#define SHOTGUN_AMMO_CLIP 6

void ModifyStartInfo(int level_number)
{
    START_INFO *start;

    start = &savegame.start[level_number];

    start->got_pistols = 1;
    start->gun_type = LG_PISTOLS;
    start->pistol_ammo = 1000;
    switch (level_number)
    {
    case LV_GYM:
        start->available = 1;
        start->pistol_ammo = start->magnum_ammo = start->uzi_ammo = start->shotgun_ammo = start->harpoon_ammo = start->rocket_ammo = start->m16_ammo = start->harpoon_ammo = start->rocket_ammo = 0;
        start->got_pistols = start->got_magnums = start->got_uzis = start->got_shotgun = start->got_harpoon = start->got_rocket = start->got_m16 = start->got_harpoon = start->got_rocket = 0;
        start->num_medis = start->num_big_medis = start->num_flares = 0;
        start->gun_type = LG_UNARMED;
        start->gun_status = LG_ARMLESS;
        break;

    case LV_FIRSTLEVEL:
        start->available = 1;
        start->got_shotgun = 1;
        start->shotgun_ammo = 2 * SHOTGUN_AMMO_CLIP;
        start->num_flares = 2;
        start->num_medis = 1;
        start->num_big_medis = 1;
        start->got_magnums = start->got_uzis = start->got_m16 = start->got_harpoon = start->got_rocket = 0;
        start->magnum_ammo = start->uzi_ammo = start->m16_ammo = start->harpoon_ammo = start->rocket_ammo = 0;
        start->gun_status = LG_ARMLESS;
        break;
    }
    if (savegame.bonus_flag && level_number != LV_GYM)
    {
        start->available = 1;
        start->got_pistols = start->got_magnums = start->got_uzis = start->got_shotgun = start->got_rocket = start->got_m16 = start->got_harpoon = 1;
        start->magnum_ammo = start->uzi_ammo = start->shotgun_ammo = start->rocket_ammo = start->m16_ammo = start->harpoon_ammo = 10001;
        start->gun_type = LG_GRENADE;
        start->num_flares = 255;
    }
}

#define GUN_ITEM 135
#define SHOTGUN_ITEM 136
#define MAGNUM_ITEM 137
#define UZI_ITEM 138
#define HARPOON_ITEM 139
#define M16_ITEM 140
#define ROCKET_GUN_ITEM 141
#define SG_AMMO_ITEM 143
#define MAG_AMMO_ITEM 144
#define UZI_AMMO_ITEM 145
#define HARPOON_AMMO_ITEM 146
#define M16_AMMO_ITEM 147
#define ROCKET_AMMO_ITEM 148
#define MEDI_ITEM 149
#define BIGMEDI_ITEM 150
#define FLAREBOX_ITEM 152
#define MAGNUM_AMMO_QTY 40
#define UZI_AMMO_QTY 80
#define SHOTGUN_AMMO_QTY (SHOTGUN_AMMO_CLIP * 2)
#define HARPOON_AMMO_QTY 3
#define M16_AMMO_QTY 40
#define ROCKET_AMMO_QTY 2

int Inv_RequestItem(int itemNum);

void CreateStartInfo(int level_number)
{
    START_INFO *start;

    start = &savegame.start[level_number];
    start->available = 1;

    if (Inv_RequestItem(GUN_ITEM))
    {
        start->pistol_ammo = 1000;
        start->got_pistols = 1;
    }
    else
    {
        start->pistol_ammo = 1000;
        start->got_pistols = 0;
    }

    if (Inv_RequestItem(MAGNUM_ITEM))
    {
        start->magnum_ammo = lara.magnums.ammo;
        start->got_magnums = 1;
    }
    else
    {
        start->magnum_ammo = Inv_RequestItem(MAG_AMMO_ITEM) * MAGNUM_AMMO_QTY;
        start->got_magnums = 0;
    }

    if (Inv_RequestItem(UZI_ITEM))
    {
        start->uzi_ammo = lara.uzis.ammo;
        start->got_uzis = 1;
    }
    else
    {
        start->uzi_ammo = Inv_RequestItem(UZI_AMMO_ITEM) * UZI_AMMO_QTY;
        start->got_uzis = 0;
    }

    if (Inv_RequestItem(SHOTGUN_ITEM))
    {
        start->shotgun_ammo = lara.shotgun.ammo;
        start->got_shotgun = 1;
    }
    else
    {
        start->shotgun_ammo = Inv_RequestItem(SG_AMMO_ITEM) * SHOTGUN_AMMO_QTY;
        start->got_shotgun = 0;
    }

    if (Inv_RequestItem(HARPOON_ITEM))
    {
        start->harpoon_ammo = lara.harpoon.ammo;
        start->got_harpoon = 1;
    }
    else
    {
        start->harpoon_ammo = Inv_RequestItem(HARPOON_AMMO_ITEM) * HARPOON_AMMO_QTY;
        start->got_harpoon = 0;
    }

    if (Inv_RequestItem(M16_ITEM))
    {
        start->m16_ammo = lara.m16.ammo;
        start->got_m16 = 1;
    }
    else
    {
        start->m16_ammo = Inv_RequestItem(M16_AMMO_ITEM) * M16_AMMO_QTY;
        start->got_m16 = 0;
    }

    if (Inv_RequestItem(ROCKET_GUN_ITEM))
    {
        start->rocket_ammo = lara.grenade.ammo;
        start->got_rocket = 1;
    }
    else
    {
        start->rocket_ammo = Inv_RequestItem(ROCKET_AMMO_ITEM) * ROCKET_AMMO_QTY;
        start->got_rocket = 0;
    }

    start->num_flares = Inv_RequestItem(FLAREBOX_ITEM);
    start->num_medis = Inv_RequestItem(MEDI_ITEM);
    start->num_big_medis = Inv_RequestItem(BIGMEDI_ITEM);

    if (lara.gun_type == LG_FLARE)
        start->gun_type = (char)lara.last_gun_type;
    else
        start->gun_type = (char)lara.gun_type;
    start->gun_status = LG_ARMLESS;
}

#include "box.h"

#define SAVE_CREATURE (1 << 7)
#define KILLED_ITEM ((sint16)0x8000)
#define MAX_FLIPMAPS 10
#define MAX_CD_TRACKS 64
#define DRAGON_FRONT 22
#define SKIDMAN 52
#define SKIDOO 13
#define MOTORBOAT 14
#define LIFT 65
#define SMASH_WINDOW 72
#define SMASH_ICE 73
#define MINES 222
#define FLARE_ITEM 152
#define PICKUP_ITEM1 205
#define PICKUP_ITEM2 206
#define PUZZLE_ITEM1 174
#define PUZZLE_ITEM2 175
#define PUZZLE_ITEM3 176
#define PUZZLE_ITEM4 177
#define KEY_ITEM1 193
#define KEY_ITEM2 194
#define KEY_ITEM3 195
#define KEY_ITEM4 196
#define PUZZLE_DONE_OFFSET 4

extern int flipmap[];
extern sint16 cd_flags[];
extern int monks_attack_lara;
extern int level_items;
extern sint16 *Meshes;
extern sint16 *AnimFrames;
extern sint16 body_bag;
extern int flipeffect;
extern int fliptimer;

void ResetSG(void);
void ReadSG(void *pointer, int size);
void InitialiseLaraInventory(int level_number);
int Inv_AddItem(int item_number);
void FlipMap(void);
void AlterFloorHeight(ITEM_INFO *item, int height);
void MovableBlock(sint16 item_number);
void ItemNewRoom(sint16 item_num, sint16 room_number);
void KillItem(sint16 item_num);
void AddActiveItem(sint16 item_num);
int EnableBaddieAI(sint16 item_number, int always);
void PuzzleHoleCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void PickUpCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void RemoveDrawnItem(sint16 item_num);
void InitialiseSkidoo(sint16 item_number);
void DragonBones(sint16 item_number);
sint16 CreateItem(void);
void InitialiseItem(sint16 item_num);
void LaraBurn(void);

void ExtractSaveGameInfo(void)
{
    int number_flares, flare_age;
    sint16 i, item_number;
    OBJECT_INFO *obj;
    ITEM_INFO *item;
    FLOOR_INFO *floor;
    sint16 room_number;
    char tflag;
    CREATURE_INFO *creature;

    InitialiseLaraInventory(CurrentLevel);

    for (i = savegame.num_pickup1; i > 0; i--)
        Inv_AddItem(PICKUP_ITEM1);
    for (i = savegame.num_pickup2; i > 0; i--)
        Inv_AddItem(PICKUP_ITEM2);
    for (i = savegame.num_puzzle1; i > 0; i--)
        Inv_AddItem(PUZZLE_ITEM1);
    for (i = savegame.num_puzzle2; i > 0; i--)
        Inv_AddItem(PUZZLE_ITEM2);
    for (i = savegame.num_puzzle3; i > 0; i--)
        Inv_AddItem(PUZZLE_ITEM3);
    for (i = savegame.num_puzzle4; i > 0; i--)
        Inv_AddItem(PUZZLE_ITEM4);
    for (i = savegame.num_key1; i > 0; i--)
        Inv_AddItem(KEY_ITEM1);
    for (i = savegame.num_key2; i > 0; i--)
        Inv_AddItem(KEY_ITEM2);
    for (i = savegame.num_key3; i > 0; i--)
        Inv_AddItem(KEY_ITEM3);
    for (i = savegame.num_key4; i > 0; i--)
        Inv_AddItem(KEY_ITEM4);

    ResetSG();

    ReadSG(&i, sizeof(int));
    if (i)
        FlipMap();

    for (i = 0; i < MAX_FLIPMAPS; i++)
    {
        ReadSG(&tflag, sizeof(char));
        flipmap[i] = (sint16)tflag << 8;
    }

    ReadSG(cd_flags, sizeof(sint16) * MAX_CD_TRACKS);

    for (i = 0; i < number_cameras; i++)
        ReadSG(&camera.fixed[i].flags, sizeof(sint16));

    for (i = 0, item = items; i < level_items; i++, item++)
    {
        obj = &objects[item->object_number];

        if (obj->control == MovableBlock)
            AlterFloorHeight(item, 1024);

        if (obj->save_position)
        {
            ReadSG(&item->pos, sizeof(PHD_3DPOS));
            ReadSG(&room_number, sizeof(sint16));
            ReadSG(&item->speed, sizeof(sint16));
            ReadSG(&item->fallspeed, sizeof(sint16));
            if (item->room_number != room_number)
                ItemNewRoom(i, room_number);
            if (obj->shadow_size)
            {
                floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
                item->floor = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
            }
        }

        if (obj->save_anim)
        {
            ReadSG(&item->current_anim_state, sizeof(sint16));
            ReadSG(&item->goal_anim_state, sizeof(sint16));
            ReadSG(&item->required_anim_state, sizeof(sint16));
            ReadSG(&item->anim_number, sizeof(sint16));
            ReadSG(&item->frame_number, sizeof(sint16));
        }

        if (obj->save_hitpoints)
            ReadSG(&item->hit_points, sizeof(sint16));

        if (obj->save_flags)
        {
            ReadSG(&item->flags, sizeof(sint16));
            if (obj->intelligent)
                ReadSG(&item->carried_item, sizeof(sint16));
            ReadSG(&item->timer, sizeof(sint16));

            if (item->flags & KILLED_ITEM)
            {
                KillItem(i);
                item->status = DEACTIVATED;
            }
            else
            {
                if ((item->flags & 1) && !item->active)
                    AddActiveItem(i);
                item->status = (item->flags & 6) >> 1;
                if (item->flags & 8)
                    item->gravity_status = 1;
                if (!(item->flags & 0x10))
                    item->collidable = 0;
            }

            if (item->flags & SAVE_CREATURE)
            {
                EnableBaddieAI(i, 1);
                creature = (CREATURE_INFO *)item->data;
                if (creature)
                {
                    ReadSG(&creature->head_rotation, sizeof(sint16));
                    ReadSG(&creature->neck_rotation, sizeof(sint16));
                    ReadSG(&creature->maximum_turn, sizeof(sint16));
                    ReadSG(&creature->flags, sizeof(sint16));
                    ReadSG(&creature->mood, sizeof(MOOD_TYPE));
                }
                else
                    SG_Point += sizeof(sint16) * 4 + sizeof(MOOD_TYPE);
            }
            else if (obj->intelligent)
            {
                item->data = NULL;
                if (item->clear_body && item->hit_points <= 0 && !(item->flags & KILLED_ITEM))
                {
                    item->next_active = body_bag;
                    body_bag = i;
                }
            }

            item->flags &= 0xff00;

            if (obj->collision == PuzzleHoleCollision && (item->status == DEACTIVATED || item->status == ACTIVE))
                item->object_number += PUZZLE_DONE_OFFSET;
            if (obj->collision == PickUpCollision && item->status == DEACTIVATED)
                RemoveDrawnItem(i);
            if ((item->object_number == SMASH_WINDOW || item->object_number == SMASH_ICE) && (item->flags & ONESHOT))
                item->mesh_bits = 0x100;
            if (item->object_number == MINES && (item->flags & ONESHOT))
                item->mesh_bits = 1;
        }

        if (obj->control == MovableBlock && item->status == NOT_ACTIVE)
            AlterFloorHeight(item, -1024);

        if (item->object_number == MOTORBOAT)
            ReadSG(item->data, 24);
        else if (item->object_number == SKIDOO)
            ReadSG(item->data, 22);
        else if (item->object_number == LIFT)
            ReadSG(item->data, sizeof(int) * 2);

        if (item->object_number == SKIDMAN && item->status == DEACTIVATED)
        {
            items[(int)item->data].object_number = SKIDOO;
            InitialiseSkidoo((sint16)item->data);
        }

        if (item->object_number == DRAGON_FRONT && item->status == DEACTIVATED)
        {
            item->pos.y_pos -= 1010;
            DragonBones(i);
            item->pos.y_pos += 1010;
        }
    }

    ReadSG(&lara, sizeof(LARA_INFO));
    lara.creature = NULL;
    lara.spaz_effect = NULL;

    for (i = 0; i < 15; i++)
        lara.mesh_ptrs[i] = (sint16 *)((long)lara.mesh_ptrs[i] + (long)Meshes);
    lara.target = NULL;
    lara.left_arm.frame_base = (sint16 *)((long)lara.left_arm.frame_base + (long)AnimFrames);
    lara.right_arm.frame_base = (sint16 *)((long)lara.right_arm.frame_base + (long)AnimFrames);

    if (lara.weapon_item != NO_ITEM)
    {
        lara.weapon_item = CreateItem();
        item = &items[lara.weapon_item];
        ReadSG(&item->object_number, sizeof(sint16));
        ReadSG(&item->anim_number, sizeof(sint16));
        ReadSG(&item->frame_number, sizeof(sint16));
        ReadSG(&item->current_anim_state, sizeof(sint16));
        ReadSG(&item->goal_anim_state, sizeof(sint16));
        item->status = ACTIVE;
        item->room_number = 255;
    }

    if (lara.burn)
    {
        lara.burn = 0;
        LaraBurn();
    }

    ReadSG(&flipeffect, sizeof(int));
    ReadSG(&fliptimer, sizeof(int));
    ReadSG(&monks_attack_lara, sizeof(int));

    ReadSG(&number_flares, sizeof(int));
    for (i = 0; i < number_flares; i++)
    {
        item_number = CreateItem();
        item = &items[item_number];
        item->object_number = FLARE_ITEM;
        ReadSG(&item->pos, sizeof(PHD_3DPOS));
        ReadSG(&item->room_number, sizeof(sint16));
        ReadSG(&item->speed, sizeof(sint16));
        ReadSG(&item->fallspeed, sizeof(sint16));
        InitialiseItem(item_number);
        AddActiveItem(item_number);
        ReadSG(&flare_age, sizeof(int));
        item->data = (void *)flare_age;
    }
}

void ResetSG(void)
{
    SG_Count = 0;
    SG_Point = savegame.buffer;
}


void S_ExitSystem(char *message);

void WriteSG(void *pointer, int size)
{
    char *data;

    SG_Count += size;
    if (SG_Count >= MAX_SAVEGAME_BUFFER)
        S_ExitSystem("FATAL: Savegame is too big to fit in buffer");

    data = pointer;
    for (; size > 0; size--)
        *(SG_Point++) = *(data++);
}


void ReadSG(void *pointer, int size)
{
    char *data;

    SG_Count += size;
    data = pointer;
    for (; size > 0; size--)
        *(data++) = *(SG_Point++);
}
