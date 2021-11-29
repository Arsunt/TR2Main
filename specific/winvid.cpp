/*
 * Copyright (c) 2017-2021 Michael Chaban. All rights reserved.
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
#include "specific/winvid.h"
#include "specific/init_3d.h"
#include "specific/init_display.h"
#include "specific/sndpc.h"
#include "global/resource.h"
#include "global/vars.h"
#include "global/memmem.h"

#if (DIRECT3D_VERSION > 0x500)
DISPLAY_ADAPTER CurrentDisplayAdapter;
#endif // (DIRECT3D_VERSION > 0x500)

#ifdef FEATURE_WINDOW_STYLE_FIX
static void setWindowStyle(bool isFullScreen) {
	static const DWORD fullScreenFlags = WS_POPUP;
	static const DWORD windowedFlags = WS_OVERLAPPEDWINDOW; // WS_BORDER|WS_DLGFRAME|WS_SYSMENU|WS_SIZEBOX|WS_MINIMIZEBOX|WS_MAXIMIZEBOX
	DWORD style = GetWindowLong(HGameWindow, GWL_STYLE);
	style &= ~(isFullScreen ? windowedFlags : fullScreenFlags);
	style |= (isFullScreen ? fullScreenFlags : windowedFlags);
	SetWindowLong(HGameWindow, GWL_STYLE, style);
}
#endif // FEATURE_WINDOW_STYLE_FIX

#ifdef FEATURE_NOLEGACY_OPTIONS
bool AvoidInterlacedVideoModes = false;
#endif // FEATURE_NOLEGACY_OPTIONS

#ifdef FEATURE_INPUT_IMPROVED
#include "modding/raw_input.h"
#include "modding/joy_output.h"
#endif // FEATURE_INPUT_IMPROVED

static bool InsertDisplayModeInListSorted(DISPLAY_MODE_LIST *modeList, DISPLAY_MODE *srcMode) {
	DISPLAY_MODE_NODE *node = NULL;
	DISPLAY_MODE *dstMode = NULL;

	if( !modeList->head || CompareVideoModes(srcMode, &modeList->head->body) ) {
		dstMode = InsertDisplayModeInListHead(modeList);
		goto FILL;
	}
	for( node = modeList->head; node; node = node->next ) {
		if( CompareVideoModes(srcMode, &node->body) ) {
			dstMode = InsertDisplayMode(modeList, node);
			goto FILL;
#if (DIRECT3D_VERSION >= 0x900)
		} else if( !memcmp(srcMode, &node->body, sizeof(DISPLAY_MODE)) ) {
			return false;
#endif // (DIRECT3D_VERSION >= 0x900)
		}
	}
	dstMode = InsertDisplayModeInListTail(modeList);

FILL:
	if( dstMode ) {
		*dstMode = *srcMode;
		return true;
	}
	return false;
}

static bool DisplayModeListCopy(DISPLAY_MODE_LIST *dst, DISPLAY_MODE_LIST *src) {
	if( dst == NULL || src == NULL || dst == src )
		return false;

	DISPLAY_MODE_NODE *node;
	DISPLAY_MODE *dstMode;

	DisplayModeListDelete(dst);
	for( node = src->head; node; node = node->next ) {
		dstMode = InsertDisplayModeInListTail(dst);
		*dstMode = node->body;
	}
	return true;
}

bool FlaggedStringCopy(STRING_FLAGGED *dst, STRING_FLAGGED *src) {
	if( dst == NULL || src == NULL || dst == src || !src->isPresented )
		return false;

	size_t srcLen = lstrlen(src->lpString);

	dst->isPresented = false;
	dst->lpString = new char[srcLen + 1];

	if( dst->lpString == NULL )
		return false;

	if( srcLen > 0 ) {
		lstrcpy(dst->lpString, src->lpString);
	} else {
		*dst->lpString = 0;
	}
	dst->isPresented = true;
	return true;
}

#if (DIRECT3D_VERSION < 0x900)
bool __cdecl DDrawCreate(LPGUID lpGUID) {
	if FAILED(DirectDrawCreate(lpGUID, &DDrawInterface, 0))
		return false;

	if FAILED(DDrawInterface->QueryInterface(IID_IDirectDraw2, (LPVOID *)&DDraw))
		return false;

	DDraw->SetCooperativeLevel(HGameWindow, DDSCL_NORMAL);
	return true;
}

void __cdecl DDrawRelease() {
	if( DDraw ) {
		DDraw->Release();
		DDraw = NULL;
	}
	if( DDrawInterface ) {
		DDrawInterface->Release();
		DDrawInterface = NULL;
	}
}
#endif // (DIRECT3D_VERSION < 0x900)

void __cdecl GameWindowCalculateSizeFromClient(int *width, int *height) {
	DWORD style, styleEx;
	RECT rect = {0, 0, *width, *height};

	style = GetWindowLong(HGameWindow, GWL_STYLE);
	styleEx = GetWindowLong(HGameWindow, GWL_EXSTYLE);
	AdjustWindowRectEx(&rect, style, FALSE, styleEx);

	*width  = rect.right  - rect.left;
	*height = rect.bottom - rect.top;
}

void __cdecl GameWindowCalculateSizeFromClientByZero(int *width, int *height) {
	DWORD style, styleEx;
	RECT rect = {0, 0, 0, 0};

	style = GetWindowLong(HGameWindow, GWL_STYLE);
	styleEx = GetWindowLong(HGameWindow, GWL_EXSTYLE);
	AdjustWindowRectEx(&rect, style, FALSE, styleEx);

	*width  += rect.left - rect.right;
	*height += rect.top  - rect.bottom;;
}

void __cdecl WinVidSetMinWindowSize(int width, int height) {
	MinWindowClientWidth = width;
	MinWindowClientHeight = height;
	GameWindowCalculateSizeFromClient(&width, &height);
	MinWindowWidth = width;
	MinWindowHeight = height;
	IsMinWindowSizeSet = true;
}

void __cdecl WinVidClearMinWindowSize() {
	IsMinWindowSizeSet = false;
}

void __cdecl WinVidSetMaxWindowSize(int width, int height) {
	MaxWindowClientWidth = width;
	MaxWindowClientHeight = height;
	GameWindowCalculateSizeFromClient(&width, &height);
	MaxWindowWidth = width;
	MaxWindowHeight = height;
	IsMaxWindowSizeSet = true;
}

void __cdecl WinVidClearMaxWindowSize() {
	IsMaxWindowSizeSet = false;
}

int __cdecl CalculateWindowWidth(int width, int height) {
	switch( SavedAppSettings.AspectMode ) {
		case AM_4_3 :
			return height*4/3;
		case AM_16_9 :
			return height*16/9;
		default :
			break;
	}
	return width;
}

int __cdecl CalculateWindowHeight(int width, int height) {
	switch( SavedAppSettings.AspectMode ) {
		case AM_4_3 :
			return width*3/4;
		case AM_16_9 :
			return width*9/16;
		default :
			break;
	}
	return height;
}

bool __cdecl WinVidGetMinMaxInfo(LPMINMAXINFO info) {

	if( !IsGameWindowCreated )
		return false;

	if( IsGameFullScreen ) {
		info->ptMinTrackSize.x = FullScreenWidth;
		info->ptMinTrackSize.y = FullScreenHeight;

		info->ptMaxTrackSize.x = FullScreenWidth;
		info->ptMaxTrackSize.y = FullScreenHeight;
		info->ptMaxSize.x = FullScreenWidth;
		info->ptMaxSize.y = FullScreenHeight;
		return true;
	}

	if( IsMinWindowSizeSet ) {
		info->ptMinTrackSize.x = MinWindowWidth;
		info->ptMinTrackSize.y = MinWindowHeight;
	}

	if( IsMinMaxInfoSpecial ) {
		int newWindowWidth = GameWindowWidth;
		int newWindowHeight = GameWindowHeight;
		GameWindowCalculateSizeFromClient(&newWindowWidth, &newWindowHeight);

		info->ptMaxTrackSize.x = newWindowWidth;
		info->ptMaxTrackSize.y = newWindowHeight;
		info->ptMaxSize.x = newWindowWidth;
		info->ptMaxSize.y = newWindowHeight;
	}
	else if( IsMaxWindowSizeSet ) {
		info->ptMaxTrackSize.x = MaxWindowWidth;
		info->ptMaxTrackSize.y = MaxWindowHeight;
		info->ptMaxSize.x = MaxWindowWidth;
		info->ptMaxSize.y = MaxWindowHeight;
	}

	return ( IsMinWindowSizeSet || IsMaxWindowSizeSet );
}

HWND __cdecl WinVidFindGameWindow() {
	return FindWindow(GameClassName, GameWindowName);
}

bool __cdecl WinVidSpinMessageLoop(bool needWait) {
	static int messageLoopCounter = 0;
	MSG msg;

	if( IsMessageLoopClosed )
		return false;

	++messageLoopCounter;
	do {
		if( needWait )
			WaitMessage();
		else
			needWait = true;

		while( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) ) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if( msg.message == WM_QUIT ) {
				AppResultCode = msg.wParam;
				IsGameToExit = true;
				StopInventory = true;
				IsMessageLoopClosed = true;
				--messageLoopCounter;
				return false;
			}
		}
	} while( !IsGameWindowActive || IsGameWindowMinimized );

	--messageLoopCounter;
	return true;
}

void __cdecl WinVidShowGameWindow(int nCmdShow) {
	if( nCmdShow != SW_SHOW || !IsGameWindowShow ) {
		IsGameWindowUpdating = TRUE;
		ShowWindow(HGameWindow, nCmdShow);
		UpdateWindow(HGameWindow);
		IsGameWindowUpdating = FALSE;
		IsGameWindowShow = TRUE;
	}
}

void __cdecl WinVidHideGameWindow() {
	if( IsGameWindowShow ) {
		IsGameWindowUpdating = TRUE;
		ShowWindow(HGameWindow, SW_HIDE);
		UpdateWindow(HGameWindow);
		IsGameWindowUpdating = FALSE;
		IsGameWindowShow = FALSE;
	}
}

void __cdecl WinVidSetGameWindowSize(int width, int height) {
	GameWindowCalculateSizeFromClient(&width, &height);
	SetWindowPos(HGameWindow, NULL, 0, 0, width, height, SWP_NOCOPYBITS|SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOMOVE);
}

#if (DIRECT3D_VERSION < 0x900)
bool __cdecl ShowDDrawGameWindow(bool active) {
	DWORD flags;
	HRESULT rc;
	RECT rect;

	if( !HGameWindow || !DDraw )
		return false;

	if( IsDDrawGameWindowShow )
		return true;

	GetWindowRect(HGameWindow, &rect);
	GameWindow_X = rect.left;
	GameWindow_Y = rect.top;

	if( active )
		WinVidShowGameWindow(SW_SHOW);

	flags = DDSCL_ALLOWMODEX | DDSCL_EXCLUSIVE | DDSCL_ALLOWREBOOT | DDSCL_FULLSCREEN;
	if( !active )
		flags |= DDSCL_NOWINDOWCHANGES;

	IsGameWindowUpdating = TRUE;
	rc = DDraw->SetCooperativeLevel(HGameWindow, flags);
	IsGameWindowUpdating = FALSE;

	if SUCCEEDED(rc) {
		IsDDrawGameWindowShow = TRUE;
		return true;
	}
	return false;
}

bool __cdecl HideDDrawGameWindow() {
	bool result = false;

	if( HGameWindow == NULL || DDraw == NULL )
		return false;

	if( !IsDDrawGameWindowShow )
		return true;

	WinVidHideGameWindow();

	IsGameWindowUpdating = TRUE;
	if SUCCEEDED(DDraw->SetCooperativeLevel(HGameWindow, DDSCL_NORMAL)) {
		IsDDrawGameWindowShow = FALSE;
		SetWindowPos(HGameWindow, 0, GameWindow_X, GameWindow_Y, 0, 0, SWP_NOCOPYBITS|SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOSIZE);
		result = true;
	}
	IsGameWindowUpdating = FALSE;

	return result;
}

HRESULT __cdecl DDrawSurfaceCreate(LPDDSDESC dsp, LPDDS *surface) {
	LPDIRECTDRAWSURFACE subSurface;
	HRESULT rc = DDraw->CreateSurface(dsp, &subSurface, NULL);

	if SUCCEEDED(rc) {
		rc = subSurface->QueryInterface(IID_IDirectDrawSurface3, (LPVOID *)surface);
		subSurface->Release();
	}

	return rc;
}

HRESULT __cdecl DDrawSurfaceRestoreLost(LPDDS surface1, LPDDS surface2, bool blank) {
	if( surface1 == NULL ) // NOTE: additional check just in case
		return 0;

	HRESULT rc = surface1->IsLost();
	if( rc != DDERR_SURFACELOST )
		return rc;

	rc = (surface2 ? surface2 : surface1)->Restore();
	if( blank && SUCCEEDED(rc) )
		WinVidClearBuffer(surface1, 0, 0);

	return rc;
}

bool __cdecl WinVidClearBuffer(LPDDS surface, LPRECT rect, DWORD fillColor) {
	DDBLTFX bltFx;

	if( surface == NULL ) // NOTE: additional check just in case
		return 0;

	memset(&bltFx, 0, sizeof(DDBLTFX));
	bltFx.dwSize = sizeof(DDBLTFX);
	bltFx.dwFillColor = fillColor;

	return SUCCEEDED(surface->Blt(rect, NULL, NULL, DDBLT_WAIT|DDBLT_COLORFILL, &bltFx));
}

HRESULT __cdecl WinVidBufferLock(LPDDS surface, LPDDSDESC desc, DWORD flags) {
	memset(desc, 0, sizeof(DDSDESC));
	desc->dwSize = sizeof(DDSDESC);

	HRESULT result = surface->Lock(NULL, desc, flags, NULL);
	if SUCCEEDED(result) {
		++LockedBufferCount;
	}
	return result;
}

HRESULT __cdecl WinVidBufferUnlock(LPDDS surface, LPDDSDESC desc) {
	HRESULT result = surface->Unlock(desc->lpSurface);
	if SUCCEEDED(result) {
		--LockedBufferCount;
	}
	return result;
}

bool __cdecl WinVidCopyBitmapToBuffer(LPDDS surface, BYTE *bitmap) {
	DWORD i;
	BYTE *src, *dst;
	DDSDESC desc;
	if( surface == NULL || bitmap == NULL ) // NOTE: additional check just in case
		return false;

	if FAILED(WinVidBufferLock(surface, &desc, DDLOCK_WRITEONLY|DDLOCK_WAIT))
		return false;

	src = bitmap;
	dst = (BYTE *)desc.lpSurface;
	for( i=0; i<desc.dwHeight; ++i ) {
		memcpy(dst, src, desc.dwWidth);
		src += desc.dwWidth;
		dst += desc.lPitch;
	}

	WinVidBufferUnlock(surface, &desc);
	return true;
}

DWORD __cdecl GetRenderBitDepth(DWORD dwRGBBitCount) {
	switch( dwRGBBitCount ) {
		case  1 : return DDBD_1;
		case  2 : return DDBD_2;
		case  4 : return DDBD_4;
		case  8 : return DDBD_8;
		case 16 : return DDBD_16;
		case 24 : return DDBD_24;
		case 32 : return DDBD_32;
	}
	return 0;
}

void __thiscall WinVidGetColorBitMasks(COLOR_BIT_MASKS *bm, LPDDPIXELFORMAT pixelFormat) {
	bm->dwRBitMask			= pixelFormat->dwRBitMask;
	bm->dwGBitMask			= pixelFormat->dwGBitMask;
	bm->dwBBitMask			= pixelFormat->dwBBitMask;
	bm->dwRGBAlphaBitMask	= pixelFormat->dwRGBAlphaBitMask;

	BitMaskGetNumberOfBits(bm->dwRBitMask,			&bm->dwRBitDepth,			&bm->dwRBitOffset);
	BitMaskGetNumberOfBits(bm->dwGBitMask,			&bm->dwGBitDepth,			&bm->dwGBitOffset);
	BitMaskGetNumberOfBits(bm->dwBBitMask,			&bm->dwBBitDepth,			&bm->dwBBitOffset);
	BitMaskGetNumberOfBits(bm->dwRGBAlphaBitMask,	&bm->dwRGBAlphaBitDepth,	&bm->dwRGBAlphaBitOffset);
}

void __cdecl BitMaskGetNumberOfBits(DWORD bitMask, DWORD *bitDepth, DWORD *bitOffset) {
	DWORD i;

	if( !bitMask ) {
		*bitOffset = 0;
		*bitDepth = 0;
		return;
	}

	for( i = 0; (bitMask & 1) == 0; ++i ) {
		bitMask >>= 1;
	}
	*bitOffset = i;

	for( i = 0; bitMask != 0; ++i ) {
		bitMask >>= 1;
	}
	*bitDepth = i;
}

DWORD __cdecl CalculateCompatibleColor(COLOR_BIT_MASKS *mask, int red, int green, int blue, int alpha) {
	return	(red	>> (8 - mask->dwRBitDepth)			<< mask->dwRBitOffset) |
			(green	>> (8 - mask->dwGBitDepth)			<< mask->dwGBitOffset) |
			(blue	>> (8 - mask->dwBBitDepth)			<< mask->dwBBitOffset) |
			(alpha	>> (8 - mask->dwRGBAlphaBitDepth)	<< mask->dwRGBAlphaBitOffset);
}
#endif // (DIRECT3D_VERSION < 0x900)

bool __cdecl WinVidGetDisplayMode(DISPLAY_MODE *dispMode) {
#if (DIRECT3D_VERSION >= 0x900)
	bool d3dClean = false;
	if( D3D == NULL ) {
		if( !D3DCreate() ) return false;
		d3dClean = true;
	}
	D3DDISPLAYMODE mode;
	HRESULT res = D3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &mode);
	if( d3dClean ) D3DRelease();
	if( SUCCEEDED(res) && mode.Format == D3DFMT_X8R8G8B8 ) {
		dispMode->width = mode.Width;
		dispMode->height = mode.Height;
		dispMode->bpp = 32;
		dispMode->vga = VGA_NoVga;
		return true;
	}
	return false;
#else // (DIRECT3D_VERSION >= 0x900)
	DDSDESC dsp;

	memset(&dsp, 0, sizeof(dsp));
	dsp.dwSize = sizeof(dsp);

	if( SUCCEEDED(DDraw->GetDisplayMode(&dsp)) &&
		((dsp.dwFlags & DDSD_WIDTH) != 0) &&
		((dsp.dwFlags & DDSD_HEIGHT) != 0) &&
		((dsp.dwFlags & DDSD_PIXELFORMAT) != 0) &&
		((dsp.ddpfPixelFormat.dwFlags & DDPF_RGB) != 0) )
	{
		dispMode->width = dsp.dwWidth;
		dispMode->height = dsp.dwHeight;
		dispMode->bpp = dsp.ddpfPixelFormat.dwRGBBitCount;

		if( (dsp.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8) != 0 )
			dispMode->vga = VGA_256Color;
		else
			dispMode->vga = VGA_NoVga;

		return true;
	}

	return false;
#endif // (DIRECT3D_VERSION >= 0x900)
}

bool __cdecl WinVidGoFullScreen(DISPLAY_MODE *dispMode) {
	FullScreenWidth = dispMode->width;
	FullScreenHeight = dispMode->height;
	FullScreenBPP = dispMode->bpp;
	FullScreenVGA = dispMode->vga;

#ifdef FEATURE_WINDOW_STYLE_FIX
	setWindowStyle(true);
#endif // FEATURE_WINDOW_STYLE_FIX

#if (DIRECT3D_VERSION < 0x900)
	HRESULT rc;
	if( !ShowDDrawGameWindow(true) )
		goto FAIL;

	IsGameWindowUpdating = true;
	rc = DDraw->SetDisplayMode(dispMode->width, dispMode->height, dispMode->bpp, 0, (dispMode->vga == VGA_Standard) ? DDSDM_STANDARDVGAMODE : 0);
	IsGameWindowUpdating = false;

	if FAILED(rc)
		goto FAIL;
#endif // (DIRECT3D_VERSION < 0x900)

	IsGameFullScreen = true;
	return true;

#if (DIRECT3D_VERSION < 0x900)
FAIL :
#ifdef FEATURE_WINDOW_STYLE_FIX
	setWindowStyle(false);
#endif // FEATURE_WINDOW_STYLE_FIX
	return false;
#endif // (DIRECT3D_VERSION < 0x900)
}

bool __cdecl WinVidGoWindowed(int width, int height, DISPLAY_MODE *dispMode) {
	int maxWidth, maxHeight;
	RECT rect;

#if (DIRECT3D_VERSION >= 0x900)
	if( !WinVidGetDisplayMode(dispMode) )
		return false;
#else // (DIRECT3D_VERSION >= 0x900)
	if( !HideDDrawGameWindow() || !WinVidGetDisplayMode(dispMode) )
		return false;
#endif // (DIRECT3D_VERSION >= 0x900)

#ifdef FEATURE_WINDOW_STYLE_FIX
	setWindowStyle(false);
#endif // FEATURE_WINDOW_STYLE_FIX

	maxWidth = dispMode->width;
	maxHeight = CalculateWindowHeight(dispMode->width, dispMode->height);

	if( maxHeight > dispMode->height ) {
		maxHeight = dispMode->height;
		maxWidth = CalculateWindowWidth(dispMode->width, dispMode->height);
	}
	WinVidSetMaxWindowSize(maxWidth, maxHeight);

	if( width > maxWidth || height > maxHeight ) {
		width = maxWidth;
		height = maxHeight;
	}

	IsGameFullScreen = false;
	IsGameWindowUpdating = true;
	WinVidSetGameWindowSize(width, height);
	IsGameWindowUpdating = false;

	GetClientRect(HGameWindow, &rect);
	MapWindowPoints(HGameWindow, NULL, (LPPOINT)&rect, 2);

	if( (rect.left > 0 || rect.right < dispMode->width) &&
		(rect.top > 0 || rect.bottom < dispMode->height) )
	{
		WinVidShowGameWindow(SW_SHOW);
	} else {
		WinVidShowGameWindow(SW_MAXIMIZE);
	}

	dispMode->width = width;
	dispMode->height = height;
	return true;
}

void __cdecl WinVidSetDisplayAdapter(DISPLAY_ADAPTER *dispAdapter) {
	bool rc;
	DISPLAY_MODE dispMode;

	dispAdapter->screenWidth = 0;
#if (DIRECT3D_VERSION < 0x900)
	dispAdapter->swWindowedSupported = false;
	dispAdapter->hwWindowedSupported = false;

	// Primary adapter GUID is NULL. Secondary adapter is not applicable
	if( dispAdapter->lpAdapterGuid != NULL )
		return;

	if( !DDrawCreate(NULL) ) return;
#endif // (DIRECT3D_VERSION < 0x900)
	rc = WinVidGetDisplayMode(&dispMode);
#if (DIRECT3D_VERSION < 0x900)
	DDrawRelease();
#endif // (DIRECT3D_VERSION < 0x900)
	if( !rc ) return;

	dispMode.width &= ~0x1F;
	if( dispMode.width*3/4 > dispMode.height )
		dispMode.width = (dispMode.height*4/3) & ~0x1F;

	dispAdapter->screenWidth = dispMode.width;
#if (DIRECT3D_VERSION < 0x900)
	dispAdapter->swWindowedSupported = ( dispMode.vga == VGA_256Color );
	dispAdapter->hwWindowedSupported = ( dispAdapter->hwRenderSupported &&
		((dispAdapter->D3DHWDeviceDesc.dwFlags & D3DDD_DEVICERENDERBITDEPTH) != 0) &&
		((GetRenderBitDepth(dispMode.bpp) & dispAdapter->D3DHWDeviceDesc.dwDeviceRenderBitDepth) != 0) );
#endif // (DIRECT3D_VERSION < 0x900)
}

bool __thiscall CompareVideoModes(DISPLAY_MODE *mode1, DISPLAY_MODE *mode2) {
#ifdef FEATURE_NOLEGACY_OPTIONS
	if( mode1->bpp < mode2->bpp ) return true;
	if( mode1->bpp > mode2->bpp ) return false;
	if( mode1->width < mode2->width ) return true;
	if( mode1->width > mode2->width ) return false;
	if( mode1->height < mode2->height ) return true;
	if( mode1->height > mode2->height ) return false;
#else // !FEATURE_NOLEGACY_OPTIONS
	DWORD square1 = mode1->width * mode1->height;
	DWORD square2 = mode2->width * mode2->height;
	if( square1 < square2 ) return true;
	if( square1 > square2 ) return false;
	if( mode1->bpp < mode2->bpp ) return true;
	if( mode1->bpp > mode2->bpp ) return false;
#endif // FEATURE_NOLEGACY_OPTIONS
	if( mode1->vga < mode2->vga ) return true;
	if( mode1->vga > mode2->vga ) return false;
	// equal state
	return false;
}

#ifdef FEATURE_NOLEGACY_OPTIONS
static void DeleteDisplayMode(DISPLAY_MODE_LIST *modeList, DISPLAY_MODE_NODE *node) {
	if( !modeList || !node ) return;
	DISPLAY_MODE_NODE *previous = node->previous;
	DISPLAY_MODE_NODE *next = node->next;
	if( previous ) previous->next = next;
	if( next ) next->previous = previous;
	if( modeList->head == node ) modeList->head = next;
	if( modeList->tail == node ) modeList->tail = previous;
	if( modeList->dwCount ) --modeList->dwCount;
	delete(node);
}

static DWORD GetProgressiveDisplayModes(DWORD bpp, DEVMODE *modes, DWORD modeNum) {
	DWORD idx = 0;
	DWORD num = 0;
	if( modes == NULL ) {
		DEVMODE mode;
		memset(&mode, 0, sizeof(mode));
		mode.dmSize = sizeof(mode);
		while( EnumDisplaySettings(NULL, idx++, &mode) ) {
			if( mode.dmBitsPerPel == bpp && !CHK_ANY(mode.dmDisplayFlags, DM_INTERLACED) ) {
				++num;
			}
		}
	} else {
		memset(modes, 0, sizeof(DEVMODE) * modeNum);
		while( num < modeNum ) {
			modes[num].dmSize = sizeof(DEVMODE);
			if( !EnumDisplaySettings(NULL, idx++, &modes[num]) ) {
				break;
			}
			if( modes[num].dmBitsPerPel == bpp && !CHK_ANY(modes[num].dmDisplayFlags, DM_INTERLACED) ) {
				++num;
			}
		}
	}
	return num;
}

static bool IsModeInList(DISPLAY_MODE *mode, DEVMODE *modes, DWORD modeNum) {
	if( !mode || !modes || !modeNum ) return false;
	for( DWORD i = 0; i < modeNum; ++i ) {
		if( modes[i].dmPelsWidth  == (DWORD)mode->width  &&
			modes[i].dmPelsHeight == (DWORD)mode->height &&
			modes[i].dmBitsPerPel == (DWORD)mode->bpp )
		{
			return true;
		}
	}
	return false;
}

static void FilterDisplayModes(DISPLAY_MODE_LIST *modeList) {
	DWORD wlistSize = 0;
	DEVMODE *whitelist = NULL;
	DISPLAY_MODE_NODE *mode, *next;
	int bppMax = 8;
	for( mode = modeList->head; mode; mode = mode->next ) {
		CLAMPL(bppMax, mode->body.bpp);
	}
	if( AvoidInterlacedVideoModes ) {
		wlistSize = GetProgressiveDisplayModes(bppMax, NULL, 0);
		if( wlistSize ) {
			whitelist = (DEVMODE *)malloc(sizeof(DEVMODE) * wlistSize);
			if( whitelist ) {
				GetProgressiveDisplayModes(bppMax, whitelist, wlistSize);
			}
		}
	}
	for( mode = modeList->head; mode; mode = next ) {
		next = mode->next;
		if( mode->body.bpp < bppMax || (whitelist && !IsModeInList(&mode->body, whitelist, wlistSize)) ) {
			DeleteDisplayMode(modeList, mode);
		}
	}
	if( whitelist ) {
		free(whitelist);
	}
}
#endif // FEATURE_NOLEGACY_OPTIONS

bool __cdecl WinVidGetDisplayModes() {
	DISPLAY_ADAPTER_NODE *adapter;
#if (DIRECT3D_VERSION >= 0x900)
	bool d3dClean = false;
	if( D3D == NULL ) {
		if( !D3DCreate() ) return false;
		d3dClean = true;
	}
	for( adapter = DisplayAdapterList.head; adapter; adapter = adapter->next ) {
		UINT num = D3D->GetAdapterModeCount(adapter->body.index, D3DFMT_X8R8G8B8);
		for( UINT i = 0; i < num; ++i ) {
			D3DDISPLAYMODE mode;
			if FAILED(D3D->EnumAdapterModes(adapter->body.index, D3DFMT_X8R8G8B8, i, &mode)) continue;
			DISPLAY_MODE videoMode = {(int)mode.Width, (int)mode.Height, 32, VGA_NoVga};
			InsertDisplayModeInListSorted(&adapter->body.hwDispModeList, &videoMode);
			InsertDisplayModeInListSorted(&adapter->body.swDispModeList, &videoMode);
		}
		FilterDisplayModes(&adapter->body.hwDispModeList);
	}
	if( d3dClean ) D3DRelease();
#else // (DIRECT3D_VERSION >= 0x900)
	for( adapter = DisplayAdapterList.head; adapter; adapter = adapter->next ) {
		DDrawCreate(adapter->body.lpAdapterGuid);
		ShowDDrawGameWindow(false);
#ifdef FEATURE_NOLEGACY_OPTIONS
		DDraw->EnumDisplayModes(0, NULL, (LPVOID)&adapter->body, EnumDisplayModesCallback);
		FilterDisplayModes(&adapter->body.hwDispModeList);
#else // FEATURE_NOLEGACY_OPTIONS
		DDraw->EnumDisplayModes(DDEDM_STANDARDVGAMODES, NULL, (LPVOID)&adapter->body, EnumDisplayModesCallback);
#endif // FEATURE_NOLEGACY_OPTIONS
		HideDDrawGameWindow();
		DDrawRelease();
	}
#endif // (DIRECT3D_VERSION >= 0x900)
	return true;
}

#if (DIRECT3D_VERSION < 0x900)
HRESULT WINAPI EnumDisplayModesCallback(LPDDSDESC lpDDSurfaceDesc, LPVOID lpContext) {
	DISPLAY_ADAPTER *adapter = (DISPLAY_ADAPTER *)lpContext;
	DISPLAY_MODE videoMode;
	VGA_MODE vgaMode = VGA_NoVga;
	DWORD renderBitDepth = 0;
	bool swRendererSupported = false;

	if( (lpDDSurfaceDesc->dwFlags & (DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT)) != (DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT) ||
		(lpDDSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_RGB) == 0 )
	{
		return DDENUMRET_OK;
	}

	if( (lpDDSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8) != 0 &&
		lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount == 8 )
	{
#ifdef FEATURE_NOLEGACY_OPTIONS
		// Check software renderer requirements for 8 bit display modes
		if( lpDDSurfaceDesc->dwHeight % 4 != 0 ||
			CHK_ANY(lpDDSurfaceDesc->ddsCaps.dwCaps, DDSCAPS_MODEX|DDSCAPS_STANDARDVGAMODE) )
		{
			return DDENUMRET_OK;
		}

		vgaMode = VGA_256Color;
#else // FEATURE_NOLEGACY_OPTIONS
		if( CHK_ANY(lpDDSurfaceDesc->ddsCaps.dwCaps, DDSCAPS_MODEX) ) {
			vgaMode = VGA_ModeX;
		} else if( CHK_ANY(lpDDSurfaceDesc->ddsCaps.dwCaps, DDSCAPS_STANDARDVGAMODE) ) {
			vgaMode = VGA_Standard;
		} else {
			vgaMode = VGA_256Color;
		}
#endif // FEATURE_NOLEGACY_OPTIONS

		if( lpDDSurfaceDesc->dwWidth == 320 &&
			lpDDSurfaceDesc->dwHeight == 200 &&
			(!adapter->isVgaMode1Presented || vgaMode < adapter->vgaMode1.vga) )
		{
			adapter->vgaMode1.width = 320;
			adapter->vgaMode1.height = 200;
			adapter->vgaMode1.bpp = 8;
			adapter->vgaMode1.vga = vgaMode;
			adapter->isVgaMode1Presented = true;
		}

		if( lpDDSurfaceDesc->dwWidth == 640 &&
			lpDDSurfaceDesc->dwHeight == 480 &&
			(!adapter->isVgaMode2Presented || vgaMode < adapter->vgaMode2.vga) )
		{
			adapter->vgaMode2.width = 640;
			adapter->vgaMode2.height = 480;
			adapter->vgaMode2.bpp = 8;
			adapter->vgaMode2.vga = vgaMode;
			adapter->isVgaMode2Presented = true;
		}
		swRendererSupported = true;
	}

	videoMode.width = lpDDSurfaceDesc->dwWidth;
	videoMode.height = lpDDSurfaceDesc->dwHeight;
	videoMode.bpp = lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount;
	videoMode.vga = vgaMode;

	renderBitDepth = GetRenderBitDepth(lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount);

	if( adapter->hwRenderSupported && 0 != ( renderBitDepth & adapter->D3DHWDeviceDesc.dwDeviceRenderBitDepth) )
		InsertDisplayModeInListSorted(&adapter->hwDispModeList, &videoMode);

	if( swRendererSupported )
		InsertDisplayModeInListSorted(&adapter->swDispModeList, &videoMode);

	return DDENUMRET_OK;
}
#endif // (DIRECT3D_VERSION < 0x900)

bool __cdecl WinVidInit() {
	AppResultCode = 0;
#if (DIRECT3D_VERSION <= 0x700)
	// NOTE: We can hack DirectDraw to support surface resolutions greater than 2048x2048.
	// Not presented in the original game. Many thanks to Gemini-Loboto3 for this idea!
#if (DIRECT3D_VERSION == 0x700)
	HMODULE hd3d = LoadLibrary("d3dim700.dll");
#else // (DIRECT3D_VERSION == 0x700)
	HMODULE hd3d = LoadLibrary("d3dim.dll");
#endif // (DIRECT3D_VERSION == 0x700)
	if( hd3d ) {
		PIMAGE_DOS_HEADER pDosHeader;
		PIMAGE_NT_HEADERS pNtHeader;
		DWORD dwCodeBase;
		DWORD dwCodeSize;
		DWORD dwPatchBase;
		DWORD dwOldProtect;

		pDosHeader = (PIMAGE_DOS_HEADER)hd3d;
		pNtHeader = (PIMAGE_NT_HEADERS)((char*)pDosHeader + pDosHeader->e_lfanew);
		dwCodeBase = (DWORD)hd3d + pNtHeader->OptionalHeader.BaseOfCode;
		dwCodeSize = pNtHeader->OptionalHeader.SizeOfCode;

		static BYTE wantedBytes[] = {0xB8, 0x00, 0x08, 0x00, 0x00, 0x39};
		dwPatchBase = (DWORD)memmem((void*)dwCodeBase, dwCodeSize, wantedBytes, sizeof(wantedBytes));
		if( dwPatchBase ) {
			dwPatchBase++;
			VirtualProtect((LPVOID)dwPatchBase, 4, PAGE_EXECUTE_READWRITE, &dwOldProtect);
			*(DWORD*)dwPatchBase = ~0;
			VirtualProtect((LPVOID)dwPatchBase, 4, dwOldProtect, &dwOldProtect);
		}
	}
#endif // (DIRECT3D_VERSION <= 0x700)
	return ( WinVidRegisterGameWindowClass() &&
			 WinVidCreateGameWindow() &&
			 WinVidGetDisplayAdapters() &&
			 DisplayAdapterList.dwCount &&
			 WinVidGetDisplayModes() );
}

bool __cdecl WinVidGetDisplayAdapters() {
	DISPLAY_ADAPTER_NODE *node, *nextNode;

	for( node = DisplayAdapterList.head; node; node = nextNode ) {
		nextNode = node->next;
		DisplayModeListDelete(&node->body.swDispModeList);
		DisplayModeListDelete(&node->body.hwDispModeList);
		FlaggedStringDelete(&node->body.driverName);
		FlaggedStringDelete(&node->body.driverDescription);
		delete(node);
	}

	DisplayAdapterList.head = NULL;
	DisplayAdapterList.tail = NULL;
	DisplayAdapterList.dwCount = 0;

	PrimaryDisplayAdapter = NULL;

	if( !EnumerateDisplayAdapters(&DisplayAdapterList) )
		return false;

#if (DIRECT3D_VERSION >= 0x900)
	PrimaryDisplayAdapter = DisplayAdapterList.head;
	return ( PrimaryDisplayAdapter != NULL );
#else // (DIRECT3D_VERSION >= 0x900)
	for( node = DisplayAdapterList.head; node; node = node->next ) {
		if( node->body.lpAdapterGuid == NULL ) { // Primary adapter GUID is NULL
			PrimaryDisplayAdapter = node;
			return true;
		}
	}
	return false;
#endif // (DIRECT3D_VERSION >= 0x900)
}

void __thiscall FlaggedStringDelete(STRING_FLAGGED *item) {
	if( item->isPresented && item->lpString ) {
		delete[] item->lpString;
		item->lpString = NULL;
		item->isPresented = false;
	}
}

bool __cdecl EnumerateDisplayAdapters(DISPLAY_ADAPTER_LIST *displayAdapterList) {
#if (DIRECT3D_VERSION >= 0x900)
	bool d3dClean = false;
	if( D3D == NULL ) {
		if( !D3DCreate() ) return false;
		d3dClean = true;
	}
	UINT num = D3D->GetAdapterCount();
	for( UINT i = 0; i < num; ++i ) {
		D3DADAPTER_IDENTIFIER9 id;
		D3DCAPS9 caps;
		if( FAILED(D3D->GetAdapterIdentifier(i, 0, &id)) || FAILED(D3D->GetDeviceCaps(i, D3DDEVTYPE_HAL, &caps)) ) {
			continue;
		}
		DISPLAY_ADAPTER_NODE *listNode = new DISPLAY_ADAPTER_NODE;
		if( listNode == NULL ) break;

		listNode->next = NULL;
		listNode->previous = displayAdapterList->tail;

		FlaggedStringsCreate(&listNode->body);
		DisplayModeListInit(&listNode->body.hwDispModeList);
		DisplayModeListInit(&listNode->body.swDispModeList);

		if( !displayAdapterList->head )
			displayAdapterList->head = listNode;

		if( displayAdapterList->tail )
			displayAdapterList->tail->next = listNode;

		displayAdapterList->tail = listNode;
		displayAdapterList->dwCount++;

		listNode->body.adapterGuid = id.DeviceIdentifier;
		listNode->body.lpAdapterGuid = &listNode->body.adapterGuid;

		lstrcpy(listNode->body.driverDescription.lpString, id.Description);
		lstrcpy(listNode->body.driverName.lpString, id.DeviceName);

		listNode->body.index = i;
		listNode->body.caps = caps;
	}
	if( d3dClean ) D3DRelease();
	return true;
#else // (DIRECT3D_VERSION >= 0x900)
	return SUCCEEDED(DirectDrawEnumerate(EnumDisplayAdaptersCallback, (LPVOID)displayAdapterList));
#endif // (DIRECT3D_VERSION >= 0x900)
}

#if (DIRECT3D_VERSION < 0x900)
BOOL WINAPI EnumDisplayAdaptersCallback(GUID FAR *lpGUID, LPTSTR lpDriverDescription, LPTSTR lpDriverName, LPVOID lpContext) {
	DDCAPS driverCaps;
	DDCAPS helCaps;
	DISPLAY_ADAPTER_NODE *listNode = new DISPLAY_ADAPTER_NODE;
	DISPLAY_ADAPTER_LIST *adapterList = (DISPLAY_ADAPTER_LIST *)lpContext;

	if( listNode == NULL || !DDrawCreate(lpGUID) )
		return TRUE;

	memset(&driverCaps, 0, sizeof(driverCaps));
	memset(&helCaps, 0, sizeof(helCaps));
	driverCaps.dwSize = sizeof(DDCAPS);
	helCaps.dwSize = sizeof(DDCAPS);

	if FAILED(DDraw->GetCaps(&driverCaps, &helCaps))
		goto CLEANUP;

	listNode->next = NULL;
	listNode->previous = adapterList->tail;

	FlaggedStringsCreate(&listNode->body);
	DisplayModeListInit(&listNode->body.hwDispModeList);
	DisplayModeListInit(&listNode->body.swDispModeList);

	if( !adapterList->head )
		adapterList->head = listNode;

	if( adapterList->tail )
		adapterList->tail->next = listNode;

	adapterList->tail = listNode;
	adapterList->dwCount++;

	if( lpGUID ) {
		// Any secondary adapter
		listNode->body.adapterGuid = *lpGUID;
		listNode->body.lpAdapterGuid = &listNode->body.adapterGuid;
	} else {
		// Primary adapter (GUID is NULL)
		memset(&listNode->body.adapterGuid, 0, sizeof(GUID));
		listNode->body.lpAdapterGuid = NULL;
	}

	lstrcpy(listNode->body.driverDescription.lpString, lpDriverDescription);
	lstrcpy(listNode->body.driverName.lpString, lpDriverName);

	listNode->body.driverCaps = driverCaps;
	listNode->body.helCaps = helCaps;

	listNode->body.screenWidth = 0;
	listNode->body.hwRenderSupported = false;
	listNode->body.swWindowedSupported = false;
	listNode->body.hwWindowedSupported = false;
	listNode->body.isVgaMode1Presented = false;
	listNode->body.isVgaMode2Presented = false;

	Enumerate3DDevices(&listNode->body);

CLEANUP :
	DDrawRelease();
	return TRUE;
}
#endif // (DIRECT3D_VERSION < 0x900)

void __thiscall FlaggedStringsCreate(DISPLAY_ADAPTER *adapter) {
	LPTSTR lpDriverDescription = new char[256];
	LPTSTR lpDriverName = new char[256];

	if( lpDriverDescription ) {
		*lpDriverDescription = 0;
		adapter->driverDescription.lpString = lpDriverDescription;
		adapter->driverDescription.isPresented = true;
	}

	if( lpDriverName ) {
		*lpDriverName = 0;
		adapter->driverName.lpString = lpDriverName;
		adapter->driverName.isPresented = true;
	}
}

bool __cdecl WinVidRegisterGameWindowClass() {
	WNDCLASSEXA wndClass;

	memset(&wndClass, 0, sizeof(wndClass));
	wndClass.cbSize = sizeof(wndClass);
	wndClass.style = 0;
	wndClass.lpfnWndProc = WinVidGameWindowProc;
	wndClass.hInstance = GameModule;
	wndClass.hIcon = LoadIcon(GameModule, MAKEINTRESOURCE(IDI_MAINICON));
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.lpszClassName = GameClassName;

	return ( RegisterClassEx(&wndClass) != 0 );
}

LRESULT CALLBACK WinVidGameWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
	HDC hdc;
#if (DIRECT3D_VERSION < 0x900)
	LPDDS surface;
#endif // (DIRECT3D_VERSION < 0x900)
	HBRUSH hBrush;
	PAINTSTRUCT paint;
#ifdef FEATURE_AUDIO_IMPROVED
	static DWORD cdVolume = 0;
#endif // FEATURE_AUDIO_IMPROVED

	if( IsFmvPlaying ) {
		switch( Msg ) {
			case WM_DESTROY :
				IsGameWindowCreated = false;
				HGameWindow = NULL;
				PostQuitMessage(0);
				break;

			case WM_MOVE :
				GameWindowPositionX = (int)(short)LOWORD(lParam);
				GameWindowPositionY = (int)(short)HIWORD(lParam);
				break;

			case WM_ACTIVATEAPP :
				IsGameWindowActive = ( wParam != 0 );
				break;

			case WM_SYSCOMMAND :
				if( wParam == SC_KEYMENU ) return 0;
				break;
		}
		return DefWindowProc(hWnd, Msg, wParam, lParam);
	}

	switch( Msg ) {
		case WM_CREATE :
			IsGameWindowCreated = true;
			break;

		case WM_DESTROY :
			IsGameWindowCreated = false;
			HGameWindow = NULL;
			PostQuitMessage(0);
			break;

		case WM_MOVE :
			GameWindowPositionX = (int)(short)LOWORD(lParam);
			GameWindowPositionY = (int)(short)HIWORD(lParam);
			break;

		case WM_SIZE:
			switch( wParam ) {
				case SIZE_RESTORED :
					IsGameWindowMinimized = false;
					IsGameWindowMaximized = false;
					break;
				case SIZE_MAXIMIZED :
					IsGameWindowMinimized = false;
					IsGameWindowMaximized = true;
					break;
				case SIZE_MINIMIZED :
					IsGameWindowMinimized = true;
					IsGameWindowMaximized = false;
					// fall through
				default :
					return DefWindowProc(hWnd, Msg, wParam, lParam);
			}

			if( IsGameFullScreen ||
				((int)(short)LOWORD(lParam) == GameWindowWidth && (int)(short)HIWORD(lParam) == GameWindowHeight) )
				break;

			GameWindowWidth  = (int)(short)LOWORD(lParam);
			GameWindowHeight = (int)(short)HIWORD(lParam);
			if( IsGameWindowUpdating )
				break;

			UpdateGameResolution();
			break;

		case WM_PAINT :
			hdc = BeginPaint(hWnd, &paint);
#if (DIRECT3D_VERSION >= 0x900)
			if( IsGameFullScreen || D3DDev == NULL || FAILED(D3DDev->TestCooperativeLevel()) ) {
				hBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
				FillRect(hdc, &paint.rcPaint, hBrush);
			} else {
				D3DDev->Clear(0, NULL, D3DCLEAR_TARGET, 0, 1.0, 0);
				D3DDev->Present(NULL, NULL, NULL, NULL);
			}
#else // (DIRECT3D_VERSION >= 0x900)
			surface = ( SavedAppSettings.RenderMode == RM_Software ) ? RenderBufferSurface : BackBufferSurface;
			if( IsGameFullScreen || !PrimaryBufferSurface || !surface ) {
				hBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
				FillRect(hdc, &paint.rcPaint, hBrush);
			}
			else {
				if( SavedAppSettings.RenderMode == RM_Software &&
					!WinVidCheckGameWindowPalette(hWnd) &&
					RenderBufferSurface )
				{
					WinVidClearBuffer(RenderBufferSurface, NULL, 0);
				}
				UpdateFrame(false, NULL);
			}
#endif // (DIRECT3D_VERSION >= 0x900)
			EndPaint(hWnd, &paint);
			return 0;

		case WM_ACTIVATE :
#if (DIRECT3D_VERSION < 0x900)
			if( LOWORD(wParam) && DDrawPalette && PrimaryBufferSurface )
				PrimaryBufferSurface->SetPalette(DDrawPalette);
#endif // (DIRECT3D_VERSION < 0x900)
			break;

		case WM_ERASEBKGND :
			return 1;

		case WM_ACTIVATEAPP :
#ifdef FEATURE_AUDIO_IMPROVED
			// NOTE: If CD audio volume is set to zero, music is paused.
			// To resume the music, the volume must be set to non zero value.
			if( wParam && !IsGameWindowActive ) {
				if( cdVolume ) {
					S_CDVolume(cdVolume);
				}
			} else if ( !wParam && IsGameWindowActive ) {
				cdVolume = S_GetCDVolume();
				S_CDVolume(0);
			}
#endif // FEATURE_AUDIO_IMPROVED
#ifdef FEATURE_INPUT_IMPROVED
			if( !wParam && IsGameWindowActive ) {
				JoyVibrationMute();
			}
#endif // FEATURE_INPUT_IMPROVED
			if( wParam && !IsGameWindowActive && IsGameFullScreen && SavedAppSettings.RenderMode == RM_Hardware )
				WinVidNeedToResetBuffers = true;
			IsGameWindowActive = ( wParam != 0 );
			break;

		case WM_SETCURSOR :
			if( IsGameFullScreen ) {
				SetCursor(NULL);
				return 1;
			}
			break;

		case WM_GETMINMAXINFO :
			if( WinVidGetMinMaxInfo((LPMINMAXINFO)lParam) )
				return 0;
			break;

		case WM_NCPAINT :
		case WM_NCLBUTTONDOWN :
		case WM_NCLBUTTONDBLCLK :
		case WM_NCRBUTTONDOWN :
		case WM_NCRBUTTONDBLCLK :
		case WM_NCMBUTTONDOWN :
		case WM_NCMBUTTONDBLCLK :
			if( IsGameFullScreen ) return 0;
			break;

		case WM_SYSCOMMAND :
			if( wParam == SC_KEYMENU ) return 0;
			break;

		case WM_SIZING :
			WinVidResizeGameWindow(hWnd, wParam, (LPRECT)lParam);
			break;

		case WM_MOVING :
			if( IsGameFullScreen || IsGameWindowMaximized ) {
				GetWindowRect(hWnd, (LPRECT)lParam);
				return 1;
			}
			break;

		case WM_ENTERSIZEMOVE :
			IsGameWindowChanging = true;
			break;

		case WM_EXITSIZEMOVE :
			IsGameWindowChanging = false;
			break;

#if (DIRECT3D_VERSION < 0x900)
		case WM_PALETTECHANGED :
			if( hWnd != (HWND)wParam && !IsGameFullScreen && DDrawPalette )
				InvalidateRect(hWnd, NULL, FALSE);
			break;
#endif // (DIRECT3D_VERSION < 0x900)
	}
	return DefWindowProc(hWnd, Msg, wParam, lParam);
}

void __cdecl WinVidResizeGameWindow(HWND hWnd, int edge, LPRECT rect) {
	bool isShiftPressed;
	int width, height;

	if( IsGameFullScreen ) {
		rect->left = 0;
		rect->top = 0;
		rect->right = FullScreenWidth;
		rect->bottom = FullScreenHeight;
	}

	isShiftPressed = ( GetAsyncKeyState(VK_SHIFT) < 0 );
	width  = rect->right  - rect->left;
	height = rect->bottom - rect->top;
	GameWindowCalculateSizeFromClientByZero(&width, &height);

	if( edge == WMSZ_TOP || edge == WMSZ_BOTTOM ) {
		if( isShiftPressed )
			height &= ~0x1F;
		width = CalculateWindowWidth(width, height);
	} else {
		if( isShiftPressed )
			width &= ~0x1F;
		height = CalculateWindowHeight(width, height);
	}

	if( IsMinWindowSizeSet ) {
		if( width < MinWindowClientWidth )
			width = MinWindowClientWidth;
		if( height < MinWindowClientHeight )
			height = MinWindowClientHeight;
	}

	if( IsMaxWindowSizeSet ) {
		if( width > MaxWindowClientWidth )
			width = MaxWindowClientWidth;
		if( height > MaxWindowClientHeight )
			height = MaxWindowClientHeight;
	}

	GameWindowCalculateSizeFromClient(&width, &height);

	switch( edge ) {
		case WMSZ_TOPLEFT :
			rect->left = rect->right - width;
			rect->top = rect->bottom - height;
			break;

		case WMSZ_RIGHT :
		case WMSZ_BOTTOM :
		case WMSZ_BOTTOMRIGHT :
			rect->right = rect->left + width;
			rect->bottom = rect->top + height;
			break;

		case WMSZ_LEFT :
		case WMSZ_BOTTOMLEFT :
			rect->left = rect->right - width;
			rect->bottom = rect->top + height;
			break;

		case WMSZ_TOP :
		case WMSZ_TOPRIGHT :
			rect->right = rect->left + width;
			rect->top = rect->bottom - height;
			break;
	}
}

bool __cdecl WinVidCheckGameWindowPalette(HWND hWnd) {
	HDC hdc;
	PALETTEENTRY sysPalette[256];
	RGB888 bufPalette[256];

	hdc = GetDC(hWnd);
	if( hdc == NULL )
		return false;

	GetSystemPaletteEntries(hdc, 0, 256, sysPalette);
	ReleaseDC(hWnd, hdc);

	for( int i=0; i < 256; ++i ) {
		bufPalette[i].red   = sysPalette[i].peRed;
		bufPalette[i].green = sysPalette[i].peGreen;
		bufPalette[i].blue  = sysPalette[i].peBlue;
	}

	return ( !memcmp(bufPalette, GamePalette8, sizeof(bufPalette)) );
}

bool __cdecl WinVidCreateGameWindow() {
	RECT rect;

	IsGameWindowActive		= true;
	IsGameWindowShow		= true;
	IsDDrawGameWindowShow	= false;
	IsMessageLoopClosed		= false;
	IsGameWindowUpdating	= false;
	IsGameWindowMinimized	= false;
	IsGameWindowMaximized	= false;
	IsGameWindowCreated		= false;
	IsGameFullScreen		= false;
	IsMinMaxInfoSpecial		= false;
	IsGameWindowChanging	= false;
	WinVidClearMinWindowSize();
	WinVidClearMaxWindowSize();

	HGameWindow = CreateWindowEx(WS_EX_APPWINDOW, GameClassName, GameWindowName, WS_OVERLAPPEDWINDOW,
								CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
								NULL, NULL, GameModule, NULL);
	if( HGameWindow == NULL)
		return false;

	GetWindowRect(HGameWindow, &rect);
	GameWindow_X = rect.left;
	GameWindow_Y = rect.top;
	WinVidHideGameWindow();

	return true;
}

void __cdecl WinVidFreeWindow() {
	WinVidExitMessage();
	UnregisterClass(GameClassName, GameModule);
}

void __cdecl WinVidExitMessage() {
	if( HGameWindow && IsWindow(HGameWindow) ) {
		PostMessage(HGameWindow, WM_CLOSE, 0, 0);
		while( WinVidSpinMessageLoop(false) ) /* just wait */;
		HGameWindow = NULL;
	}
}

DISPLAY_ADAPTER_NODE *__cdecl WinVidGetDisplayAdapter(GUID *lpGuid) {
	DISPLAY_ADAPTER_NODE *adapter;

	if( lpGuid != NULL ) {
		for( adapter = DisplayAdapterList.head; adapter; adapter = adapter->next ) {
			if( !memcmp(&adapter->body.adapterGuid, lpGuid, sizeof(GUID)) )
				return adapter;
		}
	}
	return PrimaryDisplayAdapter;
}

void __cdecl WinVidStart() {
	if( SavedAppSettings.PreferredDisplayAdapter == NULL )
#if (DIRECT3D_VERSION >= 0x900)
		throw ERR_CantInitRenderer;
#else // (DIRECT3D_VERSION >= 0x900)
		throw ERR_CantCreateDirectDraw;
#endif // (DIRECT3D_VERSION >= 0x900)

	DISPLAY_ADAPTER *preferred = &SavedAppSettings.PreferredDisplayAdapter->body;
	CurrentDisplayAdapter = *preferred;

	FlaggedStringCopy(&CurrentDisplayAdapter.driverDescription, &preferred->driverDescription);
	FlaggedStringCopy(&CurrentDisplayAdapter.driverName, &preferred->driverName);

	DisplayModeListInit(&CurrentDisplayAdapter.hwDispModeList);
	DisplayModeListCopy(&CurrentDisplayAdapter.hwDispModeList, &preferred->hwDispModeList);

	DisplayModeListInit(&CurrentDisplayAdapter.swDispModeList);
	DisplayModeListCopy(&CurrentDisplayAdapter.swDispModeList, &preferred->swDispModeList);

#if (DIRECT3D_VERSION < 0x900)
	if( !DDrawCreate(CurrentDisplayAdapter.lpAdapterGuid) )
		throw ERR_CantCreateDirectDraw;
#endif // (DIRECT3D_VERSION < 0x900)
}

void __cdecl WinVidFinish() {
#if (DIRECT3D_VERSION < 0x900)
	if( IsDDrawGameWindowShow )
		HideDDrawGameWindow();
	DDrawRelease();
#endif // (DIRECT3D_VERSION < 0x900)
}

void __thiscall DisplayModeListInit(DISPLAY_MODE_LIST *pList) {
	pList->head = NULL;
	pList->tail = NULL;
	pList->dwCount = 0;
}

void __thiscall DisplayModeListDelete(DISPLAY_MODE_LIST *pList) {
	DISPLAY_MODE_NODE *node;
	DISPLAY_MODE_NODE *nextNode;

	for( node = pList->head; node; node = nextNode ) {
		nextNode = node->next;
		delete(node);
	}
	DisplayModeListInit(pList);
}

DISPLAY_MODE *__thiscall InsertDisplayMode(DISPLAY_MODE_LIST *modeList, DISPLAY_MODE_NODE *before) {
	if( !before || !before->previous )
		return InsertDisplayModeInListHead(modeList);

	DISPLAY_MODE_NODE *node = new DISPLAY_MODE_NODE;
	if( !node )
		return NULL;

	before->previous->next = node;
	node->previous = before->previous;

	before->previous = node;
	node->next = before;

	modeList->dwCount++;
	return &node->body;
}

DISPLAY_MODE *__thiscall InsertDisplayModeInListHead(DISPLAY_MODE_LIST *modeList) {
	DISPLAY_MODE_NODE *node = new DISPLAY_MODE_NODE;
	if( !node )
		return NULL;

	node->next = modeList->head;
	node->previous = NULL;

	if( modeList->head )
		modeList->head->previous = node;

	if( !modeList->tail )
		modeList->tail = node;

	modeList->head = node;
	modeList->dwCount++;
	return &node->body;
}

DISPLAY_MODE *__thiscall InsertDisplayModeInListTail(DISPLAY_MODE_LIST *modeList) {
	DISPLAY_MODE_NODE *node = new DISPLAY_MODE_NODE;
	if( !node )
		return NULL;

	node->next = NULL;
	node->previous = modeList->tail;

	if( modeList->tail )
		modeList->tail->next = node;

	if( !modeList->head )
		modeList->head = node;

	modeList->tail = node;
	modeList->dwCount++;
	return &node->body;
}

/*
 * Inject function
 */
void Inject_WinVid() {
#if (DIRECT3D_VERSION < 0x900)
	INJECT(0x00444C80, DDrawCreate);
	INJECT(0x00444CE0, DDrawRelease);
#endif // (DIRECT3D_VERSION < 0x900)
	INJECT(0x00444D20, GameWindowCalculateSizeFromClient);
	INJECT(0x00444DA0, GameWindowCalculateSizeFromClientByZero);
	INJECT(0x00444E10, WinVidSetMinWindowSize);
	INJECT(0x00444E60, WinVidClearMinWindowSize);
	INJECT(0x00444E70, WinVidSetMaxWindowSize);
	INJECT(0x00444EC0, WinVidClearMaxWindowSize);
	INJECT(0x00444ED0, CalculateWindowWidth);
	INJECT(0x00444F20, CalculateWindowHeight);
	INJECT(0x00444F50, WinVidGetMinMaxInfo);
	INJECT(0x00445060, WinVidFindGameWindow);
	INJECT(0x00445080, WinVidSpinMessageLoop);
	INJECT(0x00445170, WinVidShowGameWindow);
	INJECT(0x004451C0, WinVidHideGameWindow);
	INJECT(0x00445200, WinVidSetGameWindowSize);
#if (DIRECT3D_VERSION < 0x900)
	INJECT(0x00445240, ShowDDrawGameWindow);
	INJECT(0x004452F0, HideDDrawGameWindow);
	INJECT(0x00445380, DDrawSurfaceCreate);
	INJECT(0x004453D0, DDrawSurfaceRestoreLost);
	INJECT(0x00445420, WinVidClearBuffer);
	INJECT(0x00445470, WinVidBufferLock);
	INJECT(0x004454B0, WinVidBufferUnlock);
	INJECT(0x004454E0, WinVidCopyBitmapToBuffer);
	INJECT(0x00445570, GetRenderBitDepth);
	INJECT(0x00445600, WinVidGetColorBitMasks);
	INJECT(0x00445680, BitMaskGetNumberOfBits);
	INJECT(0x004456D0, CalculateCompatibleColor);
#endif // (DIRECT3D_VERSION < 0x900)
	INJECT(0x00445740, WinVidGetDisplayMode);
	INJECT(0x004457D0, WinVidGoFullScreen);
	INJECT(0x00445860, WinVidGoWindowed);
	INJECT(0x00445970, WinVidSetDisplayAdapter);
	INJECT(0x00445A50, CompareVideoModes);
	INJECT(0x00445AA0, WinVidGetDisplayModes);
#if (DIRECT3D_VERSION < 0x900)
	INJECT(0x00445B00, EnumDisplayModesCallback);
#endif // (DIRECT3D_VERSION < 0x900)
	INJECT(0x00445EC0, WinVidInit);
	INJECT(0x00445F00, WinVidGetDisplayAdapters);
	INJECT(0x00445FB0, FlaggedStringDelete);
	INJECT(0x00445FD0, EnumerateDisplayAdapters);
#if (DIRECT3D_VERSION < 0x900)
	INJECT(0x00445FF0, EnumDisplayAdaptersCallback);
#endif // (DIRECT3D_VERSION < 0x900)
	INJECT(0x004461B0, FlaggedStringsCreate);
	INJECT(0x004461F0, WinVidRegisterGameWindowClass);
	INJECT(0x00446260, WinVidGameWindowProc);
	INJECT(0x00446870, WinVidResizeGameWindow);
	INJECT(0x00446A50, WinVidCheckGameWindowPalette);
	INJECT(0x00446B10, WinVidCreateGameWindow);
	INJECT(0x00446BE0, WinVidFreeWindow);
	INJECT(0x00446C10, WinVidExitMessage);
	INJECT(0x00446C60, WinVidGetDisplayAdapter);
	INJECT(0x00446CB0, WinVidStart);
	INJECT(0x00447030, WinVidFinish);
	INJECT(0x00447050, DisplayModeListInit);
	INJECT(0x00447060, DisplayModeListDelete);
//	INJECT(0x004470A0, InsertDisplayMode); // NOTE: new one is not compatible anymore (original one is junk!!!)
	INJECT(0x004470C0, InsertDisplayModeInListHead);
	INJECT(0x00447110, InsertDisplayModeInListTail);
}
