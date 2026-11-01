#include "game.h"

#include "inventry.h"
#include "specific/display.h"

#define TITLE_RING_OBJECTS 4
#define OPTION_RING_OBJECTS 3

void S_SetupAboveWater(int underwater);

extern int HiRes;
extern sint16 IsInventoryActive;
extern sint16 InventoryChosen;
extern int InventoryExtraData[];
extern sint16 inv_main_objects, inv_main_current;
extern sint16 InvOptionObjectsCount, InvOptionCurrent;
extern INVENTORY_ITEM *inv_main_list[];
extern INVENTORY_ITEM *InvOptionList[];
extern int GymInvOpenEnabled;

#include "savegame.h"

typedef struct imotion_info {
    sint16 count;
    sint16 status;
    sint16 status_target;
    sint16 radius_target;
    sint16 radius_rate;
    sint16 camera_ytarget;
    sint16 camera_yrate;
    sint16 camera_pitch_target;
    sint16 camera_pitch_rate;
    sint16 rotate_target;
    sint16 rotate_rate;
    PHD_ANGLE item_ptxrot_target;
    PHD_ANGLE item_ptxrot_rate;
    PHD_ANGLE item_xrot_target;
    PHD_ANGLE item_xrot_rate;
    sint32 item_ytrans_target;
    sint32 item_ytrans_rate;
    sint32 item_ztrans_target;
    sint32 item_ztrans_rate;
    sint32 misc;
} IMOTION_INFO;

typedef struct ring_info {
    INVENTORY_ITEM **list;
    sint16 type;
    sint16 radius;
    sint16 camera_pitch;
    sint16 rotating;
    sint16 rot_count;
    sint16 current_object;
    sint16 target_object;
    sint16 number_of_objects;
    sint16 angle_adder;
    sint16 rot_adder;
    sint16 rot_adderL;
    sint16 rot_adderR;
    PHD_3DPOS ringpos;
    PHD_3DPOS camera;
    PHD_VECTOR light;
    IMOTION_INFO *imo;
} RING_INFO;

#define MAIN_RING 0
#define OPTION_RING 1
#define KEYS_RING 2

#define RNG_OPENING 0
#define RNG_OPEN 1
#define RNG_CLOSING 2
#define RNG_MAIN2OPTION 3
#define RNG_MAIN2KEYS 4
#define RNG_KEYS2MAIN 5
#define RNG_OPTION2MAIN 6
#define RNG_SELECTING 7
#define RNG_SELECTED 8
#define RNG_DESELECTING 9
#define RNG_DESELECT 10
#define RNG_CLOSING_ITEM 11
#define RNG_EXITING_INVENTORY 12
#define RNG_DONE 13

#define ROTATE_DURATION 24
#define RINGSWITCH_FRAMES 48
#define SELECTING_FRAMES 16
#define CLOSE_FRAMES 32
#define OPEN_ROTATION -0x8000
#define CLOSE_ROTATION -0x8000
#define CAMERA_STARTHEIGHT -0x600
#define CAMERA_2_RING 0x256
#define RING_RADIUS 688
#define ITEM_TILT 0
#define LOW_LIGHT 0x1400
#define HIGH_LIGHT 0x1000
#define EXIT_INVENTORY (IN_DESELECT | IN_OPTION)

#define PASSPORT_OPTION 120
#define MAP_OPTION 121
#define PHOTO_OPTION 122
#define PASSPORT_CLOSED 133
#define DETAIL_OPTION 153
#define SOUND_OPTION 154
#define CONTROL_OPTION 155
#define GAMMA_OPTION 156
#define GUN_OPTION 157
#define SHOTGUN_OPTION 158
#define MAGNUM_OPTION 159
#define UZI_OPTION 160
#define HARPOON_OPTION 161
#define M16_OPTION 162
#define ROCKET_OPTION 163
#define MEDI_OPTION 171
#define BIGMEDI_OPTION 172
#define FLAREBOX_OPTION 173

extern TEXTSTRING *AmmoTextInfo;
extern sint32 inv_nframes;
extern sint32 InvDemoMode;
extern sint32 NoInputCounter;
extern sint32 IsResetFlag;
extern sint32 StopInventory;
extern sint32 LsAdder;
extern sint32 inputDB;
extern sint16 cdtrack;
extern sint16 Option_Music_Volume;
extern INVENTORY_ITEM *inv_keys_list[];
extern sint16 inv_keys_objects, inv_keys_current;

void S_FadeInInventory(int fade);
void S_FadeOutInventory(int fade);
void S_FinishInventory(void);
void S_CDVolume(sint32 volume);
void S_CDLoop(void);
void S_UpdateInput(void);
void GetDemoInput(void);
sint32 GetDebouncedInput(sint32 in);
void S_InitialisePolyList(int clear);
void S_OutputPolyList(void);
void S_AnimateTextures(sint32 nframes);
void SOUND_EndScene(void);
void DoInventoryPicture(void);
void DoInventoryBackground(void);
void DrawModeInfo(void);
void T_DrawText(void);
void UseItem(sint16 object_num);
void do_inventory_options(INVENTORY_ITEM *inv_item);
void RemoveInventoryText(void);
void RingIsOpen(RING_INFO *ring);
void RingIsNotOpen(RING_INFO *ring);
void RingNotActive(INVENTORY_ITEM *inv_item);
void RingActive(void);
void DrawInventoryItem(INVENTORY_ITEM *inv_item);
sint32 AnimateInventoryItem(INVENTORY_ITEM *inv_item);
void phd_GenerateW2V(PHD_3DPOS *viewpos);
void phd_RotYXZ(PHD_ANGLE ry, PHD_ANGLE rx, PHD_ANGLE rz);
void Inv_RingInit(RING_INFO *ring, sint16 type, INVENTORY_ITEM **list, sint16 qty, sint16 current, IMOTION_INFO *imo);
void Inv_RingGetView(RING_INFO *ring, PHD_3DPOS *viewer);
void Inv_RingLight(RING_INFO *ring);
void Inv_RingCalcAdders(RING_INFO *ring, sint16 rotation_duration);
void Inv_RingDoMotions(RING_INFO *ring);
void Inv_RingRotateLeft(RING_INFO *ring);
void Inv_RingRotateRight(RING_INFO *ring);
void Inv_RingMotionSetup(RING_INFO *ring, sint16 status, sint16 status_target, sint16 frames);
void Inv_RingMotionRadius(RING_INFO *ring, sint16 target);
void Inv_RingMotionRotation(RING_INFO *ring, sint16 rotation, sint16 target);
void Inv_RingMotionCameraPos(RING_INFO *ring, sint16 target);
void Inv_RingMotionCameraPitch(RING_INFO *ring, sint16 target);
void Inv_RingMotionItemSelect(RING_INFO *ring, INVENTORY_ITEM *inv_item);
void Inv_RingMotionItemDeselect(RING_INFO *ring, INVENTORY_ITEM *inv_item);
void Construct_Inventory(void);

sint32 Display_Inventory(sint32 inventory_mode)
{
    int demo_needed = 0;
    int n, frm, busy = 0, pass_open = 0;
    PHD_3DPOS viewer;
    sint16 angle;
    sint32 tempSint32;
    sint16 icam2obj = CAMERA_2_RING;
    INVENTORY_ITEM *inv_item;
    sint16 itilt = ITEM_TILT;
    IMOTION_INFO imo = {0};
    RING_INFO ring = {0};

    if (inventory_mode == INV_KEYS_MODE && !inv_keys_objects) {
        InventoryChosen = -1;
        return 0;
    }

    T_RemovePrint(AmmoTextInfo);
    AmmoTextInfo = NULL;
    AlterFOV(GAME_FOV * ONE_DEGREE);
    Inventory_Mode = inventory_mode;
    inv_nframes = 2;
    Construct_Inventory();

    if (inventory_mode != INV_TITLE_MODE)
        S_FadeInInventory(1);
    else
        S_FadeInInventory(0);

    SOUND_Stop();
    if (inventory_mode != INV_TITLE_MODE)
        S_CDVolume(0);

    switch (inventory_mode) {
    case INV_DEATH_MODE:
    case INV_SAVE_MODE:
    case INV_LOAD_MODE:
    case INV_TITLE_MODE:
        Inv_RingInit(&ring, OPTION_RING, InvOptionList, InvOptionObjectsCount, InvOptionCurrent, &imo);
        break;
    case INV_KEYS_MODE:
        Inv_RingInit(&ring, KEYS_RING, inv_keys_list, inv_keys_objects, inv_main_current, &imo);
        break;
    default:
        if (inv_main_objects)
            Inv_RingInit(&ring, MAIN_RING, inv_main_list, inv_main_objects, inv_main_current, &imo);
        else
            Inv_RingInit(&ring, OPTION_RING, InvOptionList, InvOptionObjectsCount, InvOptionCurrent, &imo);
        break;
    }

    SoundEffect(111, NULL, SFX_ALWAYS);
    inv_nframes = 2;

    do {
        if (inventory_mode == INV_TITLE_MODE && cdtrack > 0)
            S_CDLoop();

        Inv_RingCalcAdders(&ring, ROTATE_DURATION);
        S_UpdateInput();

        if (InvDemoMode) {
            if (input)
                return gameflow.on_demo_interrupt;
            GetDemoInput();
            if (input == -1)
                return gameflow.on_demo_end;
        } else {
            if (input)
                NoInputCounter = 0;
        }

        inputDB = GetDebouncedInput(input);

        if (Inventory_Mode == INV_TITLE_MODE && !input && !inputDB) {
            if (gameflow.num_demos || gameflow.noinput_timeout) {
                NoInputCounter++;
                if (NoInputCounter > gameflow.noinput_time)
                    IsResetFlag = demo_needed = 1;
            }
        } else {
            NoInputCounter = 0;
            IsResetFlag = 0;
        }

        if (StopInventory)
            return EXIT_TO_TITLE;

        if ((Inventory_Mode == INV_SAVE_MODE || Inventory_Mode == INV_LOAD_MODE || Inventory_Mode == INV_DEATH_MODE) && !pass_open) {
            inputDB = IN_SELECT;
            input = 0;
        }

        for (frm = 0; frm < inv_nframes; frm++) {
            if (idelay) {
                if (idcount)
                    idcount--;
                else
                    idelay = 0;
            }
            Inv_RingDoMotions(&ring);
        }

        ring.camera.z_pos = ring.radius + icam2obj;

        S_InitialisePolyList(0);
        if (Inventory_Mode == INV_TITLE_MODE)
            DoInventoryPicture();
        else
            DoInventoryBackground();
        S_AnimateTextures(inv_nframes);

        Inv_RingGetView(&ring, &viewer);
        phd_GenerateW2V(&viewer);
        Inv_RingLight(&ring);

        phd_PushMatrix();
        phd_TranslateAbs(ring.ringpos.x_pos, ring.ringpos.y_pos, ring.ringpos.z_pos);
        phd_RotYXZ(ring.ringpos.y_rot, ring.ringpos.x_rot, ring.ringpos.z_rot);

        for (n = 0, angle = 0; n < ring.number_of_objects; n++) {
            inv_item = *(ring.list + n);

            if (n == ring.current_object) {
                for (frm = 0; frm < inv_nframes; frm++) {
                    if (ring.rotating) {
                        LsAdder = LOW_LIGHT;
                        if (inv_item->y_rot != 0) {
                            if (inv_item->y_rot < 0)
                                inv_item->y_rot += 0x200;
                            else
                                inv_item->y_rot -= 0x200;
                        }
                    } else if (imo.status == RNG_SELECTED || imo.status == RNG_DESELECTING || imo.status == RNG_SELECTING || imo.status == RNG_DESELECT || imo.status == RNG_CLOSING_ITEM) {
                        LsAdder = HIGH_LIGHT;
                        if (inv_item->y_rot != inv_item->y_rot_sel) {
                            tempSint32 = inv_item->y_rot_sel - inv_item->y_rot;
                            if (tempSint32 > 0 && tempSint32 < 0x8000)
                                inv_item->y_rot += 0x400;
                            else
                                inv_item->y_rot -= 0x400;
                            inv_item->y_rot &= 0xfc00;
                        }
                    } else if (ring.number_of_objects == 1 || !(input & IN_LEFT || input & IN_RIGHT)) {
                        LsAdder = HIGH_LIGHT;
                        inv_item->y_rot += 0x100;
                    }
                }

                if ((imo.status == RNG_OPEN || imo.status == RNG_SELECTING || imo.status == RNG_SELECTED || imo.status == RNG_DESELECTING || imo.status == RNG_DESELECT || imo.status == RNG_CLOSING_ITEM) && !ring.rotating && !(input & IN_LEFT || input & IN_RIGHT))
                    RingNotActive(inv_item);
            } else {
                LsAdder = LOW_LIGHT;
                for (frm = 0; frm < inv_nframes; frm++) {
                    if (inv_item->y_rot != 0) {
                        if (inv_item->y_rot < 0)
                            inv_item->y_rot += 0x100;
                        else
                            inv_item->y_rot -= 0x100;
                    }
                }
            }

            if (imo.status == RNG_OPEN || imo.status == RNG_SELECTING || imo.status == RNG_SELECTED || imo.status == RNG_DESELECTING || imo.status == RNG_DESELECT || imo.status == RNG_CLOSING_ITEM)
                RingIsOpen(&ring);
            else
                RingIsNotOpen(&ring);

            if (imo.status == RNG_OPENING || imo.status == RNG_CLOSING || imo.status == RNG_MAIN2OPTION || imo.status == RNG_OPTION2MAIN || imo.status == RNG_EXITING_INVENTORY || imo.status == RNG_DONE || ring.rotating)
                RingActive();

            phd_PushMatrix();
            phd_RotYXZ(angle, 0, 0);
            phd_TranslateRel(ring.radius, 0, 0);
            phd_RotYXZ(0x4000, (sint16)(itilt + inv_item->pt_xrot), 0);
            DrawInventoryItem(inv_item);
            phd_PopMatrix();
            angle += ring.angle_adder;
            angle &= 0xffff;
        }

        phd_PopMatrix();
        DrawModeInfo();
        T_DrawText();
        S_OutputPolyList();
        SOUND_EndScene();

        camera.number_frames = inv_nframes = S_DumpScreen();
        if (CurrentLevel)
            savegame.timer += inv_nframes / 2;

        if (!ring.rotating) {
            switch (imo.status) {
            case RNG_OPEN:
                if ((input & IN_RIGHT) && ring.number_of_objects > 1) {
                    Inv_RingRotateLeft(&ring);
                    SoundEffect(108, NULL, SFX_ALWAYS);
                    break;
                }

                if ((input & IN_LEFT) && ring.number_of_objects > 1) {
                    Inv_RingRotateRight(&ring);
                    SoundEffect(108, NULL, SFX_ALWAYS);
                    break;
                }

                if (IsResetFlag || (inputDB & EXIT_INVENTORY)) {
                    if (IsResetFlag || Inventory_Mode != INV_TITLE_MODE) {
                        SoundEffect(112, NULL, SFX_ALWAYS);
                        InventoryChosen = -1;
                        if (ring.type == MAIN_RING)
                            inv_main_current = ring.current_object;
                        else
                            InvOptionCurrent = ring.current_object;

                        if (inventory_mode != INV_TITLE_MODE)
                            S_FadeOutInventory(1);
                        else
                            S_FadeOutInventory(0);

                        Inv_RingMotionSetup(&ring, RNG_CLOSING, RNG_DONE, CLOSE_FRAMES);
                        Inv_RingMotionRadius(&ring, 0);
                        Inv_RingMotionCameraPos(&ring, CAMERA_STARTHEIGHT);
                        Inv_RingMotionRotation(&ring, CLOSE_ROTATION, (sint16)(ring.ringpos.y_rot - CLOSE_ROTATION));
                        input = 0;
                        inputDB = 0;
                    }
                }

                if (inputDB & IN_SELECT) {
                    if ((Inventory_Mode == INV_SAVE_MODE || Inventory_Mode == INV_LOAD_MODE || Inventory_Mode == INV_DEATH_MODE) && !pass_open)
                        pass_open = 1;

                    item_data = 0;
                    if (ring.type == MAIN_RING) {
                        inv_main_current = ring.current_object;
                        inv_item = inv_main_list[ring.current_object];
                    } else if (ring.type == OPTION_RING) {
                        InvOptionCurrent = ring.current_object;
                        inv_item = InvOptionList[ring.current_object];
                    } else {
                        inv_keys_current = ring.current_object;
                        inv_item = inv_keys_list[ring.current_object];
                    }

                    inv_item->goal_frame = inv_item->open_frame;
                    inv_item->anim_direction = 1;
                    Inv_RingMotionSetup(&ring, RNG_SELECTING, RNG_SELECTED, SELECTING_FRAMES);
                    Inv_RingMotionRotation(&ring, 0, (sint16)(0xc000 - ring.current_object * ring.angle_adder));
                    Inv_RingMotionItemSelect(&ring, inv_item);
                    input = 0;
                    inputDB = 0;

                    switch (inv_item->object_number) {
                    case PHOTO_OPTION:
                        SoundEffect(109, NULL, SFX_ALWAYS);
                        break;
                    case MAP_OPTION:
                        SoundEffect(113, NULL, SFX_ALWAYS);
                        break;
                    case GUN_OPTION:
                    case SHOTGUN_OPTION:
                    case MAGNUM_OPTION:
                    case UZI_OPTION:
                    case HARPOON_OPTION:
                    case M16_OPTION:
                    case ROCKET_OPTION:
                        SoundEffect(114, NULL, SFX_ALWAYS);
                        break;
                    default:
                        SoundEffect(111, NULL, SFX_ALWAYS);
                        break;
                    }
                }

                if (inputDB & IN_FORWARD && inventory_mode != INV_TITLE_MODE && inventory_mode != INV_KEYS_MODE) {
                    if (ring.type == MAIN_RING) {
                        if (inv_keys_objects) {
                            Inv_RingMotionSetup(&ring, RNG_CLOSING, RNG_MAIN2KEYS, RINGSWITCH_FRAMES / 2);
                            Inv_RingMotionRadius(&ring, 0);
                            Inv_RingMotionRotation(&ring, CLOSE_ROTATION, (sint16)(ring.ringpos.y_rot - CLOSE_ROTATION));
                            Inv_RingMotionCameraPitch(&ring, 0x2000);
                            imo.misc = 0x2000;
                        }
                        input = 0;
                        inputDB = 0;
                    } else if (ring.type == OPTION_RING) {
                        if (inv_main_objects) {
                            Inv_RingMotionSetup(&ring, RNG_CLOSING, RNG_OPTION2MAIN, RINGSWITCH_FRAMES / 2);
                            Inv_RingMotionRadius(&ring, 0);
                            Inv_RingMotionRotation(&ring, CLOSE_ROTATION, (sint16)(ring.ringpos.y_rot - CLOSE_ROTATION));
                            Inv_RingMotionCameraPitch(&ring, 0x2000);
                            imo.misc = 0x2000;
                        }
                        inputDB = 0;
                    }
                } else if (inputDB & IN_BACK && inventory_mode != INV_TITLE_MODE && inventory_mode != INV_KEYS_MODE) {
                    if (ring.type == KEYS_RING) {
                        if (inv_main_objects) {
                            Inv_RingMotionSetup(&ring, RNG_CLOSING, RNG_KEYS2MAIN, RINGSWITCH_FRAMES / 2);
                            Inv_RingMotionRadius(&ring, 0);
                            Inv_RingMotionRotation(&ring, CLOSE_ROTATION, (sint16)(ring.ringpos.y_rot - CLOSE_ROTATION));
                            Inv_RingMotionCameraPitch(&ring, 0 - 0x2000);
                            imo.misc = 0 - 0x2000;
                        }
                        input = 0;
                        inputDB = 0;
                    } else if (ring.type == MAIN_RING) {
                        if (InvOptionObjectsCount && !gameflow.lockout_optionring) {
                            Inv_RingMotionSetup(&ring, RNG_CLOSING, RNG_MAIN2OPTION, RINGSWITCH_FRAMES / 2);
                            Inv_RingMotionRadius(&ring, 0);
                            Inv_RingMotionRotation(&ring, CLOSE_ROTATION, (sint16)(ring.ringpos.y_rot - CLOSE_ROTATION));
                            Inv_RingMotionCameraPitch(&ring, 0 - 0x2000);
                            imo.misc = 0 - 0x2000;
                        }
                        inputDB = 0;
                    }
                }
                break;

            case RNG_MAIN2OPTION:
                Inv_RingMotionSetup(&ring, RNG_OPENING, RNG_OPEN, RINGSWITCH_FRAMES / 2);
                Inv_RingMotionRadius(&ring, RING_RADIUS);
                imo.camera_pitch_target = 0;
                ring.camera_pitch = -(sint16)imo.misc;
                imo.camera_pitch_rate = imo.misc / (RINGSWITCH_FRAMES / 2);
                ring.list = InvOptionList;
                ring.type = OPTION_RING;
                inv_main_current = ring.current_object;
                ring.number_of_objects = InvOptionObjectsCount;
                ring.current_object = InvOptionCurrent;
                Inv_RingCalcAdders(&ring, ROTATE_DURATION);
                Inv_RingMotionRotation(&ring, OPEN_ROTATION, (sint16)(0xc000 - ring.current_object * ring.angle_adder));
                ring.ringpos.y_rot = imo.rotate_target + OPEN_ROTATION;
                break;

            case RNG_OPTION2MAIN:
                Inv_RingMotionSetup(&ring, RNG_OPENING, RNG_OPEN, RINGSWITCH_FRAMES / 2);
                Inv_RingMotionRadius(&ring, RING_RADIUS);
                imo.camera_pitch_target = 0;
                ring.camera_pitch = -(sint16)imo.misc;
                imo.camera_pitch_rate = imo.misc / (RINGSWITCH_FRAMES / 2);
                InvOptionObjectsCount = ring.number_of_objects;
                InvOptionCurrent = ring.current_object;
                ring.list = inv_main_list;
                ring.type = MAIN_RING;
                ring.number_of_objects = inv_main_objects;
                ring.current_object = inv_main_current;
                Inv_RingCalcAdders(&ring, ROTATE_DURATION);
                Inv_RingMotionRotation(&ring, OPEN_ROTATION, (sint16)(0xc000 - ring.current_object * ring.angle_adder));
                ring.ringpos.y_rot = imo.rotate_target + OPEN_ROTATION;
                break;

            case RNG_MAIN2KEYS:
                Inv_RingMotionSetup(&ring, RNG_OPENING, RNG_OPEN, RINGSWITCH_FRAMES / 2);
                Inv_RingMotionRadius(&ring, RING_RADIUS);
                imo.camera_pitch_target = 0;
                ring.camera_pitch = -(sint16)imo.misc;
                imo.camera_pitch_rate = imo.misc / (RINGSWITCH_FRAMES / 2);
                inv_main_objects = ring.number_of_objects;
                inv_main_current = ring.current_object;
                ring.list = inv_keys_list;
                ring.type = KEYS_RING;
                ring.number_of_objects = inv_keys_objects;
                ring.current_object = inv_keys_current;
                Inv_RingCalcAdders(&ring, ROTATE_DURATION);
                Inv_RingMotionRotation(&ring, OPEN_ROTATION, (sint16)(0xc000 - ring.current_object * ring.angle_adder));
                ring.ringpos.y_rot = imo.rotate_target + OPEN_ROTATION;
                break;

            case RNG_KEYS2MAIN:
                Inv_RingMotionSetup(&ring, RNG_OPENING, RNG_OPEN, RINGSWITCH_FRAMES / 2);
                Inv_RingMotionRadius(&ring, RING_RADIUS);
                imo.camera_pitch_target = 0;
                ring.camera_pitch = -(sint16)imo.misc;
                imo.camera_pitch_rate = imo.misc / (RINGSWITCH_FRAMES / 2);
                ring.list = inv_main_list;
                ring.type = MAIN_RING;
                inv_keys_current = ring.current_object;
                ring.number_of_objects = inv_main_objects;
                ring.current_object = inv_main_current;
                Inv_RingCalcAdders(&ring, ROTATE_DURATION);
                Inv_RingMotionRotation(&ring, OPEN_ROTATION, (sint16)(0xc000 - ring.current_object * ring.angle_adder));
                ring.ringpos.y_rot = imo.rotate_target + OPEN_ROTATION;
                break;

            case RNG_CLOSING_ITEM:
                inv_item = *(ring.list + ring.current_object);
                for (frm = 0; frm < inv_nframes; frm++) {
                    if (!AnimateInventoryItem(inv_item)) {
                        if (inv_item->object_number == PASSPORT_OPTION) {
                            inv_item->object_number = PASSPORT_CLOSED;
                            inv_item->current_frame = 0;
                        }
                        imo.count = SELECTING_FRAMES;
                        imo.status = imo.status_target;
                        Inv_RingMotionItemDeselect(&ring, inv_item);
                        break;
                    }
                }
                break;

            case RNG_DESELECT:
                SoundEffect(112, NULL, SFX_ALWAYS);
                Inv_RingMotionSetup(&ring, RNG_DESELECTING, RNG_OPEN, SELECTING_FRAMES);
                Inv_RingMotionRotation(&ring, 0, (sint16)(0xc000 - ring.current_object * ring.angle_adder));
                input = 0;
                inputDB = 0;
                break;

            case RNG_SELECTED:
                inv_item = *(ring.list + ring.current_object);
                if (inv_item->object_number == PASSPORT_CLOSED)
                    inv_item->object_number = PASSPORT_OPTION;

                for (frm = 0; frm < inv_nframes; frm++) {
                    busy = 0;
                    if (inv_item->y_rot == inv_item->y_rot_sel)
                        busy = AnimateInventoryItem(inv_item);
                }

                if (!busy && !idelay) {
                    do_inventory_options(inv_item);

                    if (inputDB & IN_DESELECT) {
                        inv_item->sprlist = NULL;
                        Inv_RingMotionSetup(&ring, RNG_CLOSING_ITEM, RNG_DESELECT, 0);
                        input = 0;
                        inputDB = 0;
                        if (Inventory_Mode == INV_LOAD_MODE || Inventory_Mode == INV_SAVE_MODE) {
                            Inv_RingMotionSetup(&ring, RNG_CLOSING_ITEM, RNG_EXITING_INVENTORY, 0);
                            input = inputDB = 0;
                        }
                    }

                    if (inputDB & IN_SELECT) {
                        inv_item->sprlist = NULL;
                        InventoryChosen = inv_item->object_number;
                        if (ring.type == MAIN_RING)
                            inv_main_current = ring.current_object;
                        else
                            InvOptionCurrent = ring.current_object;

                        if (Inventory_Mode == INV_TITLE_MODE && (inv_item->object_number == DETAIL_OPTION || inv_item->object_number == SOUND_OPTION || inv_item->object_number == CONTROL_OPTION || inv_item->object_number == GAMMA_OPTION))
                            Inv_RingMotionSetup(&ring, RNG_CLOSING_ITEM, RNG_DESELECT, 0);
                        else
                            Inv_RingMotionSetup(&ring, RNG_CLOSING_ITEM, RNG_EXITING_INVENTORY, 0);
                        input = 0;
                        inputDB = 0;
                    }
                }
                break;

            case RNG_EXITING_INVENTORY:
                if (!imo.count) {
                    if (inventory_mode != INV_TITLE_MODE)
                        S_FadeOutInventory(1);
                    else
                        S_FadeOutInventory(0);
                    Inv_RingMotionSetup(&ring, RNG_CLOSING, RNG_DONE, CLOSE_FRAMES);
                    Inv_RingMotionRadius(&ring, 0);
                    Inv_RingMotionCameraPos(&ring, CAMERA_STARTHEIGHT);
                    Inv_RingMotionRotation(&ring, CLOSE_ROTATION, (sint16)(ring.ringpos.y_rot - CLOSE_ROTATION));
                }
                break;

            default:
                break;
            }
        }
    } while (imo.status != RNG_DONE);

    RemoveInventoryText();
    S_FinishInventory();
    IsInventoryActive = 0;

    if (IsResetFlag)
        return EXIT_TO_TITLE;

    if (demo_needed)
        return STARTDEMO;

    if (InventoryChosen == -1) {
        if (inventory_mode != INV_TITLE_MODE && Option_Music_Volume)
            S_CDVolume(Option_Music_Volume * 25 + 5);
        return 0;
    }

    switch (InventoryChosen) {
    case PASSPORT_OPTION:
        if (InventoryExtraData[0] == 1 && Option_Music_Volume)
            S_CDVolume(Option_Music_Volume * 25 + 5);
        return 1;
    case PHOTO_OPTION:
        if (gameflow.gym_enabled) {
            InventoryExtraData[1] = 0;
            return 1;
        }
        break;
    case GUN_OPTION:
        UseItem(GUN_OPTION);
        break;
    case SHOTGUN_OPTION:
        UseItem(SHOTGUN_OPTION);
        break;
    case MAGNUM_OPTION:
        UseItem(MAGNUM_OPTION);
        break;
    case UZI_OPTION:
        UseItem(UZI_OPTION);
        break;
    case HARPOON_OPTION:
        UseItem(HARPOON_OPTION);
        break;
    case M16_OPTION:
        UseItem(M16_OPTION);
        break;
    case ROCKET_OPTION:
        UseItem(ROCKET_OPTION);
        break;
    case FLAREBOX_OPTION:
        UseItem(FLAREBOX_OPTION);
        break;
    case MEDI_OPTION:
        UseItem(MEDI_OPTION);
        break;
    case BIGMEDI_OPTION:
        UseItem(BIGMEDI_OPTION);
        break;
    default:
        break;
    }

    if (inventory_mode != INV_TITLE_MODE && Option_Music_Volume)
        S_CDVolume(Option_Music_Volume * 25 + 5);
    return 0;
}

void Construct_Inventory(void)
{
    int n;
    INVENTORY_ITEM *inv_item;

    S_SetupAboveWater(0);
    if (Inventory_Mode != INV_TITLE_MODE)
        TempVideoAdjust(HiRes, 1.0);

    phd_left = 0;
    phd_top = 0;
    phd_right = phd_winxmax;
    phd_bottom = phd_winymax;

    IsInventoryActive = 1;
    InventoryChosen = 0;
    for (n = 0; n < 8; n++)
        InventoryExtraData[n] = 0;

    if (Inventory_Mode == INV_TITLE_MODE)
    {
        if (gameflow.gym_enabled)
            InvOptionObjectsCount = TITLE_RING_OBJECTS;
        else
            InvOptionObjectsCount = TITLE_RING_OBJECTS - 1;
    }
    else
        InvOptionObjectsCount = OPTION_RING_OBJECTS;

    for (n = 0; n < inv_main_objects; n++)
    {
        inv_item = inv_main_list[n];
        inv_item->drawn_meshes = inv_item->which_meshes;
        inv_item->current_frame = 0;
        inv_item->goal_frame = inv_item->current_frame;
        inv_item->anim_count = 0;
        inv_item->y_rot = 0;
    }

    for (n = 0; n < InvOptionObjectsCount; n++)
    {
        inv_item = InvOptionList[n];
        inv_item->current_frame = 0;
        inv_item->goal_frame = inv_item->current_frame;
        inv_item->anim_count = 0;
        inv_item->y_rot = 0;
    }

    inv_main_current = 0;
    if (GymInvOpenEnabled && Inventory_Mode == INV_TITLE_MODE && !gameflow.loadsave_disabled && gameflow.gym_enabled)
        InvOptionCurrent = 3;
    else
    {
        InvOptionCurrent = 0;
        GymInvOpenEnabled = 0;
    }
    item_data = 0;
}

#define PASSPORT_OPTION 120
#define MAP_OPTION 121
#define PFRONT 1
#define PSPINE 2
#define PINFRONT 4
#define PPAGE2 8
#define PBACK 16
#define PINBACK 32
#define PPAGE1 64
#define PASS_MESH (PFRONT | PSPINE | PBACK)

void SelectMeshes(INVENTORY_ITEM *inv_item)
{
    int frame;

    if (inv_item->object_number == PASSPORT_OPTION) {
        frame = inv_item->current_frame;
        if (frame < 4)
            inv_item->drawn_meshes = PASS_MESH | PINFRONT;
        else if (frame <= 16)
            inv_item->drawn_meshes = PASS_MESH | PINFRONT | PPAGE1;
        else if (frame < 19)
            inv_item->drawn_meshes = PASS_MESH | PINFRONT | PPAGE1 | PPAGE2;
        else if (frame == 19)
            inv_item->drawn_meshes = PASS_MESH | PPAGE1 | PPAGE2;
        else if (frame < 24)
            inv_item->drawn_meshes = PASS_MESH | PPAGE1 | PPAGE2 | PINBACK;
        else if (frame < 29)
            inv_item->drawn_meshes = PASS_MESH | PPAGE2 | PINBACK;
        else if (frame == 29)
            inv_item->drawn_meshes = PASS_MESH;
    } else if (inv_item->object_number == MAP_OPTION) {
        frame = inv_item->current_frame;
        if (frame && frame < 18)
            inv_item->drawn_meshes = 0xffffffff;
        else
            inv_item->drawn_meshes = inv_item->which_meshes;
    } else
        inv_item->drawn_meshes = 0xffffffff;
}

sint32 AnimateInventoryItem(INVENTORY_ITEM *inv_item)
{
    if (inv_item->current_frame != inv_item->goal_frame) {
        if (inv_item->anim_count)
            inv_item->anim_count--;
        else {
            inv_item->anim_count = inv_item->anim_speed;
            inv_item->current_frame += inv_item->anim_direction;
            if (inv_item->current_frame >= inv_item->frames_total)
                inv_item->current_frame = 0;
            else if (inv_item->current_frame < 0)
                inv_item->current_frame = inv_item->frames_total - 1;
        }
        SelectMeshes(inv_item);
        return 1;
    }
    SelectMeshes(inv_item);
    return 0;
}

#include "savegame.h"

#define ALPHABET 255
#define SHAPE_SPRITE 1
#define SHAPE_LINE 2
#define SHAPE_BOX 3
#define SHAPE_FBOX 4

typedef struct {
    sint16 mesh_number;
    uint16 flags;
    sint16 x_minp, x_maxp, y_minp, y_maxp, z_minp, z_maxp;
    sint16 x_minc, x_maxc, y_minc, y_maxc, z_minc, z_maxc;
} STATIC_INFO;

extern STATIC_INFO StaticObjects[];
extern sint32 PhdPersp;

void phd_RotYXZ(PHD_ANGLE ry, PHD_ANGLE rx, PHD_ANGLE rz);
void phd_RotZ(PHD_ANGLE rz);
void phd_RotYXZsuperpack(sint16 **pprot, int skip);
int S_GetObjectBounds(sint16 *bptr);
void S_DrawSprite(uint32 dwFlags, sint32 nX, sint32 nY, sint32 nZ, sint16 nSprite, sint16 nShade, sint16 nScale);
void S_DrawScreenSprite(sint32 sx, sint32 sy, sint32 z, sint32 scaleH, sint32 scaleV, sint16 sprnum, sint16 shade, uint16 flags);
void S_DrawScreenLine(sint32 sx, sint32 sy, sint32 z, sint32 w, sint32 h, sint32 col, uint16 *grdptr, uint16 flags);
void S_DrawScreenBox(sint32 sx, sint32 sy, sint32 z, sint32 w, sint32 h, sint32 col, uint16 *grdptr, uint16 flags);
void S_DrawScreenFBox(sint32 sx, sint32 sy, sint32 z, sint32 w, sint32 h, sint32 col, uint16 *grdptr, uint16 flags);

void DrawInventoryItem(INVENTORY_ITEM *inv_item)
{
    int hours, minutes, seconds, totsec;
    int clip, i, poppush;
    sint16 mesh, mesh_num;
    OBJECT_INFO *object;
    sint32 *bone;
    sint16 *rotation;
    sint16 *frame;
    INVENTORY_SPRITE *isprite, **isprite_list;
    sint32 xv, yv, zv, zp;
    sint32 sx, sy;

    if (inv_item->object_number == MAP_OPTION)
    {
        totsec = savegame.timer / 30;
        seconds = totsec % 60;
        minutes = ((totsec % 3600) * -91) / 5;
        hours = ((totsec / 12) * -91) / 5;
        seconds *= -1092;
    }
    else
        hours = minutes = seconds = totsec = 0;

    phd_TranslateRel(0, inv_item->ytrans, inv_item->ztrans);
    phd_RotYXZ(inv_item->y_rot, inv_item->x_rot, 0);

    object = &objects[inv_item->object_number];
    if (!object->loaded)
        return;

    if (object->nmeshes < 0)
    {
        mesh = object->mesh_index;
        S_DrawSprite(0, 0, 0, 0, mesh, 0, 0);
        return;
    }

    if ((isprite_list = inv_item->sprlist))
    {
        zv = *(phd_mxptr + M23);
        xv = *(phd_mxptr + M03);
        yv = *(phd_mxptr + M13);
        zp = zv / PhdPersp;
        sx = (sint32)(xv / zp + phd_centerx);
        sy = (sint32)(yv / zp + phd_centery);
        while ((isprite = *isprite_list++))
        {
            if (zv < phd_znear || zv > phd_zfar)
                break;
            while (isprite->shape)
            {
                switch (isprite->shape)
                {
                case SHAPE_SPRITE:
                    S_DrawScreenSprite(sx + isprite->x, sy + isprite->y, isprite->z, isprite->param1, isprite->param2, (sint16)(StaticObjects[ALPHABET].mesh_number + isprite->sprnum), 16 * 256, 0);
                    break;
                case SHAPE_LINE:
                    S_DrawScreenLine(sx + isprite->x, sy + isprite->y, isprite->z, isprite->param1, isprite->param2, isprite->sprnum, isprite->grdptr, 0);
                    break;
                case SHAPE_BOX:
                    S_DrawScreenBox(sx + isprite->x, sy + isprite->y, isprite->z, isprite->param1, isprite->param2, isprite->sprnum, isprite->grdptr, 0);
                    break;
                case SHAPE_FBOX:
                    S_DrawScreenFBox(sx + isprite->x, sy + isprite->y, isprite->z, isprite->param1, isprite->param2, isprite->sprnum, isprite->grdptr, 0);
                    break;
                default:
                    break;
                }
                isprite++;
            }
        }
    }

    frame = object->frame_base + inv_item->current_frame * (anims[object->anim_index].interpolation >> 8);

    phd_PushMatrix();
    if ((clip = S_GetObjectBounds(frame)))
    {
        phd_TranslateRel((sint32)*(frame + 6), (sint32)*(frame + 7), (sint32)*(frame + 8));
        rotation = frame + 9;
        phd_RotYXZsuperpack(&rotation, 0);

        mesh = object->mesh_index;
        bone = bones + object->bone_index;

        mesh_num = 1;
        if (mesh_num & inv_item->drawn_meshes)
            phd_PutPolygons(meshes[mesh], clip);

        for (i = object->nmeshes - 1; i > 0; i--)
        {
            mesh++;
            mesh_num <<= 1;

            poppush = *(bone++);
            if (poppush & 1)
                phd_PopMatrix();
            if (poppush & 2)
                phd_PushMatrix();

            phd_TranslateRel(*(bone), *(bone + 1), *(bone + 2));
            phd_RotYXZsuperpack(&rotation, 0);

            if (inv_item->object_number == MAP_OPTION)
            {
                if (i == 2)
                {
                    phd_RotZ((PHD_ANGLE)seconds);
                    inv_item->misc_data[1] = inv_item->misc_data[0];
                    inv_item->misc_data[0] = seconds;
                }
                if (i == 3)
                    phd_RotZ((PHD_ANGLE)minutes);
                if (i == 4)
                    phd_RotZ((PHD_ANGLE)hours);
            }

            if (mesh_num & inv_item->drawn_meshes)
                phd_PutPolygons(meshes[mesh], clip);
            bone += 3;
        }
    }
    phd_PopMatrix();
}

extern sint32 oldInputDB;

sint32 GetDebouncedInput(sint32 input)
{
    sint32 j;

    j = input ^ (input & oldInputDB);
    oldInputDB = input;
    return input & j;
}

void DoInventoryPicture(void)
{
    S_CopyBufferToScreen();
}

#define MENU_BACKGROUND 223

void phd_GetVectorAngles(sint32 x, sint32 y, sint32 z, PHD_ANGLE *dest);
void phd_GenerateW2V(PHD_3DPOS *viewpos);
void phd_RotateLight(sint16 pitch, sint16 yaw);
void phd_RotYXZsuperpack(sint16 **pprot, int skip);
void S_InsertInvBgnd(sint16 *objptr);
extern sint32 ls_divider;

static int bz = 12288;

void DoInventoryBackground(void)
{
    PHD_ANGLE angles[2];
    sint16 *frame;
    PHD_3DPOS viewer;

    S_CopyBufferToScreen();
    if (!objects[MENU_BACKGROUND].loaded)
        return;

    phd_GetVectorAngles(0, 0x1000, 0, angles);
    viewer.x_pos = 0;
    viewer.y_pos = -0x200;
    viewer.z_pos = 0;
    viewer.x_rot = angles[1];
    viewer.y_rot = angles[0];
    viewer.z_rot = 0;
    phd_GenerateW2V(&viewer);

    ls_divider = 0x6000;
    phd_GetVectorAngles(-0x600, 0x100, 0x400, angles);
    phd_RotateLight(angles[1], angles[0]);

    phd_PushMatrix();
    phd_TranslateAbs(0, bz, 0);
    phd_RotYXZ(0, 0x4000, (sint16)0x8000);
    frame = anims[objects[MENU_BACKGROUND].anim_index].frame_ptr + 9;
    phd_RotYXZsuperpack(&frame, 0);
    phd_RotYXZ((sint16)0x8000, 0, 0);
    S_InsertInvBgnd(meshes[objects[MENU_BACKGROUND].mesh_index]);
    phd_PopMatrix();
}

#ifdef TR2_EXE
extern INVENTORY_ITEM InvPassportOption;
extern INVENTORY_ITEM InvControlOption;
extern INVENTORY_ITEM InvSoundOption;
extern INVENTORY_ITEM InvPhotoOption;
sint32 inv_nframes = 2;
sint16 InvOptionObjectsCount = 4;
INVENTORY_ITEM *InvOptionList[4] = {
    &InvPassportOption,
    &InvControlOption,
    &InvSoundOption,
    &InvPhotoOption,
};
int GymInvOpenEnabled = 1;
sint32 Inventory_Mode = 1;
sint32 NoInputCounter;
sint32 IsResetFlag;
sint32 StopInventory;
sint16 inv_main_current;
sint16 inv_keys_objects;
sint16 inv_keys_current;
sint16 InvOptionCurrent;
sint32 oldInputDB;
sint16 IsInventoryActive;
#endif
