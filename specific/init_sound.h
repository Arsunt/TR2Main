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

#ifndef INIT_SOUND_H_INCLUDED
#define INIT_SOUND_H_INCLUDED

#include "global/types.h"

#define VOLUME_PCT(x) (DSBVOLUME_MIN+(DSBVOLUME_MAX-DSBVOLUME_MIN)*(x)/100)

/*
 * Function list
 */
SOUND_ADAPTER_NODE *__cdecl GetSoundAdapter(GUID *lpGuid); // 0x00447C70
void __cdecl WinSndFreeAllSamples(); // 0x00447CC0
bool __cdecl WinSndMakeSample(DWORD sampleIdx, LPWAVEFORMATEX format, const LPVOID data, DWORD dataSize); // 0x00447CF0
bool __cdecl WinSndIsChannelPlaying(DWORD channel); // 0x00447E00
int __cdecl WinSndPlaySample(DWORD sampleIdx, int volume, DWORD pitch, int pan, DWORD flags); // 0x00447E50
int __cdecl WinSndGetFreeChannelIndex(); // 0x00447F40
void __cdecl WinSndAdjustVolumeAndPan(int channel, int volume, int pan); // 0x00447F80
void __cdecl WinSndAdjustPitch(int channel, DWORD pitch); // 0x00447FB0
void __cdecl WinSndStopSample(int channel); // 0x00447FF0
bool __cdecl WinSndInit(); // 0x00448060
bool __cdecl DSoundEnumerate(SOUND_ADAPTER_LIST *adapterList); // 0x00448100
BOOL CALLBACK DSoundEnumCallback(LPGUID lpGuid, LPCTSTR lpcstrDescription, LPCTSTR lpcstrModule, LPVOID lpContext); // 0x00448120
void __cdecl WinSndStart(HWND hWnd); // 0x00448210
bool __cdecl DSoundCreate(GUID *lpGuid); // 0x00448390
bool __cdecl DSoundBufferTest(); // 0x004483B0
void __cdecl WinSndFinish(); // 0x00448480
bool __cdecl WinSndIsSoundEnabled(); // 0x004484B0

#endif // INIT_SOUND_H_INCLUDED
