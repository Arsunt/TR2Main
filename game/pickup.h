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

#ifndef PICKUP_H_INCLUDED
#define PICKUP_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */

#define PickUpCollision ((void(__cdecl*)(__int16, ITEM_INFO *, COLL_INFO *)) 0x00437F20)

//	0x004383A0:		SwitchCollision
//	0x004385B0:		SwitchCollision2
//	0x004386B0:		DetonatorCollision
//	0x004388F0:		KeyHoleCollision

#define PuzzleHoleCollision ((void(__cdecl*)(__int16, ITEM_INFO *, COLL_INFO *)) 0x00438B30)

//	0x00438DF0:		SwitchControl
//	0x00438E30:		SwitchTrigger
//	0x00438EF0:		KeyTrigger
//	0x00438F30:		PickupTrigger
//	0x00438F70:		SecretControl

#endif // PICKUP_H_INCLUDED
