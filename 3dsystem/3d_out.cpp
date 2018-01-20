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
#include "3dsystem/3d_out.h"
#include "global/vars.h"

typedef struct {
	UINT16 x;
	UINT16 y;
} XGEN_X;

typedef struct {
	UINT16 x;
	UINT16 y;
	UINT16 g;
} XGEN_XG;

typedef struct {
	UINT16 x;
	UINT16 y;
	UINT16 g;
	UINT16 u;
	UINT16 v;
} XGEN_XGUV;

typedef struct {
	int x0;
	int x1;
} XBUF_X;

typedef struct {
	int x0;
	int g0;
	int x1;
	int g1;
} XBUF_XG;

typedef struct {
	int x0;
	int g0;
	int u0;
	int v0;
	int x1;
	int g1;
	int u1;
	int v1;
} XBUF_XGUV;

void __cdecl draw_poly_line(__int16 *bufPtr) {
	int i, j;
	int x0, y0, x1, y1;
	int xSize, ySize, xAdd, yAdd, colAdd, rowAdd;
	int swapBuf, part, partTotal;
	BYTE colorIdx;
	BYTE *drawPtr;

	x0 = *(bufPtr++);
	y0 = *(bufPtr++);
	x1 = *(bufPtr++);
	y1 = *(bufPtr++);
	colorIdx = (BYTE)*bufPtr;

	if( x1 < x0 ) {
		SWAP(x0, x1, swapBuf);
		SWAP(y0, y1, swapBuf);
	}

	if( x1 < 0 || x0 > PhdWinMaxX )
		return;

	if( x0 < 0 ) {
		y0 -= x0 * (y1 - y0) / (x1 - x0);
		x0 = 0;
	}

	if( x1 > PhdWinMaxX ) {
		y1 = y0 + (y1 - y0) * (PhdWinMaxX - x0) / (x1 - x0);
		x1 = PhdWinMaxX;
	}

	if( y1 < y0 ) {
		SWAP(x0, x1, swapBuf);
		SWAP(y0, y1, swapBuf);
	}

	if( y1 < 0 || y0 > PhdWinMaxY )
		return;

	if( y0 < 0 ) {
		x0 -= y0 * (x1 - x0) / (y1 - y0);
		y0 = 0;
	}

	if( y1 > PhdWinMaxY ) {
		x1 = x0 + (x1 - x0) * (PhdWinMaxY - y0) / (y1 - y0);
		y1 = PhdWinMaxY;
	}

	drawPtr = PrintSurfacePtr + (PhdScreenWidth * y0 + x0);

	xSize = x1 - x0;
	ySize = y1 - y0;

	if( (xSize|ySize) == 0 ) {
		*drawPtr = colorIdx;
		return;
	}

	if( xSize < 0 ) {
		xSize = -xSize;
		xAdd = -1;
	} else {
		xAdd = 1;
	}

	if( ySize < 0 ) {
		ySize = -ySize;
		yAdd = -PhdScreenWidth;
	} else {
		yAdd = PhdScreenWidth;
	}

	if( xSize >= ySize ) {
		i = xSize + 1;
		j = ySize + 1;
		colAdd = xAdd;
		rowAdd = yAdd;
	} else {
		i = ySize + 1;
		j = xSize + 1;
		colAdd = yAdd;
		rowAdd = xAdd;
	}

	partTotal = 0;
	part = PHD_ONE * j / i;

	while( i-- ) {
		partTotal += part;
		*drawPtr = colorIdx;
		drawPtr += colAdd;
		if( partTotal >= PHD_ONE ) {
			drawPtr += rowAdd;
			partTotal -= PHD_ONE;
		}
	}
}

void __cdecl draw_poly_flat(__int16 *bufPtr) {
	if( xgen_x(bufPtr + 1) )
		flatA(XGen_y0, XGen_y1, *bufPtr);
}

void __cdecl draw_poly_trans(__int16 *bufPtr) {
	if( xgen_x(bufPtr + 1) )
		transA(XGen_y0, XGen_y1, *bufPtr);
}

void __cdecl draw_poly_gouraud(__int16 *bufPtr) {
	if( xgen_xg(bufPtr + 1) )
		gourA(XGen_y0, XGen_y1, *bufPtr);
}

void __cdecl draw_poly_gtmap(__int16 *bufPtr) {
	if( xgen_xguv(bufPtr + 1) )
		gtmapA(XGen_y0, XGen_y1, TexturePageBuffer8[*bufPtr]);
}

void __cdecl draw_poly_wgtmap(__int16 *bufPtr) {
	if( xgen_xguv(bufPtr + 1) )
		wgtmapA(XGen_y0, XGen_y1, TexturePageBuffer8[*bufPtr]);
}

BOOL __cdecl xgen_x(__int16 *bufPtr) {
	int ptCount;
	XGEN_X *pt1, *pt2;
	int yMin, yMax;
	int x1, y1, x2, y2;
	int xSize, ySize;
	int x, xAdd;
	XBUF_X *xPtr;

	ptCount = *bufPtr++;
	pt2 = (XGEN_X *)bufPtr;
	pt1 = pt2 + (ptCount - 1);

	yMin = yMax = pt1->y;

	while( ptCount-- ) {
		x1 = pt1->x;
		y1 = pt1->y;
		x2 = pt2->x;
		y2 = pt2->y;
		pt1 = pt2++;

		if( y1 < y2 ) {
			CLAMPG(yMin, y1);
			xSize = x2 - x1;
			ySize = y2 - y1;

			xPtr = (XBUF_X *)XBuffer + y1;
			xAdd = PHD_ONE * xSize / ySize;
			x = x1 * PHD_ONE + (PHD_ONE - 1);

			do {
				(xPtr++)->x1 = (x += xAdd);
			} while( --ySize );
		}
		else if( y2 < y1 ) {
			CLAMPL(yMax, y1);
			xSize = x1 - x2;
			ySize = y1 - y2;

			xPtr = (XBUF_X *)XBuffer + y2;
			xAdd = PHD_ONE * xSize / ySize;
			x = x2 * PHD_ONE + 1;

			do {
				(xPtr++)->x0 = (x += xAdd);
			} while( --ySize );
		}
	}

	if( yMin == yMax )
		return FALSE;

	XGen_y0 = yMin;
	XGen_y1 = yMax;
	return TRUE;
}

BOOL __cdecl xgen_xg(__int16 *bufPtr) {
	int ptCount;
	XGEN_XG *pt1, *pt2;
	int yMin, yMax;
	int x1, y1, g1, x2, y2, g2;
	int xSize, ySize, gSize;
	int x, g, xAdd, gAdd;
	XBUF_XG *xgPtr;

	ptCount = *bufPtr++;
	pt2 = (XGEN_XG *)bufPtr;
	pt1 = pt2 + (ptCount - 1);

	yMin = yMax = pt1->y;

	while( ptCount-- ) {
		x1 = pt1->x;
		y1 = pt1->y;
		g1 = pt1->g;
		x2 = pt2->x;
		y2 = pt2->y;
		g2 = pt2->g;
		pt1 = pt2++;

		if( y1 < y2 ) {
			CLAMPG(yMin, y1);
			xSize = x2 - x1;
			ySize = y2 - y1;
			gSize = g2 - g1;

			xgPtr = (XBUF_XG *)XBuffer + y1;
			xAdd = PHD_ONE * xSize / ySize;
			gAdd = PHD_HALF * gSize / ySize;
			x = x1 * PHD_ONE + (PHD_ONE - 1);
			g = g1 * PHD_HALF;

			do {
				xgPtr->x1 = (x += xAdd);
				xgPtr->g1 = (g += gAdd);
				xgPtr++;
			} while( --ySize );
		}
		else if( y2 < y1 ) {
			CLAMPL(yMax, y1);
			xSize = x1 - x2;
			ySize = y1 - y2;
			gSize = g1 - g2;

			xgPtr = (XBUF_XG *)XBuffer + y2;
			xAdd = PHD_ONE * xSize / ySize;
			gAdd = PHD_HALF * gSize / ySize;
			x = x2 * PHD_ONE + 1;
			g = g2 * PHD_HALF;

			do {
				xgPtr->x0 = (x += xAdd);
				xgPtr->g0 = (g += gAdd);
				xgPtr++;
			} while( --ySize );
		}
	}

	if( yMin == yMax )
		return FALSE;

	XGen_y0 = yMin;
	XGen_y1 = yMax;
	return TRUE;
}

BOOL __cdecl xgen_xguv(__int16 *bufPtr) {
	int ptCount;
	XGEN_XGUV *pt1, *pt2;
	int yMin, yMax;
	int x1, y1, g1, u1, v1, x2, y2, g2, u2, v2;
	int xSize, ySize, gSize, uSize, vSize;
	int x, g, u, v, xAdd, gAdd, uAdd, vAdd;
	XBUF_XGUV *xguvPtr;

	ptCount = *bufPtr++;
	pt2 = (XGEN_XGUV *)bufPtr;
	pt1 = pt2 + (ptCount - 1);

	yMin = yMax = pt1->y;

	while( ptCount-- ) {
		x1 = pt1->x;
		y1 = pt1->y;
		g1 = pt1->g;
		u1 = pt1->u;
		v1 = pt1->v;
		x2 = pt2->x;
		y2 = pt2->y;
		g2 = pt2->g;
		u2 = pt2->u;
		v2 = pt2->v;
		pt1 = pt2++;

		if( y1 < y2 ) {
			CLAMPG(yMin, y1);
			xSize = x2 - x1;
			ySize = y2 - y1;
			gSize = g2 - g1;
			uSize = u2 - u1;
			vSize = v2 - v1;

			xguvPtr = (XBUF_XGUV *)XBuffer + y1;
			xAdd = PHD_ONE * xSize / ySize;
			gAdd = PHD_HALF * gSize / ySize;
			uAdd = PHD_HALF * uSize / ySize;
			vAdd = PHD_HALF * vSize / ySize;
			x = x1 * PHD_ONE + (PHD_ONE - 1);
			g = g1 * PHD_HALF;
			u = u1 * PHD_HALF;
			v = v1 * PHD_HALF;

			do {
				xguvPtr->x1 = (x += xAdd);
				xguvPtr->g1 = (g += gAdd);
				xguvPtr->u1 = (u += uAdd);
				xguvPtr->v1 = (v += vAdd);
				xguvPtr++;
			} while( --ySize );
		}
		else if( y2 < y1 ) {
			CLAMPL(yMax, y1);
			xSize = x1 - x2;
			ySize = y1 - y2;
			gSize = g1 - g2;
			uSize = u1 - u2;
			vSize = v1 - v2;

			xguvPtr = (XBUF_XGUV *)XBuffer + y2;
			xAdd = PHD_ONE * xSize / ySize;
			gAdd = PHD_HALF * gSize / ySize;
			uAdd = PHD_HALF * uSize / ySize;
			vAdd = PHD_HALF * vSize / ySize;
			x = x2 * PHD_ONE + 1;
			g = g2 * PHD_HALF;
			u = u2 * PHD_HALF;
			v = v2 * PHD_HALF;

			do {
				xguvPtr->x0 = (x += xAdd);
				xguvPtr->g0 = (g += gAdd);
				xguvPtr->u0 = (u += uAdd);
				xguvPtr->v0 = (v += vAdd);
				xguvPtr++;
			} while( --ySize );
		}
	}

	if( yMin == yMax )
		return FALSE;

	XGen_y0 = yMin;
	XGen_y1 = yMax;
	return TRUE;
}

void __fastcall flatA(int y0, int y1, BYTE colorIdx) {
	int x, xSize, ySize;
	BYTE *drawPtr;
	XBUF_X *xbuf;

	ySize = y1 - y0;
	if( ySize <= 0 )
		return;

	xbuf = (XBUF_X *)XBuffer + y0;
	drawPtr = PrintSurfacePtr + y0 * PhdScreenWidth;

	for( ; ySize > 0; --ySize, ++xbuf, drawPtr += PhdScreenWidth ) {
		x = xbuf->x0 / PHD_ONE;
		xSize = (xbuf->x1 / PHD_ONE) - x;
		if( xSize > 0 ) {
			memset(drawPtr + x, colorIdx, xSize);
		}
	}
}

void __fastcall transA(int y0, int y1, BYTE depthQ) {
	int x, xSize, ySize;
	BYTE *drawPtr, *linePtr;
	XBUF_X *xbuf;
	DEPTHQ_ENTRY *qt;

	ySize = y1 - y0;
	if( ySize <= 0 || depthQ >= 32 ) // NOTE: depthQ check was ( > 32) in the original code
		return;

	xbuf = (XBUF_X *)XBuffer + y0;
	drawPtr = PrintSurfacePtr + y0 * PhdScreenWidth;
	qt = DepthQTable + depthQ;

	for( ; ySize > 0; --ySize, ++xbuf, drawPtr += PhdScreenWidth ) {
		x = xbuf->x0 / PHD_ONE;
		xSize = (xbuf->x1 / PHD_ONE) - x;
		if( xSize <= 0 )
			continue;

		linePtr = drawPtr + x;
		do {
			*linePtr = qt->index[*linePtr];
			++linePtr;
		} while( --xSize );
	}
}

void __fastcall gourA(int y0, int y1, BYTE colorIdx) {
	int x, xSize, ySize;
	int g, gAdd;
	BYTE bg;
	BYTE *drawPtr, *linePtr;
	XBUF_XG *xbuf;
	GOURAUD_ENTRY *gt;

	ySize = y1 - y0;
	if( ySize <= 0 )
		return;

	xbuf = (XBUF_XG *)XBuffer + y0;
	drawPtr = PrintSurfacePtr + y0 * PhdScreenWidth;
	gt = GouraudTable + colorIdx;

	for( ; ySize > 0; --ySize, ++xbuf, drawPtr += PhdScreenWidth ) {
		x = xbuf->x0 / PHD_ONE;
		xSize = (xbuf->x1 / PHD_ONE) - x;
		if( xSize <= 0 )
			continue;

		g = xbuf->g0;
		gAdd = (xbuf->g1 - g) / xSize;

		linePtr = drawPtr + x;
		do {
			bg = g / PHD_ONE;
			*(linePtr++) = gt->index[bg];
			g += gAdd;
		} while( --xSize );
	}
}

void __fastcall gtmapA(int y0, int y1, BYTE *texPage) {
	int x, xSize, ySize;
	int g, u, v, gAdd, uAdd, vAdd;
	BYTE bg, bu, bv;
	BYTE *drawPtr, *linePtr;
	XBUF_XGUV *xbuf;
	BYTE colorIdx;

	ySize = y1 - y0;
	if( ySize <= 0 )
		return;

	xbuf = (XBUF_XGUV *)XBuffer + y0;
	drawPtr = PrintSurfacePtr + y0 * PhdScreenWidth;

	for( ; ySize > 0; --ySize, ++xbuf, drawPtr += PhdScreenWidth ) {
		x = xbuf->x0 / PHD_ONE;
		xSize = (xbuf->x1 / PHD_ONE) - x;
		if( xSize <= 0 )
			continue;

		g = xbuf->g0;
		u = xbuf->u0;
		v = xbuf->v0;
		gAdd = (xbuf->g1 - g) / xSize;
		uAdd = (xbuf->u1 - u) / xSize;
		vAdd = (xbuf->v1 - v) / xSize;

		linePtr = drawPtr + x;
		do {
			bg = g / PHD_ONE;
			bu = u / PHD_ONE;
			bv = v / PHD_ONE;

			colorIdx = texPage[bv*256 + bu];
			*(linePtr++) = DepthQTable[bg].index[colorIdx];
			g += gAdd;
			u += uAdd;
			v += vAdd;
		} while( --xSize );
	}
}

void __fastcall wgtmapA(int y0, int y1, BYTE *texPage) {
	int x, xSize, ySize;
	int g, u, v, gAdd, uAdd, vAdd;
	BYTE bg, bu, bv;
	BYTE *drawPtr, *linePtr;
	XBUF_XGUV *xbuf;
	BYTE colorIdx;

	ySize = y1 - y0;
	if( ySize <= 0 )
		return;

	xbuf = (XBUF_XGUV *)XBuffer + y0;
	drawPtr = PrintSurfacePtr + y0 * PhdScreenWidth;

	for( ; ySize > 0; --ySize, ++xbuf, drawPtr += PhdScreenWidth ) {
		x = xbuf->x0 / PHD_ONE;
		xSize = (xbuf->x1 / PHD_ONE) - x;
		if( xSize <= 0 )
			continue;

		g = xbuf->g0;
		u = xbuf->u0;
		v = xbuf->v0;
		gAdd = (xbuf->g1 - g) / xSize;
		uAdd = (xbuf->u1 - u) / xSize;
		vAdd = (xbuf->v1 - v) / xSize;

		linePtr = drawPtr + x;
		do {
			bg = g / PHD_ONE;
			bu = u / PHD_ONE;
			bv = v / PHD_ONE;

			colorIdx = texPage[bv*256 + bu];
			if( colorIdx != 0 ) {
				*linePtr = DepthQTable[bg].index[colorIdx];
			}
			++linePtr;
			g += gAdd;
			u += uAdd;
			v += vAdd;
		} while( --xSize );
	}
}

/*
 * Inject function
 */
void Inject_3Dout() {
	INJECT(0x00402960, draw_poly_line);
	INJECT(0x00402B00, draw_poly_flat);
	INJECT(0x00402B40, draw_poly_trans);
	INJECT(0x00402B80, draw_poly_gouraud);
	INJECT(0x00402BC0, draw_poly_gtmap);
	INJECT(0x00402C00, draw_poly_wgtmap);
	INJECT(0x00402C40, xgen_x);
	INJECT(0x00402D20, xgen_xg);
	INJECT(0x00402E70, xgen_xguv);

//	NOTE: asm functions below use Watcom register calling convention so they incompatible
//	INJECT(0x00457564, flatA);
//	INJECT(0x004575C5, transA);
//	INJECT(0x004576FF, gourA);
//	INJECT(0x0045785F, gtmapA);
//	INJECT(0x00457B5C, wgtmapA);
}
