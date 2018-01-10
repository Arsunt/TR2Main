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

#ifndef _3DINSERT_H_INCLUDED
#define _3DINSERT_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
BOOL __cdecl visible_zclip(PHD_VBUF *vtx0, PHD_VBUF *vtx1, PHD_VBUF *vtx2); // 0x00405840
int __cdecl ZedClipper(int vtxCount, POINT_INFO *pts, VERTEX_INFO *vtx); // 0x004058B0
int __cdecl XYGUVClipper(int vtxCount, VERTEX_INFO *vtx); // 0x004059F0

#define InsertObjectGT4 ((__int16 *(__cdecl*)(__int16 *ptrObj, int number, int sortType)) 0x00405F10)
#define InsertObjectGT3 ((__int16 *(__cdecl*)(__int16 *ptrObj, int number, int sortType)) 0x00406970)

int __cdecl XYGClipper(int vtxCount, VERTEX_INFO *vtx); // 0x004071F0
__int16 *__cdecl InsertObjectG4(__int16 *ptrObj, int number, int sortType); // 0x00407620
__int16 *__cdecl InsertObjectG3(__int16 *ptrObj, int number, int sortType); // 0x00407A00
int __cdecl XYClipper(int vtxCount, VERTEX_INFO *vtx); // 0x00407D20
void __cdecl InsertTrans8(PHD_VBUF *vbuf, __int16 shade); // 0x00407FF0
void __cdecl InsertTransQuad(int x, int y, int width, int height, int z); // 0x004084A0
void __cdecl InsertFlatRect(int x0, int y0, int x1, int y1, int z, BYTE colorIdx); // 0x00408580
void __cdecl InsertLine(int x0, int y0, int x1, int y1, int z, BYTE colorIdx); // 0x00408650
void __cdecl InsertGT3_ZBuffered(PHD_VBUF *vtx0, PHD_VBUF *vtx1, PHD_VBUF *vtx2, PHD_TEXTURE *texture, PHD_UV *uv0, PHD_UV *uv1, PHD_UV *uv2); // 0x00408710
void __cdecl DrawClippedPoly_Textured(int vtxCount); // 0x00408D60
void __cdecl InsertGT4_ZBuffered(PHD_VBUF *vtx0, PHD_VBUF *vtx1, PHD_VBUF *vtx2, PHD_VBUF *vtx3, PHD_TEXTURE *texture); // 0x00408EA0
__int16 *__cdecl InsertObjectGT4_ZBuffered(__int16 *ptrObj, int number, int sortType); // 0x004092E0
__int16 *__cdecl InsertObjectGT3_ZBuffered(__int16 *ptrObj, int number, int sortType); // 0x00409380
__int16 *__cdecl InsertObjectG4_ZBuffered(__int16 *ptrObj, int number, int sortType); // 0x00409430
void __cdecl DrawPoly_Gouraud(int vtxCount, int red, int green, int blue); // 0x004097D0
__int16 *__cdecl InsertObjectG3_ZBuffered(__int16 *ptrObj, int number, int sortType); // 0x004098D0
void __cdecl InsertFlatRect_ZBuffered(int x0, int y0, int x1, int y1, int z, BYTE colorIdx); // 0x00409BB0
void __cdecl InsertLine_ZBuffered(int x0, int y0, int x1, int y1, int z, BYTE colorIdx); // 0x00409D80

#define InsertGT3_Sorted ((void (__cdecl*)(PHD_VBUF *vtx0, PHD_VBUF *vtx1, PHD_VBUF *vtx2, PHD_TEXTURE *texture, PHD_UV *uv0, PHD_UV *uv1, PHD_UV *uv2, int sortType)) 0x00409EC0)

void __cdecl InsertClippedPoly_Textured(int vtxCount, float z, __int16 polyType, __int16 texPage); // 0x0040A5D0

#define InsertGT4_Sorted ((void (__cdecl*)(PHD_VBUF *vtx0, PHD_VBUF *vtx1, PHD_VBUF *vtx2, PHD_VBUF *vtx3, PHD_TEXTURE *texture, int sortType)) 0x0040A780)

__int16 *__cdecl InsertObjectGT4_Sorted(__int16 *ptrObj, int number, int sortType); // 0x0040AC60
__int16 *__cdecl InsertObjectGT3_Sorted(__int16 *ptrObj, int number, int sortType); // 0x0040ACF0
__int16 *__cdecl InsertObjectG4_Sorted(__int16 *ptrObj, int number, int sortType); // 0x0040AD90
void __cdecl InsertPoly_Gouraud(int vtxCount, float z, int red, int green, int blue, __int16 polyType); // 0x0040B1D0
__int16 *__cdecl InsertObjectG3_Sorted(__int16 *ptrObj, int number, int sortType); // 0x0040B350
void __cdecl InsertSprite_Sorted(int z, int x0, int y0, int x1, int y1, int spriteIdx, __int16 shade); // 0x0040B6A0
void __cdecl InsertFlatRect_Sorted(int x0, int y0, int x1, int y1, int z, BYTE colorIdx); // 0x0040B9F0
void __cdecl InsertLine_Sorted(int x0, int y0, int x1, int y1, int z, BYTE colorIdx); // 0x0040BB70
void __cdecl InsertTrans8_Sorted(PHD_VBUF *vbuf, __int16 shade); // 0x0040BCA0
void __cdecl InsertTransQuad_Sorted(int x, int y, int width, int height, int z); // 0x0040BE40
void __cdecl InsertSprite(int z, int x0, int y0, int x1, int y1, int spriteIdx, __int16 shade); // 0x0040BF80

#endif // _3DINSERT_H_INCLUDED
