#include "game.h"
#include "box.h"

typedef struct {
    sint16 zone_number, enemy_zone;
    sint32 distance, ahead, bite;
    sint16 angle, enemy_facing;
} AI_INFO;

sint16 GunShot(sint32 x, sint32 y, sint32 z, sint16 speed, PHD_ANGLE yrot, sint16 room_number);
void Richochet(GAME_VECTOR *pos);

#define SQUARE(A) ((A) * (A))
#define PEOPLE_SHOOT_RANGE SQUARE(WALL_L * 8)

int LOS(GAME_VECTOR *start, GAME_VECTOR *target);

int Targetable(ITEM_INFO *item, AI_INFO *info)
{
    GAME_VECTOR start, target;
    CREATURE_INFO *creature;
    ITEM_INFO *enemy;

    creature = (CREATURE_INFO *)item->data;
    enemy = creature->enemy;
    if (enemy->hit_points <= 0 || !info->ahead || info->distance >= PEOPLE_SHOOT_RANGE)
        return 0;

    start.x = item->pos.x_pos;
    start.y = item->pos.y_pos - STEP_L * 3;
    start.z = item->pos.z_pos;
    start.room_number = item->room_number;

    target.x = enemy->pos.x_pos;
    target.y = enemy->pos.y_pos - STEP_L * 3;
    target.z = enemy->pos.z_pos;

    return LOS(&start, &target);
}

sint16 GunHit(sint32 x, sint32 y, sint32 z, sint16 speed, PHD_ANGLE yrot, sint16 room_number)
{
    PHD_VECTOR pos;

    pos.x = pos.y = pos.z = 0;
    GetJointAbsPosition(lara_item, &pos, GetRandomControl() * 25 / 0x7fff);
    DoBloodSplat(pos.x, pos.y, pos.z, lara_item->speed, lara_item->pos.y_rot, lara_item->room_number);
    SoundEffect(50, &lara_item->pos, 0);

    return (GunShot(x, y, z, speed, yrot, room_number));
}

sint16 GunMiss(sint32 x, sint32 y, sint32 z, sint16 speed, PHD_ANGLE yrot, sint16 room_number)
{
    GAME_VECTOR pos;

    pos.x = lara_item->pos.x_pos + (GetRandomDraw() - 0x4000) * (WALL_L / 2) / 0x7fff;
    pos.y = lara_item->floor;
    pos.z = lara_item->pos.z_pos + (GetRandomDraw() - 0x4000) * (WALL_L / 2) / 0x7fff;
    pos.room_number = lara_item->room_number;
    Richochet(&pos);

    return (GunShot(x, y, z, speed, yrot, room_number));
}
