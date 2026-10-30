#include "../3dglodef.h"

typedef struct {
    float xv, yv, zv, ooz, xs, ys;
    sint16 clip, g, u, v;
} PHD_VBUF;

extern "C" PHD_VBUF PhdVBuf[];
extern "C" PHDTEXTURESTRUCT phdtextinfo[];

bool HWR_VertexBufferFull();

extern "C" void InsertGT4_Sorted(PHD_VBUF *v1, PHD_VBUF *v2, PHD_VBUF *v3, PHD_VBUF *v4, PHDTEXTURESTRUCT *tex, int sorttype);

extern "C" void InsertGT4_ZBuffered(PHD_VBUF *v1, PHD_VBUF *v2, PHD_VBUF *v3, PHD_VBUF *v4, PHDTEXTURESTRUCT *tex);

extern "C" sint16 *InsertObjectGT4_ZBuffered(sint16 *objptr, int number, int sorttype)
{
    PHD_VBUF *v1, *v2, *v3, *v4;

    while (number--) {
        v1 = &PhdVBuf[objptr[0]];
        v2 = &PhdVBuf[objptr[1]];
        v3 = &PhdVBuf[objptr[2]];
        v4 = &PhdVBuf[objptr[3]];
        if (phdtextinfo[objptr[4]].drawtype)
            InsertGT4_Sorted(v1, v2, v3, v4, &phdtextinfo[objptr[4]], sorttype);
        else
            InsertGT4_ZBuffered(v1, v2, v3, v4, &phdtextinfo[objptr[4]]);
        objptr += 5;
    }
    return objptr;
}

extern "C" sint16 *InsertObjectGT4_Sorted(sint16 *objptr, int number, int sorttype)
{
    while (number) {
        if (HWR_VertexBufferFull())
            return objptr + 5 * number;
        InsertGT4_Sorted(&PhdVBuf[objptr[0]], &PhdVBuf[objptr[1]], &PhdVBuf[objptr[2]], &PhdVBuf[objptr[3]], &phdtextinfo[objptr[4]], sorttype);
        number--;
        objptr += 5;
    }
    return objptr;
}
