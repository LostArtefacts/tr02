#include "game.h"
#include "invdata.h"

#ifdef TR2_EXE
extern char LoadGameStrings2[][50];
extern char LoadGameStrings1[][50];
extern char SaveGameStrings1[][50];
extern char SaveGameStrings2[][50];

sint32 inv_nframes = 2;
SG_COL req_bgnd_gour1[16] = {
    0x1e00, 0x1e00, 0x1a00, 0x1e00, 0x1e00, 0x1e00, 0x1e00, 0x1a00, 0x1a00, 0x1e00, 0x1e00,
    0x1e00, 0x1e00, 0x1a00, 0x1e00, 0x1e00,
};
SG_COL req_bgnd_gour2[12] = {
    0x1a00, 0x1800, 0x1e00, 0x2000, 0x2000, 0x1e00, 0x1c00, 0x1c00, 0x1a00, 0, 0, 0,
};
SG_COL req_main_gour1[16] = {
    0x2000, 0x2000, 0x1800, 0x2000, 0x2000, 0x2000, 0x2000, 0x1800, 0x1800, 0x2000, 0x2000,
    0x2000, 0x2000, 0x1800, 0x2000, 0x2000,
};
SG_COL req_main_gour2[12] = {
    0x2000, 0x2000, 0x2000, 0x2000, 0x2000, 0x2000, 0x2000, 0x2000, 0x2000, 0, 0, 0,
};
SG_COL req_sel_gour1[16] = {
    0x2000, 0x2000, 0x1400, 0x2000, 0x2000, 0x2000, 0x2000, 0x1400, 0x1400, 0x2000, 0x2000,
    0x2000, 0x2000, 0x1400, 0x2000, 0x2000,
};
SG_COL req_sel_gour2[12] = {
    0x2000, 0x1010, 0x2000, 0x1400, 0x2000, 0x1010, 0x2000, 0x1400, 0x2000, 0, 0, 0,
};
SG_COL req_unsel_gour1[16] = {
    0x2000, 0x2000, 0x1a00, 0x2000, 0x2000, 0x2000, 0x2000, 0x1a00, 0x1a00, 0x2000, 0x2000,
    0x2000, 0x2000, 0x1a00, 0x2000, 0x2000,
};
SG_COL req_unsel_gour2[10] = {
    0x2000, 0x1c40, 0x2000, 0x1c40, 0x2000, 0x1c40, 0x2000, 0x1c40, 0x2000, 0,
};
SG_COL is_gour1[2] = {
    0x1010, 0x1600,
};
SG_COL is_gour2[4] = {
    0x1a00, 0x1e00, 0, 0,
};
SG_COL is_gour3[8] = {
    0x1600, 0x1800, 0x1a00, 0x1800, 0x2000, 0x1010, 0x1010, 0x2000,
};
SG_COL is_gour4[4] = {
    0x2000, 0x1010, 0x1010, 0x2000,
};
SG_COL is_gour6_and_detail[8] = {
    0x2000, 0x1010, 0x1010, 0x2000, 0x2000, 0x1010, 0x1010, 0x2000,
};
SG_COL is_gour8[2] = {
    0x1210, 0x1800,
};
SG_COL is_gour9[2] = {
    0x1c00, 0x2000,
};
SG_COL is_gour10[4] = {
    0x1800, 0x1a00, 0x1c00, 0x1a00,
};
SG_COL is_gour11[4] = {
    0x2000, 0x1800, 0x1800, 0x2000,
};
SG_COL is_gour12[4] = {
    0x2000, 0x1800, 0x1800, 0x2000,
};
INVENTORY_SPRITE is_detail_level[10] = {
    { 2, -66, -72, 32, 132, 0, is_gour1, 1 },
    { 2, -66, -56, 32, 132, 0, is_gour2, 1 },
    { 2, -66, -72, 32, 0, 16, is_gour1, 1 },
    { 2, 66, -72, 32, 0, 16, is_gour2, 1 },
    { 3, -65, -71, 32, 130, 14, is_gour3, 1 },
    { 3, -64, -70, 32, 128, 12, is_gour3, 1 },
    { 4, -63, -69, 32, 126, 10, is_gour6_and_detail + 4, 16 },
    { 4, -63, -69, 33, 126, 10, NULL, 0 },
    { 2, 1, -69, 31, 0, 10, NULL, 2 },
    { 0, 0, 0, 0, 0, 0, NULL, 0 },
};
INVENTORY_SPRITE *is_detail_list[2] = {
    is_detail_level,
    NULL,
};
INVENTORY_SPRITE is_sound_muslevel[9] = {
    { 2, -66, -80, 32, 132, 0, is_gour1, 1 },
    { 2, -66, -65, 32, 132, 0, is_gour2, 1 },
    { 2, -66, -80, 32, 0, 16, is_gour1, 1 },
    { 2, 66, -80, 32, 0, 16, is_gour2, 1 },
    { 3, -65, -79, 32, 131, 14, is_gour3, 1 },
    { 3, -64, -78, 32, 129, 12, is_gour3, 1 },
    { 4, -63, -77, 32, 126, 10, is_gour4, 3 },
    { 4, -63, -77, 33, 127, 10, NULL, 0 },
    { 0, 0, 0, 0, 0, 0, NULL, 0 },
};
INVENTORY_SPRITE is_sound_sfxlevel[9] = {
    { 2, -66, -56, 32, 132, 0, is_gour1, 1 },
    { 2, -66, -41, 32, 132, 0, is_gour2, 1 },
    { 2, -66, -56, 32, 0, 16, is_gour1, 1 },
    { 2, 66, -56, 32, 0, 16, is_gour2, 1 },
    { 3, -65, -55, 32, 131, 14, is_gour3, 1 },
    { 3, -64, -54, 32, 129, 12, is_gour3, 1 },
    { 4, -63, -53, 32, 126, 10, is_gour6_and_detail, 15 },
    { 4, -63, -53, 33, 127, 10, NULL, 0 },
    { 0, 0, 0, 0, 0, 0, NULL, 0 },
};
INVENTORY_SPRITE is_sound_muslevellow[9] = {
    { 2, -66, -80, 32, 132, 0, is_gour8, 1 },
    { 2, -66, -65, 32, 132, 0, is_gour9, 1 },
    { 2, -66, -80, 32, 0, 16, is_gour8, 1 },
    { 2, 66, -80, 32, 0, 16, is_gour9, 1 },
    { 3, -65, -79, 32, 131, 14, is_gour10, 1 },
    { 3, -64, -78, 32, 129, 12, is_gour10, 1 },
    { 4, -63, -77, 32, 126, 10, is_gour11, 3 },
    { 4, -63, -77, 33, 127, 10, NULL, 0 },
    { 0, 0, 0, 0, 0, 0, NULL, 0 },
};
INVENTORY_SPRITE is_sound_sfxlevellow[9] = {
    { 2, -66, -56, 32, 132, 0, is_gour8, 1 },
    { 2, -66, -41, 32, 132, 0, is_gour9, 1 },
    { 2, -66, -56, 32, 0, 16, is_gour8, 1 },
    { 2, 66, -56, 32, 0, 16, is_gour9, 1 },
    { 3, -65, -55, 32, 131, 14, is_gour10, 1 },
    { 3, -64, -54, 32, 129, 12, is_gour10, 1 },
    { 4, -63, -53, 32, 126, 10, is_gour12, 15 },
    { 4, -63, -53, 33, 127, 10, NULL, 0 },
    { 0, 0, 0, 0, 0, 0, NULL, 0 },
};
INVENTORY_SPRITE is_sound_names[3] = {
    { 1, -75, -63, 32, 524288, 524288, NULL, 101 },
    { 1, -75, -40, 32, 524288, 524288, NULL, 102 },
    { 0, 0, 0, 0, 0, 0, NULL, 0 },
};
INVENTORY_SPRITE *is_sound_list[4] = {
    is_sound_names,
    is_sound_muslevel,
    is_sound_sfxlevel,
    NULL,
};
INVENTORY_ITEM InvCompassOption = {
    NULL, 121, 1, 0, 0, 0, 1, 1, 0, 4352, 0, -1536, 0, 0, 0, 0, -170, 0, 320, 0,
    0xffffffff, 0xffffffff, 0, NULL, { 0, 0, 0, 0 },
};
INVENTORY_ITEM InvPistolOption = {
    NULL, 157, 12, 0, 0, 11, 1, 1, 0, 3200, 0, 2848, 0, 0, -32768, 0, 38, 0, 352, 0,
    0xffffffff, 0xffffffff, 1, NULL, { 0, 0, 0, 0 },
};
INVENTORY_ITEM InvFlareOption = {
    NULL, 173, 31, 0, 0, 30, 1, 1, 0, 3200, 0, 0, 0, 0, -8192, 0, 0, 0, 296, 0, 0xffffffff,
    0xffffffff, 8, NULL, { 0, 0, 0, 0 },
};
INVENTORY_ITEM InvShotgunOption = {
    NULL, 158, 13, 0, 0, 12, 1, 1, 0, 3200, 0, 5120, 0, 0, 30720, 0, 0, 0, 228, 0,
    0xffffffff, 0xffffffff, 2, NULL, { 0, 0, 0, 0 },
};
INVENTORY_ITEM InvMagnumOption = {
    NULL, 159, 12, 0, 0, 11, 1, 1, 0, 3200, 0, 3360, 0, 0, -32768, 0, 0, 0, 362, 0,
    0xffffffff, 0xffffffff, 3, NULL, { 0, 0, 0, 0 },
};
INVENTORY_ITEM InvUziOption = {
    NULL, 160, 13, 0, 0, 12, 1, 1, 0, 3200, 0, 2336, 0, 0, -32768, 0, 56, 0, 322, 0,
    0xffffffff, 0xffffffff, 4, NULL, { 0, 0, 0, 0 },
};
INVENTORY_ITEM InvHarpoonOption = {
    NULL, 161, 12, 0, 0, 11, 1, 1, 0, 3200, 0, -736, 0, 0, -19456, 0, 58, 0, 296, 0,
    0xffffffff, 0xffffffff, 7, NULL, { 0, 0, 0, 0 },
};
INVENTORY_ITEM InvM16Option = {
    NULL, 162, 12, 0, 0, 11, 1, 1, 0, 3200, 0, -224, 0, 0, -18432, 0, 84, 0, 296, 0,
    0xffffffff, 0xffffffff, 5, NULL, { 0, 0, 0, 0 },
};
INVENTORY_ITEM InvGrenadeOption = {
    NULL, 163, 12, 0, 0, 11, 1, 1, 0, 3200, 0, -224, 0, 0, 14336, 0, 56, 0, 296, 0,
    0xffffffff, 0xffffffff, 6, NULL, { 0, 0, 0, 0 },
};
INVENTORY_ITEM InvPistolAmmoOption = {
    NULL, 164, 1, 0, 0, 0, 1, 1, 0, 3200, 0, -3808, 0, 0, 0, 0, 0, 0, 296, 0, 0xffffffff,
    0xffffffff, 1, NULL, { 0, 0, 0, 0 },
};
INVENTORY_ITEM InvShotgunAmmoOption = {
    NULL, 165, 1, 0, 0, 0, 1, 1, 0, 3200, 0, -3808, 0, 0, 0, 0, 0, 0, 296, 0, 0xffffffff,
    0xffffffff, 2, NULL, { 0, 0, 0, 0 },
};
INVENTORY_ITEM InvMagnumAmmoOption = {
    NULL, 166, 1, 0, 0, 0, 1, 1, 0, 3200, 0, -3808, 0, 0, 0, 0, 0, 0, 296, 0, 0xffffffff,
    0xffffffff, 3, NULL, { 0, 0, 0, 0 },
};
INVENTORY_ITEM InvUziAmmoOption = {
    NULL, 167, 1, 0, 0, 0, 1, 1, 0, 3200, 0, -3808, 0, 0, 0, 0, 0, 0, 296, 0, 0xffffffff,
    0xffffffff, 4, NULL, { 0, 0, 0, 0 },
};
INVENTORY_ITEM InvHarpoonAmmoOption = {
    NULL, 168, 1, 0, 0, 0, 1, 1, 0, 3200, 0, -3808, 0, 0, 0, 0, 0, 0, 296, 0, 0xffffffff,
    0xffffffff, 7, NULL, { 0, 0, 0, 0 },
};
INVENTORY_ITEM InvM16AmmoOption = {
    NULL, 169, 1, 0, 0, 0, 1, 1, 0, 3200, 0, -3808, 0, 0, 0, 0, 0, 0, 296, 0, 0xffffffff,
    0xffffffff, 5, NULL, { 0, 0, 0, 0 },
};
INVENTORY_ITEM InvGrenadeAmmoOption = {
    NULL, 170, 1, 0, 0, 0, 1, 1, 0, 3200, 0, -3808, 0, 0, 0, 0, 0, 0, 296, 0, 0xffffffff,
    0xffffffff, 6, NULL, { 0, 0, 0, 0 },
};
INVENTORY_ITEM InvSmallMedipackOption = {
    NULL, 171, 26, 0, 0, 25, 1, 1, 0, 4032, 0, -7296, 0, 0, -4096, 0, 0, 0, 216, 0,
    0xffffffff, 0xffffffff, 11, NULL, { 0, 0, 0, 0 },
};
INVENTORY_ITEM InvLargeMedipackOption = {
    NULL, 172, 20, 0, 0, 19, 1, 1, 0, 3616, 0, -8160, 0, 0, -4096, 0, 0, 0, 352, 0,
    0xffffffff, 0xffffffff, 10, NULL, { 0, 0, 0, 0 },
};
INVENTORY_ITEM InvPickup1Option = {
    NULL, 207, 1, 0, 0, 0, 1, 1, 0, 7200, 0, -4352, 0, 0, 0, 0, 0, 0, 256, 0, 0xffffffff,
    0xffffffff, 111, NULL, { 0, 0, 0, 0 },
};
INVENTORY_ITEM InvPickup2Option = {
    NULL, 208, 1, 0, 0, 0, 1, 1, 0, 7200, 0, -4352, 0, 0, 0, 0, 0, 0, 256, 0, 0xffffffff,
    0xffffffff, 110, NULL, { 0, 0, 0, 0 },
};
INVENTORY_ITEM InvPuzzle1Option = {
    NULL, 178, 1, 0, 0, 0, 1, 1, 0, 7200, 0, -4352, 0, 0, 0, 0, 0, 0, 256, 0, 0xffffffff,
    0xffffffff, 108, NULL, { 0, 0, 0, 0 },
};
INVENTORY_ITEM InvPuzzle2Option = {
    NULL, 179, 1, 0, 0, 0, 1, 1, 0, 7200, 0, -4352, 0, 0, 0, 0, 0, 0, 256, 0, 0xffffffff,
    0xffffffff, 107, NULL, { 0, 0, 0, 0 },
};
INVENTORY_ITEM InvPuzzle3Option = {
    NULL, 180, 1, 0, 0, 0, 1, 1, 0, 7200, 0, -4352, 0, 0, 0, 0, 0, 0, 256, 0, 0xffffffff,
    0xffffffff, 106, NULL, { 0, 0, 0, 0 },
};
INVENTORY_ITEM InvPuzzle4Option = {
    NULL, 181, 1, 0, 0, 0, 1, 1, 0, 7200, 0, -4352, 0, 0, 0, 0, 0, 0, 256, 0, 0xffffffff,
    0xffffffff, 105, NULL, { 0, 0, 0, 0 },
};
INVENTORY_ITEM InvKey1Option = {
    NULL, 197, 1, 0, 0, 0, 1, 1, 0, 7200, 0, -4352, 0, 0, 0, 0, 0, 0, 256, 0, 0xffffffff,
    0xffffffff, 101, NULL, { 0, 0, 0, 0 },
};
INVENTORY_ITEM InvKey2Option = {
    NULL, 198, 1, 0, 0, 0, 1, 1, 0, 7200, 0, -4352, 0, 0, 0, 0, 0, 0, 256, 0, 0xffffffff,
    0xffffffff, 102, NULL, { 0, 0, 0, 0 },
};
INVENTORY_ITEM InvKey3Option = {
    NULL, 199, 1, 0, 0, 0, 1, 1, 0, 7200, 0, -4352, 0, 0, 0, 0, 0, 0, 256, 0, 0xffffffff,
    0xffffffff, 103, NULL, { 0, 0, 0, 0 },
};
INVENTORY_ITEM InvKey4Option = {
    NULL, 200, 1, 0, 0, 0, 1, 1, 0, 7200, 0, -4352, 0, 0, 0, 0, 0, 0, 256, 0, 0xffffffff,
    0xffffffff, 104, NULL, { 0, 0, 0, 0 },
};
INVENTORY_ITEM InvPassportOption = {
    NULL, 133, 30, 0, 0, 14, 1, 1, 0, 4640, 0, -4320, 0, 0, 0, 0, 0, 0, 384, 0, 19, 19, 0,
    NULL, { 0, 0, 0, 0 },
};
INVENTORY_ITEM InvDetailOption = {
    NULL, 153, 1, 0, 0, 0, 1, 1, 0, 4224, 0, -7232, 0, 0, 0, 0, 16, 0, 444, 0, 0xffffffff,
    0xffffffff, 1, NULL, { 0, 0, 0, 0 },
};
INVENTORY_ITEM InvSoundOption = {
    NULL, 154, 1, 0, 0, 0, 1, 1, 0, 4832, 0, -5408, 0, 0, -3072, 0, -2, 0, 350, 0,
    0xffffffff, 0xffffffff, 2, NULL, { 0, 0, 0, 0 },
};
INVENTORY_ITEM InvControlOption = {
    NULL, 155, 1, 0, 0, 0, 1, 1, 0, 5504, 0, -2560, 5632, 5632, 13312, 0, 46, 0, 508, 0,
    0xffffffff, 0xffffffff, 3, NULL, { 0, 0, 0, 0 },
};
INVENTORY_ITEM InvPhotoOption = {
    NULL, 122, 1, 0, 0, 0, 1, 1, 0, 4640, 0, -4320, 0, 0, 0, 0, 0, 0, 384, 0, 0xffffffff,
    0xffffffff, 5, NULL, { 0, 0, 0, 0 },
};
sint16 inv_main_objects = 8;
sint16 inv_main_select = -1;
sint16 inv_main_qtys[24] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
INVENTORY_ITEM *inv_main_list[23] = {
    &InvCompassOption,
    &InvFlareOption,
    &InvPistolOption,
    &InvShotgunOption,
    &InvMagnumOption,
    &InvUziOption,
    &InvM16Option,
    &InvGrenadeOption,
    &InvHarpoonOption,
    &InvLargeMedipackOption,
    &InvSmallMedipackOption,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};
sint16 inv_keys_select = -1;
sint16 inv_keys_qtys[24] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
INVENTORY_ITEM *inv_keys_list[23] = {
    &InvPuzzle1Option,
    &InvPuzzle2Option,
    &InvPuzzle3Option,
    &InvPuzzle4Option,
    &InvKey1Option,
    &InvKey2Option,
    &InvKey3Option,
    &InvKey4Option,
    &InvPickup1Option,
    &InvPickup2Option,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};
sint16 InvOptionObjectsCount = 4;
INVENTORY_ITEM *InvOptionList[4] = {
    &InvPassportOption,
    &InvControlOption,
    &InvSoundOption,
    &InvPhotoOption,
};
int GymInvOpenEnabled = 1;
REQUEST_INFO Load_Game_Requester = {
    0, 1, 0, 5, 0, 0, 0x128, 18, 0, -32, 0, 50, LoadGameStrings1[0], LoadGameStrings2[0], NULL, NULL, 0,
    0, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, NULL, NULL,
    NULL, NULL, NULL,
    {
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    },
    {
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    },
    "", "", 0, 0,
};
REQUEST_INFO SaveGameRequester = {
    0, 1, 0, 5, 0, 0, 0x110, 18, 0, -32, 0, 50, SaveGameStrings1[0], SaveGameStrings2[0],
    NULL, NULL, 0, 0, 0, 0, 0,
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, NULL, NULL,
    NULL, NULL, NULL,
    {
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    },
    {
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    },
    "", "", 0, 0,
};
#endif
