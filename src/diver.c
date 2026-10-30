#include "game.h"
#include "box.h"

void ShootAtLara(FX_INFO *fx);

#define DIVER_HARPOON 245
#define DIVER_HARPOON_SPEED 150

sint16 Harpoon(sint32 x, sint32 y, sint32 z, sint16 speed, PHD_ANGLE yrot, sint16 room_number)
{
    sint16 fx_number;
    FX_INFO *fx;

    fx_number = CreateEffect(room_number);
    if (fx_number != NO_ITEM)
    {
        fx = &effects[fx_number];
        fx->pos.x_pos = x;
        fx->pos.y_pos = y;
        fx->pos.z_pos = z;
        fx->room_number = room_number;
        fx->pos.x_rot = fx->pos.z_rot = 0;
        fx->pos.y_rot = yrot;
        fx->speed = DIVER_HARPOON_SPEED;
        fx->fallspeed = 0;
        fx->frame_number = 0;
        fx->object_number = DIVER_HARPOON;
        fx->shade = 14 * 256;
        ShootAtLara(fx);
    }
    return fx_number;
}

#define UNDERWATER 1
#define NO_ROOM 0xff

sint32 GetWaterSurface(sint32 x, sint32 y, sint32 z, sint16 room_number)
{
    ROOM_INFO *r;
    FLOOR_INFO *floor;

    r = &room[room_number];
    floor = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];

    if (r->flags & UNDERWATER)
    {
        while (floor->sky_room != NO_ROOM)
        {
            r = &room[floor->sky_room];
            if (!(r->flags & UNDERWATER))
                return floor->ceiling << 8;
            floor = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];
        }
        return NO_HEIGHT;
    }
    else
    {
        while (floor->pit_room != NO_ROOM)
        {
            r = &room[floor->pit_room];
            if (r->flags & UNDERWATER)
                return floor->floor << 8;
            floor = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];
        }
    }
    return NO_HEIGHT;
}

typedef struct {
    sint16 zone_number, enemy_zone;
    sint32 distance, ahead, bite;
    sint16 angle, enemy_facing;
} AI_INFO;

int CreatureActive(sint16 item_number);
void CreatureAIInfo(ITEM_INFO *item, AI_INFO *info);
void CreatureMood(ITEM_INFO *item, AI_INFO *info, int violent);
sint16 CreatureTurn(ITEM_INFO *item, sint16 maximum_turn);
void CreatureHead(ITEM_INFO *item, sint16 required);
void CreatureNeck(ITEM_INFO *item, sint16 required);
int CreatureAnimation(sint16 item_number, sint16 angle, sint16 tilt);
void CreatureFloat(sint16 item_number);
void CreatureUnderwater(ITEM_INFO *item, sint32 depth);
int LOS(GAME_VECTOR *start, GAME_VECTOR *target);

#define DIVER 29
#define DIVER_DIE_ANIM 16
#define DIVER_SWIM1_TURN (ONE_DEGREE * 3)
#define DIVER_SWIM2_TURN (ONE_DEGREE * 3)

enum diver_anim {
    DIVER_EMPTY,
    DIVER_SWIM1,
    DIVER_SWIM2,
    DIVER_SHOOT1,
    DIVER_AIM1,
    DIVER_NULL1,
    DIVER_AIM2,
    DIVER_SHOOT2,
    DIVER_NULL2,
    DIVER_DEATH
};

extern BITE_INFO diver_poon;

void DiverControl(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *diver;
    sint16 angle, head, neck;
    AI_INFO info;
    sint32 water_level, shoot;
    GAME_VECTOR start, target;

    if (!CreatureActive(item_number))
        return;

    item = &items[item_number];
    diver = (CREATURE_INFO *)item->data;
    angle = head = neck = 0;

    if (item->hit_points <= 0)
    {
        if (item->current_anim_state != DIVER_DEATH)
        {
            item->anim_number = objects[DIVER].anim_index + DIVER_DIE_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = DIVER_DEATH;
        }
        CreatureFloat(item_number);
        return;
    }
    else
    {
        CreatureAIInfo(item, &info);
        CreatureMood(item, &info, 0);

        if (lara.water_status == LARA_ABOVEWATER)
        {
            start.x = item->pos.x_pos;
            start.y = item->pos.y_pos - STEP_L;
            start.z = item->pos.z_pos;
            start.room_number = item->room_number;

            target.x = lara_item->pos.x_pos;
            target.y = lara_item->pos.y_pos - (LARA_HITE - 150);
            target.z = lara_item->pos.z_pos;

            shoot = LOS(&start, &target);
            if (shoot)
            {
                diver->target.x = lara_item->pos.x_pos;
                diver->target.y = lara_item->pos.y_pos;
                diver->target.z = lara_item->pos.z_pos;
            }

            if (info.angle < -0x2000 || info.angle > 0x2000)
                shoot = 0;
        }
        else if (info.angle > -0x2000 && info.angle < 0x2000)
        {
            start.x = item->pos.x_pos;
            start.y = item->pos.y_pos;
            start.z = item->pos.z_pos;
            start.room_number = item->room_number;

            target.x = lara_item->pos.x_pos;
            target.y = lara_item->pos.y_pos;
            target.z = lara_item->pos.z_pos;

            shoot = LOS(&start, &target);
        }
        else
            shoot = 0;

        angle = CreatureTurn(item, diver->maximum_turn);

        water_level = GetWaterSurface(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number) + WALL_L / 2;

        switch (item->current_anim_state)
        {
        case DIVER_SWIM1:
            diver->maximum_turn = DIVER_SWIM1_TURN;
            if (shoot)
                neck = -info.angle;
            if (diver->target.y < water_level && item->pos.y_pos < water_level + diver->LOT.fly)
                item->goal_anim_state = DIVER_SWIM2;
            else if (diver->mood == ESCAPE_MOOD)
                break;
            else if (shoot)
                item->goal_anim_state = DIVER_AIM1;
            break;

        case DIVER_AIM1:
            diver->flags = 0;
            if (shoot)
                neck = -info.angle;
            if (!shoot || diver->mood == ESCAPE_MOOD || (diver->target.y < water_level && item->pos.y_pos < water_level + diver->LOT.fly))
                item->goal_anim_state = DIVER_SWIM1;
            else
                item->goal_anim_state = DIVER_SHOOT1;
            break;

        case DIVER_SHOOT1:
            if (shoot)
                neck = -info.angle;
            if (!diver->flags)
            {
                CreatureEffect(item, &diver_poon, Harpoon);
                diver->flags = 1;
            }
            break;

        case DIVER_SWIM2:
            diver->maximum_turn = DIVER_SWIM2_TURN;
            if (shoot)
                head = info.angle;
            if (diver->target.y > water_level)
                item->goal_anim_state = DIVER_SWIM1;
            else if (diver->mood == ESCAPE_MOOD)
                break;
            else if (shoot)
                item->goal_anim_state = DIVER_AIM2;
            break;

        case DIVER_AIM2:
            diver->flags = 0;
            if (shoot)
                head = info.angle;
            if (!shoot || diver->mood == ESCAPE_MOOD || diver->target.y > water_level)
                item->goal_anim_state = DIVER_SWIM2;
            else
                item->goal_anim_state = DIVER_SHOOT2;
            break;

        case DIVER_SHOOT2:
            if (shoot)
                head = info.angle;
            if (!diver->flags)
            {
                CreatureEffect(item, &diver_poon, Harpoon);
                diver->flags = 1;
            }
            break;
        }
    }

    CreatureHead(item, head);
    CreatureNeck(item, neck);
    CreatureAnimation(item_number, angle, 0);

    switch (item->current_anim_state)
    {
    case DIVER_SWIM1:
    case DIVER_AIM1:
    case DIVER_SHOOT1:
        CreatureUnderwater(item, WALL_L / 2);
        break;

    default:
        item->pos.y_pos = water_level - WALL_L / 2;
    }
}
