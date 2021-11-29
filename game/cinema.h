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

#ifndef CINEMA_H_INCLUDED
#define CINEMA_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
void __cdecl SetCutsceneTrack(int track); // 0x00411F30
int __cdecl StartCinematic(int levelID); // 0x00411F40
void __cdecl InitCinematicRooms(); // 0x00412060
int __cdecl DoCinematic(int nTicks); // 0x00412100
void __cdecl CalculateCinematicCamera(); // 0x00412270
int __cdecl GetCinematicRoom(int x, int y, int z); // 0x004123B0
void __cdecl ControlCinematicPlayer(__int16 itemID); // 0x00412430
void __cdecl LaraControlCinematic(__int16 itemID); // 0x00412510
void __cdecl InitialisePlayer1(__int16 itemID); // 0x004125B0
void __cdecl InitialiseGenPlayer(__int16 itemID); // 0x00412640
void __cdecl InGameCinematicCamera(); // 0x00412680

#endif // CINEMA_H_INCLUDED
