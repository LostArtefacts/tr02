#include "sound.h"
#include "game.h"
#include "specific/init.h"

#define BOAT_GETON 0
#define BOAT_STILL 1
#define BOAT_MOVING 2
#define BOAT_JUMPR 3
#define BOAT_JUMPL 4
#define BOAT_HIT 5
#define BOAT_FALL 6
#define BOAT_NOTUSED 7
#define BOAT_DEATH 8

#define BOAT_GETONLW_ANIM 0
#define BOAT_GETONRW_ANIM 8
#define BOAT_GETONJ_ANIM 6
#define BOAT_GETON_START 1
#define BOAT_FALL_ANIM 15
#define BOAT_DEATH_ANIM 18

#define BOAT_UNDO_TURN (ONE_DEGREE / 4)
#define BOAT_TURN (ONE_DEGREE / 8)
#define BOAT_MAX_TURN (ONE_DEGREE * 4)
#define BOAT_MAX_SPEED 90
#define BOAT_SLOW_SPEED (BOAT_MAX_SPEED / 3)
#define BOAT_FAST_SPEED (BOAT_MAX_SPEED + 50)
#define BOAT_MIN_SPEED 20
#define BOAT_ACCELERATION 5
#define BOAT_BRAKE 5
#define BOAT_SLOWDOWN 1
#define BOAT_REVERSE -5
#define BOAT_MAX_BACK -20
#define BOAT_MAX_KICK -80
#define BOAT_SLIP 10
#define BOAT_SIDE_SLIP 30
#define BOAT_FRONT 750
#define BOAT_SIDE 300
#define BOAT_RADIUS 500
#define BOAT_SNOW 500
#define BOAT_MAX_HEIGHT (STEP_L)
#define BOAT_GETOFF_DIST (STEP_L * 2)
#define BOAT_WAKE 700
#define BOAT_SOUND_CEILING (WALL_L * 5)

typedef struct {
    int boat_turn;
    int left_fallspeed, right_fallspeed;
    sint16 tilt_angle, extra_rotation;
    int water, pitch;
} BOAT_INFO;

void ObjectCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);

void InitialiseBoat(sint16 item_number)
{
    ITEM_INFO *boat;
    BOAT_INFO *binfo;

    boat = &items[item_number];
    binfo = (BOAT_INFO *)game_malloc(sizeof(BOAT_INFO), TEMP_ALLOC);
    boat->data = binfo;
    binfo->boat_turn = 0;
    binfo->left_fallspeed = binfo->right_fallspeed = 0;
    binfo->tilt_angle = 0;
    binfo->extra_rotation = 0;
    binfo->water = 0;
    binfo->pitch = 0;
}

#define MOTORBOAT 14
#define BOAT_ANIM 11
#define GONDOLA 24
#define GONDOLA_FLOATING 1
#define GONDOLA_CRASH 2
#define SQUARE(A) ((A) * (A))

int BoatCheckGeton(sint16 item_number, COLL_INFO *coll)
{
    int geton, dist, x1, x2;
    sint16 rot;
    ITEM_INFO *boat;

    if (lara.gun_status != LG_ARMLESS)
        return 0;
    boat = &items[item_number];
    x1 = lara_item->pos.x_pos;
    x2 = boat->pos.x_pos;
    dist = (lara_item->pos.z_pos - boat->pos.z_pos) * phd_cos(-boat->pos.y_rot);
    dist = (dist - (x1 - x2) * phd_sin(-boat->pos.y_rot)) >> W2V_SHIFT;
    if (dist > 200)
        return 0;
    rot = boat->pos.y_rot - lara_item->pos.y_rot;
    geton = 0;
    if (lara.water_status == LARA_SURFACE || lara.water_status == LARA_WADE)
    {
        if (!(input & IN_ACTION) || lara_item->gravity_status || boat->speed)
            return 0;
        if (rot > 0x2000 && rot < 0x6000)
            geton = 1;
        else if (rot > -0x6000 && rot < -0x2000)
            geton = 2;
    }
    else if (lara.water_status == LARA_ABOVEWATER)
    {
        if (lara_item->fallspeed > 0)
        {
            if (rot > -0x6000 && rot < 0x6000 && lara_item->pos.y_pos > boat->pos.y_pos)
                geton = 3;
        }
        else if (lara_item->fallspeed == 0)
        {
            if (rot > -0x6000 && rot < 0x6000)
            {
                if (x1 == x2 && lara_item->pos.y_pos == boat->pos.y_pos && lara_item->pos.z_pos == boat->pos.z_pos)
                    geton = 4;
                else
                    geton = 3;
            }
        }
    }
    if (geton && TestBoundsCollide(boat, lara_item, coll->radius))
        return TestCollision(boat, lara_item) ? geton : 0;
    return 0;
}

void BoatCollision(sint16 item_number, ITEM_INFO *litem, COLL_INFO *coll)
{
    int geton;
    ITEM_INFO *item;

    if (litem->hit_points < 0 || lara.skidoo != NO_ITEM)
        return;
    geton = BoatCheckGeton(item_number, coll);
    if (!geton)
    {
        coll->enable_baddie_push = 1;
        ObjectCollision(item_number, litem, coll);
        return;
    }
    lara.skidoo = item_number;
    if (geton == 2)
        litem->anim_number = objects[BOAT_ANIM].anim_index + BOAT_GETONLW_ANIM;
    else if (geton == 1)
        litem->anim_number = objects[BOAT_ANIM].anim_index + BOAT_GETONRW_ANIM;
    else if (geton == 3)
        litem->anim_number = objects[BOAT_ANIM].anim_index + BOAT_GETONJ_ANIM;
    else
        litem->anim_number = objects[BOAT_ANIM].anim_index + BOAT_GETON_START;
    lara.water_status = LARA_ABOVEWATER;
    item = &items[item_number];
    litem->pos.x_pos = item->pos.x_pos;
    litem->pos.y_pos = item->pos.y_pos - 5;
    litem->pos.z_pos = item->pos.z_pos;
    litem->pos.y_rot = item->pos.y_rot;
    litem->pos.x_rot = litem->pos.z_rot = 0;
    litem->gravity_status = 0;
    litem->speed = 0;
    litem->fallspeed = 0;
    litem->frame_number = anims[litem->anim_number].frame_base;
    litem->current_anim_state = litem->goal_anim_state = BOAT_GETON;
    if (litem->room_number != item->room_number)
        ItemNewRoom(lara.item_number, item->room_number);
    AnimateItem(litem);
    if (items[item_number].status != ACTIVE)
    {
        AddActiveItem(item_number);
        items[item_number].status = ACTIVE;
    }
}

sint32 TestWaterHeight(ITEM_INFO *item, sint32 z_off, sint32 x_off, PHD_VECTOR *pos)
{
    FLOOR_INFO *floor;
    sint16 room_number;
    sint32 s, c, height;

    pos->y = item->pos.y_pos - (z_off * phd_sin(item->pos.x_rot) >> W2V_SHIFT) + (x_off * phd_sin(item->pos.z_rot) >> W2V_SHIFT);
    c = phd_cos(item->pos.y_rot);
    s = phd_sin(item->pos.y_rot);
    pos->z = item->pos.z_pos + ((z_off * c - x_off * s) >> W2V_SHIFT);
    pos->x = item->pos.x_pos + ((z_off * s + x_off * c) >> W2V_SHIFT);
    room_number = item->room_number;
    GetFloor(pos->x, pos->y, pos->z, &room_number);
    height = GetWaterHeight(pos->x, pos->y, pos->z, room_number);
    if (height == NO_HEIGHT)
    {
        floor = GetFloor(pos->x, pos->y, pos->z, &room_number);
        height = GetHeight(floor, pos->x, pos->y, pos->z);
        if (height == NO_HEIGHT)
            return height;
    }
    return height - 5;
}

void DoBoatShift(int boat_number)
{
    ITEM_INFO *item, *boat;
    int item_number, distance, x, z, dx, dz;

    boat = &items[boat_number];
    item_number = room[boat->room_number].item_number;
    while (item_number != NO_ITEM) {
        item = &items[item_number];
        if (item->object_number == MOTORBOAT && item_number != boat_number && lara.skidoo != item_number) {
            x = item->pos.x_pos - boat->pos.x_pos;
            z = item->pos.z_pos - boat->pos.z_pos;
            distance = x * x + z * z;
            if (distance < SQUARE(BOAT_RADIUS * 2)) {
                boat->pos.x_pos = item->pos.x_pos - x * SQUARE(BOAT_RADIUS * 2) / distance;
                boat->pos.z_pos = item->pos.z_pos - z * SQUARE(BOAT_RADIUS * 2) / distance;
            }
            return;
        }

        if (item->object_number == GONDOLA && item->current_anim_state == GONDOLA_FLOATING) {
            x = item->pos.x_pos - (512 * phd_sin(item->pos.y_rot) >> W2V_SHIFT);
            z = item->pos.z_pos - (512 * phd_cos(item->pos.y_rot) >> W2V_SHIFT);
            dx = x - boat->pos.x_pos;
            dz = z - boat->pos.z_pos;
            distance = dx * dx + dz * dz;
            if (distance < SQUARE(BOAT_RADIUS * 2)) {
                if (boat->speed < BOAT_MAX_SPEED - 10) {
                    boat->pos.x_pos = x - dx * SQUARE(BOAT_RADIUS * 2) / distance;
                    boat->pos.z_pos = z - dz * SQUARE(BOAT_RADIUS * 2) / distance;
                } else if (item->pos.y_pos - boat->pos.y_pos < WALL_L * 2) {
                    SoundEffect(337, &item->pos, 0);
                    item->goal_anim_state = GONDOLA_CRASH;
                }
            }
        }

        item_number = item->next_item;
    }
}

#define WATER_SPRITE 89

void S_CalculateLight(sint32 x, sint32 y, sint32 z, sint16 room_number);

extern sint32 LsAdder;

void DoWakeEffect(ITEM_INFO *boat)
{
    int i;
    sint32 c, s, w, h;
    sint16 fx_number, frame;
    FX_INFO *fx;

    *(phd_mxptr + M23) = 0;
    S_CalculateLight(boat->pos.x_pos, boat->pos.y_pos, boat->pos.z_pos, boat->room_number);

    frame = GetRandomDraw() * objects[WATER_SPRITE].nmeshes >> 15;
    for (i = 0; i < 3; i++) {
        fx_number = CreateEffect(boat->room_number);
        if (fx_number != NO_ITEM) {
            fx = &effects[fx_number];
            c = phd_cos(boat->pos.y_rot);
            s = phd_sin(boat->pos.y_rot);
            h = BOAT_WAKE;
            w = (1 - i) * BOAT_SIDE;
            fx->pos.x_pos = boat->pos.x_pos + ((-(c * w) - (h * s)) >> W2V_SHIFT);
            fx->pos.y_pos = boat->pos.y_pos;
            fx->pos.z_pos = boat->pos.z_pos + (((s * w) - (h * c)) >> W2V_SHIFT);
            fx->pos.y_rot = boat->pos.y_rot - (1 - i) * 0x4000;
            fx->room_number = boat->room_number;
            fx->frame_number = frame;
            fx->counter = 20;
            fx->object_number = WATER_SPRITE;
            fx->speed = boat->speed >> 2;
            if (boat->speed < 64)
                fx->fallspeed = GetRandomDraw() * (ABS(boat->speed) - 64) >> 15;
            else
                fx->fallspeed = 0;
            fx->shade = LsAdder - 0x300;
            if (fx->shade < 0)
                fx->shade = 0;
        }
    }
}

#define GRAVITY 6

int DoBoatDynamics(int height, int fallspeed, int *y)
{
    if (height > *y) {
        *y += fallspeed;
        if (*y > height) {
            *y = height;
            fallspeed = 0;
        } else
            fallspeed += GRAVITY;
    } else {
        fallspeed += ((height - *y - fallspeed) >> 3);
        if (fallspeed < -20)
            fallspeed = -20;
        if (*y > height)
            *y = height;
    }
    return fallspeed;
}

#define BOAT_TIP (BOAT_FRONT + 250)

int DoShift(ITEM_INFO *skidoo, PHD_VECTOR *pos, PHD_VECTOR *old);
int GetCollisionAnim(ITEM_INFO *skidoo, PHD_VECTOR *moved);

int BoatDynamics(sint16 boat_number)
{
    ITEM_INFO *boat;
    BOAT_INFO *binfo;
    PHD_VECTOR moved, fl, fr, br, bl, f;
    PHD_VECTOR old, fl_old, fr_old, bl_old, br_old, f_old;
    sint32 hfl, hfr, hbr, hbl, hf;
    sint32 hfr_old, hfl_old, hbr_old, hbl_old, hf_old;
    FLOOR_INFO *floor;
    sint32 height, slip, collide;
    sint16 room_number, rot;
    int newspeed;

    boat = &items[boat_number];
    binfo = (BOAT_INFO *)boat->data;
    boat->pos.z_rot -= binfo->tilt_angle;
    hfl_old = TestWaterHeight(boat, BOAT_FRONT, -BOAT_SIDE, &fl_old);
    hfr_old = TestWaterHeight(boat, BOAT_FRONT, BOAT_SIDE, &fr_old);
    hbl_old = TestWaterHeight(boat, -BOAT_FRONT, -BOAT_SIDE, &bl_old);
    hbr_old = TestWaterHeight(boat, -BOAT_FRONT, BOAT_SIDE, &br_old);
    hf_old = TestWaterHeight(boat, BOAT_TIP, 0, &f_old);
    old.x = boat->pos.x_pos;
    old.y = boat->pos.y_pos;
    old.z = boat->pos.z_pos;
    if (bl_old.y > hbl_old)
        bl_old.y = hbl_old;
    if (br_old.y > hbr_old)
        br_old.y = hbr_old;
    if (fl_old.y > hfl_old)
        fl_old.y = hfl_old;
    if (fr_old.y > hfr_old)
        fr_old.y = hfr_old;
    if (f_old.y > hf_old)
        f_old.y = hf_old;
    boat->pos.y_rot += binfo->boat_turn + binfo->extra_rotation;
    binfo->tilt_angle = binfo->boat_turn * 6;
    boat->pos.z_pos += boat->speed * phd_cos(boat->pos.y_rot) >> W2V_SHIFT;
    boat->pos.x_pos += boat->speed * phd_sin(boat->pos.y_rot) >> W2V_SHIFT;
    slip = BOAT_SIDE_SLIP * phd_sin(boat->pos.z_rot) >> W2V_SHIFT;
    if (!slip && boat->pos.z_rot)
        slip = (boat->pos.z_rot > 0) ? 1 : -1;
    boat->pos.z_pos -= slip * phd_sin(boat->pos.y_rot) >> W2V_SHIFT;
    boat->pos.x_pos += slip * phd_cos(boat->pos.y_rot) >> W2V_SHIFT;
    slip = BOAT_SLIP * phd_sin(boat->pos.x_rot) >> W2V_SHIFT;
    if (!slip && boat->pos.x_rot)
        slip = (boat->pos.x_rot > 0) ? 1 : -1;
    boat->pos.z_pos -= slip * phd_cos(boat->pos.y_rot) >> W2V_SHIFT;
    boat->pos.x_pos -= slip * phd_sin(boat->pos.y_rot) >> W2V_SHIFT;
    moved.x = boat->pos.x_pos;
    moved.z = boat->pos.z_pos;
    DoBoatShift(boat_number);
    rot = 0;
    hbl = TestWaterHeight(boat, -BOAT_FRONT, -BOAT_SIDE, &bl);
    if (hbl < bl_old.y - STEP_L / 2)
        rot = DoShift(boat, &bl, &bl_old);
    hbr = TestWaterHeight(boat, -BOAT_FRONT, BOAT_SIDE, &br);
    if (hbr < br_old.y - STEP_L / 2)
        rot += DoShift(boat, &br, &br_old);
    hfl = TestWaterHeight(boat, BOAT_FRONT, -BOAT_SIDE, &fl);
    if (hfl < fl_old.y - STEP_L / 2)
        rot += DoShift(boat, &fl, &fl_old);
    hfr = TestWaterHeight(boat, BOAT_FRONT, BOAT_SIDE, &fr);
    if (hfr < fr_old.y - STEP_L / 2)
        rot += DoShift(boat, &fr, &fr_old);
    if (!slip)
    {
        hf = TestWaterHeight(boat, BOAT_TIP, 0, &f);
        if (hf < f_old.y - STEP_L / 2)
            DoShift(boat, &f, &f_old);
    }
    room_number = boat->room_number;
    floor = GetFloor(boat->pos.x_pos, boat->pos.y_pos, boat->pos.z_pos, &room_number);
    height = GetWaterHeight(boat->pos.x_pos, boat->pos.y_pos, boat->pos.z_pos, room_number);
    if (height == NO_HEIGHT)
        height = GetHeight(floor, boat->pos.x_pos, boat->pos.y_pos, boat->pos.z_pos);
    if (height < boat->pos.y_pos - STEP_L / 2)
        DoShift(boat, (PHD_VECTOR *)&boat->pos, &old);
    binfo->extra_rotation = rot;
    collide = GetCollisionAnim(boat, &moved);
    if (slip || collide)
    {
        newspeed = (boat->pos.z_pos - old.z) * phd_cos(boat->pos.y_rot) + (boat->pos.x_pos - old.x) * phd_sin(boat->pos.y_rot) >> W2V_SHIFT;
        if (lara.skidoo == boat_number && boat->speed > BOAT_MAX_SPEED + BOAT_ACCELERATION && newspeed < boat->speed - 10)
        {
            lara_item->hit_points -= (boat->speed - newspeed) >> 1;
            lara_item->hit_status = 1;
            SoundEffect(31, &lara_item->pos, 0);
        }
        if (slip)
        {
            if (boat->speed <= BOAT_MAX_SPEED + 10)
                boat->speed = newspeed;
        }
        else
        {
            if (boat->speed > 0 && newspeed < boat->speed)
                boat->speed = newspeed;
            else if (boat->speed < 0 && newspeed > boat->speed)
                boat->speed = newspeed;
        }
        if (boat->speed < BOAT_MAX_BACK)
            boat->speed = BOAT_MAX_BACK;
    }
    return collide;
}

int BoatUserControl(ITEM_INFO *boat)
{
    int no_turn = 1, max_speed;
    BOAT_INFO *binfo;

    binfo = (BOAT_INFO *)boat->data;

    if (boat->pos.y_pos >= binfo->water - STEP_L / 2 && binfo->water != NO_HEIGHT) {
        if (!(input & IN_JUMP) && !(input & IN_LOOK)) {
            if (((input & IN_LEFT) && !(input & IN_BACK)) || ((input & IN_RIGHT) && (input & IN_BACK))) {
                if (binfo->boat_turn > 0)
                    binfo->boat_turn -= BOAT_UNDO_TURN;
                else {
                    binfo->boat_turn -= BOAT_TURN;
                    if (binfo->boat_turn < -BOAT_MAX_TURN)
                        binfo->boat_turn = -BOAT_MAX_TURN;
                }
                no_turn = 0;
            } else if (((input & IN_RIGHT) && !(input & IN_BACK)) || ((input & IN_LEFT) && (input & IN_BACK))) {
                if (binfo->boat_turn < 0)
                    binfo->boat_turn += BOAT_UNDO_TURN;
                else {
                    binfo->boat_turn += BOAT_TURN;
                    if (binfo->boat_turn > BOAT_MAX_TURN)
                        binfo->boat_turn = BOAT_MAX_TURN;
                }
                no_turn = 0;
            }

            if (input & IN_BACK) {
                if (boat->speed > 0)
                    boat->speed -= BOAT_BRAKE;
                else if (boat->speed > BOAT_MAX_BACK)
                    boat->speed += BOAT_REVERSE;
            } else if (input & IN_FORWARD) {
                if (input & IN_ACTION)
                    max_speed = BOAT_FAST_SPEED;
                else
                    max_speed = (input & IN_SLOW) ? BOAT_SLOW_SPEED : BOAT_MAX_SPEED;

                if (boat->speed < max_speed)
                    boat->speed += BOAT_ACCELERATION / 2 + BOAT_ACCELERATION * boat->speed / (2 * max_speed);
                else if (boat->speed > max_speed + BOAT_SLOWDOWN)
                    boat->speed -= BOAT_SLOWDOWN;
            } else if (boat->speed >= 0 && boat->speed < BOAT_MIN_SPEED && (input & (IN_LEFT | IN_RIGHT)))
                boat->speed = BOAT_MIN_SPEED;
            else if (boat->speed > BOAT_SLOWDOWN)
                boat->speed -= BOAT_SLOWDOWN;
            else
                boat->speed = 0;
        } else if ((input & IN_LOOK) && boat->speed == 0)
            LookUpDown();
    }

    return no_turn;
}


void BoatAnimation(ITEM_INFO *boat, int collide)
{
    BOAT_INFO *binfo;

    binfo = (BOAT_INFO *)boat->data;

    if (lara_item->hit_points <= 0) {
        if (lara_item->current_anim_state != BOAT_DEATH) {
            lara_item->anim_number = objects[BOAT_ANIM].anim_index + BOAT_DEATH_ANIM;
            lara_item->frame_number = anims[lara_item->anim_number].frame_base;
            lara_item->current_anim_state = lara_item->goal_anim_state = BOAT_DEATH;
        }
    } else if (boat->pos.y_pos < binfo->water - STEP_L / 2 && boat->fallspeed > 0) {
        if (lara_item->current_anim_state != BOAT_FALL) {
            lara_item->anim_number = objects[BOAT_ANIM].anim_index + BOAT_FALL_ANIM;
            lara_item->frame_number = anims[lara_item->anim_number].frame_base;
            lara_item->current_anim_state = lara_item->goal_anim_state = BOAT_FALL;
        }
    } else if (collide) {
        if (lara_item->current_anim_state != BOAT_HIT) {
            lara_item->anim_number = (sint16)(objects[BOAT_ANIM].anim_index + collide);
            lara_item->frame_number = anims[lara_item->anim_number].frame_base;
            lara_item->current_anim_state = lara_item->goal_anim_state = BOAT_HIT;
        }
    } else {
        switch (lara_item->current_anim_state) {
        case BOAT_STILL:
            if (input & IN_JUMP) {
                if (input & IN_RIGHT)
                    lara_item->goal_anim_state = BOAT_JUMPR;
                else if (input & IN_LEFT)
                    lara_item->goal_anim_state = BOAT_JUMPL;
            }
            if (boat->speed > 0)
                lara_item->goal_anim_state = BOAT_MOVING;
            break;

        case BOAT_MOVING:
            if (input & IN_JUMP) {
                if (input & IN_RIGHT)
                    lara_item->goal_anim_state = BOAT_JUMPR;
                else if (input & IN_LEFT)
                    lara_item->goal_anim_state = BOAT_JUMPL;
            } else if (boat->speed <= 0)
                lara_item->goal_anim_state = BOAT_STILL;
            break;

        case BOAT_FALL:
            lara_item->goal_anim_state = BOAT_MOVING;
            break;
        }
    }
}

#define PITCH_SHIFT 4

sint32 GetCeiling(FLOOR_INFO *floor, sint32 x, sint32 y, sint32 z);

void BoatControl(sint16 item_number)
{
    ITEM_INFO *boat;
    BOAT_INFO *binfo;
    PHD_VECTOR fl, fr;
    sint32 hfl, hfr, no_turn = 1, drive = 0;
    FLOOR_INFO *floor;
    sint32 height, collide, water, ceiling, pitch;
    sint16 room_number, x_rot, z_rot;

    boat = &items[item_number];
    binfo = (BOAT_INFO *)boat->data;
    collide = BoatDynamics(item_number);
    hfl = TestWaterHeight(boat, BOAT_FRONT, -BOAT_SIDE, &fl);
    hfr = TestWaterHeight(boat, BOAT_FRONT, BOAT_SIDE, &fr);
    room_number = boat->room_number;
    floor = GetFloor(boat->pos.x_pos, boat->pos.y_pos, boat->pos.z_pos, &room_number);
    height = GetHeight(floor, boat->pos.x_pos, boat->pos.y_pos, boat->pos.z_pos);
    ceiling = GetCeiling(floor, boat->pos.x_pos, boat->pos.y_pos, boat->pos.z_pos);
    if (lara.skidoo == item_number) {
        TestTriggers(trigger_index, 0);
        TestTriggers(trigger_index, 1);
    }
    binfo->water = water = GetWaterHeight(boat->pos.x_pos, boat->pos.y_pos, boat->pos.z_pos, room_number);
    if (lara.skidoo == item_number && lara_item->hit_points > 0) {
        switch (lara_item->current_anim_state) {
        case BOAT_GETON:
        case BOAT_JUMPR:
        case BOAT_JUMPL:
            break;
        default:
            drive = 1;
            no_turn = BoatUserControl(boat);
        }
    } else {
        if (boat->speed > BOAT_SLOWDOWN)
            boat->speed -= BOAT_SLOWDOWN;
        else
            boat->speed = 0;
    }
    if (no_turn) {
        if (binfo->boat_turn < -BOAT_UNDO_TURN)
            binfo->boat_turn += BOAT_UNDO_TURN;
        else if (binfo->boat_turn > BOAT_UNDO_TURN)
            binfo->boat_turn -= BOAT_UNDO_TURN;
        else
            binfo->boat_turn = 0;
    }
    boat->floor = height - 5;
    if (binfo->water == NO_HEIGHT)
        binfo->water = height;
    else
        binfo->water -= 5;
    binfo->left_fallspeed = DoBoatDynamics(hfl, binfo->left_fallspeed, &fl.y);
    binfo->right_fallspeed = DoBoatDynamics(hfr, binfo->right_fallspeed, &fr.y);
    boat->fallspeed = DoBoatDynamics(binfo->water, boat->fallspeed, &boat->pos.y_pos);
    height = (fl.y + fr.y) >> 1;
    x_rot = phd_atan(BOAT_FRONT, boat->pos.y_pos - height);
    z_rot = phd_atan(BOAT_SIDE, height - fl.y);
    boat->pos.x_rot += (x_rot - boat->pos.x_rot) >> 1;
    boat->pos.z_rot += (z_rot - boat->pos.z_rot) >> 1;
    if (!x_rot && ABS(boat->pos.x_rot) < 4)
        boat->pos.x_rot = 0;
    if (!z_rot && ABS(boat->pos.z_rot) < 4)
        boat->pos.z_rot = 0;
    if (lara.skidoo == item_number) {
        BoatAnimation(boat, collide);
        if (room_number != boat->room_number) {
            ItemNewRoom(item_number, room_number);
            ItemNewRoom(lara.item_number, room_number);
        }
        boat->pos.z_rot += binfo->tilt_angle;
        lara_item->pos.x_pos = boat->pos.x_pos;
        lara_item->pos.y_pos = boat->pos.y_pos;
        lara_item->pos.z_pos = boat->pos.z_pos;
        lara_item->pos.x_rot = boat->pos.x_rot;
        lara_item->pos.y_rot = boat->pos.y_rot;
        lara_item->pos.z_rot = boat->pos.z_rot;
        AnimateItem(lara_item);
        if (lara_item->hit_points > 0) {
            boat->anim_number = objects[MOTORBOAT].anim_index + (lara_item->anim_number - objects[BOAT_ANIM].anim_index);
            {
                ITEM_INFO *rider = lara_item;
                boat->frame_number = anims[boat->anim_number].frame_base + (rider->frame_number - anims[rider->anim_number].frame_base);
            }
        }
        camera.target_elevation = -20 * ONE_DEGREE;
        camera.target_distance = WALL_L * 2;
    } else {
        if (room_number != boat->room_number)
            ItemNewRoom(item_number, room_number);
        boat->pos.z_rot += binfo->tilt_angle;
    }
    ceiling = water - ceiling;
    if (ceiling < BOAT_SOUND_CEILING)
        pitch = boat->speed * ceiling / BOAT_SOUND_CEILING;
    else
        pitch = boat->speed;
    binfo->pitch += (pitch - binfo->pitch) >> 2;
    if (boat->speed && water - 5 != boat->pos.y_pos)
        SoundEffect(336, &boat->pos, 0);
    else if (boat->speed > 20)
        SoundEffect(197, &boat->pos, PITCH_SHIFT + ((0x10000 - (BOAT_MAX_SPEED - binfo->pitch) * 100) << 8));
    else if (drive)
        SoundEffect(195, &boat->pos, PITCH_SHIFT + ((0x10000 - (BOAT_MAX_SPEED - binfo->pitch) * 100) << 8));
    if (boat->speed && water - 5 == boat->pos.y_pos)
        DoWakeEffect(boat);
    if (lara.skidoo != item_number)
        return;
    if ((lara_item->current_anim_state == BOAT_JUMPR || lara_item->current_anim_state == BOAT_JUMPL) && lara_item->frame_number == anims[lara_item->anim_number].frame_end) {
        sint16 room_number;
        int x, y, z;
        FLOOR_INFO *floor;

        if (lara_item->current_anim_state == BOAT_JUMPL)
            lara_item->pos.y_rot -= 0x4000;
        else
            lara_item->pos.y_rot += 0x4000;
        lara_item->anim_number = 77;
        lara_item->frame_number = anims[lara_item->anim_number].frame_base;
        lara_item->current_anim_state = lara_item->goal_anim_state = AS_FORWARDJUMP;
        lara_item->gravity_status = 1;
        lara_item->fallspeed = -40;
        lara_item->speed = 20;
        lara_item->pos.x_rot = lara_item->pos.z_rot = 0;
        lara.skidoo = NO_ITEM;
        room_number = lara_item->room_number;
        x = lara_item->pos.x_pos + (360 * phd_sin(lara_item->pos.y_rot) >> W2V_SHIFT);
        y = lara_item->pos.y_pos - 90;
        z = lara_item->pos.z_pos + (360 * phd_cos(lara_item->pos.y_rot) >> W2V_SHIFT);
        floor = GetFloor(x, y, z, &room_number);
        if (GetHeight(floor, x, y, z) >= y - STEP_L) {
            lara_item->pos.x_pos = x;
            lara_item->pos.z_pos = z;
            if (room_number != lara_item->room_number)
                ItemNewRoom(lara.item_number, room_number);
        }
        lara_item->pos.y_pos = y;
        boat->anim_number = objects[MOTORBOAT].anim_index;
        boat->frame_number = anims[boat->anim_number].frame_base;
    }
}

int ExplodingDeath(sint16 item_number, sint32 mesh_bits, sint16 damage);

#define GONDOLA_FLOATING 1
#define GONDOLA_CRASH 2
#define GONDOLA_SINK 3
#define GONDOLA_LAND 4
#define GONDOLA_SINK_SPEED 50

void GondolaControl(sint16 item_number)
{
    ITEM_INFO *item;
    sint16 room_number;
    FLOOR_INFO *floor;

    item = &items[item_number];
    switch (item->current_anim_state) {
    case GONDOLA_FLOATING:
        if (item->goal_anim_state == GONDOLA_CRASH) {
            item->mesh_bits = 0xff;
            ExplodingDeath(item_number, 240, 0);
        }
        break;

    case GONDOLA_SINK:
        item->pos.y_pos += GONDOLA_SINK_SPEED;
        room_number = item->room_number;
        floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
        item->floor = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
        if (item->pos.y_pos >= item->floor) {
            item->goal_anim_state = GONDOLA_LAND;
            item->pos.y_pos = item->floor;
        }
        break;
    }

    AnimateItem(item);
    if (item->status == DEACTIVATED)
        RemoveActiveItem(item_number);
}
