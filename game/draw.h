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

#ifndef DRAW_H_INCLUDED
#define DRAW_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
int __cdecl DrawPhaseCinematic(); // 0x00418920
int __cdecl DrawPhaseGame(); // 0x00418960

void __cdecl DrawRooms(__int16 currentRoom); // 0x004189A0
void __cdecl GetRoomBounds(); // 0x00418C50
void __cdecl SetRoomBounds(__int16 *ptrObj, int roomNumber, ROOM_INFO *parent); // 0x00418E20
void __cdecl ClipRoom(ROOM_INFO *room); // 0x004191A0
void __cdecl PrintRooms(__int16 roomNumber); // 0x00419580
void __cdecl PrintObjects(__int16 roomNumber); // 0x00419640
void __cdecl DrawEffect(__int16 fx_id); // 0x00419870
void __cdecl DrawSpriteItem(ITEM_INFO *item); // 0x004199C0
void __cdecl DrawDummyItem(ITEM_INFO *item);
void __cdecl DrawAnimatingItem(ITEM_INFO *item); // 0x00419A50

void __cdecl DrawLara(ITEM_INFO *laraitem); // 0x00419DD0

void __cdecl DrawLaraInt(ITEM_INFO *item, __int16 *frame1, __int16 *frame2, int frac, int rate);

void __cdecl InitInterpolate(int frac, int rate); // 0x0041B6F0
void __cdecl phd_PopMatrix_I(); // 0x0041B730
void __cdecl phd_PushMatrix_I(); // 0x0041B760
void __cdecl phd_RotY_I(__int16 angle); // 0x0041B790
void __cdecl phd_RotX_I(__int16 angle); // 0x0041B7D0
void __cdecl phd_RotZ_I(__int16 angle); // 0x0041B810
void __cdecl phd_TranslateRel_I(int x, int y, int z); // 0x0041B850
void __cdecl phd_TranslateRel_ID(int x1, int y1, int z1, int x2, int y2, int z2); // 0x0041B8A0
void __cdecl phd_RotYXZ_I(__int16 y, __int16 x, __int16 z); // 0x0041B8F0
void __cdecl phd_RotYXZsuperpack_I(UINT16 **pptr1, UINT16 **pptr2, int index); // 0x0041B940
void __cdecl phd_RotYXZsuperpack(UINT16 **pptr, int index); // 0x0041B980
void __cdecl phd_PutPolygons_I(__int16 *ptrObj, int clip); // 0x0041BA30

void __cdecl InterpolateMatrix(); // 0x0041BA60
void __cdecl InterpolateArmMatrix(); // 0x0041BC10

void __cdecl DrawGunFlash(int weapon, int clip);

#define CalculateObjectLighting ((void(__cdecl*)(ITEM_INFO*, __int16*)) 0x0041BE80)
int __cdecl GetFrames(ITEM_INFO *item, __int16** frames, int* rate); // 0x0041BF70
#define GetBoundsAccurate ((__int16*(__cdecl*)(ITEM_INFO*)) 0x0041C010)
#define GetBestFrame ((__int16*(__cdecl*)(ITEM_INFO*)) 0x0041C090)

void __cdecl AddDynamicLight(int x, int y, int z, int intensity, int falloff); // 0x0041C0D0

#endif // DRAW_H_INCLUDED
