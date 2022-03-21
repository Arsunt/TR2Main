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
#include "game/control.h"
#include "global/vars.h"

void __cdecl ShutThatDoor(DOORPOS_DATA *door) {
	FLOOR_INFO* floor = door->floor;
	if (floor != NULL) {
		floor->index = 0;
		floor->ceiling = -127;
		floor->floor = -127;
		floor->box = -1;
		floor->skyRoom = -1;
		floor->pitRoom = -1;
		if (door->box != -1) {
			Boxes[door->box].overlapIndex |= 0x40; // BLOCKED
		}
	}
}

void __cdecl OpenThatDoor(DOORPOS_DATA *door) {
	if (door->floor) {
		*door->floor = door->data;
		if (door->box != -1) {
			Boxes[door->box].overlapIndex &= ~0x40; // UNBLOCKED
		}
	}
}

void __cdecl DoorControl(__int16 itemID) {
	ITEM_INFO *item = &Items[itemID];
	DOOR_DATA *data = (DOOR_DATA*)item->data;

	if (TriggerActive(item)) {
		if (item->currentAnimState) {
			OpenThatDoor(&data->d1);
			OpenThatDoor(&data->d2);
			OpenThatDoor(&data->d1flip);
			OpenThatDoor(&data->d2flip);
		} else {
			item->goalAnimState = 1;
		}
	}
	else if (item->currentAnimState == 1) {
		item->goalAnimState = 0;
	} else {
		ShutThatDoor(&data->d1);
		ShutThatDoor(&data->d2);
		ShutThatDoor(&data->d1flip);
		ShutThatDoor(&data->d2flip);
	}
	AnimateItem(item);
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
//	INJECT(0x00434E30, InitialiseWindow);
//	INJECT(0x00434EB0, SmashWindow);
//	INJECT(0x00434F80, WindowControl);
//	INJECT(0x00435020, SmashIceControl);
	INJECT(0x00435100, ShutThatDoor);
	INJECT(0x00435150, OpenThatDoor);
//	INJECT(0x00435190, InitialiseDoor);
	INJECT(0x00435570, DoorControl);
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
