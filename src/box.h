#ifndef BOX_H
#define BOX_H

#include "items.h"

#define BOX_NUMBER 0x7fff
#define NO_BOX -1
#define BLOCKED 0x4000
#define NO_FLYING 0
#define MAX_TILT (ONE_DEGREE * 3)

typedef struct box_info {
    uint8 left, right, top, bottom;
    sint16 height;
    sint16 overlap_index;
} BOX_INFO;

typedef struct box_node {
    sint16 exit_box;
    uint16 search_number;
    sint16 next_expansion;
    sint16 box_number;
} BOX_NODE;

typedef struct lot_info {
    BOX_NODE *node;
    sint16 head, tail;
    uint16 search_number, block_mask;
    sint16 step, drop, fly, zone_count;
    sint16 target_box, required_box;
    PHD_VECTOR target;
} LOT_INFO;

typedef struct {
    sint32 x;
    sint32 y;
    sint32 z;
    sint32 mesh_num;
} BITE_INFO;

extern BOX_INFO *boxes;
extern int number_boxes;

void TargetBox(LOT_INFO *LOT, sint16 box_number);
void CreatureTilt(ITEM_INFO *item, sint16 angle);
sint16 CreatureEffect(ITEM_INFO *item, BITE_INFO *bite, sint16 (*generate)(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 yrot, sint16 room_number));
void GetJointAbsPosition(ITEM_INFO *item, PHD_VECTOR *vec, int joint);

#endif
