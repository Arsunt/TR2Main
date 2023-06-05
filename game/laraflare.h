/*
 * Copyright (c) 2017-2020 Michael Chaban. All rights reserved.
 * Original game is created by Core Design Ltd. in 1997.
 * Lara Croft and Tomb Raider are trademarks of Embracer Group AB.
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

#ifndef LARA_FLARE_H_INCLUDED
#define LARA_FLARE_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
//	0x0042F840:		DoFlareLight
//	0x0042F8E0:		DoFlareInHand

void __cdecl DrawFlareInAir(ITEM_INFO *item);

//	0x0042FAC0:		CreateFlare
//	0x0042FCA0:		set_flare_arm
//	0x0042FCF0:		draw_flare
//	0x0042FE60:		undraw_flare
//	0x00430090:		draw_flare_meshes
//	0x004300B0:		undraw_flare_meshes
//	0x004300D0:		ready_flare
//	0x00430110:		FlareControl

#endif // LARA_FLARE_H_INCLUDED
