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
#include "game/objects.h"
#include "3dsystem/phd_math.h"
#include "game/effects.h"
#include "game/items.h"
#include "game/missile.h"
#include "game/sound.h"
#include "global/vars.h"

void __cdecl InitialiseWindow(__int16 itemID) {
	ITEM_INFO *item;
	ROOM_INFO *room;
	BOX_INFO *box;

	item = &Items[itemID];
	item->flags = 0;
	item->meshBits = 1;
	room = &RoomInfo[item->roomNumber];
	box = &Boxes[room->floor[((item->pos.x - room->x) >> WALL_SHIFT) * room->xSize + ((item->pos.z - room->z) >> WALL_SHIFT)].box];
	if (CHK_ANY(box->overlapIndex, 0x8000))
		box->overlapIndex |= 0x4000;
}

void __cdecl SmashWindow(__int16 itemID) {
	ITEM_INFO *item;
	ROOM_INFO *room;
	BOX_INFO *box;

	item = &Items[itemID];
	room = &RoomInfo[item->roomNumber];
	box = &Boxes[room->floor[((item->pos.x - room->x) >> WALL_SHIFT) * room->xSize + ((item->pos.z - room->z) >> WALL_SHIFT)].box];
	if (CHK_ANY(box->overlapIndex, 0x8000))
		box->overlapIndex &= ~0x4000;
	PlaySoundEffect(58, &item->pos, 0);
	item->collidable = 0;
	item->meshBits = 0xFFFE;
	ExplodingDeath(itemID, 0xFEFE, 0);
	item->flags |= IFL_INVISIBLE;
	if (item->status == ITEM_ACTIVE)
		RemoveActiveItem(itemID);
	item->status = ITEM_DISABLED;
}

void __cdecl WindowControl(__int16 itemID) {
	ITEM_INFO *item;
	int val;

	item = &Items[itemID];
	if (!CHK_ANY(item->flags, IFL_INVISIBLE)) {
		if (Lara.skidoo == -1) {
			if (item->touchBits) {
				item->touchBits = 0;
				val = phd_cos(LaraItem->pos.rotY - item->pos.rotY) * LaraItem->speed >> W2V_SHIFT;
				if (ABS(val) >= 50)
					SmashWindow(itemID);
			}
		} else {
			if (ItemNearLara(&item->pos, 512))
				SmashWindow(itemID);
		}
	}
}

/*
 * Inject function
 */
void Inject_Objects() {
//	INJECT(0x004342C0, EarthQuake);
//	INJECT(0x004343A0, ControlCutShotgun);
//	INJECT(0x004343E0, InitialiseFinalLevel);
//	INJECT(0x004344B0, FinalLevelCounter);
//	INJECT(0x004346C0, MiniCopterControl);
//	INJECT(0x004347A0, InitialiseDyingMonk);
//	INJECT(0x00434820, DyingMonk);
//	INJECT(0x004348B0, ControlGongBonger);
//	INJECT(0x00434970, DeathSlideCollision);
//	INJECT(0x00434A30, ControlDeathSlide);
//	INJECT(0x00434CC0, BigBowlControl);
//	INJECT(0x00434DB0, BellControl);

	INJECT(0x00434E30, InitialiseWindow);
	INJECT(0x00434EB0, SmashWindow);
	INJECT(0x00434F80, WindowControl);

//	INJECT(0x00435020, SmashIceControl);
//	INJECT(0x00435100, ShutThatDoor);
//	INJECT(0x00435150, OpenThatDoor);
//	INJECT(0x00435190, InitialiseDoor);
//	INJECT(0x00435570, DoorControl);
//	INJECT(0x00435640, OnDrawBridge);
//	INJECT(0x00435700, DrawBridgeFloor);
//	INJECT(0x00435740, DrawBridgeCeiling);
//	INJECT(0x00435780, DrawBridgeCollision);
//	INJECT(0x004357B0, InitialiseLift);
//	INJECT(0x004357F0, LiftControl);
//	INJECT(0x004358D0, LiftFloorCeiling);
//	INJECT(0x00435A50, LiftFloor);
//	INJECT(0x00435A90, LiftCeiling);
//	INJECT(0x00435AD0, BridgeFlatFloor);
//	INJECT(0x00435AF0, BridgeFlatCeiling);
//	INJECT(0x00435B10, GetOffset);
//	INJECT(0x00435B50, BridgeTilt1Floor);
//	INJECT(0x00435B80, BridgeTilt1Ceiling);
//	INJECT(0x00435BC0, BridgeTilt2Floor);
//	INJECT(0x00435BF0, BridgeTilt2Ceiling);
//	INJECT(0x00435C30, CopterControl);
//	INJECT(0x00435D40, GeneralControl);
//	INJECT(0x00435E20, DetonatorControl);
}
