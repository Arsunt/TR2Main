/*
 * Copyright (c) 2017-2018 Michael Chaban. All rights reserved.
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

#ifndef FRONTEND_H_INCLUDED
#define FRONTEND_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
UINT16 __cdecl S_COLOUR(int red, int green, int blue); // 0x0044C2A0
void __cdecl S_DrawScreenLine(int x, int y, int z, int xLen, int yLen, BYTE colorIdx, LPVOID gour, UINT16 flags); // 0x0044C2D0
void __cdecl S_DrawScreenBox(int sx, int sy, int z, int width, int height, BYTE colorIdx, LPVOID gour, UINT16 flags); // 0x0044C310
void __cdecl S_DrawScreenFBox(int sx, int sy, int z, int width, int height, BYTE colorIdx, LPVOID gour, UINT16 flags); // 0x0044C430
void __cdecl S_FinishInventory(); // 0x0044C460
void __cdecl S_FadeToBlack(); // 0x0044C470
void __cdecl S_Wait(int timeout, BOOL inputCheck); // 0x0044C4C0
bool __cdecl S_PlayFMV(LPCTSTR fileName); // 0x0044C520
bool __cdecl S_IntroFMV(LPCTSTR fileName1, LPCTSTR fileName2); // 0x0044C530

#endif // FRONTEND_H_INCLUDED
