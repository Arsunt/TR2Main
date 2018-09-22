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

static const WCHAR *GDI_Encoders[] = {
	L"image/bmp",
	L"image/jpeg",
	L"image/png",
};

static ULONG_PTR GDI_Token = 0;

static int GetEncoderClsid(const WCHAR *format, CLSID *pClsid) {
	unsigned int num = 0, nSize = 0;
	GetImageEncodersSize(&num, &nSize);
	if( nSize == 0 ) {
		return -1;
	}
	ImageCodecInfo *pImageCodecInfo = (ImageCodecInfo *)malloc(nSize);
	if( pImageCodecInfo == NULL ) {
		return -1;
	}
	GetImageEncoders(num, nSize, pImageCodecInfo);

	for( DWORD j = 0; j < num; ++j) {
		if(wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;
		}
	}
	free(pImageCodecInfo);
	return -1;
}

HBITMAP CreateBitmapFromDC(HDC dc, RECT *rect, LPVOID *lpBits, PALETTEENTRY *pal) {
	if( dc == NULL || rect == NULL || lpBits == NULL ) {
		return NULL; // wrong parameters
	}

	WORD nBPP = GetDeviceCaps(dc, BITSPIXEL);
	int width  = ABS(rect->right  - rect->left);
	int height = ABS(rect->bottom - rect->top);

	HDC hdcDestination = CreateCompatibleDC(dc);
	DWORD infoSize = sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD);
	BITMAPINFO *info = (BITMAPINFO *)malloc(infoSize);

	if( info == NULL ) {
		DeleteDC(hdcDestination);
		return NULL; // failed to create bitmap info
	}

	memset(info, 0, infoSize);
	info->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	info->bmiHeader.biWidth = width;
	info->bmiHeader.biHeight = -height;
	info->bmiHeader.biPlanes = 1;
	info->bmiHeader.biBitCount = nBPP;
	info->bmiHeader.biCompression = BI_RGB;

	if( pal != NULL ) {
		for( int i = 0; i < 256; ++i ) {
			info->bmiColors[i].rgbRed   = pal[i].peRed;
			info->bmiColors[i].rgbGreen = pal[i].peGreen;
			info->bmiColors[i].rgbBlue  = pal[i].peBlue;
		}
	}

	HBITMAP hbmDestination = CreateDIBSection(dc, info, DIB_RGB_COLORS, lpBits, NULL, 0);
	free(info);

	if( hbmDestination == NULL ) {
		DeleteDC(hdcDestination);
		return NULL; // failed to create bitmap
	}

	int stateDestination = SaveDC(hdcDestination);
	SelectObject(hdcDestination, hbmDestination);
	BitBlt(hdcDestination, 0, 0, width, height, dc, rect->left, rect->top, SRCCOPY);
	RestoreDC(hdcDestination, stateDestination);
	DeleteDC(hdcDestination);

	return hbmDestination;
}

bool __cdecl GDI_Init() {
	if( !GDI_Token ) {
		GdiplusStartupInput gdiplusStartupInput;
		GdiplusStartup(&GDI_Token, &gdiplusStartupInput, NULL);
	}
	return ( GDI_Token != 0 );
}

void __cdecl GDI_Cleanup() {
	if( GDI_Token ) {
		GdiplusShutdown(GDI_Token);
		GDI_Token = 0;
	}
}

int GDI_SaveImageFile(LPCSTR filename, GDI_FILEFMT format, DWORD quality, HBITMAP hbmBitmap) {
	if( filename == NULL || !*filename || hbmBitmap == NULL ) {
		return -1; // wrong parameters
	}

	if( format < 0 || format >= ARRAY_SIZE(GDI_Encoders) ) {
		return -1; // wrong format
	}

	Status status = Ok;

	CLSID imageCLSID;
	Bitmap *gdi_bitmap = new Bitmap(hbmBitmap, (HPALETTE)NULL);
	if( gdi_bitmap == NULL ) {
		return -1;
	}

	EncoderParameters encoderParams;
	encoderParams.Count = 1;
	encoderParams.Parameter[0].NumberOfValues = 1;
	encoderParams.Parameter[0].Guid  = EncoderQuality;
	encoderParams.Parameter[0].Type  = EncoderParameterValueTypeLong;
	encoderParams.Parameter[0].Value = &quality;
	GetEncoderClsid(GDI_Encoders[format], &imageCLSID);

#ifdef UNICODE
	status = gdi_bitmap->Save(filename, &imageCLSID, &encoderParams);
#else // !UNICODE
	{
		WCHAR wc_fname[MAX_PATH] = {0};
		if( !MultiByteToWideChar(CP_ACP, 0, filename, strlen(filename), wc_fname, MAX_PATH) ) {
			delete gdi_bitmap;
			return -1;
		}
		status = gdi_bitmap->Save(wc_fname, &imageCLSID, &encoderParams);
	}
#endif // UNICODE

	delete gdi_bitmap;
	return ( status == Ok ) ? 0 : -1;
}


int GDI_LoadImageBitmap(HBITMAP hbmBitmap, BYTE **bmPtr, DWORD *width, DWORD *height, DWORD bpp) {
	if( hbmBitmap == NULL || bmPtr == NULL || width == NULL || height == NULL ) {
		return -1; // wrong parameters
	}

	DWORD i;
	int result = 0;
	DWORD bmSize = 0;
	BYTE *src = NULL;
	BYTE *dst = NULL;
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

	Bitmap *gdi_bitmap = new Bitmap(hbmBitmap, (HPALETTE)NULL);
	if( gdi_bitmap == NULL ) {
		return -1;
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
	return result;
}

int GDI_LoadImageFile(LPCSTR filename, BYTE **bmPtr, DWORD *width, DWORD *height, DWORD bpp) {
	if( filename == NULL || !*filename || bmPtr == NULL || width == NULL || height == NULL ) {
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

#ifdef UNICODE
	gdi_bitmap = new Bitmap(filename);
#else // !UNICODE
	{
		WCHAR wc_fname[MAX_PATH] = {0};
		if( !MultiByteToWideChar(CP_ACP, 0, filename, strlen(filename), wc_fname, MAX_PATH) ) {
			// failed to get UNICODE filename
			result = -1;
			goto CLEANUP;
		}
		gdi_bitmap = new Bitmap(wc_fname);
	}
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
	return result;
}
