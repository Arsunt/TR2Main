/*
 * Copyright (c) 2017-2018 Michael Chaban. All rights reserved.
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
void __cdecl RingIsOpen(RING_INFO *ring); // 0x00423C20
void __cdecl RingIsNotOpen(); // 0x00423D90
void __cdecl RingNotActive(INVENTORY_ITEM *item); // 0x00423E20
void __cdecl RingActive(); // 0x00424290
BOOL __cdecl Inv_AddItem(GAME_OBJECT_ID itemID); // 0x004242D0
void __cdecl Inv_InsertItem(INVENTORY_ITEM *item); // 0x00424AE0
int __cdecl Inv_RequestItem(GAME_OBJECT_ID itemID); // 0x00424C10
void __cdecl Inv_RemoveAllItems(); // 0x00424C90
BOOL __cdecl Inv_RemoveItem(GAME_OBJECT_ID itemID); // 0x00424CB0
GAME_OBJECT_ID __cdecl Inv_GetItemOption(GAME_OBJECT_ID itemID); // 0x00424DC0
void __cdecl RemoveInventoryText(); // 0x00424FB0
void __cdecl Inv_RingInit(RING_INFO *ring, __int16 type, INVENTORY_ITEM **itemList, __int16 objCount, __int16 currentObj, INV_MOTION_INFO *motionInfo); // 0x00424FE0
void __cdecl Inv_RingGetView(RING_INFO *ring, PHD_3DPOS *view); // 0x004250F0
void __cdecl Inv_RingLight(RING_INFO *ring); // 0x00425150
void __cdecl Inv_RingCalcAdders(RING_INFO *ring, __int16 rotDuration); // 0x00425190
void __cdecl Inv_RingDoMotions(RING_INFO *ring); // 0x004251C0
void __cdecl Inv_RingRotateLeft(RING_INFO *ring); // 0x00425300
void __cdecl Inv_RingRotateRight(RING_INFO *ring); // 0x00425330
void __cdecl Inv_RingMotionInit(RING_INFO *ring, __int16 framesCount, __int16 status, __int16 statusTarget); // 0x00425360
void __cdecl Inv_RingMotionSetup(RING_INFO *ring, __int16 status, __int16 statusTarget, __int16 framesCount); // 0x004253D0
void __cdecl Inv_RingMotionRadius(RING_INFO *ring, __int16 target); // 0x00425400
void __cdecl Inv_RingMotionRotation(RING_INFO *ring, __int16 rotation, __int16 target); // 0x00425430
void __cdecl Inv_RingMotionCameraPos(RING_INFO *ring, __int16 target); // 0x00425460
void __cdecl Inv_RingMotionCameraPitch(RING_INFO *ring, __int16 target); // 0x00425490
void __cdecl Inv_RingMotionItemSelect(RING_INFO *ring, INVENTORY_ITEM *item); // 0x004254B0
void __cdecl Inv_RingMotionItemDeselect(RING_INFO *ring, INVENTORY_ITEM *item); // 0x00425510

#endif // INVFUNC_H_INCLUDED
