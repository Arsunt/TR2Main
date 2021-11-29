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

#ifndef XINPUT_EX_H_INCLUDED
#define XINPUT_EX_H_INCLUDED

#include "global/types.h"

#ifdef FEATURE_INPUT_IMPROVED
// all essential definitions are inside XInput.h, but no static lib linked here
#include <XInput.h>

#ifndef XINPUT_GAMEPAD_GUIDE
#define XINPUT_GAMEPAD_GUIDE (0x400)
#endif // XINPUT_GAMEPAD_GUIDE

#define XINPUT_DPAD(x) (CHK_ALL((x), XINPUT_GAMEPAD_DPAD_UP|XINPUT_GAMEPAD_DPAD_DOWN|XINPUT_GAMEPAD_DPAD_LEFT|XINPUT_GAMEPAD_DPAD_RIGHT))

BOOL IsXInputDevice(DWORD dwVendorId, DWORD dwProductId);
#endif // FEATURE_INPUT_IMPROVED

#endif // XINPUT_EX_H_INCLUDED
