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

#ifndef SPHERE_H_INCLUDED
#define SPHERE_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
#define TestCollision ((int(__cdecl*)(ITEM_INFO*, ITEM_INFO*)) 0x0043FA60)
#define GetSpheres ((int(__cdecl*)(ITEM_INFO*, SPHERE_INFO*, BOOL)) 0x0043FB90)
#define GetJointAbsPosition ((void(__cdecl*)(ITEM_INFO*, PHD_VECTOR*, int)) 0x0043FE70)
#define BaddieBiteEffect ((void(__cdecl*)(ITEM_INFO*,BITE_INFO*)) 0x00440010)

#endif // SPHERE_H_INCLUDED
