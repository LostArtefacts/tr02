#include "game.h"

enum switch_states { SS_OFF, SS_ON };

int MoveLaraPosition(PHD_VECTOR *vec, ITEM_INFO *item, ITEM_INFO *laraitem);
void AnimateLara(ITEM_INFO *item);
extern sint16 Switch2Bounds[];
extern PHD_VECTOR Switch2Position;

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
