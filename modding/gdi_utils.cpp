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
#include "modding/gdi_utils.h"
#include "global/vars.h"
#include <gdiplus.h>

using namespace Gdiplus;

int GDI_LoadImageFile(LPCSTR filename, BYTE **bmPtr, DWORD *width, DWORD *height, DWORD bpp) {
	if( bmPtr == NULL || width == NULL || height == NULL ) {
		return -1; // wrong parameters
	}

	DWORD i;
	int result = 0;
	DWORD bmSize = 0;
	BYTE *src = NULL;
	BYTE *dst = NULL;
	Bitmap *gdi_bitmap = NULL;
	BitmapData bmData;
	Status status;
	PixelFormat pixelFmt;

	switch( bpp ) {
		case 32 :
			pixelFmt = PixelFormat32bppARGB;
			break;
		case 16 :
			pixelFmt = PixelFormat16bppARGB1555;
			break;
		default :
			// unsupported pixel format;
			return -1;
			break;
	}

	ULONG_PTR gdiplusToken = 0;
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

#ifdef UNICODE
	gdi_bitmap = new Bitmap(filename);
#else // !UNICODE
	WCHAR wc_fname[MAX_PATH];
	if( !MultiByteToWideChar(CP_UTF8, 0, filename, strlen(filename), wc_fname, MAX_PATH) ) {
		// failed to get UNICODE filename
		result = -1;
		goto CLEANUP;
	}
	gdi_bitmap = new Bitmap(wc_fname);
#endif // UNICODE

	if( gdi_bitmap == NULL ) {
		// failed to create gdi_bitmap
		result = -1;
		goto CLEANUP;
	}

	*width = gdi_bitmap->GetWidth();
	*height = gdi_bitmap->GetHeight();

	bmSize = (*width) * (*height) * (bpp/8);
	*bmPtr = (BYTE *)malloc(bmSize * (bpp/8));
	if( *bmPtr == NULL ) {
		// failed to allocate output bitmap
		result = -1;
		goto CLEANUP;
	}

	{ // rect is temporary here
		Rect rect(0, 0, *width, *height);
		status = gdi_bitmap->LockBits(&rect, ImageLockModeRead, pixelFmt, &bmData);
	}

	if( status != Ok ) {
		// failed to lock the bitmap
		printf("status = %d\n", status);
		free(bmPtr);
		result = -1;
		goto CLEANUP;
	}

	src = (BYTE *)bmData.Scan0;
	if( bmData.Stride < 0 ) {
		src += ABS(bmData.Stride) * (*height - 1);
	}

	dst = *bmPtr;
	for( i = 0; i < *height; ++i ) {
		memcpy(dst, src, (*width) * (bpp/8));
		dst += (*width) * (bpp/8);
		src += bmData.Stride;
	}

	gdi_bitmap->UnlockBits(&bmData);

CLEANUP :
	if( gdi_bitmap != NULL ) {
		delete gdi_bitmap;
		gdi_bitmap = NULL;
	}
	GdiplusShutdown(gdiplusToken);
	return result;
}
