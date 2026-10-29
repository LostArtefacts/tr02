#ifndef GAME_H
#define GAME_H

#include "types.h"
#include "items.h"
#include "lara.h"
#include "anim.h"
#include "camera.h"
#include "room.h"

extern int level_complete;

void finish_level_effect(ITEM_INFO *item);
void turn180_effect(ITEM_INFO *item);
void floor_shake_effect(ITEM_INFO *item);
void lara_hands_free(ITEM_INFO *item);

#endif
