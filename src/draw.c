#include "game.h"
#include "larafire.h"

#define FLARE_FIRE 235
#define GUN_FLASH 240
#define M16_FLASH 241

void phd_RotYXZ(PHD_ANGLE ry, PHD_ANGLE rx, PHD_ANGLE rz);
int phd_TranslateRel(sint32 x, sint32 y, sint32 z);
void S_CalculateStaticLight(sint16 adder);

#define S_LongMemCpy(d, s, n) memcpy(d, s, (n) * sizeof(sint32))

extern int IM_rate, IM_frac;
extern sint32 *IMptr;
extern sint32 IMstack[];

extern int UnderwaterCamera;

void DrawRooms(sint16 current_room);
void S_OutputPolyList(void);
int S_DumpScreen(void);
void S_AnimateTextures(int nframes);

int DrawPhaseCinematic(void)
{
    UnderwaterCamera = 0;
    DrawRooms(camera.pos.room_number);
    S_OutputPolyList();
    camera.number_frames = S_DumpScreen();
    S_AnimateTextures(camera.number_frames);
    return camera.number_frames;
}

void DrawGameInfo(int timed);

int DrawPhaseGame(void)
{
    DrawRooms(camera.pos.room_number);
    DrawGameInfo(1);
    S_OutputPolyList();
    camera.number_frames = S_DumpScreen();
    S_AnimateTextures(camera.number_frames);
    return camera.number_frames;
}

#define UNDERWATER 1
#define INSIDE 64

extern int MidSort;
extern int OutsideCamera;

void S_LightRoom(ROOM_INFO *r);
void ClipRoom(ROOM_INFO *r);

#define OUTSIDE 8
#define HORIZON 254

extern int BoundStart, BoundEnd;
extern int BoundRooms[];
extern int OutsideLeft, OutsideRight, OutsideTop, OutsideBottom;

void GetRoomBounds(void);
void phd_RotYXZsuperpack(sint16 **pprot, int skip);
void S_InitialisePolyList(int clear);
void S_InsertBackground(sint16 *objptr);
void DrawLara(ITEM_INFO *item);
void PrintRooms(sint16 room_number);
void PrintObjects(sint16 room_number);

void DrawRooms(sint16 current_room)
{
    int i;
    ROOM_INFO *r;
    sint16 *frame;

    r = &room[current_room];
    phd_left = r->test_left = 0;
    phd_top = r->test_top = 0;
    phd_right = r->test_right = phd_winxmax;
    phd_bottom = r->test_bottom = phd_winymax;

    r->bound_active = 2;
    BoundRooms[0] = current_room;
    BoundStart = 0;
    BoundEnd = 1;
    number_draw_rooms = 0;

    OutsideCamera = r->flags & OUTSIDE;
    if (OutsideCamera) {
        OutsideLeft = OutsideTop = 0;
        OutsideRight = phd_winxmax;
        OutsideBottom = phd_winymax;
    } else {
        OutsideLeft = phd_winxmax;
        OutsideTop = phd_winymax;
        OutsideRight = OutsideBottom = 0;
    }
    UnderwaterCamera = r->flags & UNDERWATER;

    GetRoomBounds();

    MidSort = 0;
    if (OutsideCamera) {
        phd_left = OutsideLeft;
        phd_right = OutsideRight;
        phd_top = OutsideTop;
        phd_bottom = OutsideBottom;
        if (objects[HORIZON].loaded) {
            S_SetupAboveWater(UnderwaterCamera);
            phd_PushMatrix();
            *(phd_mxptr + M03) = *(phd_mxptr + M13) = *(phd_mxptr + M23) = 0;
            frame = anims[objects[HORIZON].anim_index].frame_ptr + 9;
            phd_RotYXZsuperpack(&frame, 0);
            S_InitialisePolyList(0);
            S_InsertBackground(meshes[objects[HORIZON].mesh_index]);
            phd_PopMatrix();
        } else {
            S_InitialisePolyList(1);
            OutsideCamera = -1;
        }
    } else
        S_InitialisePolyList(0);

    if (objects[LARA].loaded && !(lara_item->flags & ONESHOT)) {
        if (room[lara_item->room_number].flags & UNDERWATER)
            S_SetupBelowWater(UnderwaterCamera);
        else
            S_SetupAboveWater(UnderwaterCamera);
        MidSort = (sint16)room[lara_item->room_number].bound_active >> 8;
        if (MidSort)
            MidSort--;
        DrawLara(lara_item);
    }

    for (i = 0; i < number_draw_rooms; i++)
        PrintRooms(draw_rooms[i]);

    for (i = 0; i < number_draw_rooms; i++)
        PrintObjects(draw_rooms[i]);
}

void PrintRooms(sint16 room_number)
{
    ROOM_INFO *r;

    r = &room[room_number];
    if (r->flags & UNDERWATER)
        S_SetupBelowWater(UnderwaterCamera);
    else
        S_SetupAboveWater(UnderwaterCamera);

    MidSort = (sint16)r->bound_active >> 8;

    phd_TranslateAbs(r->x, r->y, r->z);
    phd_left = r->left;
    phd_right = r->right;
    phd_top = r->top;
    phd_bottom = r->bottom;

    S_LightRoom(r);
    if (OutsideCamera > 0 && !(r->flags & INSIDE)) {
        S_InsertRoom(r->data, 1);
    } else {
        if (OutsideCamera >= 0)
            ClipRoom(r);
        S_InsertRoom(r->data, 0);
    }
}

#define SPRITE_ABS 0x01000000
#define SPRITE_SEMITRANS 0x02000000
#define SPRITE_SHADE 0x08000000

extern sint32 LsAdder;

void S_CalculateStaticMeshLight(sint32 x, sint32 y, sint32 z, sint32 shade, sint32 shadeB, ROOM_INFO *r);
void S_DrawSprite(uint32 dwFlags, sint32 nX, sint32 nY, sint32 nZ, sint16 nSprite, sint16 nShade, sint16 nScale);

void DrawSpriteItem(ITEM_INFO *item)
{
    OBJECT_INFO *obj;

    S_CalculateStaticMeshLight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->shade, item->shadeB, &room[item->room_number]);

    obj = &objects[item->object_number];
    S_DrawSprite(SPRITE_ABS | (obj->semi_transparent ? SPRITE_SEMITRANS : 0) | SPRITE_SHADE,
                 item->pos.x_pos, item->pos.y_pos, item->pos.z_pos,
                 (sint16)(obj->mesh_index - item->frame_number),
                 (sint16)(LsAdder + 16 * 256), 0);
}

#define ROT_X 4
#define ROT_Y 8
#define ROT_Z 16

extern sint16 null_rotations[];

int GetFrames(ITEM_INFO *item, sint16 *frmptr[], int *rate);
void phd_RotZ(PHD_ANGLE rz);
void S_PrintShadow(sint16 size, sint16 *bptr, ITEM_INFO *iptr, int unknown);
int S_GetObjectBounds(sint16 *bptr);
void CalculateObjectLighting(ITEM_INFO *item, sint16 *frame);
void phd_PutPolygons_I(sint16 *ptr, int clip);
void phd_PopMatrix_I(void);
void phd_PushMatrix_I(void);
void phd_RotY_I(sint16 ang);
void phd_RotX_I(sint16 ang);
void phd_RotZ_I(sint16 ang);
void InitInterpolate(int frac, int rate);
void phd_TranslateRel_I(sint32 x, sint32 y, sint32 z);
void phd_TranslateRel_ID(sint32 x, sint32 y, sint32 z, sint32 x2, sint32 y2, sint32 z2);
void phd_RotYXZsuperpack_I(sint16 **pprot1, sint16 **pprot2, int skip);

void DrawAnimatingItem(ITEM_INFO *item)
{
    int clip, i, poppush;
    OBJECT_INFO *object;
    sint32 *bone;
    sint16 *rotation1, *rotation2;
    sint16 *extra_rotation;
    sint16 **meshpp;
    uint32 bit;
    sint16 *frmptr[2];
    int rate, frac;

    frac = GetFrames(item, frmptr, &rate);
    object = &objects[item->object_number];
    if (object->shadow_size)
        S_PrintShadow(object->shadow_size, frmptr[0], item, 0);

    phd_PushMatrix();
    phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);

    if ((clip = S_GetObjectBounds(frmptr[0])))
    {
        CalculateObjectLighting(item, frmptr[0]);
        if (item->data)
            extra_rotation = (sint16 *)item->data;
        else
            extra_rotation = null_rotations;

        meshpp = &meshes[object->mesh_index];
        bone = bones + object->bone_index;
        bit = 1;

        if (!frac)
        {
            phd_TranslateRel((sint32)*(frmptr[0] + 6), (sint32)*(frmptr[0] + 7), (sint32)*(frmptr[0] + 8));
            rotation1 = frmptr[0] + 9;
            phd_RotYXZsuperpack(&rotation1, 0);
            if (bit & item->mesh_bits)
                phd_PutPolygons(*meshpp, clip);
            meshpp++;

            for (i = object->nmeshes - 1; i > 0; i--, bone += 4, meshpp++)
            {
                poppush = *bone;
                if (poppush & 1)
                    phd_PopMatrix();
                if (poppush & 2)
                    phd_PushMatrix();
                phd_TranslateRel(*(bone + 1), *(bone + 2), *(bone + 3));
                phd_RotYXZsuperpack(&rotation1, 0);
                if (poppush & (ROT_X | ROT_Y | ROT_Z))
                {
                    if (poppush & ROT_Y)
                        phd_RotY(*(extra_rotation++));
                    if (poppush & ROT_X)
                        phd_RotX(*(extra_rotation++));
                    if (poppush & ROT_Z)
                        phd_RotZ(*(extra_rotation++));
                }
                bit <<= 1;
                if (bit & item->mesh_bits)
                    phd_PutPolygons(*meshpp, clip);
            }
        }
        else
        {
            InitInterpolate(frac, rate);
            phd_TranslateRel_ID((sint32)*(frmptr[0] + 6), (sint32)*(frmptr[0] + 7), (sint32)*(frmptr[0] + 8), (sint32)*(frmptr[1] + 6), (sint32)*(frmptr[1] + 7), (sint32)*(frmptr[1] + 8));
            rotation1 = frmptr[0] + 9;
            rotation2 = frmptr[1] + 9;
            phd_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
            if (bit & item->mesh_bits)
                phd_PutPolygons_I(*meshpp, clip);
            meshpp++;

            for (i = object->nmeshes - 1; i > 0; i--, bone += 4, meshpp++)
            {
                poppush = *bone;
                if (poppush & 1)
                    phd_PopMatrix_I();
                if (poppush & 2)
                    phd_PushMatrix_I();
                phd_TranslateRel_I(*(bone + 1), *(bone + 2), *(bone + 3));
                phd_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
                if (poppush & (ROT_X | ROT_Y | ROT_Z))
                {
                    if (poppush & ROT_Y)
                        phd_RotY_I(*(extra_rotation++));
                    if (poppush & ROT_X)
                        phd_RotX_I(*(extra_rotation++));
                    if (poppush & ROT_Z)
                        phd_RotZ_I(*(extra_rotation++));
                }
                bit <<= 1;
                if (bit & item->mesh_bits)
                    phd_PutPolygons_I(*meshpp, clip);
            }
        }
    }
    phd_PopMatrix();
}

void InitInterpolate(int frac, int rate)
{
    IM_frac = frac;
    IM_rate = rate;

    IMptr = IMstack;
    S_LongMemCpy(IMstack, phd_mxptr, 12);
}

void phd_PopMatrix_I(void)
{
    phd_PopMatrix();
    IMptr -= 12;
}

void phd_PushMatrix_I(void)
{
    phd_PushMatrix();
    S_LongMemCpy(IMptr + 12, IMptr, 12);
    IMptr += 12;
}

void phd_RotY_I(sint16 ang)
{
    sint32 *oldm;

    phd_RotY(ang);
    oldm = phd_mxptr;
    phd_mxptr = IMptr;
    phd_RotY(ang);
    phd_mxptr = oldm;
}

void phd_RotX_I(sint16 ang)
{
    sint32 *oldm;

    phd_RotX(ang);
    oldm = phd_mxptr;
    phd_mxptr = IMptr;
    phd_RotX(ang);
    phd_mxptr = oldm;
}

void phd_RotZ_I(sint16 ang)
{
    sint32 *oldm;

    phd_RotZ(ang);
    oldm = phd_mxptr;
    phd_mxptr = IMptr;
    phd_RotZ(ang);
    phd_mxptr = oldm;
}

void phd_TranslateRel_I(sint32 x, sint32 y, sint32 z)
{
    sint32 *oldm;

    phd_TranslateRel(x, y, z);
    oldm = phd_mxptr;
    phd_mxptr = IMptr;
    phd_TranslateRel(x, y, z);
    phd_mxptr = oldm;
}

void phd_TranslateRel_ID(sint32 x, sint32 y, sint32 z, sint32 x2, sint32 y2, sint32 z2)
{
    sint32 *oldm;

    phd_TranslateRel(x, y, z);
    oldm = phd_mxptr;
    phd_mxptr = IMptr;
    phd_TranslateRel(x2, y2, z2);
    phd_mxptr = oldm;
}

void phd_RotYXZ_I(sint16 y, sint16 x, sint16 z)
{
    sint32 *oldm;

    phd_RotYXZ(y, x, z);
    oldm = phd_mxptr;
    phd_mxptr = IMptr;
    phd_RotYXZ(y, x, z);
    phd_mxptr = oldm;
}

void phd_RotYXZsuperpack(sint16 **pprot, int skip);

void phd_RotYXZsuperpack_I(sint16 **pprot1, sint16 **pprot2, int skip)
{
    sint32 *oldm;

    phd_RotYXZsuperpack(pprot1, skip);
    oldm = phd_mxptr;
    phd_mxptr = IMptr;
    phd_RotYXZsuperpack(pprot2, skip);
    phd_mxptr = oldm;
}

void InterpolateMatrix(void);

void phd_PutPolygons_I(sint16 *ptr, int clip)
{
    phd_PushMatrix();
    InterpolateMatrix();
    phd_PutPolygons(ptr, clip);
    phd_PopMatrix();
}

void InterpolateMatrix(void)
{
    sint32 frac, rate;
    sint32 *mptr, *iptr;

    mptr = phd_mxptr;
    iptr = IMptr;
    frac = IM_frac;
    rate = IM_rate;

    if (rate == 2)
    {
        *(mptr + M00) = (*(iptr + M00) + *(mptr + M00)) >> 1;
        *(mptr + M01) = (*(iptr + M01) + *(mptr + M01)) >> 1;
        *(mptr + M02) = (*(iptr + M02) + *(mptr + M02)) >> 1;
        *(mptr + M03) = (*(iptr + M03) + *(mptr + M03)) >> 1;
        *(mptr + M10) = (*(iptr + M10) + *(mptr + M10)) >> 1;
        *(mptr + M11) = (*(iptr + M11) + *(mptr + M11)) >> 1;
        *(mptr + M12) = (*(iptr + M12) + *(mptr + M12)) >> 1;
        *(mptr + M13) = (*(iptr + M13) + *(mptr + M13)) >> 1;
        *(mptr + M20) = (*(iptr + M20) + *(mptr + M20)) >> 1;
        *(mptr + M21) = (*(iptr + M21) + *(mptr + M21)) >> 1;
        *(mptr + M22) = (*(iptr + M22) + *(mptr + M22)) >> 1;
        *(mptr + M23) = (*(iptr + M23) + *(mptr + M23)) >> 1;
    }
    else
    {
        *(mptr + M00) += ((*(iptr + M00) - *(mptr + M00)) * frac) / rate;
        *(mptr + M01) += ((*(iptr + M01) - *(mptr + M01)) * frac) / rate;
        *(mptr + M02) += ((*(iptr + M02) - *(mptr + M02)) * frac) / rate;
        *(mptr + M03) += ((*(iptr + M03) - *(mptr + M03)) * frac) / rate;
        *(mptr + M10) += ((*(iptr + M10) - *(mptr + M10)) * frac) / rate;
        *(mptr + M11) += ((*(iptr + M11) - *(mptr + M11)) * frac) / rate;
        *(mptr + M12) += ((*(iptr + M12) - *(mptr + M12)) * frac) / rate;
        *(mptr + M13) += ((*(iptr + M13) - *(mptr + M13)) * frac) / rate;
        *(mptr + M20) += ((*(iptr + M20) - *(mptr + M20)) * frac) / rate;
        *(mptr + M21) += ((*(iptr + M21) - *(mptr + M21)) * frac) / rate;
        *(mptr + M22) += ((*(iptr + M22) - *(mptr + M22)) * frac) / rate;
        *(mptr + M23) += ((*(iptr + M23) - *(mptr + M23)) * frac) / rate;
    }
}

void InterpolateArmMatrix(void)
{
    sint32 *mptr, *iptr;
    sint32 frac, rate;

    mptr = phd_mxptr;
    iptr = IMptr;
    frac = IM_frac;
    rate = IM_rate;

    if (rate == 2)
    {
        *(mptr + M00) = *(mptr + M00 - 24);
        *(mptr + M01) = *(mptr + M01 - 24);
        *(mptr + M02) = *(mptr + M02 - 24);
        *(mptr + M03) = (*(iptr + M03) + *(mptr + M03)) / 2;
        *(mptr + M10) = *(mptr + M10 - 24);
        *(mptr + M11) = *(mptr + M11 - 24);
        *(mptr + M12) = *(mptr + M12 - 24);
        *(mptr + M13) = (*(iptr + M13) + *(mptr + M13)) / 2;
        *(mptr + M20) = *(mptr + M20 - 24);
        *(mptr + M21) = *(mptr + M21 - 24);
        *(mptr + M22) = *(mptr + M22 - 24);
        *(mptr + M23) = (*(iptr + M23) + *(mptr + M23)) / 2;
    }
    else
    {
        *(mptr + M00) = *(mptr + M00 - 24);
        *(mptr + M01) = *(mptr + M01 - 24);
        *(mptr + M02) = *(mptr + M02 - 24);
        *(mptr + M03) += ((*(iptr + M03) - *(mptr + M03)) * frac) / rate;
        *(mptr + M10) = *(mptr + M10 - 24);
        *(mptr + M11) = *(mptr + M11 - 24);
        *(mptr + M12) = *(mptr + M12 - 24);
        *(mptr + M13) += ((*(iptr + M13) - *(mptr + M13)) * frac) / rate;
        *(mptr + M20) = *(mptr + M20 - 24);
        *(mptr + M21) = *(mptr + M21 - 24);
        *(mptr + M22) = *(mptr + M22 - 24);
        *(mptr + M23) += ((*(iptr + M23) - *(mptr + M23)) * frac) / rate;
    }
}

void DrawGunFlash(int weapon_type, int clip)
{
    sint16 light;
    int g_len, nZOff;

    switch (weapon_type)
    {
    case LG_FLARE:
        phd_TranslateRel(11, 32, 80);
        phd_RotX(-90 * ONE_DEGREE);
        phd_RotY((sint16)(GetRandomDraw() * 2));
        S_CalculateStaticLight(2048);
        phd_PutPolygons(meshes[objects[FLARE_FIRE].mesh_index], clip);
        return;

    case LG_MAGNUMS:
        light = 4096;
        g_len = 215;
        nZOff = 65;
        break;

    case LG_UZIS:
        light = 2560;
        g_len = 200;
        nZOff = 50;
        break;

    case LG_SHOTGUN:
        return;

    case LG_M16:
        phd_TranslateRel(0, 400, 99);
        phd_RotYXZ(0, -85 * ONE_DEGREE, (sint16)((GetRandomDraw() * 2 & 0x4000) + 0x2000));
        S_CalculateStaticLight(2560);
        phd_PutPolygons(meshes[objects[M16_FLASH].mesh_index], clip);
        return;

    default:
        light = 5120;
        g_len = 185;
        nZOff = 40;
        break;
    }

    phd_TranslateRel(0, g_len, nZOff);
    phd_RotYXZ(0, -90 * ONE_DEGREE, (sint16)(GetRandomDraw() * 2));
    S_CalculateStaticLight(light);
    phd_PutPolygons(meshes[objects[GUN_FLASH].mesh_index], clip);
}

void __cdecl phd_PushUnitMatrix(void);
void S_CalculateLight(sint32 x, sint32 y, sint32 z, sint16 room_number);
void CalculateObjectLighting(ITEM_INFO *item, sint16 *frame);

void CalculateObjectLighting(ITEM_INFO *item, sint16 *frame)
{
    sint32 *mptr;
    sint32 x, y, z;

    if (item->shade < 0)
    {
        phd_PushUnitMatrix();
        mptr = phd_mxptr;
        *(mptr + M03) = *(mptr + M13) = *(mptr + M23) = 0;
        phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);
        phd_TranslateRel((*(frame + 0) + *(frame + 1)) >> 1, (*(frame + 2) + *(frame + 3)) >> 1, (*(frame + 4) + *(frame + 5)) >> 1);
        x = (*(phd_mxptr + M03) >> W2V_SHIFT) + item->pos.x_pos;
        y = (*(phd_mxptr + M13) >> W2V_SHIFT) + item->pos.y_pos;
        z = (*(phd_mxptr + M23) >> W2V_SHIFT) + item->pos.z_pos;
        phd_PopMatrix();
        S_CalculateLight(x, y, z, item->room_number);
    }
    else
        S_CalculateStaticMeshLight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->shade, item->shadeB, &room[item->room_number]);
}

int GetFrames(ITEM_INFO *item, sint16 *frmptr[], int *rate)
{
    ANIM_STRUCT *anim;
    int frm;
    int first, second;
    int frame_size;
    int interp, rat;

    frm = item->frame_number;
    anim = &anims[item->anim_number];
    frmptr[0] = frmptr[1] = anim->frame_ptr;
    rat = *rate = anim->interpolation & 0x00ff;
    frame_size = anim->interpolation >> 8;
    frm -= anim->frame_base;
    first = frm / rat;
    interp = frm % rat;
    frmptr[0] += first * frame_size;
    frmptr[1] = frmptr[0] + frame_size;
    if (interp == 0)
        return 0;
    second = first * rat + rat;
    if (second > anim->frame_end)
        *rate = anim->frame_end - (second - rat);
    return interp;
}

extern sint16 interpolated_bounds[6];

sint16 *GetBoundsAccurate(ITEM_INFO *item)
{
    int i, frac, rate;
    sint16 *bptr, *frmptr[2];

    if ((frac = GetFrames(item, frmptr, &rate)) == 0)
        return frmptr[0];
    else
    {
        bptr = interpolated_bounds;

        for (i = 0; i < 6; i++, bptr++, frmptr[0]++, frmptr[1]++)
            *(bptr) = *(frmptr[0]) + ((*(frmptr[1]) - *(frmptr[0])) * frac) / rate;

        return interpolated_bounds;
    }
}

sint16 *GetBestFrame(ITEM_INFO *item)
{
    int frac, rate;
    sint16 *frmptr[2];

    frac = GetFrames(item, frmptr, &rate);

    if (frac <= rate / 2)
        return frmptr[0];
    else
        return frmptr[1];
}

#define MAX_DYNAMICS 10

typedef struct {
    sint32 x, y, z;
    sint16 intensity, intensity2;
    sint32 falloff, falloff2;
} LIGHT_INFO;

extern sint32 number_dynamics;
extern LIGHT_INFO dynamic[MAX_DYNAMICS];

void AddDynamicLight(sint32 x, sint32 y, sint32 z, sint32 intensity, sint32 falloff)
{
    LIGHT_INFO *dl;

    if (number_dynamics >= MAX_DYNAMICS)
        dl = dynamic;
    else
        dl = &dynamic[number_dynamics++];

    dl->x = x;
    dl->y = y;
    dl->z = z;
    dl->intensity = (sint16)intensity;
    dl->falloff = falloff;
}
