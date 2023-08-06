/*
 * Copyright (c) 2017-2020 Michael Chaban. All rights reserved.
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
#include "specific/display.h"
#include "3dsystem/3d_gen.h"
#include "specific/output.h"
#include "global/vars.h"

void __cdecl IncreaseScreenSize() {
	if( ScreenSizer < 1.0 ) {
		ScreenSizer += 0.08;
		if( ScreenSizer > 1.0 ) {
			ScreenSizer = 1.0;
		}
		GameSizer = ScreenSizer;
		setup_screen_size();
	}
}

void __cdecl DecreaseScreenSize() {
	if( ScreenSizer > 0.44 ) {
		ScreenSizer -= 0.08;
		if( ScreenSizer < 0.44 ) {
			ScreenSizer = 0.44;
		}
		GameSizer = ScreenSizer;
		setup_screen_size();
	}
}

void __cdecl setup_screen_size() {
	int wwidth, wheight;
	int xoff, yoff;

	wwidth = (int)((double)GameVidWidth * ScreenSizer);
	wheight = (int)((double)GameVidHeight * ScreenSizer);

	if( wwidth > GameVidWidth )
		wwidth = GameVidWidth;

	if( wheight > GameVidHeight )
		wheight = GameVidHeight;

	xoff = (GameVidWidth - wwidth) / 2;
	yoff = (GameVidHeight - wheight) / 2;

#if (DIRECT3D_VERSION >= 0x900)
	phd_InitWindow(xoff, yoff, wwidth, wheight, VIEW_NEAR, VIEW_FAR, 80, GameVidWidth, GameVidHeight);
#else // (DIRECT3D_VERSION >= 0x900)
	phd_InitWindow(xoff, yoff, wwidth, wheight, VIEW_NEAR, VIEW_FAR, 80, GameVidBufWidth, GameVidBufHeight);
#endif // (DIRECT3D_VERSION >= 0x900)

	DumpX = xoff;
	DumpY = yoff;
	DumpWidth = wwidth;
	DumpHeight = wheight;

	WinVidNeedToResetBuffers = true;
}

void __cdecl TempVideoAdjust(int hires, double sizer) {
	IsVidSizeLock = TRUE;
	if( ScreenSizer != sizer ) {
		ScreenSizer = sizer;
		setup_screen_size();
	}
}

void __cdecl TempVideoRemove() {
	IsVidSizeLock = FALSE;
	if( ScreenSizer != GameSizer ) {
		ScreenSizer = GameSizer;
		setup_screen_size();
	}
}

void __cdecl S_FadeInInventory(BOOL isFade) {
	if( InventoryMode != INV_TitleMode )
		S_CopyScreenToBuffer();

	if( isFade ) {
		FadeValue = 0x100000;
		FadeLimit = 0x180000;
		FadeAdder = +0x08000;
	}
}

void __cdecl S_FadeOutInventory(BOOL isFade) {
	if( isFade ) {
		FadeValue = 0x180000;
		FadeLimit = 0x100000;
		FadeAdder = -0x08000;
	}
}

/*
 * Inject function
 */
void Inject_Display() {
	INJECT(0x004478C0, IncreaseScreenSize);
	INJECT(0x00447930, DecreaseScreenSize);
	INJECT(0x004479A0, setup_screen_size);
	INJECT(0x00447A40, TempVideoAdjust);
	INJECT(0x00447A80, TempVideoRemove);
	INJECT(0x00447AC0, S_FadeInInventory);
	INJECT(0x00447B00, S_FadeOutInventory);
}
