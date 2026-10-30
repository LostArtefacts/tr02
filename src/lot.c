#include "game.h"
#include "box.h"
#include "specific/init.h"

#define NUM_SLOTS 5

extern CREATURE_INFO *baddie_slots;
extern int slots_used;

void InitialiseLOTarray(void)
{
    int i;
    CREATURE_INFO *creature;

    baddie_slots = (CREATURE_INFO *)game_malloc(NUM_SLOTS * sizeof(CREATURE_INFO), CREATURE_DATA);
    creature = baddie_slots;

    for (i = 0; i < NUM_SLOTS; i++, creature++)
    {
        creature->item_num = NO_ITEM;
        creature->LOT.node = (BOX_NODE *)game_malloc(sizeof(BOX_NODE) * number_boxes, CREATURE_LOT);
    }
    slots_used = 0;
}

void DisableBaddieAI(sint16 item_num)
{
    ITEM_INFO *item;
    CREATURE_INFO *creature;

    if (item_num == lara.item_number)
    {
        creature = lara.creature;
        lara.creature = NULL;
    }
    else
    {
        item = &items[item_num];
        creature = (CREATURE_INFO *)item->data;
        item->data = NULL;
    }

    if (creature)
    {
        creature->item_num = NO_ITEM;
        slots_used--;
    }
}

void InitialiseSlot(sint16 item_number, int slot);

int EnableBaddieAI(sint16 item_number, int Always)
{
    int x, y, z;
    int worstslot, slot;
    int worstdist, dist;
    ITEM_INFO *item;
    CREATURE_INFO *creature;

    if (lara.item_number == item_number) {
        if (lara.creature)
            return 1;
    } else if (items[item_number].data)
        return 1;

    if (slots_used < NUM_SLOTS) {
        creature = baddie_slots;
        for (slot = 0; slot < NUM_SLOTS; slot++, creature++) {
            if (creature->item_num == NO_ITEM) {
                InitialiseSlot(item_number, slot);
                return 1;
            }
        }
    }

    if (!Always) {
        item = &items[item_number];
        x = (item->pos.x_pos - camera.pos.x) >> 8;
        y = (item->pos.y_pos - camera.pos.y) >> 8;
        z = (item->pos.z_pos - camera.pos.z) >> 8;
        worstdist = x * x + y * y + z * z;
    } else
        worstdist = 0;

    worstslot = -1;
    creature = baddie_slots;
    for (slot = 0; slot < NUM_SLOTS; slot++, creature++) {
        item = &items[creature->item_num];
        x = (item->pos.x_pos - camera.pos.x) >> 8;
        y = (item->pos.y_pos - camera.pos.y) >> 8;
        z = (item->pos.z_pos - camera.pos.z) >> 8;
        dist = x * x + y * y + z * z;
        if (dist > worstdist) {
            worstdist = dist;
            worstslot = slot;
        }
    }

    if (worstslot >= 0) {
        items[baddie_slots[worstslot].item_num].status = INVISIBLE;
        DisableBaddieAI(baddie_slots[worstslot].item_num);
        InitialiseSlot(item_number, worstslot);
        return 1;
    }
    return 0;
}

extern sint16 *ground_zone[4][2];
extern sint16 *fly_zone[2];

#define ZONE(A) (((A) >> 8) - 1)

#define SHARK 25
#define BARACUDDA 28
#define DIVER 29
#define WORKER3 32
#define WORKER4 33
#define JELLY 35
#define SPIDER 36
#define CROW 38
#define YETI 45
#define EAGLE 47
#define SKIDMAN 51
#define DINOSAUR 214

void ClearLOT(LOT_INFO *LOT);
void CreateZone(ITEM_INFO *item);

void InitialiseSlot(sint16 item_number, int slot)
{
    CREATURE_INFO *creature;
    ITEM_INFO *item;

    creature = &baddie_slots[slot];
    item = &items[item_number];
    if (item_number == lara.item_number)
        lara.creature = creature;
    else
        item->data = (void *)creature;
    creature->item_num = item_number;
    creature->mood = BORED_MOOD;
    creature->head_rotation = creature->neck_rotation = 0;
    creature->maximum_turn = ONE_DEGREE;
    creature->flags = 0;
    creature->enemy = NULL;
    creature->LOT.step = STEP_L;
    creature->LOT.drop = -STEP_L * 2;
    creature->LOT.block_mask = BLOCKED;
    creature->LOT.fly = NO_FLYING;
    switch (item->object_number)
    {
    case LARA:
        creature->LOT.step = WALL_L * 20;
        creature->LOT.drop = -WALL_L * 20;
        creature->LOT.fly = STEP_L;
        break;

    case SHARK:
    case BARACUDDA:
    case DIVER:
    case JELLY:
    case CROW:
    case EAGLE:
        creature->LOT.step = WALL_L * 20;
        creature->LOT.drop = -WALL_L * 20;
        creature->LOT.fly = STEP_L / 16;
        if (item->object_number == SHARK)
            creature->LOT.block_mask = BLOCKABLE;
        break;

    case SPIDER:
    case SKIDMAN:
        creature->LOT.step = STEP_L * 2;
        creature->LOT.drop = -WALL_L;
        break;

    case WORKER3:
    case WORKER4:
    case YETI:
        creature->LOT.step = WALL_L;
        creature->LOT.drop = -WALL_L;
        break;

    case DINOSAUR:
        creature->LOT.block_mask = BLOCKABLE;
        break;
    }
    ClearLOT(&creature->LOT);
    if (item_number != lara.item_number)
        CreateZone(item);
    slots_used++;
}

void CreateZone(ITEM_INFO *item)
{
    int i;
    CREATURE_INFO *creature;
    BOX_NODE *node;
    sint16 *zone, zone_number;
    sint16 *flip, flip_number;
    ROOM_INFO *r;

    creature = (CREATURE_INFO *)item->data;

    if (creature->LOT.fly == NO_FLYING)
    {
        zone = ground_zone[ZONE(creature->LOT.step)][0];
        flip = ground_zone[ZONE(creature->LOT.step)][1];
    }
    else
    {
        zone = fly_zone[0];
        flip = fly_zone[1];
    }

    r = &room[item->room_number];
    item->box_number = r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size].box;

    zone_number = zone[item->box_number];
    flip_number = flip[item->box_number];

    for (i = 0, creature->LOT.zone_count = 0, node = creature->LOT.node; i < number_boxes; i++, zone++, flip++)
    {
        if (*zone == zone_number || *flip == flip_number)
        {
            node->box_number = i;
            node++;
            creature->LOT.zone_count++;
        }
    }
}

void ClearLOT(LOT_INFO *LOT)
{
    int i;
    BOX_NODE *node;

    LOT->head = LOT->tail = NO_BOX;
    LOT->search_number = 0;
    LOT->target_box = NO_BOX;
    LOT->required_box = NO_BOX;

    for (i = 0, node = LOT->node; i < number_boxes; i++, node++)
    {
        node->exit_box = node->next_expansion = NO_BOX;
        node->search_number = 0;
    }
}
