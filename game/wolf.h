/*
 * Copyright (c) 2017-2019 Michael Chaban. All rights reserved.
 * Original game is created by Core Design Ltd. in 1997.
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

#ifndef WOLF_H_INCLUDED
#define WOLF_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
// NOTE: these functions are presented in the "Golden Mask" only
void __cdecl InitialiseWolf(__int16 itemID); // tomb2gold:0x004431C0
void __cdecl WolfControl(__int16 itemID); // tomb2gold:0x004431F0

#endif // WOLF_H_INCLUDED
