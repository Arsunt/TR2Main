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
#include "game/cinema.h"
#include "3dsystem/3d_gen.h"
#include "3dsystem/phd_math.h"
#include "game/control.h"
#include "game/draw.h"
#include "game/hair.h"
#include "game/items.h"
#include "game/laramisc.h"
#include "game/setup.h"
#include "game/sphere.h"
#include "specific/frontend.h"
#include "specific/input.h"
#include "specific/output.h"
#include "specific/sndpc.h"
#include "global/vars.h"

#ifdef FEATURE_INPUT_IMPROVED
#include "modding/joy_output.h"
#endif // FEATURE_INPUT_IMPROVED

void __cdecl SetCutsceneTrack(int track) {
	CineTrackID = track;
}

int __cdecl StartCinematic(int levelID) {
	int result;
	BOOL soundWasActive;
	int nTicks;

	CineLevelID = levelID;
	IsTitleLoaded = FALSE;
	S_FadeToBlack();

	if( !InitialiseLevel(levelID, GFL_CUTSCENE) ) {
		return 2;
	}

	InitCinematicRooms();
	InitialisePlayer1(Lara.item_number);
	Camera.targetAngle = CineTargetAngle;

	soundWasActive = SoundIsActive;
	SoundIsActive = FALSE;

	CineFrameIdx = 0;
	S_ClearScreen();

	if( !StartSyncedAudio(CineTrackID) ) {
		return 1;
	}

	S_CDVolume(255);
	CineCurrentFrame = 0;

	do {
		DrawPhaseCinematic();
		nTicks = CineCurrentFrame - TICKS_PER_FRAME * (CineFrameIdx - 4);
		if( nTicks < TICKS_PER_FRAME ) {
			nTicks = TICKS_PER_FRAME;
		}
		result = DoCinematic(nTicks);
	} while( !result );

	S_CDVolume((MusicVolume > 0) ? (25 * MusicVolume + 5) : 0);
	S_CDStop();
	SoundIsActive = soundWasActive;
	S_SoundStopAllSamples();

	IsLevelComplete = TRUE;
	return result;
}

void __cdecl InitCinematicRooms() {
	for( int i=0; i<RoomCount; ++i ) {
		if( RoomInfo[i].flippedRoom >= 0 ) {
			RoomInfo[RoomInfo[i].flippedRoom].boundActive = 1;
		}
		RoomInfo[i].flags |= ROOM_OUTSIDE;
	}
	DrawRoomsCount = 0;
	for( int i=0; i<RoomCount; ++i ) {
		if( !RoomInfo[i].boundActive ) {
			DrawRoomsArray[DrawRoomsCount++] = i;
		}
	}
}

int __cdecl DoCinematic(int nTicks) {
	static int tickCount = 0;
	int id = -1;
	int next = -1;

	for( tickCount += CineTickRate*nTicks; tickCount >= 0; tickCount -= PHD_ONE ) {
		if( S_UpdateInput() ) {
			return 3;
		}
		if( CHK_ANY(InputStatus, IN_ACTION) ) {
			return 1;
		}
		if( CHK_ANY(InputStatus, IN_OPTION) ) {
			return 2;
		}

		DynamicLightCount = 0;

		for( id = NextItemActive; id >= 0; id = next ) {
			next = Items[id].nextActive;
			if( Objects[Items[id].objectID].control ) {
				Objects[Items[id].objectID].control(id);
			}
		}

		for( id = NextEffectActive; id >= 0; id = next ) {
			next = Effects[id].next_active;
			if( Objects[Effects[id].object_number].control ) {
				Objects[Effects[id].object_number].control(id);
			}
		}

		HairControl(1);
		CalculateCinematicCamera();

		if( ++CineFrameIdx >= CineFramesCount ) {
			return 1;
		}
	}

	CineCurrentFrame = S_CDGetLoc()*4/5;
#ifdef FEATURE_INPUT_IMPROVED
	UpdateJoyOutput(false);
#endif // FEATURE_INPUT_IMPROVED
	return 0;
}

void __cdecl CalculateCinematicCamera() {
	CINE_FRAME_INFO *frame;
	int c, s, sourceX, sourceY, sourceZ, targetX, targetY, targetZ;
	__int16 roomID;

	frame = &CineFrames[CineFrameIdx];
	c = phd_cos(Camera.targetAngle);
	s = phd_sin(Camera.targetAngle);
	targetX = LaraItem->pos.x + ((s * frame->zTarget + c * frame->xTarget) >> W2V_SHIFT);
	targetY = LaraItem->pos.y + frame->yTarget;
	targetZ = LaraItem->pos.z + ((c * frame->zTarget - s * frame->xTarget) >> W2V_SHIFT);
	sourceX = LaraItem->pos.x + ((s * frame->xPos + c * frame->zPos) >> W2V_SHIFT);
	sourceZ = LaraItem->pos.z + ((c * frame->xPos - s * frame->zPos) >> W2V_SHIFT);
	sourceY = LaraItem->pos.y + frame->yPos;
	roomID = GetCinematicRoom(sourceX, sourceY, sourceZ);
	if (roomID >= 0)
		Camera.pos.roomNumber = roomID;
	AlterFOV(frame->fov);
	phd_LookAt(sourceX, sourceY, sourceZ, targetX, targetY, targetZ, frame->roll);
}

int __cdecl GetCinematicRoom(int x, int y, int z) {
	__int16 roomID;
	int i;
	ROOM_INFO *room;

	roomID = -1;
	for (i = 0; i < RoomCount; ++i) {
		room = &RoomInfo[i];
		if (x >= room->x + 1024 &&
			x < room->x + (room->ySize << WALL_SHIFT) - 1024 &&
			y >= room->maxCeiling &&
			y <= room->minFloor &&
			z >= room->z + 1024 &&
			z < room->z + (room->xSize << WALL_SHIFT) - 1024)
		{
			roomID = i;
			break;
		}
	}
	return roomID;
}

void __cdecl ControlCinematicPlayer(__int16 itemID) {
	ITEM_INFO *item;
	PHD_VECTOR pos;
	__int16 roomID;

	item = &Items[itemID];
	item->pos.rotY = Camera.targetAngle;
	item->pos.x = Camera.pos.x;
	item->pos.y = Camera.pos.y;
	item->pos.z = Camera.pos.z;
	pos.x = 0;
	pos.y = 0;
	pos.z = 0;
	GetJointAbsPosition(item, &pos, 0);
	roomID = GetCinematicRoom(pos.x, pos.y, pos.z);
	if (roomID != -1 && item->roomNumber != roomID)
		ItemNewRoom(itemID, roomID);
	if (item->dynamic_light && item->status != ITEM_INVISIBLE) {
		pos.x = 0;
		pos.y = 0;
		pos.z = 0;
		GetJointAbsPosition(item, &pos, 0);
		AddDynamicLight(pos.x, pos.y, pos.z, 12, 11);
	}
	AnimateItem(item);
}

void __cdecl LaraControlCinematic(__int16 itemID) {
	ITEM_INFO *item;
	PHD_VECTOR pos;
	__int16 roomID;

	item = &Items[itemID];
	item->pos.rotY = Camera.targetAngle;
	item->pos.x = Camera.pos.x;
	item->pos.y = Camera.pos.y;
	item->pos.z = Camera.pos.z;
	pos.x = 0;
	pos.y = 0;
	pos.z = 0;
	GetJointAbsPosition(item, &pos, 0);
	roomID = GetCinematicRoom(pos.x, pos.y, pos.z);
	if (roomID != -1 && item->roomNumber != roomID)
		ItemNewRoom(itemID, roomID);
	AnimateLara(item);
}

void __cdecl InitialisePlayer1(__int16 itemID) {
	ITEM_INFO *item;

	Objects[ID_LARA].drawRoutine = DrawLara;
	Objects[ID_LARA].control = LaraControlCinematic;
	AddActiveItem(itemID);
	item = &Items[itemID];
	Camera.pos.x = item->pos.x;
	Camera.pos.y = item->pos.y;
	Camera.pos.z = item->pos.z;
	item->pos.rotY = 0;
	Camera.targetAngle = 0;
	Camera.pos.roomNumber = item->roomNumber;
	item->dynamic_light = 0;
	item->goalAnimState = AS_WALK;
	item->currentAnimState = AS_WALK;
	item->frameNumber = 0;
	item->animNumber = 0;
	Lara.hit_direction = -1;
}

void __cdecl InitialiseGenPlayer(__int16 itemID) {
	ITEM_INFO *item;

	AddActiveItem(itemID);
	item = &Items[itemID];
	item->pos.rotY = 0;
	item->dynamic_light = 0;
}

void __cdecl InGameCinematicCamera() {
	CINE_FRAME_INFO *frame;
	int c, s;

	++CineFrameIdx;
	if (CineFrameIdx >= CineFramesCount)
		CineFrameIdx = CineFramesCount - 1;
	frame = &CineFrames[CineFrameIdx];
	c = phd_cos(CinematicPos.rotY);
	s = phd_sin(CinematicPos.rotY);
	Camera.target.x = CinematicPos.x + ((s * frame->zTarget + c * frame->xTarget) >> W2V_SHIFT);
	Camera.target.y = CinematicPos.y + frame->yTarget;
	Camera.target.z = CinematicPos.z + ((c * frame->zTarget - s * frame->xTarget) >> W2V_SHIFT);
	Camera.pos.x = CinematicPos.x + ((s * frame->xPos + c * frame->zPos) >> W2V_SHIFT);
	Camera.pos.y = CinematicPos.y + frame->yPos;
	Camera.pos.z = CinematicPos.z + ((c * frame->xPos - s * frame->zPos) >> W2V_SHIFT);
	AlterFOV(frame->fov);
	phd_LookAt(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.target.x, Camera.target.y, Camera.target.z, frame->roll);
	GetFloor(Camera.pos.x, Camera.pos.y, Camera.pos.z, &Camera.pos.roomNumber);
	if (Camera.isLaraMic) {
		Camera.actualAngle = LaraItem->pos.rotY + Lara.head_y_rot + Lara.torso_y_rot;
		Camera.micPos.x = LaraItem->pos.x;
		Camera.micPos.y = LaraItem->pos.y;
		Camera.micPos.z = LaraItem->pos.z;
	} else {
		Camera.actualAngle = phd_atan(Camera.target.z - Camera.pos.z, Camera.target.x - Camera.pos.x);
		Camera.micPos.x = Camera.pos.x + (phd_sin(Camera.actualAngle) * PhdPersp >> W2V_SHIFT);
		Camera.micPos.z = Camera.pos.z + (phd_cos(Camera.actualAngle) * PhdPersp >> W2V_SHIFT);
		Camera.micPos.y = Camera.pos.y;
	}
}

/*
 * Inject function
 */
void Inject_Cinema() {
	INJECT(0x00411F30, SetCutsceneTrack);
	INJECT(0x00411F40, StartCinematic);
	INJECT(0x00412060, InitCinematicRooms);
	INJECT(0x00412100, DoCinematic);
	INJECT(0x00412270, CalculateCinematicCamera);
	INJECT(0x004123B0, GetCinematicRoom);
	INJECT(0x00412430, ControlCinematicPlayer);
	INJECT(0x00412510, LaraControlCinematic);
	INJECT(0x004125B0, InitialisePlayer1);
	INJECT(0x00412640, InitialiseGenPlayer);
	INJECT(0x00412680, InGameCinematicCamera);
}
