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

#ifndef SOUND_H_INCLUDED
#define SOUND_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
int __cdecl GetRealTrack(int trackID); // 0x0043F430
#define PlaySoundEffect ((void(__cdecl*)(DWORD, PHD_3DPOS *, DWORD)) 0x0043F470)
#define StopSoundEffect ((void(__cdecl*)(int)) 0x0043F910)
#define SOUND_EndScene ((void(__cdecl*)(void)) 0x0043F970)
#define SOUND_Stop ((void(__cdecl*)(void)) 0x0043FA00)
void __cdecl SOUND_Init(); // 0x0043FA30

#endif // SOUND_H_INCLUDED
