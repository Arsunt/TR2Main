/*
 * Copyright (c) 2017-2021 Michael Chaban. All rights reserved.
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

#ifndef CONTROL_H_INCLUDED
#define CONTROL_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
int __cdecl ControlPhase(int nTicks, BOOL demoMode);

#define AnimateItem ((void(__cdecl*)(ITEM_INFO*)) 0x004146C0)

// 0x00414A30:		GetChange
// 0x00414AE0:		TranslateItem

#define GetFloor ((FLOOR_INFO*(__cdecl*)(int, int, int, __int16*)) 0x00414B40)
#define GetWaterHeight ((int(__cdecl*)(int, int, int, __int16)) 0x00414CE0)
#define GetHeight ((int(__cdecl*)(FLOOR_INFO*, int, int, int)) 0x00414E50)

// 0x004150D0:		RefreshCamera

#define TestTriggers ((void(__cdecl*)(__int16*, BOOL)) 0x004151C0)
#define TriggerActive ((int(__cdecl*)(ITEM_INFO*)) 0x004158A0)
#define GetCeiling ((int(__cdecl*)(FLOOR_INFO*, int, int, int)) 0x00415900)

// 0x00415B60:		GetDoor

#define LOS ((int(__cdecl*)(GAME_VECTOR*, GAME_VECTOR*)) 0x00415BB0)

// 0x00415C50:		zLOS
// 0x00415F40:		xLOS
// 0x00416230:		ClipTarget
// 0x00416310:		ObjectOnLOS

#define FlipMap ((void(__cdecl*)(void)) 0x00416610)

// 0x004166D0:		RemoveRoomFlipItems
// 0x00416770:		AddRoomFlipItems

void __cdecl TriggerCDTrack(__int16 value, UINT16 flags, __int16 type); // 0x004167D0
void __cdecl TriggerNormalCDTrack(__int16 value, UINT16 flags, __int16 type); // 0x00416800;

#endif // CONTROL_H_INCLUDED
