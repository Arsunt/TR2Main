/*
 * Copyright (c) 2017-2018 Michael Chaban. All rights reserved.
 * CD Audio solution in this file was designed by PaulD.
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
#include "modding/cd_pauld.h"
#include "global/vars.h"

#define CD_ALIAS "T2"

typedef struct TrackInfo_t {
	DWORD from;
	DWORD to;
	bool active;
} TRACK_INFO;

static TRACK_INFO Tracks[60];
static bool isCDAudioEnabled = false;

bool __cdecl PaulD_CD_Init() {
	static LPCTSTR audioFiles[2] = {
		"audio\\cdaudio.mp3",
		"audio\\cdaudio.wav", // MCI volume control doesn't work with waveaudio. Wave option added just in case
	};
	static LPCTSTR audioTypes[2] = {
		"mpegvideo",
		"waveaudio",
	};
	HANDLE hFile;
	DWORD fileSize, bytesRead, offset;
	char cmdString[256];
	char *buf;
	int rc;
	int audioType = -1;

	if( isCDAudioEnabled )
		return true;

	for( int i=0; i<2; ++i ) {
		hFile = CreateFile(audioFiles[i], GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if( hFile != INVALID_HANDLE_VALUE ) {
			audioType = i;
			CloseHandle(hFile);
			break;
		}
	}
	if( audioType < 0 )
		return false;

	hFile = CreateFile("audio\\cdaudio.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if( hFile == INVALID_HANDLE_VALUE )
		return false;

	fileSize = GetFileSize(hFile, NULL);
	buf = (char *)malloc(fileSize);
	if( buf == NULL ) {
		CloseHandle(hFile);
		return false;
	}
	rc = ReadFile(hFile, buf, fileSize, &bytesRead, NULL);
	CloseHandle(hFile);
	if( !rc || bytesRead == 0 ) return false;

	memset(Tracks, 0, sizeof(Tracks));
	offset = 0;

	while( offset < bytesRead ) {
		DWORD index, from, to;

		while( buf[offset] == 0x0A || buf[offset] == 0x0D ) {
			if( ++offset >= bytesRead ) goto PARSE_END; // skip empty lines
		}
		rc = sscanf(&buf[offset], "%lu %lu %lu", &index, &from, &to); // track comment is ignored
		if( rc == 3 && index > 0 && index <= 60 ) {
			Tracks[index-1].active = true;
			Tracks[index-1].from = from;
			Tracks[index-1].to = to;
		}
		while( buf[offset] != 0x0A && buf[offset] != 0x0D ) {
			if( ++offset >= bytesRead ) goto PARSE_END; // skip until newline
		}
	}
PARSE_END :
	free(buf);

	// Fix cdaudio.dat if it's broken (like the one from the Steam version)
	for( DWORD i=0; i<ARRAY_SIZE(Tracks); ++i ) {
		if( !Tracks[i].active ) continue;
		if( Tracks[i].from >= Tracks[i].to && i < ARRAY_SIZE(Tracks)-1 ) {
			// Trying to fix data using the next track
			for( DWORD j=i+1; j<ARRAY_SIZE(Tracks); ++j ) {
				if( Tracks[j].active ) {
					Tracks[i].to = Tracks[j].from;
					break;
				}
			}
		}
		if( Tracks[i].from >= Tracks[i].to && i > 0 ) {
			// Trying to fix data using the previous track
			for( int j=i-1; j>=0; --j ) {
				if( Tracks[j].active ) {
					Tracks[i].from = Tracks[j].to;
					break;
				}
			}
		}
	}

	wsprintf(cmdString, "open %s type %s alias " CD_ALIAS, audioFiles[audioType], audioTypes[audioType]);
	rc = mciSendString(cmdString, NULL, 0, 0);
	if( rc == 0 ) {
		mciSendString("set " CD_ALIAS " time format ms", NULL, 0, 0);
		isCDAudioEnabled = true;
	}
	return true;
}

void __cdecl PaulD_CD_Cleanup() {
	if( isCDAudioEnabled ) {
		PaulD_CDStop();
		mciSendString("close " CD_ALIAS, NULL, 0, 0);
		isCDAudioEnabled = false;
	}
}

void __cdecl PaulD_CDLoop() {
	int rc;
	char statusString[32];
	char cmdString[256];

	if( CD_LoopTrack == 0 || ++CD_LoopCounter < 150 )
		return;

	CD_LoopCounter = 0;
	rc = mciSendString("status " CD_ALIAS " mode", statusString, sizeof(statusString), 0);

	if( (rc == 0) && !strncmp(statusString, "stopped", sizeof(statusString)) ) {
		wsprintf(cmdString, "play " CD_ALIAS " from %lu to %lu", Tracks[CD_LoopTrack-1].from, Tracks[CD_LoopTrack-1].to);
		mciSendString(cmdString, NULL, 0, 0);
	}
}

void __cdecl PaulD_CDPlay(__int16 trackID, BOOL isLooped) {
	__int16 track;
	char cmdString[256];

	if( MusicVolume == 0 )
		return;

	track = GetRealTrack(trackID);
	if( track < 1 || !Tracks[track-1].active )
		return;

	CD_TrackID = trackID;

	wsprintf(cmdString, "play " CD_ALIAS " from %lu to %lu", Tracks[track-1].from, Tracks[track-1].to);
	mciSendString(cmdString, NULL, 0, 0);

	if( isLooped ) {
		CD_LoopTrack = track;
		CD_LoopCounter = 120;
	}
}

void __cdecl PaulD_CDStop() {
	if( CD_TrackID > 0 ) {
		mciSendString("stop " CD_ALIAS, NULL, 0, 0);
		CD_TrackID = 0;
		CD_LoopTrack = 0;
	}
}

BOOL __cdecl PaulD_StartSyncedAudio(int trackID) {
	__int16 track;
	char cmdString[256];

	track = GetRealTrack(trackID);
	if( track < 1 || !Tracks[track-1].active )
		return FALSE;

	CD_TrackID = trackID;
	if( 0 != mciSendString("set " CD_ALIAS " time format ms", NULL, 0, 0) )
		return FALSE;

	wsprintf(cmdString, "play " CD_ALIAS " from %lu to %lu", Tracks[track-1].from, Tracks[track-1].to);
	return ( 0 == mciSendString(cmdString, NULL, 0, 0) );
}

DWORD __cdecl PaulD_CDGetLoc() {
	__int16 track;
	char statusString[32];

	if( 0 != mciSendString("status " CD_ALIAS " position", statusString, sizeof(statusString), 0) )
		return 0;

	track = GetRealTrack(CD_TrackID);
	// calculate audio frames position (75 audio frames per second)
	return (atol(statusString) - Tracks[track-1].from) * 75 / 1000;
}

void __cdecl PaulD_CDVolume(DWORD volume) {
	char cmdString[256];

	if( volume > 0 )
		volume = (volume - 5) * 4; // 0..255 -> 0..1000

	wsprintf(cmdString, "setaudio " CD_ALIAS " volume to %lu", volume);
	mciSendString(cmdString, NULL, 0, 0);

	char statusString[32];
	if( 0 == mciSendString("status " CD_ALIAS " mode", statusString, sizeof(statusString), 0) ) {
		if( volume > 0 ) {
			if( !strncmp(statusString, "paused", sizeof(statusString)) ) {
				mciSendString("resume " CD_ALIAS, NULL, 0, 0);
			}
		} else {
			if( !strncmp(statusString, "playing", sizeof(statusString)) ) {
				mciSendString("pause " CD_ALIAS, NULL, 0, 0);
			}
		}
	}
}

