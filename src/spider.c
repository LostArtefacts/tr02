#include "game.h"
#include "box.h"

int CreatureAnimation(sint16 item_number, sint16 angle, sint16 tilt);

#define SPIDER 36
#define SPIDER_EMPTY 0
#define SPIDER_STOP 1
#define SPIDER_WALK1 2
#define SPIDER_WALK2 3
#define SPIDER_ATTACK1 4
#define SPIDER_ATTACK2 5
#define SPIDER_ATTACK3 6
#define SPIDER_DEATH 7
#define SPIDER_LEAP_ANIM 2

void SpiderLeap(sint16 item_number, sint16 angle)
{
    ITEM_INFO *item;
    sint32 x, y, z;
    sint16 room_number;

    item = &items[item_number];
    x = item->pos.x_pos;
    y = item->pos.y_pos;
    z = item->pos.z_pos;
    room_number = item->room_number;

    CreatureAnimation(item_number, angle, 0);

    if (item->pos.y_pos > y - STEP_L * 3 / 2)
        return;

    item->pos.x_pos = x;
    item->pos.y_pos = y;
    item->pos.z_pos = z;
    if (item->room_number != room_number)
        ItemNewRoom(item_number, room_number);

    item->anim_number = objects[SPIDER].anim_index + SPIDER_LEAP_ANIM;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = SPIDER_ATTACK2;

    CreatureAnimation(item_number, angle, 0);
}
