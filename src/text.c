#include "game.h"
#include "text.h"

#define MAX_TEXT_STRINGS 64
#define MAX_STRING_SIZE 64

#define T_ACTIVE (1 << 0)
#define T_FLASH (1 << 1)
#define T_ROTATE_H (1 << 2)
#define T_ROTATE_V (1 << 3)
#define T_CENTRE_H (1 << 4)
#define T_CENTRE_V (1 << 5)
#define T_RIGHTALIGN (1 << 7)
#define T_BOTTOMALIGN (1 << 8)
#define T_ADDBACKGROUND (1 << 9)
#define T_ADDOUTLINE (1 << 10)
#define T_RIGHTJUSTIFY (1 << 11)

extern TEXTSTRING T_textStrings[MAX_TEXT_STRINGS];
extern sint16 T_numStrings;

void DisplayModeInfo(char *string);

void T_InitPrint(void)
{
    int n;
    TEXTSTRING *textString = T_textStrings;

    DisplayModeInfo(0);

    for (n = 0; n < MAX_TEXT_STRINGS; n++)
    {
        textString->flags = 0;
        textString++;
    }
    T_numStrings = 0;
}
