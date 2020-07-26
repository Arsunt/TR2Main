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
#include "game/laraswim.h"
#include "global/vars.h"

void __cdecl SwimTurn(ITEM_INFO *item) {
	if( CHK_ANY(InputStatus, IN_FORWARD) ) {
		item->pos.rotX -= 2*PHD_DEGREE;
	} else if( CHK_ANY(InputStatus, IN_BACK) ) {
		item->pos.rotX += 2*PHD_DEGREE;
	}

	if( CHK_ANY(InputStatus, IN_LEFT) ) {
		Lara.turn_rate -= PHD_DEGREE*9/4;
		CLAMPL(Lara.turn_rate, -6*PHD_DEGREE);
		item->pos.rotZ -= 3*PHD_DEGREE;
	} else if( CHK_ANY(InputStatus, IN_RIGHT) ) {
		Lara.turn_rate += PHD_DEGREE*9/4;
		CLAMPG(Lara.turn_rate, 6*PHD_DEGREE);
		item->pos.rotZ += 3*PHD_DEGREE;
	}
}

void __cdecl lara_as_swim(ITEM_INFO *item, COLL_INFO *coll) {
	if( item->hitPoints <= 0 ) {
		item->goalAnimState = AS_UWDEATH;
		return;
	}

	if( CHK_ANY(InputStatus, IN_ROLL) ) {
		item->currentAnimState = AS_WATERROLL;
		item->animNumber = 203;
		item->frameNumber = Anims[item->animNumber].frameBase;
	} else {
		SwimTurn(item);
		item->fallSpeed += 8;
		CLAMPG(item->fallSpeed, 200);
		if( !CHK_ANY(InputStatus, IN_JUMP) ) {
			item->goalAnimState = AS_GLIDE;
		}
	}
}

/*
 * Inject function
 */
void Inject_LaraSwim() {
//	INJECT(0x00432000, LaraUnderWater);

	INJECT(0x00432230, SwimTurn);
	INJECT(0x004322C0, lara_as_swim);

//	INJECT(0x00432330, lara_as_glide);
//	INJECT(0x004323B0, lara_as_tread);
//	INJECT(0x00432440, lara_as_dive);
//	INJECT(0x00432460, lara_as_uwdeath);
//	INJECT(0x004324C0, lara_as_waterroll);
//	INJECT(0x004324D0, lara_col_swim);
//	INJECT(----------, lara_col_glide);
//	INJECT(----------, lara_col_tread);
//	INJECT(----------, lara_col_dive);
//	INJECT(0x004324F0, lara_col_uwdeath);
//	INJECT(----------, lara_col_waterroll);
//	INJECT(0x00432550, GetWaterDepth);
//	INJECT(0x004326F0, LaraTestWaterDepth);
//	INJECT(0x004327C0, LaraSwimCollision);
//	INJECT(0x00432920, LaraWaterCurrent);
}
