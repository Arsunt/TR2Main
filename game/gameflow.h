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

#ifndef GAMEFLOW_H_INCLUDED
#define GAMEFLOW_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
BOOL __cdecl GF_LoadScriptFile(LPCTSTR fileName); // 0x0041FA40
BOOL __cdecl GF_DoFrontEndSequence(); // 0x0041FC30
int __cdecl GF_DoLevelSequence(DWORD levelID, GF_LEVEL_TYPE levelType); // 0x0041FC50
int __cdecl GF_InterpretSequence(__int16 *seq, GF_LEVEL_TYPE levelType, int seqType); // 0x0041FCC0
void __cdecl GF_ModifyInventory(int levelID, BOOL isSecret); // 0x004201A0
int __cdecl GF_CurrentEvent(); // NOTE: not presented in the original game

#endif // GAMEFLOW_H_INCLUDED
