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

#ifndef MOVE_BLOCK_H_INCLUDED
#define MOVE_BLOCK_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
//	0x004339A0:		InitialiseMovingBlock

#define MovableBlock ((void(__cdecl*)(__int16)) 0x004339D0)

//	0x00433B20:		MovableBlockCollision
//	0x00433D80:		TestBlockMovable
//	0x00433DD0:		TestBlockPush
//	0x00433F20:		TestBlockPull

#define AlterFloorHeight ((void(__cdecl*)(ITEM_INFO *, int)) 0x00434160)

//	0x00434220:		DrawMovableBlock

#define DrawUnclippedItem ((void(__cdecl*)(ITEM_INFO *)) 0x00434250)

#endif // MOVE_BLOCK_H_INCLUDED
