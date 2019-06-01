/*
 * Copyright (c) 2017-2019 Michael Chaban. All rights reserved.
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
#include "game/draw.h"
#include "game/setup.h"
#include "specific/frontend.h"
#include "specific/output.h"
#include "specific/sndpc.h"
#include "global/vars.h"

void __cdecl SetCutsceneTrack(int track) {
	CineTrackID = track;
}

int __cdecl StartCinematic(int levelID) {
	int result;
	BOOL soundWasActive;
	int framesCount;

	CineLevelID = levelID;
	IsTitleLoaded = FALSE;
	S_FadeToBlack();

	if( !InitialiseLevel(levelID, 4) ) {
		return 2;
	}

	InitCinematicRooms();
	InitialisePlayer1(Lara_ItemNumber);
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
		framesCount = CineCurrentFrame + 2 * (4 - CineFrameIdx);
		if( framesCount < 2 ) {
			framesCount = 2;
		}
		result = DoCinematic(framesCount);
	} while( !result );

	S_CDVolume((MusicVolume > 0) ? (25 * MusicVolume + 5) : 0);
	S_CDStop();
	SoundIsActive = soundWasActive;
	S_SoundStopAllSamples();

	IsLevelComplete = TRUE;
	return result;
}

/*
 * Inject function
 */
void Inject_Cinema() {
	INJECT(0x00411F30, SetCutsceneTrack);
	INJECT(0x00411F40, StartCinematic);
//	INJECT(0x00412060, InitCinematicRooms);
//	INJECT(0x00412100, DoCinematic);
//	INJECT(0x00412270, CalculateCinematicCamera);
//	INJECT(0x004123B0, GetCinematicRoom);
//	INJECT(0x00412430, ControlCinematicPlayer);
//	INJECT(0x00412510, LaraControlCinematic);
//	INJECT(0x004125B0, InitialisePlayer1);
//	INJECT(0x00412640, InitialiseGenPlayer);
//	INJECT(0x00412680, InGameCinematicCamera);
}
