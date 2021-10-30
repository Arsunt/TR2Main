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

#ifndef LARAMISC_H_INCLUDED
#define LARAMISC_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
void __cdecl LaraControl(__int16 itemID);

#define AnimateLara ((void(__cdecl*)(ITEM_INFO*)) 0x00430A10)

void __cdecl UseItem(__int16 itemID); // 0x00430D10
void __cdecl LaraCheatGetStuff(); // 0x00430ED0
void __cdecl ControlLaraExtra(__int16 itemID); // 0x00430F90
void __cdecl InitialiseLaraLoad(__int16 itemID); // 0x00430FB0

#define InitialiseLara ((void(__cdecl*)(int)) 0x00430FE0)

void __cdecl InitialiseLaraInventory(int levelID); // 0x004312A0

#define LaraInitialiseMeshes ((void(__cdecl*)(int)) 0x00431610)

#endif // LARAMISC_H_INCLUDED
