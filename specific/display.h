/*
 * Copyright (c) 2017 Michael Chaban. All rights reserved.
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

#ifndef DISPLAY_H_INCLUDED
#define DISPLAY_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
void __cdecl IncreaseScreenSize(); // 0x004478C0
void __cdecl DecreaseScreenSize(); // 0x00447930
void __cdecl setup_screen_size(); // 0x004479A0
void __cdecl TempVideoAdjust(int hires, double sizer); // 0x00447A40
void __cdecl TempVideoRemove(); // 0x00447A80
void __cdecl S_FadeInInventory(BOOL isFade); // 0x00447AC0
void __cdecl S_FadeOutInventory(BOOL isFade); // 0x00447B00

#endif // DISPLAY_H_INCLUDED
