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
#include "game/lara.h"
#include "global/vars.h"

void __cdecl lara_col_jumper(ITEM_INFO *item, COLL_INFO *coll) {
	coll->badPos = 0x7F00;
	coll->badNeg = -0x0180;
	coll->badCeiling = 0x00C0;

	GetLaraCollisionInfo(item, coll);
	LaraDeflectEdgeJump(item, coll);

	if( item->fallSpeed > 0 && coll->sideMid.floor <= 0 ) {
		item->goalAnimState = LaraLandedBad(item, coll) ? AS_DEATH : AS_STOP;
		item->fallSpeed = 0;
		item->bitFields &= ~8; // clear gravity_status flag
		item->pos.y += coll->sideMid.floor;
	}

	// NOTE: Low ceiling check must be skipped because it produces the bug
	// Core Design removed this check in later game releases
#ifndef FEATURE_JUMP_COLLISION_FIX
	if( ABS(coll->sideMid.ceiling - coll->sideMid.floor) < 0x02FA ) {
		item->currentAnimState = AS_FASTFALL;
		item->goalAnimState = AS_FASTFALL;
		item->animNumber = 32;
		item->frameNumber = Anims[item->animNumber].frameBase + 1;
		item->speed /= 4;
		Lara_MoveAngle += PHD_180;
		if( item->fallSpeed <= 0 )
			item->fallSpeed = 1;
	}
#endif // !FEATURE_JUMPCOLLISION_FIX
}

/*
 * Inject function
 */
void Inject_Lara() {
	INJECT(0x0042A040, lara_col_jumper);
}
