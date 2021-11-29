/*
 * Copyright (c) 2017-2021 Michael Chaban. All rights reserved.
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

#ifndef RAW_INPUT_H_INCLUDED
#define RAW_INPUT_H_INCLUDED

#include "global/types.h"
/*
 * This whole RawInput implementation is created for Sony DualShock 4 / DualSense, so it uses PlayStation button names
 */
typedef struct {
	LONG dPad; // -1 if unpressed, and 0-36000 if pressed
	float axisLX, axisLY, axisRX, axisRY, axisL2, axisR2;
	WORD btnSquare:1, btnCross:1, btnCircle:1, btnTriangle:1;
	WORD btnL1:1, btnR1:1, btnL2:1, btnR2:1, btnL3:1, btnR3:1;
	WORD btnShare:1, btnOptions:1, btnPS:1, btnTouch:1, btnReserved:2;
} RINPUT_STATE;

/*
 * Function list
 */
LPCTSTR GetRawInputName(WORD vid, WORD pid, BOOL bt);
bool RawInputEnumerate(BOOL(CALLBACK *callback)(LPGUID, LPCTSTR, LPCTSTR, WORD, WORD, LPVOID), LPVOID lpContext);
bool RawInputStart(LPCTSTR lpName);
void RawInputStop();
bool RawInputSetState(WORD leftMotor, WORD rightMotor, DWORD color);
bool RawInputGetState(RINPUT_STATE *pState);

#endif // RAW_INPUT_H_INCLUDED
