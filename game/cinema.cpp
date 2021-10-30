/*
 * Copyright (c) 2017-2020 Michael Chaban. All rights reserved.
 * Original game is created by Core Design Ltd. in 1997.
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
#include "game/draw.h"
#include "game/hair.h"
#include "game/setup.h"
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

/*
 * Inject function
 */
void Inject_Cinema() {
	INJECT(0x00411F30, SetCutsceneTrack);
	INJECT(0x00411F40, StartCinematic);
	INJECT(0x00412060, InitCinematicRooms);
	INJECT(0x00412100, DoCinematic);

//	INJECT(0x00412270, CalculateCinematicCamera);
//	INJECT(0x004123B0, GetCinematicRoom);
//	INJECT(0x00412430, ControlCinematicPlayer);
//	INJECT(0x00412510, LaraControlCinematic);
//	INJECT(0x004125B0, InitialisePlayer1);
//	INJECT(0x00412640, InitialiseGenPlayer);
//	INJECT(0x00412680, InGameCinematicCamera);
}
