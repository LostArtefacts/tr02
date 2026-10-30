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
