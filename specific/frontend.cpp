/*
 * Copyright (c) 2017-2018 Michael Chaban. All rights reserved.
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
#include "specific/frontend.h"
#include "specific/display.h"
#include "specific/fmv.h"
#include "specific/input.h"
#include "specific/output.h"
#include "specific/texture.h"
#include "specific/utils.h"
#include "global/vars.h"

static void FadeWait() {
	// Null function
}

UINT16 __cdecl S_COLOUR(int red, int green, int blue) {
	return FindNearestPaletteEntry(GamePalette8, red, green, blue, false);
}

void __cdecl S_DrawScreenLine(int x, int y, int z, int xLen, int yLen, BYTE colorIdx, D3DCOLOR *gour, UINT16 flags) {
	ins_line(x, y, x + xLen, y + yLen, PhdNearZ + z * 8, colorIdx);
}

void __cdecl S_DrawScreenBox(int sx, int sy, int z, int width, int height, BYTE colorIdx, GOURAUD_OUTLINE *gour, UINT16 flags) {
	const BYTE colorIdx1 = 15;
	const BYTE colorIdx2 = 31;
	int sx1 = sx + width;
	int sy1 = sy + height;

	// NOTE: line coordinates slightly adjusted to fill gaps in the box corners (original game HWR bug)
	if( SavedAppSettings.RenderMode == RM_Hardware ) {
		++width;
		++height;
	}
	S_DrawScreenLine(sx,	sy-1,	z,	width+1,	0,			colorIdx1, NULL, flags);
	S_DrawScreenLine(sx+1,	sy,		z,	width-1,	0,			colorIdx2, NULL, flags);

	S_DrawScreenLine(sx1,	sy+1,	z,	0,			height-1,	colorIdx1, NULL, flags);
	S_DrawScreenLine(sx1+1,	sy,		z,	0,			height+1,	colorIdx2, NULL, flags);

	S_DrawScreenLine(sx-1,	sy-1,	z,	0,			height+1,	colorIdx1, NULL, flags);
	S_DrawScreenLine(sx,	sy,		z,	0,			height-1,	colorIdx2, NULL, flags);

	S_DrawScreenLine(sx,	sy1,	z,	width-1,	0,			colorIdx1, NULL, flags);
	S_DrawScreenLine(sx-1,	sy1+1,	z,	width+1,	0,			colorIdx2, NULL, flags);
}

void __cdecl S_DrawScreenFBox(int sx, int sy, int z, int width, int height, BYTE colorIdx, GOURAUD_FILL *gour, UINT16 flags) {
	int adder;
#ifdef FEATURE_FOV_FIX
	adder = GetRenderScale(2);
#else // !FEATURE_FOV_FIX
	// NOTE: in the original code the adder was 1, but 1 is insufficient,
	// because there was visible gap between FBox and bottom/right Frame
	adder = 2;
#endif // FEATURE_FOV_FIX
	ins_trans_quad(sx, sy, width + adder, height + adder, PhdNearZ + z * 8);
}

void __cdecl S_FinishInventory() {
	if( InventoryMode != INV_TitleMode )
		TempVideoRemove();
}

void __cdecl S_FadeToBlack() {
	memset(GamePalette8, 0, sizeof(RGB888)*256);
	FadeToPal(10, GamePalette8);
	FadeWait();

	// make two blank frames
	ScreenClear(false); ScreenDump();
	ScreenClear(false); ScreenDump();
}

void __cdecl S_Wait(int timeout, BOOL inputCheck) {
	DWORD ticks;

	// Wait for key event to clear or timeout
	for( ; timeout > 0; timeout -= ticks ) {
		if( !inputCheck || InputStatus == 0 )
			break;
		S_UpdateInput();
		if( IsGameToExit ) return; // NOTE: this line is not in the original game
		while( 0 == (ticks = Sync()) ) /* just wait a tick */;
	}

	// Wait for key event to set or timeout
	for( ; timeout > 0; timeout -= ticks ) {
		S_UpdateInput();
		if( IsGameToExit ) return; // NOTE: this line is not in the original game
		if( inputCheck && InputStatus != 0 )
			break;
		while( 0 == (ticks = Sync()) ) /* just wait a tick */;
	}
}

bool __cdecl S_PlayFMV(LPCTSTR fileName) {
	return PlayFMV(fileName);
}

bool __cdecl S_IntroFMV(LPCTSTR fileName1, LPCTSTR fileName2) {
	return IntroFMV(fileName1, fileName2);
}

/*
 * Inject function
 */
void Inject_Frontend() {
	INJECT(0x0044C2A0, S_COLOUR);
	INJECT(0x0044C2D0, S_DrawScreenLine);
	INJECT(0x0044C310, S_DrawScreenBox);
	INJECT(0x0044C430, S_DrawScreenFBox);
	INJECT(0x0044C460, S_FinishInventory);
	INJECT(0x0044C470, S_FadeToBlack);
	INJECT(0x0044C4C0, S_Wait);
	INJECT(0x0044C520, S_PlayFMV);
	INJECT(0x0044C530, S_IntroFMV);
}
