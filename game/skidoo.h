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

#ifndef SKIDOO_H_INCLUDED
#define SKIDOO_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
#define InitialiseSkidoo ((void(__cdecl*)(__int16)) 0x0043CEE0)

//	0x0043CF20:		SkidooCheckGeton
//	0x0043D010:		SkidooCollision
//	0x0043D110:		SkidooBaddieCollision
//	0x0043D310:		TestHeight
//	0x0043D3D0:		DoShift
//	0x0043D650:		DoDynamics
//	0x0043D6B0:		GetCollisionAnim

void __cdecl DoSnowEffect(ITEM_INFO *item); // 0x0043D740

//	0x0043D880:		SkidooDynamics
//	0x0043DD20:		SkidooUserControl
//	0x0043DEE0:		SkidooCheckGetOffOK
//	0x0043DFF0:		SkidooAnimation

void __cdecl SkidooExplode(ITEM_INFO *item); // 0x0043E2D0

//	0x0043E350:		SkidooCheckGetOff

void __cdecl SkidooGuns(); // 0x0043E590

//	0x0043E6B0:		SkidooControl

void __cdecl DrawSkidoo(ITEM_INFO *item);

#define InitialiseSkidman ((void(__cdecl*)(__int16)) 0x0043EDF0)
#define SkidManControl ((void(__cdecl*)(__int16)) 0x0043EE80)

//	0x0043F280:		SkidmanPush

#define SkidmanCollision ((void(__cdecl*)(__int16, ITEM_INFO *, COLL_INFO *)) 0x0043F3A0)

#endif // SKIDOO_H_INCLUDED
