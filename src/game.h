#ifndef GAME_H
#define GAME_H

#include <stddef.h>

#include "types.h"
#include "items.h"
#include "lara.h"
#include "anim.h"
#include "camera.h"
#include "room.h"
#include "laraanim.h"

extern int level_complete;
extern int flipeffect, fliptimer;

int SoundEffect(int nSFX, PHD_3DPOS *pPos, int flags);

void finish_level_effect(ITEM_INFO *item);
void turn180_effect(ITEM_INFO *item);
void floor_shake_effect(ITEM_INFO *item);
void lara_normal_effect(ITEM_INFO *item);
void BoilerFX(ITEM_INFO *item);
void FloodFX(ITEM_INFO *item);
void RubbleFX(ITEM_INFO *item);
void ChandelierFX(ITEM_INFO *item);
void lara_hands_free(ITEM_INFO *item);

#endif
