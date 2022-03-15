/*
 * Copyright (c) 2017-2021 Michael Chaban. All rights reserved.
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
#include "game/collide.h"
#include "3dsystem/phd_math.h"
#include "3dsystem/3d_gen.h"
#include "game/draw.h"
#include "game/items.h"
#include "game/control.h"
#include "game/sound.h"
#include "game/sphere.h"
#include "global/vars.h"

int __cdecl FindGridShift(int src, int dest) {
	int srcShift, destShift;
	srcShift = src >> WALL_SHIFT;
	destShift = dest >> WALL_SHIFT;
	if (srcShift == destShift) {
		return 0;
	}
	src &= 1023;
	if (destShift <= srcShift) {
		return -1 - src;
	} else {
		return 1025 - src;
	}
}

int __cdecl CollideStaticObjects(COLL_INFO *coll, int x, int y, int z, __int16 roomID, int hite) {
	int rxMin = x - coll->radius;
	int rxMax = x + coll->radius;
	int ryMin = y - hite;
	int ryMax = y;
	int rzMin = z - coll->radius;
	int rzMax = z + coll->radius;

	coll->hitStatic = 0;
	GetNearByRooms(x, y, z, coll->radius + 50, hite + 50, roomID);

	// outer loop
	for( int i = 0; i < DrawRoomsCount; ++i ) {
		ROOM_INFO *room = &RoomInfo[DrawRoomsArray[i]];
		for( int j = 0; j < room->numMeshes; ++j ) {
			MESH_INFO *mesh = &room->mesh[j];
			if( CHK_ANY(StaticObjects[mesh->staticNumber].flags, 1) ) {
				continue;
			}

			STATIC_BOUNDS *bounds = &StaticObjects[mesh->staticNumber].collisionBounds;
			int yMin = mesh->y + bounds->yMin;
			int yMax = mesh->y + bounds->yMax;
			int xMin = mesh->x;
			int xMax = mesh->x;
			int zMin = mesh->z;
			int zMax = mesh->z;

			switch( mesh->yRot ) {
				case -PHD_90: // west
					xMin -= bounds->zMax;
					xMax -= bounds->zMin;
					zMin += bounds->xMin;
					zMax += bounds->xMax;
					break;
				case -PHD_180: // south
					xMin -= bounds->xMax;
					xMax -= bounds->xMin;
					zMin -= bounds->zMax;
					zMax -= bounds->zMin;
					break;
				case PHD_90: // east
					xMin += bounds->zMin;
					xMax += bounds->zMax;
					zMin -= bounds->xMax;
					zMax -= bounds->xMin;
					break;
				default: // north
					xMin += bounds->xMin;
					xMax += bounds->xMax;
					zMin += bounds->zMin;
					zMax += bounds->zMax;
					break;
			}

			if( rxMax <= xMin || rxMin >= xMax ||
				ryMax <= yMin || ryMin >= yMax ||
				rzMax <= zMin || rzMin >= zMax )
			{
				continue;
			}
			int shift[2];

			shift[0] = rxMax - xMin;
			shift[1] = xMax - rxMin;
			int xShift = (shift[0] < shift[1]) ? -shift[0] : shift[1];

			shift[0] = rzMax - zMin;
			shift[1] = zMax - rzMin;
			int zShift = (shift[0] < shift[1]) ? -shift[0] : shift[1];

			switch ( coll->quadrant ) {
				case 0: // north
					if( xShift > coll->radius || xShift < -coll->radius ) {
						coll->shift.x = coll->old.x - x;
						coll->shift.z = zShift;
						coll->collType = COLL_FRONT;
					} else if( xShift > 0 ) {
						coll->shift.x = xShift;
						coll->shift.z = 0;
						coll->collType = COLL_LEFT;
					} else if( xShift < 0 ) {
						coll->shift.x = xShift;
						coll->shift.z = 0;
						coll->collType = COLL_RIGHT;
					}
					break;

				case 1: // east
					if( zShift > coll->radius || zShift < -coll->radius ) {
						coll->shift.x = xShift;
						coll->shift.z = coll->old.z - z;
						coll->collType = COLL_FRONT;
					} else if( zShift > 0 ) {
						coll->shift.x = 0;
						coll->shift.z = zShift;
						coll->collType = COLL_RIGHT;
					} else if( zShift < 0 ) {
						coll->shift.x = 0;
						coll->shift.z = zShift;
						coll->collType = COLL_LEFT;
					}
					break;

				case 2: // south
					if( xShift > coll->radius || xShift < -coll->radius ) {
						coll->shift.x = coll->old.x - x;
						coll->shift.z = zShift;
						coll->collType = COLL_FRONT;
					} else if( xShift > 0 ) {
						coll->shift.x = xShift;
						coll->shift.z = 0;
						coll->collType = COLL_RIGHT;
					} else if( xShift < 0 )  {
						coll->shift.x = xShift;
						coll->shift.z = 0;
						coll->collType = COLL_LEFT;
					}
					break;

				case 3: // west
					if( zShift > coll->radius || zShift < -coll->radius ) {
						coll->shift.x = xShift;
						coll->shift.z = coll->old.z - z;
						coll->collType = COLL_FRONT;
					} else if( zShift > 0 ) {
						coll->shift.x = 0;
						coll->shift.z = zShift;
						coll->collType = COLL_LEFT;
					} else if( zShift < 0 ) {
						coll->shift.x = 0;
						coll->shift.z = zShift;
						coll->collType = COLL_RIGHT;
					}
					break;
			}

			coll->hitStatic = 1;
			return 1;
		}
	}
	return 0;
}

void __cdecl GetNearByRooms(int x, int y, int z, int r, int h, __int16 roomID) {
	DrawRoomsArray[0] = roomID;
	DrawRoomsCount = 1;
	GetNewRoom(x + r, y,     z + r, roomID);
	GetNewRoom(x - r, y,     z + r, roomID);
	GetNewRoom(x + r, y,     z - r, roomID);
	GetNewRoom(x - r, y,     z - r, roomID);
	GetNewRoom(x + r, y - h, z + r, roomID);
	GetNewRoom(x - r, y - h, z + r, roomID);
	GetNewRoom(x + r, y - h, z - r, roomID);
	GetNewRoom(x - r, y - h, z - r, roomID);
}

void __cdecl GetNewRoom(int x, int y, int z, __int16 roomID) {
	GetFloor(x, y, z, &roomID);
	for( int i = 0; i < DrawRoomsCount; ++i ) {
		if( DrawRoomsArray[i] == roomID ) {
			return;
		}
	}
	DrawRoomsArray[DrawRoomsCount++] = roomID;
}

void __cdecl ShiftItem(ITEM_INFO* item, COLL_INFO* coll) {
	item->pos.x += coll->shift.x;
	item->pos.y += coll->shift.y;
	item->pos.z += coll->shift.z;
	coll->shift.z = 0;
	coll->shift.y = 0;
	coll->shift.x = 0;
}

void __cdecl UpdateLaraRoom(ITEM_INFO* item, int height) {
	FLOOR_INFO* floor;
	int x, y, z;
	short roomID;

	x = item->pos.x;
	y = item->pos.y + height;
	z = item->pos.z;
	roomID = item->roomNumber;
	floor = GetFloor(x, y, z, &roomID);
	item->floor = GetHeight(floor, x, y, z);
	if (item->roomNumber != roomID) {
		ItemNewRoom(Lara.item_number, roomID);
	}
}

__int16 __cdecl GetTiltType(FLOOR_INFO* floor, int x, int y, int z) {
	__int16* data;
	unsigned char i;

	for (i = floor->pitRoom; i != 255; i = floor->pitRoom) {
		floor = &RoomInfo[i].floor[((z - RoomInfo[i].z) >> WALL_SHIFT) + RoomInfo[i].xSize * ((x - RoomInfo[i].x) >> WALL_SHIFT)];
	}
	if (floor->index == 0) {
		return 0;
	}
	data = &FloorData[floor->index];
	if (y + 512 >= floor->floor << 8 && *(BYTE*)data == 2) {
		return data[1];
	} else {
		return 0;
	}
}

void __cdecl LaraBaddieCollision(ITEM_INFO* laraitem, COLL_INFO* coll) {
	DOOR_INFOS* doors;
	DOOR_INFO* door;
	ITEM_INFO* item;
	OBJECT_INFO* obj;
	int x, y, z;
	__int16 roomArray[20];
	__int16 roomCount;
	__int16 itemID;
	__int16 i;

	laraitem->hit_status = 0;
	Lara.hit_direction = -1;

	// NOTE: added some nullptr check just in case something want wrong.
	if (laraitem->hitPoints > 0) {
		roomArray[0] = laraitem->roomNumber;
		roomCount = 1;
		doors = RoomInfo[roomArray[0]].doors;
		if (doors) {
			for (i = doors->wCount; i > 0; i--) {
				door = doors[i].door;
				if (door) { // NOTE: this check was not there in the original game
					roomArray[roomCount++] = door->room;
				}
			}
		}
		if (roomCount > 0) {
			itemID = RoomInfo[roomArray[0]].itemNumber;
			if (itemID != -1) {
				for (i = roomCount; i > 0; i--) {
					item = &Items[itemID];
					if (item && item->collidable && item->status != ITEM_INVISIBLE) { // NOTE: "item" was not there in the original game
						obj = &Objects[item->objectID];
						if (obj && obj->collision) { // NOTE: "obj" was not there in the original game
							x = laraitem->pos.x - item->pos.x;
							y = laraitem->pos.y - item->pos.y;
							z = laraitem->pos.z - item->pos.z;
							if (x > -4096 && x < 4096 && z > -4096 && z < 4096 && y > -4096 && y < 4096) {
								obj->collision(itemID, laraitem, coll);
							}
						}
					}
					itemID = item->nextItem;
				}
			}
		}
		if (Lara.spaz_effect_count != 0) {
			EffectSpaz(laraitem, coll); // NOTE: coll is not used !
		}
		if (Lara.hit_direction == -1) {
			Lara.hit_frame = 0;
		}
		InventoryChosen = -1;
	}
}

void __cdecl EffectSpaz(ITEM_INFO* laraitem, COLL_INFO* coll) {
	Lara.hit_direction = (unsigned __int16)(laraitem->pos.rotY + PHD_180 - phd_atan(Lara.spaz_effect->pos.z - laraitem->pos.z, Lara.spaz_effect->pos.x - laraitem->pos.x) + PHD_90) >> W2V_SHIFT;
	if (!Lara.hit_frame) {
		PlaySoundEffect(31, &laraitem->pos, 0);
	}
	if (++Lara.hit_frame > 34) {
		Lara.hit_frame = 34;
	}
	--Lara.spaz_effect_count;
}

void __cdecl CreatureCollision(__int16 itemID, ITEM_INFO* laraitem, COLL_INFO* coll) {
	ITEM_INFO* item;

	item = &Items[itemID];
	if (TestBoundsCollide(item, laraitem, coll->radius)) {
		if (TestCollision(item, laraitem)) {
			if (CHK_ANY(coll->flags, 0x8) && Lara.water_status != 1) { // NOTE: original checked "(Lara.water_status == 0) != 2" but it's always true !
				ItemPushLara(item, laraitem, coll, CHK_ANY(coll->flags, 0x10), FALSE);
			}
		}
	}
}

void __cdecl ObjectCollision(__int16 itemID, ITEM_INFO* laraitem, COLL_INFO* coll) {
	ITEM_INFO* item;

	item = &Items[itemID];
	if (TestBoundsCollide(item, laraitem, coll->radius)) {
		if (TestCollision(item, laraitem)) {
			if CHK_ANY(coll->flags, 0x8) {
				ItemPushLara(item, laraitem, coll, FALSE, TRUE);
			}
		}
	}
}

void __cdecl DoorCollision(__int16 itemID, ITEM_INFO* laraitem, COLL_INFO* coll) {
	ITEM_INFO* item;

	item = &Items[itemID];
	if (TestBoundsCollide(item, laraitem, coll->radius)) {
		if (TestCollision(item, laraitem)) {
			if CHK_ANY(coll->flags, 0x8) {
				if (item->currentAnimState == item->goalAnimState) {
					ItemPushLara(item, laraitem, coll, FALSE, TRUE);
				} else {
					ItemPushLara(item, laraitem, coll, CHK_ANY(coll->flags, 0x10), TRUE);
				}
			}
		}
	}
}

void __cdecl TrapCollision(__int16 itemID, ITEM_INFO* laraitem, COLL_INFO* coll) {
	ITEM_INFO* item;

	item = &Items[itemID];
	if (item->status == ITEM_ACTIVE) {
		if (TestBoundsCollide(item, laraitem, coll->radius)) {
			TestCollision(item, laraitem);
		}
	} else if (item->status != ITEM_INVISIBLE) {
		ObjectCollision(itemID, laraitem, coll);
	}
}

void __cdecl ItemPushLara(ITEM_INFO* item, ITEM_INFO* laraitem, COLL_INFO* coll, BOOL spazon, BOOL bigpush) {
	int x, z, rx, rz;
	int l, r, t, b;
	int bndMin, bndMax;
	int radius;
	__int16* bounds;
	__int16 c, s;
	__int16 minx, maxx, minz, maxz;
	__int16 oldFacing;

	x = laraitem->pos.x - item->pos.x;
	z = laraitem->pos.z - item->pos.z;
	s = phd_sin(item->pos.rotY);
	c = phd_cos(item->pos.rotY);
	rx = (x * c - z * s) >> W2V_SHIFT;
	rz = (x * s + z * c) >> W2V_SHIFT;
	bounds = GetBestFrame(item);
	minx = bounds[0];
	maxx = bounds[1];
	minz = bounds[4];
	maxz = bounds[5];

	if (bigpush) {
		radius = coll->radius;
		maxx += radius;
		minz -= radius;
		maxz += radius;
		minx -= radius;
	}

	if (rx >= minx && rx <= maxx && rz >= minz && rz <= maxz) {
		l = rx - minx;
		r = maxx - rx;
		t = maxz - rz;
		b = rz - minz;

		if (r <= l && r <= t && r <= b) {
			rx += r;
		} else if (l <= r && l <= t && l <= b) {
			rx -= l;
		} else if (t <= r && t <= l && t <= b) {
			rz += t;
		} else {
			rz -= b;
		}

		laraitem->pos.x = item->pos.x + ((rz * s + rx * c) >> W2V_SHIFT);
		laraitem->pos.z = item->pos.z + ((rz * c - rx * s) >> W2V_SHIFT);

		bndMin = (bounds[1] + bounds[0]) / 2;
		bndMax = (bounds[5] + bounds[4]) / 2;
		rx -= (bndMax * s + bndMin * c) >> W2V_SHIFT;
		rz -= (bndMax * c - bndMin * s) >> W2V_SHIFT;

		if (spazon && bounds[3] - bounds[2] > 256) {
			Lara.hit_direction = (unsigned __int16)(laraitem->pos.rotY + PHD_180 - phd_atan(rz, rx) + PHD_90) >> W2V_SHIFT;
			if (!Lara.hit_frame) {
				PlaySoundEffect(31, &laraitem->pos, 0);
			}
			if (++Lara.hit_frame > 34) {
				Lara.hit_frame = 34;
			}
		}

		x = coll->old.x;
		z = coll->old.z;
		oldFacing = coll->facing;
		coll->badPos = -NO_HEIGHT;
		coll->badNeg = -384;
		coll->badCeiling = 0;
		coll->facing = phd_atan(laraitem->pos.z - z, laraitem->pos.x - x);
		GetCollisionInfo(coll, laraitem->pos.x, laraitem->pos.y, laraitem->pos.z, laraitem->roomNumber, 762);
		coll->facing = oldFacing;
		if (coll->collType == 0) {
			coll->old.x = laraitem->pos.x;
			coll->old.y = laraitem->pos.y;
			coll->old.z = laraitem->pos.z;
			UpdateLaraRoom(laraitem, -10);
		} else {
			laraitem->pos.x = coll->old.x;
			laraitem->pos.z = coll->old.z;
		}
	}
}

BOOL __cdecl TestBoundsCollide(ITEM_INFO* item, ITEM_INFO* laraitem, int radius) {
	__int16 *boundItem;
	__int16 *boundLara;
	int x, z, s, c;
	int rx, rz;

	boundItem = GetBestFrame(item);
	boundLara = GetBestFrame(laraitem);

	if (item->pos.y + boundItem[3] > item->pos.y + boundLara[2] && item->pos.y + boundItem[2] < item->pos.y + boundLara[3]) {
		c = phd_cos(item->pos.rotY);
		s = phd_sin(item->pos.rotY);
		x = laraitem->pos.x - item->pos.x;
		z = laraitem->pos.z - item->pos.z;
		rx = (c * x - s * z) >> W2V_SHIFT;
		rz = (s * x + c * z) >> W2V_SHIFT;
		if (rx >= boundItem[0] - radius &&
			rx <= radius + boundItem[1] &&
			rz >= boundItem[4] - radius &&
			rz <= radius + boundItem[5]) {
			return TRUE;
		}
	}

	return FALSE;
}

BOOL __cdecl TestLaraPosition(__int16* bounds, ITEM_INFO* item, ITEM_INFO* laraitem) {
	int x, y, z;
	int xBound, yBound, zBound;
	short yRot, xRot, zRot;

	xRot = laraitem->pos.rotX - item->pos.rotX;
	yRot = laraitem->pos.rotY - item->pos.rotY;
	zRot = laraitem->pos.rotZ - item->pos.rotZ;

	if (xRot < bounds[6] || xRot > bounds[7] ||
		yRot < bounds[8] || yRot > bounds[9] ||
		zRot < bounds[10] || zRot > bounds[11]) {
		return 0;
	}

	x = laraitem->pos.x - item->pos.x;
	y = laraitem->pos.y - item->pos.y;
	z = laraitem->pos.z - item->pos.z;

	phd_PushUnitMatrix();
	phd_RotYXZ(item->pos.rotY, item->pos.rotX, item->pos.rotZ);
	xBound = (x * PhdMatrixPtr->_00 + y * PhdMatrixPtr->_10 + z * PhdMatrixPtr->_20) >> W2V_SHIFT;
	yBound = (x * PhdMatrixPtr->_01 + y * PhdMatrixPtr->_11 + z * PhdMatrixPtr->_21) >> W2V_SHIFT;
	zBound = (x * PhdMatrixPtr->_02 + y * PhdMatrixPtr->_12 + z * PhdMatrixPtr->_22) >> W2V_SHIFT;
	phd_PopMatrix();

	return xBound >= bounds[0]
		&& xBound <= bounds[1]
		&& yBound >= bounds[2]
		&& yBound <= bounds[3]
		&& zBound >= bounds[4]
		&& zBound <= bounds[5];
}

void __cdecl AlignLaraPosition(PHD_VECTOR* pos, ITEM_INFO* item, ITEM_INFO* laraitem) {
	FLOOR_INFO* floor;
	int x, y, z;
	int height, ceiling;
	__int16 roomID;

	laraitem->pos.rotX = item->pos.rotX;
	laraitem->pos.rotY = item->pos.rotY;
	laraitem->pos.rotZ = item->pos.rotZ;

	phd_PushUnitMatrix();
	phd_RotYXZ(item->pos.rotY, item->pos.rotX, item->pos.rotZ);
	x = item->pos.x + ((pos->x * PhdMatrixPtr->_00 + pos->y * PhdMatrixPtr->_01 + pos->z * PhdMatrixPtr->_02) >> W2V_SHIFT);
	y = item->pos.y + ((pos->x * PhdMatrixPtr->_10 + pos->y * PhdMatrixPtr->_11 + pos->z * PhdMatrixPtr->_12) >> W2V_SHIFT);
	z = item->pos.z + ((pos->x * PhdMatrixPtr->_20 + pos->y * PhdMatrixPtr->_21 + pos->z * PhdMatrixPtr->_22) >> W2V_SHIFT);
	phd_PopMatrix();

	roomID = laraitem->roomNumber;
	floor = GetFloor(x, y, z, &roomID);
	height = GetHeight(floor, x, y, z);
	ceiling = GetCeiling(floor, x, y, z);

	if (ABS(height - laraitem->pos.y) <= 256 && ABS(ceiling - laraitem->pos.y) >= 762) {
		laraitem->pos.x = x;
		laraitem->pos.y = y;
		laraitem->pos.z = z;
	}
}

BOOL __cdecl MoveLaraPosition(PHD_VECTOR* pos, ITEM_INFO* item, ITEM_INFO* laraitem) {
	PHD_3DPOS newpos;
	FLOOR_INFO* floor;
	int height, distance;
	int xDist, yDist, zDist;
	__int16 roomID;

	newpos.rotX = item->pos.rotX;
	newpos.rotY = item->pos.rotY;
	newpos.rotZ = item->pos.rotZ;

	phd_PushUnitMatrix();
	phd_RotYXZ(item->pos.rotY, item->pos.rotX, item->pos.rotZ);
	newpos.x = item->pos.x + ((pos->x * PhdMatrixPtr->_00 + pos->y * PhdMatrixPtr->_01 + pos->z * PhdMatrixPtr->_02) >> W2V_SHIFT);
	newpos.y = item->pos.y + ((pos->x * PhdMatrixPtr->_10 + pos->y * PhdMatrixPtr->_11 + pos->z * PhdMatrixPtr->_12) >> W2V_SHIFT);
	newpos.z = item->pos.z + ((pos->x * PhdMatrixPtr->_20 + pos->y * PhdMatrixPtr->_21 + pos->z * PhdMatrixPtr->_22) >> W2V_SHIFT);
	phd_PopMatrix();

	if (item->objectID != ID_FLARE_ITEM) {
		return Move3DPosTo3DPos(&laraitem->pos, &newpos, 16, 2 * PHD_DEGREE);
	}

	roomID = laraitem->roomNumber;
	floor = GetFloor(newpos.x, newpos.y, newpos.z, &roomID);
	height = GetHeight(floor, newpos.x, newpos.y, newpos.z);

	if (ABS(height - laraitem->pos.y) > 512) {
		return FALSE;
	}

	zDist = SQR(newpos.z - laraitem->pos.z);
	yDist = SQR(newpos.y - laraitem->pos.y);
	xDist = SQR(newpos.x - laraitem->pos.z);
	distance = phd_sqrt(xDist + yDist + zDist);
	return distance < 128 || Move3DPosTo3DPos(&laraitem->pos, &newpos, 16, 2 * PHD_DEGREE);
}

BOOL __cdecl Move3DPosTo3DPos(PHD_3DPOS* src, PHD_3DPOS* dest, int velocity, __int16 angleAdder) {
	int x, y, z, distance;
	__int16 xRot, yRot, zRot;

	x = dest->x - src->x;
	y = dest->y - src->y;
	z = dest->z - src->z;
	distance = phd_sqrt(SQR(z) + SQR(y) + SQR(x));
	if (velocity < distance) {
		src->x += velocity * x / distance;
		src->y += velocity * y / distance;
		src->z += velocity * z / distance;
	} else {
		src->x = dest->x;
		src->y = dest->y;
		src->z = dest->z;
	}

	xRot = dest->rotX - src->rotX;
	yRot = dest->rotY - src->rotY;
	zRot = dest->rotZ - src->rotZ;
	if (xRot <= angleAdder) {
		if (xRot >= -angleAdder) {
			src->rotX = dest->rotX;
		} else {
			src->rotX -= angleAdder;
		}
	} else {
		src->rotX += angleAdder;
	}
	if (yRot <= angleAdder) {
		if (yRot >= -angleAdder) {
			src->rotY = dest->rotY;
		} else {
			src->rotY -= angleAdder;
		}
	} else {
		src->rotY += angleAdder;
	}
	if (zRot <= angleAdder) {
		if (zRot >= -angleAdder) {
			src->rotZ = dest->rotZ;
		} else {
			src->rotZ -= angleAdder;
		}
	} else {
		src->rotZ += angleAdder;
	}
	return src->x == dest->x
		&& src->y == dest->y
		&& src->z == dest->z
		&& src->rotX == dest->rotX
		&& src->rotY == dest->rotY
		&& src->rotZ == dest->rotZ;
}

/*
 * Inject function
 */
void Inject_Collide() {
//	INJECT(0x004128D0, GetCollisionInfo);
	INJECT(0x00412F90, FindGridShift);
	INJECT(0x00412FC0, CollideStaticObjects);
	INJECT(0x004133B0, GetNearByRooms);
	INJECT(0x00413480, GetNewRoom);
	INJECT(0x004134E0, ShiftItem);
	INJECT(0x00413520, UpdateLaraRoom);
	INJECT(0x00413580, GetTiltType);
	INJECT(0x00413620, LaraBaddieCollision);
	INJECT(0x004137C0, EffectSpaz);
	INJECT(0x00413840, CreatureCollision);
	INJECT(0x004138C0, ObjectCollision);
	INJECT(0x00413920, DoorCollision);
	INJECT(0x004139A0, TrapCollision);
	INJECT(0x00413A10, ItemPushLara);
	INJECT(0x00413D20, TestBoundsCollide);
	INJECT(0x00413DF0, TestLaraPosition);
	INJECT(0x00413F30, AlignLaraPosition);
	INJECT(0x00414070, MoveLaraPosition);
	INJECT(0x00414200, Move3DPosTo3DPos);
}
