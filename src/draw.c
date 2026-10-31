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
void S_SetupBelowWater(int underwater);
void S_SetupAboveWater(int underwater);
void ClipRoom(ROOM_INFO *r);

#define OUTSIDE 8
#define HORIZON 254

extern int BoundStart, BoundEnd;
extern int BoundRooms[];
extern int OutsideLeft, OutsideRight, OutsideTop, OutsideBottom;

void GetRoomBounds(void);
void SetRoomBounds(sint16 *objptr, int room_number, ROOM_INFO *parent);
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

struct door_vbuf {
    sint32 xv, yv, zv;
};

extern struct door_vbuf vbufdoor[4];
extern sint32 PhdPersp;

void SetRoomBounds(sint16 *objptr, int room_number, ROOM_INFO *parent)
{
    int i, z_behind, z_toofar;
    sint32 left, right, top, bottom;
    sint32 xv, yv, zv, xs, ys;
    sint32 *mptr;
    ROOM_INFO *r;
    struct door_vbuf *dest, *last;

    r = &room[room_number];
    if (r->left <= parent->test_left && r->right >= parent->test_right && r->top <= parent->test_top && r->bottom >= parent->test_bottom)
        return;

    left = parent->test_right;
    right = parent->test_left;
    top = parent->test_bottom;
    bottom = parent->test_top;

    objptr += 3;
    mptr = phd_mxptr;
    dest = vbufdoor;
    z_behind = z_toofar = 0;

    for (i = 0; i < 4; i++, dest++, objptr += 3) {
        dest->xv = xv = *(mptr + M00) * *(objptr) + *(mptr + M01) * *(objptr + 1) + *(mptr + M02) * *(objptr + 2) + *(mptr + M03);
        dest->yv = yv = *(mptr + M10) * *(objptr) + *(mptr + M11) * *(objptr + 1) + *(mptr + M12) * *(objptr + 2) + *(mptr + M13);
        dest->zv = zv = *(mptr + M20) * *(objptr) + *(mptr + M21) * *(objptr + 1) + *(mptr + M22) * *(objptr + 2) + *(mptr + M23);

        if (zv > 0) {
            if (zv > phd_zfar)
                z_toofar++;

            zv /= PhdPersp;
            if (zv) {
                xs = xv / zv + phd_centerx;
                ys = yv / zv + phd_centery;
            } else {
                xs = (xv >= 0) ? phd_right : phd_left;
                ys = (yv >= 0) ? phd_bottom : phd_top;
            }

            if (xs - 1 < left)
                left = xs - 1;
            if (xs + 1 > right)
                right = xs + 1;
            if (ys - 1 < top)
                top = ys - 1;
            if (ys + 1 > bottom)
                bottom = ys + 1;
        } else
            z_behind++;
    }

    if (z_behind == 4 || z_toofar == 4)
        return;

    if (z_behind > 0) {
        dest = vbufdoor;
        last = dest + 3;
        for (i = 0; i < 4; i++, last = dest, dest++) {
            if ((dest->zv < 0) ^ (last->zv < 0)) {
                if (dest->xv < 0 && last->xv < 0)
                    left = 0;
                else if (dest->xv > 0 && last->xv > 0)
                    right = phd_winxmax;
                else {
                    left = 0;
                    right = phd_winxmax;
                }

                if (dest->yv < 0 && last->yv < 0)
                    top = 0;
                else if (dest->yv > 0 && last->yv > 0)
                    bottom = phd_winymax;
                else {
                    top = 0;
                    bottom = phd_winymax;
                }
            }
        }
    }

    if (left < parent->test_left)
        left = parent->test_left;
    if (right > parent->test_right)
        right = parent->test_right;
    if (top < parent->test_top)
        top = parent->test_top;
    if (bottom > parent->test_bottom)
        bottom = parent->test_bottom;

    if (left >= right || top >= bottom)
        return;

    if (r->bound_active & 2) {
        if (left < r->test_left)
            r->test_left = (sint16)left;
        if (top < r->test_top)
            r->test_top = (sint16)top;
        if (right > r->test_right)
            r->test_right = (sint16)right;
        if (bottom > r->test_bottom)
            r->test_bottom = (sint16)bottom;
    } else {
        BoundRooms[(BoundEnd++) % 128] = room_number;
        r->bound_active |= 2;
        r->bound_active += (sint16)(MidSort << 8);
        r->test_left = (sint16)left;
        r->test_right = (sint16)right;
        r->test_top = (sint16)top;
        r->test_bottom = (sint16)bottom;
    }
}

void S_InsertBackPolygon(sint32 x1, sint32 y1, sint32 x2, sint32 y2, sint32 colour);

extern sint32 Var464180[12][2];

void ClipRoom(ROOM_INFO *r)
{
    sint32 ymax, ymin, xmin, xmax;
    sint32 y, z, *mptr, x;
    sint32 zv[8], yv[8], xv[8], clip[8];
    sint32 zdiv, znom;
    int i, p1, clip_room, p2;

    xv[0] = xv[3] = xv[4] = xv[7] = 1024;
    yv[0] = yv[1] = yv[2] = yv[3] = r->maxceiling - r->y;
    zv[0] = zv[1] = zv[4] = zv[5] = 1024;
    xv[1] = xv[2] = xv[5] = xv[6] = (r->y_size << 10) - 1024;
    yv[4] = yv[5] = yv[6] = yv[7] = r->minfloor - r->y;
    zv[2] = zv[3] = zv[6] = zv[7] = (r->x_size << 10) - 1024;

    mptr = phd_mxptr;
    clip_room = 0;
    for (i = 0; i < 8; i++) {
        x = xv[i];
        y = yv[i];
        z = zv[i];
        xv[i] = *(mptr + M00) * x + *(mptr + M01) * y + *(mptr + M02) * z + *(mptr + M03);
        yv[i] = *(mptr + M10) * x + *(mptr + M11) * y + *(mptr + M12) * z + *(mptr + M13);
        zv[i] = *(mptr + M20) * x + *(mptr + M21) * y + *(mptr + M22) * z + *(mptr + M23);
        if (zv[i] > phd_zfar)
            clip[i] = clip_room = 1;
        else
            clip[i] = 0;
    }

    if (!clip_room)
        return;

    xmin = ymin = 0x10000000;
    xmax = ymax = -0x10000000;
    for (i = 0; i < 12; i++) {
        p1 = Var464180[i][0];
        p2 = Var464180[i][1];
        if (clip[p1] ^ clip[p2]) {
            zdiv = (zv[p2] - zv[p1]) >> W2V_SHIFT;
            if (zdiv) {
                znom = (phd_zfar - zv[p1]) >> W2V_SHIFT;
                x = xv[p1] + ((((xv[p2] - xv[p1]) >> W2V_SHIFT) * znom / zdiv) << W2V_SHIFT);
                y = yv[p1] + ((((yv[p2] - yv[p1]) >> W2V_SHIFT) * znom / zdiv) << W2V_SHIFT);
                if (x < xmin)
                    xmin = x;
                if (x > xmax)
                    xmax = x;
                if (y < ymin)
                    ymin = y;
                if (y > ymax)
                    ymax = y;
            } else {
                if (xv[p1] < xmin)
                    xmin = xv[p1];
                if (xv[p2] < xmin)
                    xmin = xv[p2];
                if (xv[p1] > xmax)
                    xmax = xv[p1];
                if (xv[p2] > xmax)
                    xmax = xv[p2];
                if (yv[p1] < ymin)
                    ymin = yv[p1];
                if (yv[p2] < ymin)
                    ymin = yv[p2];
                if (yv[p1] > ymax)
                    ymax = yv[p1];
                if (yv[p2] > ymax)
                    ymax = yv[p2];
            }
        }
    }

    xmin = phd_centerx + xmin / (phd_zfar / PhdPersp);
    ymin = phd_centery + ymin / (phd_zfar / PhdPersp);
    xmax = phd_centerx + xmax / (phd_zfar / PhdPersp);
    ymax = phd_centery + ymax / (phd_zfar / PhdPersp);
    if (xmin > phd_right || ymin > phd_bottom || xmax < phd_left || ymax < phd_top)
        return;

    if (xmin < phd_left)
        xmin = phd_left;
    if (ymin < phd_top)
        ymin = phd_top;
    if (xmax > phd_right)
        xmax = phd_right;
    if (ymax > phd_bottom)
        ymax = phd_bottom;
    S_InsertBackPolygon(xmin, ymin, xmax, ymax, 0);
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

typedef struct {
    sint32 x, y, z;
    PHD_ANGLE y_rot;
    sint16 shade;
    sint16 shadeB;
    sint16 static_number;
} MESH_INFO;

typedef struct {
    sint16 mesh_number;
    uint16 flags;
    sint16 x_minp, x_maxp, y_minp, y_maxp, z_minp, z_maxp;
    sint16 x_minc, x_maxc, y_minc, y_maxc, z_minc, z_maxc;
} STATIC_INFO;

#define STATIC_DRAW 2

extern STATIC_INFO StaticObjects[];

int S_GetObjectBounds(sint16 *bptr);
void S_CalculateStaticMeshLight(sint32 x, sint32 y, sint32 z, sint32 shade, sint32 shadeB, ROOM_INFO *r);
void DrawEffect(sint16 fx_num);

void PrintObjects(sint16 room_number)
{
    ROOM_INFO *r;
    MESH_INFO *mesh;
    ITEM_INFO *item;
    int i;
    sint16 clip;
    sint16 item_number, fx_number;

    r = &room[room_number];
    if (r->flags & UNDERWATER)
        S_SetupBelowWater(UnderwaterCamera);
    else
        S_SetupAboveWater(UnderwaterCamera);

    MidSort = (sint16)r->bound_active >> 8;
    r->bound_active = 0;

    phd_PushMatrix();
    phd_TranslateAbs(r->x, r->y, r->z);

    phd_left = r->left;
    phd_right = r->right;
    phd_top = r->top;
    phd_bottom = r->bottom;

    for (i = 0, mesh = (MESH_INFO *)r->mesh; i < r->num_meshes; i++, mesh++) {
        if (StaticObjects[mesh->static_number].flags & STATIC_DRAW) {
            phd_PushMatrix();
            phd_TranslateAbs(mesh->x, mesh->y, mesh->z);
            phd_RotY(mesh->y_rot);
            clip = S_GetObjectBounds(&StaticObjects[mesh->static_number].x_minp);
            if (clip) {
                S_CalculateStaticMeshLight(mesh->x, mesh->y, mesh->z, mesh->shade, mesh->shadeB, r);
                phd_PutPolygons(meshes[StaticObjects[mesh->static_number].mesh_number], clip);
            }
            phd_PopMatrix();
        }
    }

    phd_left = 0;
    phd_top = 0;
    phd_right = phd_winxmax + 1;
    phd_bottom = phd_winymax + 1;

    for (item_number = r->item_number; item_number != NO_ITEM; item_number = item->next_item) {
        item = &items[item_number];
        if (item->status != INVISIBLE)
            (*objects[item->object_number].draw_routine)(item);
    }

    for (fx_number = r->fx_number; fx_number != NO_ITEM; fx_number = effects[fx_number].next_fx)
        DrawEffect(fx_number);

    phd_PopMatrix();

    r->left = phd_winxmax;
    r->bottom = 0;
    r->right = 0;
    r->top = phd_winymax;
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

#define SPAZ_FORWARD_A 125
#define SPAZ_BACK_A 126
#define SPAZ_RIGHT_A 127
#define SPAZ_LEFT_A 128

void DrawLaraInt(ITEM_INFO *item, sint16 *frame1, sint16 *frame2, int frac, int rate);
void DrawGunFlash(int weapon_type, int clip);

void DrawLara(ITEM_INFO *item)
{
    OBJECT_INFO *object;
    sint32 *bone, *bonew;
    sint16 *rotation, *rotationw;
    sint16 *frame;
    int clip, fire_arms;
    sint32 savemox[12];
    sint16 *frmptr[2];
    int frac, rate, size;
    int left, right, top, bottom;

    left = phd_left;
    right = phd_right;
    top = phd_top;
    bottom = phd_bottom;
    phd_left = phd_top = 0;
    phd_bottom = phd_winymax;
    phd_right = phd_winxmax;

    if (lara.hit_direction < 0 && (frac = GetFrames(item, frmptr, &rate)))
    {
        DrawLaraInt(item, frmptr[0], frmptr[1], frac, rate);
        phd_left = left;
        phd_right = right;
        phd_top = top;
        phd_bottom = bottom;
        return;
    }

    object = &objects[item->object_number];
    if (lara.hit_direction >= 0)
    {
        switch (lara.hit_direction)
        {
        default:
        case 0:
            frame = anims[SPAZ_FORWARD_A].frame_ptr;
            size = anims[SPAZ_FORWARD_A].interpolation >> 8;
            break;
        case 1:
            frame = anims[SPAZ_RIGHT_A].frame_ptr;
            size = anims[SPAZ_RIGHT_A].interpolation >> 8;
            break;
        case 2:
            frame = anims[SPAZ_BACK_A].frame_ptr;
            size = anims[SPAZ_BACK_A].interpolation >> 8;
            break;
        case 3:
            frame = anims[SPAZ_LEFT_A].frame_ptr;
            size = anims[SPAZ_LEFT_A].interpolation >> 8;
            break;
        }
        frame += (int)(lara.hit_frame * size);
    }
    else
        frame = frmptr[0];

    if (lara.skidoo == NO_ITEM)
        S_PrintShadow(object->shadow_size, frame, item, 0);

    S_LongMemCpy(savemox, phd_mxptr, 12);

    phd_PushMatrix();
    phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);

    if (!(clip = S_GetObjectBounds(frame)))
    {
        phd_PopMatrix();
        return;
    }

    phd_PushMatrix();
    CalculateObjectLighting(item, frame);

    bone = bones + object->bone_index;
    rotation = frame + 9;

    phd_TranslateRel((sint32)*(frame + 6), (sint32)*(frame + 7), (sint32)*(frame + 8));
    phd_RotYXZsuperpack(&rotation, 0);
    phd_PutPolygons(lara.mesh_ptrs[HIPS], clip);

    phd_PushMatrix();
    phd_TranslateRel(*(bone + 1), *(bone + 2), *(bone + 3));
    phd_RotYXZsuperpack(&rotation, 0);
    phd_PutPolygons(lara.mesh_ptrs[THIGH_L], clip);

    phd_TranslateRel(*(bone + 1 + 4), *(bone + 2 + 4), *(bone + 3 + 4));
    phd_RotYXZsuperpack(&rotation, 0);
    phd_PutPolygons(lara.mesh_ptrs[CALF_L], clip);

    phd_TranslateRel(*(bone + 1 + 8), *(bone + 2 + 8), *(bone + 3 + 8));
    phd_RotYXZsuperpack(&rotation, 0);
    phd_PutPolygons(lara.mesh_ptrs[FOOT_L], clip);
    phd_PopMatrix();

    phd_PushMatrix();
    phd_TranslateRel(*(bone + 1 + 12), *(bone + 2 + 12), *(bone + 3 + 12));
    phd_RotYXZsuperpack(&rotation, 0);
    phd_PutPolygons(lara.mesh_ptrs[THIGH_R], clip);

    phd_TranslateRel(*(bone + 1 + 16), *(bone + 2 + 16), *(bone + 3 + 16));
    phd_RotYXZsuperpack(&rotation, 0);
    phd_PutPolygons(lara.mesh_ptrs[CALF_R], clip);

    phd_TranslateRel(*(bone + 1 + 20), *(bone + 2 + 20), *(bone + 3 + 20));
    phd_RotYXZsuperpack(&rotation, 0);
    phd_PutPolygons(lara.mesh_ptrs[FOOT_R], clip);
    phd_PopMatrix();

    phd_TranslateRel(*(bone + 1 + 24), *(bone + 2 + 24), *(bone + 3 + 24));
    if (lara.weapon_item != NO_ITEM && lara.gun_type == LG_M16
        && (items[lara.weapon_item].current_anim_state == 0 || items[lara.weapon_item].current_anim_state == 2
            || items[lara.weapon_item].current_anim_state == 4))
    {
        rotation = lara.right_arm.frame_base + lara.right_arm.frame_number * (anims[lara.right_arm.anim_number].interpolation >> 8) + 9;
        phd_RotYXZsuperpack(&rotation, 7);
    }
    else
        phd_RotYXZsuperpack(&rotation, 0);
    phd_RotYXZ(lara.torso_y_rot, lara.torso_x_rot, lara.torso_z_rot);
    phd_PutPolygons(lara.mesh_ptrs[TORSO], clip);

    phd_PushMatrix();
    phd_TranslateRel(*(bone + 1 + 52), *(bone + 2 + 52), *(bone + 3 + 52));
    rotationw = rotation;
    phd_RotYXZsuperpack(&rotation, 6);
    rotation = rotationw;
    phd_RotYXZ(lara.head_y_rot, lara.head_x_rot, lara.head_z_rot);
    phd_PutPolygons(lara.mesh_ptrs[HEAD], clip);

    S_LongMemCpy(phd_mxptr, savemox, 12);
    DrawHair();
    phd_PopMatrix();

    if (lara.back_gun)
    {
        phd_PushMatrix();
        bonew = bones + objects[lara.back_gun].bone_index;
        phd_TranslateRel(*(bonew + 1 + 52), *(bonew + 2 + 52), *(bonew + 3 + 52));
        rotationw = objects[lara.back_gun].frame_base + 9;
        phd_RotYXZsuperpack(&rotationw, 14);
        phd_PutPolygons(meshes[objects[lara.back_gun].mesh_index + HEAD], clip);
        phd_PopMatrix();
    }

    fire_arms = LG_UNARMED;
    if (lara.gun_status == LG_READY || lara.gun_status == LG_SPECIAL || lara.gun_status == LG_DRAW || lara.gun_status == LG_UNDRAW)
        fire_arms = lara.gun_type;

    switch (fire_arms)
    {
    case LG_UNARMED:
    case LG_FLARE:
        phd_PushMatrix();
        phd_TranslateRel(*(bone + 1 + 28), *(bone + 2 + 28), *(bone + 3 + 28));
        phd_RotYXZsuperpack(&rotation, 0);
        phd_PutPolygons(lara.mesh_ptrs[UARM_R], clip);

        phd_TranslateRel(*(bone + 1 + 32), *(bone + 2 + 32), *(bone + 3 + 32));
        phd_RotYXZsuperpack(&rotation, 0);
        phd_PutPolygons(lara.mesh_ptrs[LARM_R], clip);

        phd_TranslateRel(*(bone + 1 + 36), *(bone + 2 + 36), *(bone + 3 + 36));
        phd_RotYXZsuperpack(&rotation, 0);
        phd_PutPolygons(lara.mesh_ptrs[HAND_R], clip);
        phd_PopMatrix();

        phd_PushMatrix();
        phd_TranslateRel(*(bone + 1 + 40), *(bone + 2 + 40), *(bone + 3 + 40));
        if (lara.flare_control_left)
        {
            rotation = lara.left_arm.frame_base + (anims[lara.left_arm.anim_number].interpolation >> 8) * (lara.left_arm.frame_number - anims[lara.left_arm.anim_number].frame_base) + 9;
            phd_RotYXZsuperpack(&rotation, 11);
        }
        else
            phd_RotYXZsuperpack(&rotation, 0);
        phd_PutPolygons(lara.mesh_ptrs[UARM_L], clip);

        phd_TranslateRel(*(bone + 1 + 44), *(bone + 2 + 44), *(bone + 3 + 44));
        phd_RotYXZsuperpack(&rotation, 0);
        phd_PutPolygons(lara.mesh_ptrs[LARM_L], clip);

        phd_TranslateRel(*(bone + 1 + 48), *(bone + 2 + 48), *(bone + 3 + 48));
        phd_RotYXZsuperpack(&rotation, 0);
        phd_PutPolygons(lara.mesh_ptrs[HAND_L], clip);

        if (lara.gun_type == LG_FLARE && lara.left_arm.flash_gun)
            DrawGunFlash(LG_FLARE, clip);
        break;

    case LG_PISTOLS:
    case LG_MAGNUMS:
    case LG_UZIS:
        phd_PushMatrix();
        phd_TranslateRel(*(bone + 1 + 28), *(bone + 2 + 28), *(bone + 3 + 28));
        *(phd_mxptr + M00) = *(phd_mxptr + M00 - 24);
        *(phd_mxptr + M01) = *(phd_mxptr + M01 - 24);
        *(phd_mxptr + M02) = *(phd_mxptr + M02 - 24);
        *(phd_mxptr + M10) = *(phd_mxptr + M10 - 24);
        *(phd_mxptr + M11) = *(phd_mxptr + M11 - 24);
        *(phd_mxptr + M12) = *(phd_mxptr + M12 - 24);
        *(phd_mxptr + M20) = *(phd_mxptr + M20 - 24);
        *(phd_mxptr + M21) = *(phd_mxptr + M21 - 24);
        *(phd_mxptr + M22) = *(phd_mxptr + M22 - 24);
        phd_RotYXZ(lara.right_arm.y_rot, lara.right_arm.x_rot, lara.right_arm.z_rot);
        rotation = lara.right_arm.frame_base + (anims[lara.right_arm.anim_number].interpolation >> 8) * (lara.right_arm.frame_number - anims[lara.right_arm.anim_number].frame_base) + 9;
        phd_RotYXZsuperpack(&rotation, 8);
        phd_PutPolygons(lara.mesh_ptrs[UARM_R], clip);

        phd_TranslateRel(*(bone + 1 + 32), *(bone + 2 + 32), *(bone + 3 + 32));
        phd_RotYXZsuperpack(&rotation, 0);
        phd_PutPolygons(lara.mesh_ptrs[LARM_R], clip);

        phd_TranslateRel(*(bone + 1 + 36), *(bone + 2 + 36), *(bone + 3 + 36));
        phd_RotYXZsuperpack(&rotation, 0);
        phd_PutPolygons(lara.mesh_ptrs[HAND_R], clip);

        if (lara.right_arm.flash_gun)
            S_LongMemCpy(savemox, phd_mxptr, 12);
        phd_PopMatrix();

        phd_PushMatrix();
        phd_TranslateRel(*(bone + 1 + 40), *(bone + 2 + 40), *(bone + 3 + 40));
        *(phd_mxptr + M00) = *(phd_mxptr + M00 - 24);
        *(phd_mxptr + M01) = *(phd_mxptr + M01 - 24);
        *(phd_mxptr + M02) = *(phd_mxptr + M02 - 24);
        *(phd_mxptr + M10) = *(phd_mxptr + M10 - 24);
        *(phd_mxptr + M11) = *(phd_mxptr + M11 - 24);
        *(phd_mxptr + M12) = *(phd_mxptr + M12 - 24);
        *(phd_mxptr + M20) = *(phd_mxptr + M20 - 24);
        *(phd_mxptr + M21) = *(phd_mxptr + M21 - 24);
        *(phd_mxptr + M22) = *(phd_mxptr + M22 - 24);
        phd_RotYXZ(lara.left_arm.y_rot, lara.left_arm.x_rot, lara.left_arm.z_rot);
        rotation = lara.left_arm.frame_base + (anims[lara.left_arm.anim_number].interpolation >> 8) * (lara.left_arm.frame_number - anims[lara.left_arm.anim_number].frame_base) + 9;
        phd_RotYXZsuperpack(&rotation, 11);
        phd_PutPolygons(lara.mesh_ptrs[UARM_L], clip);

        phd_TranslateRel(*(bone + 1 + 44), *(bone + 2 + 44), *(bone + 3 + 44));
        phd_RotYXZsuperpack(&rotation, 0);
        phd_PutPolygons(lara.mesh_ptrs[LARM_L], clip);

        phd_TranslateRel(*(bone + 1 + 48), *(bone + 2 + 48), *(bone + 3 + 48));
        phd_RotYXZsuperpack(&rotation, 0);
        phd_PutPolygons(lara.mesh_ptrs[HAND_L], clip);

        if (lara.left_arm.flash_gun)
            DrawGunFlash(fire_arms, clip);
        if (lara.right_arm.flash_gun)
        {
            S_LongMemCpy(phd_mxptr, savemox, 12);
            DrawGunFlash(fire_arms, clip);
        }
        break;

    case LG_SHOTGUN:
    case LG_M16:
    case LG_GRENADE:
    case LG_HARPOON:
        phd_PushMatrix();
        phd_TranslateRel(*(bone + 1 + 28), *(bone + 2 + 28), *(bone + 3 + 28));
        rotation = lara.right_arm.frame_base + lara.right_arm.frame_number * (anims[lara.right_arm.anim_number].interpolation >> 8) + 9;
        phd_RotYXZsuperpack(&rotation, 8);
        phd_PutPolygons(lara.mesh_ptrs[UARM_R], clip);

        phd_TranslateRel(*(bone + 1 + 32), *(bone + 2 + 32), *(bone + 3 + 32));
        phd_RotYXZsuperpack(&rotation, 0);
        phd_PutPolygons(lara.mesh_ptrs[LARM_R], clip);

        phd_TranslateRel(*(bone + 1 + 36), *(bone + 2 + 36), *(bone + 3 + 36));
        phd_RotYXZsuperpack(&rotation, 0);
        phd_PutPolygons(lara.mesh_ptrs[HAND_R], clip);

        if (lara.right_arm.flash_gun)
            S_LongMemCpy(savemox, phd_mxptr, 12);
        phd_PopMatrix();

        phd_PushMatrix();
        phd_TranslateRel(*(bone + 1 + 40), *(bone + 2 + 40), *(bone + 3 + 40));
        phd_RotYXZsuperpack(&rotation, 0);
        phd_PutPolygons(lara.mesh_ptrs[UARM_L], clip);

        phd_TranslateRel(*(bone + 1 + 44), *(bone + 2 + 44), *(bone + 3 + 44));
        phd_RotYXZsuperpack(&rotation, 0);
        phd_PutPolygons(lara.mesh_ptrs[LARM_L], clip);

        phd_TranslateRel(*(bone + 1 + 48), *(bone + 2 + 48), *(bone + 3 + 48));
        phd_RotYXZsuperpack(&rotation, 0);
        phd_PutPolygons(lara.mesh_ptrs[HAND_L], clip);

        if (lara.right_arm.flash_gun)
        {
            S_LongMemCpy(phd_mxptr, savemox, 12);
            DrawGunFlash(fire_arms, clip);
        }
        break;
    }

    phd_PopMatrix();
    phd_PopMatrix();
    phd_PopMatrix();

    phd_left = left;
    phd_right = right;
    phd_top = top;
    phd_bottom = bottom;
}

void phd_RotYXZ_I(sint16 y, sint16 x, sint16 z);
void InterpolateArmMatrix(void);

void DrawLaraInt(ITEM_INFO *item, sint16 *frame1, sint16 *frame2, int frac, int rate)
{
    OBJECT_INFO *object;
    sint32 *bone, *bonew;
    sint16 *rotation1, *rotation2, *rotationw1, *rotationw2;
    sint16 *bounds;
    int clip, fire_arms;
    sint32 savemox1[12];

    object = &objects[item->object_number];
    bounds = GetBoundsAccurate(item);
    if (lara.skidoo == NO_ITEM)
        S_PrintShadow(object->shadow_size, bounds, item, 0);

    S_LongMemCpy(savemox1, phd_mxptr, 12);

    phd_PushMatrix();
    phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);

    if (!(clip = S_GetObjectBounds(frame1)))
    {
        phd_PopMatrix();
        return;
    }

    phd_PushMatrix();
    CalculateObjectLighting(item, frame1);

    bone = bones + object->bone_index;
    rotation1 = frame1 + 9;
    rotation2 = frame2 + 9;

    InitInterpolate(frac, rate);

    phd_TranslateRel_ID((sint32)*(frame1 + 6), (sint32)*(frame1 + 7), (sint32)*(frame1 + 8), (sint32)*(frame2 + 6), (sint32)*(frame2 + 7), (sint32)*(frame2 + 8));
    phd_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
    phd_PutPolygons_I(lara.mesh_ptrs[HIPS], clip);

    phd_PushMatrix_I();
    phd_TranslateRel_I(*(bone + 1), *(bone + 2), *(bone + 3));
    phd_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
    phd_PutPolygons_I(lara.mesh_ptrs[THIGH_L], clip);

    phd_TranslateRel_I(*(bone + 1 + 4), *(bone + 2 + 4), *(bone + 3 + 4));
    phd_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
    phd_PutPolygons_I(lara.mesh_ptrs[CALF_L], clip);

    phd_TranslateRel_I(*(bone + 1 + 8), *(bone + 2 + 8), *(bone + 3 + 8));
    phd_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
    phd_PutPolygons_I(lara.mesh_ptrs[FOOT_L], clip);
    phd_PopMatrix_I();

    phd_PushMatrix_I();
    phd_TranslateRel_I(*(bone + 1 + 12), *(bone + 2 + 12), *(bone + 3 + 12));
    phd_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
    phd_PutPolygons_I(lara.mesh_ptrs[THIGH_R], clip);

    phd_TranslateRel_I(*(bone + 1 + 16), *(bone + 2 + 16), *(bone + 3 + 16));
    phd_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
    phd_PutPolygons_I(lara.mesh_ptrs[CALF_R], clip);

    phd_TranslateRel_I(*(bone + 1 + 20), *(bone + 2 + 20), *(bone + 3 + 20));
    phd_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
    phd_PutPolygons_I(lara.mesh_ptrs[FOOT_R], clip);
    phd_PopMatrix_I();

    phd_TranslateRel_I(*(bone + 1 + 24), *(bone + 2 + 24), *(bone + 3 + 24));
    if (lara.weapon_item != NO_ITEM && lara.gun_type == LG_M16
        && (items[lara.weapon_item].current_anim_state == 0 || items[lara.weapon_item].current_anim_state == 2
            || items[lara.weapon_item].current_anim_state == 4))
    {
        rotation1 = rotation2 = lara.right_arm.frame_base + lara.right_arm.frame_number * (anims[lara.right_arm.anim_number].interpolation >> 8) + 9;
        phd_RotYXZsuperpack_I(&rotation1, &rotation2, 7);
    }
    else
        phd_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
    phd_RotYXZ_I(lara.torso_y_rot, lara.torso_x_rot, lara.torso_z_rot);
    phd_PutPolygons_I(lara.mesh_ptrs[TORSO], clip);

    phd_PushMatrix_I();
    phd_TranslateRel_I(*(bone + 1 + 52), *(bone + 2 + 52), *(bone + 3 + 52));
    rotationw1 = rotation1;
    rotationw2 = rotation2;
    phd_RotYXZsuperpack_I(&rotation1, &rotation2, 6);
    rotation1 = rotationw1;
    rotation2 = rotationw2;
    phd_RotYXZ_I(lara.head_y_rot, lara.head_x_rot, lara.head_z_rot);
    phd_PutPolygons_I(lara.mesh_ptrs[HEAD], clip);

    S_LongMemCpy(phd_mxptr, savemox1, 12);
    DrawHair();
    phd_PopMatrix_I();

    if (lara.back_gun)
    {
        phd_PushMatrix_I();
        bonew = bones + objects[lara.back_gun].bone_index;
        phd_TranslateRel_I(*(bonew + 1 + 52), *(bonew + 2 + 52), *(bonew + 3 + 52));
        rotationw1 = rotationw2 = objects[lara.back_gun].frame_base + 9;
        phd_RotYXZsuperpack_I(&rotationw1, &rotationw2, 14);
        phd_PutPolygons_I(meshes[objects[lara.back_gun].mesh_index + HEAD], clip);
        phd_PopMatrix_I();
    }

    fire_arms = LG_UNARMED;
    if (lara.gun_status == LG_READY || lara.gun_status == LG_SPECIAL || lara.gun_status == LG_DRAW || lara.gun_status == LG_UNDRAW)
        fire_arms = lara.gun_type;

    switch (fire_arms)
    {
    case LG_UNARMED:
    case LG_FLARE:
        phd_PushMatrix_I();
        phd_TranslateRel_I(*(bone + 1 + 28), *(bone + 2 + 28), *(bone + 3 + 28));
        phd_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
        phd_PutPolygons_I(lara.mesh_ptrs[UARM_R], clip);

        phd_TranslateRel_I(*(bone + 1 + 32), *(bone + 2 + 32), *(bone + 3 + 32));
        phd_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
        phd_PutPolygons_I(lara.mesh_ptrs[LARM_R], clip);

        phd_TranslateRel_I(*(bone + 1 + 36), *(bone + 2 + 36), *(bone + 3 + 36));
        phd_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
        phd_PutPolygons_I(lara.mesh_ptrs[HAND_R], clip);
        phd_PopMatrix_I();

        phd_PushMatrix_I();
        phd_TranslateRel_I(*(bone + 1 + 40), *(bone + 2 + 40), *(bone + 3 + 40));
        if (lara.flare_control_left)
        {
            rotation1 = rotation2 = lara.left_arm.frame_base + (anims[lara.left_arm.anim_number].interpolation >> 8) * (lara.left_arm.frame_number - anims[lara.left_arm.anim_number].frame_base) + 9;
            phd_RotYXZsuperpack_I(&rotation1, &rotation2, 11);
        }
        else
            phd_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
        phd_PutPolygons_I(lara.mesh_ptrs[UARM_L], clip);

        phd_TranslateRel_I(*(bone + 1 + 44), *(bone + 2 + 44), *(bone + 3 + 44));
        phd_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
        phd_PutPolygons_I(lara.mesh_ptrs[LARM_L], clip);

        phd_TranslateRel_I(*(bone + 1 + 48), *(bone + 2 + 48), *(bone + 3 + 48));
        phd_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
        phd_PutPolygons_I(lara.mesh_ptrs[HAND_L], clip);

        if (lara.gun_type == LG_FLARE && lara.left_arm.flash_gun)
        {
            phd_TranslateRel_I(11, 32, 80);
            phd_RotX_I(-90 * ONE_DEGREE);
            phd_RotY_I((sint16)(GetRandomDraw() * 2));
            S_CalculateStaticLight(2048);
            phd_PutPolygons_I(meshes[objects[FLARE_FIRE].mesh_index], clip);
        }
        break;

    case LG_PISTOLS:
    case LG_MAGNUMS:
    case LG_UZIS:
        phd_PushMatrix_I();
        phd_TranslateRel_I(*(bone + 1 + 28), *(bone + 2 + 28), *(bone + 3 + 28));
        InterpolateArmMatrix();
        phd_RotYXZ(lara.right_arm.y_rot, lara.right_arm.x_rot, lara.right_arm.z_rot);
        rotation1 = lara.right_arm.frame_base + (anims[lara.right_arm.anim_number].interpolation >> 8) * (lara.right_arm.frame_number - anims[lara.right_arm.anim_number].frame_base) + 9;
        phd_RotYXZsuperpack(&rotation1, 8);
        phd_PutPolygons(lara.mesh_ptrs[UARM_R], clip);

        phd_TranslateRel(*(bone + 1 + 32), *(bone + 2 + 32), *(bone + 3 + 32));
        phd_RotYXZsuperpack(&rotation1, 0);
        phd_PutPolygons(lara.mesh_ptrs[LARM_R], clip);

        phd_TranslateRel(*(bone + 1 + 36), *(bone + 2 + 36), *(bone + 3 + 36));
        phd_RotYXZsuperpack(&rotation1, 0);
        phd_PutPolygons(lara.mesh_ptrs[HAND_R], clip);

        if (lara.right_arm.flash_gun)
            S_LongMemCpy(savemox1, phd_mxptr, 12);
        phd_PopMatrix_I();

        phd_PushMatrix_I();
        phd_TranslateRel_I(*(bone + 1 + 40), *(bone + 2 + 40), *(bone + 3 + 40));
        InterpolateArmMatrix();
        phd_RotYXZ(lara.left_arm.y_rot, lara.left_arm.x_rot, lara.left_arm.z_rot);
        rotation1 = lara.left_arm.frame_base + (anims[lara.left_arm.anim_number].interpolation >> 8) * (lara.left_arm.frame_number - anims[lara.left_arm.anim_number].frame_base) + 9;
        phd_RotYXZsuperpack(&rotation1, 11);
        phd_PutPolygons(lara.mesh_ptrs[UARM_L], clip);

        phd_TranslateRel(*(bone + 1 + 44), *(bone + 2 + 44), *(bone + 3 + 44));
        phd_RotYXZsuperpack(&rotation1, 0);
        phd_PutPolygons(lara.mesh_ptrs[LARM_L], clip);

        phd_TranslateRel(*(bone + 1 + 48), *(bone + 2 + 48), *(bone + 3 + 48));
        phd_RotYXZsuperpack(&rotation1, 0);
        phd_PutPolygons(lara.mesh_ptrs[HAND_L], clip);

        if (lara.left_arm.flash_gun)
            DrawGunFlash(fire_arms, clip);
        if (lara.right_arm.flash_gun)
        {
            S_LongMemCpy(phd_mxptr, savemox1, 12);
            DrawGunFlash(fire_arms, clip);
        }
        break;

    case LG_SHOTGUN:
    case LG_M16:
    case LG_GRENADE:
    case LG_HARPOON:
        phd_PushMatrix_I();
        phd_TranslateRel_I(*(bone + 1 + 28), *(bone + 2 + 28), *(bone + 3 + 28));
        rotation1 = rotation2 = lara.right_arm.frame_base + lara.right_arm.frame_number * (anims[lara.right_arm.anim_number].interpolation >> 8) + 9;
        phd_RotYXZsuperpack_I(&rotation1, &rotation2, 8);
        phd_PutPolygons_I(lara.mesh_ptrs[UARM_R], clip);

        phd_TranslateRel_I(*(bone + 1 + 32), *(bone + 2 + 32), *(bone + 3 + 32));
        phd_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
        phd_PutPolygons_I(lara.mesh_ptrs[LARM_R], clip);

        phd_TranslateRel_I(*(bone + 1 + 36), *(bone + 2 + 36), *(bone + 3 + 36));
        phd_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
        phd_PutPolygons_I(lara.mesh_ptrs[HAND_R], clip);

        if (lara.right_arm.flash_gun)
            S_LongMemCpy(savemox1, phd_mxptr, 12);
        phd_PopMatrix_I();

        phd_PushMatrix_I();
        phd_TranslateRel_I(*(bone + 1 + 40), *(bone + 2 + 40), *(bone + 3 + 40));
        phd_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
        phd_PutPolygons_I(lara.mesh_ptrs[UARM_L], clip);

        phd_TranslateRel_I(*(bone + 1 + 44), *(bone + 2 + 44), *(bone + 3 + 44));
        phd_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
        phd_PutPolygons_I(lara.mesh_ptrs[LARM_L], clip);

        phd_TranslateRel_I(*(bone + 1 + 48), *(bone + 2 + 48), *(bone + 3 + 48));
        phd_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
        phd_PutPolygons_I(lara.mesh_ptrs[HAND_L], clip);

        if (lara.right_arm.flash_gun)
        {
            S_LongMemCpy(phd_mxptr, savemox1, 12);
            DrawGunFlash(fire_arms, clip);
        }
        break;
    }

    phd_PopMatrix();
    phd_PopMatrix();
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
