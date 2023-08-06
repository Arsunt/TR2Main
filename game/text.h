/*
 * Copyright (c) 2017-2018 Michael Chaban. All rights reserved.
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

#ifndef TEXT_H_INCLUDED
#define TEXT_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
void __cdecl T_InitPrint(); // 0x00440500
TEXT_STR_INFO *__cdecl T_Print(int x, int y, __int16 z, const char *str); // 0x00440530
void __cdecl T_ChangeText(TEXT_STR_INFO *textInfo, const char *newString); // 0x00440640
void __cdecl T_SetScale(TEXT_STR_INFO *textInfo, int scaleH, int scaleV); // 0x00440680
void __cdecl T_FlashText(TEXT_STR_INFO *textInfo, __int16 state, __int16 rate); // 0x004406A0
void __cdecl T_AddBackground(TEXT_STR_INFO *textInfo, __int16 xSize, __int16 ySize, __int16 xOff, __int16 yOff, __int16 zOff, INV_COLOURS invColour, GOURAUD_FILL *gour, UINT16 flags); // 0x004406D0
void __cdecl T_RemoveBackground(TEXT_STR_INFO *textInfo); // 0x00440760
void __cdecl T_AddOutline(TEXT_STR_INFO *textInfo, BOOL state, INV_COLOURS invColour, GOURAUD_OUTLINE *gour, UINT16 flags); // 0x00440770
void __cdecl T_RemoveOutline(TEXT_STR_INFO *textInfo); // 0x004407A0
void __cdecl T_CentreH(TEXT_STR_INFO *textInfo, UINT16 state); // 0x004407B0
void __cdecl T_CentreV(TEXT_STR_INFO *textInfo, UINT16 state); // 0x004407D0
void __cdecl T_RightAlign(TEXT_STR_INFO *textInfo, bool state); // 0x004407F0
void __cdecl T_BottomAlign(TEXT_STR_INFO *textInfo, bool state); // 0x00440810
DWORD __cdecl T_GetTextWidth(TEXT_STR_INFO *textInfo); // 0x00440830
BOOL __cdecl T_RemovePrint(TEXT_STR_INFO *textInfo); // 0x00440940
__int16 __cdecl T_GetStringLen(const char *str); // 0x00440970
void __cdecl T_DrawText(); // 0x004409A0
void __cdecl T_DrawTextBox(int sx, int sy, int z, int width, int height); // 0x004409D0
void __cdecl T_DrawThisText(TEXT_STR_INFO *textInfo); // 0x00440B60
DWORD __cdecl GetTextScaleH(DWORD baseScale); // 0x00440F40
DWORD __cdecl GetTextScaleV(DWORD baseScale); // 0x00440F80

#ifdef FEATURE_HUD_IMPROVED
void T_HideText(TEXT_STR_INFO *textInfo, __int16 state);
#endif // FEATURE_HUD_IMPROVED

#endif // TEXT_H_INCLUDED
