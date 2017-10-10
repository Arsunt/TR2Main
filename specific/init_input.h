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

#ifndef INIT_INPUT_H_INCLUDED
#define INIT_INPUT_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
bool __cdecl DInputCreate(); // 0x004472A0
void __cdecl DInputRelease(); // 0x004472D0
void __cdecl WinInReadKeyboard(LPVOID lpInputData); // 0x004472F0
DWORD __cdecl WinInReadJoystick(int *xPos, int *yPos); // 0x00447350
bool __cdecl WinInputInit(); // 0x00447460
bool __cdecl DInputEnumDevices(JOYSTICK_LIST *joystickList); // 0x004474E0
BOOL CALLBACK DInputEnumDevicesCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef); // 0x00447510
void __thiscall FlaggedStringCreate(STRING_FLAGGED *item, DWORD dwSize); // 0x00447600
JOYSTICK_NODE *__cdecl GetJoystick(GUID *lpGuid); // 0x00447620
void __cdecl DInputKeyboardCreate(); // 0x00447670
void __cdecl DInputKeyboardRelease(); // 0x00447740
bool __cdecl DInputJoystickSelect(); // 0x00447770
void __cdecl WinInStart(); // 0x00447860
void __cdecl WinInFinish(); // 0x00447890
void __cdecl WinInRunControlPanel(HWND hWnd); // 0x004478A0

#endif // INIT_INPUT_H_INCLUDED
