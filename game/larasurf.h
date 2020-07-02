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

#ifndef LARA_SURF_H_INCLUDED
#define LARA_SURF_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
#define LaraSurface ((void(__cdecl*)(ITEM_INFO*, COLL_INFO*)) 0x00431710)

//	0x00431870:		lara_as_surfswim
//	0x004318E0:		lara_as_surfback
//	0x00431940:		lara_as_surfleft
//	0x004319A0:		lara_as_surfright
//	0x00431A00:		lara_as_surftread
//	0x00431AC0:		lara_col_surfswim
//	0x00431B00:		lara_col_surfback
//	0x00431B30:		lara_col_surfleft
//	0x00431B60:		lara_col_surfright
//	0x00431B90:		lara_col_surftread
//	0x00431BF0:		LaraSurfaceCollision
//	0x00431CF0:		LaraTestWaterStepOut
//	0x00431DE0:		LaraTestWaterClimbOut

#endif // LARA_SURF_H_INCLUDED
