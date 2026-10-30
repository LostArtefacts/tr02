#include "game.h"
#include "savegame.h"

#define GUN_ITEM 135
#define SHOTGUN_ITEM 136
#define MAGNUM_ITEM 137
#define UZI_ITEM 138
#define HARPOON_ITEM 139
#define M16_ITEM 140
#define GRENADE_ITEM 141
#define MEDI_ITEM 149
#define BIGMEDI_ITEM 150
#define FLAREBOX_ITEM 151

int Inv_AddItem(int itemNum);

#define COMMAND_MOVE_ORIGIN 1
#define COMMAND_JUMP_VELOCITY 2
#define COMMAND_ATTACK_READY 3
#define COMMAND_DEACTIVATE 4
#define COMMAND_SOUND_FX 5
#define COMMAND_EFFECT 6
#define SFX_LANDANDWATER 0
#define SFX_LANDONLY 0x4000
#define SFX_WATERONLY 0x8000
#define GRAVITY 6

extern sint16 *commands;
int GetChange(ITEM_INFO *item, ANIM_STRUCT *anim);
void TranslateItem(ITEM_INFO *item, sint32 x, sint32 y, sint32 z);

void AnimateLara(ITEM_INFO *item)
{
    int i;
    ANIM_STRUCT *anim;
    sint16 *command;
    sint32 speed;
    uint16 type, num;

    item->frame_number++;
    anim = &anims[item->anim_number];
    if (anim->number_changes > 0)
    {
        if (GetChange(item, anim))
        {
            anim = &anims[item->anim_number];
            item->current_anim_state = anim->current_anim_state;
        }
    }
    if (item->frame_number > anim->frame_end)
    {
        if (anim->number_commands > 0)
        {
            command = &commands[anim->command_index];
            for (i = anim->number_commands; i > 0; i--)
            {
                switch (*(command++))
                {
                case COMMAND_MOVE_ORIGIN:
                    TranslateItem(item, (sint32)*command, (sint32)*(command + 1), (sint32)*(command + 2));
                    command += 3;
                    break;

                case COMMAND_JUMP_VELOCITY:
                    item->fallspeed = *(command++);
                    item->speed = *(command++);
                    item->gravity_status = 1;
                    if (lara.calc_fallspeed)
                    {
                        item->fallspeed = lara.calc_fallspeed;
                        lara.calc_fallspeed = 0;
                    }
                    break;

                case COMMAND_ATTACK_READY:
                    if (lara.gun_status != LG_SPECIAL)
                        lara.gun_status = LG_ARMLESS;
                    break;

                case COMMAND_DEACTIVATE:
                    break;

                case COMMAND_SOUND_FX:
                case COMMAND_EFFECT:
                    command += 2;
                    break;
                }
            }
        }
        item->anim_number = anim->jump_anim_num;
        item->frame_number = anim->jump_frame_num;
        anim = &anims[item->anim_number];
        item->current_anim_state = anim->current_anim_state;
    }
    if (anim->number_commands > 0)
    {
        command = &commands[anim->command_index];
        for (i = anim->number_commands; i > 0; i--)
        {
            switch (*(command++))
            {
            case COMMAND_MOVE_ORIGIN:
                command += 3;
                break;

            case COMMAND_JUMP_VELOCITY:
                command += 2;
                break;

            case COMMAND_ATTACK_READY:
            case COMMAND_DEACTIVATE:
                break;

            case COMMAND_SOUND_FX:
                if (item->frame_number == *command)
                {
                    type = *(command + 1) & 0xc000;
                    if (type == SFX_LANDANDWATER ||
                        (type == SFX_LANDONLY && (lara.water_surface_dist >= 0 || lara.water_surface_dist == NO_HEIGHT)) ||
                        (type == SFX_WATERONLY && lara.water_surface_dist < 0 && lara.water_surface_dist != NO_HEIGHT))
                    {
                        num = *(command + 1) & 0x3fff;
                        SoundEffect((int)num, &item->pos, SFX_ALWAYS);
                    }
                }
                command += 2;
                break;

            case COMMAND_EFFECT:
                if (item->frame_number == *command)
                {
                    type = *(command + 1) & 0xc000;
                    if (type == SFX_LANDANDWATER ||
                        (type == SFX_LANDONLY && (lara.water_surface_dist >= 0 || lara.water_surface_dist == NO_HEIGHT)) ||
                        (type == SFX_WATERONLY && lara.water_surface_dist < 0))
                    {
                        num = *(command + 1) & 0x3fff;
                        (*effect_routines[(int)num])(item);
                    }
                }
                command += 2;
                break;
            }
        }
    }
    if (!item->gravity_status)
    {
        speed = anim->velocity;
        if (anim->acceleration)
            speed += anim->acceleration * (item->frame_number - anim->frame_base);
        item->speed = (sint16)(speed >> 16);
    }
    else
    {
        speed = anim->velocity + anim->acceleration * (item->frame_number - anim->frame_base - 1);
        item->speed -= (sint16)(speed >> 16);
        speed += anim->acceleration;
        item->speed += (sint16)(speed >> 16);
        item->fallspeed += (item->fallspeed < FASTFALL_SPEED) ? GRAVITY : 1;
        item->pos.y_pos += item->fallspeed;
    }
    item->pos.x_pos += (phd_sin(lara.move_angle) * item->speed) >> W2V_SHIFT;
    item->pos.z_pos += (phd_cos(lara.move_angle) * item->speed) >> W2V_SHIFT;
}

#define GUN_OPTION 157
#define SHOTGUN_OPTION 158
#define MAGNUM_OPTION 159
#define UZI_OPTION 160
#define HARPOON_OPTION 161
#define M16_OPTION 162
#define GRENADE_OPTION 163
#define MEDI_OPTION 171
#define BIGMEDI_OPTION 172
#define FLAREBOX_OPTION 173
#define NUMBER_OBJECTS 265
#define LARA_HITPOINTS 1000

int Inv_RemoveItem(int itemNum);

void UseItem(sint16 object_num)
{
    if (object_num < 0 || object_num >= NUMBER_OBJECTS)
        return;
    switch (object_num)
    {
    case GUN_ITEM:
    case GUN_OPTION:
        lara.request_gun_type = LG_PISTOLS;
        break;

    case MAGNUM_ITEM:
    case MAGNUM_OPTION:
        lara.request_gun_type = LG_MAGNUMS;
        break;

    case UZI_ITEM:
    case UZI_OPTION:
        lara.request_gun_type = LG_UZIS;
        break;

    case SHOTGUN_ITEM:
    case SHOTGUN_OPTION:
        lara.request_gun_type = LG_SHOTGUN;
        break;

    case HARPOON_ITEM:
    case HARPOON_OPTION:
        lara.request_gun_type = LG_HARPOON;
        break;

    case M16_ITEM:
    case M16_OPTION:
        lara.request_gun_type = LG_M16;
        break;

    case GRENADE_ITEM:
    case GRENADE_OPTION:
        lara.request_gun_type = LG_GRENADE;
        break;

    case FLAREBOX_ITEM:
    case FLAREBOX_OPTION:
        lara.request_gun_type = LG_FLARE;
        break;

    case MEDI_ITEM:
    case MEDI_OPTION:
        if (lara_item->hit_points > 0 && lara_item->hit_points < LARA_HITPOINTS)
        {
            lara_item->hit_points += LARA_HITPOINTS / 2;
            if (lara_item->hit_points > LARA_HITPOINTS)
                lara_item->hit_points = LARA_HITPOINTS;
            Inv_RemoveItem(MEDI_ITEM);
            SoundEffect(116, NULL, SFX_ALWAYS);
            savegame.health_used++;
        }
        break;

    case BIGMEDI_ITEM:
    case BIGMEDI_OPTION:
        if (lara_item->hit_points > 0 && lara_item->hit_points < LARA_HITPOINTS)
        {
            lara_item->hit_points += LARA_HITPOINTS;
            if (lara_item->hit_points > LARA_HITPOINTS)
                lara_item->hit_points = LARA_HITPOINTS;
            Inv_RemoveItem(BIGMEDI_ITEM);
            SoundEffect(116, NULL, SFX_ALWAYS);
            savegame.health_used += 2;
        }
        break;

    default:
        break;
    }
}

void LaraCheatGetStuff(void)
{
    int i;

    Inv_AddItem(HARPOON_ITEM);
    Inv_AddItem(M16_ITEM);
    Inv_AddItem(GRENADE_ITEM);
    Inv_AddItem(SHOTGUN_ITEM);
    Inv_AddItem(UZI_ITEM);
    Inv_AddItem(MAGNUM_ITEM);
    Inv_AddItem(GUN_ITEM);
    for (i = 0; i < 10; i++)
    {
        Inv_AddItem(FLAREBOX_ITEM);
        Inv_AddItem(MEDI_ITEM);
        Inv_AddItem(BIGMEDI_ITEM);
    }

    lara.magnums.ammo = 1000;
    lara.uzis.ammo = 2000;
    lara.shotgun.ammo = 300;
    lara.harpoon.ammo = 300;
    lara.m16.ammo = 300;
    lara.grenade.ammo = 300;
}

void ControlLaraExtra(sint16 item_num)
{
    AnimateItem(&items[item_num]);
}

void InitialiseLaraLoad(sint16 item_num)
{
    lara.item_number = item_num;
    lara_item = &items[item_num];
}

#define LARA_AIR 1800
#define UNDERWATER 1
#define LARA_EXTRA 12
#define EXTRA_BREATH 0
#define TREAD_A 108
#define TREAD_F GF(TREAD_A, 0)

extern sint32 GF_LaraStartAnim;
extern sint16 cine_frame;
extern PHD_3DPOS cinematic_pos;
void InitialiseLaraInventory(int level_number);

void InitialiseLara(int type)
{
    ITEM_INFO *item;
    int i;

    item = lara_item;
    item->collidable = 0;
    item->data = (void *)&lara;
    item->hit_points = LARA_HITPOINTS;
    lara.calc_fallspeed = 0;
    lara.climb_status = 0;
    lara.pose_count = 0;
    lara.hit_frame = 0;
    lara.hit_direction = -1;
    lara.air = LARA_AIR;
    lara.dive_count = 0;
    lara.death_count = 0;
    lara.current_active = 0;
    lara.spaz_effect_count = 0;
    lara.flare_age = 0;
    lara.skidoo = NO_ITEM;
    lara.weapon_item = NO_ITEM;
    lara.back_gun = 0;
    lara.flare_frame = 0;
    lara.flare_control_left = lara.flare_control_right = 0;
    lara.extra_anim = 0;
    lara.look = 1;
    lara.burn = 0;
    lara.water_surface_dist = 100;
    lara.last_pos.x = item->pos.x_pos;
    lara.last_pos.y = item->pos.y_pos;
    lara.last_pos.z = item->pos.z_pos;
    lara.spaz_effect = NULL;
    lara.mesh_effects = 0;
    lara.target = NULL;
    lara.turn_rate = 0;
    lara.move_angle = 0;
    lara.head_x_rot = lara.head_y_rot = lara.head_z_rot = 0;
    lara.torso_x_rot = lara.torso_y_rot = lara.torso_z_rot = 0;
    lara.left_arm.flash_gun = lara.right_arm.flash_gun = 0;
    lara.left_arm.lock = lara.right_arm.lock = 0;
    lara.creature = NULL;
    if (type == IL_NORMAL && GF_LaraStartAnim)
    {
        lara.water_status = LARA_ABOVEWATER;
        lara.gun_status = LG_HANDSBUSY;
        item->anim_number = objects[LARA_EXTRA].anim_index;
        item->frame_number = anims[item->anim_number].frame_base;
        item->current_anim_state = EXTRA_BREATH;
        item->goal_anim_state = (sint16)GF_LaraStartAnim;
        AnimateLara(item);
        lara.extra_anim = 1;
        camera.type = CINEMATIC_CAMERA;
        cine_frame = 0;
        cinematic_pos = item->pos;
    }
    else if (room[item->room_number].flags & UNDERWATER)
    {
        lara.water_status = LARA_UNDERWATER;
        item->fallspeed = 0;
        item->goal_anim_state = AS_TREAD;
        item->current_anim_state = AS_TREAD;
        item->anim_number = TREAD_A;
        item->frame_number = TREAD_F;
    }
    else
    {
        lara.water_status = LARA_ABOVEWATER;
        item->goal_anim_state = AS_STOP;
        item->current_anim_state = AS_STOP;
        item->anim_number = STOP_A;
        item->frame_number = STOP_F;
    }
    if (type == IL_CUTSCENE)
    {
        for (i = 0; i < 15; i++)
            lara.mesh_ptrs[i] = meshes[objects[LARA].mesh_index + i];
        lara.mesh_ptrs[THIGH_L] = meshes[objects[PISTOLS].mesh_index + THIGH_L];
        lara.mesh_ptrs[THIGH_R] = meshes[objects[PISTOLS].mesh_index + THIGH_R];
        lara.gun_status = LG_ARMLESS;
    }
    else
        InitialiseLaraInventory(CurrentLevel);
}

#define MAP_CLOSED 134
#define FLARE_ITEM 152
#define SG_AMMO_ITEM 143
#define MAG_AMMO_ITEM 144
#define UZI_AMMO_ITEM 145
#define HARPOON_AMMO_ITEM 146
#define M16_AMMO_ITEM 147
#define ROCKET_AMMO_ITEM 148
#define MAGNUM_AMMO_QTY 40
#define UZI_AMMO_QTY 80
#define SHOTGUN_AMMO_QTY 12
#define ROCKET_AMMO_QTY 2
#define M16_AMMO_QTY 40
#define HARPOON_AMMO_QTY 3

extern sint16 GF_RemoveWeapons;
extern sint16 GF_RemoveAmmo;
void Inv_RemoveAllItems(void);
int GlobalItemReplace(int in_objnum, int out_objnum);
void LaraInitialiseMeshes(int level_number);
void InitialiseNewWeapon(void);

void InitialiseLaraInventory(int level_number)
{
    int i, ammo;
    START_INFO *start;

    Inv_RemoveAllItems();
    start = &savegame.start[level_number];
    if (GF_RemoveWeapons)
    {
        start->got_pistols = start->got_magnums = start->got_uzis = start->got_shotgun = start->got_m16 = start->got_rocket = start->got_harpoon = 0;
        start->gun_type = LG_UNARMED;
        start->gun_status = LG_ARMLESS;
        GF_RemoveWeapons = 0;
    }
    if (GF_RemoveAmmo)
    {
        start->pistol_ammo = start->magnum_ammo = start->uzi_ammo = start->shotgun_ammo = start->harpoon_ammo = start->rocket_ammo = start->m16_ammo = 0;
        start->num_medis = start->num_big_medis = start->num_flares = 0;
        GF_RemoveAmmo = 0;
    }
    Inv_AddItem(MAP_CLOSED);
    lara.pistols.ammo = 1000;
    if (start->got_pistols)
        Inv_AddItem(GUN_ITEM);
    if (start->got_magnums)
    {
        Inv_AddItem(MAGNUM_ITEM);
        lara.magnums.ammo = start->magnum_ammo;
        GlobalItemReplace(MAGNUM_ITEM, MAG_AMMO_ITEM);
    }
    else
    {
        ammo = start->magnum_ammo / MAGNUM_AMMO_QTY;
        for (i = 0; i < ammo; i++)
            Inv_AddItem(MAG_AMMO_ITEM);
        lara.magnums.ammo = 0;
    }
    if (start->got_uzis)
    {
        Inv_AddItem(UZI_ITEM);
        lara.uzis.ammo = start->uzi_ammo;
        GlobalItemReplace(UZI_ITEM, UZI_AMMO_ITEM);
    }
    else
    {
        ammo = start->uzi_ammo / UZI_AMMO_QTY;
        for (i = 0; i < ammo; i++)
            Inv_AddItem(UZI_AMMO_ITEM);
        lara.uzis.ammo = 0;
    }
    if (start->got_shotgun)
    {
        Inv_AddItem(SHOTGUN_ITEM);
        lara.shotgun.ammo = start->shotgun_ammo;
        GlobalItemReplace(SHOTGUN_ITEM, SG_AMMO_ITEM);
    }
    else
    {
        ammo = start->shotgun_ammo / SHOTGUN_AMMO_QTY;
        for (i = 0; i < ammo; i++)
            Inv_AddItem(SG_AMMO_ITEM);
        lara.shotgun.ammo = 0;
    }
    if (start->got_rocket)
    {
        Inv_AddItem(GRENADE_ITEM);
        lara.grenade.ammo = start->rocket_ammo;
        GlobalItemReplace(GRENADE_ITEM, ROCKET_AMMO_ITEM);
    }
    else
    {
        ammo = start->rocket_ammo / ROCKET_AMMO_QTY;
        for (i = 0; i < ammo; i++)
            Inv_AddItem(ROCKET_AMMO_ITEM);
        lara.grenade.ammo = 0;
    }
    if (start->got_m16)
    {
        Inv_AddItem(M16_ITEM);
        lara.m16.ammo = start->m16_ammo;
        GlobalItemReplace(M16_ITEM, M16_AMMO_ITEM);
    }
    else
    {
        ammo = start->m16_ammo / M16_AMMO_QTY;
        for (i = 0; i < ammo; i++)
            Inv_AddItem(M16_AMMO_ITEM);
        lara.m16.ammo = 0;
    }
    if (start->got_harpoon)
    {
        Inv_AddItem(HARPOON_ITEM);
        lara.harpoon.ammo = start->harpoon_ammo;
        GlobalItemReplace(HARPOON_ITEM, HARPOON_AMMO_ITEM);
    }
    else
    {
        ammo = start->harpoon_ammo / HARPOON_AMMO_QTY;
        for (i = 0; i < ammo; i++)
            Inv_AddItem(HARPOON_AMMO_ITEM);
        lara.harpoon.ammo = 0;
    }
    for (i = start->num_flares; i > 0; i--)
        Inv_AddItem(FLARE_ITEM);
    for (i = start->num_medis; i > 0; i--)
        Inv_AddItem(MEDI_ITEM);
    for (i = start->num_big_medis; i > 0; i--)
        Inv_AddItem(BIGMEDI_ITEM);
    lara.gun_status = LG_ARMLESS;
    lara.request_gun_type = lara.gun_type = lara.last_gun_type = start->gun_type;
    LaraInitialiseMeshes(level_number);
    InitialiseNewWeapon();
}
