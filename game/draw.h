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
#define DrawPhaseCinematic ((int(__cdecl*)(void)) 0x00418920)
#define DrawPhaseGame ((int(__cdecl*)(void)) 0x00418960)

void __cdecl DrawRooms(__int16 currentRoom); // 0x004189A0

#define GetRoomBounds ((void(__cdecl*)(void)) 0x00418C50)
#define SetRoomBounds ((void(__cdecl*)(__int16, int, ROOM_INFO*)) 0x00418E20)
#define ClipRoom ((void(__cdecl*)(ROOM_INFO*)) 0x004191A0)
#define PrintRooms ((void(__cdecl*)(__int16)) 0x00419580)
#define PrintObjects ((void(__cdecl*)(__int16)) 0x00419640)

void __cdecl DrawEffect(__int16 fx_id); // 0x00419870
void __cdecl DrawSpriteItem(ITEM_INFO *item); // 0x004199C0
void __cdecl DrawDummyItem(ITEM_INFO *item);
void __cdecl DrawAnimatingItem(ITEM_INFO *item); // 0x00419A50

#define DrawLara ((void(__cdecl*)(ITEM_INFO*)) 0x00419DD0)
#define DrawLaraInt ((void(__cdecl*)(ITEM_INFO*, __int16*, __int16*, int, int)) 0x0041AB00)
#define InitInterpolate ((void(__cdecl*)(int, int)) 0x0041B6F0)
#define phd_PopMatrix_I ((void(__cdecl*)(void)) 0x0041B730)
#define phd_PushMatrix_I ((void(__cdecl*)(void)) 0x0041B760)
#define phd_RotY_I ((void(__cdecl*)(__int16)) 0x0041B790)
#define phd_RotX_I ((void(__cdecl*)(__int16)) 0x0041B7D0)
#define phd_RotZ_I ((void(__cdecl*)(__int16)) 0x0041B810)
#define phd_TranslateRel_I ((void(__cdecl*)(int, int, int)) 0x0041B850)
#define phd_TranslateRel_ID ((void(__cdecl*)(int, int, int, int, int, int)) 0x0041B8A0)
#define phd_RotYXZ_I ((void(__cdecl*)(int, int, int)) 0x0041B8F0)
#define phd_RotYXZsuperpack_I ((void(__cdecl*)(UINT16**, UINT16**, int)) 0x0041B940)

void __cdecl phd_RotYXZsuperpack(UINT16 **pptr, int index); // 0x0041B980
void __cdecl phd_PutPolygons_I(__int16 *ptrObj, int clip); // 0x0041BA30

#define InterpolateMatrix ((void(__cdecl*)(void)) 0x0041BA60)
#define InterpolateArmMatrix ((void(__cdecl*)(void)) 0x0041BC10)
#define DrawGunFlash ((void(__cdecl*)(int, int)) 0x0041BD10)
#define CalculateObjectLighting ((void(__cdecl*)(ITEM_INFO*, __int16*)) 0x0041BE80)
#define GetFrames ((int(__cdecl*)(ITEM_INFO*, __int16**, int*)) 0x0041BF70)
#define GetBoundsAccurate ((__int16*(__cdecl*)(ITEM_INFO*)) 0x0041C010)
#define GetBestFrame ((__int16*(__cdecl*)(ITEM_INFO*)) 0x0041C090)
#define AddDynamicLight ((void(__cdecl*)(int, int, int, int, int)) 0x0041C0D0)

#endif // DRAW_H_INCLUDED
