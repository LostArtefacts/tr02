#include "game.h"
#include "box.h"

#define ESCAPE_DIST (WALL_L * 5)

int SearchLOT(LOT_INFO *LOT, int expansion);

void InitialiseCreature(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];

    item->pos.y_rot += (GetRandomControl() - 0x4000) >> 1;
    item->collidable = 1;
    item->data = NULL;
}

typedef struct {
    sint16 zone_number, enemy_zone;
    sint32 distance, ahead, bite;
    sint16 angle, enemy_facing;
} AI_INFO;

extern sint16 *ground_zone[4][2];
extern sint16 *fly_zone[2];

#define ZONE(A) (((A) >> 8) - 1)
#define BLOCKED_SEARCH 0x8000
#define FRONT_ARC 0x4000
#define BANDIT1 48
#define BANDIT2 49
#define MONK1 53
#define MONK2 54

void GetBaddieTarget(sint16 item_num, int goody);

int EnableBaddieAI(sint16 item_number, int always);

int CreatureActive(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    if (item->status == INVISIBLE)
    {
        if (!EnableBaddieAI(item_number, 0))
            return 0;
        item->status = ACTIVE;
    }
    return 1;
}

void CreatureAIInfo(ITEM_INFO *item, AI_INFO *info)
{
    ROOM_INFO *r;
    sint32 x, z;
    sint16 angle, *zone;
    CREATURE_INFO *creature;
    OBJECT_INFO *object;
    ITEM_INFO *enemy;

    if (item->data == NULL)
        return;

    creature = (CREATURE_INFO *)item->data;
    switch (item->object_number)
    {
    case BANDIT1:
    case BANDIT2:
        GetBaddieTarget(creature->item_num, 0);
        break;

    case MONK1:
    case MONK2:
        GetBaddieTarget(creature->item_num, 1);
        break;

    default:
        creature->enemy = lara_item;
    }

    enemy = creature->enemy;
    if (!enemy)
        enemy = lara_item;

    if (creature->LOT.fly == NO_FLYING)
        zone = ground_zone[ZONE(creature->LOT.step)][flip_status];
    else
        zone = fly_zone[flip_status];

    r = &room[item->room_number];
    item->box_number = r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size].box;
    info->zone_number = zone[item->box_number];

    r = &room[enemy->room_number];
    enemy->box_number = r->floor[((enemy->pos.z_pos - r->z) >> WALL_SHIFT) + ((enemy->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size].box;
    info->enemy_zone = zone[enemy->box_number];

    if (boxes[enemy->box_number].overlap_index & creature->LOT.block_mask)
        info->enemy_zone |= BLOCKED;
    else if (creature->LOT.node[item->box_number].search_number == (creature->LOT.search_number | BLOCKED_SEARCH))
        info->enemy_zone |= BLOCKED;

    object = &objects[item->object_number];
    z = enemy->pos.z_pos - (item->pos.z_pos + (object->pivot_length * phd_cos(item->pos.y_rot) >> W2V_SHIFT));
    x = enemy->pos.x_pos - (item->pos.x_pos + (object->pivot_length * phd_sin(item->pos.y_rot) >> W2V_SHIFT));
    angle = phd_atan(z, x);
    if (creature->enemy)
        info->distance = z * z + x * x;
    else
        info->distance = 0x7fffffff;
    info->angle = angle - item->pos.y_rot;
    info->enemy_facing = 0x8000 + angle - enemy->pos.y_rot;
    info->ahead = (info->angle > -FRONT_ARC && info->angle < FRONT_ARC);
    info->bite = (info->ahead && enemy->hit_points > 0 && ABS(enemy->pos.y_pos - item->pos.y_pos) <= STEP_L * 3 / 2);
}

int UpdateLOT(LOT_INFO *LOT, int expansion)
{
    BOX_NODE *expand;

    if (LOT->required_box != NO_BOX && LOT->required_box != LOT->target_box)
    {
        LOT->target_box = LOT->required_box;

        expand = &LOT->node[LOT->target_box];
        if (expand->next_expansion == NO_BOX && LOT->tail != LOT->target_box)
        {
            expand->next_expansion = LOT->head;

            if (LOT->head == NO_BOX)
                LOT->tail = LOT->target_box;

            LOT->head = LOT->target_box;
        }

        expand->search_number = ++LOT->search_number;
        expand->exit_box = NO_BOX;
    }

    return SearchLOT(LOT, expansion);
}

void TargetBox(LOT_INFO *LOT, sint16 box_number)
{
    BOX_INFO *box;

    box_number &= BOX_NUMBER;

    box = &boxes[box_number];

    LOT->target.z = (((uint32)box->left << WALL_SHIFT)) + GetRandomControl() * (((uint32)box->right - (uint32)box->left - 1) >> (15 - WALL_SHIFT)) + WALL_L / 2;
    LOT->target.x = ((uint32)box->top << WALL_SHIFT) + GetRandomControl() * (((uint32)box->bottom - (uint32)box->top - 1) >> (15 - WALL_SHIFT)) + WALL_L / 2;
    LOT->required_box = box_number;

    if (LOT->fly == NO_FLYING)
        LOT->target.y = box->height;
    else
        LOT->target.y = box->height - STEP_L * 3 / 2;
}

int EscapeBox(ITEM_INFO *item, ITEM_INFO *enemy, sint16 box_number)
{
    BOX_INFO *box;
    sint32 x, z;

    box = &boxes[box_number];
    z = (((uint32)box->left + (uint32)box->right) << (WALL_SHIFT - 1)) - enemy->pos.z_pos;
    x = (((uint32)box->top + (uint32)box->bottom) << (WALL_SHIFT - 1)) - enemy->pos.x_pos;

    if (x > -ESCAPE_DIST && x < ESCAPE_DIST && z > -ESCAPE_DIST && z < ESCAPE_DIST)
        return 0;

    if (((z > 0) ^ (item->pos.z_pos > enemy->pos.z_pos)) && ((x > 0) ^ (item->pos.x_pos > enemy->pos.x_pos)))
        return 0;

    return 1;
}

int ValidBox(ITEM_INFO *item, sint16 zone_number, sint16 box_number)
{
    BOX_INFO *box;
    CREATURE_INFO *creature;
    sint16 *zone;

    creature = (CREATURE_INFO *)item->data;

    if (creature->LOT.fly == NO_FLYING)
        zone = ground_zone[ZONE(creature->LOT.step)][flip_status];
    else
        zone = fly_zone[flip_status];

    if (zone[box_number] != zone_number)
        return 0;

    box = &boxes[box_number];
    if (box->overlap_index & creature->LOT.block_mask)
        return 0;

    if (item->pos.z_pos > ((sint32)box->left << WALL_SHIFT) && item->pos.z_pos < ((sint32)box->right << WALL_SHIFT) &&
        item->pos.x_pos > ((sint32)box->top << WALL_SHIFT) && item->pos.x_pos < ((sint32)box->bottom << WALL_SHIFT))
        return 0;

    return 1;
}

#define SQUARE(A) ((A) * (A))

int CreatureCreature(sint16 item_number)
{
    sint16 link;
    sint32 x, y, z, distance, radius;
    ROOM_INFO *r;
    ITEM_INFO *item;

    item = &items[item_number];
    x = item->pos.x_pos;
    y = item->pos.y_pos;
    z = item->pos.z_pos;
    radius = SQUARE(objects[item->object_number].radius);
    r = &room[item->room_number];
    link = r->item_number;
    do
    {
        item = &items[link];
        if (link == item_number)
            return 0;
        if (item != lara_item && item->status == ACTIVE && item->speed != 0)
        {
            distance = SQUARE(item->pos.x_pos - x) + SQUARE(item->pos.y_pos - y) + SQUARE(item->pos.z_pos - z);
            if (distance < radius)
                return 1;
        }
        link = item->next_item;
    }
    while (link != NO_ITEM);
    return 0;
}

int BadFloor(sint32 x, sint32 y, sint32 z, sint32 box_height, sint32 next_height, sint16 room_number, LOT_INFO *LOT)
{
    sint32 height;
    FLOOR_INFO *floor;

    floor = GetFloor(x, y, z, &room_number);
    if (floor->box == NO_BOX)
        return (1);

    if (boxes[floor->box].overlap_index & LOT->block_mask)
        return (1);

    height = boxes[floor->box].height;
    if (box_height - height > LOT->step || box_height - height < LOT->drop)
        return (1);

    if (box_height - height < -LOT->step && height > next_height)
        return (1);

    if ((LOT->fly != NO_FLYING) && y > height + LOT->fly)
        return (1);

    return (0);
}


#define DONT_TARGET (-16384)

extern sint16 body_bag;

int ExplodingDeath(sint16 item_number, sint32 mesh_bits, sint16 damage);
void DisableBaddieAI(sint16 item_number);

void CreatureDie(sint16 item_number, int explode)
{
    sint16 pickup_number;
    ITEM_INFO *item, *pickup;

    item = &items[item_number];
    item->collidable = 0;
    item->hit_points = DONT_TARGET;
    if (explode)
    {
        ExplodingDeath(item_number, 0xffffffff, 0);
        KillItem(item_number);
    }
    else
        RemoveActiveItem(item_number);

    DisableBaddieAI(item_number);
    item->flags |= ONESHOT;
    if (item->clear_body)
    {
        item->next_active = body_bag;
        body_bag = item_number;
    }

    pickup_number = item->carried_item;
    while (pickup_number != NO_ITEM)
    {
        pickup = &items[pickup_number];
        pickup->pos.x_pos = item->pos.x_pos;
        pickup->pos.y_pos = item->pos.y_pos;
        pickup->pos.z_pos = item->pos.z_pos;
        ItemNewRoom(pickup_number, item->room_number);
        pickup_number = pickup->carried_item;
    }
}

sint16 CreatureTurn(ITEM_INFO *item, sint16 maximum_turn)
{
    sint32 x, z, range;
    sint16 angle;
    CREATURE_INFO *creature;

    if (item->data == NULL || item->speed == 0 || maximum_turn == 0)
        return 0;

    creature = (CREATURE_INFO *)item->data;

    z = creature->target.z - item->pos.z_pos;
    x = creature->target.x - item->pos.x_pos;
    angle = phd_atan(z, x) - item->pos.y_rot;
    range = (item->speed << 14) / maximum_turn;

    if ((angle > FRONT_ARC || angle < -FRONT_ARC) && x * x + z * z < SQUARE(range))
        maximum_turn >>= 1;

    if (angle > maximum_turn)
        angle = maximum_turn;
    else if (angle < -maximum_turn)
        angle = -maximum_turn;

    item->pos.y_rot += angle;

    return angle;
}

void CreatureTilt(ITEM_INFO *item, sint16 angle)
{
    angle = (angle << 2) - item->pos.z_rot;
    if (angle < -MAX_TILT)
        angle = -MAX_TILT;
    else if (angle > MAX_TILT)
        angle = MAX_TILT;

    item->pos.z_rot += angle;
}

#define MAX_HEAD_CHANGE (ONE_DEGREE * 5)
#define HEAD_ARC 0x3000

void CreatureHead(ITEM_INFO *item, sint16 required)
{
    sint16 change;
    CREATURE_INFO *creature;

    if (item->data == NULL)
        return;

    creature = (CREATURE_INFO *)item->data;

    change = required - creature->head_rotation;
    if (change > MAX_HEAD_CHANGE)
        change = MAX_HEAD_CHANGE;
    else if (change < -MAX_HEAD_CHANGE)
        change = -MAX_HEAD_CHANGE;

    creature->head_rotation += change;

    if (creature->head_rotation > HEAD_ARC)
        creature->head_rotation = HEAD_ARC;
    else if (creature->head_rotation < -HEAD_ARC)
        creature->head_rotation = -HEAD_ARC;
}

void CreatureNeck(ITEM_INFO *item, sint16 required)
{
    sint16 change;
    CREATURE_INFO *creature;

    if (item->data == NULL)
        return;

    creature = (CREATURE_INFO *)item->data;

    change = required - creature->neck_rotation;
    if (change > MAX_HEAD_CHANGE)
        change = MAX_HEAD_CHANGE;
    else if (change < -MAX_HEAD_CHANGE)
        change = -MAX_HEAD_CHANGE;

    creature->neck_rotation += change;

    if (creature->neck_rotation > HEAD_ARC)
        creature->neck_rotation = HEAD_ARC;
    else if (creature->neck_rotation < -HEAD_ARC)
        creature->neck_rotation = -HEAD_ARC;
}

#define CREATURE_FLOAT_SPEED 32

void CreatureFloat(sint16 item_number)
{
    sint32 water_level;
    ITEM_INFO *item;
    FLOOR_INFO *floor;
    sint16 room_number;

    item = &items[item_number];
    item->hit_points = DONT_TARGET;
    item->pos.x_rot = 0;

    water_level = GetWaterHeight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);
    if (item->pos.y_pos > water_level)
        item->pos.y_pos -= CREATURE_FLOAT_SPEED;
    if (item->pos.y_pos < water_level)
        item->pos.y_pos = water_level;

    AnimateItem(item);

    room_number = item->room_number;
    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    item->floor = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    if (room_number != item->room_number)
        ItemNewRoom(item_number, room_number);
}

void CreatureUnderwater(ITEM_INFO *item, sint32 depth)
{
    sint32 water_level;

    water_level = GetWaterHeight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);
    if (item->pos.y_pos < water_level + depth)
    {
        item->pos.y_pos = water_level + depth;
        if (item->pos.x_rot > 2 * ONE_DEGREE)
            item->pos.x_rot -= 2 * ONE_DEGREE;
        else if (item->pos.x_rot > 0)
            item->pos.x_rot = 0;
    }
}

sint16 CreatureEffect(ITEM_INFO *item, BITE_INFO *bite, sint16 (*generate)(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 yrot, sint16 room_number))
{
    PHD_VECTOR pos;

    pos.x = bite->x;
    pos.y = bite->y;
    pos.z = bite->z;
    GetJointAbsPosition(item, &pos, bite->mesh_num);
    return ((*generate)(pos.x, pos.y, pos.z, item->speed, item->pos.y_rot, item->room_number));
}

int CreatureAnimation(sint16 item_number, sint16 angle, sint16 tilt);

int CreatureVault(sint16 item_number, sint16 angle, int vault, int shift)
{
    ITEM_INFO *item;
    sint32 y, xx, yy, x_floor, y_floor;
    sint16 room_number;

    item = &items[item_number];
    xx = item->pos.z_pos >> WALL_SHIFT;
    yy = item->pos.x_pos >> WALL_SHIFT;
    y = item->pos.y_pos;
    room_number = item->room_number;

    CreatureAnimation(item_number, angle, 0);

    if (item->floor > y + STEP_L * 7 / 2)
        vault = -4;
    else if (item->pos.y_pos > y - STEP_L * 3 / 2)
        return 0;
    else if (item->pos.y_pos > y - STEP_L * 5 / 2)
        vault = 2;
    else if (item->pos.y_pos > y - STEP_L * 7 / 2)
        vault = 3;
    else
        vault = 4;

    x_floor = item->pos.z_pos >> WALL_SHIFT;
    y_floor = item->pos.x_pos >> WALL_SHIFT;
    if (xx == x_floor)
    {
        if (yy == y_floor)
            return 0;
        if (yy < y_floor)
        {
            item->pos.x_pos = (y_floor << WALL_SHIFT) - shift;
            item->pos.y_rot = 0x4000;
        }
        else
        {
            item->pos.x_pos = (yy << WALL_SHIFT) + shift;
            item->pos.y_rot = -0x4000;
        }
    }
    else if (yy == y_floor)
    {
        if (xx < x_floor)
        {
            item->pos.z_pos = (x_floor << WALL_SHIFT) - shift;
            item->pos.y_rot = 0;
        }
        else
        {
            item->pos.z_pos = (xx << WALL_SHIFT) + shift;
            item->pos.y_rot = -0x8000;
        }
    }

    item->pos.y_pos = item->floor = y;
    if (room_number != item->room_number)
        ItemNewRoom(item_number, room_number);
    return vault;
}

#define LARA_EXTRA 12
#define EXTRA_BREATH 0

void CreatureKill(ITEM_INFO *item, int kill_anim, int kill_state, int lara_kill_state)
{
    item->anim_number = objects[item->object_number].anim_index + kill_anim;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = kill_state;

    lara_item->anim_number = objects[LARA_EXTRA].anim_index;
    lara_item->frame_number = anims[lara_item->anim_number].frame_base;
    lara_item->current_anim_state = EXTRA_BREATH;
    lara_item->goal_anim_state = lara_kill_state;
    lara_item->pos.x_pos = item->pos.x_pos;
    lara_item->pos.y_pos = item->pos.y_pos;
    lara_item->pos.z_pos = item->pos.z_pos;
    lara_item->pos.y_rot = item->pos.y_rot;
    lara_item->pos.x_rot = item->pos.x_rot;
    lara_item->pos.z_rot = item->pos.z_rot;
    lara_item->fallspeed = 0;
    lara_item->gravity_status = 0;
    lara_item->speed = 0;
    if (item->room_number != lara_item->room_number)
        ItemNewRoom(lara.item_number, item->room_number);
    AnimateItem(lara_item);
    lara_item->current_anim_state = lara_item->goal_anim_state = lara_kill_state;

    lara.extra_anim = 1;
    lara.hit_direction = -1;
    lara.air = -1;
    lara.gun_status = LG_HANDSBUSY;
    lara.gun_type = LG_UNARMED;
    camera.pos.room_number = lara_item->room_number;
}
