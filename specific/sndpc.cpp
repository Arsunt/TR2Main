/*
 * Copyright (c) 2017 Michael Chaban. All rights reserved.
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
#include "specific/sndpc.h"
#include "game/sound.h"
#include "specific/file.h"
#include "specific/init_sound.h"
#include "specific/utils.h"
#include "global/resource.h"
#include "global/vars.h"

int __cdecl S_SoundPlaySample(int channel, UINT16 volume, int pitch, int pan) {
	if( !SoundIsActive )
		return -3;

	int calcPan = S_Sound_CalculateSamplePan(pan);
	int calcVolume = S_Sound_CalculateSampleVolume(volume);
	return WinSndPlaySample(channel, calcVolume, pitch, calcPan, 0);
}

int __cdecl S_Sound_CalculateSampleVolume(DWORD volume) {
	return (int)(((double)(S_MasterVolume * volume)/0x200000.p0 - 1.0) * 5000.0);
}

int __cdecl S_Sound_CalculateSamplePan(__int16 pan) {
	return pan/16;
}

int __cdecl S_SoundPlaySampleLooped(int channel, UINT16 volume, DWORD pitch, int pan) {
	if( !SoundIsActive )
		return -3;

	int calcPan = S_Sound_CalculateSamplePan(pan);
	int calcVolume = S_Sound_CalculateSampleVolume(volume);
	return WinSndPlaySample(channel, calcVolume, pitch, calcPan, DSBPLAY_LOOPING);
}

void __cdecl S_SoundSetPanAndVolume(int channel, int pan, UINT16 volume) {
	if( SoundIsActive ) {
		int calcPan = S_Sound_CalculateSamplePan(pan);
		int calcVolume = S_Sound_CalculateSampleVolume(volume);
		WinSndAdjustVolumeAndPan(channel, calcVolume, calcPan);
	}
}

void __cdecl S_SoundSetPitch(int channel, DWORD pitch) {
	if( SoundIsActive )
		WinSndAdjustPitch(channel, pitch);
}

void __cdecl S_SoundSetMasterVolume(DWORD volume) {
	S_MasterVolume = volume;
}

void __cdecl S_SoundStopSample(int channel) {
	if( SoundIsActive )
		WinSndStopSample(channel);
}

void __cdecl S_SoundStopAllSamples() {
	if( SoundIsActive )
		for( DWORD i=0; i<32; ++i )
			WinSndStopSample(i);
}

BOOL __cdecl S_SoundSampleIsPlaying(int channel) {
	if( !SoundIsActive )
		return FALSE;

	return WinSndIsChannelPlaying(channel);
}

bool __cdecl CD_Init() {
	MCI_OPEN_PARMS openParams;
	MCI_SET_PARMS setParams;

#ifndef FEATURE_NOCD_DATA
	while( !SelectDrive() ) {
		if( !CD_NoteAlert(MAKEINTRESOURCE(IDD_CD_PROMPT), HGameWindow) )
			return false;
	}
#endif // !FEATURE_NOCD_DATA

	openParams.lpstrDeviceType = "cdaudio";
	if( 0 == mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE, (DWORD_PTR)&openParams) ) {
		MciDeviceID = openParams.wDeviceID;
		setParams.dwTimeFormat = MCI_FORMAT_TMSF;
		mciSendCommand(MciDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD_PTR)&setParams);
	}
	return true;
}

void __cdecl CD_Cleanup() {
	MCI_GENERIC_PARMS stop;
	stop.dwCallback = 0;

	mciSendCommand(MciDeviceID, MCI_STOP,  0, (DWORD_PTR)&stop);
	mciSendCommand(MciDeviceID, MCI_CLOSE, 0, 0);
}

void __cdecl S_CDLoop() {
	int rc;
	MCI_PLAY_PARMS playParams;
	MCI_STATUS_PARMS statusParams;

	if( CD_LoopID == 0 || ++CD_LoopCounter < 150 )
		return;

	CD_LoopCounter = 0;
	statusParams.dwItem = MCI_STATUS_MODE;
	rc = mciSendCommand(MciDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD_PTR)&statusParams);
	if( (rc == 0) && (statusParams.dwReturn == MCI_MODE_STOP) ) {
		playParams.dwFrom = CD_LoopID;
		playParams.dwTo = CD_LoopID + 1;
		mciSendCommand(MciDeviceID, MCI_PLAY, MCI_NOTIFY_FAILURE|MCI_NOTIFY_ABORTED, (DWORD_PTR)&playParams);
	}
}

void __cdecl S_CDPlay(__int16 trackID, BOOL isLooped) {
	__int16 track;
	MCI_PLAY_PARMS playParams;

	if( MusicVolume == 0 )
		return;

	CD_TrackID = trackID;
	track = GetRealTrack(trackID);
	playParams.dwFrom = track;
	playParams.dwTo = track + 1;
	mciSendCommand(MciDeviceID, MCI_PLAY, MCI_NOTIFY_FAILURE|MCI_NOTIFY_ABORTED, (DWORD_PTR)&playParams);

	if( isLooped ) {
		CD_LoopID = track;
		CD_LoopCounter = 120;
	}
}

void __cdecl S_CDStop() {
	MCI_GENERIC_PARMS stop;

	stop.dwCallback = 0;
	if( CD_TrackID > 0 ) {
		mciSendCommand(MciDeviceID, MCI_STOP, 0, (DWORD_PTR)&stop);
		CD_TrackID = 0;
		CD_LoopID = 0;
	}
}

BOOL __cdecl StartSyncedAudio(int trackID) {
	__int16 track;
	MCI_PLAY_PARMS playParams;
	MCI_SET_PARMS setParams;

	CD_TrackID = trackID;
	track = GetRealTrack(trackID);

	setParams.dwTimeFormat = MCI_FORMAT_TMSF;
	if( 0 != mciSendCommand(MciDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD_PTR)&setParams) )
		return FALSE;

	playParams.dwFrom = track;
	playParams.dwTo = track + 1;
	return ( 0 == mciSendCommand(MciDeviceID, MCI_PLAY, MCI_NOTIFY_FAILURE|MCI_NOTIFY_ABORTED, (DWORD_PTR)&playParams) );
}

DWORD __cdecl S_CDGetLoc() {
	DWORD pos;
	MCI_STATUS_PARMS statusParams;

	statusParams.dwItem = MCI_STATUS_POSITION;
	if( 0 != mciSendCommand(MciDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD_PTR)&statusParams) )
		return 0;

	pos = statusParams.dwReturn;
	// calculate audio frames position (75 audio frames per second)
	return (MCI_TMSF_MINUTE(pos)*60 + MCI_TMSF_SECOND(pos))*75 + MCI_TMSF_FRAME(pos);
}

void __cdecl S_CDVolume(DWORD volume) {
	AUXCAPS caps;
	bool isVolumeSet = false;
	UINT deviceID = (UINT)(-1);
	UINT auxDevCount = auxGetNumDevs();

	if( auxDevCount == 0)
		return;

	volume *= 0x100; // 0 .. 255 -> 0..65280

	for( UINT i=0; i<auxDevCount; ++i ) {
		auxGetDevCaps(i, &caps, sizeof(AUXCAPS));

		switch( caps.wTechnology ) {
			case AUXCAPS_CDAUDIO :
				auxSetVolume(i, MAKELONG(volume, volume));
				isVolumeSet = true;
				break;

			case AUXCAPS_AUXIN :
				deviceID = i;
				break;
		}
	}

	if( !isVolumeSet && deviceID != (UINT)(-1) )
		auxSetVolume(deviceID, MAKELONG(volume, volume));
}

/*
 * Inject function
 */
void Inject_SndPC() {
	INJECT(0x004553B0, S_SoundPlaySample);
	INJECT(0x00455400, S_Sound_CalculateSampleVolume);
	INJECT(0x00455430, S_Sound_CalculateSamplePan);
	INJECT(0x00455460, S_SoundPlaySampleLooped);
	INJECT(0x004554B0, S_SoundSetPanAndVolume);
	INJECT(0x004554F0, S_SoundSetPitch);
	INJECT(0x00455510, S_SoundSetMasterVolume);
	INJECT(0x00455520, S_SoundStopSample);
	INJECT(0x00455540, S_SoundStopAllSamples);
	INJECT(0x00455550, S_SoundSampleIsPlaying);
	INJECT(0x00455570, CD_Init);
	INJECT(0x00455600, CD_Cleanup);
	INJECT(0x00455640, S_CDLoop);
	INJECT(0x004556E0, S_CDPlay);
	INJECT(0x00455760, S_CDStop);
	INJECT(0x004557A0, StartSyncedAudio);
	INJECT(0x00455830, S_CDGetLoc);
	INJECT(0x004558A0, S_CDVolume);
}
