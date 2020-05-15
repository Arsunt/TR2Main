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
#include "3dsystem/scalespr.h"
#include "specific/output.h"
#include "global/vars.h"

#ifdef FEATURE_VIEW_IMPROVED
extern int CalculateFogShade(int depth);
#endif // FEATURE_VIEW_IMPROVED

void __cdecl S_DrawSprite(DWORD flags, int x, int y, int z, __int16 spriteIdx, __int16 shade, __int16 scale) {
	int xv, yv, zv, zp, depth;
	int x1, y1, x2, y2;

	if( CHK_ANY(flags, SPR_ABS) ) { // absolute coords
		x -= MatrixW2V._03;
		y -= MatrixW2V._13;
		z -= MatrixW2V._23;

		if( x < -PhdViewDistance || x > PhdViewDistance )
			return;
		if( y < -PhdViewDistance || y > PhdViewDistance)
			return;
		if( z < -PhdViewDistance || z > PhdViewDistance)
			return;

		zv = MatrixW2V._20 * x + MatrixW2V._21 * y + MatrixW2V._22 * z;
		if( zv < PhdNearZ || zv >= PhdFarZ )
			return;
		yv = MatrixW2V._10 * x + MatrixW2V._11 * y + MatrixW2V._12 * z;
		xv = MatrixW2V._00 * x + MatrixW2V._01 * y + MatrixW2V._02 * z;

	} else if( (x|y|z) == 0 ) { // zero point coords
		zv = PhdMatrixPtr->_23;
		if( zv < PhdNearZ || zv > PhdFarZ )
			return;
		yv = PhdMatrixPtr->_13;
		xv = PhdMatrixPtr->_03;

	} else { // relative coords
		zv = PhdMatrixPtr->_20 * x + PhdMatrixPtr->_21 * y + PhdMatrixPtr->_22 * z + PhdMatrixPtr->_23;
		if( zv < PhdNearZ || zv > PhdFarZ )
			return;
		yv = PhdMatrixPtr->_10 * x + PhdMatrixPtr->_11 * y + PhdMatrixPtr->_12 * z + PhdMatrixPtr->_13;
		xv = PhdMatrixPtr->_00 * x + PhdMatrixPtr->_01 * y + PhdMatrixPtr->_02 * z + PhdMatrixPtr->_03;
	}

	x1 = PhdSpriteInfo[spriteIdx].x1;
	y1 = PhdSpriteInfo[spriteIdx].y1;
	x2 = PhdSpriteInfo[spriteIdx].x2;
	y2 = PhdSpriteInfo[spriteIdx].y2;

	if( CHK_ANY(flags, SPR_SCALE) ) { // scaling required
		x1 = x1 * scale / PHD_ONE;
		y1 = y1 * scale / PHD_ONE;
		x2 = x2 * scale / PHD_ONE;
		y2 = y2 * scale / PHD_ONE;
	}

	x1 <<= W2V_SHIFT;
	y1 <<= W2V_SHIFT;
	x2 <<= W2V_SHIFT;
	y2 <<= W2V_SHIFT;

	zp = zv / PhdPersp;

	x1 = (x1 + xv) / zp + PhdWinCenterX;
	if( x1 >= PhdWinWidth )
		return;

	y1 = (y1 + yv) / zp + PhdWinCenterY;
	if( y1 >= PhdWinHeight )
		return;

	x2 = (x2 + xv) / zp + PhdWinCenterX;
	if( x2 < 0 )
		return;

	y2 = (y2 + yv) / zp + PhdWinCenterY;
	if( y2 < 0 )
		return;

	if( CHK_ANY(flags, SPR_SHADE) ) { // shading required
		depth = zv >> W2V_SHIFT;
#ifdef FEATURE_VIEW_IMPROVED
		if( depth > PhdViewDistance )
			return;

		shade += CalculateFogShade(depth);
		CLAMP(shade, 0, 0x1FFF);
#else // !FEATURE_VIEW_IMPROVED
		if( depth > DEPTHQ_START ) {
			shade += depth - DEPTHQ_START;
			if( shade > 0x1FFF ) {
				return;
			}
		}
#endif // FEATURE_VIEW_IMPROVED
	} else {
		shade = 0x1000;
	}

#ifdef FEATURE_VIDEOFX_IMPROVED
	ins_sprite(zv, x1, y1, x2, y2, spriteIdx, shade, flags);
#else // FEATURE_VIDEOFX_IMPROVED
	ins_sprite(zv, x1, y1, x2, y2, spriteIdx, shade);
#endif // FEATURE_VIDEOFX_IMPROVED
}

void __cdecl S_DrawPickup(int sx, int sy, int scale, __int16 spriteIdx, __int16 shade) {
	int x1, x2, y1, y2;

#ifdef FEATURE_HUD_IMPROVED
	scale = GetRenderScale(scale);
#endif // FEATURE_HUD_IMPROVED

	x1 = sx + (PhdSpriteInfo[spriteIdx].x1 * scale / PHD_ONE);
	x2 = sx + (PhdSpriteInfo[spriteIdx].x2 * scale / PHD_ONE);
	y1 = sy + (PhdSpriteInfo[spriteIdx].y1 * scale / PHD_ONE);
	y2 = sy + (PhdSpriteInfo[spriteIdx].y2 * scale / PHD_ONE);

	if( x2 >= 0 && y2 >= 0 && x1 < PhdWinWidth && y1 < PhdWinHeight ) {
#ifdef FEATURE_VIDEOFX_IMPROVED
		ins_sprite(200, x1, y1, x2, y2, spriteIdx, shade, 0);
#else // FEATURE_VIDEOFX_IMPROVED
		ins_sprite(200, x1, y1, x2, y2, spriteIdx, shade);
#endif // FEATURE_VIDEOFX_IMPROVED
	}
}

__int16 *__cdecl ins_room_sprite(__int16 *ptrObj, int vtxCount) {
	PHD_VBUF *vbuf;
	PHD_SPRITE *sprite;
	double zp;
	int x1, y1, x2, y2;

	for( int i = 0; i < vtxCount; ++i ) {
		vbuf = &PhdVBuf[*ptrObj];
		if( (vbuf->clip & 0x80) == 0 ) {
			sprite = &PhdSpriteInfo[ptrObj[1]];
			zp = (double)vbuf->zv / (double)PhdPersp;

			x1 = (int)((vbuf->xv + (double)((int)sprite->x1 << W2V_SHIFT)) / zp) + PhdWinCenterX;
			y1 = (int)((vbuf->yv + (double)((int)sprite->y1 << W2V_SHIFT)) / zp) + PhdWinCenterY;
			x2 = (int)((vbuf->xv + (double)((int)sprite->x2 << W2V_SHIFT)) / zp) + PhdWinCenterX;
			y2 = (int)((vbuf->yv + (double)((int)sprite->y2 << W2V_SHIFT)) / zp) + PhdWinCenterY;

			if( x2 >= PhdWinLeft && y2 >= PhdWinTop && x1 < PhdWinRight && y1 < PhdWinBottom ) {
#ifdef FEATURE_VIDEOFX_IMPROVED
				ins_sprite((int)vbuf->zv, x1, y1, x2, y2, ptrObj[1], vbuf->g, 0);
#else // FEATURE_VIDEOFX_IMPROVED
				ins_sprite((int)vbuf->zv, x1, y1, x2, y2, ptrObj[1], vbuf->g);
#endif // FEATURE_VIDEOFX_IMPROVED
			}
		}
		ptrObj += 2;
	}
	return ptrObj;
}

void __cdecl S_DrawScreenSprite2d(int sx, int sy, int sz, int scaleH, int scaleV, __int16 spriteIdx, __int16 shade, UINT16 flags) {
	int x1, y1, x2, y2, z;
	PHD_SPRITE *sprite = &PhdSpriteInfo[spriteIdx];

	x1 = sx + sprite->x1 * scaleH / PHD_ONE;
	y1 = sy + sprite->y1 * scaleV / PHD_ONE;
	x2 = sx + sprite->x2 * scaleH / PHD_ONE;
	y2 = sy + sprite->y2 * scaleV / PHD_ONE;
	z = PhdNearZ + sz * 8;

	if( x2 >= 0 && y2 >= 0 && x1 < PhdWinWidth && y1 < PhdWinHeight ) {
#ifdef FEATURE_VIDEOFX_IMPROVED
		ins_sprite(z, x1, y1, x2, y2, spriteIdx, shade, 0);
#else // FEATURE_VIDEOFX_IMPROVED
		ins_sprite(z, x1, y1, x2, y2, spriteIdx, shade);
#endif // FEATURE_VIDEOFX_IMPROVED
	}
}

void __cdecl S_DrawScreenSprite(int sx, int sy, int sz, int scaleH, int scaleV, __int16 spriteIdx, __int16 shade, UINT16 flags) {
	int x1, y1, x2, y2, z;
	PHD_SPRITE *sprite = &PhdSpriteInfo[spriteIdx];

	x1 = sx + (sprite->x1 / 8) * scaleH / PHD_ONE;
	y1 = sy + (sprite->y1 / 8) * scaleV / PHD_ONE;
	x2 = sx + (sprite->x2 / 8) * scaleH / PHD_ONE;
	y2 = sy + (sprite->y2 / 8) * scaleV / PHD_ONE;
	z = sz * 8;

	if( x2 >= 0 && y2 >= 0 && x1 < PhdWinWidth && y1 < PhdWinHeight ) {
#ifdef FEATURE_VIDEOFX_IMPROVED
		ins_sprite(z, x1, y1, x2, y2, spriteIdx, shade, 0);
#else // FEATURE_VIDEOFX_IMPROVED
		ins_sprite(z, x1, y1, x2, y2, spriteIdx, shade);
#endif // FEATURE_VIDEOFX_IMPROVED
	}
}

void __cdecl draw_scaled_spriteC(__int16 *ptrObj) {
	int i, j;
	int x1, y1, x2, y2, width, height;
	int u, uBase, vBase, uAdd, vAdd;
	__int16 sprIdx, shade;
	BYTE *srcBase, *src, *dst;
	BYTE pix;
	int dstAdd;
	bool isDepthQ;
	PHD_SPRITE *sprite;
	DEPTHQ_ENTRY *depthQ;

	x1 = ptrObj[0];
	y1 = ptrObj[1];
	x2 = ptrObj[2];
	y2 = ptrObj[3];
	sprIdx = ptrObj[4];
	shade = ptrObj[5];

	if( x1 >= x2 || y1 >= y2 || x2 <= 0 || y2 <= 0 || x1 >= PhdWinMaxX || y1 >= PhdWinMaxY )
		return;

	sprite = &PhdSpriteInfo[sprIdx];
	depthQ = &DepthQTable[shade >> 8];

	uBase = vBase = 0x4000;

	uAdd = (sprite->width - 64) * 256 / (x2 - x1);
	vAdd = (sprite->height - 64) * 256 / (y2 - y1);

	if( x1 < 0 ) {
		uBase -= uAdd * x1;
		x1 = 0;
	}

	if( y1 < 0 ) {
		vBase -= vAdd * y1;
		y1 = 0;
	}

	CLAMPG(x2, PhdWinMaxX + 1);
	CLAMPG(y2, PhdWinMaxY + 1);

	width = x2 - x1;
	height = y2 - y1;

	srcBase = (BYTE *)TexturePageBuffer8[sprite->texPage] + sprite->offset;
	dst = PrintSurfacePtr + (PhdWinMinY + y1) * PhdScreenWidth + (PhdWinMinX + x1);
	dstAdd = PhdScreenWidth - width;

	isDepthQ = (GameVid_IsWindowedVga || depthQ != &DepthQTable[15]); // NOTE: index was 16 in the original code, this was wrong

	for( i = 0; i < height; ++i ) {
		u = uBase;
		src = srcBase + (vBase >> 16) * 256;
		for( j = 0; j < width; ++j ) {
			pix = src[u >> 16];
			if( pix != 0 ) {
				*dst = ( isDepthQ ) ? depthQ->index[pix] : pix;
			}
			u += uAdd;
			++dst;
		}
		dst += dstAdd;
		vBase += vAdd;
	}
}

/*
 * Inject function
 */
void Inject_ScaleSpr() {
	INJECT(0x0040C030, S_DrawSprite);
	INJECT(0x0040C300, S_DrawPickup);
	INJECT(0x0040C390, ins_room_sprite);
	INJECT(0x0040C4F0, S_DrawScreenSprite2d);
	INJECT(0x0040C590, S_DrawScreenSprite);
	INJECT(0x0040C630, draw_scaled_spriteC);
}
