/*
 * Copyright (c) 2017-2019 Michael Chaban. All rights reserved.
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

#ifndef LARAMISC_H_INCLUDED
#define LARAMISC_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
// 0x00430380:		LaraControl
// 0x00430A10:		AnimateLara

void __cdecl UseItem(__int16 itemID); // 0x00430D10

// 0x00430ED0:		LaraCheatGetStuff
// 0x00430F90:		ControlLaraExtra
// 0x00430FB0:		InitialiseLaraLoad
// 0x00430FE0:		InitialiseLara
// 0x004312A0:		InitialiseLaraInventory
// 0x00431610:		LaraInitialiseMeshes

#endif // LARAMISC_H_INCLUDED
