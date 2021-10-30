/*
 * Copyright (c) 2017-2018 Michael Chaban. All rights reserved.
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

#ifndef PHD_MATH_H_INCLUDED
#define PHD_MATH_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
int __fastcall phd_atan(int x, int y); // 0x00457EA0
int __fastcall phd_cos(__int16 angle); // 0x00457EE8
int __fastcall phd_sin(__int16 angle); // 0x00457EEE
DWORD __fastcall phd_sqrt(DWORD n); // 0x00457F23

#endif // PHD_MATH_H_INCLUDED
