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

#ifndef SMAIN_H_INCLUDED
#define SMAIN_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
BOOL __cdecl GameMain(); // 0x00454B10
__int16 __cdecl TitleSequence(); // 0x00454DE0
void __cdecl CheckCheatMode(); // 0x00454EF0
void __cdecl S_SaveSettings(); // 0x00455250
void __cdecl S_LoadSettings(); // 0x004552D0

#endif // SMAIN_H_INCLUDED
