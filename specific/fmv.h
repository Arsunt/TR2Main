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

#ifndef FMV_H_INCLUDED
#define FMV_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
bool __cdecl PlayFMV(LPCTSTR fileName); // 0x0044BE50
void __cdecl WinPlayFMV(LPCSTR fileName, bool isPlayback); // 0x0044BED0
void __cdecl WinStopFMV(bool isPlayback); // 0x0044C1B0
bool __cdecl IntroFMV(LPCTSTR fileName1, LPCTSTR fileName2); // 0x0044C200

#endif // FMV_H_INCLUDED
