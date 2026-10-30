#include "game.h"
#include "box.h"

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
