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

#include "global/precompiled.h"
#include "specific/frontend.h"
#include "specific/display.h"
#include "specific/fmv.h"
#include "specific/input.h"
#include "specific/output.h"
#include "specific/texture.h"
#include "specific/utils.h"
#include "global/vars.h"

#ifdef FEATURE_BACKGROUND_IMPROVED
#include "modding/background_new.h"
#endif // FEATURE_BACKGROUND_IMPROVED

#ifdef FEATURE_HUD_IMPROVED
#include "3dsystem/3dinsert.h"

DWORD InvTextBoxMode = 0;
#endif // FEATURE_HUD_IMPROVED

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
	int adder;
#ifdef FEATURE_HUD_IMPROVED
	if( SavedAppSettings.RenderMode == RM_Hardware && InvTextBoxMode ) {
		adder = GetRenderScale(1);
		sx += adder;
		sy += adder;
	} else {
		adder = GetRenderScale(2);
	}
#else // !FEATURE_HUD_IMPROVED
	// NOTE: in the original code there was no adder at all for this function
	adder = 2;
#endif // FEATURE_HUD_IMPROVED
	width += adder;
	height += adder;

#ifdef FEATURE_HUD_IMPROVED
	if( SavedAppSettings.RenderMode == RM_Hardware && InvTextBoxMode && gour != NULL ) {
		int x[3], y[3], lw;
		int sz = PhdNearZ + z * 8;

		lw = GetRenderScale(1);
		x[0] = sx;
		y[0] = sy;
		x[2] = sx + width;
		y[2] = sy + height;
		x[1] = (x[0] + x[2]) / 2;
		y[1] = (y[0] + y[2]) / 2;

		InsertGourQuad(x[0]-lw, y[0]-lw, x[1], y[0], sz,
						gour->clr[0], gour->clr[1],
						gour->clr[1], gour->clr[0]);
		InsertGourQuad(x[1], y[0]-lw, x[2], y[0], sz,
						gour->clr[1], gour->clr[2],
						gour->clr[2], gour->clr[1]);

		InsertGourQuad(x[2], y[0]-lw, x[2]+lw, y[1], sz,
						gour->clr[2], gour->clr[2],
						gour->clr[3], gour->clr[3]);
		InsertGourQuad(x[2], y[1], x[2]+lw, y[2], sz,
						gour->clr[3], gour->clr[3],
						gour->clr[4], gour->clr[4]);

		InsertGourQuad(x[1], y[2], x[2]+lw, y[2]+lw, sz,
						gour->clr[5], gour->clr[4],
						gour->clr[4], gour->clr[5]);
		InsertGourQuad(x[0], y[2], x[1], y[2]+lw, sz,
						gour->clr[6], gour->clr[5],
						gour->clr[5], gour->clr[6]);

		InsertGourQuad(x[0]-lw, y[1], x[0], y[2]+lw, sz,
						gour->clr[7], gour->clr[7],
						gour->clr[6], gour->clr[6]);
		InsertGourQuad(x[0]-lw, y[0], x[0], y[1], sz,
						gour->clr[8], gour->clr[8],
						gour->clr[7], gour->clr[7]);
		return;
	}
#endif // FEATURE_HUD_IMPROVED
	const BYTE colorIdx1 = 15;
	const BYTE colorIdx2 = 31;
	int sx1 = sx + width;
	int sy1 = sy + height;

#ifdef FEATURE_HUD_IMPROVED
	int pixel = GetRenderScale(1);

	ins_flat_rect(sx+pixel*0, sy-pixel*1, sx1+pixel*2, sy-pixel*0, z, colorIdx1);
	ins_flat_rect(sx+pixel*1, sy-pixel*0, sx1+pixel*1, sy+pixel*1, z, colorIdx2);

	ins_flat_rect(sx1+pixel*0, sy+pixel*1, sx1+pixel*1, sy1+pixel*1, z, colorIdx1);
	ins_flat_rect(sx1+pixel*1, sy+pixel*0, sx1+pixel*2, sy1+pixel*2, z, colorIdx2);

	ins_flat_rect(sx-pixel*1, sy-pixel*1, sx+pixel*0, sy1+pixel*1, z, colorIdx1);
	ins_flat_rect(sx-pixel*0, sy-pixel*0, sx+pixel*1, sy1+pixel*0, z, colorIdx2);

	ins_flat_rect(sx-pixel*0, sy1+pixel*0, sx1+pixel*0, sy1+pixel*1, z, colorIdx1);
	ins_flat_rect(sx-pixel*1, sy1+pixel*1, sx1+pixel*1, sy1+pixel*2, z, colorIdx2);
#else // !FEATURE_HUD_IMPROVED
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
#endif // !FEATURE_HUD_IMPROVED
}

void __cdecl S_DrawScreenFBox(int sx, int sy, int z, int width, int height, BYTE colorIdx, GOURAUD_FILL *gour, UINT16 flags) {
	int adder;
	int sz = PhdNearZ + z * 8;
#ifdef FEATURE_HUD_IMPROVED
	if( SavedAppSettings.RenderMode == RM_Hardware && InvTextBoxMode ) {
		adder = GetRenderScale(1);
		sx += adder;
		sy += adder;
	} else {
		adder = GetRenderScale(2);
	}
#else // !FEATURE_HUD_IMPROVED
	// NOTE: in the original code the adder was 1, but 1 is insufficient,
	// because there was visible gap between FBox and bottom/right Frame
	adder = 2;
#endif // FEATURE_HUD_IMPROVED
	width += adder;
	height += adder;

#ifdef FEATURE_HUD_IMPROVED
	if( SavedAppSettings.RenderMode == RM_Hardware && InvTextBoxMode && gour != NULL ) {
		int x[3], y[3];

		x[0] = sx;
		y[0] = sy;
		x[2] = sx + width;
		y[2] = sy + height;
		x[1] = (x[0] + x[2]) / 2;
		y[1] = (y[0] + y[2]) / 2;

		InsertGourQuad(x[0], y[0], x[1], y[1], sz,
						gour->clr[0][0], gour->clr[0][1],
						gour->clr[0][2], gour->clr[0][3]);
		InsertGourQuad(x[1], y[0], x[2], y[1], sz,
						gour->clr[1][0], gour->clr[1][1],
						gour->clr[1][2], gour->clr[1][3]);
		InsertGourQuad(x[1], y[1], x[2], y[2], sz,
						gour->clr[2][0], gour->clr[2][1],
						gour->clr[2][2], gour->clr[2][3]);
		InsertGourQuad(x[0], y[1], x[1], y[2], sz,
						gour->clr[3][0], gour->clr[3][1],
						gour->clr[3][2], gour->clr[3][3]);
		return;
	}
#endif // FEATURE_HUD_IMPROVED
	ins_trans_quad(sx, sy, width, height, sz);
}

void __cdecl S_FinishInventory() {
	if( InventoryMode != INV_TitleMode )
		TempVideoRemove();
}

void __cdecl S_FadeToBlack() {
#ifdef FEATURE_BACKGROUND_IMPROVED
	if( SavedAppSettings.RenderMode == RM_Hardware ) {
		S_CopyScreenToBuffer();
		BGND2_ShowPicture(0, 0, 10, 2, FALSE);
		return;
	}
#endif // FEATURE_BACKGROUND_IMPROVED
	memset(GamePalette8, 0, sizeof(GamePalette8));
	FadeToPal(10, GamePalette8);
	FadeWait();

	// make two blank frames
	ScreenClear(false); ScreenDump();
	ScreenClear(false); ScreenDump();
}

void __cdecl S_Wait(int timeout, BOOL inputCheck) {
	// Wait for key event to clear or timeout
	for( ; timeout > 0; --timeout ) {
		if( !inputCheck || InputStatus == 0 )
			break;
		S_UpdateInput();
		if( IsGameToExit ) return; // NOTE: this line is not in the original game
		SyncTicks(1); // NOTE: there was another code in the original game
	}

	// Wait for key event to set or timeout
	for( ; timeout > 0; --timeout ) {
		S_UpdateInput();
		if( IsGameToExit ) return; // NOTE: this line is not in the original game
		if( inputCheck && InputStatus != 0 )
			break;
		SyncTicks(1); // NOTE: there was another code in the original game
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
