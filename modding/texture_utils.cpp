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
#include "modding/texture_utils.h"
#include "specific/texture.h"
#include "specific/utils.h"
#include "global/vars.h"

// This prevents texture bleeding instead of UV adjustment
static int FillEdgePadding(DWORD width, DWORD height, DWORD side, BYTE *bitmap, DWORD bpp) {
	if( !width || !height || width > side || height > side || bitmap == NULL ) {
		return -1;
	}
	switch( bpp ) {
		case  8 :
		case 16 :
		case 32 :
			break;
		default :
			return -1;
	}

	DWORD i;
	DWORD padRight = side - width;
	DWORD padBottom = side - height;

	if( padRight > 0 ) {
		switch( bpp ) {
			case  8 : {
				BYTE *p = (BYTE *)bitmap;
				for( i = 0; i < height ; ++i ) {
					p += width;
					p[0] = p[-1];
					p += padRight;
				}
				break;
			}
			case 16 : {
				UINT16 *p = (UINT16 *)bitmap;
				for( i = 0; i < height ; ++i ) {
					p += width;
					p[0] = p[-1];
					p += padRight;
				}
				break;
			}
			case 32 : {
				DWORD *p = (DWORD *)bitmap;
				for( i = 0; i < height ; ++i ) {
					p += width;
					p[0] = p[-1];
					p += padRight;
				}
				break;
			}
			default :
				break;
		}
	}

	if( padBottom > 0 ) {
		DWORD pitch = (width + padRight?1:0) * (bpp/8);
		BYTE *p = bitmap + height * pitch;
		memcpy(p, p - pitch, pitch);
		p += pitch;
	}

	return 0;
}

int MakeCustomTexture(DWORD x, DWORD y, DWORD width, DWORD height, DWORD pitch, DWORD side, BYTE *bitmap, RGB888 *bmpPal, int hwrPal, BYTE *swrBuf, bool keyColor) {
	int pageIndex = -1;
	if( bmpPal == NULL ) { // source bitmap is not indexed
		if( SavedAppSettings.RenderMode != RM_Hardware || TextureFormat.bpp < 16 ) { // texture cannot be indexed in this case
			return -1;
		}
		UINT16 *tmpBmp = (UINT16 *)calloc(2, SQR(side));
		UINT16 *bmpDst = tmpBmp;
		UINT16 *bmpSrc = (UINT16 *)bitmap + x + y * pitch;

		for( DWORD j = 0; j < height; ++j ) {
			for( DWORD i = 0; i < width; ++i ) {
				bmpDst[i] = bmpSrc[i];
			}
			bmpSrc += pitch;
			bmpDst += side;
		}
		FillEdgePadding(width, height, side, (BYTE *)tmpBmp, 16);
		pageIndex = AddTexturePage16(side, side, (BYTE *)tmpBmp);
		free(tmpBmp);
	} else if( SavedAppSettings.RenderMode == RM_Hardware && TextureFormat.bpp >= 16 ) {
		UINT16 *tmpBmp = (UINT16 *)calloc(2, SQR(side));
		UINT16 *bmpDst = tmpBmp;
		BYTE *bmpSrc = bitmap + x + y * pitch;

		// Translating bitmap data from 8 bit bitmap to 16 bit bitmap
		for( DWORD j = 0; j < height; ++j ) {
			for( DWORD i = 0; i < width; ++i ) {
				if( !keyColor || bmpSrc[i] ) {
					RGB888 *color = &bmpPal[bmpSrc[i]]; // get RGB color from palette
					bmpDst[i] = (1 << 15) // convert RGB to 16 bit
							| (((UINT16)color->red   >> 3) << 10)
							| (((UINT16)color->green >> 3) << 5)
							| (((UINT16)color->blue  >> 3));
				} else {
					bmpDst[i] = 0;
				}
			}
			bmpSrc += pitch;
			bmpDst += side;
		}
		FillEdgePadding(width, height, side, (BYTE *)tmpBmp, 16);
		pageIndex = AddTexturePage16(side, side, (BYTE *)tmpBmp);
		free(tmpBmp);
	} else if( SavedAppSettings.RenderMode == RM_Hardware ) {
		BYTE *tmpBmp = (BYTE *)calloc(1, SQR(side));
		UT_MemBlt(tmpBmp, 0, 0, width, height, side, bitmap, x, y, pitch);
		FillEdgePadding(width, height, side, tmpBmp, 8);
		pageIndex = AddTexturePage8(side, side, tmpBmp, hwrPal);
		free(tmpBmp);
	} else if( swrBuf != NULL && width == 256 && height == 256 && side == 256 ) {
		for( DWORD i=0; i<ARRAY_SIZE(TexturePageBuffer8); ++i ) {
			if( TexturePageBuffer8[i] == NULL || TexturePageBuffer8[i] == swrBuf ) {
				UT_MemBlt(swrBuf, 0, 0, width, height, side, bitmap, x, y, pitch);
				TexturePageBuffer8[i] = swrBuf;
				pageIndex = i;
				break;
			}
		}
	}
	return pageIndex;
}

