/*
 * Copyright (c) 2017-2020 Michael Chaban. All rights reserved.
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

#ifndef PSX_BAR_H_INCLUDED
#define PSX_BAR_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */

void __cdecl PSX_DrawHealthBar(int x0, int y0, int x1, int y1, int bar, int pixel, int alpha);
void __cdecl PSX_DrawAirBar(int x0, int y0, int x1, int y1, int bar, int pixel, int alpha);
void __cdecl PSX_InsertHealthBar(int x0, int y0, int x1, int y1, int bar, int pixel, int alpha);
void __cdecl PSX_InsertAirBar(int x0, int y0, int x1, int y1, int bar, int pixel, int alpha);

#endif // PSX_BAR_H_INCLUDED
