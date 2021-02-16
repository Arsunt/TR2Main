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
#include "game/boat.h"
#include "3dsystem/phd_math.h"
#include "game/control.h"
#include "game/items.h"
#include "game/missile.h"
#include "game/sound.h"
#include "specific/game.h"
#include "specific/output.h"
#include "global/vars.h"

void __cdecl DoBoatShift(int itemID) {
	ITEM_INFO *item, *link;
	__int16 linkID;
	int x, z, dx, dz;

	item = &Items[itemID];
	for (linkID = RoomInfo[item->roomNumber].itemNumber; linkID != -1; linkID = link->nextItem) {
		link = &Items[linkID];
		if (link->objectID == ID_BOAT && linkID != itemID && Lara.skidoo != linkID) {
			dz = link->pos.z - item->pos.z;
			dx = link->pos.x - item->pos.x;
			if (SQR(dx) + SQR(dz) < SQR(1000)) {
				item->pos.x = link->pos.x - SQR(1000) * dx / (SQR(dx) + SQR(dz));
				item->pos.z = link->pos.z - SQR(1000) * dz / (SQR(dx) + SQR(dz));
			}
		} else {
			if (link->objectID == ID_GONDOLA && link->currentAnimState == 1) {
				x = link->pos.x - (512 * phd_sin(link->pos.rotY) >> W2V_SHIFT);
				z = link->pos.z - (512 * phd_cos(link->pos.rotY) >> W2V_SHIFT);
				dx = x - item->pos.x;
				dz = z - item->pos.z;
				if (SQR(dx) + SQR(dz) < SQR(1000)) {
					if (item->speed < 80) {
						item->pos.x = x - SQR(1000) * dx / (SQR(dx) + SQR(dz));
						item->pos.z = z - SQR(1000) * dz / (SQR(dx) + SQR(dz));
					} else {
						if (link->pos.y - item->pos.y < 2048) {
							PlaySoundEffect(337, &link->pos, 0);
							link->goalAnimState = 2;
						}
					}
				}
			}
		}
	}
}

void __cdecl DoWakeEffect(ITEM_INFO *item) {
	__int16 frame_number, fxID;
	int i;
	FX_INFO *fx;

	PhdMatrixPtr->_23 = 0;
	S_CalculateLight(item->pos.x, item->pos.y, item->pos.z, item->roomNumber);
	frame_number = Objects[ID_WATER_SPRITE].nMeshes * GetRandomDraw() >> 15;
	for (i = 0; i < 3; ++i) {
		fxID = CreateEffect(item->roomNumber);
		if (fxID != -1) {
			fx = &Effects[fxID];
			fx->pos.x = item->pos.x + ((-700 * phd_sin(item->pos.rotY) + 300 * (i - 1) * phd_cos(item->pos.rotY)) >> W2V_SHIFT);
			fx->pos.y = item->pos.y;
			fx->pos.z = item->pos.z + ((-700 * phd_cos(item->pos.rotY) - 300 * (i - 1) * phd_sin(item->pos.rotY)) >> W2V_SHIFT);
			fx->pos.rotY = PHD_90 * (i - 1) + item->pos.rotY;
			fx->room_number = item->roomNumber;
			fx->frame_number = frame_number;
			fx->counter = 20;
			fx->object_number = ID_WATER_SPRITE;
			fx->speed = item->speed >> 2;
			if (item->speed < 64) {
				fx->fallspeed = (ABS(item->speed) - 64) * GetRandomDraw() >> 15;
			} else {
				fx->fallspeed = 0;
			}
			fx->shade = LsAdder - 768;
			CLAMPL(fx->shade, 0);
		}
	}
}

void __cdecl GondolaControl(__int16 itemID) {
	ITEM_INFO *item;
	__int16 roomID;

	item = &Items[itemID];
	switch (item->currentAnimState) {
		case 1:
			if (item->goalAnimState == 2) {
				item->meshBits = 0xFF;
				ExplodingDeath(itemID, 0xF0, 0);
			}
			break;
		case 3:
			item->pos.y += 50;
			roomID = item->roomNumber;
			item->floor = GetHeight(GetFloor(item->pos.x, item->pos.y, item->pos.z, &roomID), item->pos.x, item->pos.y, item->pos.z);
			if (item->pos.y >= item->floor) {
				item->goalAnimState = 4;
				item->pos.y = item->floor;
			}
			break;
	}
	AnimateItem(item);
	if (item->status == ITEM_DISABLED)
		RemoveActiveItem(itemID);
}

/*
 * Inject function
 */
void Inject_Boat() {
//	INJECT(0x0040CB10, InitialiseBoat);
//	INJECT(0x0040CB50, BoatCheckGeton);
//	INJECT(0x0040CCC0, BoatCollision);
//	INJECT(0x0040CE20, TestWaterHeight);

	INJECT(0x0040CF20, DoBoatShift);
	INJECT(0x0040D0F0, DoWakeEffect);

//	INJECT(0x0040D270, DoBoatDynamics);
//	INJECT(0x0040D2C0, BoatDynamics);
//	INJECT(0x0040D7A0, BoatUserControl);
//	INJECT(0x0040D930, BoatAnimation);
//	INJECT(0x0040DAA0, BoatControl);

	INJECT(0x0040E0D0, GondolaControl);
}
