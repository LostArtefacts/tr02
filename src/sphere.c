#include "game.h"
#include "box.h"

int TestCollision(ITEM_INFO *item, ITEM_INFO *laraitem)
{
    int num1;
    int num2;
    int x1, y1, z1, r1;
    int x, y, z, r;
    int i, j;
    uint32 flags;
    SPHERE *ptr1;
    SPHERE *ptr2;
    SPHERE slist_baddie[34];
    SPHERE slist_lara[34];

    flags = 0;
    num1 = GetSpheres(item, &slist_baddie[0], 1);
    num2 = GetSpheres(laraitem, &slist_lara[0], 1);
    ptr1 = &slist_baddie[0];
    for (i = 0; i < num1; i++, ptr1++)
    {
        x1 = ptr1->x;
        y1 = ptr1->y;
        z1 = ptr1->z;
        r1 = ptr1->r;
        if (r1 > 0)
        {
            ptr2 = &slist_lara[0];
            for (j = 0; j < num2; j++, ptr2++)
            {
                if (ptr2->r > 0)
                {
                    x = ptr2->x - x1;
                    y = ptr2->y - y1;
                    z = ptr2->z - z1;
                    r = ptr2->r + r1;
                    if ((x * x + y * y + z * z) < (r * r))
                    {
                        flags |= (1 << i);
                        break;
                    }
                }
            }
        }
    }
    item->touch_bits = flags;
    return flags;
}

#define ROT_X 4
#define ROT_Y 8
#define ROT_Z 16

void __cdecl phd_PushUnitMatrix(void);
sint16 *GetBestFrame(ITEM_INFO *item);
void phd_RotYXZsuperpack(sint16 **pprot, int skip);
void phd_RotYXZ(PHD_ANGLE ry, PHD_ANGLE rx, PHD_ANGLE rz);
void phd_RotZ(PHD_ANGLE rz);

int phd_TranslateRel(sint32 x, sint32 y, sint32 z);

int GetSpheres(ITEM_INFO *item, SPHERE *ptr, int WorldSpace)
{
    int i, poppush;
    OBJECT_INFO *object;
    sint32 *bone;
    sint16 *rotation;
    sint16 *frame;
    sint32 x, y, z;
    sint16 *objptr, **meshpp;
    sint16 *extra_rotation;

    if (item == NULL)
        return 0;

    if (WorldSpace)
    {
        x = item->pos.x_pos;
        y = item->pos.y_pos;
        z = item->pos.z_pos;
        phd_PushUnitMatrix();
        *(phd_mxptr + M03) = 0;
        *(phd_mxptr + M13) = 0;
        *(phd_mxptr + M23) = 0;
    }
    else
    {
        x = y = z = 0;
        phd_PushMatrix();
        phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    }
    phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);
    frame = GetBestFrame(item);
    phd_TranslateRel((sint32)frame[6], (sint32)frame[7], (sint32)frame[8]);
    rotation = frame + 9;
    phd_RotYXZsuperpack(&rotation, 0);

    object = &objects[item->object_number];
    meshpp = &meshes[object->mesh_index];
    bone = bones + object->bone_index;

    objptr = *(meshpp++);
    phd_PushMatrix();
    phd_TranslateRel((sint32)objptr[0], (sint32)objptr[1], (sint32)objptr[2]);
    ptr->x = x + (*(phd_mxptr + M03) >> W2V_SHIFT);
    ptr->y = y + (*(phd_mxptr + M13) >> W2V_SHIFT);
    ptr->z = z + (*(phd_mxptr + M23) >> W2V_SHIFT);
    ptr->r = (sint32)objptr[3];
    ptr++;
    phd_PopMatrix();

    extra_rotation = (sint16 *)item->data;
    for (i = object->nmeshes - 1; i > 0; i--, bone += 3)
    {
        poppush = *(bone++);
        if (poppush & 1)
            phd_PopMatrix();
        if (poppush & 2)
            phd_PushMatrix();

        phd_TranslateRel(*(bone), *(bone + 1), *(bone + 2));
        phd_RotYXZsuperpack(&rotation, 0);

        if ((poppush & (ROT_X | ROT_Y | ROT_Z)) && extra_rotation)
        {
            if (poppush & ROT_Y)
                phd_RotY(*(extra_rotation++));
            if (poppush & ROT_X)
                phd_RotX(*(extra_rotation++));
            if (poppush & ROT_Z)
                phd_RotZ(*(extra_rotation++));
        }

        objptr = *(meshpp++);
        phd_PushMatrix();
        phd_TranslateRel((sint32)objptr[0], (sint32)objptr[1], (sint32)objptr[2]);
        ptr->x = x + (*(phd_mxptr + M03) >> W2V_SHIFT);
        ptr->y = y + (*(phd_mxptr + M13) >> W2V_SHIFT);
        ptr->z = z + (*(phd_mxptr + M23) >> W2V_SHIFT);
        ptr->r = (sint32)objptr[3];
        ptr++;
        phd_PopMatrix();
    }
    phd_PopMatrix();
    return object->nmeshes;
}

void GetJointAbsPosition(ITEM_INFO *item, PHD_VECTOR *vec, int joint)
{
    int i, poppush;
    OBJECT_INFO *object;
    sint32 *bone;
    sint16 *rotation;
    sint16 *frame;
    sint16 *extra_rotation;

    object = &objects[item->object_number];
    frame = GetBestFrame(item);

    phd_PushUnitMatrix();
    *(phd_mxptr + M03) = 0;
    *(phd_mxptr + M13) = 0;
    *(phd_mxptr + M23) = 0;
    phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);

    phd_TranslateRel((sint32)frame[6], (sint32)frame[7], (sint32)frame[8]);

    rotation = frame + 9;
    phd_RotYXZsuperpack(&rotation, 0);

    bone = bones + object->bone_index;

    extra_rotation = (sint16 *)item->data;
    for (i = 0; i < joint; i++, bone += 3)
    {
        poppush = *(bone++);
        if (poppush & 1)
            phd_PopMatrix();
        if (poppush & 2)
            phd_PushMatrix();

        phd_TranslateRel(*(bone), *(bone + 1), *(bone + 2));
        phd_RotYXZsuperpack(&rotation, 0);

        if (poppush & (ROT_X | ROT_Y | ROT_Z))
        {
            if (poppush & ROT_Y)
                phd_RotY(*(extra_rotation++));
            if (poppush & ROT_X)
                phd_RotX(*(extra_rotation++));
            if (poppush & ROT_Z)
                phd_RotZ(*(extra_rotation++));
        }
    }
    phd_TranslateRel(vec->x, vec->y, vec->z);
    vec->x = (*(phd_mxptr + M03) >> W2V_SHIFT) + item->pos.x_pos;
    vec->y = (*(phd_mxptr + M13) >> W2V_SHIFT) + item->pos.y_pos;
    vec->z = (*(phd_mxptr + M23) >> W2V_SHIFT) + item->pos.z_pos;
    phd_PopMatrix();
}

void BaddieBiteEffect(ITEM_INFO *item, BITE_INFO *bite)
{
    PHD_VECTOR pos;

    pos.x = bite->x;
    pos.y = bite->y;
    pos.z = bite->z;
    GetJointAbsPosition(item, &pos, bite->mesh_num);
    DoBloodSplat(pos.x, pos.y, pos.z, item->speed, item->pos.y_rot, item->room_number);
}
