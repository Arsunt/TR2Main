/*
 * Copyright (c) 2017-2021 Michael Chaban. All rights reserved.
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
#include "game/collide.h"
#include "3dsystem/phd_math.h"
#include "game/control.h"
#include "global/vars.h"

void __cdecl GetCollisionInfo(COLL_INFO *coll, int x, int y, int z, __int16 roomID, int height) {
	int head, top, h, c, frontX, frontZ, leftX, leftZ, rightX, rightZ;
	FLOOR_INFO *floor;
	__int16 tilt;

	coll->collType = 0;
	coll->shift.z = 0;
	coll->shift.y = 0;
	coll->shift.x = 0;
	head = y - height;
	top = head - 160;
	coll->quadrant = (UINT16) (coll->facing + PHD_45) >> W2V_SHIFT;
	floor = GetFloor(x, top, z, &roomID);
	h = GetHeight(floor, x, top, z);
	if (h != NO_HEIGHT)
		h -= y;
	c = GetCeiling(floor, x, top, z);
	if (c != NO_HEIGHT)
		c -= head;
	coll->sideMid.floor = h;
	coll->sideMid.ceiling = c;
	coll->sideMid.type = HeightType;
	coll->trigger = TriggerPtr;
	tilt = GetTiltType(floor, x, LaraItem->pos.y, z);
	coll->xTilt = tilt;
	coll->zTilt = tilt >> 8;
	switch (coll->quadrant) {
		case 0:
			frontX = coll->radius * phd_sin(coll->facing) >> W2V_SHIFT;
			frontZ = coll->radius;
			leftX = -coll->radius;
			leftZ = coll->radius;
			rightX = coll->radius;
			rightZ = coll->radius;
			break;
		case 1:
			frontX = coll->radius;
			frontZ = coll->radius * phd_cos(coll->facing) >> W2V_SHIFT;
			leftX = coll->radius;
			leftZ = coll->radius;
			rightX = coll->radius;
			rightZ = -coll->radius;
			break;
		case 2:
			frontX = coll->radius * phd_sin(coll->facing) >> W2V_SHIFT;
			frontZ = -coll->radius;
			leftX = coll->radius;
			leftZ = -coll->radius;
			rightX = -coll->radius;
			rightZ = -coll->radius;
			break;
		case 3:
			frontX = -coll->radius;
			frontZ = coll->radius * phd_cos(coll->facing) >> W2V_SHIFT;
			rightZ = coll->radius;
			leftX = -coll->radius;
			leftZ = -coll->radius;
			rightX = -coll->radius;
			break;
		default:
			frontZ = 0;
			frontX = 0;
			leftZ = 0;
			leftX = 0;
			rightZ = 0;
			rightX = 0;
			break;
	}
	floor = GetFloor(x + frontX, top, z + frontZ, &roomID);
	h = GetHeight(floor, x + frontX, top, z + frontZ);
	if (h != NO_HEIGHT)
		h -= y;
	c = GetCeiling(floor, x + frontX, top, z + frontZ);
	if (c != NO_HEIGHT)
		c -= head;
	coll->sideFront.floor = h;
	coll->sideFront.ceiling = c;
	coll->sideFront.type = HeightType;
	if (CHK_ANY(coll->flags, 1) && coll->sideFront.type == HT_BIG_SLOPE && coll->sideFront.floor < 0) {
		coll->sideFront.floor = -32767;
	} else {
		if ((CHK_ANY(coll->flags, 2) &&
			coll->sideFront.type == HT_BIG_SLOPE &&
			coll->sideFront.floor > 0) ||
			(CHK_ANY(coll->flags, 4) &&
			coll->sideFront.floor > 0 &&
			TriggerPtr &&
			(*TriggerPtr & DATA_TYPE) == FT_LAVA))
			coll->sideFront.floor = 512;
	}
	floor = GetFloor(x + leftX, top, z + leftZ, &roomID);
	h = GetHeight(floor, x + leftX, top, z + leftZ);
	if (h != NO_HEIGHT)
		h -= y;
	c = GetCeiling(floor, x + leftX, top, z + leftZ);
	if (c != NO_HEIGHT)
		c -= head;
	coll->sideLeft.ceiling = c;
	coll->sideLeft.floor = h;
	coll->sideLeft.type = HeightType;
	if (CHK_ANY(coll->flags, 1) && coll->sideLeft.type == HT_BIG_SLOPE && coll->sideLeft.floor < 0) {
		coll->sideLeft.floor = -32767;
	} else {
		if ((CHK_ANY(coll->flags, 2) &&
			coll->sideLeft.type == HT_BIG_SLOPE &&
			coll->sideLeft.floor > 0) ||
			(CHK_ANY(coll->flags, 4) &&
			coll->sideLeft.floor > 0 &&
			TriggerPtr &&
			(*TriggerPtr & DATA_TYPE) == FT_LAVA))
			coll->sideLeft.floor = 512;
	}
	floor = GetFloor(x + rightX, top, z + rightZ, &roomID);
	h = GetHeight(floor, x + rightX, top, z + rightZ);
	if (h != NO_HEIGHT)
		h -= y;
	c = GetCeiling(floor, x + rightX, top, z + rightZ);
	if (c != NO_HEIGHT)
		c -= head;
	coll->sideRight.ceiling = c;
	coll->sideRight.floor = h;
	coll->sideRight.type = HeightType;
	if (CHK_ANY(coll->flags, 1) && coll->sideRight.type == HT_BIG_SLOPE && coll->sideRight.floor < 0) {
		coll->sideRight.floor = -32767;
	} else {
		if ((CHK_ANY(coll->flags, 2) &&
			coll->sideRight.type == HT_BIG_SLOPE &&
			coll->sideRight.floor > 0) ||
			(CHK_ANY(coll->flags, 4) &&
			coll->sideRight.floor > 0 &&
			TriggerPtr &&
			(*TriggerPtr & DATA_TYPE) == FT_LAVA))
			coll->sideRight.floor = 512;
	}
	if (CollideStaticObjects(coll, x, y, z, roomID, height)) {
		floor = GetFloor(x + coll->shift.x, y, z + coll->shift.z, &roomID);
		if (GetHeight(floor, x + coll->shift.x, y, z + coll->shift.z) < y - 512 ||
			GetCeiling(floor, x + coll->shift.x, y, z + coll->shift.z) > head)
		{
			coll->shift.x = -coll->shift.x;
			coll->shift.z = -coll->shift.z;
		}
	}
	if (coll->sideMid.floor == NO_HEIGHT) {
		coll->shift.y = coll->old.y - y;
		coll->shift.x = coll->old.x - x;
		coll->shift.z = coll->old.z - z;
		coll->collType = COLL_FRONT;
	} else {
		if (coll->sideMid.floor - coll->sideMid.ceiling <= 0) {
			coll->shift.y = coll->old.y - y;
			coll->shift.x = coll->old.x - x;
			coll->shift.z = coll->old.z - z;
			coll->collType = COLL_CLAMP;
		} else {
			if (coll->sideMid.ceiling >= 0) {
				coll->shift.y = coll->sideMid.ceiling;
				coll->collType = COLL_TOP;
			}
			if (coll->sideFront.floor <= coll->badPos &&
				coll->sideFront.floor >= coll->badNeg &&
				coll->sideFront.ceiling <= coll->badCeiling)
			{
				if (coll->sideFront.ceiling >= coll->badCeiling)
				{
					coll->shift.y = coll->old.y - y;
					coll->shift.x = coll->old.x - x;
					coll->shift.z = coll->old.z - z;
					coll->collType = COLL_TOPFRONT;
				} else {
					if (coll->sideLeft.floor <= coll->badPos && coll->sideLeft.floor >= coll->badNeg) {
						if (coll->sideRight.floor > coll->badPos || coll->sideRight.floor < coll->badNeg) {
							switch (coll->quadrant) {
								case 0:
								case 2:
									coll->shift.x = FindGridShift(x + rightX, x + frontX);
									break;
								case 1:
								case 3:
									coll->shift.z = FindGridShift(z + rightZ, z + frontZ);
									break;
							}
							coll->collType = COLL_RIGHT;
						}
					} else {
						switch (coll->quadrant) {
							case 0:
							case 2:
								coll->shift.x = FindGridShift(x + leftX, x + frontX);
								break;
							case 1:
							case 3:
								coll->shift.z = FindGridShift(z + leftZ, z + frontZ);
								break;
						}
						coll->collType = COLL_LEFT;
					}
				}
			} else {
				switch (coll->quadrant) {
					case 0:
					case 2:
						coll->shift.x = coll->old.x - x;
						coll->shift.z = FindGridShift(z + frontZ, z);
						break;
					case 1:
					case 3:
						coll->shift.x = FindGridShift(x + frontX, x);
						coll->shift.z = coll->old.z - z;
						break;
				}
				coll->collType = COLL_FRONT;
			}
		}
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

/*
 * Inject function
 */
void Inject_Collide() {
	INJECT(0x004128D0, GetCollisionInfo);

//	INJECT(0x00412F90, FindGridShift);

	INJECT(0x00412FC0, CollideStaticObjects);
	INJECT(0x004133B0, GetNearByRooms);
	INJECT(0x00413480, GetNewRoom);

//	INJECT(0x004134E0, ShiftItem);
//	INJECT(0x00413520, UpdateLaraRoom);
//	INJECT(0x00413580, GetTiltType);
//	INJECT(0x00413620, LaraBaddieCollision);
//	INJECT(0x004137C0, EffectSpaz);
//	INJECT(0x00413840, CreatureCollision);
//	INJECT(0x004138C0, ObjectCollision);
//	INJECT(0x00413920, DoorCollision);
//	INJECT(0x004139A0, TrapCollision);
//	INJECT(0x00413A10, ItemPushLara);
//	INJECT(0x00413D20, TestBoundsCollide);
//	INJECT(0x00413DF0, TestLaraPosition);
//	INJECT(0x00413F30, AlignLaraPosition);
//	INJECT(0x00414070, MoveLaraPosition);
//	INJECT(0x00414200, Move3DPosTo3DPos);
}
