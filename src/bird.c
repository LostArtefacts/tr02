#include "game.h"
#include "box.h"

void InitialiseCreature(sint16 item_number);

#define CROW 38
#define EAGLE 47

#define BIRD_STOP 2
#define BIRD_EAT 7
#define BIRD_START_ANIM 5
#define CROW_START_ANIM 14

void InitialiseEagle(sint16 item_number)
{
    ITEM_INFO *item;

    InitialiseCreature(item_number);

    item = &items[item_number];
    if (item->object_number == CROW) {
        item->anim_number = objects[CROW].anim_index + CROW_START_ANIM;
        item->frame_number = anims[item->anim_number].frame_base;
        item->current_anim_state = item->goal_anim_state = BIRD_EAT;
    } else {
        item->anim_number = objects[EAGLE].anim_index + BIRD_START_ANIM;
        item->frame_number = anims[item->anim_number].frame_base;
        item->current_anim_state = item->goal_anim_state = BIRD_STOP;
    }
}
