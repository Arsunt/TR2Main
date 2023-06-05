/*
 * Copyright (c) 2017 Michael Chaban. All rights reserved.
 * Original game is created by Core Design Ltd. in 1997.
 * Lara Croft and Tomb Raider are trademarks of Embracer Group AB.
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
#include "specific/setupwnd.h"
#include "specific/utils.h"
#include "global/vars.h"
#include <cpl.h>

#define WINLONG_X	(0)
#define WINLONG_Y	(4)
#define WINLONG_BMP	(8)

static void SE_CenterBitmapResource(HWND hWnd, BITMAP_RESOURCE *pBitmap, LPCTSTR imageName) {
	int x, y, width, height;

	if( pBitmap == NULL || imageName == NULL || *imageName == 0 )
		return;

	SE_LoadBitmapResource(pBitmap, imageName);

	x = (int)GetWindowLong(hWnd, WINLONG_X);
	y = (int)GetWindowLong(hWnd, WINLONG_Y);
	width = pBitmap->bmpInfo->bmiHeader.biWidth + 4;
	height = pBitmap->bmpInfo->bmiHeader.biHeight + 4;

	MoveWindow(hWnd, x-width/2, y-height/2, width, height, TRUE);
}

void __thiscall SE_ReleaseBitmapResource(BITMAP_RESOURCE *bmpRsrc) {
	if( bmpRsrc->hPalette != NULL ) {
		DeleteObject(bmpRsrc->hPalette);
		bmpRsrc->hPalette = NULL;
	}

	if( bmpRsrc->bmpData != NULL && (bmpRsrc->flags & 1) != 0 ) {
		free(bmpRsrc->bmpData);
		bmpRsrc->bmpData = NULL;
	}
}

void __thiscall SE_LoadBitmapResource(BITMAP_RESOURCE *bmpRsrc, LPCTSTR lpName) {
	static LOGPALETTE *logPalette = (LOGPALETTE *)malloc(sizeof(LOGPALETTE) + sizeof(PALETTEENTRY)*255);
	PALETTEENTRY *bmpPalette;
	BYTE *bitmap;

	if( bmpRsrc->hPalette != NULL ) {
		DeleteObject(bmpRsrc->hPalette);
		bmpRsrc->hPalette = NULL;
	}

	bitmap = (BYTE *)UT_LoadResource(lpName, RT_BITMAP);
	bmpRsrc->bmpInfo = (PBITMAPINFO)bitmap;
	bmpRsrc->flags = 0;

	if( bitmap == NULL )
		return;

	bmpPalette = (PALETTEENTRY *)(bitmap + sizeof(BITMAPINFOHEADER));
	bmpRsrc->bmpData = bitmap + sizeof(BITMAPINFOHEADER) + sizeof(PALETTEENTRY)*256;

	logPalette->palVersion = 0x0300;
	logPalette->palNumEntries = 256;

	for( int i=0; i<256; ++i ) {
		logPalette->palPalEntry[i].peRed	= bmpPalette[i].peRed;
		logPalette->palPalEntry[i].peGreen	= bmpPalette[i].peGreen;
		logPalette->palPalEntry[i].peBlue	= bmpPalette[i].peBlue;
		logPalette->palPalEntry[i].peFlags	= (i>=10 && i<246) ? PC_NOCOLLAPSE : 0;
	}
	bmpRsrc->hPalette = CreatePalette(logPalette);
}

void __thiscall SE_DrawBitmap(BITMAP_RESOURCE *bmpRsrc, HDC hdc, int x, int y) {
	int width, height;
	HPALETTE hPalette;

	if( bmpRsrc->hPalette == NULL)
		return;

	width = bmpRsrc->bmpInfo->bmiHeader.biWidth;
	height = bmpRsrc->bmpInfo->bmiHeader.biHeight;

	hPalette = SelectPalette(hdc, bmpRsrc->hPalette, FALSE);
	RealizePalette(hdc);
	SetDIBitsToDevice(hdc, x, y, width, height, 0, 0, 0, height, bmpRsrc->bmpData, bmpRsrc->bmpInfo, DIB_RGB_COLORS);
	SelectPalette(hdc, hPalette, FALSE);
}

void __thiscall SE_UpdateBitmapPalette(BITMAP_RESOURCE *bmpRsrc, HWND hWnd, HWND hSender) {
	if( bmpRsrc->hPalette != NULL && hSender != hWnd )
		SE_ChangeBitmapPalette(bmpRsrc, hWnd);
}

void __thiscall SE_ChangeBitmapPalette(BITMAP_RESOURCE *bmpRsrc, HWND hWnd) {
	HDC hdc;
	HPALETTE hPalette;

	if( bmpRsrc->hPalette == NULL)
		return;

	hdc = GetDC(hWnd);
	hPalette = SelectPalette(hdc, bmpRsrc->hPalette, FALSE);
	UnrealizeObject(bmpRsrc->hPalette);
	RealizePalette(hdc);
	SelectPalette(hdc, hPalette, FALSE);
	ReleaseDC(hWnd, hdc);
}

bool __cdecl SE_RegisterSetupWindowClass() {
	WNDCLASSA wndClass;
	memset(&wndClass, 0, sizeof(WNDCLASSA));
	wndClass.style = CS_HREDRAW|CS_VREDRAW;
	wndClass.lpfnWndProc = SE_SetupWindowProc;
	wndClass.hInstance = GameModule;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.lpszClassName = DialogClassName;
	wndClass.cbWndExtra = sizeof(LONG) * 3;

	return ( 0 != RegisterClass(&wndClass) );
}

LRESULT CALLBACK SE_SetupWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	RECT rect;
	POINT point;
	HDC hdc;
	PAINTSTRUCT paint;
	char imageName[64];
	BITMAP_RESOURCE *pBitmap = (BITMAP_RESOURCE *)GetWindowLong(hWnd, WINLONG_BMP);

	switch( uMsg ) {
		case WM_CREATE :
			GetClientRect(hWnd, &rect);
			point.x = rect.right / 2;
			point.y = rect.bottom / 2;
			MapWindowPoints(hWnd, GetParent(hWnd), &point, 1);
			SetWindowLong(hWnd, WINLONG_X, (LONG)point.x);
			SetWindowLong(hWnd, WINLONG_Y, (LONG)point.y);
			pBitmap = new BITMAP_RESOURCE;
			if( pBitmap != NULL ) {
				memset(pBitmap, 0, sizeof(BITMAP_RESOURCE));
				SetWindowLong(hWnd, WINLONG_BMP, (LONG)pBitmap);
				GetWindowText(hWnd, imageName, sizeof(imageName));
				SE_CenterBitmapResource(hWnd, pBitmap, imageName);
			}
			return 0;

		case WM_DESTROY :
			if( pBitmap != NULL ) {
				SE_ReleaseBitmapResource(pBitmap);
				delete(pBitmap);
			}
			break;

		case WM_PAINT :
			GetClientRect(hWnd, &rect);
			hdc = BeginPaint(hWnd, &paint);
			DrawEdge(hdc, &rect, BDR_RAISEDINNER|BDR_SUNKENOUTER, BF_ADJUST|BF_BOTTOM|BF_RIGHT|BF_TOP|BF_LEFT);
			SE_DrawBitmap(pBitmap, hdc, rect.left, rect.top);
			EndPaint(hWnd, &paint);
			return 0;

		case WM_QUERYNEWPALETTE :
			SE_ChangeBitmapPalette(pBitmap, hWnd);
			InvalidateRect(hWnd, NULL, FALSE);
			UpdateWindow(hWnd);
			return 1;

		case WM_PALETTECHANGED :
			SE_UpdateBitmapPalette(pBitmap, hWnd, (HWND)wParam);
			InvalidateRect(hWnd, NULL, FALSE);
			UpdateWindow(hWnd);
			break;

		case WM_CPL_LAUNCH :
			SE_CenterBitmapResource(hWnd, pBitmap, (LPCTSTR)wParam);
			return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void __cdecl SE_PassMessageToImage(HWND hWnd, UINT uMsg, WPARAM wParam) {
	if( uMsg == WM_QUERYNEWPALETTE || uMsg == WM_PALETTECHANGED )
		SendMessage(hWnd, uMsg, wParam, 0);
}

/*
 * Inject function
 */
void Inject_SetupWnd() {
	INJECT(0x00456FC0, SE_ReleaseBitmapResource);
	INJECT(0x00457000, SE_LoadBitmapResource);
	INJECT(0x004570D0, SE_DrawBitmap);
	INJECT(0x00457140, SE_UpdateBitmapPalette);
	INJECT(0x00457160, SE_ChangeBitmapPalette);
	INJECT(0x004571C0, SE_RegisterSetupWindowClass);
	INJECT(0x00457230, SE_SetupWindowProc);
	INJECT(0x00457470, SE_PassMessageToImage);
}
