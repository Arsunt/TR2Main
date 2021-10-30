/*
 * Copyright (c) 2017-2020 Michael Chaban. All rights reserved.
 * Original game is created by Core Design Ltd. in 1997.
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
#include "game/box.h"
#include "game/control.h"
#include "game/items.h"
#include "game/lot.h"
#include "game/missile.h"
#include "global/vars.h"

void __cdecl CreatureDie(__int16 itemID, BOOL explode) {
	ITEM_INFO *item = &Items[itemID];
	item->collidable = 0;
	item->hitPoints = HP_DONT_TARGET;

	if( explode ) {
		ExplodingDeath(itemID, ~0, 0);
		KillItem(itemID);
	} else {
		RemoveActiveItem(itemID);
	}

	DisableBaddieAI(itemID);
	item->flags |= IFL_INVISIBLE;
	if( item->clear_body ) {
		item->nextActive = PrevItemActive;
		PrevItemActive = itemID;
	}

	ITEM_INFO *pickup = NULL;
	for( int i = item->carriedItem; i != -1; i = pickup->carriedItem ) {
		pickup = &Items[i];
		pickup->pos.x = item->pos.x;
		pickup->pos.y = item->pos.y;
		pickup->pos.z = item->pos.z;
		ItemNewRoom(i, item->roomNumber);
	}
}

void __cdecl CreatureKill(ITEM_INFO *item, int killAnim, int killState, int laraKillState) {
#ifdef FEATURE_CHEAT
	// Return Lara to normal state if Dozy cheat enabled
	if( Lara.water_status == LWS_Cheat ) {
		Lara.water_status = LWS_AboveWater;
		Lara.mesh_effects = 0;
	}
#endif // FEATURE_CHEAT
	item->animNumber = killAnim + Objects[item->objectID].animIndex;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->currentAnimState = killState;
	LaraItem->animNumber = Objects[ID_LARA_EXTRA].animIndex;
	LaraItem->frameNumber = Anims[LaraItem->animNumber].frameBase;
	LaraItem->currentAnimState = 0;
	LaraItem->goalAnimState = laraKillState;
	LaraItem->pos.x = item->pos.x;
	LaraItem->pos.y = item->pos.y;
	LaraItem->pos.z = item->pos.z;
	LaraItem->pos.rotY = item->pos.rotY;
	LaraItem->pos.rotX = item->pos.rotX;
	LaraItem->pos.rotZ = item->pos.rotZ;
	LaraItem->fallSpeed = 0;
	LaraItem->gravity = 0;
	LaraItem->speed = 0;
	if( LaraItem->roomNumber != item->roomNumber ) {
		ItemNewRoom(Lara.item_number, item->roomNumber);
	}
	AnimateItem(LaraItem);
	LaraItem->goalAnimState = laraKillState;
	LaraItem->currentAnimState = laraKillState;
	Lara.extra_anim = 1;
	Lara.hit_direction = -1;
	Lara.air = -1;
	Lara.gun_status = LGS_HandBusy;
	Lara.gun_type = LGT_Unarmed;
	Camera.pos.roomNumber = LaraItem->roomNumber;
}

/*
 * Inject function
 */
void Inject_Box() {
//	INJECT(0x0040E190, InitialiseCreature);
//	INJECT(0x0040E1C0, CreatureActive);
//	INJECT(0x0040E210, CreatureAIInfo);
//	INJECT(0x0040E470, SearchLOT);
//	INJECT(0x0040E670, UpdateLOT);
//	INJECT(0x0040E6E0, TargetBox);
//	INJECT(0x0040E780, StalkBox);
//	INJECT(0x0040E880, EscapeBox);
//	INJECT(0x0040E930, ValidBox);
//	INJECT(0x0040E9E0, CreatureMood);
//	INJECT(0x0040EE50, CalculateTarget);
//	INJECT(0x0040F2B0, CreatureCreature);
//	INJECT(0x0040F3B0, BadFloor);

	INJECT(0x0040F440, CreatureDie);

//	INJECT(0x0040F500, CreatureAnimation);
//	INJECT(0x0040FDD0, CreatureTurn);
//	INJECT(0x0040FEB0, CreatureTilt);
//	INJECT(0x0040FEF0, CreatureHead);
//	INJECT(0x0040FF40, CreatureNeck);
//	INJECT(0x0040FF90, CreatureFloat);
//	INJECT(0x00410040, CreatureUnderwater);
//	INJECT(0x00410090, CreatureEffect);
//	INJECT(0x004100F0, CreatureVault);

	INJECT(0x00410230, CreatureKill);

//	INJECT(0x004103A0, GetBaddieTarget);
}
