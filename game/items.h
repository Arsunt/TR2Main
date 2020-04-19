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

#ifndef ITEMS_H_INCLUDED
#define ITEMS_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
void __cdecl InitialiseItemArray(int itemCount); // 0x00426CD0

// 0x00426D30:		KillItem
// 0x00426E50:		CreateItem

void __cdecl InitialiseItem(__int16 itemIndex); // 0x00426E90

// 0x00427050:		RemoveActiveItem
// 0x004270E0:		RemoveDrawnItem

void __cdecl AddActiveItem(__int16 itemIndex); // 0x00427150

// 0x004271B0:		ItemNewRoom

int __cdecl GlobalItemReplace(int oldItemID, int newItemID); // 0x00427250

// 0x004272D0:		InitialiseFXArray

#define CreateEffect ((__int16(__cdecl*)(__int16)) 0x00427300)
#define KillEffect ((void(__cdecl*)(__int16)) 0x00427370)

// 0x00427460:		EffectNewRoom
// 0x00427500:		ClearBodyBag

#endif // ITEMS_H_INCLUDED
