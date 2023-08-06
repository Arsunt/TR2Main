/*
 * Copyright (c) 2017-2020 Michael Chaban. All rights reserved.
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

#ifndef ITEMS_H_INCLUDED
#define ITEMS_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
void __cdecl InitialiseItemArray(int itemCount); // 0x00426CD0

#define KillItem ((void(__cdecl*)(__int16)) 0x00426D30)
#define CreateItem ((__int16(__cdecl*)(void)) 0x00426E50)

void __cdecl InitialiseItem(__int16 itemIndex); // 0x00426E90

#define RemoveActiveItem ((void(__cdecl*)(__int16)) 0x00427050)
#define RemoveDrawnItem ((void(__cdecl*)(__int16)) 0x004270E0)

void __cdecl AddActiveItem(__int16 itemIndex); // 0x00427150

#define ItemNewRoom ((void(__cdecl*)(__int16, __int16)) 0x004271B0)

int __cdecl GlobalItemReplace(int oldItemID, int newItemID); // 0x00427250

#define InitialiseFXArray ((void(__cdecl*)(void)) 0x004272D0)
#define CreateEffect ((__int16(__cdecl*)(__int16)) 0x00427300)
#define KillEffect ((void(__cdecl*)(__int16)) 0x00427370)
#define EffectNewRoom ((void(__cdecl*)(__int16, __int16)) 0x00427460)
#define ClearBodyBag ((void(__cdecl*)(void)) 0x00427500)

#endif // ITEMS_H_INCLUDED
