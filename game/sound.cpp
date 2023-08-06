/*
 * Copyright (c) 2017-2018 Michael Chaban. All rights reserved.
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
#include "game/sound.h"
#include "specific/sndpc.h"
#include "global/vars.h"

int __cdecl GetRealTrack(int trackID) {
	static char vtracks[] = {2, 19, 20, 26, -1};
	int idx = 0;
	int track = 2;

	for( int i = 2; i < trackID; ++i ) {
		if( (vtracks[idx] >= 0) && (i == vtracks[idx]) )
			++idx;
		else
			++track;
	}
	return track;
}

void __cdecl SOUND_Init() {
	S_SoundSetMasterVolume(32); // 50% sfx volume

	for( int i=0; i<32; ++i )
		SfxInfos[i].sampleIdx = -1;

	SoundIsActive = TRUE;
}

/*
 * Inject function
 */
void Inject_Sound() {
	INJECT(0x0043F430, GetRealTrack);

//	INJECT(0x0043F470, PlaySoundEffect);
//	INJECT(0x0043F910, StopSoundEffect);
//	INJECT(0x0043F970, SOUND_EndScene);
//	INJECT(0x0043FA00, SOUND_Stop);

	INJECT(0x0043FA30, SOUND_Init);
}
