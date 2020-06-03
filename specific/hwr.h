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

#ifndef HWR_H_INCLUDED
#define HWR_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
void __cdecl HWR_InitState(); // 0x0044D0B0
void __cdecl HWR_ResetTexSource(); // 0x0044D1E0
void __cdecl HWR_ResetColorKey(); // 0x0044D210
void __cdecl HWR_ResetZBuffer(); // 0x0044D240
void __cdecl HWR_TexSource(HWR_TEXHANDLE texSource); // 0x0044D2A0
void __cdecl HWR_EnableColorKey(bool state); // 0x0044D2D0
void __cdecl HWR_EnableZBuffer(bool ZWriteEnable, bool ZEnable); // 0x0044D320
void __cdecl HWR_BeginScene(); // 0x0044D3B0
void __cdecl HWR_DrawPolyList(); // 0x0044D3E0
void __cdecl HWR_LoadTexturePages(int pagesCount, LPVOID pagesBuffer, RGB888 *palette); // 0x0044D560
void __cdecl HWR_FreeTexturePages(); // 0x0044D5F0
void __cdecl HWR_GetPageHandles(); // 0x0044D640
bool __cdecl HWR_VertexBufferFull(); // 0x0044D680
bool __cdecl HWR_Init(); // 0x0044D6B0

#endif // HWR_INCLUDED
