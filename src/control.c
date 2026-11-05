#include <windows.h>
#include <stdio.h>
#include "game.h"
#include "box.h"
#include "savegame.h"
#include "inventry.h"

typedef struct {
    sint16 goal_anim_state;
    sint16 number_ranges;
    sint16 range_index;
} CHANGE_STRUCT;

typedef struct {
    sint16 start_frame;
    sint16 end_frame;
    sint16 link_anim_num;
    sint16 link_frame_num;
} RANGE_STRUCT;

extern CHANGE_STRUCT *changes;
extern RANGE_STRUCT *ranges;

#define MAX_FRAMES 10
#define DEATH_WAIT (10 * 30)

static int framecount = 0;

extern int IsResetFlag;
extern int NoInputCounter;
extern int OverlayStatus;
extern int InventoryExtraData[];
extern int HealthBarTimer;
extern sint16 cdtrack;
extern sint32 number_dynamics;
extern sint16 next_item_active;
void S_CDLoop(void);
void CheckCheatMode(void);
int S_UpdateInput(void);
void GetDemoInput(void);
int Display_Inventory(int inv_mode);
void CreateSaveGameInfo(void);
int S_SaveGame(void *data, int size, int slot);
void S_SaveSettings(void);
void LaraControl(sint16 item_num);
void HairControl(int in_cutscene);
void CalculateCamera(void);

sint32 ControlPhase(sint32 nframes, int demo_mode)
{
    sint32 return_val;
    sint16 item_num, nex;

    if (nframes > MAX_FRAMES)
        nframes = MAX_FRAMES;

    framecount += nframes;
    for (; framecount > 0; framecount -= 2)
    {
        if (cdtrack > 0)
            S_CDLoop();

        if (!gameflow.cheatmodecheck_disabled)
            CheckCheatMode();

        if (level_complete)
            return 1;

        S_UpdateInput();

        if (IsResetFlag)
            return EXIT_TO_TITLE;

        if (demo_mode)
        {
            if (input)
                return gameflow.on_demo_interrupt;

            GetDemoInput();
            if (input == -1)
            {
                input = 0;
                return gameflow.on_demo_end;
            }
        }
        else if (gameflow.noinput_timeout)
        {
            if (input)
                NoInputCounter = 0;
            else
            {
                NoInputCounter++;
                if (NoInputCounter > gameflow.noinput_time)
                    return STARTDEMO;
            }
        }

        if (lara.death_count > DEATH_WAIT || (lara.death_count > 2 * 30 && input) || OverlayStatus == 2)
        {
            if (demo_mode)
                return gameflow.ondeath_demo_mode;

            if (CurrentLevel == LV_GYM)
                return EXIT_TO_TITLE;

            if (OverlayStatus == 2)
            {
                OverlayStatus = 1;
                return_val = Display_Inventory(INV_DEATH_MODE);
                if (return_val)
                    return return_val;
            }
            else
                OverlayStatus = 2;
        }

        if ((input & IN_OPTION || input & IN_LOAD || input & IN_SAVE || OverlayStatus <= 0) && !lara.death_count && !lara.extra_anim)
        {
            if (OverlayStatus <= 0)
            {
                if (OverlayStatus == -1)
                    return_val = Display_Inventory(INV_LOAD_MODE);
                else if (OverlayStatus == -2)
                    return_val = Display_Inventory(INV_SAVE_MODE);
                else
                    return_val = Display_Inventory(INV_GAME_MODE);

                OverlayStatus = 1;
                if (return_val)
                {
                    if (InventoryExtraData[0] == 1)
                    {
                        if (CurrentLevel == LV_GYM)
                            return STARTGAME | LV_FIRSTLEVEL;

                        CreateSaveGameInfo();
                        S_SaveGame(&savegame, sizeof(SAVEGAME_INFO), InventoryExtraData[1]);
                        S_SaveSettings();
                    }
                    else
                        return return_val;
                }
            }
            else
            {
                if (!gameflow.loadsave_disabled)
                {
                    if (input & IN_LOAD)
                        OverlayStatus = -1;
                    else if (input & IN_SAVE)
                        OverlayStatus = -2;
                    else
                        OverlayStatus = 0;
                }
                else
                    OverlayStatus = 0;
            }
        }

        number_dynamics = 0;

        item_num = next_item_active;
        while (item_num != NO_ITEM)
        {
            nex = items[item_num].next_active;
            if (objects[items[item_num].object_number].control)
                (*objects[items[item_num].object_number].control)(item_num);
            item_num = nex;
        }

        item_num = next_fx_active;
        while (item_num != NO_ITEM)
        {
            nex = effects[item_num].next_active;
            if (objects[effects[item_num].object_number].control)
                (*objects[effects[item_num].object_number].control)(item_num);
            item_num = nex;
        }

        LaraControl(0);
        HairControl(0);
        CalculateCamera();
        SoundEffects();

        HealthBarTimer--;

        if (CurrentLevel != LV_GYM || assault_timer_active)
            savegame.timer++;
    }
    return 0;
}

#define COMMAND_MOVE_ORIGIN 1
#define COMMAND_JUMP_VELOCITY 2
#define COMMAND_ATTACK_READY 3
#define COMMAND_DEACTIVATE 4
#define COMMAND_SOUND_FX 5
#define COMMAND_EFFECT 6
#define SFX_LANDANDWATER 0
#define SFX_LANDONLY 0x4000
#define SFX_WATERONLY 0x8000
#define UNDERWATER 1
#define GRAVITY 6
#define NO_ROOM 0xff

extern sint16 *commands;
int GetChange(ITEM_INFO *item, ANIM_STRUCT *anim);
void TranslateItem(ITEM_INFO *item, sint32 x, sint32 y, sint32 z);

void AnimateItem(ITEM_INFO *item)
{
    int i;
    ANIM_STRUCT *anim;
    sint16 *command;
    sint32 speed;
    uint16 num;
    uint16 type;

    item->touch_bits = 0;
    item->hit_status = 0;

    anim = &anims[item->anim_number];

    item->frame_number++;

    if (anim->number_changes > 0)
    {
        if (GetChange(item, anim))
        {
            anim = &anims[item->anim_number];
            item->current_anim_state = anim->current_anim_state;
            if (item->required_anim_state == item->current_anim_state)
                item->required_anim_state = 0;
        }
    }

    if (item->frame_number > anim->frame_end)
    {
        if (anim->number_commands > 0)
        {
            command = &commands[anim->command_index];
            for (i = anim->number_commands; i > 0; i--)
            {
                switch (*(command++))
                {
                case COMMAND_MOVE_ORIGIN:
                    TranslateItem(item, (sint32)*(command), (sint32)*(command + 1), (sint32)*(command + 2));
                    command += 3;
                    break;

                case COMMAND_JUMP_VELOCITY:
                    item->fallspeed = *(command++);
                    item->speed = *(command++);
                    item->gravity_status = 1;
                    break;

                case COMMAND_ATTACK_READY:
                    break;

                case COMMAND_DEACTIVATE:
                    item->status = DEACTIVATED;
                    break;

                case COMMAND_SOUND_FX:
                case COMMAND_EFFECT:
                    command += 2;
                    break;
                }
            }
        }

        item->anim_number = anim->jump_anim_num;
        item->frame_number = anim->jump_frame_num;

        anim = &anims[item->anim_number];
        if (item->current_anim_state != anim->current_anim_state)
        {
            item->current_anim_state = anim->current_anim_state;
            item->goal_anim_state = item->current_anim_state;
        }

        if (item->required_anim_state == item->current_anim_state)
            item->required_anim_state = 0;
    }

    if (anim->number_commands > 0)
    {
        command = &commands[anim->command_index];
        for (i = anim->number_commands; i > 0; i--)
        {
            switch (*(command++))
            {
            case COMMAND_MOVE_ORIGIN:
                command += 3;
                break;

            case COMMAND_JUMP_VELOCITY:
                command += 2;
                break;

            case COMMAND_ATTACK_READY:
                break;

            case COMMAND_DEACTIVATE:
                break;

            case COMMAND_SOUND_FX:
                if (item->frame_number == *(command))
                {
                    type = *(command + 1) & 0xc000;
                    num = *(command + 1) & 0x3fff;
                    if (objects[item->object_number].water_creature)
                        SoundEffect((int)num, &item->pos, UNDERWATER);
                    else if (item->room_number == NO_ROOM)
                    {
                        item->pos.x_pos = lara_item->pos.x_pos;
                        item->pos.y_pos = lara_item->pos.y_pos - LARA_HITE;
                        item->pos.z_pos = lara_item->pos.z_pos;
                        if (item->object_number == HARPOON_GUN)
                            SoundEffect((int)num, &item->pos, SFX_ALWAYS);
                        else
                            SoundEffect((int)num, &item->pos, 0);
                    }
                    else if (room[item->room_number].flags & UNDERWATER)
                    {
                        if (type == SFX_LANDANDWATER || type == SFX_WATERONLY)
                            SoundEffect((int)num, &item->pos, 0);
                    }
                    else if (type == SFX_LANDANDWATER || type == SFX_LANDONLY)
                        SoundEffect((int)num, &item->pos, 0);
                }
                command += 2;
                break;

            case COMMAND_EFFECT:
                if (item->frame_number == *(command))
                {
                    num = *(command + 1) & 0x3fff;
                    (*effect_routines[(int)num])(item);
                }
                command += 2;
                break;
            }
        }
    }

    if (!item->gravity_status)
    {
        speed = anim->velocity;
        if (anim->acceleration != 0)
            speed += anim->acceleration * (item->frame_number - anim->frame_base);
        item->speed = (sint16)(speed >> 16);
    }
    else
    {
        item->fallspeed += (item->fallspeed < FASTFALL_SPEED) ? GRAVITY : 1;
        item->pos.y_pos += item->fallspeed;
    }

    item->pos.x_pos += (phd_sin(item->pos.y_rot) * item->speed) >> W2V_SHIFT;
    item->pos.z_pos += (phd_cos(item->pos.y_rot) * item->speed) >> W2V_SHIFT;
}

int GetChange(ITEM_INFO *item, ANIM_STRUCT *anim)
{
    int i, j;
    CHANGE_STRUCT *change;
    RANGE_STRUCT *range;

    if (item->current_anim_state == item->goal_anim_state)
        return 0;

    change = &changes[anim->change_index];
    for (i = 0; i < anim->number_changes; i++, change++)
    {
        if (change->goal_anim_state == item->goal_anim_state)
        {
            range = &ranges[change->range_index];
            for (j = 0; j < change->number_ranges; j++, range++)
            {
                if (item->frame_number >= range->start_frame && item->frame_number <= range->end_frame)
                {
                    item->anim_number = range->link_anim_num;
                    item->frame_number = range->link_frame_num;
                    return 1;
                }
            }
        }
    }
    return 0;
}


sint16 GetDoor(FLOOR_INFO *floor);
extern sint16 *floor_data;

void TranslateItem(ITEM_INFO *item, sint32 x, sint32 y, sint32 z)
{
    sint32 c;
    sint32 s;

    c = phd_cos(item->pos.y_rot);
    s = phd_sin(item->pos.y_rot);
    item->pos.x_pos += (c * x + s * z) >> W2V_SHIFT;
    item->pos.y_pos += y;
    item->pos.z_pos += (c * z - s * x) >> W2V_SHIFT;
}

FLOOR_INFO *GetFloor(sint32 x, sint32 y, sint32 z, sint16 *room_number)
{
    sint32 x_floor, y_floor;
    ROOM_INFO *r;
    FLOOR_INFO *floor;
    sint16 data;

    r = &room[*room_number];
    do
    {
        x_floor = (z - r->z) >> WALL_SHIFT;
        y_floor = (x - r->x) >> WALL_SHIFT;

        if (x_floor <= 0)
        {
            x_floor = 0;
            if (y_floor < 1)
                y_floor = 1;
            else if (y_floor > r->y_size - 2)
                y_floor = r->y_size - 2;
        }
        else if (x_floor >= r->x_size - 1)
        {
            x_floor = r->x_size - 1;
            if (y_floor < 1)
                y_floor = 1;
            else if (y_floor > r->y_size - 2)
                y_floor = r->y_size - 2;
        }
        else if (y_floor < 0)
            y_floor = 0;
        else if (y_floor >= r->y_size)
            y_floor = r->y_size - 1;

        floor = &r->floor[x_floor + y_floor * r->x_size];
        data = GetDoor(floor);
        if (data != NO_ROOM)
        {
            *room_number = data;
            r = &room[data];
        }
    } while (data != NO_ROOM);

    if (y >= ((sint32)floor->floor << 8))
    {
        do
        {
            if (floor->pit_room == NO_ROOM)
                return floor;
            *room_number = floor->pit_room;
            r = &room[floor->pit_room];
            floor = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];
        } while (y >= ((sint32)floor->floor << 8));
    }
    else if (y < ((sint32)floor->ceiling << 8))
    {
        do
        {
            if (floor->sky_room == NO_ROOM)
                return floor;
            *room_number = floor->sky_room;
            r = &room[floor->sky_room];
            floor = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];
        } while (y < ((sint32)floor->ceiling << 8));
    }
    return floor;
}

#define REVERSE 0x4000

#define DATA_TYPE 0x00ff
#define DOOR_TYPE 1
#define TILT_TYPE 2
#define ROOF_TYPE 3

#define TRIGGER_TYPE 4
#define LAVA_TYPE 5
#define CLIMB_TYPE 6
#define END_BIT 0x8000
#define VALUE_BITS 0x03ff
#define TRIG_BITS(T) ((T & 0x3fff) >> 10)
#define TO_OBJECT 0
#define TO_CAMERA 1
#define WALL 0
#define SMALL_SLOPE 1
#define BIG_SLOPE 2

extern int height_type;
extern int IsChunkyCamera;
extern sint16 GF_NoFloor;
void S_ExitSystem(char *message);

sint32 GetWaterHeight(sint32 x, sint32 y, sint32 z, sint16 room_number)
{
    sint32 x_floor, y_floor;
    ROOM_INFO *r;
    FLOOR_INFO *floor;
    sint16 data;

    r = &room[room_number];
    do
    {
        x_floor = (z - r->z) >> WALL_SHIFT;
        y_floor = (x - r->x) >> WALL_SHIFT;

        if (x_floor <= 0)
        {
            x_floor = 0;
            if (y_floor < 1)
                y_floor = 1;
            else if (y_floor > r->y_size - 2)
                y_floor = r->y_size - 2;
        }
        else if (x_floor >= r->x_size - 1)
        {
            x_floor = r->x_size - 1;
            if (y_floor < 1)
                y_floor = 1;
            else if (y_floor > r->y_size - 2)
                y_floor = r->y_size - 2;
        }
        else if (y_floor < 0)
            y_floor = 0;
        else if (y_floor >= r->y_size)
            y_floor = r->y_size - 1;

        floor = &r->floor[x_floor + y_floor * r->x_size];
        data = GetDoor(floor);
        if (data != NO_ROOM)
        {
            room_number = data;
            r = &room[data];
        }
    } while (data != NO_ROOM);

    if (r->flags & UNDERWATER)
    {
        while (floor->sky_room != NO_ROOM)
        {
            r = &room[floor->sky_room];
            if (!(r->flags & UNDERWATER))
                break;
            floor = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];
        }
        return floor->ceiling << 8;
    }
    else
    {
        while (floor->pit_room != NO_ROOM)
        {
            r = &room[floor->pit_room];
            if (r->flags & UNDERWATER)
                return floor->floor << 8;
            floor = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];
        }
    }
    return NO_HEIGHT;
}

sint32 GetHeight(FLOOR_INFO *floor, sint32 x, sint32 y, sint32 z)
{
    sint32 height, xoff, yoff;
    ROOM_INFO *r;
    ITEM_INFO *item;
    sint16 *data, type, trigger;

    height_type = WALL;
    while (floor->pit_room != NO_ROOM)
    {
        r = &room[floor->pit_room];
        floor = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];
    }

    height = floor->floor << 8;
    if (GF_NoFloor && GF_NoFloor == height)
        height = 0x4000;

    trigger_index = NULL;
    if (!floor->index)
        return height;

    data = &floor_data[floor->index];
    do
    {
        type = *(data++);
        switch (type & DATA_TYPE)
        {
        case TILT_TYPE:
            xoff = *data >> 8;
            yoff = (sint8)*data;
            if (!IsChunkyCamera || (ABS(xoff) <= 2 && ABS(yoff) <= 2))
            {
                if (ABS(xoff) > 2 || ABS(yoff) > 2)
                    height_type = BIG_SLOPE;
                else
                    height_type = SMALL_SLOPE;

                if (xoff < 0)
                    height -= (z & (WALL_L - 1)) * xoff >> 2;
                else
                    height += ((WALL_L - 1 - z) & (WALL_L - 1)) * xoff >> 2;

                if (yoff < 0)
                    height -= (x & (WALL_L - 1)) * yoff >> 2;
                else
                    height += ((WALL_L - 1 - x) & (WALL_L - 1)) * yoff >> 2;
            }
            data++;
            break;

        case DOOR_TYPE:
        case ROOF_TYPE:
            data++;
            break;

        case LAVA_TYPE:
            trigger_index = data - 1;
            break;

        case CLIMB_TYPE:
            if (!trigger_index)
                trigger_index = data - 1;
            break;

        case TRIGGER_TYPE:
            if (!trigger_index)
                trigger_index = data - 1;
            data++;
            do
            {
                trigger = *(data++);
                if (TRIG_BITS(trigger) != TO_OBJECT)
                {
                    if (TRIG_BITS(trigger) == TO_CAMERA)
                        trigger = *(data++);
                    continue;
                }
                item = &items[trigger & VALUE_BITS];
                if (objects[item->object_number].floor)
                    (*objects[item->object_number].floor)(item, x, y, z, &height);
            } while (!(trigger & END_BIT));
            break;

        default:
            S_ExitSystem("GetHeight(): Unknown type");
            break;
        }
    } while (!(type & END_BIT));

    return height;
}

#define TO_TARGET 6
#define NO_CAMERA -1

void RefreshCamera(sint16 type, sint16 *data)
{
    sint16 trigger, value, target_ok;

    target_ok = 2;
    do
    {
        trigger = *(data++);
        value = trigger & VALUE_BITS;
        switch (TRIG_BITS(trigger))
        {
        case TO_CAMERA:
            data++;
            if (value == camera.last)
            {
                camera.number = value;
                if (camera.timer < 0 || camera.type == LOOK_CAMERA || camera.type == COMBAT_CAMERA)
                {
                    camera.timer = -1;
                    target_ok = 0;
                    break;
                }
                camera.type = FIXED_CAMERA;
                target_ok = 1;
            }
            else
                target_ok = 0;
            break;

        case TO_TARGET:
            if (camera.type != LOOK_CAMERA && camera.type != COMBAT_CAMERA)
                camera.item = &items[value];
            break;
        }
    } while (!(trigger & END_BIT));

    if (camera.item && (!target_ok || (target_ok == 2 && camera.item->looked_at && camera.item != camera.last_item)))
        camera.item = NULL;

    if (camera.number == NO_CAMERA && camera.timer > 0)
        camera.timer = -1;
}

#define TRIGGER 0
#define PAD 1
#define SWITCH 2
#define KEY 3
#define PICKUP 4
#define HEAVY 5
#define ANTIPAD 6
#define COMBAT 7
#define DUMMY 8
#define ANTITRIGGER 9
#define TO_SINK 2
#define TO_FLIPMAP 3
#define TO_FLIPON 4
#define TO_FLIPOFF 5
#define TO_FINISH 7
#define TO_CD 8
#define TO_FLIPEFFECT 9
#define TO_SECRET 10
#define TO_BODYBAG 11

extern int flipmap[];
int SwitchTrigger(sint16 item_num, sint16 timer);
int KeyTrigger(sint16 item_num);
int PickupTrigger(sint16 item_num);
void LavaBurn(ITEM_INFO *item);
int EnableBaddieAI(sint16 item_number, int always);
void ClearBodyBag(void);
void TriggerCDTrack(sint16 value, sint16 flags, sint16 type);

void TestTriggers(sint16 *data, int heavy)
{
    ITEM_INFO *item, *camera_item;
    OBJECT_VECTOR *obvector;
    sint16 type, trigger, value, timer, flags;
    sint16 camera_flags, camera_timer;
    int switch_off = 0, flip = 0, flip_available = 0, neweffect = -1;
    LOT_INFO *LOT;
    int quad;

    if (!heavy)
        lara.climb_status = 0;

    if (data == NULL)
        return;

    if ((*data & DATA_TYPE) == LAVA_TYPE)
    {
        if (!heavy && (lara_item->pos.y_pos == lara_item->floor || lara.water_status != LARA_ABOVEWATER))
            LavaBurn(lara_item);
        if (*data & END_BIT)
            return;
        data++;
    }

    if ((*data & DATA_TYPE) == CLIMB_TYPE)
    {
        if (!heavy)
        {
            quad = (uint16)(lara_item->pos.y_rot + 0x2000) / 0x4000;
            if ((1 << (quad + 8)) & *data)
                lara.climb_status = 1;
        }
        if (*data & END_BIT)
            return;
        data++;
    }

    type = (*(data++) >> 8) & 0x3f;
    flags = *(data++);
    timer = flags & 0xff;

    if (camera.type != HEAVY_CAMERA)
        RefreshCamera(type, data);

    if (heavy)
    {
        if (type != HEAVY)
            return;
    }
    else
    {
        switch (type)
        {
        case SWITCH:
            value = *(data++) & VALUE_BITS;
            if (!SwitchTrigger(value, timer))
                return;
            switch_off = (items[value].current_anim_state == 1);
            break;

        case PAD:
        case ANTIPAD:
            if (lara_item->pos.y_pos != lara_item->floor)
                return;
            break;

        case KEY:
            value = *(data++) & VALUE_BITS;
            if (!KeyTrigger(value))
                return;
            break;

        case PICKUP:
            value = *(data++) & VALUE_BITS;
            if (!PickupTrigger(value))
                return;
            break;

        case HEAVY:
            return;

        case COMBAT:
            if (lara.gun_status != LG_READY)
                return;
            break;

        case DUMMY:
            return;
        }
    }

    camera_item = NULL;
    do
    {
        trigger = *(data++);
        value = trigger & VALUE_BITS;
        switch (TRIG_BITS(trigger))
        {
        case TO_OBJECT:
            item = &items[value];
            if (item->flags & ONESHOT)
                break;

            item->timer = timer;
            if (timer != 1)
                item->timer *= 30;

            if (type == SWITCH)
                item->flags ^= (flags & CODE_BITS);
            else if (type == ANTIPAD || type == ANTITRIGGER)
            {
                item->flags &= (0xffffffff - (flags & CODE_BITS));
                if (flags & ONESHOT)
                    item->flags |= ONESHOT;
            }
            else if (flags & CODE_BITS)
                item->flags |= (flags & CODE_BITS);

            if ((item->flags & CODE_BITS) != CODE_BITS)
                break;

            if (flags & ONESHOT)
                item->flags |= ONESHOT;

            if (!item->active)
            {
                if (objects[item->object_number].intelligent)
                {
                    if (item->status == NOT_ACTIVE)
                    {
                        item->touch_bits = 0;
                        item->status = ACTIVE;
                        AddActiveItem(value);
                        EnableBaddieAI(value, 1);
                    }
                    else if (item->status == INVISIBLE)
                    {
                        item->touch_bits = 0;
                        if (EnableBaddieAI(value, 0))
                            item->status = ACTIVE;
                        else
                            item->status = INVISIBLE;
                        AddActiveItem(value);
                    }
                }
                else
                {
                    item->touch_bits = 0;
                    AddActiveItem(value);
                    item->status = ACTIVE;
                }
            }
            break;

        case TO_CAMERA:
            trigger = *(data++);
            camera_flags = trigger;
            camera_timer = trigger & 0xff;

            if (camera.fixed[value].flags & ONESHOT)
                break;

            camera.number = value;

            if (camera.type == LOOK_CAMERA || camera.type == COMBAT_CAMERA)
                break;
            if (type == COMBAT)
                break;
            if (type == SWITCH && timer && switch_off)
                break;

            if (camera.number != camera.last || type == SWITCH)
            {
                camera.timer = camera_timer * 30;
                if (camera_flags & ONESHOT)
                    camera.fixed[camera.number].flags |= ONESHOT;
                camera.speed = ((camera_flags & CODE_BITS) >> 6) + 1;
                camera.type = heavy ? HEAVY_CAMERA : FIXED_CAMERA;
            }
            break;

        case TO_TARGET:
            camera_item = &items[value];
            break;

        case TO_SINK:
            obvector = &camera.fixed[value];
            if (!lara.creature)
                EnableBaddieAI(lara.item_number, 1);
            LOT = &lara.creature->LOT;
            LOT->target.x = obvector->x;
            LOT->target.y = obvector->y;
            LOT->target.z = obvector->z;
            LOT->required_box = obvector->flags;
            lara.current_active = obvector->data * 6;
            break;

        case TO_FLIPMAP:
            flip_available = 1;
            if (flipmap[value] & ONESHOT)
                break;

            if (type == SWITCH)
                flipmap[value] ^= (flags & CODE_BITS);
            else if (flags & CODE_BITS)
                flipmap[value] |= (flags & CODE_BITS);

            if ((flipmap[value] & CODE_BITS) == CODE_BITS)
            {
                if (flags & ONESHOT)
                    flipmap[value] |= ONESHOT;
                if (!flip_status)
                    flip = 1;
            }
            else if (flip_status)
                flip = 1;
            break;

        case TO_FLIPON:
            flip_available = 1;
            if ((flipmap[value] & CODE_BITS) == CODE_BITS && !flip_status)
                flip = 1;
            break;

        case TO_FLIPOFF:
            flip_available = 1;
            if ((flipmap[value] & CODE_BITS) == CODE_BITS && flip_status)
                flip = 1;
            break;

        case TO_FLIPEFFECT:
            neweffect = value;
            break;

        case TO_FINISH:
            level_complete = 1;
            break;

        case TO_CD:
            TriggerCDTrack(value, flags, type);
            break;

        case TO_SECRET:
            break;

        case TO_BODYBAG:
            ClearBodyBag();
            break;
        }
    } while (!(trigger & END_BIT));

    if (camera_item && (camera.type == FIXED_CAMERA || camera.type == HEAVY_CAMERA))
        camera.item = camera_item;

    if (flip)
        FlipMap();

    if (neweffect != -1 && (flip || !flip_available))
    {
        flipeffect = neweffect;
        fliptimer = 0;
    }
}

int TriggerActive(ITEM_INFO *item)
{
    int ok;

    ok = (item->flags & REVERSE) ? 0 : 1;

    if ((item->flags & CODE_BITS) != CODE_BITS)
        return !ok;

    if (!item->timer)
        return ok;

    if (item->timer == -1)
        return !ok;

    item->timer--;
    if (!item->timer)
        item->timer = -1;

    return ok;
}

sint32 GetCeiling(FLOOR_INFO *floor, sint32 x, sint32 y, sint32 z)
{
    sint32 height, xoff, yoff;
    ROOM_INFO *r;
    FLOOR_INFO *f;
    ITEM_INFO *item;
    sint16 *data, type, trigger;

    f = floor;
    while (f->sky_room != NO_ROOM)
    {
        r = &room[f->sky_room];
        f = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];
    }

    height = f->ceiling << 8;

    if (f->index)
    {
        data = &floor_data[f->index];
        type = *(data++) & DATA_TYPE;
        if (type == TILT_TYPE)
        {
            data++;
            type = *(data++) & DATA_TYPE;
        }

        if (type == ROOF_TYPE)
        {
            xoff = *data >> 8;
            yoff = (sint8)*data;
            if (!IsChunkyCamera || (xoff >= -2 && xoff <= 2 && yoff >= -2 && yoff <= 2))
            {
                if (xoff < 0)
                    height += (z & (WALL_L - 1)) * xoff >> 2;
                else
                    height -= ((WALL_L - 1 - z) & (WALL_L - 1)) * xoff >> 2;

                if (yoff >= 0)
                    height -= (x & (WALL_L - 1)) * yoff >> 2;
                else
                    height += ((WALL_L - 1 - x) & (WALL_L - 1)) * yoff >> 2;
            }
        }
    }

    while (floor->pit_room != NO_ROOM)
    {
        r = &room[floor->pit_room];
        floor = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];
    }

    if (!floor->index)
        return height;

    data = &floor_data[floor->index];
    do
    {
        type = *(data++);
        switch (type & DATA_TYPE)
        {
        case DOOR_TYPE:
        case TILT_TYPE:
        case ROOF_TYPE:
            data++;
            break;

        case LAVA_TYPE:
        case CLIMB_TYPE:
            break;

        case TRIGGER_TYPE:
            data++;
            do
            {
                trigger = *(data++);
                if (TRIG_BITS(trigger) != TO_OBJECT)
                {
                    if (TRIG_BITS(trigger) == TO_CAMERA)
                        trigger = *(data++);
                }
                else
                {
                    item = &items[trigger & VALUE_BITS];
                    if (objects[item->object_number].ceiling)
                        (*objects[item->object_number].ceiling)(item, x, y, z, &height);
                }
            } while (!(trigger & END_BIT));
            break;

        default:
            S_ExitSystem("GetCeiling(): Unknown type");
            break;
        }
    } while (!(type & END_BIT));

    return height;
}

sint16 GetDoor(FLOOR_INFO *floor)
{
    sint16 *data, type;

    if (!floor->index)
        return NO_ROOM;

    data = &floor_data[floor->index];
    type = *(data++);

    if (type == TILT_TYPE)
    {
        data++;
        type = *(data++);
    }

    if (type == ROOF_TYPE)
    {
        data++;
        type = *(data++);
    }

    if ((type & DATA_TYPE) == DOOR_TYPE)
        return *data;
    return NO_ROOM;
}

int xLOS(GAME_VECTOR *start, GAME_VECTOR *target);
int zLOS(GAME_VECTOR *start, GAME_VECTOR *target);
int ClipTarget(GAME_VECTOR *start, GAME_VECTOR *target, FLOOR_INFO *floor);

int LOS(GAME_VECTOR *start, GAME_VECTOR *target)
{
    int los1, los2;
    FLOOR_INFO *floor;

    if ((ABS(target->z - start->z)) > (ABS(target->x - start->x)))
    {
        los1 = xLOS(start, target);
        los2 = zLOS(start, target);
    }
    else
    {
        los1 = zLOS(start, target);
        los2 = xLOS(start, target);
    }

    if (!los2)
        return 0;

    floor = GetFloor(target->x, target->y, target->z, &target->room_number);

    if (!ClipTarget(start, target, floor) || los1 != 1 || los2 != 1)
        return 0;
    else
        return 1;
}

#define RETURN_TARGET { target->x = x; target->y = y; target->z = z; target->room_number = last_room; return 0; }
#define RETURN_TARGET2 { target->x = x; target->y = y; target->z = z; target->room_number = room_number; return -1; }

extern int los_rooms[];
extern int number_los_rooms;

int zLOS(GAME_VECTOR *start, GAME_VECTOR *target)
{
    sint32 x, y, z;
    sint32 dx, dy, dz;
    sint16 room_number, last_room;
    FLOOR_INFO *floor;

    dz = target->z - start->z;
    if (dz == 0)
        return 1;

    dx = ((target->x - start->x) << WALL_SHIFT) / dz;
    dy = ((target->y - start->y) << WALL_SHIFT) / dz;

    los_rooms[0] = room_number = last_room = start->room_number;
    number_los_rooms = 1;

    if (dz < 0)
    {
        z = start->z & ~(WALL_L - 1);
        x = start->x + (dx * (z - start->z) >> WALL_SHIFT);
        y = start->y + (dy * (z - start->z) >> WALL_SHIFT);
        while (z > target->z)
        {
            floor = GetFloor(x, y, z, &room_number);
            if (y > GetHeight(floor, x, y, z) || y < GetCeiling(floor, x, y, z))
                RETURN_TARGET2;

            if (room_number != last_room)
            {
                los_rooms[number_los_rooms++] = room_number;
                last_room = room_number;
            }

            floor = GetFloor(x, y, z - 1, &room_number);
            if (y > GetHeight(floor, x, y, z - 1) || y < GetCeiling(floor, x, y, z - 1))
                RETURN_TARGET;

            z -= WALL_L;
            x -= dx;
            y -= dy;
        }
    }
    else
    {
        z = start->z | (WALL_L - 1);
        x = start->x + (dx * (z - start->z) >> WALL_SHIFT);
        y = start->y + (dy * (z - start->z) >> WALL_SHIFT);
        while (z < target->z)
        {
            floor = GetFloor(x, y, z, &room_number);
            if (y > GetHeight(floor, x, y, z) || y < GetCeiling(floor, x, y, z))
                RETURN_TARGET2;

            if (room_number != last_room)
            {
                los_rooms[number_los_rooms++] = room_number;
                last_room = room_number;
            }

            floor = GetFloor(x, y, z + 1, &room_number);
            if (y > GetHeight(floor, x, y, z + 1) || y < GetCeiling(floor, x, y, z + 1))
                RETURN_TARGET;

            z += WALL_L;
            x += dx;
            y += dy;
        }
    }

    target->room_number = room_number;
    return 1;
}

int xLOS(GAME_VECTOR *start, GAME_VECTOR *target)
{
    sint32 x, y, z;
    sint32 dx, dy, dz;
    sint16 room_number, last_room;
    FLOOR_INFO *floor;

    dx = target->x - start->x;
    if (dx == 0)
        return 1;

    dy = ((target->y - start->y) << WALL_SHIFT) / dx;
    dz = ((target->z - start->z) << WALL_SHIFT) / dx;

    los_rooms[0] = room_number = last_room = start->room_number;
    number_los_rooms = 1;

    if (dx < 0)
    {
        x = start->x & ~(WALL_L - 1);
        y = start->y + (dy * (x - start->x) >> WALL_SHIFT);
        z = start->z + (dz * (x - start->x) >> WALL_SHIFT);
        while (x > target->x)
        {
            floor = GetFloor(x, y, z, &room_number);
            if (y > GetHeight(floor, x, y, z) || y < GetCeiling(floor, x, y, z))
                RETURN_TARGET2;

            if (room_number != last_room)
            {
                los_rooms[number_los_rooms++] = room_number;
                last_room = room_number;
            }

            floor = GetFloor(x - 1, y, z, &room_number);
            if (y > GetHeight(floor, x - 1, y, z) || y < GetCeiling(floor, x - 1, y, z))
                RETURN_TARGET;

            x -= WALL_L;
            y -= dy;
            z -= dz;
        }
    }
    else
    {
        x = start->x | (WALL_L - 1);
        y = start->y + (dy * (x - start->x) >> WALL_SHIFT);
        z = start->z + (dz * (x - start->x) >> WALL_SHIFT);
        while (x < target->x)
        {
            floor = GetFloor(x, y, z, &room_number);
            if (y > GetHeight(floor, x, y, z) || y < GetCeiling(floor, x, y, z))
                RETURN_TARGET2;

            if (room_number != last_room)
            {
                los_rooms[number_los_rooms++] = room_number;
                last_room = room_number;
            }

            floor = GetFloor(x + 1, y, z, &room_number);
            if (y > GetHeight(floor, x + 1, y, z) || y < GetCeiling(floor, x + 1, y, z))
                RETURN_TARGET;

            x += WALL_L;
            y += dy;
            z += dz;
        }
    }

    target->room_number = room_number;
    return 1;
}

int ClipTarget(GAME_VECTOR *start, GAME_VECTOR *target, FLOOR_INFO *floor)
{
    sint32 dx, dy, dz, height;

    dx = target->x - start->x;
    dy = target->y - start->y;
    dz = target->z - start->z;

    height = GetHeight(floor, target->x, target->y, target->z);
    if (target->y > height && start->y < height)
    {
        target->y = height;
        target->x = start->x + dx * (height - start->y) / dy;
        target->z = start->z + dz * (height - start->y) / dy;
        return 0;
    }

    height = GetCeiling(floor, target->x, target->y, target->z);
    if (target->y < height && start->y > height)
    {
        target->y = height;
        target->x = start->x + dx * (height - start->y) / dy;
        target->z = start->z + dz * (height - start->y) / dy;
        return 0;
    }

    return 1;
}

#define SMASH_WINDOW 72
#define BELL 88

int ObjectOnLOS(GAME_VECTOR *start, GAME_VECTOR *target)
{
    int i, j, failure;
    sint32 dx, dy, dz, dist, along, side;
    sint16 item_number;
    sint16 *bounds, *zext, *xext;
    ITEM_INFO *item;

    dx = target->x - start->x;
    dy = target->y - start->y;
    dz = target->z - start->z;
    for (i = 0; i < number_los_rooms; i++) {
        item_number = room[los_rooms[i]].item_number;
        while (item_number != NO_ITEM) {
            item = &items[item_number];
            if (item->status != DEACTIVATED && (item->object_number == SMASH_WINDOW || item->object_number == BELL)) {
                bounds = GetBoundsAccurate(item);
                if (((uint16)(item->pos.y_rot + 0x2000) / 0x4000) & 1) {
                    zext = &bounds[0];
                    xext = &bounds[4];
                } else {
                    zext = &bounds[4];
                    xext = &bounds[0];
                }
                failure = 0;
                if (ABS(dz) > ABS(dx)) {
                    dist = item->pos.z_pos + zext[0] - start->z;
                    for (j = 0; j < 2; j++) {
                        if ((dist & 0x80000000) == (dz & 0x80000000)) {
                            along = dy * dist / dz;
                            if (along > item->pos.y_pos + bounds[2] - start->y && along < item->pos.y_pos + bounds[3] - start->y) {
                                side = dx * dist / dz;
                                if (side < item->pos.x_pos + xext[0] - start->x)
                                    failure |= 1;
                                else if (side > item->pos.x_pos + xext[1] - start->x)
                                    failure |= 2;
                                else
                                    return item_number;
                            }
                        }
                        dist = item->pos.z_pos + zext[1] - start->z;
                    }
                    if (failure == 3)
                        return item_number;
                } else {
                    dist = item->pos.x_pos + xext[0] - start->x;
                    for (j = 0; j < 2; j++) {
                        if ((dist & 0x80000000) == (dx & 0x80000000)) {
                            along = dy * dist / dx;
                            if (along > item->pos.y_pos + bounds[2] - start->y && along < item->pos.y_pos + bounds[3] - start->y) {
                                side = dz * dist / dx;
                                if (side < item->pos.z_pos + zext[0] - start->z)
                                    failure |= 1;
                                else if (side > item->pos.z_pos + zext[1] - start->z)
                                    failure |= 2;
                                else
                                    return item_number;
                            }
                        }
                        dist = item->pos.x_pos + xext[1] - start->x;
                    }
                    if (failure == 3)
                        return item_number;
                }
            }
            item_number = items[item_number].next_item;
        }
    }
    return NO_ITEM;
}

extern sint16 number_rooms;
void RemoveRoomFlipItems(ROOM_INFO *r);
void AddRoomFlipItems(ROOM_INFO *r);

void FlipMap(void)
{
    int i;
    ROOM_INFO *r, *flipped, temp;

    for (i = 0, r = room; i < number_rooms; i++, r++)
    {
        if (r->flipped_room < 0)
            continue;

        RemoveRoomFlipItems(r);

        flipped = &room[r->flipped_room];
        temp = *r;
        *r = *flipped;
        *flipped = temp;

        r->flipped_room = flipped->flipped_room;
        flipped->flipped_room = -1;

        r->item_number = flipped->item_number;
        r->fx_number = flipped->fx_number;

        AddRoomFlipItems(r);
    }

    flip_status = !flip_status;
}

#define KILLED_ITEM (-0x8000)

void MovableBlock(sint16 item_number);
void AlterFloorHeight(ITEM_INFO *item, int height);
void RemoveDrawnItem(sint16 item_num);

void RemoveRoomFlipItems(ROOM_INFO *r)
{
    ITEM_INFO *item;
    sint16 item_num;

    for (item_num = r->item_number; item_num != NO_ITEM; item_num = items[item_num].next_item)
    {
        item = &items[item_num];
        if (objects[item->object_number].control == MovableBlock)
            AlterFloorHeight(item, 1024);
        else if ((item->flags & ONESHOT) && objects[item->object_number].intelligent && item->hit_points <= 0)
        {
            RemoveDrawnItem(item_num);
            item->flags |= KILLED_ITEM;
        }
    }
}

void AddRoomFlipItems(ROOM_INFO *r)
{
    ITEM_INFO *item;
    sint16 item_num;

    for (item_num = r->item_number; item_num != NO_ITEM; item_num = items[item_num].next_item)
    {
        item = &items[item_num];
        if (objects[item->object_number].control == MovableBlock)
            AlterFloorHeight(item, -1024);
    }
}

#define MAX_CD_TRACKS 64

void TriggerNormalCDTrack(sint16 value, sint16 flags, sint16 type);

void TriggerCDTrack(sint16 value, sint16 flags, sint16 type)
{
    if (value <= 1 || value >= MAX_CD_TRACKS)
        return;

    TriggerNormalCDTrack(value, flags, type);
}

extern sint16 cd_flags[];
void S_CDPlay(sint16 track, int mode);

void TriggerNormalCDTrack(sint16 value, sint16 flags, sint16 type)
{
    int timer, code;

    if (type != SWITCH)
    {
        code = flags & CODE_BITS;
        if (cd_flags[value] & code)
            return;
        if (flags & ONESHOT)
            cd_flags[value] |= code;
    }

    if (value != cdtrack)
    {
        timer = flags & 0xff;
        if (timer)
        {
            cd_flags[value] = (cd_flags[value] & 0xff00) + ((timer * 30) & 0xff);
            cdtrack = value;
        }
        else
            S_CDPlay(value, 0);
    }
    else
    {
        timer = cd_flags[value] & 0xff;
        if (timer)
        {
            timer--;
            if (!timer)
            {
                cdtrack = -1;
                S_CDPlay(value, 0);
            }
            cd_flags[value] = (cd_flags[value] & 0xff00) + timer;
        }
    }
}

#ifdef TR2_EXE
sint16 cdtrack = -1;
int animation_rate = 32768;
int flipeffect = -1;
int number_los_rooms;
sint16 *floor_data;
sint16 *commands;
sint16 cd_flags[64];
int flip_status;
int flipmap[10];
sint16 *trigger_index;
int los_rooms[20];
#endif
