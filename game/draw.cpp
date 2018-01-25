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

#include "global/precompiled.h"
#include "game/draw.h"
#include "3dsystem/3d_gen.h"
#include "global/vars.h"

void __cdecl phd_RotYXZsuperpack(UINT16 **pptr, int index) {
	for( int i = 0; i < index; ++i ) {
		if( (**pptr >> 14) == 0 )
			*pptr += 2;
		else
			++*pptr;
	}

	switch( **pptr >> 14 ) {
		case 0 :
			phd_RotYXZpack(((UINT16)(*pptr)[0] << 16) + (UINT16)(*pptr)[1]);
			*pptr += 2;
			break;
		case 1 :
			phd_RotX(**pptr << 6);
			++*pptr;
			break;
		case 2 :
			phd_RotY(**pptr << 6);
			++*pptr;
			break;
		case 3 :
			phd_RotZ(**pptr << 6);
			++*pptr;
			break;
	}
}

/*
 * Inject function
 */
void Inject_Draw() {
//	INJECT(0x00418920, DrawPhaseCinematic);
//	INJECT(0x00418960, DrawPhaseGame);
//	INJECT(0x004189A0, DrawRooms);
//	INJECT(0x00418C50, GetRoomBounds);
//	INJECT(0x00418E20, SetRoomBounds);
//	INJECT(0x004191A0, ClipRoom);
//	INJECT(0x00419580, PrintRooms);
//	INJECT(0x00419640, PrintObjects);
//	INJECT(0x00419870, DrawEffect);
//	INJECT(0x004199C0, DrawSpriteItem);
//	INJECT(----------, DrawDummyItem);
//	INJECT(0x00419A50, DrawAnimatingItem);
//	INJECT(0x00419DD0, DrawLara);
//	INJECT(0x0041AB00, DrawLaraInt);
//	INJECT(0x0041B6F0, InitInterpolate);
//	INJECT(0x0041B730, phd_PopMatrix_I);
//	INJECT(0x0041B760, phd_PushMatrix_I);
//	INJECT(0x0041B790, phd_RotY_I);
//	INJECT(0x0041B7D0, phd_RotX_I);
//	INJECT(0x0041B810, phd_RotZ_I);
//	INJECT(0x0041B850, phd_TranslateRel_I);
//	INJECT(0x0041B8A0, phd_TranslateRel_ID);
//	INJECT(0x0041B8F0, phd_RotYXZ_I);
//	INJECT(0x0041B940, phd_RotYXZsuperpack_I);

	INJECT(0x0041B980, phd_RotYXZsuperpack);

//	INJECT(0x0041BA30, phd_PutPolygons_I);
//	INJECT(0x0041BA60, InterpolateMatrix);
//	INJECT(0x0041BC10, InterpolateArmMatrix);
//	INJECT(0x0041BD10, DrawGunFlash);
//	INJECT(0x0041BE80, CalculateObjectLighting);
//	INJECT(0x0041BF70, GetFrames);
//	INJECT(0x0041C010, GetBoundsAccurate);
//	INJECT(0x0041C090, GetBestFrame);
//	INJECT(0x0041C0D0, AddDynamicLight);
}
