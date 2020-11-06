/*
 * Copyright (c) 2017-2020 Michael Chaban. All rights reserved.
 * Copyright (c) 2019 TokyoSU <vlevasseur5@gmail.com>
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
#include "game/demo.h"
#include "game/draw.h"
#include "game/health.h"
#include "game/invfunc.h"
#include "game/laramisc.h"
#include "game/sound.h"
#include "game/text.h"
#include "specific/display.h"
#include "specific/frontend.h"
#include "specific/input.h"
#include "specific/option.h"
#include "specific/output.h"
#include "specific/sndpc.h"
#include "global/vars.h"

#ifdef FEATURE_INPUT_IMPROVED
#include "modding/joy_output.h"
#endif // FEATURE_INPUT_IMPROVED

#ifdef FEATURE_HUD_IMPROVED
extern void RemoveJoystickHintText(bool isSelect, bool isDeselect);
extern void DisplayJoystickHintText(bool isSelect, bool isDeselect);
#endif // FEATURE_HUD_IMPROVED

typedef enum {
	RINGSTATE_OPENING,
	RINGSTATE_OPEN,
	RINGSTATE_CLOSING,
	RINGSTATE_MAIN2OPTION,
	RINGSTATE_MAIN2KEYS,
	RINGSTATE_KEYS2MAIN,
	RINGSTATE_OPTION2MAIN,
	RINGSTATE_SELECTING,
	RINGSTATE_SELECTED,
	RINGSTATE_DESELECTING,
	RINGSTATE_DESELECT,
	RINGSTATE_CLOSING_ITEM,
	RINGSTATE_EXITING_INVENTORY,
	RINGSTATE_DONE
} RING_STATES;

#define PASS_SPINE		(0x01)
#define PASS_FRONT		(0x02)
#define PASS_INFRONT	(0x04)
#define PASS_PAGE2		(0x08)
#define PASS_BACK		(0x10)
#define PASS_INBACK		(0x20)
#define PASS_PAGE1		(0x40)
#define PASS_BASE		(PASS_FRONT|PASS_SPINE|PASS_BACK)

int __cdecl Display_Inventory(INVENTORY_MODE invMode) {
	BOOL isDemoNeeded = FALSE;
	BOOL isPassOpen = FALSE;
	int itemAnimateFrame = 0;
	__int16 itemRotation = 0;
	INVENTORY_ITEM *item = NULL;
	RING_INFO ring;
	PHD_3DPOS viewPos;
	INV_MOTION_INFO motion;

	memset(&ring, 0, sizeof(RING_INFO));
	memset(&motion, 0, sizeof(INV_MOTION_INFO));

	if( invMode == INV_KeysMode && !InvKeyObjectsCount ) {
		InventoryChosen = -1;
		return 0;
	}

	T_RemovePrint(AmmoTextInfo);
	AmmoTextInfo = NULL;
	AlterFOV(80*PHD_DEGREE);
	InventoryMode = invMode;
	int nTicks = TICKS_PER_FRAME;
	Construct_Inventory();

	if( InventoryMode == INV_TitleMode ) {
		S_FadeInInventory(0);
	} else {
		S_FadeInInventory(1);
	}

	SOUND_Stop();

	if( InventoryMode != INV_TitleMode ) {
#ifdef FEATURE_AUDIO_IMPROVED
		extern double InventoryMusicMute;
		double volume = (1.0 - InventoryMusicMute) * (double)(MusicVolume * 25 + 5);
		if( volume >= 1.0 ) {
			S_CDVolume((DWORD)volume);
		} else {
			S_CDVolume(0);
		}
#else // FEATURE_AUDIO_IMPROVED
		S_CDVolume(0); // NOTE: Core supposed to pause CD Audio this way
#endif // FEATURE_AUDIO_IMPROVED
	}

	switch( InventoryMode ) {
		case INV_TitleMode :
		case INV_SaveMode :
		case INV_LoadMode :
		case INV_DeathMode :
			Inv_RingInit(&ring, RING_Option, InvOptionList, InvOptionObjectsCount, InvOptionCurrent, &motion);
			break;

		case INV_KeysMode :
			Inv_RingInit(&ring, RING_Keys, InvKeysList, InvKeyObjectsCount, InvMainCurrent, &motion);
			break;

		default :
			if (InvMainObjectsCount) {
				Inv_RingInit(&ring, RING_Main, InvMainList, InvMainObjectsCount, InvMainCurrent, &motion);
			} else {
				Inv_RingInit(&ring, RING_Option, InvOptionList, InvOptionObjectsCount, InvOptionCurrent, &motion);
			}
			break;
	}

	PlaySoundEffect(111, NULL, SFX_ALWAYS);
	nTicks = TICKS_PER_FRAME;

	do {
		if( InventoryMode == INV_TitleMode && CD_TrackID > 0 ) {
			S_CDLoop();
		}

		Inv_RingCalcAdders(&ring, 24);
		S_UpdateInput();

		if( InvDemoMode ) {
			if( InputStatus ) {
				return GF_GameFlow.onDemo_Interrupt;
			}
			GetDemoInput();

			if( InputStatus == (DWORD)-1 ) {
				return GF_GameFlow.onDemo_End;
			}
		} else if( InputStatus ) {
			NoInputCounter = 0;
		}

		InputDB = GetDebouncedInput(InputStatus);

		if( InventoryMode != INV_TitleMode || InputStatus || InputDB ) {
			NoInputCounter = 0;
			IsResetFlag = FALSE;
		} else if( GF_GameFlow.num_Demos || CHK_ANY(GF_GameFlow.flags, GFF_NoInputTimeout) ) {
			if( ++NoInputCounter > GF_GameFlow.noInput_Time ) {
				isDemoNeeded = TRUE;
				IsResetFlag = TRUE;
			}
		}

		if( StopInventory ) {
			return GF_EXIT_TO_TITLE;
		}

		if( (InventoryMode == INV_SaveMode ||
			 InventoryMode == INV_LoadMode ||
			 InventoryMode == INV_DeathMode) && !isPassOpen )
		{
			InputStatus = 0;
			InputDB = IN_SELECT;
		}

		for( int i = 0; i < nTicks; ++i ) {
			if( IsInvOptionsDelay ) {
				if( InvOptionsDelayCounter ) {
					--InvOptionsDelayCounter;
				} else {
					IsInvOptionsDelay = FALSE;
				}
			}
			Inv_RingDoMotions(&ring);
		}

		ring.camera.z = (ring.radius + 0x256);
		S_InitialisePolyList(0);

		if( InventoryMode == INV_TitleMode ) {
			DoInventoryPicture();
		} else {
			DoInventoryBackground();
		}

		S_AnimateTextures(nTicks);
		Inv_RingGetView(&ring, &viewPos);
		phd_GenerateW2V(&viewPos);
		Inv_RingLight(&ring);

		phd_PushMatrix();
		phd_TranslateAbs(ring.ringPos.x, ring.ringPos.y, ring.ringPos.z);
		phd_RotYXZ(ring.ringPos.rotY, ring.ringPos.rotX, ring.ringPos.rotZ);

		itemRotation = 0;

		for( int i = 0; i < ring.objCount; ++i ) {
			item = ring.itemList[i];

			if( i == ring.currentObj ) {
				for( int j = 0; j < nTicks; ++j ) {
					if( ring.isRotating ) {
						LsAdder = 0x1400;
						if( item->zRot > 0 ) {
							item->zRot -= 0x200;
						} else if( item->zRot < 0 ) {
							item->zRot += 0x200;
						}
					} else if( motion.status == RINGSTATE_SELECTED
						|| motion.status == RINGSTATE_DESELECTING
						|| motion.status == RINGSTATE_SELECTING
						|| motion.status == RINGSTATE_DESELECT
						|| motion.status == RINGSTATE_CLOSING_ITEM )
					{
						LsAdder = 0x1000;
						if( item->zRot != item->yRot ) {
							if( item->yRot <= item->zRot || item->yRot >= item->zRot + PHD_180 ) {
								item->zRot -= 0x400;
							} else {
								item->zRot += 0x400;
							}
							item->zRot &= 0xFC00;
						}
					} else if( ring.objCount == 1 || !CHK_ANY(InputStatus, IN_LEFT|IN_RIGHT) ) {
						LsAdder = 0x1000;
						item->zRot += 0x100;
					}
				}

				if( (motion.status == RINGSTATE_OPEN
					|| motion.status == RINGSTATE_SELECTING
					|| motion.status == RINGSTATE_SELECTED
					|| motion.status == RINGSTATE_DESELECTING
					|| motion.status == RINGSTATE_DESELECT
					|| motion.status == RINGSTATE_CLOSING_ITEM)
					&& !ring.isRotating
					&& !CHK_ANY(InputStatus, IN_LEFT|IN_RIGHT) )
				{
					RingNotActive(item);
				}
			} else {
				LsAdder = 0x1400;
				for( int i = 0; i < nTicks; ++i ) {
					if( item->zRot > 0 ) {
						item->zRot -= 0x100;
					} else if( item->zRot < 0 ) {
						item->zRot += 0x100;
					}
				}
			}

			if( motion.status == RINGSTATE_OPEN
				|| motion.status == RINGSTATE_SELECTING
				|| motion.status == RINGSTATE_SELECTED
				|| motion.status == RINGSTATE_DESELECTING
				|| motion.status == RINGSTATE_DESELECT
				|| motion.status == RINGSTATE_CLOSING_ITEM )
			{
				RingIsOpen(&ring);
			} else {
				RingIsNotOpen();
			}

			if( motion.status == RINGSTATE_OPENING
				|| motion.status == RINGSTATE_CLOSING
				|| motion.status == RINGSTATE_MAIN2OPTION
				|| motion.status == RINGSTATE_OPTION2MAIN
				|| motion.status == RINGSTATE_EXITING_INVENTORY
				|| motion.status == RINGSTATE_DONE
				|| ring.isRotating )
			{
				RingActive();
			}

			phd_PushMatrix();
			phd_RotYXZ(itemRotation, 0, 0);
			phd_TranslateRel(ring.radius, 0, 0);
			phd_RotYXZ(PHD_90, item->xRotPt, 0);
			DrawInventoryItem(item);
			phd_PopMatrix();
			itemRotation += ring.angleAdder;
		}
		phd_PopMatrix();

		DrawModeInfo();
		T_DrawText();
		S_OutputPolyList();
		SOUND_EndScene();

		Camera.numberFrames = nTicks = S_DumpScreen();

		if( CurrentLevel != 0 ) { // not Lara home
			SaveGame.statistics.timer += nTicks / TICKS_PER_FRAME;
		}

		if( !ring.isRotating ) {
			switch( motion.status ) {
				case RINGSTATE_OPEN :
#ifdef FEATURE_HUD_IMPROVED
					DisplayJoystickHintText(true, InventoryMode != INV_TitleMode && InventoryMode != INV_DeathMode);
#endif // FEATURE_HUD_IMPROVED

					if( CHK_ANY(InputStatus, IN_RIGHT) && ring.objCount > 1 ) {
						Inv_RingRotateLeft(&ring);
						PlaySoundEffect(108, 0, SFX_ALWAYS);
						break;
					}

					if( CHK_ANY(InputStatus, IN_LEFT) && ring.objCount > 1 ) {
						Inv_RingRotateRight(&ring);
						PlaySoundEffect(108, 0, SFX_ALWAYS);
						break;
					}

					if( IsResetFlag || (InventoryMode != INV_TitleMode && CHK_ANY(InputDB, IN_DESELECT|IN_OPTION)) ) {
						PlaySoundEffect(112, 0, SFX_ALWAYS);
						InventoryChosen = -1;

						if( ring.type != RING_Main ) {
							InvOptionCurrent = ring.currentObj;
						} else {
							InvMainCurrent = ring.currentObj;
						}

						if (InventoryMode == INV_TitleMode) {
							S_FadeOutInventory(FALSE);
						} else {
							S_FadeOutInventory(TRUE);
						}

						Inv_RingMotionSetup(&ring, RINGSTATE_CLOSING, RINGSTATE_DONE, 32);
						Inv_RingMotionRadius(&ring, 0);
						Inv_RingMotionCameraPos(&ring, -0x600);
						Inv_RingMotionRotation(&ring, -PHD_180, ring.ringPos.rotY + PHD_180);
						InputStatus = 0;
						InputDB = 0;
					}

					if( CHK_ANY(InputDB, IN_SELECT) ) {
						if( (InventoryMode == INV_SaveMode || InventoryMode == INV_LoadMode || InventoryMode == INV_DeathMode) && !isPassOpen ) {
							isPassOpen = TRUE;
						}

						SoundOptionLine = 0;

						switch (ring.type) {
							case RING_Main :
								InvMainCurrent = ring.currentObj;
								item = InvMainList[ring.currentObj];
								break;
							case RING_Option :
								InvOptionCurrent = ring.currentObj;
								item = InvOptionList[ring.currentObj];
								break;
							case RING_Keys :
							default :
								InvKeysCurrent = ring.currentObj;
								item = InvKeysList[ring.currentObj];
								break;
						}

						item->goalFrame = item->openFrame;
						item->animDirection = 1;
						Inv_RingMotionSetup(&ring, RINGSTATE_SELECTING, RINGSTATE_SELECTED, 16);
						Inv_RingMotionRotation(&ring, 0, -PHD_90 - ring.angleAdder * ring.currentObj);
						Inv_RingMotionItemSelect(&ring, item);
						InputStatus = 0;
						InputDB = 0;

						switch (item->objectID) {
							case ID_COMPASS_OPTION :
								PlaySoundEffect(113, 0, SFX_ALWAYS);
								break;
							case ID_PHOTO_OPTION :
								PlaySoundEffect(109, 0, SFX_ALWAYS);
								break;
							case ID_PISTOL_OPTION :
							case ID_SHOTGUN_OPTION :
							case ID_MAGNUM_OPTION :
							case ID_UZI_OPTION :
							case ID_HARPOON_OPTION :
							case ID_M16_OPTION :
							case ID_GRENADE_OPTION :
								PlaySoundEffect(114, 0, SFX_ALWAYS);
								break;
							default :
								PlaySoundEffect(111, 0, SFX_ALWAYS);
								break;
						}
					}

					if( CHK_ANY(InputDB, IN_FORWARD) && InventoryMode != INV_TitleMode && InventoryMode != INV_KeysMode ) {
						if( ring.type == RING_Main ) {
							if( InvKeyObjectsCount ) {
								Inv_RingMotionSetup(&ring, RINGSTATE_CLOSING, RINGSTATE_MAIN2KEYS, 24);
								Inv_RingMotionRadius(&ring, 0);
								Inv_RingMotionRotation(&ring, -PHD_180, ring.ringPos.rotY - PHD_180);
								Inv_RingMotionCameraPitch(&ring, 0x2000);
								motion.misc = 0x2000;
							}
							InputStatus = 0;
							InputDB = 0;
						} else if( ring.type == RING_Option ) {
							if( InvMainObjectsCount ) {
								Inv_RingMotionSetup(&ring, RINGSTATE_CLOSING, RINGSTATE_OPTION2MAIN, 24);
								Inv_RingMotionRadius(&ring, 0);
								Inv_RingMotionRotation(&ring, -PHD_180, ring.ringPos.rotY - PHD_180);
								Inv_RingMotionCameraPitch(&ring, 0x2000);
								motion.misc = 0x2000;
							}
							InputDB = 0;
						}
					} else if( InputDB & IN_BACK && InventoryMode != INV_TitleMode && InventoryMode != INV_KeysMode ) {
						if( ring.type == RING_Keys ) {
							if (InvMainObjectsCount) {
								Inv_RingMotionSetup(&ring, RINGSTATE_CLOSING, RINGSTATE_KEYS2MAIN, 24);
								Inv_RingMotionRadius(&ring, 0);
								Inv_RingMotionRotation(&ring, -PHD_180, ring.ringPos.rotY - PHD_180);
								Inv_RingMotionCameraPitch(&ring, -0x2000);
								motion.misc = -0x2000;
							}
							InputStatus = 0;
							InputDB = 0;
						} else if( ring.type == RING_Main ) {
							if( InvOptionObjectsCount || !(GF_GameFlow.flags & GFF_LockoutOptionRing) ) {
								Inv_RingMotionSetup(&ring, RINGSTATE_CLOSING, RINGSTATE_MAIN2OPTION, 24);
								Inv_RingMotionRadius(&ring, 0);
								Inv_RingMotionRotation(&ring, -PHD_180, ring.ringPos.rotY - PHD_180);
								Inv_RingMotionCameraPitch(&ring, -0x2000);
								motion.misc = -0x2000;
							}
							InputDB = 0;
						}
					}
					break;
				case RINGSTATE_MAIN2OPTION:
					Inv_RingMotionSetup(&ring, RINGSTATE_OPENING, RINGSTATE_OPEN, 24);
					Inv_RingMotionRadius(&ring, 0x2B0);
					ring.cameraPitch = -motion.misc;

					motion.cameraTarget_pitch = 0;
					motion.cameraRate_pitch = motion.misc / 24;

					ring.itemList = InvOptionList;
					ring.type = RING_Option;
					InvMainCurrent = ring.currentObj;
					ring.objCount = InvOptionObjectsCount;
					ring.currentObj = InvOptionCurrent;

					Inv_RingCalcAdders(&ring, 24);
					Inv_RingMotionRotation(&ring, -PHD_180, -PHD_90 - ring.angleAdder * ring.currentObj);
					ring.ringPos.rotY = motion.rotateTarget - PHD_180;
					break;
				case RINGSTATE_MAIN2KEYS :
					Inv_RingMotionSetup(&ring, RINGSTATE_OPENING, RINGSTATE_OPEN, 24);
					Inv_RingMotionRadius(&ring, 0x2B0);

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
					Inv_RingMotionRotation(&ring, -PHD_180, -PHD_90 - ring.angleAdder * ring.currentObj);
					ring.ringPos.rotY = motion.rotateTarget - PHD_180;
					break;
				case RINGSTATE_KEYS2MAIN :
					Inv_RingMotionSetup(&ring, RINGSTATE_OPENING, RINGSTATE_OPEN, 24);
					Inv_RingMotionRadius(&ring, 0x2B0);

					ring.cameraPitch = -motion.misc;
					motion.cameraRate_pitch = motion.misc / 24;
					motion.cameraTarget_pitch = 0;

					ring.itemList = InvMainList;
					ring.type = RING_Main;
					InvKeyObjectsCount = ring.objCount;
					InvKeysCurrent = ring.currentObj;
					ring.objCount = InvMainObjectsCount;
					ring.currentObj = InvMainCurrent;

					Inv_RingCalcAdders(&ring, 24);
					Inv_RingMotionRotation(&ring, -PHD_180, -PHD_90 - ring.angleAdder * ring.currentObj);
					ring.ringPos.rotY = motion.rotateTarget - PHD_180;
					break;
				case RINGSTATE_OPTION2MAIN :
					Inv_RingMotionSetup(&ring, RINGSTATE_OPENING, RINGSTATE_OPEN, 24);
					Inv_RingMotionRadius(&ring, 0x2B0);

					ring.cameraPitch = -motion.misc;
					motion.cameraRate_pitch = motion.misc / 24;
					motion.cameraTarget_pitch = 0;

					ring.itemList = InvMainList;
					ring.type = RING_Main;
					InvOptionObjectsCount = ring.objCount;
					InvOptionCurrent = ring.currentObj;
					ring.objCount = InvMainObjectsCount;
					ring.currentObj = InvMainCurrent;

					Inv_RingCalcAdders(&ring, 24);
					Inv_RingMotionRotation(&ring, -PHD_180, -PHD_90 - ring.angleAdder * ring.currentObj);
					ring.ringPos.rotY = motion.rotateTarget - PHD_180;
					break;
				case RINGSTATE_SELECTED :
#ifdef FEATURE_HUD_IMPROVED
					DisplayJoystickHintText(true, InventoryMode != INV_DeathMode);
#endif // FEATURE_HUD_IMPROVED
					item = ring.itemList[ring.currentObj];

					if( item->objectID == ID_PASSPORT_CLOSED ) {
						item->objectID = ID_PASSPORT_OPTION;
					}
					for( int i = 0; i < nTicks; ++i ) {
						itemAnimateFrame = 0;
						if( item->zRot == item->yRot ) {
							itemAnimateFrame = AnimateInventoryItem(item);
						}
					}

					if( !itemAnimateFrame && !IsInvOptionsDelay ) {
						do_inventory_options(item);

						if( CHK_ANY(InputDB, IN_DESELECT) ) {
							item->sprites = NULL;
							Inv_RingMotionSetup(&ring, RINGSTATE_CLOSING_ITEM, RINGSTATE_DESELECT, 0);
							InputStatus = 0;
							InputDB = 0;

							if( InventoryMode == INV_SaveMode || InventoryMode == INV_LoadMode ) {
								Inv_RingMotionSetup(&ring, RINGSTATE_CLOSING_ITEM, RINGSTATE_EXITING_INVENTORY, 0);
								InputDB = 0;
								InputStatus = 0;
							}
						}

						if( CHK_ANY(InputDB, IN_SELECT) ) {
							item->sprites = NULL;
							InventoryChosen = item->objectID;

							if( ring.type == RING_Main ) {
								InvMainCurrent = ring.currentObj;
							} else {
								InvOptionCurrent = ring.currentObj;
							}

							if( InventoryMode == INV_TitleMode
								&& (item->objectID == ID_DETAIL_OPTION
								||  item->objectID == ID_SOUND_OPTION
								||  item->objectID == ID_CONTROL_OPTION
								||  item->objectID == ID_GAMMA_OPTION) )
							{
								Inv_RingMotionSetup(&ring, RINGSTATE_CLOSING_ITEM, RINGSTATE_DESELECT, 0);
							} else {
								Inv_RingMotionSetup(&ring, RINGSTATE_CLOSING_ITEM, RINGSTATE_EXITING_INVENTORY, 0);
							}

							InputStatus = 0;
							InputDB = 0;
						}
					}
					break;
				case RINGSTATE_DESELECT :
#ifdef FEATURE_HUD_IMPROVED
					RemoveJoystickHintText(false, InventoryMode == INV_TitleMode || InventoryMode == INV_DeathMode);
#endif // FEATURE_HUD_IMPROVED
					PlaySoundEffect(112, 0, SFX_ALWAYS);
					Inv_RingMotionSetup(&ring, RINGSTATE_DESELECTING, RINGSTATE_OPEN, 16);
					Inv_RingMotionRotation(&ring, 0, -PHD_90 - ring.angleAdder * ring.currentObj);
					InputStatus = 0;
					InputDB = 0;
					break;
				case RINGSTATE_CLOSING_ITEM :
					item = ring.itemList[ring.currentObj];
					for( int i = 0; i < nTicks; ++i ) {
						if( !AnimateInventoryItem(item) ) {
							if( item->objectID == ID_PASSPORT_OPTION ) {
								item->objectID = ID_PASSPORT_CLOSED;
								item->currentFrame = 0;
							}
							motion.framesCount = 16;
							motion.status = motion.statusTarget;
							Inv_RingMotionItemDeselect(&ring, item);
						}
					}
					break;
				case RINGSTATE_CLOSING :
#ifdef FEATURE_HUD_IMPROVED
					RemoveJoystickHintText(true, true);
#endif // FEATURE_HUD_IMPROVED
					break;
				case RINGSTATE_EXITING_INVENTORY :
#ifdef FEATURE_HUD_IMPROVED
					RemoveJoystickHintText(true, true);
#endif // FEATURE_HUD_IMPROVED
					if( !motion.framesCount ) {
						if( InventoryMode == INV_TitleMode ) {
							S_FadeOutInventory(FALSE);
						} else {
							S_FadeOutInventory(TRUE);
						}
						Inv_RingMotionSetup(&ring, RINGSTATE_CLOSING, RINGSTATE_DONE, 32);
						Inv_RingMotionRadius(&ring, 0);
						Inv_RingMotionCameraPos(&ring, -0x600);
						Inv_RingMotionRotation(&ring, -PHD_180, ring.ringPos.rotY - PHD_180);
					}
					break;
			}
		}
#ifdef FEATURE_INPUT_IMPROVED
		UpdateJoyOutput(false);
#endif // FEATURE_INPUT_IMPROVED
	} while( motion.status != RINGSTATE_DONE );

#ifdef FEATURE_HUD_IMPROVED
	RemoveJoystickHintText(true, true);
#endif // FEATURE_HUD_IMPROVED
	RemoveInventoryText();
	S_FinishInventory();
	IsInventoryActive = 0;

	if( IsResetFlag ) {
		return GF_EXIT_TO_TITLE;
	}

	if( isDemoNeeded ) {
		return GF_START_DEMO;
	}

	switch( InventoryChosen ) {
		case ID_PASSPORT_OPTION :
			if( MusicVolume && InventoryExtraData[0] == 1 ) {
				S_CDVolume(25 * MusicVolume + 5);
			}
			return 1;
		case ID_PHOTO_OPTION :
			if( CHK_ANY(GF_GameFlow.flags, GFF_GymEnabled) ) {
				InventoryExtraData[1] = 0;
				return 1;
			}
			break;
		case ID_PISTOL_OPTION :
			UseItem(ID_PISTOL_OPTION);
			break;
		case ID_SHOTGUN_OPTION :
			UseItem(ID_SHOTGUN_OPTION);
			break;
		case ID_MAGNUM_OPTION :
			UseItem(ID_MAGNUM_OPTION);
			break;
		case ID_UZI_OPTION :
			UseItem(ID_UZI_OPTION);
			break;
		case ID_HARPOON_OPTION :
			UseItem(ID_HARPOON_OPTION);
			break;
		case ID_M16_OPTION :
			UseItem(ID_M16_OPTION);
			break;
		case ID_GRENADE_OPTION :
			UseItem(ID_GRENADE_OPTION);
			break;
		case ID_SMALL_MEDIPACK_OPTION :
			UseItem(ID_SMALL_MEDIPACK_OPTION);
			break;
		case ID_LARGE_MEDIPACK_OPTION :
			UseItem(ID_LARGE_MEDIPACK_OPTION);
			break;
		case ID_FLARES_OPTION :
			UseItem(ID_FLARES_OPTION);
			break;
	}

	if( MusicVolume && InventoryMode != INV_TitleMode ) {
#ifdef FEATURE_AUDIO_IMPROVED
		if( Camera.underwater ) {
			extern double UnderwaterMusicMute;
			double volume = (1.0 - UnderwaterMusicMute) * (double)(MusicVolume * 25 + 5);
			if( volume >= 1.0 ) {
				S_CDVolume((DWORD)volume);
			} else {
				S_CDVolume(0);
			}
		} else {
			S_CDVolume(MusicVolume * 25 + 5);
		}
#else // FEATURE_AUDIO_IMPROVED
		S_CDVolume(MusicVolume * 25 + 5);
#endif // FEATURE_AUDIO_IMPROVED
	}
	return 0;
}

void __cdecl Construct_Inventory() {
	S_SetupAboveWater(FALSE);

	if( InventoryMode != INV_TitleMode ) {
		TempVideoAdjust(HiRes, 1.0);
	}

	memset(InventoryExtraData, 0, sizeof(InventoryExtraData));

	PhdWinLeft = 0;
	PhdWinTop = 0;
	PhdWinRight = PhdWinMaxX;
	PhdWinBottom = PhdWinMaxY;

	IsInventoryActive = 1;
	InventoryChosen = 0;
	InvOptionObjectsCount = ARRAY_SIZE(InvOptionList) - ((InventoryMode == INV_TitleMode) ? 0 : 1);

	for( int i = 0; i < InvMainObjectsCount; ++i ) {
		InvMainList[i]->currentFrame = 0;
		InvMainList[i]->meshesDrawn = InvMainList[i]->meshesSel;
		InvMainList[i]->goalFrame = 0;
		InvMainList[i]->animCount = 0;
		InvMainList[i]->zRot = 0;
	}

	for( int i = 0; i < InvOptionObjectsCount; ++i ) {
		InvOptionList[i]->currentFrame = 0;
		InvOptionList[i]->goalFrame = 0;
		InvOptionList[i]->animCount = 0;
		InvOptionList[i]->zRot = 0;
	}

	InvMainCurrent = 0;
	if( GymInvOpenEnabled && InventoryMode == INV_TitleMode && !CHK_ANY(GF_GameFlow.flags, GFF_LoadSaveDisabled) && CHK_ANY(GF_GameFlow.flags, GFF_GymEnabled) ) {
		InvOptionCurrent = ARRAY_SIZE(InvOptionList) - 1;
	} else {
		InvOptionCurrent = 0;
		GymInvOpenEnabled = FALSE;
	}

	SoundOptionLine = 0;
}

void __cdecl SelectMeshes(INVENTORY_ITEM *invItem) {
	if( invItem->objectID == ID_PASSPORT_OPTION ) {
		if (invItem->currentFrame < 4) {
			invItem->meshesDrawn = PASS_BASE | PASS_INFRONT;
		} else if (invItem->currentFrame <= 16) {
			invItem->meshesDrawn = PASS_BASE | PASS_INFRONT | PASS_PAGE1;
		} else if (invItem->currentFrame < 19) {
			invItem->meshesDrawn = PASS_BASE | PASS_INFRONT | PASS_PAGE1 | PASS_PAGE2;
		} else if (invItem->currentFrame == 19) {
			invItem->meshesDrawn = PASS_BASE | PASS_PAGE1 | PASS_PAGE2;
		} else if (invItem->currentFrame < 24) {
			invItem->meshesDrawn = PASS_BASE | PASS_PAGE1 | PASS_PAGE2 | PASS_INBACK;
		} else if (invItem->currentFrame < 29) {
			invItem->meshesDrawn = PASS_BASE | PASS_PAGE2 | PASS_INBACK;
		} else if (invItem->currentFrame == 29) {
			invItem->meshesDrawn = PASS_BASE;
		}
	} else if (invItem->objectID != ID_GAMMA_OPTION) {
		invItem->meshesDrawn = ~0;
	}
}

int __cdecl AnimateInventoryItem(INVENTORY_ITEM *invItem) {
	int frame = invItem->currentFrame;
	int animID = invItem->animCount;

	if( frame == invItem->goalFrame ) {
		SelectMeshes(invItem);
		return 0;
	}

	if( animID ) {
		invItem->animCount = (animID - 1);
		SelectMeshes(invItem);
		return 1;
	}

	invItem->animCount = invItem->animSpeed;
	invItem->currentFrame = (frame + invItem->animDirection);

	if( invItem->currentFrame >= invItem->framesTotal ) {
		invItem->currentFrame = 0;
	} else if( invItem->currentFrame < 0 ) {
		invItem->currentFrame = invItem->framesTotal - 1;
	}

	SelectMeshes(invItem);
	return 1;
}

void __cdecl DrawInventoryItem(INVENTORY_ITEM* invItem) {
	int hours=0, minutes=0, seconds=0, totsec=0, clip=0;
	short* frame[2] = {0};

	if( invItem->objectID == ID_COMPASS_OPTION ) {
		totsec = SaveGame.statistics.timer/30;
		seconds = totsec%60;
		minutes = ((totsec%3600)*-91)/5;
		hours = ((totsec/12)*-91)/5;
		seconds *= -1092;
	}

	phd_TranslateRel(0, invItem->yTrans, invItem->zTrans);
	phd_RotYXZ(invItem->zRot, invItem->yRotSel, 0);
	OBJECT_INFO *obj = &Objects[invItem->objectID];

	if( !obj->loaded ) {
		return;
	}

	if( obj->nMeshes < 0 ) {
		S_DrawSprite(0, 0, 0, 0, obj->meshIndex, 0, 0);
		return;
	}

	if( invItem->sprites ) {
		clip = PhdMatrixPtr->_23;
		int sx = PhdWinCenterX + PhdMatrixPtr->_03 / (clip / PhdPersp);
		int sy = PhdWinCenterY + PhdMatrixPtr->_13 / (clip / PhdPersp);
		INVENTORY_SPRITE *sprite = (INVENTORY_SPRITE*)invItem->sprites;

		for( int i = sprite->y; sprite->shape != 0; i += 4) {
			if (clip < PhdNearZ || clip > PhdFarZ) {
				break;
			}
			for( int j = sprite->shape; j != 0; j++ ) {
				switch( j ) {
					case 1 :
						S_DrawScreenSprite(sx+sprite->x, sy+sprite->y,sprite->z,sprite->param1,sprite->param2,StaticObjects[ID_ALPHABET].meshIndex+sprite->invColour,0x1000,0);
						break;
					case 2 :
						S_DrawScreenLine(sx+sprite->x, sy+sprite->y,sprite->z,sprite->param1,sprite->param2,sprite->invColour,(D3DCOLOR*)sprite->gour,0);
						break;
					case 3 :
						S_DrawScreenBox(sx+sprite->x, sy+sprite->y,sprite->z,sprite->param1,sprite->param2,sprite->invColour,(GOURAUD_OUTLINE*)sprite->gour,0);
						break;
					case 4 :
						S_DrawScreenFBox(sx+sprite->x, sy+sprite->y,sprite->z,sprite->param1,sprite->param2,sprite->invColour,(GOURAUD_FILL*)sprite->gour,0);
						break;
				}
			}
		}
	}

	phd_PushMatrix();
	frame[0] = (&obj->frameBase[invItem->currentFrame * (Anims[obj->animIndex].interpolation >> 8)]);
	clip = S_GetObjectBounds(frame[0]);
	if( clip ) {
		phd_TranslateRel((int)*(frame[0] + 6), (int)*(frame[0] + 7), (int)*(frame[0] + 8));
		UINT16 *rotation = (UINT16 *)(frame[0] + 9);
		phd_RotYXZsuperpack(&rotation, 0);
		__int16 mesh = obj->meshIndex;
		int *bones = &AnimBones[obj->boneIndex];
		__int16 mesh_num = 1;

		if( mesh_num & invItem->meshesDrawn ) {
#ifdef FEATURE_VIDEOFX_IMPROVED
			SetMeshReflectState(invItem->objectID, 0);
#endif // FEATURE_VIDEOFX_IMPROVED
			phd_PutPolygons(MeshPtr[mesh], clip);
#ifdef FEATURE_VIDEOFX_IMPROVED
			ClearMeshReflectState();
#endif // FEATURE_VIDEOFX_IMPROVED
		}

		for( int i = obj->nMeshes-1; i > 0; --i ) {
			++mesh;
			mesh_num <<= 1;

			int pushpop = *(bones++);
			if( CHK_ANY(pushpop, 1) ) {
				phd_PopMatrix();
			}
			if( CHK_ANY(pushpop, 2) ) {
				phd_PushMatrix();
			}

			phd_TranslateRel(bones[0], bones[1], bones[2]);
			phd_RotYXZsuperpack(&rotation, 0);

			if( invItem->objectID == ID_COMPASS_OPTION ) {
				switch( i ) {
					case 2 :
						phd_RotZ(seconds);
						invItem->reserved2 = invItem->reserved1;
						invItem->reserved1 = seconds;
						break;
					case 3 :
						phd_RotZ(minutes);
						break;
					case 4 :
						phd_RotZ(hours);
						break;
				}
			}

			if( CHK_ANY(mesh_num, invItem->meshesDrawn) ) {
#ifdef FEATURE_VIDEOFX_IMPROVED
				SetMeshReflectState(invItem->objectID, obj->nMeshes-i);
#endif // FEATURE_VIDEOFX_IMPROVED
				phd_PutPolygons(MeshPtr[mesh], clip);
#ifdef FEATURE_VIDEOFX_IMPROVED
				ClearMeshReflectState();
#endif // FEATURE_VIDEOFX_IMPROVED
			}
			bones += 3;
		}
	}
	phd_PopMatrix();
}

DWORD __cdecl GetDebouncedInput(DWORD input) {
	static DWORD oldInput = 0;
	DWORD result = input & ~oldInput;
#ifdef FEATURE_INPUT_IMPROVED
	static int holdBack = -12;
	static int holdForward = -12;

	if( !CHK_ANY(input, IN_BACK) || CHK_ANY(input, IN_FORWARD) ) {
		holdBack = -12;
	} else if( CHK_ANY(input, IN_BACK) && ++holdBack >= 3 ) {
		result |= IN_BACK;
		holdBack = 0;
	}

	if( !CHK_ANY(input, IN_FORWARD) || CHK_ANY(input, IN_BACK) ) {
		holdForward = -12;
	} else if( CHK_ANY(input, IN_FORWARD) && ++holdForward >= 3 ) {
		result |= IN_FORWARD;
		holdForward = 0;
	}
#endif // FEATURE_INPUT_IMPROVED
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
