/*
 * Copyright (c) 2017-2018 Michael Chaban. All rights reserved.
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

#ifndef INVTEXT_H_INCLUDED
#define INVTEXT_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
void __cdecl Init_Requester(REQUEST_INFO *req); // 0x00425580

// 0x00425610:		Remove_Requester
// 0x004256C0:		ResetTextParams
// 0x004256E0:		GetTextParams1
// 0x00425740:		GetTextParams2
// 0x004257A0:		Display_Requester

void __cdecl SetRequesterHeading(REQUEST_INFO *req, const char *string1, DWORD flags1, const char *string2, DWORD flags2); // 0x00426010
void __cdecl RemoveAllReqItems(REQUEST_INFO *req); // 0x004260C0
void __cdecl ChangeRequesterItem(REQUEST_INFO *req, DWORD itemIdx, const char *string1, DWORD flags1, const char *string2, DWORD flags2); // 0x004260E0
void __cdecl AddRequesterItem(REQUEST_INFO *req, const char *string1, DWORD flags1, const char *string2, DWORD flags2); // 0x004261A0
void __cdecl SetPCRequesterSize(REQUEST_INFO *req, int maxLines, __int16 yPos); // 0x00426250

// 0x00426290:		AddAssaultTime
// 0x00426320:		ShowGymStatsText

#define ShowStatsText ((void(__cdecl*)(char *,BOOL)) 0x00426500)
#define ShowEndStatsText ((void(__cdecl*)(void)) 0x004268A0)

#endif // INVTEXT_H_INCLUDED
