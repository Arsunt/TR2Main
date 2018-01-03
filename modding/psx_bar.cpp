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
#include "modding/psx_bar.h"
#include "specific/hwr.h"
#include "global/vars.h"

#ifdef FEATURE_HEALTHBAR_IMPROVED

static D3DCOLOR InterpolateColor(D3DCOLOR color0, D3DCOLOR color1, DWORD value, DWORD range) {
	if( value == 0 )
		return color0;

	if( value == range )
		return color1;

	D3DCOLOR result = 0;
	BYTE *c0  = (BYTE *)&color0;
	BYTE *c1  = (BYTE *)&color1;
	BYTE *res = (BYTE *)&result;

	for( int i=0; i<4; ++i ) {
		res[i] = (DWORD)c0[i]*(range-value)/range + (DWORD)c1[i]*value/range;
	}
	return result;
}

static void __cdecl DrawCololoredRect(float sx0, float sy0, float sx1, float sy1, float z, D3DCOLOR color0, D3DCOLOR color1, D3DCOLOR color2, D3DCOLOR color3) {
	double sz, rhw;
	D3DTLVERTEX vertex[4];

	if( z > PhdFarZ ) {
		return;
	}
	if( z < PhdNearZ ) {
		z = PhdNearZ;
	}

	rhw = RhwFactor / (double)z;
	sz = FltResZBuf - rhw * FltResZORhw;

	vertex[0].sx = sx0;
	vertex[0].sy = sy0;
	vertex[0].color = color0;

	vertex[1].sx = sx1;
	vertex[1].sy = sy0;
	vertex[1].color = color1;

	vertex[2].sx = sx0;
	vertex[2].sy = sy1;
	vertex[2].color = color2;

	vertex[3].sx = sx1;
	vertex[3].sy = sy1;
	vertex[3].color = color3;

	for( int i=0; i<4; ++i ) {
		vertex[i].sz = sz;
		vertex[i].rhw = rhw;
		vertex[i].specular = 0;
	}

	HWR_TexSource(0);
	HWR_EnableColorKey(false);
	_Direct3DDevice2->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DVT_TLVERTEX, &vertex, 4, D3DDP_DONOTUPDATEEXTENTS|D3DDP_DONOTCLIP);
}

static void PSX_DrawBar(int x0, int y0, int x1, int y1, int bar, int pixel, D3DCOLOR *left, D3DCOLOR *right, D3DCOLOR *frame) {
	// The frame
	DrawCololoredRect(x0-pixel*3, y0-pixel*1, x1+pixel*3, y1+pixel*1, PhdNearZ + 40, frame[4], frame[5], frame[5], frame[4]);
	DrawCololoredRect(x0-pixel*2, y0-pixel*2, x1+pixel*2, y1+pixel*2, PhdNearZ + 30, frame[2], frame[3], frame[3], frame[2]);
	DrawCololoredRect(x0-pixel*1, y0-pixel*1, x1+pixel*1, y1+pixel*1, PhdNearZ + 20, frame[0], frame[1], frame[1], frame[0]);

	// The bar
	if( bar > 0 ) {
		int i;
		int dy[4];
		D3DCOLOR dl[4], dr[4];
		int dh = (y1 - y0) - pixel * 2;

		for( i=0; i<4; ++i ) {
			dy[i] = dh * i / 3;
			if( i>0 && i<3 ) {
				dl[i] = InterpolateColor(left[i-1],  left[i],  dy[i], dh);
				dr[i] = InterpolateColor(right[i-1], right[i], dy[i], dh);
			} else {
				dl[i] = left[i];
				dr[i] = right[i];
			}
		}

		for( i=0; i<3; ++i ) {
			DrawCololoredRect(x0, y1-dy[i+1], x0+bar, y1-dy[i], PhdNearZ + 10, dl[i+1], dr[i+1], dl[i], dr[i]);
		}

		DrawCololoredRect(x0, y0+pixel*0, x0+bar, y0+pixel*1, PhdNearZ + 10, left[2], right[2], left[3], right[3]);
		DrawCololoredRect(x0, y0+pixel*1, x0+bar, y0+pixel*2, PhdNearZ + 10, left[5], right[5], left[4], right[4]);
	}
}

void __cdecl PSX_DrawHealthBar(int x0, int y0, int x1, int y1, int bar, int pixel) {
	D3DCOLOR left[6]  = {0xFF680000, 0xFF700000, 0xFF980000, 0xFFD80000, 0xFFE40000, 0xFFF00000};
	D3DCOLOR right[6] = {0xFF004400, 0xFF007400, 0xFF009C00, 0xFF00D400, 0xFF00E800, 0xFF00FC00};
	D3DCOLOR frame[6] = {0xFF000000, 0xFF000000, 0xFF508484, 0xFFA0A0A0, 0xFF284242, 0xFF505050};

	for( int i=0; i<6; ++i )
		right[i] = InterpolateColor(left[i], right[i], bar, x1-x0);

	PSX_DrawBar(x0, y0, x1, y1, bar, pixel, left, right, frame);
}

void __cdecl PSX_DrawAirBar(int x0, int y0, int x1, int y1, int bar, int pixel) {
	D3DCOLOR left[6]  = {0xFF004054, 0xFF005064, 0xFF006874, 0xFF007884, 0xFF00848E, 0xFF009098};
	D3DCOLOR right[6] = {0xFF004000, 0xFF005000, 0xFF006800, 0xFF007800, 0xFF008400, 0xFF009000};
	D3DCOLOR frame[6] = {0xFF000000, 0xFF000000, 0xFF508484, 0xFFA0A0A0, 0xFF284242, 0xFF505050};

	for( int i=0; i<6; ++i )
		right[i] = InterpolateColor(left[i], right[i], bar, x1-x0);

	PSX_DrawBar(x0, y0, x1, y1, bar, pixel, left, right, frame);
}

#endif // FEATURE_HEALTHBAR_IMPROVED
