#include "global.h"
#include <dinput.h>
#include "input.h"

int Key(int number)
{
    sint16 key;

    key = layout[1][number];
    if (key >= 256)
    {
        if ((1 << (key & 0xff)) & joy_fire)
            return 1;
    }
    else
    {
        if (rawkey(key))
            return 1;
        if (key == DIK_RCONTROL)
            return rawkey(DIK_LCONTROL);
        if (key == DIK_LCONTROL)
            return rawkey(DIK_RCONTROL);
        if (key == DIK_RSHIFT)
            return rawkey(DIK_LSHIFT);
        if (key == DIK_LSHIFT)
            return rawkey(DIK_RSHIFT);
        if (key == DIK_RMENU)
            return rawkey(DIK_LMENU);
        if (key == DIK_LMENU)
            return rawkey(DIK_RMENU);
    }
    if (conflict[number])
        return 0;
    key = layout[0][number];
    if (rawkey(key))
        return 1;
    if (key == DIK_RCONTROL)
        return rawkey(DIK_LCONTROL);
    if (key == DIK_LCONTROL)
        return rawkey(DIK_RCONTROL);
    if (key == DIK_RSHIFT)
        return rawkey(DIK_LSHIFT);
    if (key == DIK_LSHIFT)
        return rawkey(DIK_RSHIFT);
    if (key == DIK_RMENU)
        return rawkey(DIK_LMENU);
    if (key == DIK_LMENU)
        return rawkey(DIK_RMENU);
    return 0;
}
