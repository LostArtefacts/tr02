#ifndef _3D_GEN_H
#define _3D_GEN_H

#include "../types.h"

#ifdef __cplusplus
extern "C" {
#endif

void S_InsertBackground(sint16 *objptr);
void S_InsertRoom(sint16 *objptr, int outside);
void S_InsertInvBgnd(sint16 *objptr);

#ifdef __cplusplus
}
#endif

#endif
