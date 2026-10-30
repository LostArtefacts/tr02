#include "game.h"

#define DRAW1_F 5
#define DRAW2_F 13
#define RECOIL_F 24

void set_pistol_arm(LARA_ARM *arm, int frame)
{
    int anim_base;

    anim_base = objects[PISTOLS].anim_index;
    if (frame < DRAW1_F)
        arm->anim_number = anim_base;
    else if (frame < DRAW2_F)
        arm->anim_number = anim_base + 1;
    else if (frame < RECOIL_F)
        arm->anim_number = anim_base + 2;
    else
        arm->anim_number = anim_base + 3;
    arm->frame_base = anims[arm->anim_number].frame_ptr;
    arm->frame_number = frame;
}

void ready_pistols(int weapon_type);
void draw_pistol_meshes(int weapon_type);

void draw_pistols(int weapon_type)
{
    sint16 ani;

    ani = lara.left_arm.frame_number;
    ani++;
    if (ani < DRAW1_F || ani > RECOIL_F - 1)
        ani = DRAW1_F;
    else if (ani == DRAW2_F)
    {
        draw_pistol_meshes(weapon_type);
        SoundEffect(6, &lara_item->pos, 0);
    }
    else if (ani == RECOIL_F - 1)
    {
        ready_pistols(weapon_type);
        ani = 0;
    }
    set_pistol_arm(&lara.right_arm, ani);
    set_pistol_arm(&lara.left_arm, ani);
}

void undraw_pistol_mesh_left(int weapon_type);
void undraw_pistol_mesh_right(int weapon_type);

void undraw_pistols(int weapon_type)
{
    sint16 anil, anir;

    anil = lara.left_arm.frame_number;
    if (anil >= RECOIL_F)
        anil = DRAW1_F - 1;
    else if (anil > 0 && anil < DRAW1_F)
    {
        lara.left_arm.x_rot -= lara.left_arm.x_rot / anil;
        lara.left_arm.y_rot -= lara.left_arm.y_rot / anil;
        anil--;
    }
    else if (anil == 0)
    {
        lara.left_arm.x_rot = lara.left_arm.y_rot = lara.left_arm.z_rot = 0;
        anil = RECOIL_F - 1;
    }
    else if (anil > DRAW1_F && anil < RECOIL_F)
    {
        anil--;
        if (anil == DRAW2_F - 1)
        {
            undraw_pistol_mesh_left(weapon_type);
            SoundEffect(7, &lara_item->pos, 0);
        }
    }
    set_pistol_arm(&lara.left_arm, anil);

    anir = lara.right_arm.frame_number;
    if (anir >= RECOIL_F)
        anir = DRAW1_F - 1;
    else if (anir > 0 && anir < DRAW1_F)
    {
        lara.right_arm.x_rot -= lara.right_arm.x_rot / anir;
        lara.right_arm.y_rot -= lara.right_arm.y_rot / anir;
        anir--;
    }
    else if (anir == 0)
    {
        lara.right_arm.x_rot = lara.right_arm.y_rot = lara.right_arm.z_rot = 0;
        anir = RECOIL_F - 1;
    }
    else if (anir > DRAW1_F && anir < RECOIL_F)
    {
        anir--;
        if (anir == DRAW2_F - 1)
        {
            undraw_pistol_mesh_right(weapon_type);
            SoundEffect(7, &lara_item->pos, 0);
        }
    }
    set_pistol_arm(&lara.right_arm, anir);

    if (anil == DRAW1_F && anir == DRAW1_F)
    {
        lara.gun_status = LG_ARMLESS;
        lara.right_arm.frame_number = lara.left_arm.frame_number = 0;
        lara.target = NULL;
        lara.left_arm.lock = lara.right_arm.lock = 0;
    }

    if (!(input & IN_LOOK))
    {
        lara.torso_y_rot = lara.head_y_rot = (lara.left_arm.y_rot + lara.right_arm.y_rot) / 4;
        lara.torso_x_rot = lara.head_x_rot = (lara.left_arm.x_rot + lara.right_arm.x_rot) / 4;
    }
}

void ready_pistols(int weapon_type)
{
    lara.gun_status = LG_READY;
    lara.left_arm.x_rot = lara.left_arm.y_rot = lara.left_arm.z_rot = 0;
    lara.right_arm.x_rot = lara.right_arm.y_rot = lara.right_arm.z_rot = 0;
    lara.left_arm.frame_number = lara.right_arm.frame_number = 0;
    lara.target = NULL;
    lara.left_arm.lock = lara.right_arm.lock = 0;

    lara.left_arm.frame_base = lara.right_arm.frame_base = objects[PISTOLS].frame_base;
}

void draw_pistol_meshes(int weapon_type)
{
    int obj;

    obj = WeaponObject(weapon_type);
    lara.mesh_ptrs[HAND_L] = meshes[objects[obj].mesh_index + HAND_L];
    lara.mesh_ptrs[HAND_R] = meshes[objects[obj].mesh_index + HAND_R];
    lara.mesh_ptrs[THIGH_L] = meshes[objects[LARA].mesh_index + THIGH_L];
    lara.mesh_ptrs[THIGH_R] = meshes[objects[LARA].mesh_index + THIGH_R];
}

void undraw_pistol_mesh_left(int weapon_type)
{
    int obj;

    obj = WeaponObject(weapon_type);
    lara.mesh_ptrs[THIGH_L] = meshes[objects[obj].mesh_index + THIGH_L];
    lara.mesh_ptrs[HAND_L] = meshes[objects[LARA].mesh_index + HAND_L];
}

void undraw_pistol_mesh_right(int weapon_type)
{
    int obj;

    obj = WeaponObject(weapon_type);
    lara.mesh_ptrs[THIGH_R] = meshes[objects[obj].mesh_index + THIGH_R];
    lara.mesh_ptrs[HAND_R] = meshes[objects[LARA].mesh_index + HAND_R];
}

typedef struct {
    PHD_ANGLE lock_angles[4];
    PHD_ANGLE left_angles[4];
    PHD_ANGLE right_angles[4];
    PHD_ANGLE aim_speed;
    PHD_ANGLE shot_accuracy;
    int gun_height;
    int damage;
    int target_dist;
    sint16 recoil_frame;
    sint16 flash_time;
    sint16 sample_num;
} WEAPON_INFO;

extern WEAPON_INFO weapons[];

void LaraTargetInfo(WEAPON_INFO *winfo);
void LaraGetNewTarget(WEAPON_INFO *winfo);
void AimWeapon(WEAPON_INFO *winfo, LARA_ARM *arm);
void AnimatePistols(int weapon_type);
void AddDynamicLight(sint32 x, sint32 y, sint32 z, sint32 intensity, sint32 falloff);

void PistolHandler(int weapon_type)
{
    WEAPON_INFO *winfo;

    winfo = &weapons[weapon_type];
    if (input & IN_ACTION)
        LaraTargetInfo(winfo);
    else
        lara.target = NULL;
    if (lara.target == NULL)
        LaraGetNewTarget(winfo);
    AimWeapon(winfo, &lara.left_arm);
    AimWeapon(winfo, &lara.right_arm);
    if (lara.left_arm.lock && !lara.right_arm.lock)
    {
        lara.head_y_rot = lara.torso_y_rot = lara.left_arm.y_rot / 2;
        lara.head_x_rot = lara.torso_x_rot = lara.left_arm.x_rot / 2;
    }
    else if (lara.right_arm.lock && !lara.left_arm.lock)
    {
        lara.head_y_rot = lara.torso_y_rot = lara.right_arm.y_rot / 2;
        lara.head_x_rot = lara.torso_x_rot = lara.right_arm.x_rot / 2;
    }
    else if (lara.left_arm.lock && lara.right_arm.lock)
    {
        lara.head_y_rot = lara.torso_y_rot = (lara.left_arm.y_rot + lara.right_arm.y_rot) / 4;
        lara.head_x_rot = lara.torso_x_rot = (lara.left_arm.x_rot + lara.right_arm.x_rot) / 4;
    }
    AnimatePistols(weapon_type);
    if (lara.left_arm.flash_gun || lara.right_arm.flash_gun)
    {
        AddDynamicLight(lara_item->pos.x_pos + (phd_sin(lara_item->pos.y_rot) >> (W2V_SHIFT - 10)),
            lara_item->pos.y_pos - WALL_L / 2,
            lara_item->pos.z_pos + (phd_cos(lara_item->pos.y_rot) >> (W2V_SHIFT - 10)),
            12, 11);
    }
}

int FireWeapon(int weapon_type, ITEM_INFO *target, ITEM_INFO *src, PHD_ANGLE *angles);

void AnimatePistols(int weapon_type)
{
    PHD_ANGLE angles[2];
    sint16 anil, anir, sound_already;
    WEAPON_INFO *winfo;
    static int uzi_left = 0, uzi_right = 0;

    winfo = &weapons[weapon_type];
    anir = lara.right_arm.frame_number;
    sound_already = 0;
    if (lara.right_arm.lock || ((input & IN_ACTION) && !lara.target))
    {
        if (anir >= 0 && anir < DRAW1_F - 1)
            anir++;
        else if (anir == DRAW1_F - 1)
        {
            if (input & IN_ACTION)
            {
                angles[0] = lara.right_arm.y_rot + lara_item->pos.y_rot;
                angles[1] = lara.right_arm.x_rot;
                if (FireWeapon(weapon_type, lara.target, lara_item, angles))
                {
                    lara.right_arm.flash_gun = winfo->flash_time;
                    SoundEffect(winfo->sample_num, &lara_item->pos, 0);
                    sound_already = 1;
                    if (weapon_type == LG_UZIS)
                        uzi_right = 1;
                }
                anir = RECOIL_F;
            }
            else if (uzi_right)
            {
                SoundEffect(winfo->sample_num + 1, &lara_item->pos, 0);
                uzi_right = 0;
            }
        }
        else if (anir >= RECOIL_F)
        {
            if (weapon_type == LG_UZIS)
            {
                SoundEffect(winfo->sample_num, &lara_item->pos, 0);
                uzi_right = 1;
            }
            anir++;
            if (anir == RECOIL_F + winfo->recoil_frame)
                anir = DRAW1_F - 1;
        }
    }
    else
    {
        if (anir >= RECOIL_F)
            anir = DRAW1_F - 1;
        else if (anir > 0 && anir <= DRAW1_F - 1)
            anir--;
        if (uzi_right)
        {
            SoundEffect(winfo->sample_num + 1, &lara_item->pos, 0);
            uzi_right = 0;
        }
    }
    set_pistol_arm(&lara.right_arm, anir);

    anil = lara.left_arm.frame_number;
    if (lara.left_arm.lock || ((input & IN_ACTION) && !lara.target))
    {
        if (anil >= 0 && anil < DRAW1_F - 1)
            anil++;
        else if (anil == DRAW1_F - 1)
        {
            if (input & IN_ACTION)
            {
                angles[0] = lara.left_arm.y_rot + lara_item->pos.y_rot;
                angles[1] = lara.left_arm.x_rot;
                if (FireWeapon(weapon_type, lara.target, lara_item, angles))
                {
                    lara.left_arm.flash_gun = winfo->flash_time;
                    if (!sound_already)
                        SoundEffect(winfo->sample_num, &lara_item->pos, 0);
                    if (weapon_type == LG_UZIS)
                        uzi_left = 1;
                }
                anil = RECOIL_F;
            }
            else if (uzi_left)
            {
                SoundEffect(winfo->sample_num + 1, &lara_item->pos, 0);
                uzi_left = 0;
            }
        }
        else if (anil >= RECOIL_F)
        {
            if (weapon_type == LG_UZIS)
            {
                SoundEffect(winfo->sample_num, &lara_item->pos, 0);
                uzi_left = 1;
            }
            anil++;
            if (anil == RECOIL_F + winfo->recoil_frame)
                anil = DRAW1_F - 1;
        }
    }
    else
    {
        if (anil >= RECOIL_F)
            anil = DRAW1_F - 1;
        else if (anil > 0 && anil <= DRAW1_F - 1)
            anil--;
        if (uzi_left)
        {
            SoundEffect(winfo->sample_num + 1, &lara_item->pos, 0);
            uzi_left = 0;
        }
    }
    set_pistol_arm(&lara.left_arm, anil);
}
