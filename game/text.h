/*
 * Copyright (c) 2017 Michael Chaban. All rights reserved.
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

#ifndef TEXT_H_INCLUDED
#define TEXT_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
void __cdecl T_InitPrint(); // 0x00440500
TEXT_STR_INFO *__cdecl T_Print(int x, int y, __int16 z, const char *str); // 0x00440530
void __cdecl T_ChangeText(TEXT_STR_INFO *textInfo, char *newString); // 0x00440640

// 0x00440680:		T_SetScale
// 0x004406A0:		T_FlashText
// 0x004406D0:		T_AddBackground
// 0x00440760:		T_RemoveBackground
// 0x00440770:		T_AddOutline
// 0x004407A0:		T_RemoveOutline
// 0x004407B0:		T_CentreH
// 0x004407D0:		T_CentreV

void __cdecl T_RightAlign(TEXT_STR_INFO *textInfo, bool state); // 0x004407F0
void __cdecl T_BottomAlign(TEXT_STR_INFO *textInfo, bool state); // 0x00440810

// 0x00440830:		T_GetTextWidth

BOOL __cdecl T_RemovePrint(TEXT_STR_INFO *textInfo); // 0x00440940
__int16 __cdecl T_GetStringLen(const char *str); // 0x00440970
void __cdecl T_DrawText(); // 0x004409A0

// 0x004409D0:		T_DrawTextBox

#define T_DrawThisText ((void(__cdecl*)(TEXT_STR_INFO *)) 0x00440B60)

DWORD __cdecl GetTextScaleH(DWORD baseScale); // 0x00440F40
DWORD __cdecl GetTextScaleV(DWORD baseScale); // 0x00440F80

#endif // TEXT_H_INCLUDED
