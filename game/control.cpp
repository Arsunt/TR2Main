/*
 * Copyright (c) 2017-2020 Michael Chaban. All rights reserved.
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
#include "game/control.h"
#include "game/camera.h"
#include "game/demo.h"
#include "game/effects.h"
#include "game/hair.h"
#include "game/inventory.h"
#include "game/laramisc.h"
#include "game/savegame.h"
#include "specific/game.h"
#include "specific/input.h"
#include "specific/smain.h"
#include "specific/sndpc.h"
#include "global/vars.h"

#ifdef FEATURE_BACKGROUND_IMPROVED
#include "modding/pause.h"
#endif // FEATURE_BACKGROUND_IMPROVED

#ifdef FEATURE_INPUT_IMPROVED
#include "modding/joy_output.h"
#endif // FEATURE_INPUT_IMPROVED

int __cdecl ControlPhase(int nTicks, BOOL demoMode) {
	static int tickCount = 0;
	int id = -1;
	int next = -1;
	int result = 0;

	CLAMPG(nTicks, 5 * TICKS_PER_FRAME);
	for( tickCount += nTicks; tickCount > 0; tickCount -= TICKS_PER_FRAME ) {
		if( CD_TrackID > 0 ) {
			S_CDLoop();
		}
		if( !CHK_ANY(GF_GameFlow.flags, GFF_CheatModeCheckDisabled) ) {
			CheckCheatMode();
		}
		if( IsLevelComplete ) {
			return 1;
		}
		S_UpdateInput();
		if( IsResetFlag ) {
			return GF_EXIT_TO_TITLE;
		}
		if( demoMode ) {
			if( InputStatus ) {
				return GF_GameFlow.onDemo_Interrupt;
			}
			GetDemoInput();
			if( InputStatus == (DWORD)~0 ) {
				InputStatus = 0;
				return GF_GameFlow.onDemo_End;
			}
		} else {
			if( CHK_ANY(GF_GameFlow.flags, GFF_NoInputTimeout) ) {
				if( InputStatus ) {
					NoInputCounter = 0;
				} else if( ++NoInputCounter > GF_GameFlow.noInput_Time ) {
					return GF_START_DEMO;
				}
			}
		}
		if( OverlayStatus == 2 || Lara.death_count > 10*30 || (Lara.death_count > 2*30 && InputStatus) ) {
			if( demoMode ) {
				return GF_GameFlow.onDeath_DemoMode;
			}
			if( CurrentLevel == 0 ) { // Lara's Home
				return GF_EXIT_TO_TITLE;
			}
			if( OverlayStatus == 2 ) {
				OverlayStatus = 1;
				result = Display_Inventory(INV_DeathMode);
				if( result ) {
					return result;
				}
			} else {
				OverlayStatus = 2;
			}
		}
		if( !Lara.death_count && !Lara.extra_anim && (CHK_ANY(InputStatus, IN_OPTION|IN_LOAD|IN_SAVE) || OverlayStatus <= 0) ) {
			if( OverlayStatus > 0 ) {
				if( CHK_ANY(GF_GameFlow.flags, GFF_LoadSaveDisabled) ) {
					OverlayStatus = 0;
				} else if( CHK_ANY(InputStatus, IN_LOAD) ) {
					OverlayStatus = -1;
				} else if( CHK_ANY(InputStatus, IN_SAVE) ) {
					OverlayStatus = -2;
				} else {
					OverlayStatus = 0;
				}
			} else {
				if( OverlayStatus == -1 ) {
					result = Display_Inventory(INV_LoadMode);
				} else if( OverlayStatus == -2 ) {
					result = Display_Inventory(INV_SaveMode);
				} else {
					result = Display_Inventory(INV_GameMode);
				}
				OverlayStatus = 1;
				if( result ) {
					if( InventoryExtraData[0] != 1 ) {
						return result;

					}
					if( CurrentLevel == 0 ) { // Lara's Home
						return 1;
					}
					CreateSaveGameInfo();
					S_SaveGame(&SaveGame, sizeof(SaveGame), InventoryExtraData[1]);
					S_SaveSettings();
				}
			}
		}

#ifdef FEATURE_BACKGROUND_IMPROVED
		if( !Lara.death_count && !Lara.extra_anim && CHK_ANY(InputStatus, IN_PAUSE) && S_Pause() ) {
			return 1;
		}
#endif // FEATURE_BACKGROUND_IMPROVED

		DynamicLightCount = 0;

		for( id = NextItemActive; id >= 0; id = next ) {
			next = Items[id].nextActive;
			// NOTE: there is no IFL_CLEARBODY check in the original code
			if( Objects[Items[id].objectID].control && !CHK_ANY(Items[id].flags, IFL_CLEARBODY) ) {
				Objects[Items[id].objectID].control(id);
			}
		}

		for( id = NextEffectActive; id >= 0; id = next ) {
			next = Effects[id].next_active;
			if( Objects[Effects[id].object_number].control ) {
				Objects[Effects[id].object_number].control(id);
			}
		}

		LaraControl(0);
		HairControl(0);
		CalculateCamera();
		SoundEffects();
		--HealthBarTimer;

		// Update statistics timer for normal levels
		if( CurrentLevel != 0 || IsAssaultTimerActive ) {
			++SaveGame.statistics.timer;
		}
	}
#ifdef FEATURE_INPUT_IMPROVED
	UpdateJoyOutput(!IsDemoLevelType);
#endif // FEATURE_INPUT_IMPROVED
	return 0;
}

int __cdecl ClipTarget(GAME_VECTOR *start, GAME_VECTOR *target, FLOOR_INFO *floor) {
	int dx, dy, dz, height, ceiling;

	dx = target->x - start->x;
	dy = target->y - start->y;
	dz = target->z - start->z;
	height = GetHeight(floor, target->x, target->y, target->z);
	if (target->y > height && start->y < height) {
		target->y = height;
		target->x = start->x + (target->y - start->y) * dx / dy;
		target->z = start->z + (target->y - start->y) * dz / dy;
		return 0;
	}
	ceiling = GetCeiling(floor, target->x, target->y, target->z);
	if (target->y < ceiling && start->y > ceiling) {
		target->y = ceiling;
		target->x = start->x + (target->y - start->y) * dx / dy;
		target->z = start->z + (target->y - start->y) * dz / dy;
		return 0;
	}
	return 1;
}

/*
 * Inject function
 */
void Inject_Control() {
	INJECT(0x00414370, ControlPhase);
//	INJECT(0x004146C0, AnimateItem);
//	INJECT(0x00414A30, GetChange);
//	INJECT(0x00414AE0, TranslateItem);
//	INJECT(0x00414B40, GetFloor);
//	INJECT(0x00414CE0, GetWaterHeight);
//	INJECT(0x00414E50, GetHeight);
//	INJECT(0x004150D0, RefreshCamera);
//	INJECT(0x004151C0, TestTriggers);
//	INJECT(0x004158A0, TriggerActive);
//	INJECT(0x00415900, GetCeiling);
//	INJECT(0x00415B60, GetDoor);
//	INJECT(0x00415BB0, LOS);
//	INJECT(0x00415C50, zLOS);
//	INJECT(0x00415F40, xLOS);

	INJECT(0x00416230, ClipTarget);

//	INJECT(0x00416310, ObjectOnLOS);
//	INJECT(0x00416610, FlipMap);
//	INJECT(0x004166D0, RemoveRoomFlipItems);
//	INJECT(0x00416770, AddRoomFlipItems);
//	INJECT(0x004167D0, TriggerCDTrack);
//	INJECT(0x00416800, TriggerNormalCDTrack);
}
