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

/*
 * Inject function
 */
void Inject_3Dout() {
	INJECT(0x00402960, draw_poly_line);
}
