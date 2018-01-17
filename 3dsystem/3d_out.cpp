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
	int x0;
	int x1;
} XBUF_X;

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

int __cdecl xgen_x(__int16 *bufPtr) {
	int ptCount;
	POS_2D *pt1, *pt2;
	int yMin, yMax;
	int x1, y1, x2, y2;
	int xSize, ySize;
	int x, xAdd;
	XBUF_X *xPtr;

	ptCount = *bufPtr++;
	pt2 = (POS_2D *)bufPtr;
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
		else if ( y2 < y1 ) {
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
		return 0;

	XGen_y0 = yMin;
	XGen_y1 = yMax;
	return 1;
}

void __fastcall flatA(int y0, int y1, BYTE colorIdx) {
	int x0, x1;
	int xSize, ySize;
	BYTE *drawPtr, *linePtr;;
	XBUF_X *xbuf;

	ySize = y1 - y0;
	if( ySize <= 0 )
		return;

	xbuf = (XBUF_X *)XBuffer + y0;
	drawPtr = PrintSurfacePtr + y0 * PhdScreenWidth;

	do {
		x0 = xbuf->x0 / PHD_ONE;
		x1 = xbuf->x1 / PHD_ONE;
		++xbuf;

		xSize = x1 - x0;
		linePtr = drawPtr + x0;

		if( xSize > 0 ) {
			memset(linePtr, colorIdx, xSize);
		}
		drawPtr += PhdScreenWidth;
	} while( --ySize );
}

void __fastcall transA(int y0, int y1, BYTE depthQ) {
	int x0, x1;
	int xSize, ySize;
	BYTE *drawPtr, *linePtr;
	XBUF_X *xbuf;
	DEPTHQ_ENTRY *q;

	ySize = y1 - y0;
	if( ySize <= 0 || depthQ >= 32 ) // NOTE: depthQ check was ( > 32) in the original code
		return;

	xbuf = (XBUF_X *)XBuffer + y0;
	drawPtr = PrintSurfacePtr + y0 * PhdScreenWidth;
	q = DepthQTable + depthQ;

	do {
		x0 = xbuf->x0 / PHD_ONE;
		x1 = xbuf->x1 / PHD_ONE;
		++xbuf;

		xSize = x1 - x0;
		linePtr = drawPtr + x0;

		while( xSize-- ) {
			*linePtr = q->index[*linePtr];
			++linePtr;
		}
		drawPtr += PhdScreenWidth;
	} while( --ySize );
}

/*
 * Inject function
 */
void Inject_3Dout() {
	INJECT(0x00402960, draw_poly_line);
	INJECT(0x00402B00, draw_poly_flat);
	INJECT(0x00402B40, draw_poly_trans);

	INJECT(0x00402C40, xgen_x);

//	NOTE: asm functions below use Watcom register calling convention so they incompatible
//	INJECT(0x00457564, flatA);
//	INJECT(0x004575C5, transA);
}
