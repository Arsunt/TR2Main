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
#include "game/box.h"
#include "3dsystem/phd_math.h"
#include "game/control.h"
#include "game/items.h"
#include "game/lot.h"
#include "game/missile.h"
#include "specific/game.h"
#include "global/vars.h"

/*void __cdecl InitialiseCreature(__int16 itemID) {
	ITEM_INFO *item = &Items[itemID];
	item->pos.rotY += (GetRandomControl() - PHD_90) >> 1;
	item->collidable = 1;
	item->data = NULL;
}

BOOL __cdecl CreatureActive(__int16 itemID) {
	ITEM_INFO *item = &Items[itemID];

	if (CHK_ANY(item->status, ITEM_INVISIBLE)) {
		BOOL isActive = EnableBaddieAI(itemID, FALSE);
		if (isActive) {
			item->status = ITEM_ACTIVE;
			return TRUE;
		}
	}

	return CHK_ANY(item->status, ITEM_ACTIVE);
}

void __cdecl CreatureAIInfo(ITEM_INFO *item, AI_INFO *ai) {
	CREATURE_INFO *creature = (CREATURE_INFO*)item->data;
	if (creature == NULL) {
		return;
	}

	ITEM_INFO *enemy = NULL;
	__int16 *zone = NULL;

	switch (item->objectID) {
	case ID_BANDIT1:
	case ID_BANDIT2:
		GetBaddieTarget(creature->item_num, 0);
		break;
	case ID_MONK1:
	case ID_MONK2:
		GetBaddieTarget(creature->item_num, 1);
		break;
	default:
		creature->enemy = LaraItem;
		break;
	}

	enemy = creature->enemy;
	if (enemy == NULL) {
		enemy = LaraItem;
	}

	if (creature->LOT.fly != 0) {
		zone = FlyZones[FlipStatus];
	} else {
		zone = GroundZones[2 * (creature->LOT.step >> 8) + FlipStatus];
	}

	ROOM_INFO *roomSource = &RoomInfo[item->roomNumber];
	item->boxNumber = roomSource->floor[((item->pos.z - roomSource->z) >> WALL_SHIFT) + roomSource->xSize * ((item->pos.x - roomSource->x) >> WALL_SHIFT)].box;
	ai->zone_number = zone[item->boxNumber];

	ROOM_INFO *roomTarget = &RoomInfo[enemy->roomNumber];
	enemy->boxNumber = roomTarget->floor[((enemy->pos.z - roomTarget->z) >> WALL_SHIFT) + roomTarget->xSize * ((enemy->pos.x - roomTarget->x) >> WALL_SHIFT)].box;
	ai->enemy_zone = zone[enemy->boxNumber];

	if ( CHK_ANY(Boxes[enemy->boxNumber].overlapIndex, creature->LOT.block_mask) || creature->LOT.node[item->boxNumber].search_number == (creature->LOT.search_number | 0x8000) ) {
		ai->enemy_zone |= 0x4000;
	}

	OBJECT_INFO *obj = &Objects[item->objectID];
	int x = enemy->pos.x - ((obj->pivotLength * phd_cos(item->pos.rotY)) >> W2V_SHIFT) - item->pos.x;
	int z = enemy->pos.z - ((obj->pivotLength * phd_sin(item->pos.rotY)) >> W2V_SHIFT) - item->pos.z;
	int angle = phd_atan(z, x);
	if (creature->enemy != NULL) {
		ai->distance = SQR(x) + SQR(z);
	} else {
		ai->distance = 0x7FFFFFFF;
	}
	ai->angle = angle - item->pos.rotY;
	ai->enemy_facing = angle - enemy->pos.rotY + PHD_180;
	ai->ahead = angle > -PHD_90 && angle < PHD_90;
	if (ai->ahead && enemy->hitPoints >= 0 && ABS(item->pos.y - enemy->pos.y) < 384) {
		ai->bite = 1;
	} else {
		ai->bite = 0;
	}
}*/

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

void __cdecl GetBaddieTarget(__int16 itemNum, int flags) {
	ITEM_INFO *item = &Items[itemNum], *target = NULL, *bestItem = NULL;
	CREATURE_INFO *creature = (CREATURE_INFO*)item->data, *creatureFound = NULL;
	int bestDistance = 0x7FFFFFFF, distance = 0;
	int x, y, z;

	for (int i = 0; i < MAXIMUM_CREATURE_SLOTS; i++) {
		creatureFound = &ActiveCreatures[i];
		if (creatureFound->item_num == -1 || creatureFound->item_num == itemNum)
			continue;
		target = &Items[creatureFound->item_num];
		switch (flags) {
		case 1: // MONK
			if (target->objectID != ID_BANDIT1 && target->objectID != ID_BANDIT2)
				continue;
			break;
		case 0: // BANDIT (Mercenary)
			if (target->objectID != ID_MONK1 && target->objectID != ID_MONK2)
				continue;
			break;
		default:
			continue;
		}
		x = (target->pos.x - item->pos.x) >> 6;
		y = (target->pos.y - item->pos.y) >> 6;
		z = (target->pos.z - item->pos.z) >> 6;
		distance = SQR(z) + SQR(y) + SQR(x);
		if (distance < bestDistance) {
			bestItem = target;
			bestDistance = distance;
		}
	}

	if (bestItem == NULL) {
		if (flags == 0 || MonksAttackLara) {
			creature->enemy = LaraItem;
		} else {
			creature->enemy = NULL;
		}
		return;
	}

	if (flags == 0 || MonksAttackLara) {
		x = (LaraItem->pos.x - item->pos.x) >> 6;
		y = (LaraItem->pos.y - item->pos.y) >> 6;
		z = (LaraItem->pos.z - item->pos.z) >> 6;
		distance = SQR(z) + SQR(y) + SQR(x);
		if (distance < bestDistance) {
			bestItem = LaraItem;
			bestDistance = distance;
		}
	}

	target = creature->enemy;
	if (target != NULL && target->status == ITEM_ACTIVE) {
		x = (target->pos.x - item->pos.x) >> 6;
		y = (target->pos.y - item->pos.y) >> 6;
		z = (target->pos.z - item->pos.z) >> 6;
		distance = SQR(z) + SQR(y) + SQR(x);
		if (distance < (bestDistance + 0x400000)) {
			creature->enemy = bestItem;
		}
	} else {
		creature->enemy = bestItem;
	}
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
	INJECT(0x004103A0, GetBaddieTarget);
}
