/*
 * Copyright (c) 2017-2020 Michael Chaban. All rights reserved.
 * Copyright (c) 2020 ChocolateFan <asasas9500@gmail.com>
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
#include "game/lara1gun.h"
#include "3dsystem/phd_math.h"
#include "game/box.h"
#include "game/control.h"
#include "game/draw.h"
#include "game/items.h"
#include "game/lara.h"
#include "game/larafire.h"
#include "game/objects.h"
#include "game/sound.h"
#include "global/vars.h"

void __cdecl FireRocket() {
	__int16 itemID;
	ITEM_INFO *item;
	PHD_VECTOR pos;

	if (Lara.grenade_ammo > 0) {
		itemID = CreateItem();
		if (itemID != -1) {
			item = &Items[itemID];
			item->objectID = ID_ROCKET;
			item->roomNumber = LaraItem->roomNumber;
			pos.x = -2;
			pos.y = 373;
			pos.z = 77;
			GetLaraJointAbsPosition(&pos, 10);
			item->pos.x = pos.x;
			item->pos.y = pos.y;
			item->pos.z = pos.z;
			InitialiseItem(itemID);
			item->pos.rotX = LaraItem->pos.rotX + Lara.left_arm.x_rot;
			item->pos.rotZ = 0;
			item->speed = 200;
			item->fallSpeed = 0;
			item->pos.rotY = LaraItem->pos.rotY + Lara.left_arm.y_rot;
			AddActiveItem(itemID);
			if (!SaveGame.bonusFlag)
				--Lara.grenade_ammo;
			++SaveGame.statistics.shots;
		}
	}
}

void __cdecl ControlRocket(__int16 itemID) {
	ITEM_INFO *item, *link;
	int oldX, oldY, oldZ, displacement, c, s, r, oldR;
	__int16 room, linkID, *frame, fxID;
	FLOOR_INFO *floor;
	BOOL collision;
	FX_INFO *fx;

	item = &Items[itemID];
	oldX = item->pos.x;
	oldY = item->pos.y;
	oldZ = item->pos.z;
	if (item->speed < 190)
		++item->fallSpeed;
	--item->speed;
	item->pos.y += item->fallSpeed - (item->speed * phd_sin(item->pos.rotX) >> W2V_SHIFT);
	item->pos.z += phd_cos(item->pos.rotY) * (item->speed * phd_cos(item->pos.rotX) >> W2V_SHIFT) >> W2V_SHIFT;
	item->pos.x += phd_sin(item->pos.rotY) * (item->speed * phd_cos(item->pos.rotX) >> W2V_SHIFT) >> W2V_SHIFT;
	room = item->roomNumber;
	floor = GetFloor(item->pos.x, item->pos.y, item->pos.z, &room);
	item->floor = GetHeight(floor, item->pos.x, item->pos.y, item->pos.z);
	if (item->roomNumber != room)
		ItemNewRoom(itemID, room);
	if (item->pos.y < item->floor && item->pos.y > GetCeiling(floor, item->pos.x, item->pos.y, item->pos.z)) {
		collision = FALSE;
		displacement = 0;
	} else {
		displacement = 512;
		collision = TRUE;
	}
	for (linkID = RoomInfo[item->roomNumber].itemNumber; linkID != -1; linkID = link->nextItem) {
		link = &Items[linkID];
		if (link != LaraItem &&
			link->collidable &&
			(link->objectID == ID_WINDOW1 ||
			(Objects[link->objectID].intelligent &&
			link->status != ITEM_INVISIBLE &&
			Objects[link->objectID].collision)))
		{
			frame = GetBestFrame(link);
			if (item->pos.y + displacement >= link->pos.y + frame[2] && item->pos.y - displacement <= link->pos.y + frame[3]) {
				c = phd_cos(link->pos.rotY);
				s = phd_sin(link->pos.rotY);
				r = (c * (item->pos.x - link->pos.x) - s * (item->pos.z - link->pos.z)) >> W2V_SHIFT;
				oldR = (c * (oldX - link->pos.x) - s * (oldZ - link->pos.z)) >> W2V_SHIFT;
				if ((r + displacement >= frame[0] ||
					oldR + displacement >= frame[0]) &&
					(r - displacement <= frame[1] ||
					oldR - displacement <= frame[1]))
				{
					r = (s * (item->pos.x - link->pos.x) + c * (item->pos.z - link->pos.z)) >> W2V_SHIFT;
					oldR = (s * (oldX - link->pos.x) + c * (oldZ - link->pos.z)) >> W2V_SHIFT;
					if ((r + displacement >= frame[4] ||
						oldR + displacement >= frame[4]) &&
						(r - displacement <= frame[5] ||
						oldR - displacement <= frame[5]))
					{
						if (link->objectID == ID_WINDOW1) {
							SmashWindow(linkID);
						} else {
							if (link->status == ITEM_ACTIVE) {
								HitTarget(link, NULL, 30);
								++SaveGame.statistics.hits;
								if (link->hitPoints <= 0) {
									++SaveGame.statistics.kills;
									if (link->objectID != ID_DRAGON_FRONT && link->objectID != ID_GIANT_YETI)
										CreatureDie(linkID, TRUE);
								}
							}
							collision = TRUE;
						}
					}
				}
			}
		}
	}
	if (collision) {
		fxID = CreateEffect(item->roomNumber);
		if (fxID != -1) {
			fx = &Effects[fxID];
			fx->pos.x = oldX;
			fx->pos.y = oldY;
			fx->pos.z = oldZ;
			fx->speed = 0;
			fx->frame_number = 0;
			fx->counter = 0;
			fx->object_number = ID_EXPLOSION;
		}
		PlaySoundEffect(105, NULL, 0);
		KillItem(itemID);
	}
}

/*
 * Inject function
 */
void Inject_Lara1Gun() {
//	INJECT(0x0042BC90, draw_shotgun_meshes);
//	INJECT(0x0042BCD0, undraw_shotgun_meshes);
//	INJECT(0x0042BD00, ready_shotgun);
//	INJECT(0x0042BD70, RifleHandler);
//	INJECT(0x0042BE70, FireShotgun);
//	INJECT(0x0042BF70, FireM16);
//	INJECT(0x0042BFF0, FireHarpoon);
//	INJECT(0x0042C180, ControlHarpoonBolt);

	INJECT(0x0042C4D0, FireRocket);
	INJECT(0x0042C5C0, ControlRocket);

//	INJECT(0x0042C9D0, draw_shotgun);
//	INJECT(0x0042CB40, undraw_shotgun);
//	INJECT(0x0042CC50, AnimateShotgun);
}
