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

#include "global/precompiled.h"
#include "3dsystem/scalespr.h"
#include "specific/output.h"
#include "global/vars.h"

void __cdecl S_DrawPickup(int sx, int sy, int scale, __int16 spriteIdx, __int16 shade) {
	int x1, x2, y1, y2;

#ifdef FEATURE_SPRITE_SCALE_FIX
	scale = GetRenderScale(scale);
#endif

	x1 = sx + (PhdSpriteInfo[spriteIdx].x1 * scale / PHD_ONE);
	x2 = sx + (PhdSpriteInfo[spriteIdx].x2 * scale / PHD_ONE);
	y1 = sy + (PhdSpriteInfo[spriteIdx].y1 * scale / PHD_ONE);
	y2 = sy + (PhdSpriteInfo[spriteIdx].y2 * scale / PHD_ONE);

	if( x2 >= 0 && y2 >= 0 && x1 < PhdWinWidth && y1 < PhdWinHeight )
		ins_sprite(200, x1, y1, x2, y2, spriteIdx, shade);
}

/*
 * Inject function
 */
void Inject_ScaleSpr() {
	INJECT(0x0040C300, S_DrawPickup);
}
