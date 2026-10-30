#ifndef DISPLAY_H
#define DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

void TempVideoAdjust(int hires, double sizer);
void TempVideoRemove(void);
void IncreaseScreenSize();
void DecreaseScreenSize();
void S_FadeOutInventory(int fade);
void S_FadeInInventory(int fade);

#ifdef __cplusplus
}
#endif

#endif
