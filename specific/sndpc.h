/*
 * Copyright (c) 2017 Michael Chaban. All rights reserved.
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

#ifndef SNDPC_H_INCLUDED
#define SNDPC_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
int __cdecl S_SoundPlaySample(int channel, UINT16 volume, int pitch, int pan); // 0x004553B0
int __cdecl S_Sound_CalculateSampleVolume(DWORD volume); // 0x00455400
int __cdecl S_Sound_CalculateSamplePan(__int16 pan); // 0x00455430
int __cdecl S_SoundPlaySampleLooped(int channel, UINT16 volume, DWORD pitch, int pan); // 0x00455460
void __cdecl S_SoundSetPanAndVolume(int channel, int pan, UINT16 volume); // 0x004554B0
void __cdecl S_SoundSetPitch(int channel, DWORD pitch); // 0x004554F0
void __cdecl S_SoundSetMasterVolume(DWORD volume); // 0x00455510
void __cdecl S_SoundStopSample(int channel); // 0x00455520
void __cdecl S_SoundStopAllSamples(); // 0x00455540
BOOL __cdecl S_SoundSampleIsPlaying(int channel); // 0x00455550
bool __cdecl CD_Init(); // 0x00455570
void __cdecl CD_Cleanup(); // 0x00455600
void __cdecl S_CDLoop(); // 0x00455640
void __cdecl S_CDPlay(__int16 trackID, BOOL isLooped); // 0x004556E0
void __cdecl S_CDStop(); // 0x00455760
BOOL __cdecl StartSyncedAudio(int trackID); // 0x004557A0
DWORD __cdecl S_CDGetLoc(); // 0x00455830
void __cdecl S_CDVolume(DWORD volume); // 0x004558A0
DWORD __cdecl S_GetCDVolume(); // NOTE: this function is not presented in the original game

#endif // SNDPC_H_INCLUDED
