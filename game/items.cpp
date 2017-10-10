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
#include "game/items.h"
#include "global/vars.h"

void __cdecl InitialiseItemArray(int itemCount) {
	int i;

	NextItemFree = LevelItemCount;
	PrevItemActive = -1;
	NextItemActive = -1;

	for( i = LevelItemCount; i+1 < itemCount; ++i ) {
		Items[i].bitFields &= ~1; // clear active flag
		Items[i].nextItem = i+1;
	}
	Items[i].nextItem = -1;
}

void __cdecl InitialiseItem(__int16 itemIndex) {
	// TODO: define all bitfields as bitfields
	// TODO: complete some comments
	ITEM_INFO *item;
	ROOM_INFO *room;
	FLOOR_INFO *floor;

	item = &Items[itemIndex];
	item->animNumber = Objects[item->objectID].animIndex;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->goalAnimState = Anims[item->animNumber].currentAnimState;
	item->currentAnimState = item->goalAnimState;
	item->requiredAnimState = 0;
	item->pos.rotX = 0;
	item->pos.rotZ = 0;
	item->speed = 0;
	item->fallSpeed = 0;
	item->hitPoints = Objects[item->objectID].hitPoints;
	item->timer = 0;
	item->meshBits = 0xFFFFFFFF;
	item->touchBits = 0;
	item->data = 0;

	item->bitFields &= ~0x015F; // clear most bitFields
	item->bitFields |= 0x0020; // set "collidable"

	if( (item->flags & 0x0100) != 0 ) { // check "not visible"
		item->bitFields |= 6; // set invisible status
		item->flags &= ~0x0100; // clear "not visible"
	} else if ( Objects[item->objectID].flags & 2 ) {
		item->bitFields |= 6; // set invisible status
	}

	if( (item->flags & 0x8000) != 0 ) { // check ???
		item->bitFields |= 0x0100; // clear ???
		item->flags &= ~0x8000; // set ???
	}

	if ( (item->flags & 0x3E00) == 0x3E00 ) { // check all codeBits
		item->flags &= ~0x3E00; // clear all codeBits
		item->flags |= 0x4000; // set reverse bits
		AddActiveItem(itemIndex);
		item->bitFields |= 2; // set active (bit1)
		item->bitFields &= ~4; // set active (bit2)
	}

	room = &RoomInfo[item->roomNumber];
	item->nextItem = room->itemNumber;
	room->itemNumber = itemIndex;

	floor = &room->floor[((item->pos.z - room->z) >> WALL_SHIFT) + room->xSize * ((item->pos.x - room->x) >> WALL_SHIFT)];
	item->floor = floor->floor << 8;

	if ( SaveGame.bonusFlag && !IsDemoLevelType )
		item->hitPoints *= 2;

	if( Objects[item->objectID].initialise != NULL )
		Objects[item->objectID].initialise(itemIndex);
}

void __cdecl AddActiveItem(__int16 itemIndex) {
	ITEM_INFO *item = &Items[itemIndex];

	if( Objects[item->objectID].control == NULL ) {
		item->bitFields &= ~6; // set not active status
	}
	else if( (item->bitFields & 1) == 0 ) { // check active flag
		item->bitFields |= 1; // set active flag
		item->nextActive = NextItemActive;
		NextItemActive = itemIndex;
	}
}

int __cdecl GlobalItemReplace(int oldItemID, int newItemID) {
	int i, j;
	int result = 0;

	for( i = 0; i < RoomCount; ++i ) {
		for( j = RoomInfo[i].itemNumber; j != -1; j = Items[j].nextItem ) {
			if( Items[j].objectID == oldItemID ) {
				Items[j].objectID = newItemID;
				++result;
			}
		}
	}
	return result;
}

/*
 * Inject function
 */
void Inject_Items() {
	INJECT(0x00426CD0, InitialiseItemArray);
	INJECT(0x00426E90, InitialiseItem);
	INJECT(0x00427150, AddActiveItem);
	INJECT(0x00427250, GlobalItemReplace);
}
