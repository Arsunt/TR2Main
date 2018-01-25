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

#ifndef DRAW_H_INCLUDED
#define DRAW_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
// 0x00418920:		DrawPhaseCinematic

#define DrawPhaseGame ((int(__cdecl*)(void)) 0x00418960)

// 0x004189A0:		DrawRooms
// 0x00418C50:		GetRoomBounds
// 0x00418E20:		SetRoomBounds
// 0x004191A0:		ClipRoom
// 0x00419580:		PrintRooms
// 0x00419640:		PrintObjects
// 0x00419870:		DrawEffect
// 0x004199C0:		DrawSpriteItem
// ----------:		DrawDummyItem

#define DrawAnimatingItem ((void(__cdecl*)(ITEM_INFO*)) 0x00419A50)

// 0x00419DD0:		DrawLara
// 0x0041AB00:		DrawLaraInt
// 0x0041B6F0:		InitInterpolate
// 0x0041B730:		phd_PopMatrix_I
// 0x0041B760:		phd_PushMatrix_I
// 0x0041B790:		phd_RotY_I
// 0x0041B7D0:		phd_RotX_I
// 0x0041B810:		phd_RotZ_I
// 0x0041B850:		phd_TranslateRel_I
// 0x0041B8A0:		phd_TranslateRel_ID
// 0x0041B8F0:		phd_RotYXZ_I
// 0x0041B940:		phd_RotYXZsuperpack_I

void __cdecl phd_RotYXZsuperpack(UINT16 **pptr, int index); // 0x0041B980

// 0x0041BA30:		phd_PutPolygons_I
// 0x0041BA60:		InterpolateMatrix
// 0x0041BC10:		InterpolateArmMatrix
// 0x0041BD10:		DrawGunFlash
// 0x0041BE80:		CalculateObjectLighting
// 0x0041BF70:		GetFrames
// 0x0041C010:		GetBoundsAccurate
// 0x0041C090:		GetBestFrame
// 0x0041C0D0:		AddDynamicLight

#endif // DRAW_H_INCLUDED
