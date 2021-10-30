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

#ifndef INVENTORY_H_INCLUDED
#define INVENTORY_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
int __cdecl Display_Inventory(INVENTORY_MODE invMode); // 0x00422060
void __cdecl Construct_Inventory(); // 0x004232F0
void __cdecl SelectMeshes(INVENTORY_ITEM *invItem); // 0x00423450
int __cdecl AnimateInventoryItem(INVENTORY_ITEM *invItem); // 0x004234E0
void __cdecl DrawInventoryItem(INVENTORY_ITEM *invItem); // 0x00423570
DWORD __cdecl GetDebouncedInput(DWORD input); // 0x004239A0
void __cdecl DoInventoryPicture(); //0x004239C0
void __cdecl DoInventoryBackground(); // 0x004239D0

#endif // INVENTORY_H_INCLUDED
