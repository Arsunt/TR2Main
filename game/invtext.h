/*
 * Copyright (c) 2017-2019 Michael Chaban. All rights reserved.
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

#ifndef INVTEXT_H_INCLUDED
#define INVTEXT_H_INCLUDED

#include "global/types.h"

// Requester flags
#define REQFLAG_NOCURSOR	(1)

// Requester item flags
#define REQFLAG_ACTIVE		(1)
#define REQFLAG_LEFT		(2)
#define REQFLAG_RIGHT		(4)

/*
 * Function list
 */
void __cdecl Init_Requester(REQUEST_INFO *req); // 0x00425580
void __cdecl Remove_Requester(REQUEST_INFO *req); // 0x00425610
void __cdecl ReqItemCentreAlign(REQUEST_INFO *req, TEXT_STR_INFO *textInfo); // 0x004256C0
void __cdecl ReqItemLeftAlign(REQUEST_INFO *req, TEXT_STR_INFO *textInfo); // 0x004256E0
void __cdecl ReqItemRightAlign(REQUEST_INFO *req, TEXT_STR_INFO *textInfo); // 0x00425740
int __cdecl Display_Requester(REQUEST_INFO *req, BOOL removeOnDeselect, BOOL isBackground); // 0x004257A0
void __cdecl SetRequesterHeading(REQUEST_INFO *req, const char *string1, DWORD flags1, const char *string2, DWORD flags2); // 0x00426010
void __cdecl RemoveAllReqItems(REQUEST_INFO *req); // 0x004260C0
void __cdecl ChangeRequesterItem(REQUEST_INFO *req, DWORD itemIdx, const char *string1, DWORD flags1, const char *string2, DWORD flags2); // 0x004260E0
void __cdecl AddRequesterItem(REQUEST_INFO *req, const char *string1, DWORD flags1, const char *string2, DWORD flags2); // 0x004261A0
void __cdecl SetPCRequesterSize(REQUEST_INFO *req, int maxLines, __int16 yPos); // 0x00426250
BOOL __cdecl AddAssaultTime(DWORD newTime); // 0x00426290
void __cdecl ShowGymStatsText(); // 0x00426320
void __cdecl ShowStatsText(char *timeString, BOOL removeOnDeselect); // 0x00426500
void __cdecl ShowEndStatsText(); // 0x004268A0

#endif // INVTEXT_H_INCLUDED
