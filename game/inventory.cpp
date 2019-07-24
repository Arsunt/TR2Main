/*
 * Copyright (c) 2017-2019 Michael Chaban. All rights reserved.
 * Original game is written by Core Design Ltd. in 1997.
 * Lara Croft and Tomb Raider are trademarks of Square Enix Ltd.
 *
 * This file is part of TR2Main.
 *
 * TR2Main is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TR2Main is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TR2Main.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "global/precompiled.h"
#include "game/inventory.h"
#include "3dsystem/3d_gen.h"
#include "3dsystem/scalespr.h"
#include "game/laramisc.h"
#include "game/text.h"
#include "game/draw.h"
#include "game/sound.h"
#include "game/invfunc.h"
#include "game/demo.h"
#include "game/health.h"
#include "specific/output.h"
#include "specific/display.h"
#include "specific/sndpc.h"
#include "specific/input.h"
#include "specific/frontend.h"
#include "specific/option.h"
#include "global/vars.h"

typedef enum PassportPage_t
{
	PSPINE = 1,
	PFRONT = 2,
	PINFRONT = 4,
	PPAGE2 = 8,
	PBACK = 16,
	PINBACK = 32,
	PPAGE1 = 64,
} PAGE;
#define PASS_MESH		(PFRONT|PSPINE|PBACK)

#define EXIT_NORMAL     (0)
#define CAN_CAUSE_CRASH (1)
#define EXIT_TO_TITLE   (0x500)
#define START_DEMO      (0x400)
#define RING_ROTATING_OPEN -0x8000
#define RING_ROTATING_CLOSE -0x8000
#define	EXIT_INVENTORY	(IN_DESELECT|IN_OPTION)

int __cdecl Display_Inventory(INVENTORY_MODE invMode) {
    int itemCount, DemoNeeded, itemAnimateFrame, PassOpen;
    int i, j, k, f, frm;
    __int16 itemCheckAngle, itemRotation;
    INVENTORY_ITEM *item;
    RING_INFO ring;
    PHD_3DPOS viewPos;
    INV_MOTION_INFO motion;

    memset(&ring, 0, sizeof(RING_INFO));
    memset(&motion, 0, sizeof(INV_MOTION_INFO));
    DemoNeeded = 0;
    PassOpen = 0;
    InventoryMode = invMode;

    if (InventoryMode == INV_KeysMode && !InvKeyObjectsCount)
    {
        InventoryChosen = -1;
        return EXIT_NORMAL;
    }

    T_RemovePrint(AmmoTextInfo);
    AmmoTextInfo = NULL;
    AlterFOV(14560);
    InvNFrames = 2;
    Construct_Inventory();

    if (InventoryMode == INV_TitleMode)
        S_FadeInInventory(0);
    else
        S_FadeInInventory(1);

    SOUND_Stop();

    if (InventoryMode != INV_TitleMode)
        S_CDVolume(0);

    switch (InventoryMode)
    {
        case INV_TitleMode:
        case INV_SaveMode:
        case INV_LoadMode:
        case INV_DeathMode:
            Inv_RingInit(&ring, RING_Option, InvOptionList, InvOptionObjectsCount, InvOptionCurrent, &motion);
            break;

        case INV_KeysMode:
            Inv_RingInit(&ring, RING_Keys, InvKeysList, InvKeyObjectsCount, InvMainCurrent, &motion);
            break;

        default:
            if (InvMainObjectsCount)
                Inv_RingInit(&ring, RING_Main, InvMainList, InvMainObjectsCount, InvMainCurrent, &motion);
            else
                Inv_RingInit(&ring, RING_Option, InvOptionList, InvOptionObjectsCount, InvOptionCurrent, &motion);
            break;
    }

    PlaySoundEffect(111, 0, SFX_ALWAYS);
    InvNFrames = 2;

    do
    {
        if (InventoryMode == INV_TitleMode && CD_TrackID > 0)
            S_CDLoop();

        Inv_RingCalcAdders(&ring, 24);
        S_UpdateInput();

        if (InvDemoMode)
        {
            if (InputStatus)
                return GF_GameFlow.onDemo_Interrupt;

            GetDemoInput();

            if (InputStatus == (DWORD)-1)
                return GF_GameFlow.onDemo_End;
        }
        else if (InputStatus)
        {
            NoInputCounter = 0;
        }

        InputDB = GetDebouncedInput(InputStatus);

        if (InventoryMode != INV_TitleMode || InputStatus || InputDB)
        {
            NoInputCounter = 0;
            IsResetFlag = 0;
        }
        else if (GF_GameFlow.num_Demos || CHK_ANY(GF_GameFlow.flags, GFF_NoInputTimeout))
        {
            NoInputCounter++;
            if (NoInputCounter > GF_GameFlow.noInput_Time)
            {
                DemoNeeded = 1;
                IsResetFlag = 1;
            }
        }

        if (StopInventory)
            return EXIT_TO_TITLE;

        if ((InventoryMode == INV_SaveMode ||
             InventoryMode == INV_LoadMode ||
             InventoryMode == INV_DeathMode) && !PassOpen)
        {
            InputStatus = 0;
            InputDB = IN_SELECT;
        }

        for (i = 0; i < InvNFrames; ++i)
        {
            if (IsInvOptionsDelay)
            {
                if (InvOptionsDelayCounter)
                    --InvOptionsDelayCounter;
                else
                    IsInvOptionsDelay = 0;
            }

            Inv_RingDoMotions(&ring);
        }

        ring.camera.z = (ring.radius + 598);
        S_InitialisePolyList(0);

        if (InventoryMode == INV_TitleMode)
            DoInventoryPicture();
        else
            DoInventoryBackground();

        S_AnimateTextures(InvNFrames);
        Inv_RingGetView(&ring, &viewPos);
        phd_GenerateW2V(&viewPos);
        Inv_RingLight(&ring);

        phd_PushMatrix();
        phd_TranslateAbs(ring.ringPos.x, ring.ringPos.y, ring.ringPos.z);
        phd_RotYXZ(ring.ringPos.rotY, ring.ringPos.rotX, ring.ringPos.rotZ);

        itemRotation = 0;
        itemCount = 0;

        if (ring.objCount > 0)
        {
            for (itemCount = 0; itemCount < ring.objCount; ++itemCount)
            {
                item = ring.itemList[itemCount];

                if (itemCount == ring.currentObj)
                {
                    for (j = 0; j < InvNFrames; ++j)
                    {
                        if (ring.isRotating)
                        {
                            LsAdder = 5120;

                            if (item->zRot)
                            {
                                if (item->zRot >= 0)
                                    item->zRot -= 512;
                                else
                                    item->zRot += 512;
                            }
                            continue;
                        }
                        else
                        {
                            if (motion.status != RNG_SELECTED
                            &&  motion.status != RNG_DESELECTING
                            &&  motion.status != RNG_SELECTING
                            &&  motion.status != RNG_DESELECT
                            &&  motion.status != RNG_CLOSING_ITEM)
                            {
                                if (ring.objCount != 1 && CHK_ANY(InputStatus, 0xC))
                                    continue;

                                LsAdder = 4096;
                                item->zRot += 256;
                                continue;
                            }

                            LsAdder = 4096;
                            if (item->zRot != item->yRot)
                            {
                                itemCheckAngle = (item->yRot - item->zRot);
                                if (itemCheckAngle <= 0 || itemCheckAngle >= 0x8000)
                                    item->zRot -= 1024;
                                else
                                    item->zRot += 1024;

                                item->zRot &= 0xFC00u;
                                continue;
                            }
                        }
                    }

                    if((motion.status != RNG_OPEN
                    &&  motion.status != RNG_SELECTING
                    &&  motion.status != RNG_SELECTED
                    &&  motion.status != RNG_DESELECTING
                    &&  motion.status != RNG_DESELECT
                    &&  motion.status != RNG_CLOSING_ITEM)
                    ||  ring.isRotating
                    ||  CHK_ANY(InputStatus, 0xC))
                    {
                        goto RING_STATUS;
                    }

                    RingNotActive(item);
                }
                else
                {
                    LsAdder = 5120;

                    for (f = 0; f < InvNFrames; ++f)
                    {
                        if (item->zRot != 0)
                        {
                            if (item->zRot >= 0)
                                item->zRot -= 256;
                            else
                                item->zRot += 256;
                        }
                    }
                }
RING_STATUS:
                if (motion.status == RNG_OPEN
                ||  motion.status == RNG_SELECTING
                ||  motion.status == RNG_SELECTED
                ||  motion.status == RNG_DESELECTING
                ||  motion.status == RNG_DESELECT
                ||  motion.status == RNG_CLOSING_ITEM)
                {
                    RingIsOpen(&ring);
                }
                else
                {
                    RingIsNotOpen();
                }

                if (!motion.status
                ||  motion.status == RNG_CLOSING
                ||  motion.status == RNG_MAIN2OPTION
                ||  motion.status == RNG_OPTION2MAIN
                ||  motion.status == RNG_EXITING_INVENTORY
                ||  motion.status == RNG_DONE
                ||  ring.isRotating)
                {
                    RingActive();
                }

                phd_PushMatrix();
                phd_RotYXZ(itemRotation, 0, 0);
                phd_TranslateRel(ring.radius, 0, 0);
                phd_RotYXZ(0x4000, item->xRotPt, 0);
                DrawInventoryItem(item);
                phd_PopMatrix();
                itemRotation += ring.angleAdder;
            }
        }
        phd_PopMatrix();

        DrawModeInfo();
        T_DrawText();
        S_OutputPolyList();
        SOUND_EndScene();

        Camera.numberFrames = InvNFrames = S_DumpScreen();

        if (CurrentLevel)
            SaveGame.statistics.timer += InvNFrames / 2;

        if (!ring.isRotating)
        {
            switch (motion.status)
            {
                case RNG_OPEN:
                    if (CHK_ANY(InputStatus, IN_RIGHT) && ring.objCount > 1)
                    {
                        Inv_RingRotateLeft(&ring);
                        PlaySoundEffect(108, 0, SFX_ALWAYS);
                        continue;
                    }

                    if (CHK_ANY(InputStatus, IN_LEFT) && ring.objCount > 1)
                    {
                        Inv_RingRotateRight(&ring);
                        PlaySoundEffect(108, 0, SFX_ALWAYS);
                        continue;
                    }

                    if ((IsResetFlag || InventoryMode != INV_TitleMode) && CHK_ANY(InputDB, EXIT_INVENTORY))
                    {
                        PlaySoundEffect(112, 0, SFX_ALWAYS);
                        InventoryChosen = -1;

                        if (ring.type)
                            InvOptionCurrent = ring.currentObj;
                        else
                            InvMainCurrent = ring.currentObj;

                        if (InventoryMode == INV_TitleMode)
                            S_FadeOutInventory(FALSE);
                        else
                            S_FadeOutInventory(TRUE);

                        Inv_RingMotionSetup(&ring, RNG_CLOSING, RNG_DONE, 32);
                        Inv_RingMotionRadius(&ring, 0);
                        Inv_RingMotionCameraPos(&ring, -1536);
                        Inv_RingMotionRotation(&ring, RING_ROTATING_CLOSE, ring.ringPos.rotY - RING_ROTATING_CLOSE);
                        InputStatus = 0;
                        InputDB = 0;
                    }

                    if (CHK_ANY(InputDB, IN_SELECT))
                    {
                        if ((InventoryMode == INV_SaveMode || InventoryMode == INV_LoadMode || InventoryMode == INV_DeathMode) && !PassOpen)
                            PassOpen = 1;

                        SoundOptionLine = 0;

                        switch (ring.type)
                        {
                            case RING_Main:
                                InvMainCurrent = ring.currentObj;
                                item = InvMainList[ring.currentObj];
                                break;
                            case RING_Option:
                                InvOptionCurrent = ring.currentObj;
                                item = InvOptionList[ring.currentObj];
                                break;
                            case RING_Keys:
                                InvKeysCurrent = ring.currentObj;
                                item = InvKeysList[ring.currentObj];
                                break;
                            // to prevent crash:
                            default:
                                InvOptionCurrent = ring.currentObj;
                                item = InvOptionList[ring.currentObj];
                                break;
                        }

                        item->goalFrame = item->openFrame;
                        item->animDirection = 1;
                        Inv_RingMotionSetup(&ring, RNG_SELECTING, RNG_SELECTED, 16);
                        Inv_RingMotionRotation(&ring, 0, -16384 - ring.angleAdder * ring.currentObj);
                        Inv_RingMotionItemSelect(&ring, item);
                        InputStatus = 0;
                        InputDB = 0;

                        switch (item->objectID)
                        {
                            case ID_COMPASS_OPTION:
                                PlaySoundEffect(113, 0, SFX_ALWAYS);
                                break;

                            case ID_PHOTO_OPTION:
                                PlaySoundEffect(109, 0, SFX_ALWAYS);
                                break;

                            case ID_PISTOL_OPTION:
                            case ID_SHOTGUN_OPTION:
                            case ID_MAGNUM_OPTION:
                            case ID_UZI_OPTION:
                            case ID_HARPOON_OPTION:
                            case ID_M16_OPTION:
                            case ID_GRENADE_OPTION:
                                PlaySoundEffect(114, 0, SFX_ALWAYS);
                                break;

                            default:
                                PlaySoundEffect(111, 0, SFX_ALWAYS);
                                break;
                        }
                    }

                    if (CHK_ANY(InputDB, IN_FORWARD) && InventoryMode != INV_TitleMode && InventoryMode != INV_KeysMode)
                    {
                        if (ring.type == RING_Main)
                        {
                            if (InvKeyObjectsCount)
                            {
                                Inv_RingMotionSetup(&ring, RNG_CLOSING, RNG_MAIN2KEYS, 24);
                                Inv_RingMotionRadius(&ring, 0);
                                Inv_RingMotionRotation(&ring, RING_ROTATING_CLOSE, ring.ringPos.rotY + RING_ROTATING_CLOSE);
                                Inv_RingMotionCameraPitch(&ring, 0x2000);
                                motion.misc = 0x2000;
                            }
                            InputStatus = 0;
                            InputDB = 0;
                        }
                        else if (ring.type == RING_Option)
                        {
                            if (InvMainObjectsCount)
                            {
                                Inv_RingMotionSetup(&ring, RNG_CLOSING, RNG_OPTION2MAIN, 24);
                                Inv_RingMotionRadius(&ring, 0);
                                Inv_RingMotionRotation(&ring, RING_ROTATING_CLOSE, ring.ringPos.rotY + RING_ROTATING_CLOSE);
                                Inv_RingMotionCameraPitch(&ring, 0x2000);
                                motion.misc = 0x2000;
                            }
                            InputDB = 0;
                        }
                    }
                    else if (InputDB & IN_BACK && InventoryMode != INV_TitleMode && InventoryMode != INV_KeysMode)
                    {
                        if (ring.type == RING_Keys)
                        {
                            if (InvMainObjectsCount)
                            {
                                Inv_RingMotionSetup(&ring, RNG_CLOSING, RNG_KEYS2MAIN, 24);
                                Inv_RingMotionRadius(&ring, 0);
                                Inv_RingMotionRotation(&ring, RING_ROTATING_CLOSE, ring.ringPos.rotY + RING_ROTATING_CLOSE);
                                Inv_RingMotionCameraPitch(&ring, -0x2000);
                                motion.misc = -0x2000;
                            }
                            InputStatus = 0;
                            InputDB = 0;
                        }
                        else if (ring.type == RING_Main)
                        {
                            if (InvOptionObjectsCount || !(GF_GameFlow.flags & GFF_LockoutOptionRing))
                            {
                                Inv_RingMotionSetup(&ring, RNG_CLOSING, RNG_MAIN2OPTION, 24);
                                Inv_RingMotionRadius(&ring, 0);
                                Inv_RingMotionRotation(&ring, RING_ROTATING_CLOSE, ring.ringPos.rotY + RING_ROTATING_CLOSE);
                                Inv_RingMotionCameraPitch(&ring, -0x2000);
                                motion.misc = -0x2000;
                            }
                            InputDB = 0;
                        }
                    }
                    continue;
                case RNG_MAIN2OPTION:
                    Inv_RingMotionSetup(&ring, RNG_OPENING, RNG_OPEN, 24);
                    Inv_RingMotionRadius(&ring, 688);
                    ring.cameraPitch = -motion.misc;

                    motion.cameraTarget_pitch = 0;
                    motion.cameraRate_pitch = motion.misc / 24;

                    ring.itemList = InvOptionList;
                    ring.type = RING_Option;
                    InvMainCurrent = ring.currentObj;
                    ring.objCount = InvOptionObjectsCount;
                    ring.currentObj = InvOptionCurrent;

                    Inv_RingCalcAdders(&ring, 24);
                    Inv_RingMotionRotation(&ring, RING_ROTATING_OPEN, -16384 - (ring.angleAdder * ring.currentObj));
                    ring.ringPos.rotY = motion.rotateTarget + RING_ROTATING_OPEN;
                    continue;
                case RNG_MAIN2KEYS:
                    Inv_RingMotionSetup(&ring, RNG_OPENING, RNG_OPEN, 24);
                    Inv_RingMotionRadius(&ring, 688);

                    ring.cameraPitch = -motion.misc;
                    motion.cameraRate_pitch = motion.misc / 24;
                    motion.cameraTarget_pitch = 0;

                    InvMainCurrent = ring.currentObj;
                    InvMainObjectsCount = ring.objCount;
                    ring.itemList = InvKeysList;
                    ring.type = RING_Keys;
                    ring.objCount = InvKeyObjectsCount;
                    ring.currentObj = InvKeysCurrent;

                    Inv_RingCalcAdders(&ring, 24);
                    Inv_RingMotionRotation(&ring, RING_ROTATING_CLOSE, -16384 - (ring.angleAdder * ring.currentObj));
                    ring.ringPos.rotY = motion.rotateTarget + RING_ROTATING_CLOSE;
                    continue;
                case RNG_KEYS2MAIN:
                    Inv_RingMotionSetup(&ring, RNG_OPENING, RNG_OPEN, 24);
                    Inv_RingMotionRadius(&ring, 688);

                    ring.cameraPitch = -LOWORD(motion.misc);
                    motion.cameraRate_pitch = motion.misc / 24;
                    motion.cameraTarget_pitch = 0;

                    ring.itemList = InvMainList;
                    ring.type = RING_Main;
                    InvOptionObjectsCount = ring.objCount;
                    InvOptionCurrent = ring.currentObj;
                    ring.objCount = InvMainObjectsCount;
                    ring.currentObj = InvMainCurrent;

                    Inv_RingCalcAdders(&ring, 24);
                    Inv_RingMotionRotation(&ring, RING_ROTATING_CLOSE, -16384 - (ring.angleAdder * ring.currentObj));
                    ring.ringPos.rotY = motion.rotateTarget + RING_ROTATING_CLOSE;
                    continue;
                case RNG_OPTION2MAIN:
                    Inv_RingMotionSetup(&ring, RNG_OPENING, RNG_OPEN, 24);
                    Inv_RingMotionRadius(&ring, 688);

                    ring.cameraPitch = -LOWORD(motion.misc);
                    motion.cameraRate_pitch = motion.misc / 24;
                    motion.cameraTarget_pitch = 0;

                    ring.itemList = InvMainList;
                    ring.type = RING_Main;
                    InvOptionObjectsCount = ring.objCount;
                    InvOptionCurrent = ring.currentObj;
                    ring.objCount = InvMainObjectsCount;
                    ring.currentObj = InvMainCurrent;

                    Inv_RingCalcAdders(&ring, 24);
                    Inv_RingMotionRotation(&ring, RING_ROTATING_OPEN, -16384 - (ring.angleAdder * ring.currentObj));
                    ring.ringPos.rotY = motion.rotateTarget + RING_ROTATING_OPEN;
                    continue;
                case RNG_SELECTED:
                    item = ring.itemList[ring.currentObj];

                    if (item->objectID == ID_PASSPORT_CLOSED)
                        item->objectID = ID_PASSPORT_OPTION;

                    for (k = 0; k < InvNFrames; ++k)
                    {
                        itemAnimateFrame = 0;
                        if (item->zRot == item->yRot)
                            itemAnimateFrame = AnimateInventoryItem(item);
                    }

                    if (!itemAnimateFrame && !IsInvOptionsDelay)
                    {
                        do_inventory_options(item);

                        if (InputDB & IN_DESELECT)
                        {
                            item->sprites = NULL;
                            Inv_RingMotionSetup(&ring, RNG_CLOSING_ITEM, RNG_DESELECT, 0);
                            InputStatus = 0;
                            InputDB = 0;

                            if (InventoryMode == INV_SaveMode || InventoryMode == INV_LoadMode)
                            {
                                Inv_RingMotionSetup(&ring, RNG_CLOSING_ITEM, RNG_EXITING_INVENTORY, 0);
                                InputDB = 0;
                                InputStatus = 0;
                            }
                        }

                        if (InputDB & IN_SELECT)
                        {
                            item->sprites = NULL;
                            InventoryChosen = item->objectID;

                            if (ring.type)
                                InvOptionCurrent = ring.currentObj;
                            else
                                InvMainCurrent = ring.currentObj;

                            if (InventoryMode != INV_TitleMode
                            || (item->objectID != ID_DETAIL_OPTION
                            &&  item->objectID != ID_SOUND_OPTION
                            &&  item->objectID != ID_CONTROL_OPTION
                            &&  item->objectID != ID_GAMMA_OPTION))
                                Inv_RingMotionSetup(&ring, RNG_CLOSING_ITEM, RNG_EXITING_INVENTORY, 0);
                            else
                                Inv_RingMotionSetup(&ring, RNG_CLOSING_ITEM, RNG_DESELECT, 0);

                            InputStatus = 0;
                            InputDB = 0;
                        }
                    }
                    continue;
                case RNG_DESELECT:
                    PlaySoundEffect(112, 0, SFX_ALWAYS);
                    Inv_RingMotionSetup(&ring, RNG_DESELECTING, RNG_OPEN, 16);
                    Inv_RingMotionRotation(&ring, 0, -16384 - (ring.angleAdder * ring.currentObj));
                    InputStatus = 0;
                    InputDB = 0;
                    continue;
                case RNG_CLOSING_ITEM:
                    item = ring.itemList[ring.currentObj];

                    for (frm = 0; frm < InvNFrames; ++frm)
                    {
                        if (!AnimateInventoryItem(item))
                        {
                            // change passport to single mesh !
                            if (item->objectID == ID_PASSPORT_OPTION)
                            {
                                item->objectID = ID_PASSPORT_CLOSED;
                                item->currentFrame = 0;
                            }

                            motion.framesCount = 16;
                            motion.status = motion.statusTarget;
                            Inv_RingMotionItemDeselect(&ring, item);
                        }
                    }
                    break;
                case RNG_EXITING_INVENTORY:
                    if (!motion.framesCount)
                    {
                        if (InventoryMode == INV_TitleMode)
                            S_FadeOutInventory(FALSE);
                        else
                            S_FadeOutInventory(TRUE);

                        Inv_RingMotionSetup(&ring, RNG_CLOSING, RNG_DONE, 32);
                        Inv_RingMotionRadius(&ring, 0);
                        Inv_RingMotionCameraPos(&ring, -1536);
                        Inv_RingMotionRotation(&ring, RING_ROTATING_CLOSE, ring.ringPos.rotY + RING_ROTATING_CLOSE);
                    }
                    continue;

                default:
                    continue;
            }
        }
    }
    while (motion.status != RNG_DONE);

    RemoveInventoryText();
    S_FinishInventory();
    IsInventoryActive = 0;

    if (IsResetFlag)
        return EXIT_TO_TITLE;

    if (DemoNeeded)
        return START_DEMO;

    if (InventoryChosen == -1)
    {
        if (InventoryMode == INV_TitleMode)
            return EXIT_NORMAL;
        S_CDVolume(25 * MusicVolume + 5);
        return EXIT_NORMAL;
    }
    else
    {
        switch (InventoryChosen)
        {
            case ID_PASSPORT_OPTION:
                if (InventoryExtraData[0] != 1 || !MusicVolume)
                    return CAN_CAUSE_CRASH;
                S_CDVolume(25 * MusicVolume + 5);
                return CAN_CAUSE_CRASH;

            case ID_PHOTO_OPTION:
                // GFF_HOME_ENABLED
                if (!(GF_GameFlow.flags & GFF_Unknown))
                    break;

                InventoryExtraData[1] = 0;
                return CAN_CAUSE_CRASH;

            case ID_PISTOL_OPTION:
                UseItem(ID_PISTOL_OPTION);
                break;
            case ID_SHOTGUN_OPTION:
                UseItem(ID_SHOTGUN_OPTION);
                break;
            case ID_MAGNUM_OPTION:
                UseItem(ID_MAGNUM_OPTION);
                break;
            case ID_UZI_OPTION:
                UseItem(ID_UZI_OPTION);
                break;
            case ID_HARPOON_OPTION:
                UseItem(ID_HARPOON_OPTION);
                break;
            case ID_M16_OPTION:
                UseItem(ID_M16_OPTION);
                break;
            case ID_GRENADE_OPTION:
                UseItem(ID_GRENADE_OPTION);
                break;
            case ID_SMALL_MEDIPACK_OPTION:
                UseItem(ID_SMALL_MEDIPACK_OPTION);
                break;
            case ID_LARGE_MEDIPACK_OPTION:
                UseItem(ID_LARGE_MEDIPACK_OPTION);
                break;
            case ID_FLARES_OPTION:
                UseItem(ID_FLARES_OPTION);
                break;

            default:
                break;
        }
    }

    if (MusicVolume && InventoryMode != INV_TitleMode)
        S_CDVolume(MusicVolume * 25 + 5);

    return EXIT_NORMAL;
}

void __cdecl Construct_Inventory() {
    INVENTORY_ITEM *invItem;
    int i;

    S_SetupAboveWater(FALSE);

    if (InventoryMode != INV_TitleMode)
        TempVideoAdjust(HiRes, 1.0);

    memset(InventoryExtraData, 0, sizeof(InventoryExtraData));

    PhdWinRight = PhdWinMaxX;
    PhdWinLeft = 0;
    PhdWinTop = 0;
    PhdWinBottom = PhdWinMaxY;

    IsInventoryActive = 1;
    InventoryChosen = 0;

    if (InventoryMode == INV_TitleMode)
        InvOptionObjectsCount = 4;
    else
        InvOptionObjectsCount = 3;

    for (i = 0; i < InvMainObjectsCount; ++i)
    {
        invItem = InvMainList[i];
        invItem->currentFrame = 0;
        invItem->meshesDrawn = invItem->meshesSel;
        invItem->goalFrame = 0;
        invItem->animCount = 0;
        invItem->zRot = 0;
    }

    for (i = 0; i < InvOptionObjectsCount; ++i)
    {
        invItem = InvOptionList[i];
        invItem->currentFrame = 0;
        invItem->goalFrame = 0;
        invItem->animCount = 0;
        invItem->zRot = 0;
    }

    InvMainCurrent = 0;
    if  (GymInvOpenEnabled && InventoryMode == INV_TitleMode && !(GF_GameFlow.flags & GFF_LoadSaveDisabled) && GF_GameFlow.flags & GFF_Unknown)
    {
        InvOptionCurrent = 3;
    }
    else
    {
        InvOptionCurrent = 0;
        GymInvOpenEnabled = FALSE;
    }

    SoundOptionLine = 0;
}

void __cdecl SelectMeshes(INVENTORY_ITEM *invItem) {
    int frame;

    if (invItem->objectID == ID_PASSPORT_OPTION)
    {
        frame = invItem->currentFrame;
        if (frame < 4)
            invItem->meshesDrawn = PASS_MESH | PINFRONT;
        else if (frame <= 16)
            invItem->meshesDrawn = PASS_MESH | PINFRONT | PPAGE1;
        else if (frame < 19)
            invItem->meshesDrawn = PASS_MESH | PINFRONT | PPAGE1 | PPAGE2;
        else if (frame == 19)
            invItem->meshesDrawn = PASS_MESH | PPAGE1 | PPAGE2;
        else if (frame < 24)
            invItem->meshesDrawn = PASS_MESH | PPAGE1 | PPAGE2 | PINBACK;
        else if (frame < 29)
            invItem->meshesDrawn = PASS_MESH | PPAGE2 | PINBACK;
        else if (frame == 29)
            invItem->meshesDrawn = PASS_MESH;
    }
    else if (invItem->objectID != ID_GAMMA_OPTION)
        invItem->meshesDrawn = 0xFFFFFFFF;
}

int __cdecl AnimateInventoryItem(INVENTORY_ITEM *invItem) {
    int frame = invItem->currentFrame;
    int animID = invItem->animCount;
    __int16 animDirection;

    if (frame == invItem->goalFrame)
    {
        SelectMeshes(invItem);
        return 0;
    }
    else
    {
        if (animID)
        {
             invItem->animCount = (animID - 1);
             SelectMeshes(invItem);
             return 1;
        }
        else
        {
            invItem->animCount = invItem->animSpeed;
            animDirection = (frame + invItem->animDirection);
            invItem->currentFrame = animDirection;

            if (animDirection < invItem->framesTotal)
            {
                if (animDirection < 0)
                    invItem->currentFrame = (invItem->framesTotal - 1);
                SelectMeshes(invItem);
                return 1;
            }
            else
            {
                invItem->currentFrame = 0;
                SelectMeshes(invItem);
                return 1;
            }
        }
    }

    return 1;
}

void __cdecl DrawInventoryItem(INVENTORY_ITEM* invItem) {
    int	hours, minutes, seconds, totsec;
    int sx, sy, j, l;
    INVENTORY_SPRITE *sprite;
    int clip, *bones, pushpop;
    short* frame[2];
    __int16 mesh, mesh_num, i;
    UINT16* rotation;
    OBJECT_INFO* obj;

    if (invItem->objectID == ID_COMPASS_OPTION)
    {
        totsec = SaveGame.statistics.timer/30;
		seconds = totsec%60;
		minutes = ((totsec%3600)*-91)/5;
		hours = ((totsec/12)*-91)/5;
		seconds *=-1092;
    }
    else
    {
        hours=minutes=seconds=totsec=0;
    }

    phd_TranslateRel(0, invItem->yTrans, invItem->zTrans);
    phd_RotYXZ(invItem->zRot, invItem->yRotSel, 0);
    obj = &Objects[invItem->objectID];

    if (obj->loaded)
    {
        if (obj->nMeshes >= 0)
        {
            if (invItem->sprites)
            {
                clip = PhdMatrixPtr->_23;
                sx = PhdWinCenterX + PhdMatrixPtr->_03 / (clip / PhdPersp);
                sy = PhdWinCenterY + PhdMatrixPtr->_13 / (clip / PhdPersp);
                sprite = (INVENTORY_SPRITE*)invItem->sprites;

                for (l = sprite->y; !(sprite->shape == 0); l+=4)
                {
                    if (clip < PhdNearZ || clip > PhdFarZ)
                        break;

                    for (j = sprite->shape; j != 0; j++)
                    {
                        switch (j)
                        {
                            case 1:
                                S_DrawScreenSprite(sx+sprite->x, sy+sprite->y,sprite->z,sprite->param1,sprite->param2,StaticObjects[ID_ALPHABET].meshIndex+sprite->invColour,4096,0);
                                break;
                            case 2:
                                S_DrawScreenLine(sx+sprite->x, sy+sprite->y,sprite->z,sprite->param1,sprite->param2,sprite->invColour,(D3DCOLOR*)sprite->gour,0);
                                break;
                            case 3:
                                S_DrawScreenBox(sx+sprite->x, sy+sprite->y,sprite->z,sprite->param1,sprite->param2,sprite->invColour,(GOURAUD_OUTLINE*)sprite->gour,0);
                                break;
                            case 4:
                                S_DrawScreenFBox(sx+sprite->x, sy+sprite->y,sprite->z,sprite->param1,sprite->param2,sprite->invColour,(GOURAUD_FILL*)sprite->gour,0);
                                break;
                            default:
                                break;
                        }
                    }
                }
            }

            frame[0] = (&obj->frameBase[invItem->currentFrame * (Anims[obj->animIndex].interpolation >> 8)]);
            phd_PushMatrix();

            if ((clip = S_GetObjectBounds(frame[0])))
            {
                phd_TranslateRel((int)*(frame[0] + 6), (int)*(frame[0] + 7), (int)*(frame[0] + 8));
                rotation = (UINT16*)(frame[0] + 9);
                phd_RotYXZsuperpack(&rotation, 0);
                mesh = obj->meshIndex;
                bones = &AnimBones[obj->boneIndex];
                mesh_num = 1;

                if (mesh_num & invItem->meshesDrawn)
                    phd_PutPolygons(MeshPtr[mesh]);

                for (i = obj->nMeshes - 1; i > 0; i--)
                {
                    ++mesh;
                    mesh_num <<= 1;

                    pushpop = *(bones++);
                    if (pushpop & 1)
                        phd_PopMatrix();
                    if (pushpop & 2)
                        phd_PushMatrix();

                    phd_TranslateRel(*(bones), *(bones+1), *(bones+2));
                    phd_RotYXZsuperpack(&rotation, 0);

                    if (invItem->objectID == ID_COMPASS_OPTION)
                    {
                        if (i == 2)
                        {
                            phd_RotZ(seconds);
                            invItem->reserved2 = invItem->reserved1;
                            invItem->reserved1 = seconds;
                        }

                        if (i == 3)
                            phd_RotZ(minutes);

                        if (i == 4)
                            phd_RotZ(hours);
                    }

                    if (mesh_num & invItem->meshesDrawn)
                        phd_PutPolygons(MeshPtr[mesh]);

                    bones += 3;
                }
            }
            phd_PopMatrix();
        }
        else
        {
            S_DrawSprite(0, 0, 0, 0, obj->meshIndex, 0, 0);
        }
    }
}

DWORD __cdecl GetDebouncedInput(DWORD input) {
	static DWORD oldInput = 0;
	DWORD result = input & ~oldInput;

	oldInput = input;
	return result;
}

void __cdecl DoInventoryPicture() {
    S_CopyBufferToScreen();
}

void __cdecl DoInventoryBackground() {
	VECTOR_ANGLES angles;
	PHD_3DPOS viewPos;
	UINT16 *ptr;

	S_CopyBufferToScreen();

	if( Objects[ID_INV_BACKGROUND].loaded ) {
		// set view
		phd_GetVectorAngles(0, 0x1000, 0, &angles);
		viewPos.x = 0;
		viewPos.y = -0x200;
		viewPos.z = 0;
		viewPos.rotX = angles.pitch;
		viewPos.rotY = angles.yaw;
		viewPos.rotZ = 0;
		phd_GenerateW2V(&viewPos);

		// set lighting
		LsDivider = 0x6000;
		phd_GetVectorAngles(-0x600, 0x100, 0x400, &angles);
		phd_RotateLight(angles.pitch, angles.yaw);

		// transform and insert the mesh
		phd_PushMatrix();
		ptr = (UINT16 *)&Anims[Objects[ID_INV_BACKGROUND].animIndex].framePtr[9];
		phd_TranslateAbs(0, 0x3000, 0);
		phd_RotYXZ(0, PHD_90, PHD_180);
		phd_RotYXZsuperpack(&ptr, 0);
		phd_RotYXZ(PHD_180, 0, 0);
		S_InsertInvBgnd(MeshPtr[Objects[ID_INV_BACKGROUND].meshIndex]);
		phd_PopMatrix();
	}
}

/*
 * Inject function
 */
void Inject_Inventory() {
    INJECT(0x00422060, Display_Inventory);
    INJECT(0x004232F0, Construct_Inventory);
    INJECT(0x00423450, SelectMeshes);
    INJECT(0x004234E0, AnimateInventoryItem);
    INJECT(0x00423570, DrawInventoryItem);
    INJECT(0x004239A0, GetDebouncedInput);
    INJECT(0x004239C0, DoInventoryPicture);
    INJECT(0x004239D0, DoInventoryBackground);
}
