/*
 * Copyright (c) 2017-2020 Michael Chaban. All rights reserved.
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
void __cdecl LaraControl(__int16 itemID);
void __cdecl AnimateLara(ITEM_INFO* item); // 0x00430A10
void __cdecl UseItem(__int16 itemID); // 0x00430D10
void __cdecl LaraCheatGetStuff(); // 0x00430ED0
void __cdecl ControlLaraExtra(__int16 itemID); // 0x00430F90
void __cdecl InitialiseLaraLoad(__int16 itemID); // 0x00430FB0
void __cdecl InitialiseLara(GF_LEVEL_TYPE type); // 0x00430FE0
void __cdecl InitialiseLaraInventory(int levelID); // 0x004312A0
void __cdecl LaraInitialiseMeshes(int levelID); // 0x00431610

#endif // LARAMISC_H_INCLUDED
