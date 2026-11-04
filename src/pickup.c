#include "game.h"
#include <windows.h>
#include "savegame.h"

enum switch_states { SS_OFF, SS_ON };

int MoveLaraPosition(PHD_VECTOR *vec, ITEM_INFO *item, ITEM_INFO *laraitem);
void AnimateLara(ITEM_INFO *item);
extern sint16 Switch2Bounds[];
extern PHD_VECTOR Switch2Position;

#define FLARE_ITEM 152
#define SECRET1 190
#define SECRET2 191
#define SECRET3 192

extern sint16 PickUpBounds[];
extern PHD_VECTOR PickUpPosition;
extern sint16 PickUpBoundsUW[];
extern PHD_VECTOR PickUpPositionUW;
extern uint32 CurrentLevel;
void AddDisplayPickup(sint16 objnum);
int Inv_AddItem(int itemNum);
void GF_ModifyInventory(int level, int type);
void RemoveDrawnItem(sint16 item_num);
void InitialiseNewWeapon(void);
void draw_flare_meshes(void);

void PickUpCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll)
{
    ITEM_INFO *item;
    sint16 x_rot, y_rot, z_rot;

    item = &items[item_num];
    y_rot = item->pos.y_rot;
    z_rot = item->pos.z_rot;
    x_rot = item->pos.x_rot;
    item->pos.y_rot = laraitem->pos.y_rot;
    item->pos.z_rot = 0;

    if (lara.water_status == LARA_ABOVEWATER || lara.water_status == LARA_WADE)
    {
        item->pos.x_rot = 0;
        if (!TestLaraPosition(PickUpBounds, item, laraitem))
            goto restore;
        if (laraitem->current_anim_state == AS_PICKUP)
        {
            if (laraitem->frame_number == anims[135].frame_base + 42 && item->object_number != FLARE_ITEM)
            {
                AddDisplayPickup(item->object_number);
                Inv_AddItem(item->object_number);
                if ((item->object_number == SECRET1 || item->object_number == SECRET2 || item->object_number == SECRET3)
                    && (savegame.secrets & 1) + ((savegame.secrets >> 1) & 1) + ((savegame.secrets >> 2) & 1) >= 3)
                    GF_ModifyInventory(CurrentLevel, 1);
                item->status = INVISIBLE;
                RemoveDrawnItem(item_num);
            }
            return;
        }
        if (laraitem->current_anim_state == AS_FLAREPICKUP)
        {
            if (laraitem->frame_number == anims[204].frame_base + 58 && item->object_number == FLARE_ITEM && lara.gun_type != LG_FLARE)
            {
                lara.request_gun_type = LG_FLARE;
                lara.gun_type = LG_FLARE;
                InitialiseNewWeapon();
                lara.gun_status = LG_SPECIAL;
                lara.flare_age = (sint32)item->data & 0x7fff;
                KillItem(item_num);
            }
            return;
        }
        if ((input & IN_ACTION) && !laraitem->gravity_status && laraitem->current_anim_state == AS_STOP && lara.gun_status == LG_ARMLESS
            && (lara.gun_type != LG_FLARE || item->object_number != FLARE_ITEM))
        {
            if (item->object_number == FLARE_ITEM)
            {
                laraitem->goal_anim_state = AS_FLAREPICKUP;
                do
                    AnimateLara(laraitem);
                while (laraitem->current_anim_state != AS_FLAREPICKUP);
                laraitem->goal_anim_state = AS_STOP;
                lara.gun_status = LG_HANDSBUSY;
            }
            else
            {
                AlignLaraPosition(&PickUpPosition, item, laraitem);
                laraitem->goal_anim_state = AS_PICKUP;
                do
                    AnimateLara(laraitem);
                while (laraitem->current_anim_state != AS_PICKUP);
                laraitem->goal_anim_state = AS_STOP;
                lara.gun_status = LG_HANDSBUSY;
            }
            return;
        }
    }
    else if (lara.water_status == LARA_UNDERWATER)
    {
        item->pos.x_rot = -4550;
        if (!TestLaraPosition(PickUpBoundsUW, item, laraitem))
            goto restore;
        if (laraitem->current_anim_state == AS_PICKUP)
        {
            if (laraitem->frame_number == anims[130].frame_base + 18 && item->object_number != FLARE_ITEM)
            {
                AddDisplayPickup(item->object_number);
                Inv_AddItem(item->object_number);
                if ((item->object_number == SECRET1 || item->object_number == SECRET2 || item->object_number == SECRET3)
                    && (savegame.secrets & 1) + ((savegame.secrets >> 1) & 1) + ((savegame.secrets >> 2) & 1) >= 3)
                    GF_ModifyInventory(CurrentLevel, 1);
                item->status = INVISIBLE;
                RemoveDrawnItem(item_num);
            }
            return;
        }
        if (laraitem->current_anim_state == AS_FLAREPICKUP)
        {
            if (laraitem->frame_number == anims[206].frame_base + 20 && item->object_number == FLARE_ITEM && lara.gun_type != LG_FLARE)
            {
                lara.request_gun_type = LG_FLARE;
                lara.gun_type = LG_FLARE;
                InitialiseNewWeapon();
                lara.gun_status = LG_SPECIAL;
                lara.flare_age = (sint32)item->data & 0x7fff;
                draw_flare_meshes();
                KillItem(item_num);
            }
            return;
        }
        if ((input & IN_ACTION) && laraitem->current_anim_state == AS_TREAD && lara.gun_status == LG_ARMLESS
            && (lara.gun_type != LG_FLARE || item->object_number != FLARE_ITEM))
        {
            if (MoveLaraPosition(&PickUpPositionUW, item, laraitem))
            {
                if (item->object_number == FLARE_ITEM)
                {
                    laraitem->fallspeed = 0;
                    laraitem->current_anim_state = AS_FLAREPICKUP;
                    laraitem->anim_number = 206;
                    laraitem->goal_anim_state = AS_TREAD;
                    laraitem->frame_number = anims[206].frame_base;
                }
                else
                {
                    laraitem->goal_anim_state = AS_PICKUP;
                    do
                        AnimateLara(laraitem);
                    while (laraitem->current_anim_state != AS_PICKUP);
                    laraitem->goal_anim_state = AS_TREAD;
                }
            }
            return;
        }
    }
restore:
    item->pos.x_rot = x_rot;
    item->pos.y_rot = y_rot;
    item->pos.z_rot = z_rot;
}

#define SMALL_SWITCH 93
#define PUSH_SWITCH 103
#define AIRLOCK_SWITCH 92
#define LARA_EXTRA 12
#define EXTRA_BREATH 0
#define EXTRA_AIRLOCK 4

extern sint16 Switch1Bounds[];
extern PHD_VECTOR SmallSwitchPosition;
extern PHD_VECTOR PushSwitchPosition;
extern PHD_VECTOR AirlockPosition;

void SwitchCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll)
{
    ITEM_INFO *item;

    item = &items[item_num];
    if (!(input & IN_ACTION) || item->status != NOT_ACTIVE || lara.gun_status != LG_ARMLESS || laraitem->gravity_status)
        return;

    if (laraitem->current_anim_state == AS_STOP)
    {
        if (!TestLaraPosition(Switch1Bounds, item, laraitem))
            return;
        laraitem->pos.y_rot = item->pos.y_rot;

        if (item->object_number == SMALL_SWITCH)
            AlignLaraPosition(&SmallSwitchPosition, item, laraitem);
        else if (item->object_number == PUSH_SWITCH)
            AlignLaraPosition(&PushSwitchPosition, item, laraitem);
        else if (item->object_number == AIRLOCK_SWITCH)
        {
            if (item->current_anim_state == SS_ON)
                return;
            AlignLaraPosition(&AirlockPosition, item, laraitem);
        }

        if (item->current_anim_state == SS_ON)
        {
            if (item->object_number == SMALL_SWITCH)
                laraitem->anim_number = objects[LARA].anim_index + 195;
            else if (item->object_number == PUSH_SWITCH)
                laraitem->anim_number = objects[LARA].anim_index + 197;
            else
                laraitem->anim_number = objects[LARA].anim_index + 63;
            laraitem->current_anim_state = AS_SWITCHON;
            item->goal_anim_state = SS_OFF;
        }
        else
        {
            laraitem->current_anim_state = AS_SWITCHOFF;
            if (item->object_number == SMALL_SWITCH)
                laraitem->anim_number = objects[LARA].anim_index + 196;
            else if (item->object_number == PUSH_SWITCH)
                laraitem->anim_number = objects[LARA].anim_index + 197;
            else if (item->object_number == AIRLOCK_SWITCH)
            {
                laraitem->anim_number = objects[LARA_EXTRA].anim_index;
                laraitem->frame_number = anims[laraitem->anim_number].frame_base;
                laraitem->current_anim_state = EXTRA_BREATH;
                laraitem->goal_anim_state = EXTRA_AIRLOCK;
                AnimateItem(laraitem);
                lara.extra_anim = 1;
            }
            else
                laraitem->anim_number = objects[LARA].anim_index + 64;
            item->goal_anim_state = SS_ON;
        }

        if (!lara.extra_anim)
        {
            laraitem->frame_number = anims[laraitem->anim_number].frame_base;
            laraitem->goal_anim_state = AS_STOP;
        }
        lara.gun_status = LG_HANDSBUSY;

        item->status = ACTIVE;
        AddActiveItem(item_num);
        AnimateItem(item);
    }
}

void SwitchCollision2(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll)
{
    ITEM_INFO *item;

    item = &items[item_num];
    if (!(input & IN_ACTION) || item->status != NOT_ACTIVE || lara.water_status != LARA_UNDERWATER || lara.gun_status != LG_ARMLESS)
        return;

    if (laraitem->current_anim_state == AS_TREAD)
    {
        if (!TestLaraPosition(Switch2Bounds, item, laraitem))
            return;

        if (item->current_anim_state == SS_ON || item->current_anim_state == SS_OFF)
        {
            if (MoveLaraPosition(&Switch2Position, item, laraitem))
            {
                laraitem->fallspeed = 0;
                laraitem->goal_anim_state = AS_SWITCHON;
                do
                {
                    AnimateLara(laraitem);
                } while (laraitem->current_anim_state != AS_SWITCHON);
                laraitem->goal_anim_state = AS_TREAD;
                lara.gun_status = LG_HANDSBUSY;
                item->status = ACTIVE;
                if (item->current_anim_state == SS_ON)
                    item->goal_anim_state = SS_OFF;
                else
                    item->goal_anim_state = SS_ON;
                AddActiveItem(item_num);
                AnimateItem(item);
            }
        }
    }
}

#define DETONATOR2 227
#define GONG_BONGER 225
#define KEY_OPTION2 198
#define EXTRA_PLUNGER 1
#define EXTRA_GONGBONG 5

extern sint16 InventoryChosen;
extern sint16 PickUpBounds[];
extern sint16 GongBounds[];
extern PHD_VECTOR DetonatorPosition;
int Display_Inventory(int inv_mode);
int Inv_RemoveItem(int itemNum);
sint16 CreateItem(void);
void InitialiseItem(sint16 item_num);

void ObjectCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);

void DetonatorCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll)
{
    ITEM_INFO *item, *gong;
    sint16 x_rot, y_rot, z_rot;
    sint16 gong_num;

    if (lara.extra_anim)
        return;

    item = &items[item_num];

    x_rot = item->pos.x_rot;
    y_rot = item->pos.y_rot;
    z_rot = item->pos.z_rot;

    item->pos.y_rot = laraitem->pos.y_rot;
    item->pos.z_rot = 0;
    item->pos.x_rot = 0;

    if (item->status == DEACTIVATED || !(input & IN_ACTION) || lara.gun_status != LG_ARMLESS || laraitem->gravity_status || laraitem->current_anim_state != AS_STOP)
        goto normal_collision;

    if (item->object_number == DETONATOR2)
    {
        if (!TestLaraPosition(PickUpBounds, item, laraitem))
            goto normal_collision;
    }
    else
    {
        if (!TestLaraPosition(GongBounds, item, laraitem))
            goto normal_collision;
        item->pos.x_rot = x_rot;
        item->pos.y_rot = y_rot;
        item->pos.z_rot = z_rot;
    }

    if (InventoryChosen == -1)
        Display_Inventory(INV_KEYS_MODE);

    if (InventoryChosen != KEY_OPTION2)
        goto normal_collision;

    Inv_RemoveItem(KEY_OPTION2);

    AlignLaraPosition(&DetonatorPosition, item, laraitem);

    laraitem->anim_number = objects[LARA_EXTRA].anim_index;
    laraitem->frame_number = anims[laraitem->anim_number].frame_base;
    laraitem->current_anim_state = EXTRA_BREATH;
    if (item->object_number == DETONATOR2)
        laraitem->goal_anim_state = EXTRA_PLUNGER;
    else
    {
        laraitem->pos.y_rot += -32768;
        laraitem->goal_anim_state = EXTRA_GONGBONG;
    }

    AnimateItem(laraitem);

    lara.extra_anim = 1;

    lara.gun_status = LG_HANDSBUSY;

    if (item->object_number == DETONATOR2)
    {
        item->status = ACTIVE;
        AddActiveItem(item_num);
    }
    else
    {
        gong_num = CreateItem();
        if (gong_num != NO_ITEM)
        {
            gong = &items[gong_num];
            gong->object_number = GONG_BONGER;
            gong->pos.x_pos = laraitem->pos.x_pos;
            gong->pos.y_pos = laraitem->pos.y_pos;
            gong->pos.z_pos = laraitem->pos.z_pos;
            gong->pos.y_rot = laraitem->pos.y_rot;
            laraitem->pos.z_rot = 0;
            gong->pos.x_rot = 0;
            gong->room_number = laraitem->room_number;
            InitialiseItem(gong_num);
            AddActiveItem(gong_num);
            gong->status = ACTIVE;
        }
    }
    return;

normal_collision:
    item->pos.x_rot = x_rot;
    item->pos.y_rot = y_rot;
    item->pos.z_rot = z_rot;
    ObjectCollision(item_num, laraitem, coll);
}

#define KEY_OPTION1 197
#define KEY_OPTION2 198
#define KEY_OPTION3 199
#define KEY_OPTION4 200
#define KEY_HOLE1 201
#define KEY_HOLE2 202
#define KEY_HOLE3 203
#define KEY_HOLE4 204

extern sint16 InventoryChosen;
extern sint16 inv_keys_objects;
extern sint32 pup_x, pup_y, pup_z;
extern sint16 KeyHoleBounds[];
extern PHD_VECTOR KeyHolePosition;
int Display_Inventory(int inv_mode);
int Inv_RemoveItem(int itemNum);

void KeyHoleCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll)
{
    ITEM_INFO *item;
    int correct = 0;

    if (laraitem->current_anim_state == AS_STOP)
    {
        item = &items[item_num];
        if ((InventoryChosen == -1 && !(input & IN_ACTION)) || lara.gun_status != LG_ARMLESS || laraitem->gravity_status)
            return;

        if (!TestLaraPosition(KeyHoleBounds, item, laraitem))
            return;
        if (item->status != NOT_ACTIVE)
        {
            if (laraitem->pos.x_pos != pup_x || laraitem->pos.y_pos != pup_y || laraitem->pos.z_pos != pup_z)
            {
                pup_x = laraitem->pos.x_pos;
                pup_y = laraitem->pos.y_pos;
                pup_z = laraitem->pos.z_pos;
                SoundEffect(2, &laraitem->pos, 0);
            }
            return;
        }

        if (InventoryChosen == -1)
        {
            Display_Inventory(INV_KEYS_MODE);
            if (InventoryChosen == -1 && inv_keys_objects)
                return;
            if (InventoryChosen != -1)
                pup_y = laraitem->pos.y_pos - 1;
        }
        else
            pup_y = laraitem->pos.y_pos - 1;

        switch (item->object_number)
        {
        case KEY_HOLE1:
            if (InventoryChosen != KEY_OPTION1)
                break;
            Inv_RemoveItem(KEY_OPTION1);
            correct = 1;
            break;

        case KEY_HOLE2:
            if (InventoryChosen != KEY_OPTION2)
                break;
            Inv_RemoveItem(KEY_OPTION2);
            correct = 1;
            break;

        case KEY_HOLE3:
            if (InventoryChosen != KEY_OPTION3)
                break;
            Inv_RemoveItem(KEY_OPTION3);
            correct = 1;
            break;

        case KEY_HOLE4:
            if (InventoryChosen != KEY_OPTION4)
                break;
            Inv_RemoveItem(KEY_OPTION4);
            correct = 1;
            break;
        }

        InventoryChosen = -1;
        if (!correct)
        {
            if (laraitem->pos.x_pos != pup_x || laraitem->pos.y_pos != pup_y || laraitem->pos.z_pos != pup_z)
            {
                SoundEffect(2, &laraitem->pos, 0);
                pup_x = laraitem->pos.x_pos;
                pup_y = laraitem->pos.y_pos;
                pup_z = laraitem->pos.z_pos;
            }
            return;
        }

        AlignLaraPosition(&KeyHolePosition, item, laraitem);
        laraitem->goal_anim_state = AS_USEKEY;
        do
            AnimateLara(laraitem);
        while (laraitem->current_anim_state != AS_USEKEY);

        laraitem->goal_anim_state = AS_STOP;
        lara.gun_status = LG_HANDSBUSY;
        item->status = ACTIVE;

        pup_x = laraitem->pos.x_pos;
        pup_y = laraitem->pos.y_pos;
        pup_z = laraitem->pos.z_pos;
    }
}

#define PUZZLE_OPTION1 178
#define PUZZLE_OPTION2 179
#define PUZZLE_OPTION3 180
#define PUZZLE_OPTION4 181
#define PUZZLE_HOLE1 182
#define PUZZLE_HOLE2 183
#define PUZZLE_HOLE3 184
#define PUZZLE_HOLE4 185
#define PUZZLE_DONE1 186
#define PUZZLE_DONE2 187
#define PUZZLE_DONE3 188
#define PUZZLE_DONE4 189
#define USEPUZZLE_F GF(134, 80)

extern sint16 PuzzleHoleBounds[];
extern PHD_VECTOR PuzzleHolePosition;

void PuzzleHoleCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll)
{
    ITEM_INFO *item;
    int correct = 0;

    item = &items[item_num];
    if (laraitem->current_anim_state == AS_STOP)
    {
        if ((InventoryChosen == -1 && !(input & IN_ACTION)) || lara.gun_status != LG_ARMLESS || laraitem->gravity_status)
            return;

        if (!TestLaraPosition(PuzzleHoleBounds, item, laraitem))
            return;
        if (item->status != NOT_ACTIVE)
        {
            if (laraitem->pos.x_pos != pup_x || laraitem->pos.y_pos != pup_y || laraitem->pos.z_pos != pup_z)
            {
                pup_x = laraitem->pos.x_pos;
                pup_y = laraitem->pos.y_pos;
                pup_z = laraitem->pos.z_pos;
                SoundEffect(2, &laraitem->pos, 0);
            }
            return;
        }
        if (InventoryChosen == -1)
        {
            Display_Inventory(INV_KEYS_MODE);
            if (InventoryChosen == -1 && inv_keys_objects)
                return;
            if (InventoryChosen != -1)
                pup_y = laraitem->pos.y_pos - 1;
        }
        else
            pup_y = laraitem->pos.y_pos - 1;

        switch (item->object_number)
        {
        case PUZZLE_HOLE1:
            if (InventoryChosen != PUZZLE_OPTION1)
                break;
            Inv_RemoveItem(PUZZLE_OPTION1);
            correct = 1;
            break;

        case PUZZLE_HOLE2:
            if (InventoryChosen != PUZZLE_OPTION2)
                break;
            Inv_RemoveItem(PUZZLE_OPTION2);
            correct = 1;
            break;

        case PUZZLE_HOLE3:
            if (InventoryChosen != PUZZLE_OPTION3)
                break;
            Inv_RemoveItem(PUZZLE_OPTION3);
            correct = 1;
            break;

        case PUZZLE_HOLE4:
            if (InventoryChosen != PUZZLE_OPTION4)
                break;
            Inv_RemoveItem(PUZZLE_OPTION4);
            correct = 1;
            break;
        }

        InventoryChosen = -1;
        if (!correct)
        {
            if (laraitem->pos.x_pos != pup_x || laraitem->pos.y_pos != pup_y || laraitem->pos.z_pos != pup_z)
            {
                SoundEffect(2, &laraitem->pos, 0);
                pup_x = laraitem->pos.x_pos;
                pup_y = laraitem->pos.y_pos;
                pup_z = laraitem->pos.z_pos;
            }
            return;
        }
        AlignLaraPosition(&PuzzleHolePosition, item, laraitem);
        laraitem->goal_anim_state = AS_USEPUZZLE;
        do
        {
            AnimateLara(laraitem);
        } while (laraitem->current_anim_state != AS_USEPUZZLE);
        laraitem->goal_anim_state = AS_STOP;
        lara.gun_status = LG_HANDSBUSY;
        item->status = ACTIVE;
        pup_x = laraitem->pos.x_pos;
        pup_y = laraitem->pos.y_pos;
        pup_z = laraitem->pos.z_pos;
        return;
    }
    else if (laraitem->current_anim_state == AS_USEPUZZLE)
    {
        if (!TestLaraPosition(PuzzleHoleBounds, item, laraitem))
            return;
        if (laraitem->frame_number == USEPUZZLE_F)
        {
            switch (item->object_number)
            {
            case PUZZLE_HOLE1:
                item->object_number = PUZZLE_DONE1;
                break;

            case PUZZLE_HOLE2:
                item->object_number = PUZZLE_DONE2;
                break;

            case PUZZLE_HOLE3:
                item->object_number = PUZZLE_DONE3;
                break;

            case PUZZLE_HOLE4:
                item->object_number = PUZZLE_DONE4;
                break;
            }
        }
    }
}

void SwitchControl(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];

    item->flags |= CODE_BITS;

    if (!TriggerActive(item))
    {
        item->goal_anim_state = SS_ON;
        item->timer = 0;
    }

    AnimateItem(item);
}

int SwitchTrigger(sint16 item_num, sint16 timer)
{
    ITEM_INFO *item;

    item = &items[item_num];
    if (item->object_number == AIRLOCK_SWITCH)
    {
        if (item->status == DEACTIVATED)
        {
            RemoveActiveItem(item_num);
            item->status = NOT_ACTIVE;
            return 0;
        }
        else if ((item->flags & ONESHOT) || item->current_anim_state == SS_OFF)
            return 0;
        item->flags |= ONESHOT;
        return 1;
    }

    if (item->status == DEACTIVATED)
    {
        if (item->current_anim_state == SS_OFF && timer > 0)
        {
            item->timer = timer;
            if (timer != 1)
                item->timer *= 30;
            item->status = ACTIVE;
        }
        else
        {
            RemoveActiveItem(item_num);
            item->status = NOT_ACTIVE;
        }
        return 1;
    }
    return 0;
}

int KeyTrigger(sint16 item_num)
{
    ITEM_INFO *item;

    item = &items[item_num];

    if (item->status == ACTIVE && lara.gun_status != LG_HANDSBUSY)
    {
        item->status = DEACTIVATED;
        return 1;
    }
    return 0;
}

int PickupTrigger(sint16 item_num)
{
    ITEM_INFO *item;

    item = &items[item_num];
    if (item->status == INVISIBLE)
    {
        item->status = DEACTIVATED;
        return 1;
    }
    return 0;
}

void RemoveDrawnItem(sint16 item_num);

void SecretControl(sint16 item_number)
{
    items[item_number].status = INVISIBLE;
    RemoveDrawnItem(item_number);
}

#ifdef TR2_EXE
sint16 PickUpBounds[12] = {
    -256, 256, -100, 100, -256, 256, -1820, 1820, 0, 0, 0, 0,
};
PHD_VECTOR PickUpPosition = { 0, 0, -100 };
sint16 GongBounds[12] = {
    -512, 1024, -100, 100, -812, -412, -5460, 5460, 0, 0, 0, 0,
};
sint16 PickUpBoundsUW[12] = {
    -512, 512, -512, 512, -512, 512, -8190, 8190, -8190, 8190, -8190, 8190,
};
PHD_VECTOR PickUpPositionUW = { 0, -200, -350 };
sint16 Switch1Bounds[12] = {
    -220, 220, 0, 0, 292, 512, -1820, 1820, -5460, 5460, -1820, 1820,
};
PHD_VECTOR SmallSwitchPosition = { 0, 0, 362 };
PHD_VECTOR PushSwitchPosition = { 0, 0, 292 };
PHD_VECTOR AirlockPosition = { 0, 0, 212 };
sint16 Switch2Bounds[12] = {
    -1024, 1024, -1024, 1024, -1024, 512, -14560, 14560, -14560, 14560, -14560, 14560,
};
PHD_VECTOR Switch2Position = { 0, 0, 108 };
sint16 KeyHoleBounds[12] = {
    -200, 200, 0, 0, 312, 512, -1820, 1820, -5460, 5460, -1820, 1820,
};
PHD_VECTOR KeyHolePosition = { 0, 0, 362 };
sint16 PuzzleHoleBounds[12] = {
    -200, 200, 0, 0, 312, 512, -1820, 1820, -5460, 5460, -1820, 1820,
};
PHD_VECTOR PuzzleHolePosition = { 0, 0, 327 };
sint32 pup_x;
sint32 pup_y;
sint32 pup_z;
PHD_VECTOR DetonatorPosition;
#endif
