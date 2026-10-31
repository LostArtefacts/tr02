#include "game.h"

#define DRAGON_FRONT 22
#define TWINKLE 239

void KillEffect(sint16 fx_num);

void ControlTwinkle(sint16 fx_number)
{
    int c, s;
    FX_INFO *fx;
    ITEM_INFO *item;
    PHD_VECTOR pos;

    fx = &effects[fx_number];

    fx->frame_number--;
    if (fx->frame_number <= objects[fx->object_number].nmeshes)
        fx->frame_number = 0;

    if (fx->counter >= 0)
    {
        item = &items[fx->counter];

        pos.x = item->pos.x_pos;
        pos.y = item->pos.y_pos;
        pos.z = item->pos.z_pos;
        if (item->object_number == DRAGON_FRONT)
        {
            c = phd_cos(item->pos.y_rot);
            s = phd_sin(item->pos.y_rot);
            pos.z += (490 * c - 1100 * s) >> W2V_SHIFT;
            pos.x += (490 * s + 1100 * c) >> W2V_SHIFT;
            pos.y -= 540;
        }
        fx->pos.x_pos += (pos.x - fx->pos.x_pos) >> 4;
        fx->pos.y_pos += (pos.y - fx->pos.y_pos) >> 4;
        fx->pos.z_pos += (pos.z - fx->pos.z_pos) >> 4;

        if (ABS(fx->pos.x_pos - pos.x) < STEP_L && ABS(fx->pos.y_pos - pos.y) < STEP_L && ABS(fx->pos.z_pos - pos.z) < STEP_L)
            KillEffect(fx_number);
    }
    else
    {
        fx->counter++;
        if (fx->counter == 0)
            KillEffect(fx_number);
    }
}

void AddDynamicLight(sint32 x, sint32 y, sint32 z, sint32 intensity, sint32 falloff);

void CreateBartoliLight(sint16 item_number)
{
    ITEM_INFO *item;
    FX_INFO *fx;
    int fx_number;

    item = &items[item_number];
    fx_number = CreateEffect(item->room_number);
    if (fx_number != NO_ITEM)
    {
        fx = &effects[fx_number];
        fx->object_number = TWINKLE;
        fx->pos.y_rot = (sint16)(GetRandomDraw() << 1);
        fx->pos.x_pos = item->pos.x_pos + (WALL_L * 5 * phd_sin(fx->pos.y_rot) >> W2V_SHIFT);
        fx->pos.y_pos = item->pos.y_pos + (GetRandomDraw() >> 2) - WALL_L;
        fx->pos.z_pos = item->pos.z_pos + (WALL_L * 5 * phd_cos(fx->pos.y_rot) >> W2V_SHIFT);
        fx->room_number = item->room_number;
        fx->counter = item_number;
        fx->frame_number = 0;
    }

    AddDynamicLight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, (GetRandomDraw() * 4 >> 15) + 12, (GetRandomDraw() * 4 >> 15) + 10);
}

#include "box.h"

#define DRAGON_BACK 23
#define DRAGON_DEATH 11
#define DRAGON_LCOL -512
#define DRAGON_RCOL 512
#define DRAGON_CLOSE 900
#define DRAGON_FAR 2300
#define DRAGON_MID ((DRAGON_CLOSE + DRAGON_FAR) / 2)
#define DRAGON_DEAD_ANIM 22
#define DRAGON_ALMOST_LIVE 100
#define LARA_EXTRA 12
#define EXTRA_BREATH 0
#define EXTRA_PULLDAGGER 7

int TestCollision(ITEM_INFO *item, ITEM_INFO *laraitem);
extern sint16 cine_frame;
extern PHD_3DPOS cinematic_pos;

void DragonCollision(sint16 item_number, ITEM_INFO *laraitem, COLL_INFO *coll)
{
    ITEM_INFO *item;
    int c, s, rx, rz, shift, side_shift, angle;
    int anim, frame;

    item = &items[item_number];

    if (!TestBoundsCollide(item, laraitem, coll->radius))
        return;
    if (!TestCollision(item, laraitem))
        return;

    if (item->current_anim_state == DRAGON_DEATH)
    {
        rx = laraitem->pos.x_pos - item->pos.x_pos;
        rz = laraitem->pos.z_pos - item->pos.z_pos;
        c = phd_cos(item->pos.y_rot);
        s = phd_sin(item->pos.y_rot);

        side_shift = (rx * s + rz * c) >> W2V_SHIFT;
        if (side_shift > DRAGON_LCOL && side_shift < DRAGON_RCOL)
        {
            shift = (rx * c - rz * s) >> W2V_SHIFT;
            if (shift <= DRAGON_CLOSE && shift >= DRAGON_FAR)
                return;

            angle = laraitem->pos.y_rot - item->pos.y_rot;

            anim = item->anim_number - objects[DRAGON_BACK].anim_index;
            frame = item->frame_number - anims[item->anim_number].frame_base;
            if ((anim == DRAGON_DEAD_ANIM || (anim == DRAGON_DEAD_ANIM + 1 && frame <= DRAGON_ALMOST_LIVE))
                && (input & IN_ACTION) && item->object_number == DRAGON_BACK && !laraitem->gravity_status
                && shift <= DRAGON_MID && shift > DRAGON_CLOSE - 350 && side_shift > -350 && side_shift < 350
                && angle > 0x4000 - 30 * ONE_DEGREE && angle < 0x4000 + 30 * ONE_DEGREE)
            {
                laraitem->anim_number = objects[LARA_EXTRA].anim_index;
                laraitem->frame_number = anims[laraitem->anim_number].frame_base;
                laraitem->current_anim_state = EXTRA_BREATH;
                laraitem->goal_anim_state = EXTRA_PULLDAGGER;

                laraitem->pos.x_pos = item->pos.x_pos;
                laraitem->pos.y_pos = item->pos.y_pos;
                laraitem->pos.z_pos = item->pos.z_pos;
                laraitem->pos.y_rot = item->pos.y_rot;
                laraitem->pos.x_rot = item->pos.x_rot;
                laraitem->pos.z_rot = item->pos.z_rot;
                laraitem->fallspeed = 0;
                laraitem->gravity_status = 0;
                laraitem->speed = 0;

                if (item->room_number != laraitem->room_number)
                    ItemNewRoom(lara.item_number, item->room_number);

                AnimateItem(lara_item);

                lara.extra_anim = 1;
                lara.gun_status = LG_HANDSBUSY;
                lara.hit_direction = -1;

                lara.mesh_ptrs[HAND_R] = meshes[objects[LARA_EXTRA].mesh_index + HAND_R];

                camera.type = CINEMATIC_CAMERA;
                cine_frame = 0;
                memcpy(&cinematic_pos, &laraitem->pos, sizeof(PHD_3DPOS));

                ((CREATURE_INFO *)items[(int)item->data].data)->flags = -1;
                return;
            }

            if (shift < DRAGON_MID)
                shift = DRAGON_CLOSE - shift;
            else
                shift = DRAGON_FAR - shift;

            laraitem->pos.x_pos += shift * c >> W2V_SHIFT;
            laraitem->pos.z_pos -= shift * s >> W2V_SHIFT;
            return;
        }
    }

    ItemPushLara(item, laraitem, coll, 1, 0);
}

typedef struct {
    sint16 zone_number, enemy_zone;
    sint32 distance, ahead, bite;
    sint16 angle, enemy_facing;
} AI_INFO;

int CreatureActive(sint16 item_number);
void CreatureAIInfo(ITEM_INFO *item, AI_INFO *info);
void CreatureMood(ITEM_INFO *item, AI_INFO *info, int violent);
sint16 CreatureTurn(ITEM_INFO *item, sint16 maximum_turn);
void CreatureHead(ITEM_INFO *item, sint16 required);
int CreatureAnimation(sint16 item_number, sint16 angle, sint16 tilt);
void DisableBaddieAI(sint16 item_number);
void DragonBones(sint16 item_number);
sint16 DragonFire(sint32 x, sint32 y, sint32 z, sint16 speed, PHD_ANGLE yrot, sint16 room_number);

#define SQUARE(A) ((A) * (A))

#define DRAGON_EMPTY 0
#define DRAGON_WALK 1
#define DRAGON_LEFT 2
#define DRAGON_RIGHT 3
#define DRAGON_AIM1 4
#define DRAGON_FIRE1 5
#define DRAGON_STOP 6
#define DRAGON_TURNLEFT 7
#define DRAGON_TURNRIGHT 8
#define DRAGON_SWIPELEFT 9
#define DRAGON_SWIPERIGHT 10
#define DRAGON_DIE_ANIM 21
#define DRAGON_LIVE_TIME (30 * 11)
#define DRAGON_CLOSE_RANGE SQUARE(WALL_L * 3)
#define DRAGON_STOP_RANGE SQUARE(WALL_L * 6)
#define DRAGON_WALK_TURN (ONE_DEGREE * 2)
#define DRAGON_NEED_TURN (ONE_DEGREE * 1)
#define DRAGON_TOUCH_DAMAGE 10
#define DRAGON_SWIPE_DAMAGE 250
#define DRAGON_TOUCH_L 0x7f000000
#define DRAGON_TOUCH_R 0x0fe
#define VIOLENT 1

extern BITE_INFO dragon_mouth;

sint16 CreateItem(void);
void InitialiseItem(sint16 item_num);

#define DEAD_DRAGON_FRONT 218
#define DEAD_DRAGON_BACK 219

void DragonBones(sint16 item_number)
{
    sint16 bone_back, bone_front;
    ITEM_INFO *front, *back, *item;

    bone_front = CreateItem();
    bone_back = CreateItem();

    if (bone_back != NO_ITEM && bone_front != NO_ITEM)
    {
        item = &items[item_number];

        back = &items[bone_back];
        back->object_number = DEAD_DRAGON_BACK;
        back->pos.x_pos = item->pos.x_pos;
        back->pos.y_pos = item->pos.y_pos;
        back->pos.z_pos = item->pos.z_pos;
        back->pos.x_rot = back->pos.z_rot = 0;
        back->pos.y_rot = item->pos.y_rot;
        back->room_number = item->room_number;
        back->shade = -1;

        InitialiseItem(bone_back);

        front = &items[bone_front];
        front->object_number = DEAD_DRAGON_FRONT;
        front->pos.x_pos = item->pos.x_pos;
        front->pos.y_pos = item->pos.y_pos;
        front->pos.z_pos = item->pos.z_pos;
        front->pos.x_rot = front->pos.z_rot = 0;
        front->pos.y_rot = item->pos.y_rot;
        front->room_number = item->room_number;
        front->shade = -1;

        InitialiseItem(bone_front);

        front->mesh_bits = 0xff3fffff;
    }
}
