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

extern sint16 *ground_zone[4][2];
extern sint16 *fly_zone[2];

#define ZONE(A) (((A) >> 8) - 1)

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
