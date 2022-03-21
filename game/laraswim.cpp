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
#include "3dsystem/phd_math.h"
#include "game/control.h"
#include "game/collide.h"
#include "game/items.h"
#include "game/lara.h"
#include "game/larafire.h"
#include "game/laramisc.h"
#include "game/objects.h"
#include "global/vars.h"

#ifdef FEATURE_CHEAT
static int OpenDoorsCheatCooldown = 0;
// NOTE: this come from Tomb1Main (Door Cheat)
void DoorOpenNearest(ITEM_INFO *lara_item) {
	int max_dist = SQR((1024 * 2) >> 8);
	for (int itemID = 0; itemID < LevelItemCount; itemID++) {
		ITEM_INFO *item = &Items[itemID];
		if ((item->objectID < ID_DOOR_TYPE1
		||   item->objectID > ID_DOOR_TYPE8)
		&&   item->objectID != ID_TRAPDOOR_TYPE1
		&&   item->objectID != ID_TRAPDOOR_TYPE2
		&&   item->objectID != ID_TRAPDOOR_TYPE3) {
			continue;
		}
		int dx = (item->pos.x - lara_item->pos.x) >> 8;
		int dy = (item->pos.y - lara_item->pos.y) >> 8;
		int dz = (item->pos.z - lara_item->pos.z) >> 8;
		int dist = SQR(dx) + SQR(dy) + SQR(dz);
		if (dist > max_dist) {
			continue;
		}
		if (!item->active) {
			AddActiveItem(itemID);
			item->flags |= IFL_CODEBITS;
		} else if CHK_ANY(item->flags, IFL_CODEBITS) {
			item->flags &= ~IFL_CODEBITS;
		} else {
			item->flags |= IFL_CODEBITS;
		}
		item->timer = 0;
		item->touchBits = 0;
	}
}
#endif

void __cdecl LaraUnderWater(ITEM_INFO *item, COLL_INFO *coll) {
	int s, s2, c;

	coll->badPos = -NO_HEIGHT;
	coll->badNeg = -400;
	coll->badCeiling = 400;
	coll->old.x = item->pos.x;
	coll->old.y = item->pos.y;
	coll->old.z = item->pos.z;
	coll->radius = 300;
	coll->trigger = 0;
	coll->flags &= ~0x17;
	coll->flags |= 0x18;

	if (CHK_ANY(InputStatus, IN_LOOK) && Lara.look) {
		LookLeftRight();
	} else {
		ResetLook();
	}

	Lara.look = TRUE;
	if (Lara.extra_anim) {
		ExtraFunctions[item->currentAnimState](item);
	} else {
		LaraControlFunctions[item->currentAnimState](item);
	}

	if (item->pos.rotZ < -364 || item->pos.rotZ > 364) {
		if (item->pos.rotZ >= 0) {
			item->pos.rotZ -= 364;
		} else {
			item->pos.rotZ += 364;
		}
	} else {
		item->pos.rotZ = 0;
	}

	CLAMP(item->pos.rotX, -15470, 15470)
	CLAMP(item->pos.rotZ, -4004, 4004)

	if (Lara.turn_rate < -364 || Lara.turn_rate > 364) {
		if (Lara.turn_rate >= 0) {
			Lara.turn_rate -= 364;
		} else {
			Lara.turn_rate += 364;
		}
	} else {
		Lara.turn_rate = 0;
	}

	item->pos.rotY += Lara.turn_rate;
	if (Lara.current_active && Lara.water_status != LWS_Cheat) {
		LaraWaterCurrent(coll);
	}
	AnimateLara(item);

	s = (item->fallSpeed * phd_sin(item->pos.rotY)) >> 16;
	s2 = (item->fallSpeed * phd_sin(item->pos.rotX)) >> 16;
	c = (item->fallSpeed * phd_cos(item->pos.rotY)) >> 16;
	item->pos.x += (phd_cos(item->pos.rotX) * s) >> W2V_SHIFT;
	item->pos.y -= s2;
	item->pos.z += (phd_cos(item->pos.rotX) * c) >> W2V_SHIFT;

	if (Lara.extra_anim == 0) {
		if (Lara.water_status != LWS_Cheat) {
			LaraBaddieCollision(item, coll);
		}
		LaraCollisionFunctions[item->currentAnimState](item, coll);
	}

#ifdef FEATURE_CHEAT
	// NOTE: this come from Tomb1Main (Door Cheat)
	// Press Draw key to open any near door !
	if (Lara.water_status == LWS_Cheat) {
		if (OpenDoorsCheatCooldown) {
			OpenDoorsCheatCooldown--;
		} else if CHK_ANY(InputStatus, IN_DRAW) {
			OpenDoorsCheatCooldown = FRAMES_PER_SECOND;
			DoorOpenNearest(LaraItem);
		}
	}
#endif // FEATURE_CHEAT

	UpdateLaraRoom(item, 0);
	LaraGun();
	TestTriggers(coll->trigger, FALSE);
}

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
#ifdef FEATURE_CHEAT
		if( Lara.water_status == LWS_Cheat ) {
			CLAMPG(item->fallSpeed, 400);
		} else {
			CLAMPG(item->fallSpeed, 200);
		}
#else // FEATURE_CHEAT
		CLAMPG(item->fallSpeed, 200);
#endif // FEATURE_CHEAT
		if( !CHK_ANY(InputStatus, IN_JUMP) ) {
			item->goalAnimState = AS_GLIDE;
		}
	}
}

/*
 * Inject function
 */
void Inject_LaraSwim() {
	INJECT(0x00432000, LaraUnderWater);

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
