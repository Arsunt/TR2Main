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

#ifdef FEATURE_AUDIO_IMPROVED
double InventoryMusicMute = 0.8;
double UnderwaterMusicMute = 0.8;
#endif // FEATURE_AUDIO_IMPROVED

#ifdef FEATURE_PAULD_CDAUDIO
#include "modding/cd_pauld.h"
static bool PaulD_isActive = false;
#endif // FEATURE_PAULD_CDAUDIO

// NOTE: There is no such flag in the original game.
// It is added to provide additional protection against crashes
static bool isCDAudioEnabled = false;
// NOTE: this variable is absent in the original game
static DWORD CDVolume = 0;

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

#ifdef FEATURE_PAULD_CDAUDIO
	PaulD_isActive = PaulD_CD_Init();
	if( PaulD_isActive ) return true;
#endif // FEATURE_PAULD_CDAUDIO

	if( isCDAudioEnabled )
		return true;

	openParams.lpstrDeviceType = "cdaudio";
	if( 0 == mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE, (DWORD_PTR)&openParams) ) {
		MciDeviceID = openParams.wDeviceID;
		setParams.dwTimeFormat = MCI_FORMAT_TMSF;
		mciSendCommand(MciDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD_PTR)&setParams);
		isCDAudioEnabled = true;
	}
	return true;
}

void __cdecl CD_Cleanup() {
	MCI_GENERIC_PARMS params;

#ifdef FEATURE_PAULD_CDAUDIO
	if( PaulD_isActive ) {
		PaulD_CD_Cleanup();
		return;
	}
#endif // FEATURE_PAULD_CDAUDIO

	if( !isCDAudioEnabled )
		return;

	mciSendCommand(MciDeviceID, MCI_STOP, 0, (DWORD_PTR)&params);
	mciSendCommand(MciDeviceID, MCI_CLOSE, 0, (DWORD_PTR)&params);
	isCDAudioEnabled = false;
}

void __cdecl S_CDLoop() {
	int rc;
	MCI_PLAY_PARMS playParams;
	MCI_STATUS_PARMS statusParams;

#ifdef FEATURE_PAULD_CDAUDIO
	if( PaulD_isActive ) {
		PaulD_CDLoop();
		return;
	}
#endif // FEATURE_PAULD_CDAUDIO

	if( CD_LoopTrack == 0 || ++CD_LoopCounter < 150 )
		return;

	CD_LoopCounter = 0;
	statusParams.dwItem = MCI_STATUS_MODE;
	rc = mciSendCommand(MciDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD_PTR)&statusParams);
	if( (rc == 0) && (statusParams.dwReturn == MCI_MODE_STOP) ) {
		playParams.dwFrom = CD_LoopTrack;
		playParams.dwTo = CD_LoopTrack + 1;
		mciSendCommand(MciDeviceID, MCI_PLAY, MCI_NOTIFY_FAILURE|MCI_NOTIFY_ABORTED, (DWORD_PTR)&playParams);
	}
}

void __cdecl S_CDPlay(__int16 trackID, BOOL isLooped) {
	__int16 track;
	MCI_PLAY_PARMS playParams;

#ifdef FEATURE_PAULD_CDAUDIO
	if( PaulD_isActive ) {
		PaulD_CDPlay(trackID, isLooped);
		return;
	}
#endif // FEATURE_PAULD_CDAUDIO

	if( MusicVolume == 0 )
		return;

	CD_TrackID = trackID;
	track = GetRealTrack(trackID);
	playParams.dwFrom = track;
	playParams.dwTo = track + 1;
	mciSendCommand(MciDeviceID, MCI_PLAY, MCI_NOTIFY_FAILURE|MCI_NOTIFY_ABORTED, (DWORD_PTR)&playParams);

	if( isLooped ) {
		CD_LoopTrack = track;
		CD_LoopCounter = 120;
	}
}

void __cdecl S_CDStop() {
	MCI_GENERIC_PARMS params;

#ifdef FEATURE_PAULD_CDAUDIO
	if( PaulD_isActive ) {
		PaulD_CDStop();
		return;
	}
#endif // FEATURE_PAULD_CDAUDIO

	if( CD_TrackID > 0 ) {
		mciSendCommand(MciDeviceID, MCI_STOP, 0, (DWORD_PTR)&params);
		CD_TrackID = 0;
		CD_LoopTrack = 0;
	}
}

BOOL __cdecl StartSyncedAudio(int trackID) {
	__int16 track;
	MCI_PLAY_PARMS playParams;
	MCI_SET_PARMS setParams;

#ifdef FEATURE_PAULD_CDAUDIO
	if( PaulD_isActive )
		return PaulD_StartSyncedAudio(trackID);
#endif // FEATURE_PAULD_CDAUDIO

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

#ifdef FEATURE_PAULD_CDAUDIO
	if( PaulD_isActive )
		return PaulD_CDGetLoc();
#endif // FEATURE_PAULD_CDAUDIO

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
	CDVolume = volume; // NOTE: store current CD Audio volume

#ifdef FEATURE_PAULD_CDAUDIO
	if( PaulD_isActive ) {
		PaulD_CDVolume(volume);
		return;
	}
#endif // FEATURE_PAULD_CDAUDIO

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

DWORD __cdecl S_GetCDVolume() {
	return CDVolume;
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
