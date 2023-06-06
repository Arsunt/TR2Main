/*
 * Copyright (c) 2017-2018 Michael Chaban. All rights reserved.
 * Original game is created by Core Design Ltd. in 1997.
 * Lara Croft and Tomb Raider are trademarks of Embracer Group AB.
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
#include "game/lot.h"
#include "specific/init.h"
#include "global/vars.h"

#ifdef FEATURE_GOLD
extern bool IsGold();
#endif

void __cdecl InitialiseLOTarray() {
	ActiveCreatures = (CREATURE_INFO*)game_malloc(MAXIMUM_CREATURE_SLOTS * sizeof(CREATURE_INFO), GBUF_CreatureData);
	for (int i = 0; i < MAXIMUM_CREATURE_SLOTS; i++) {
		CREATURE_INFO *creature = &ActiveCreatures[i];
		creature->item_num = -1;
		creature->LOT.node = (BOX_NODE*)game_malloc(sizeof(BOX_NODE) * BoxesCount, GBUF_CreatureLOT);
	}
	ActiveCreaturesUsed = 0;
}

void __cdecl DisableBaddieAI(__int16 itemNum) {
	CREATURE_INFO *creature = NULL;
	ITEM_INFO *item = &Items[itemNum];

	if (itemNum == Lara.item_number) {
		creature = Lara.creature;
		Lara.creature = NULL;
	} else {
		creature = (CREATURE_INFO*)item->data;
		item->data = NULL;
	}

	if (creature != NULL) {
		creature->item_num = -1;
		ActiveCreaturesUsed--;
	}
}

BOOL __cdecl EnableBaddieAI(__int16 itemNum, BOOL always) {
	if (itemNum == Lara.item_number && Lara.creature != NULL)
		return TRUE;

	ITEM_INFO *item = &Items[itemNum];
	if (item->data != NULL)
		return TRUE;

	CREATURE_INFO *creature = NULL;
	if (ActiveCreaturesUsed >= MAXIMUM_CREATURE_SLOTS) {
		int x, y, z;
		int distance, bestdistance, slot = -1;
		if (!always) {
			item = &Items[itemNum];
			x = (item->pos.x - Camera.pos.x) >> 8;
			y = (item->pos.y - Camera.pos.y) >> 8;
			z = (item->pos.z - Camera.pos.z) >> 8;
			bestdistance = SQR(x) + SQR(y) + SQR(z);
		} else {
			bestdistance = 0;
		}

		for (int i = 0; i < MAXIMUM_CREATURE_SLOTS; i++) {
			item = &Items[ActiveCreatures[i].item_num];
			x = (item->pos.x - Camera.pos.x) >> 8;
			y = (item->pos.y - Camera.pos.y) >> 8;
			z = (item->pos.z - Camera.pos.z) >> 8;
			distance = SQR(x) + SQR(y) + SQR(z);
			if (distance > bestdistance) {
				bestdistance = distance;
				slot = i;
			}
		}

		if (slot >= 0 && slot < MAXIMUM_CREATURE_SLOTS) {
			creature = &ActiveCreatures[slot];
			item = &Items[creature->item_num];
			item->status = ITEM_INVISIBLE;
			DisableBaddieAI(creature->item_num);
			InitialiseSlot(itemNum, slot);
			return TRUE;
		}
	} else {
		for (int i = 0; i < MAXIMUM_CREATURE_SLOTS; i++) {
			creature = &ActiveCreatures[i];
			if (creature->item_num == -1) {
				InitialiseSlot(itemNum, i);
				return TRUE;
			}
		}
	}

	return FALSE;
}

void __cdecl InitialiseSlot(__int16 itemNum, int creatureNum) {
	CREATURE_INFO *creature = &ActiveCreatures[creatureNum];
	ITEM_INFO *item = &Items[itemNum];

	if (itemNum == Lara.item_number)
		Lara.creature = creature;
	else
		item->data = creature;
	creature->item_num = itemNum;
	creature->mood = MOOD_BORED;
	creature->neck_rotation = 0;
	creature->head_rotation = 0;
	creature->maximum_turn = PHD_DEGREE;
	creature->flags = 0;
	creature->enemy = NULL;
	creature->LOT.step = 256;
	creature->LOT.drop = -512;
	creature->LOT.block_mask = 0x8000;
	creature->LOT.fly = 0;

	switch (item->objectID) {
	case ID_LARA:
		creature->LOT.step = 20480;
		creature->LOT.drop = -20480;
		creature->LOT.fly = 256;
		break;
	case ID_SHARK:
	case ID_BARRACUDA:
	case ID_DIVER:
	case ID_JELLY:
	case ID_CROW:
	case ID_EAGLE:
		creature->LOT.step = 20480;
		creature->LOT.drop = -20480;
		creature->LOT.fly = 16;
		if (item->objectID == ID_SHARK)
			creature->LOT.block_mask = 0x8000;
		break;
	case ID_WORKER3:
	case ID_WORKER4:
	case ID_YETI:
		creature->LOT.step = 1024;
		creature->LOT.drop = -1024;
		break;
#ifdef FEATURE_GOLD
	case ID_SPIDER_or_WOLF: // NOTE: don't include wolf since he not climb !
		if (!IsGold()) {
			creature->LOT.step = 512;
			creature->LOT.drop = -1024;
		}
		break;
	case ID_SKIDOO_ARMED:
		creature->LOT.step = 512;
		creature->LOT.drop = -1024;
		break;
#else
	case ID_SPIDER_or_WOLF:
	case ID_SKIDOO_ARMED:
		creature->LOT.step = 512;
		creature->LOT.drop = -1024;
		break;
#endif
	case ID_DINO:
		creature->LOT.block_mask = 0x8000;
		break;
	}

	ClearLOT(&creature->LOT);
	if (itemNum != Lara.item_number)
		CreateZone(item);
	ActiveCreaturesUsed++;
}

/*
 * Inject function
 */
void Inject_Lot() {
	INJECT(0x00432B10, InitialiseLOTarray);
	INJECT(0x00432B70, DisableBaddieAI);
	INJECT(0x00432BC0, EnableBaddieAI);
	INJECT(0x00432D70, InitialiseSlot);
//	INJECT(0x00432F80, CreateZone);
//	INJECT(0x00433040, ClearLOT);
}
