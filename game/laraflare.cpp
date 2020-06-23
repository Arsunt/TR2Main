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

#include "global/precompiled.h"
#include "game/laraflare.h"
#include "3dsystem/3d_gen.h"
#include "3dsystem/scalespr.h"
#include "game/draw.h"
#include "specific/game.h"
#include "specific/output.h"
#include "global/vars.h"

#ifdef FEATURE_VIDEOFX_IMPROVED
extern DWORD AlphaBlendMode;
#endif // FEATURE_VIDEOFX_IMPROVED

void __cdecl DrawFlareInAir(ITEM_INFO *item) {
	int rate;
	__int16 *ptr[2];
	GetFrames(item, ptr, &rate);
	phd_PushMatrix();
	phd_TranslateAbs(item->pos.x, item->pos.y, item->pos.z);
	phd_RotYXZ(item->pos.rotY, item->pos.rotX, item->pos.rotZ);
	int clip = S_GetObjectBounds(ptr[0]);
	if( clip ) {
		CalculateObjectLighting(item, ptr[0]);
		phd_PutPolygons(MeshPtr[Objects[ID_FLARE_ITEM].meshIndex], clip);
		if( CHK_ANY((DWORD)item->data, 0x8000) ) {
			phd_TranslateRel(-6, 6, 80);
			phd_RotX(-90 * PHD_DEGREE);
			phd_RotY(2 * GetRandomDraw());
			S_CalculateStaticLight(0x800);
			phd_PutPolygons(MeshPtr[Objects[ID_FLARE_FIRE].meshIndex], clip);
#ifdef FEATURE_VIDEOFX_IMPROVED
			if( AlphaBlendMode ) {
				int shade = (GetRandomDraw() & 0xFFF) + 0x1000;
				DWORD flags = GLOW_FLARE_COLOR;
				flags |= SPR_BLEND_ADD|SPR_TINT|SPR_SHADE|SPR_SEMITRANS;
				S_DrawSprite(flags, 0, 0, 0, Objects[ID_GLOW].meshIndex, shade, 0);
			}
#endif // FEATURE_VIDEOFX_IMPROVED
		}
	}
	phd_PopMatrix();
}

/*
 * Inject function
 */
void Inject_LaraFlare() {
//	INJECT(0x0042F840, DoFlareLight);
//	INJECT(0x0042F8E0, DoFlareInHand);

	INJECT(0x0042F9C0, DrawFlareInAir);

//	INJECT(0x0042FAC0, CreateFlare);
//	INJECT(0x0042FCA0, set_flare_arm);
//	INJECT(0x0042FCF0, draw_flare);
//	INJECT(0x0042FE60, undraw_flare);
//	INJECT(0x00430090, draw_flare_meshes);
//	INJECT(0x004300B0, undraw_flare_meshes);
//	INJECT(0x004300D0, ready_flare);
//	INJECT(0x00430110, FlareControl);
}
