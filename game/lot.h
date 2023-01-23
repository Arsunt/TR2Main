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

#ifndef LOT_H_INCLUDED
#define LOT_H_INCLUDED

#include "global/types.h"

#define MAXIMUM_CREATURE_SLOTS 16

/*
 * Function list
 */

void __cdecl InitialiseLOTarray(); // 0x00432B10
void __cdecl DisableBaddieAI(__int16 itemNum); // 0x00432B70
BOOL __cdecl EnableBaddieAI(__int16 itemNum, BOOL always); // 0x00432BC0
void __cdecl InitialiseSlot(__int16 itemNum, int creatureNum); // 0x00432D70
#define CreateZone ((void(__cdecl*)(ITEM_INFO*)) 0x00432F80)
#define ClearLOT ((void(__cdecl*)(LOT_INFO*)) 0x00433040)

#endif // LOT_H_INCLUDED
