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

#ifndef BOAT_H_INCLUDED
#define BOAT_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
//	0x0040CB10:		InitialiseBoat
//	0x0040CB50:		BoatCheckGeton
//	0x0040CCC0:		BoatCollision
//	0x0040CE20:		TestWaterHeight

void __cdecl DoBoatShift(int itemID); // 0x0040CF20
void __cdecl DoWakeEffect(ITEM_INFO *item); // 0x0040D0F0

//	0x0040D270:		DoBoatDynamics
//	0x0040D2C0:		BoatDynamics
//	0x0040D7A0:		BoatUserControl
//	0x0040D930:		BoatAnimation
//	0x0040DAA0:		BoatControl

void __cdecl GondolaControl(__int16 itemID); // 0x0040E0D0

#endif // BOAT_H_INCLUDED
