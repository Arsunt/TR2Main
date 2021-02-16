/*
 * Copyright (c) 2017-2020 Michael Chaban. All rights reserved.
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

#ifndef TR2MAIN_PRECOMPILED_HEADER
#define TR2MAIN_PRECOMPILED_HEADER

// There is no DirectInput above 8.0
#if (DIRECTINPUT_VERSION > 0x800)
#undef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION (0x800)
#endif // (DIRECTINPUT_VERSION > 0x800)

// There is no DirectSound above 9.0
#if (DIRECTSOUND_VERSION > 0x900)
#undef DIRECTSOUND_VERSION
#define DIRECTSOUND_VERSION (0x900)
#endif // (DIRECTSOUND_VERSION > 0x900)

// Some defines are required for DX9 and above
#if (DIRECT3D_VERSION >= 0x900)
#ifndef FEATURE_NOLEGACY_OPTIONS
#define FEATURE_NOLEGACY_OPTIONS
#endif // !FEATURE_NOLEGACY_OPTIONS
#endif // (DIRECT3D_VERSION >= 0x900)

#include <stdio.h>
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <dinput.h>
#include <dsound.h>

#if (DIRECT3D_VERSION >= 0x900)
#include <d3d9.h>
#include <d3dx9.h>
#else // (DIRECT3D_VERSION >= 0x900)
#include <ddraw.h>
#include <d3d.h>
#endif // (DIRECT3D_VERSION >= 0x900)
#endif // TR2MAIN_PRECOMPILED_HEADER
