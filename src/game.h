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
#include "collide.h"
#include "larafunc.h"
#include "cinema.h"
#include "title.h"
#include "invdata.h"
#include "input.h"
#include "specific/specific.h"
#include "specific/global.h"

extern int level_complete;
extern int flipeffect, fliptimer;

int SoundEffect(int nSFX, PHD_3DPOS *pPos, int flags);
void FlipMap(void);
void InitialiseHair(void);

void finish_level_effect(ITEM_INFO *item);
void turn180_effect(ITEM_INFO *item);
void floor_shake_effect(ITEM_INFO *item);
void lara_normal_effect(ITEM_INFO *item);
void BoilerFX(ITEM_INFO *item);
void FloodFX(ITEM_INFO *item);
void RubbleFX(ITEM_INFO *item);
void ChandelierFX(ITEM_INFO *item);
void ExplosionFX(ITEM_INFO *item);
void PistonFX(ITEM_INFO *item);
void CurtainFX(ITEM_INFO *item);
void StatueFX(ITEM_INFO *item);
void SetChangeFX(ITEM_INFO *item);
void ControlDingDong(sint16 item_number);
void lara_hands_free(ITEM_INFO *item);
void flip_map_effect(ITEM_INFO *item);
void invisibility_on(ITEM_INFO *item);
void invisibility_off(ITEM_INFO *item);
void dynamic_light_on(ITEM_INFO *item);
void dynamic_light_off(ITEM_INFO *item);
void reset_hair(ITEM_INFO *item);
void DoInventoryPicture(void);

#endif
