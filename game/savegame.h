/*
 * Copyright (c) 2017-2021 Michael Chaban. All rights reserved.
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

#ifndef SAVEGAME_H_INCLUDED
#define SAVEGAME_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
void __cdecl InitialiseStartInfo(); // 0x00439190
void __cdecl ModifyStartInfo(int levelIdx); // 0x00439200
void __cdecl CreateStartInfo(int levelID); // 0x004392E0
void __cdecl CreateSaveGameInfo(); // 0x004394F0
void __cdecl ExtractSaveGameInfo(); // 0x00439A20
void __cdecl ResetSG(); // 0x0043A280
void __cdecl WriteSG(void *ptr, int len); // 0x0043A2A0
void __cdecl ReadSG(void *ptr, int len); // 0x0043A2F0

#endif // SAVEGAME_H_INCLUDED
