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
