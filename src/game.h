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
#include "effects.h"
#include "title.h"
#include "invdata.h"
#include "input.h"
#include "objnums.h"
#include "gameflow.h"
#include "laraflar.h"
#include "lara1gun.h"
#include "sound.h"
#include "sphere.h"
#include "specific/specific.h"
#include "specific/global.h"

extern int level_complete;
extern sint16 *trigger_index;
extern int flipeffect, fliptimer;
extern int number_draw_rooms;
extern sint16 draw_rooms[];
extern int flip_status;
extern void (*effect_routines[])(ITEM_INFO *item);
extern int assault_timer_display, assault_timer_active;

void FlipMap(void);
FLOOR_INFO *GetFloor(sint32 x, sint32 y, sint32 z, sint16 *room_number);
sint32 GetHeight(FLOOR_INFO *floor, sint32 x, sint32 y, sint32 z);
void ItemNewRoom(sint16 item_number, sint16 room_number);
void InitialiseHair(void);
void DrawHair(void);
void DrawAnimatingItem(ITEM_INFO *item);
void DrawUnclippedItem(ITEM_INFO *item);
void LiftFloorCeiling(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *floor, sint32 *ceiling);
void LiftCeiling(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height);
sint32 DoDemoSequence(int level_number);
sint16 *GetBoundsAccurate(ITEM_INFO *item);
sint32 GetWaterHeight(sint32 x, sint32 y, sint32 z, sint16 room_number);
void TestTriggers(sint16 *data, int heavy);
void AnimateItem(ITEM_INFO *item);
int TriggerActive(ITEM_INFO *item);

void SoundEffects(void);
sint16 DoBloodSplat(sint32 x, sint32 y, sint32 z, sint16 speed, PHD_ANGLE direction, sint16 room_num);
void DoLotsOfBlood(sint32 x, sint32 y, sint32 z, sint16 speed, PHD_ANGLE direction, sint16 room_num, int number);
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
void draw_right_gun(ITEM_INFO *item);
void draw_left_gun(ITEM_INFO *item);
void invisibility_on(ITEM_INFO *item);
void invisibility_off(ITEM_INFO *item);
void dynamic_light_on(ITEM_INFO *item);
void dynamic_light_off(ITEM_INFO *item);
void reset_hair(ITEM_INFO *item);
void DoInventoryPicture(void);
void AssaultStop(ITEM_INFO *item);

#endif
