#ifndef _INPUT_H_
#define _INPUT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../types.h"

extern sint32 input;

extern int S_UpdateInput();

#ifdef __cplusplus
}
#endif

extern sint16 layout[2][14];
extern int conflict[];
extern int joy_fire;

extern unsigned char keymap[];
#define rawkey(A) (keymap[A] & 0x80)

int Key(int number);
void DefaultConflict(void);

#endif
