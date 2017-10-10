/*
 * Copyright (c) 2017 Michael Chaban. All rights reserved.
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

#ifndef INVFUNC_H_INCLUDED
#define INVFUNC_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
void __cdecl InitColours(); // 0x00423B10

// 0x00423C20:		RingIsOpen
// 0x00423D90:		RingIsNotOpen
// 0x00423E20:		RingNotActive
// 0x00424290:		RingActive

BOOL __cdecl Inv_AddItem(GAME_OBJECT_ID itemID); // 0x004242D0
void __cdecl Inv_InsertItem(INVENTORY_ITEM *item); // 0x00424AE0
int __cdecl Inv_RequestItem(GAME_OBJECT_ID itemID); // 0x00424C10
void __cdecl Inv_RemoveAllItems(); // 0x00424C90
BOOL __cdecl Inv_RemoveItem(GAME_OBJECT_ID itemID); // 0x00424CB0
GAME_OBJECT_ID __cdecl Inv_GetItemOption(GAME_OBJECT_ID itemID); // 0x00424DC0

// 0x00424FB0:		RemoveInventoryText
// 0x00424FE0:		Inv_RingInit
// 0x004250F0:		Inv_RingGetView
// 0x00425150:		Inv_RingLight
// 0x00425190:		Inv_RingCalcAdders
// 0x004251C0:		Inv_RingDoMotions
// 0x00425300:		Inv_RingRotateLeft
// 0x00425330:		Inv_RingRotateRight
// 0x00425360:		Inv_RingMotionInit
// 0x004253D0:		Inv_RingMotionSetup
// 0x00425400:		Inv_RingMotionRadius
// 0x00425430:		Inv_RingMotionRotation
// 0x00425460:		Inv_RingMotionCameraPos
// 0x00425490:		Inv_RingMotionCameraPitch
// 0x004254B0:		Inv_RingMotionItemSelect
// 0x00425510:		Inv_RingMotionItemDeselect

#endif // INVFUNC_H_INCLUDED
