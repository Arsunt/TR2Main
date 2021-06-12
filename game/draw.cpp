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
#include "game/draw.h"
#include "3dsystem/3d_gen.h"
#include "3dsystem/scalespr.h"
#include "game/hair.h"
#include "specific/game.h"
#include "specific/output.h"
#include "global/vars.h"

#ifdef FEATURE_EXTENDED_LIMITS
LIGHT_INFO DynamicLights[64];
int BoundRooms[1024];
__int16 DrawRoomsArray[1024];
STATIC_INFO StaticObjects[256];
#endif // FEATURE_EXTENDED_LIMITS

#ifdef FEATURE_VIDEOFX_IMPROVED
extern DWORD AlphaBlendMode;
static int GoldenLaraAlpha = 0;

void ResetGoldenLaraAlpha() {
	GoldenLaraAlpha = ( Lara.water_status == LWS_Cheat ) ? 0xFF : 0;
}
#endif // FEATURE_VIDEOFX_IMPROVED

void __cdecl DrawRooms(__int16 currentRoom) {
	ROOM_INFO *room = &RoomInfo[currentRoom];

	PhdWinLeft = room->left = 0;
	PhdWinTop = room->top = 0;
	PhdWinRight = room->right = PhdWinMaxX;
	PhdWinBottom = room->bottom = PhdWinMaxY;

	room->boundActive = 2;
	BoundRooms[0] = currentRoom;
	BoundStart = 0;
	BoundEnd = 1;
	DrawRoomsCount = 0;
	OutsideCamera = room->flags & ROOM_OUTSIDE;

	if( OutsideCamera ) {
		OutsideLeft = 0;
		OutsideTop = 0;
		OutsideRight = PhdWinMaxX;
		OutsideBottom = PhdWinMaxY;
	} else {
		OutsideLeft = PhdWinMaxX;
		OutsideTop = PhdWinMaxY;
		OutsideRight = 0;
		OutsideBottom = 0;
	}

	UnderwaterCamera = room->flags & ROOM_UNDERWATER;
	GetRoomBounds();
	MidSort = 0;

	if( OutsideCamera ) {
		PhdWinLeft = OutsideLeft;
		PhdWinRight = OutsideRight;
		PhdWinBottom = OutsideBottom;
		PhdWinTop = OutsideTop;
		if( Objects[ID_SKYBOX].loaded ) {
			// Draw skybox background
			S_SetupAboveWater(UnderwaterCamera);
			phd_PushMatrix();
			PhdMatrixPtr->_03 = PhdMatrixPtr->_13 = PhdMatrixPtr->_23 = 0;

			UINT16 *ptr = (UINT16 *)&Anims[Objects[ID_SKYBOX].animIndex].framePtr[9];
			phd_RotYXZsuperpack(&ptr, 0);
#ifdef FEATURE_VIEW_IMPROVED
			S_InitialisePolyList(1); // Fill backbuffer with black
#else // !FEATURE_VIEW_IMPROVED
			S_InitialisePolyList(0); // Leave backbuffer uncleaned
#endif // FEATURE_VIEW_IMPROVED
			S_InsertBackground(MeshPtr[Objects[ID_SKYBOX].meshIndex]);
			phd_PopMatrix();
		} else {
			S_InitialisePolyList(1); // Fill backbuffer with black
			OutsideCamera = -1;
		}
	} else {
#ifdef FEATURE_VIEW_IMPROVED
		S_InitialisePolyList(1); // Fill backbuffer with black
#else // !FEATURE_VIEW_IMPROVED
		S_InitialisePolyList(0); // Leave backbuffer uncleaned
#endif // FEATURE_VIEW_IMPROVED
	}

	// Draw Lara
	if( Objects[ID_LARA].loaded && !(LaraItem->flags & IFL_INVISIBLE) ) {
		if( RoomInfo[LaraItem->roomNumber].flags & ROOM_UNDERWATER ) {
			S_SetupBelowWater(UnderwaterCamera);
		} else {
			S_SetupAboveWater(UnderwaterCamera);
		}
		MidSort = RoomInfo[LaraItem->roomNumber].boundActive >> 8;
		if( MidSort ) --MidSort;
#if defined(FEATURE_VIDEOFX_IMPROVED)
		if( Lara.mesh_effects ) {
			if( GoldenLaraAlpha < 0xFF ) GoldenLaraAlpha += 8;
			CLAMPG(GoldenLaraAlpha, 0xFF);
		} else {
			if( GoldenLaraAlpha > 0 ) GoldenLaraAlpha -= 8;
			CLAMPL(GoldenLaraAlpha, 0);
		}
		if( GoldenLaraAlpha ) {
			// NOTE: this is dirty trick for Golden Lara while Dozy cheat mode.
			// In TR1 there is bit mask in mesh_effects for distinct meshes,
			// but whole Lara must be golden here, including braid and weapons.
			SetMeshReflectState(ID_NONE, RGBA_MAKE(0xFF,0xC0,0x40,GoldenLaraAlpha));
		}
#endif // FEATURE_VIDEOFX_IMPROVED
		DrawLara(LaraItem);
#if defined(FEATURE_VIDEOFX_IMPROVED)
		ClearMeshReflectState();
#endif // FEATURE_VIDEOFX_IMPROVED
	}

	// Draw rooms
	for( int i = 0; i < DrawRoomsCount; ++i ) {
		PrintRooms(DrawRoomsArray[i]);
	}

	// Draw movable and static objects
	for( int i = 0; i < DrawRoomsCount; ++i ) {
		PrintObjects(DrawRoomsArray[i]);
	}
#ifdef FEATURE_VIEW_IMPROVED
	for( int i = 0; i < DrawRoomsCount; ++i ) {
		RoomInfo[DrawRoomsArray[i]].boundActive = 0;
	}
	MidSort = 0;
#endif // FEATURE_VIEW_IMPROVED
}

void __cdecl GetRoomBounds() {
	while( BoundStart != BoundEnd ) {
		int roomNumber = BoundRooms[BoundStart++ % ARRAY_SIZE(BoundRooms)];
		ROOM_INFO *room = &RoomInfo[roomNumber];
		room->boundActive &= ~2;
		MidSort = (room->boundActive >> 8) + 1;

		CLAMPG(room->boundLeft, room->left)
		CLAMPG(room->boundTop, room->top)
		CLAMPL(room->boundRight, room->right)
		CLAMPL(room->boundBottom, room->bottom)

		if( !CHK_ANY(room->boundActive, 1) ) {
			DrawRoomsArray[DrawRoomsCount++] = roomNumber;
			room->boundActive |= 1;
			if( CHK_ANY(room->flags, ROOM_OUTSIDE) ) {
				OutsideCamera = ROOM_OUTSIDE;
			}
		}

		// NOTE: The original game checks just ROOM_INSIDE flag here
		if( CHK_ANY(room->flags, ROOM_OUTSIDE) || !CHK_ANY(room->flags, ROOM_INSIDE) ) {
			CLAMPG(OutsideLeft, room->boundLeft)
			CLAMPG(OutsideTop, room->boundTop)
			CLAMPL(OutsideRight, room->boundRight)
			CLAMPL(OutsideBottom, room->boundBottom)
		}

		if( !room->doors ) continue;
		phd_PushMatrix();
		phd_TranslateAbs(room->x, room->y, room->z);

		for( int i = 0; i < room->doors->wCount; ++i ) {
			DOOR_INFO *door = &room->doors->door[i];
			if( door->x * (room->x + door->vertex[0].x - MatrixW2V._03)
				+ door->y * (room->y + door->vertex[0].y - MatrixW2V._13)
				+ door->z * (room->z + door->vertex[0].z - MatrixW2V._23) < 0 )
			{
				SetRoomBounds((__int16 *)&door->x, door->room, room);
			}
		}
		phd_PopMatrix();
	}
}

void __cdecl SetRoomBounds(__int16 *ptrObj, int roomNumber, ROOM_INFO *parent) {
	ROOM_INFO *room = &RoomInfo[roomNumber];
	if( room->boundLeft <= parent->left
		&& room->boundRight >= parent->right
		&& room->boundTop <= parent->top
		&& room->boundBottom >= parent->bottom )
	{
		return;
	}

	PHD_VECTOR view[4];
	int left = parent->right;
	int right = parent->left;
	int top = parent->bottom;
	int bottom = parent->top;
	int tooFar = 0;
	int tooNear = 0;

	for( int i=0; i<4; ++i ) {
		ptrObj += 3;
		PHD_MATRIX *m = PhdMatrixPtr;
		int x = view[i].x = ptrObj[0] * m->_00 + ptrObj[1] * m->_01 + ptrObj[2] * m->_02 + m->_03;
		int y = view[i].y = ptrObj[0] * m->_10 + ptrObj[1] * m->_11 + ptrObj[2] * m->_12 + m->_13;
		int z = view[i].z = ptrObj[0] * m->_20 + ptrObj[1] * m->_21 + ptrObj[2] * m->_22 + m->_23;
		if( z <= 0 ) {
			++tooNear;
			continue;
		}
		if( z > PhdFarZ ) {
			++tooFar;
		}
		z /= PhdPersp;
		if( z ) {
			x = PhdWinCenterX + x / z;
			y = PhdWinCenterY + y / z;
		} else {
			x = ( x < 0 ) ? PhdWinLeft : PhdWinRight;
			y = ( y < 0 ) ? PhdWinTop : PhdWinBottom;
		}
		CLAMPG(left, x - 1);
		CLAMPG(top, y - 1);
		CLAMPL(right, x + 1);
		CLAMPL(bottom, y + 1);
	}

	if( tooNear == 4 || tooFar == 4 ) {
		return;
	}

	if( tooNear > 0 ) {
		for( int i=0; i<4; ++i ) {
			int j=(i+3)%4;
			if( (view[i].z < 0) == (view[j].z < 0) ) {
				continue;
			}
			if( view[i].x <= 0 || view[j].x <= 0 ) {
				left = 0;
			}
			if( view[i].x >= 0 || view[j].x >= 0 ) {
				right = PhdWinMaxX;
			}
			if( view[i].y <= 0 || view[j].y <= 0 ) {
				top = 0;
			}
			if( view[i].y >= 0 || view[j].y >= 0 ) {
				bottom = PhdWinMaxY;
			}
		}
	}

	CLAMPL(left, parent->left);
	CLAMPL(top, parent->top);
	CLAMPG(right, parent->right);
	CLAMPG(bottom, parent->bottom);

	if( left >= right || top >= bottom ) {
		return;
	}

	if( CHK_ANY(room->boundActive, 2) ) {
		CLAMPG(room->left, left);
		CLAMPG(room->top, top);
		CLAMPL(room->right, right);
		CLAMPL(room->bottom, bottom);
	} else {
		BoundRooms[BoundEnd++ % ARRAY_SIZE(BoundRooms)] = roomNumber;
		room->boundActive |= 2;
#ifdef FEATURE_VIEW_IMPROVED
		if( !CHK_ANY(room->boundActive, 1) ) {
			room->boundActive += MidSort << 8;
		}
#else // FEATURE_VIEW_IMPROVED
		room->boundActive += MidSort << 8;
#endif // FEATURE_VIEW_IMPROVED
		room->left = left;
		room->right = right;
		room->top = top;
		room->bottom = bottom;
	}
}

void __cdecl ClipRoom(ROOM_INFO *room) {
	static const int p[12][2] = {
		{0, 1}, {1, 2}, {2, 3}, {3, 0},
		{4, 5}, {5, 6}, {6, 7}, {7, 4},
		{0, 4}, {1, 5}, {2, 6}, {3, 7},
	};
	int xv[8], yv[8], zv[8], clip[8];
	int xMin, yMin, xMax, yMax;
	int clipRoom = 0;

	xv[0] = xv[3] = xv[4] = xv[7] = 0x400;
	yv[0] = yv[1] = yv[2] = yv[3] = room->maxCeiling - room->y;
	zv[0] = zv[1] = zv[4] = zv[5] = 0x400;

	xv[1] = xv[2] = xv[5] = xv[6] = (room->ySize - 1) * 0x400;
	yv[4] = yv[5] = yv[6] = yv[7] = room->minFloor - room->y;
	zv[2] = zv[3] = zv[6] = zv[7] = (room->xSize - 1) * 0x400;

	for( int i=0; i<8; ++i ) {
		PHD_MATRIX *m = PhdMatrixPtr;
		int x = xv[i];
		int y = yv[i];
		int z = zv[i];
		xv[i] = (x * m->_00) + (y * m->_01) + (z * m->_02) + m->_03;
		yv[i] = (x * m->_10) + (y * m->_11) + (z * m->_12) + m->_13;
		zv[i] = (x * m->_20) + (y * m->_21) + (z * m->_22) + m->_23;
		if( zv[i] > PhdFarZ ) {
			clip[i] = 1;
			clipRoom = 1;
		} else {
			clip[i] = 0;
		}
	}
	if( !clipRoom ) {
		return;
	}

	xMin = yMin = 0x10000000;
	xMax = yMax = -0x10000000;
	for( int i=0; i<12; ++i ) {
		int p1 = p[i][0];
		int p2 = p[i][1];
		if( clip[p1] ^ clip[p2] ) {
			int zDiv = (zv[p2] - zv[p1]) >> W2V_SHIFT;
			if( zDiv ) {
				int zNom = (PhdFarZ - zv[p1]) >> W2V_SHIFT;
				int x = xv[p1] + ((((xv[p2] - xv[p1]) >> W2V_SHIFT) * zNom / zDiv) << W2V_SHIFT);
				int y = yv[p1] + ((((yv[p2] - yv[p1]) >> W2V_SHIFT) * zNom / zDiv) << W2V_SHIFT);
				CLAMPG(xMin, x);
				CLAMPG(yMin, y);
				CLAMPL(xMax, x);
				CLAMPL(yMax, y);
			} else {
				CLAMPG(xMin, xv[p1]);
				CLAMPG(xMin, xv[p2]);
				CLAMPG(yMin, yv[p1]);
				CLAMPG(yMin, yv[p2]);
				CLAMPL(xMax, xv[p1]);
				CLAMPL(xMax, xv[p2]);
				CLAMPL(yMax, yv[p1]);
				CLAMPL(yMax, yv[p2]);
			}
		}
	}
	xMin = PhdWinCenterX + xMin / (PhdFarZ / PhdPersp);
	yMin = PhdWinCenterY + yMin / (PhdFarZ / PhdPersp);
	xMax = PhdWinCenterX + xMax / (PhdFarZ / PhdPersp);
	yMax = PhdWinCenterY + yMax / (PhdFarZ / PhdPersp);
	if( xMin <= PhdWinRight && yMin <= PhdWinBottom && xMax >= PhdWinLeft && yMax >= PhdWinTop ) {
		CLAMPL(xMin, PhdWinLeft);
		CLAMPL(yMin, PhdWinTop);
		CLAMPG(xMax, PhdWinRight);
		CLAMPG(yMax, PhdWinBottom);
		S_InsertBackPolygon(xMin, yMin, xMax, yMax);
	}
}

void __cdecl PrintRooms(__int16 roomNumber) {
	ROOM_INFO *room = &RoomInfo[roomNumber];
#ifdef FEATURE_VIEW_IMPROVED
	if( CHK_ANY(room->boundActive, 4) ) {
		return;
	}
#endif // FEATURE_VIEW_IMPROVED
	if( CHK_ANY(room->flags, ROOM_UNDERWATER) ) {
		S_SetupBelowWater(UnderwaterCamera);
	} else {
		S_SetupAboveWater(UnderwaterCamera);
	}
	MidSort = room->boundActive >> 8;
	phd_TranslateAbs(room->x, room->y, room->z);
	PhdWinLeft = room->boundLeft;
	PhdWinRight = room->boundRight;
	PhdWinTop = room->boundTop;
	PhdWinBottom = room->boundBottom;
	S_LightRoom(room);
	if( OutsideCamera > 0 && !CHK_ANY(room->flags, ROOM_INSIDE) ) {
		S_InsertRoom(room->data, 1);
	} else {
		if( OutsideCamera >= 0 ) {
			ClipRoom(room);
		}
		S_InsertRoom(room->data, 0);
	}
#ifdef FEATURE_VIEW_IMPROVED
	room->boundActive |= 4;
#endif // FEATURE_VIEW_IMPROVED
}

void __cdecl PrintObjects(__int16 roomNumber) {
	ROOM_INFO *room = &RoomInfo[roomNumber];
	if( CHK_ANY(room->flags, ROOM_UNDERWATER) ) {
		S_SetupBelowWater(UnderwaterCamera);
	} else {
		S_SetupAboveWater(UnderwaterCamera);
	}

	MidSort = room->boundActive >> 8;
#ifndef FEATURE_VIEW_IMPROVED
	room->boundActive = 0;
#endif // FEATURE_VIEW_IMPROVED

	phd_PushMatrix();
	phd_TranslateAbs(room->x, room->y, room->z);
	PhdWinLeft = room->boundLeft;
	PhdWinTop = room->boundTop;
	PhdWinRight = room->boundRight;
	PhdWinBottom = room->boundBottom;

	MESH_INFO *mesh = room->mesh;
	for( int i = 0; i < room->numMeshes; ++i ) {
		if( !CHK_ANY(StaticObjects[mesh[i].staticNumber].flags, 2) ) {
			continue;
		}
		phd_PushMatrix();
		phd_TranslateAbs(mesh[i].x, mesh[i].y, mesh[i].z);
		phd_RotY(mesh[i].yRot);
		__int16 clip = S_GetObjectBounds((__int16 *)&StaticObjects[mesh[i].staticNumber].drawBounds);
		if( clip ) {
			S_CalculateStaticMeshLight(mesh[i].x, mesh[i].y, mesh[i].z, mesh[i].shade1, mesh[i].shade2, room);
#ifdef FEATURE_VIDEOFX_IMPROVED
			SetMeshReflectState(mesh[i].staticNumber, -1);
#endif // FEATURE_VIDEOFX_IMPROVED
			phd_PutPolygons(MeshPtr[StaticObjects[mesh[i].staticNumber].meshIndex], clip);
#ifdef FEATURE_VIDEOFX_IMPROVED
			ClearMeshReflectState();
#endif // FEATURE_VIDEOFX_IMPROVED
		}
		phd_PopMatrix();
	}

	PhdWinLeft = 0;
	PhdWinTop = 0;
	PhdWinRight = PhdWinMaxX + 1;
	PhdWinBottom = PhdWinMaxY + 1;

	for( __int16 id = room->itemNumber; id >= 0; id = Items[id].nextItem ) {
		if( Items[id].status != ITEM_INVISIBLE ) {
			Objects[Items[id].objectID].drawRoutine(&Items[id]);
		}
	}

	for( __int16 id = room->fxNumber; id >= 0; id = Effects[id].next_fx ) {
		DrawEffect(id);
	}

	phd_PopMatrix();
	room->boundLeft = PhdWinMaxX;
	room->boundTop = PhdWinMaxY;
	room->boundRight = 0;
	room->boundBottom = 0;
}

void __cdecl DrawEffect(__int16 fx_id) {
	FX_INFO *fx = &Effects[fx_id];
	OBJECT_INFO *obj = &Objects[fx->object_number];
	if( !obj->loaded ) return;
	if( fx->object_number == ID_GLOW ) {
		// NOTE: Core's hacky way to store the sprite flags in the rotation fields
		S_DrawSprite((fx->pos.rotY << 16)|(fx->pos.rotX), // flags
					fx->pos.x, fx->pos.y, fx->pos.z, // coordinates
					Objects[ID_GLOW].meshIndex, // sprite id
					fx->shade, fx->frame_number); // shade, scale
	} else if( obj->nMeshes < 0 ) {
		S_DrawSprite(SPR_ABS | SPR_SHADE | (obj->semi_transparent ? SPR_SEMITRANS : 0), // flags
					fx->pos.x, fx->pos.y, fx->pos.z, // coordinates
					obj->meshIndex - fx->frame_number, // sprite id
					fx->shade, 0);  // shade, scale
	} else {
		phd_PushMatrix();
		phd_TranslateAbs(fx->pos.x, fx->pos.y, fx->pos.z);
		if( PhdMatrixPtr->_23 > PhdNearZ && PhdMatrixPtr->_23 < PhdFarZ ) {
			__int16 *meshPtr = NULL;
			phd_RotYXZ(fx->pos.rotY, fx->pos.rotX, fx->pos.rotZ);
			S_CalculateStaticLight(fx->shade);
			if( obj->nMeshes ) {
				meshPtr = MeshPtr[obj->meshIndex];
			} else {
				meshPtr = MeshPtr[fx->frame_number];
			}
			phd_PutPolygons(meshPtr, -1);
		}
		phd_PopMatrix();
	}
}

void __cdecl DrawSpriteItem(ITEM_INFO *item) {
	OBJECT_INFO *obj;

	phd_PushUnitMatrix(); // NOTE: this push is workaround for sprites with no matrix
	S_CalculateStaticMeshLight(item->pos.x, item->pos.y, item->pos.z,
		item->shade1, item->shade2, &RoomInfo[item->roomNumber]);
	phd_PopMatrix(); // NOTE: this pop is workaround for sprites with no matrix

	obj = &Objects[item->objectID];

	// NOTE: SPR_ITEM is not presented in the original game
	S_DrawSprite(SPR_ITEM | SPR_ABS | SPR_SHADE | (obj->semi_transparent ? SPR_SEMITRANS : 0),
				 item->pos.x, item->pos.y, item->pos.z,
				 obj->meshIndex - item->frameNumber,
				 LsAdder + 0x1000, 0);
}

void __cdecl DrawDummyItem(ITEM_INFO *item) {
}

void __cdecl DrawAnimatingItem(ITEM_INFO *item) {
	static __int16 no_rotation[12] = {0};
	__int16 *frames[2] = {0};
	int rate = 0;
	DWORD bit = 1;
	int frac = GetFrames(item, frames, &rate);
	OBJECT_INFO *obj = &Objects[item->objectID];

	if( obj->shadowSize ) {
		S_PrintShadow(obj->shadowSize, frames[0], item);
	}

	phd_PushMatrix();
	phd_TranslateAbs(item->pos.x, item->pos.y, item->pos.z);
	phd_RotYXZ(item->pos.rotY, item->pos.rotX, item->pos.rotZ);
	int clip = S_GetObjectBounds(frames[0]);

	if( clip ) {
		CalculateObjectLighting(item, frames[0]);

		__int16 *rots = item->data ? (__int16 *)item->data : no_rotation;
		__int16 **meshPtr = &MeshPtr[obj->meshIndex];
		int *bonePtr = &AnimBones[obj->boneIndex];
		if( frac ) {
			InitInterpolate(frac, rate);
			phd_TranslateRel_ID(frames[0][6], frames[0][7], frames[0][8], frames[1][6], frames[1][7], frames[1][8]);
			UINT16 *rot1 = (UINT16 *)&frames[0][9];
			UINT16 *rot2 = (UINT16 *)&frames[1][9];
			phd_RotYXZsuperpack_I(&rot1, &rot2, 0);

			if( CHK_ANY(item->meshBits, 1) ) {
#ifdef FEATURE_VIDEOFX_IMPROVED
				SetMeshReflectState(item->objectID, 0);
#endif // FEATURE_VIDEOFX_IMPROVED
				phd_PutPolygons_I(meshPtr[0], clip);
#ifdef FEATURE_VIDEOFX_IMPROVED
				ClearMeshReflectState();
#endif // FEATURE_VIDEOFX_IMPROVED
			}

			for( int i = 1; i < obj->nMeshes; ++i ) {
				DWORD state = *bonePtr;
				if( CHK_ANY(state, 1) ) {
					phd_PopMatrix_I();
				}
				if( CHK_ANY(state, 2) ) {
					phd_PushMatrix_I();
				}
				phd_TranslateRel_I(bonePtr[1], bonePtr[2], bonePtr[3]);
				phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
				if( CHK_ANY(state, 0x1C) ) {
					if( CHK_ANY(state, 0x08) ) {
						phd_RotY_I(*(rots++));
					}
					if( CHK_ANY(state, 0x04) ) {
						phd_RotX_I(*(rots++));
					}
					if( CHK_ANY(state, 0x10) ) {
						phd_RotZ_I(*(rots++));
					}
				}
				bonePtr+=4;
				bit <<= 1;
				if( CHK_ANY(item->meshBits, bit) ) {
#ifdef FEATURE_VIDEOFX_IMPROVED
					SetMeshReflectState(item->objectID, i);
#endif // FEATURE_VIDEOFX_IMPROVED
					phd_PutPolygons_I(meshPtr[i], clip);
#ifdef FEATURE_VIDEOFX_IMPROVED
					ClearMeshReflectState();
#endif // FEATURE_VIDEOFX_IMPROVED
				}
			}
		} else {
			phd_TranslateRel(frames[0][6], frames[0][7], frames[0][8]);
			UINT16 *rot = (UINT16 *)&frames[0][9];
			phd_RotYXZsuperpack(&rot, 0);

			if( CHK_ANY(item->meshBits, 1) ) {
#ifdef FEATURE_VIDEOFX_IMPROVED
				SetMeshReflectState(item->objectID, 0);
#endif // FEATURE_VIDEOFX_IMPROVED
				phd_PutPolygons(meshPtr[0], clip);
#ifdef FEATURE_VIDEOFX_IMPROVED
				ClearMeshReflectState();
#endif // FEATURE_VIDEOFX_IMPROVED
			}

			for( int i = 1; i < obj->nMeshes; ++i) {
				DWORD state = *bonePtr;
				if( CHK_ANY(state, 1) ) {
					phd_PopMatrix();
				}
				if( CHK_ANY(state, 2) ) {
					phd_PushMatrix();
				}
				phd_TranslateRel(bonePtr[1], bonePtr[2], bonePtr[3]);
				phd_RotYXZsuperpack(&rot, 0);
				if( CHK_ANY(state, 0x1C) ) {
					if( CHK_ANY(state, 0x08) ) {
						phd_RotY(*(rots++));
					}
					if( CHK_ANY(state, 0x04) ) {
						phd_RotX(*(rots++));
					}
					if( CHK_ANY(state, 0x10) ) {
						phd_RotZ(*(rots++));
					}
				}
				bonePtr += 4;
				bit <<= 1;
				if( CHK_ANY(item->meshBits, bit) ) {
#ifdef FEATURE_VIDEOFX_IMPROVED
					SetMeshReflectState(item->objectID, i);
#endif // FEATURE_VIDEOFX_IMPROVED
					phd_PutPolygons(meshPtr[i], clip);
#ifdef FEATURE_VIDEOFX_IMPROVED
					ClearMeshReflectState();
#endif // FEATURE_VIDEOFX_IMPROVED
				}
			}
		}
	}
	phd_PopMatrix();
}

void __cdecl DrawLaraInt(ITEM_INFO *item, __int16 *frame1, __int16 *frame2, int frac, int rate) {
	PHD_MATRIX matrix;
	UINT16 *rot1, *rot2, *rot1copy, *rot2copy;
	int frame, *bones;

	OBJECT_INFO *obj = &Objects[item->objectID];
	__int16 *bounds = GetBoundsAccurate(item);
	if( Lara.skidoo == -1 ) {
		S_PrintShadow(obj->shadowSize, bounds, item);
	}
	matrix = *PhdMatrixPtr;
	phd_PushMatrix();
	phd_TranslateAbs(item->pos.x, item->pos.y, item->pos.z);
	phd_RotYXZ(item->pos.rotY, item->pos.rotX, item->pos.rotZ);

	int clip = S_GetObjectBounds(frame1);
	if( !clip ) {
		phd_PopMatrix();
		return;
	}

	phd_PushMatrix();
	CalculateObjectLighting(item, frame1);
	bones = &AnimBones[obj->boneIndex];

	rot1 = (UINT16 *)frame1 + 9;
	rot2 = (UINT16 *)frame2 + 9;

	InitInterpolate(frac, rate);
	phd_TranslateRel_ID(frame1[6], frame1[7], frame1[8], frame2[6], frame2[7], frame2[8]);
	phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
	phd_PutPolygons_I(Lara.mesh_ptrs[0], clip);

	phd_PushMatrix_I();
	phd_TranslateRel_I(bones[1], bones[2], bones[3]);
	phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
	phd_PutPolygons_I(Lara.mesh_ptrs[1], clip);
	phd_TranslateRel_I(bones[5], bones[6], bones[7]);
	phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
	phd_PutPolygons_I(Lara.mesh_ptrs[2], clip);
	phd_TranslateRel_I(bones[9], bones[10], bones[11]);
	phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
	phd_PutPolygons_I(Lara.mesh_ptrs[3], clip);
	phd_PopMatrix_I();

	phd_PushMatrix_I();
	phd_TranslateRel_I(bones[13], bones[14], bones[15]);
	phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
	phd_PutPolygons_I(Lara.mesh_ptrs[4], clip);
	phd_TranslateRel_I(bones[17], bones[18], bones[19]);
	phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
	phd_PutPolygons_I(Lara.mesh_ptrs[5], clip);
	phd_TranslateRel_I(bones[21], bones[22], bones[23]);
	phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
	phd_PutPolygons_I(Lara.mesh_ptrs[6], clip);
	phd_PopMatrix_I();

	phd_TranslateRel_I(bones[25], bones[26], bones[27]);
	if ( Lara.weapon_item != -1 && Lara.gun_type == LGT_M16
		&& (Items[Lara.weapon_item].currentAnimState == 0
		|| Items[Lara.weapon_item].currentAnimState == 2
		|| Items[Lara.weapon_item].currentAnimState == 4) )
	{
		frame = Lara.right_arm.frame_number * (Anims[Lara.right_arm.anim_number].interpolation >> 8) + 9;
		rot1 = rot2 = (UINT16 *)&Lara.right_arm.frame_base[frame];
		phd_RotYXZsuperpack_I(&rot1, &rot2, 7);
	} else {
		phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
	}
	phd_RotYXZ_I(Lara.torso_y_rot, Lara.torso_x_rot, Lara.torso_z_rot);
	phd_PutPolygons_I(Lara.mesh_ptrs[7], clip);

	phd_PushMatrix_I();
	phd_TranslateRel_I(bones[53], bones[54], bones[55]);
	rot1copy = rot1;
	rot2copy = rot2;
	phd_RotYXZsuperpack_I(&rot1, &rot2, 6);
	rot1 = rot1copy;
	rot2 = rot2copy;
	phd_RotYXZ_I(Lara.head_y_rot, Lara.head_x_rot, Lara.head_z_rot);
	phd_PutPolygons_I(Lara.mesh_ptrs[14], clip);
	*PhdMatrixPtr = matrix;
	DrawHair();
	phd_PopMatrix_I();

	if( Lara.back_gun ) {
		phd_PushMatrix_I();
		int *bone = &AnimBones[Objects[Lara.back_gun].boneIndex];
		phd_TranslateRel_I(bone[53], bone[54], bone[55]);
		rot2copy = (UINT16 *)Objects[Lara.back_gun].frameBase + 9;
		rot1copy = (UINT16 *)Objects[Lara.back_gun].frameBase + 9;
		phd_RotYXZsuperpack_I(&rot1copy, &rot2copy, 14);
		phd_PutPolygons_I(MeshPtr[Objects[Lara.back_gun].meshIndex + 14], clip);
		phd_PopMatrix_I();
	}

	int gunType = LGT_Unarmed;
	if( Lara.gun_status == LGS_Ready
		|| Lara.gun_status == LGS_Special
		|| Lara.gun_status == LGS_Draw
		|| Lara.gun_status == LGS_Undraw )
	{
		gunType = Lara.gun_type;
	}

	switch( gunType ) {
		case LGT_Unarmed:
		case LGT_Flare:
			phd_PushMatrix_I();
			phd_TranslateRel_I(bones[29], bones[30], bones[31]);
			phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
			phd_PutPolygons_I(Lara.mesh_ptrs[8], clip);
			phd_TranslateRel_I(bones[33], bones[34], bones[35]);
			phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
			phd_PutPolygons_I(Lara.mesh_ptrs[9], clip);
			phd_TranslateRel_I(bones[37], bones[38], bones[39]);
			phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
			phd_PutPolygons_I(Lara.mesh_ptrs[10], clip);
			phd_PopMatrix_I();
			phd_PushMatrix_I();
			phd_TranslateRel_I(bones[41], bones[42], bones[43]);
			if( Lara.flare_control_left ) {
				frame = (Anims[Lara.left_arm.anim_number].interpolation >> 8) * (Lara.left_arm.frame_number - Anims[Lara.left_arm.anim_number].frameBase) + 9;
				rot1 = rot2 = (UINT16 *)&Lara.left_arm.frame_base[frame];
				phd_RotYXZsuperpack_I(&rot1, &rot2, 11);
			} else {
				phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
			}
			phd_PutPolygons_I(Lara.mesh_ptrs[11], clip);
			phd_TranslateRel_I(bones[45], bones[46], bones[47]);
			phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
			phd_PutPolygons_I(Lara.mesh_ptrs[12], clip);
			phd_TranslateRel_I(bones[49], bones[50], bones[51]);
			phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
			phd_PutPolygons_I(Lara.mesh_ptrs[13], clip);
			if( Lara.gun_type == LGT_Flare ) {
				if( Lara.left_arm.flash_gun ) {
					phd_TranslateRel_I(11, 32, 80);
					phd_RotX_I(-90 * PHD_DEGREE);
					phd_RotY_I(2 * GetRandomDraw());
					S_CalculateStaticLight(0x800);
					phd_PutPolygons_I(MeshPtr[Objects[ID_FLARE_FIRE].meshIndex], clip);
#ifdef FEATURE_VIDEOFX_IMPROVED
					if( AlphaBlendMode ) {
						int shade = (GetRandomDraw() & 0xFFF) + 0x1000;
						DWORD flags = GLOW_FLARE_COLOR;
						flags |= SPR_BLEND_ADD|SPR_TINT|SPR_SHADE|SPR_SEMITRANS;
						S_DrawSprite(flags, 0, 0, 0, Objects[ID_GLOW].meshIndex, shade, 0);
					}
#endif // FEATURE_VIDEOFX_IMPROVED
				}
			}
			break;
		case LGT_Pistols:
		case LGT_Magnums:
		case LGT_Uzis:
			phd_PushMatrix_I();
			phd_TranslateRel_I(bones[29], bones[30], bones[31]);
			InterpolateArmMatrix();
			phd_RotYXZ(Lara.right_arm.y_rot, Lara.right_arm.x_rot, Lara.right_arm.z_rot);
			frame = (Anims[Lara.right_arm.anim_number].interpolation >> 8) * (Lara.right_arm.frame_number - Anims[Lara.right_arm.anim_number].frameBase) + 9;
			rot1 = (UINT16 *)&Lara.right_arm.frame_base[frame];
			phd_RotYXZsuperpack(&rot1, 8);
			phd_PutPolygons(Lara.mesh_ptrs[8], clip);
			phd_TranslateRel(bones[33], bones[34], bones[35]);
			phd_RotYXZsuperpack(&rot1, 0);
			phd_PutPolygons(Lara.mesh_ptrs[9], clip);
			phd_TranslateRel(bones[37], bones[38], bones[39]);
			phd_RotYXZsuperpack(&rot1, 0);
			phd_PutPolygons(Lara.mesh_ptrs[10], clip);
			if( Lara.right_arm.flash_gun ) {
				matrix = *PhdMatrixPtr;
			}
			phd_PopMatrix_I();
			phd_PushMatrix_I();
			phd_TranslateRel_I(bones[41], bones[42], bones[43]);
			InterpolateArmMatrix();
			phd_RotYXZ(Lara.left_arm.y_rot, Lara.left_arm.x_rot, Lara.left_arm.z_rot);
			frame = (Anims[Lara.left_arm.anim_number].interpolation >> 8) * (Lara.left_arm.frame_number - Anims[Lara.left_arm.anim_number].frameBase) + 9;
			rot1 = (UINT16 *)&Lara.left_arm.frame_base[frame];
			phd_RotYXZsuperpack(&rot1, 11);
			phd_PutPolygons(Lara.mesh_ptrs[11], clip);
			phd_TranslateRel(bones[45], bones[46], bones[47]);
			phd_RotYXZsuperpack(&rot1, 0);
			phd_PutPolygons(Lara.mesh_ptrs[12], clip);
			phd_TranslateRel(bones[49], bones[50], bones[51]);
			phd_RotYXZsuperpack(&rot1, 0);
			phd_PutPolygons(Lara.mesh_ptrs[13], clip);
			if( Lara.left_arm.flash_gun ) {
				DrawGunFlash(gunType, clip);
			}
			if( Lara.right_arm.flash_gun ) {
				*PhdMatrixPtr = matrix;
				DrawGunFlash(gunType, clip);
			}
			break;
		case LGT_Shotgun:
		case LGT_M16:
		case LGT_Grenade:
		case LGT_Harpoon:
			phd_PushMatrix_I();
			phd_TranslateRel_I(bones[29], bones[30], bones[31]);
			frame = Lara.right_arm.frame_number * (Anims[Lara.right_arm.anim_number].interpolation >> 8) + 9;
			rot1 = rot2 = (UINT16 *)&Lara.right_arm.frame_base[frame];
			phd_RotYXZsuperpack_I(&rot1, &rot2, 8);
			phd_PutPolygons_I(Lara.mesh_ptrs[8], clip);
			phd_TranslateRel_I(bones[33], bones[34], bones[35]);
			phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
			phd_PutPolygons_I(Lara.mesh_ptrs[9], clip);
			phd_TranslateRel_I(bones[37], bones[38], bones[39]);
			phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
			phd_PutPolygons_I(Lara.mesh_ptrs[10], clip);
			if( Lara.right_arm.flash_gun ) {
				matrix = *PhdMatrixPtr;
			}
			phd_PopMatrix_I();
			phd_PushMatrix_I();
			phd_TranslateRel_I(bones[41], bones[42], bones[43]);
			phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
			phd_PutPolygons_I(Lara.mesh_ptrs[11], clip);
			phd_TranslateRel_I(bones[45], bones[46], bones[47]);
			phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
			phd_PutPolygons_I(Lara.mesh_ptrs[12], clip);
			phd_TranslateRel_I(bones[49], bones[50], bones[51]);
			phd_RotYXZsuperpack_I(&rot1, &rot2, 0);
			phd_PutPolygons_I(Lara.mesh_ptrs[13], clip);
			if( Lara.right_arm.flash_gun ) {
				*PhdMatrixPtr = matrix;
				DrawGunFlash(gunType, clip);
			}
			break;
		default:
			break;
	}
	phd_PopMatrix();
	phd_PopMatrix();
	phd_PopMatrix();
}

void __cdecl phd_RotYXZsuperpack(UINT16 **pptr, int index) {
	for( int i = 0; i < index; ++i ) {
		if( (**pptr >> 14) == 0 )
			*pptr += 2;
		else
			++*pptr;
	}

	switch( **pptr >> 14 ) {
		case 0 :
			phd_RotYXZpack(((UINT16)(*pptr)[0] << 16) + (UINT16)(*pptr)[1]);
			*pptr += 2;
			break;
		case 1 :
			phd_RotX(**pptr << 6);
			++*pptr;
			break;
		case 2 :
			phd_RotY(**pptr << 6);
			++*pptr;
			break;
		case 3 :
			phd_RotZ(**pptr << 6);
			++*pptr;
			break;
	}
}

void __cdecl phd_PutPolygons_I(__int16 *ptrObj, int clip) {
	phd_PushMatrix();
	InterpolateMatrix();
	phd_PutPolygons(ptrObj, clip);
	phd_PopMatrix();
}

void __cdecl DrawGunFlash(int weapon, int clip) {
	__int16 light;
	int len;
	int off;

	switch( weapon ) {
		case LGT_Shotgun:
			return;
		case LGT_Flare:
			phd_TranslateRel(11, 32, 80);
			phd_RotX(-90 * PHD_DEGREE);
			phd_RotY(2 * GetRandomDraw());
			S_CalculateStaticLight(0x800);
			phd_PutPolygons(MeshPtr[Objects[ID_FLARE_FIRE].meshIndex], clip);
#ifdef FEATURE_VIDEOFX_IMPROVED
			if( AlphaBlendMode ) {
				int shade = (GetRandomDraw() & 0xFFF) + 0x1000;
				DWORD flags = GLOW_FLARE_COLOR;
				flags |= SPR_BLEND_ADD|SPR_TINT|SPR_SHADE|SPR_SEMITRANS;
				S_DrawSprite(flags, 0, 0, 0, Objects[ID_GLOW].meshIndex, shade, 0);
			}
#endif // FEATURE_VIDEOFX_IMPROVED
			return;
		case LGT_M16:
			phd_TranslateRel(0, 400, 99);
			phd_RotYXZ(0, -85 * PHD_DEGREE, (2 * GetRandomDraw() & 0x4000) + 0x2000);
			S_CalculateStaticLight(0xA00);
			phd_PutPolygons(MeshPtr[Objects[ID_M16_FLASH].meshIndex], clip);
#ifdef FEATURE_VIDEOFX_IMPROVED
			if( AlphaBlendMode ) {
				DWORD flags = GLOW_M16_COLOR;
				flags |= SPR_BLEND_ADD|SPR_TINT|SPR_SCALE|SPR_SEMITRANS;
				S_DrawSprite(flags, 0, 0, -65, Objects[ID_GLOW].meshIndex, 0, 0x200);
			}
#endif // FEATURE_VIDEOFX_IMPROVED
			return;
		case LGT_Magnums:
			light = 0x1000;
			len = 215;
			off = 65;
			break;
		case LGT_Uzis:
			light = 0xA00;
			len = 200;
			off = 50;
			break;
		default:
			light = 0x1400;
			len = 185;
			off = 40;
			break;
	}
	phd_TranslateRel(0, len, off);
	phd_RotYXZ(0, -90 * PHD_DEGREE, 2 * GetRandomDraw());
	S_CalculateStaticLight(light);
	phd_PutPolygons(MeshPtr[Objects[ID_GUN_FLASH].meshIndex], clip);
#ifdef FEATURE_VIDEOFX_IMPROVED
	if( AlphaBlendMode ) {
		DWORD flags = GLOW_PISTOL_COLOR;
		flags |= SPR_BLEND_ADD|SPR_TINT|SPR_SCALE|SPR_SEMITRANS;
		S_DrawSprite(flags, 0, 0, 0, Objects[ID_GLOW].meshIndex, 0, 0x200);
	}
#endif // FEATURE_VIDEOFX_IMPROVED
}

void __cdecl CalculateObjectLighting(ITEM_INFO *item, __int16 *frame) {
	int x, y, z;

	if (item->shade1 < 0) {
		phd_PushUnitMatrix();
		PhdMatrixPtr->_23 = 0;
		PhdMatrixPtr->_13 = 0;
		PhdMatrixPtr->_03 = 0;
		phd_RotYXZ(item->pos.rotY, item->pos.rotX, item->pos.rotZ);
		phd_TranslateRel((frame[0] + frame[1]) >> 1, (frame[2] + frame[3]) >> 1, (frame[4] + frame[5]) >> 1);
		x = item->pos.x + (PhdMatrixPtr->_03 >> W2V_SHIFT);
		y = item->pos.y + (PhdMatrixPtr->_13 >> W2V_SHIFT);
		z = item->pos.z + (PhdMatrixPtr->_23 >> W2V_SHIFT);
		phd_PopMatrix();
		S_CalculateLight(x, y, z, item->roomNumber);
	} else {
		S_CalculateStaticMeshLight(item->pos.x, item->pos.y, item->pos.z, item->shade1, item->shade2, &RoomInfo[item->roomNumber]);
	}
}

void __cdecl AddDynamicLight(int x, int y, int z, int intensity, int falloff) {
	int idx = ( DynamicLightCount < ARRAY_SIZE(DynamicLights) ) ? DynamicLightCount++ : 0;
	DynamicLights[idx].x = x;
	DynamicLights[idx].y = y;
	DynamicLights[idx].z = z;
	DynamicLights[idx].intensity1 = intensity;
	DynamicLights[idx].fallOff1 = falloff;
}

/*
 * Inject function
 */
void Inject_Draw() {
//	INJECT(0x00418920, DrawPhaseCinematic);
//	INJECT(0x00418960, DrawPhaseGame);

	INJECT(0x004189A0, DrawRooms);
	INJECT(0x00418C50, GetRoomBounds);
	INJECT(0x00418E20, SetRoomBounds);
	INJECT(0x004191A0, ClipRoom);
	INJECT(0x00419580, PrintRooms);
	INJECT(0x00419640, PrintObjects);
	INJECT(0x00419870, DrawEffect);
	INJECT(0x004199C0, DrawSpriteItem);

//	INJECT(----------, DrawDummyItem);
	INJECT(0x00419A50, DrawAnimatingItem);

//	INJECT(0x00419DD0, DrawLara);

	INJECT(0x0041AB00, DrawLaraInt);

//	INJECT(0x0041B6F0, InitInterpolate);
//	INJECT(0x0041B730, phd_PopMatrix_I);
//	INJECT(0x0041B760, phd_PushMatrix_I);
//	INJECT(0x0041B790, phd_RotY_I);
//	INJECT(0x0041B7D0, phd_RotX_I);
//	INJECT(0x0041B810, phd_RotZ_I);
//	INJECT(0x0041B850, phd_TranslateRel_I);
//	INJECT(0x0041B8A0, phd_TranslateRel_ID);
//	INJECT(0x0041B8F0, phd_RotYXZ_I);
//	INJECT(0x0041B940, phd_RotYXZsuperpack_I);

	INJECT(0x0041B980, phd_RotYXZsuperpack);
	INJECT(0x0041BA30, phd_PutPolygons_I);

//	INJECT(0x0041BA60, InterpolateMatrix);
//	INJECT(0x0041BC10, InterpolateArmMatrix);

	INJECT(0x0041BD10, DrawGunFlash);
	INJECT(0x0041BE80, CalculateObjectLighting);

//	INJECT(0x0041BF70, GetFrames);
//	INJECT(0x0041C010, GetBoundsAccurate);
//	INJECT(0x0041C090, GetBestFrame);

	INJECT(0x0041C0D0, AddDynamicLight);
}
