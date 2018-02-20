/*
 * Copyright (c) 2017-2018 Michael Chaban. All rights reserved.
 * CD Audio solution in this file was designed by PaulD.
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

#ifndef CD_PAULD_H_INCLUDED
#define CD_PAULD_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
bool __cdecl PaulD_CD_Init();
void __cdecl PaulD_CD_Cleanup();
void __cdecl PaulD_CDLoop();
void __cdecl PaulD_CDPlay(__int16 trackID, BOOL isLooped);
void __cdecl PaulD_CDStop();
BOOL __cdecl PaulD_StartSyncedAudio(int trackID);
DWORD __cdecl PaulD_CDGetLoc();
void __cdecl PaulD_CDVolume(DWORD volume);

#endif // CD_PAULD_H_INCLUDED
