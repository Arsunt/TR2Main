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
#include "specific/screenshot.h"
#include "specific/winvid.h"
#include "global/vars.h"

#ifdef FEATURE_SCREENSHOT_IMPROVED
#include "modding/file_utils.h"
#include "modding/gdi_utils.h"

extern LPDDS CaptureBufferSurface;

DWORD ScreenshotFormat = 1;
char ScreenshotPath[MAX_PATH];

typedef struct {
	HBITMAP bitmap;
	GDI_FILEFMT format;
	BYTE quality;
	char fileName[MAX_PATH];
} TASK_PARAMS;

static DWORD WINAPI SaveImageTask(CONST LPVOID lpParam) {
	TASK_PARAMS *params = (TASK_PARAMS *)lpParam;
	if( params != NULL ) {
		if( params->bitmap != NULL ) {
			GDI_SaveImageFile(params->fileName, params->format, params->quality, params->bitmap);
			DeleteObject(params->bitmap);
		}
		delete params;
	}
	ExitThread(0);
}

static void __cdecl ScreenShotPNG(LPDDS screen) {
	static SYSTEMTIME lastTime = {0, 0, 0, 0, 0, 0, 0, 0};
	static int lastIndex = 0;
	RECT rect = {0, 0, 0, 0};
	HDC dc;

#if (DIRECT3D_VERSION >= 0x900)
	screen = NULL;
	DISPLAY_MODE mode;

	if( CaptureBufferSurface != NULL ) {
		screen = CaptureBufferSurface;
	} else if( !WinVidGetDisplayMode(&mode)
		|| FAILED(D3DDev->CreateOffscreenPlainSurface(mode.width, mode.height, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &screen, NULL))
		|| FAILED(D3DDev->GetFrontBufferData(0, screen)) )
	{
		if( screen != NULL ) screen->Release();
		return;
	}

	if FAILED(screen->GetDC(&dc)) {
		if( screen != CaptureBufferSurface ) {
			screen->Release();
		}
		return;
	}
#else // (DIRECT3D_VERSION >= 0x900)
	if( screen == NULL || FAILED(screen->GetDC(&dc)) ) {
		return;
	}
#endif // (DIRECT3D_VERSION >= 0x900)

	if( GetClientRect(HGameWindow, &rect) ) {
		HBITMAP bitmap;
		LPVOID lpBits;

		if( CaptureBufferSurface == NULL ) {
			MapWindowPoints(HGameWindow, GetParent(HGameWindow), (LPPOINT)&rect, 2);
		}

		bitmap = CreateBitmapFromDC(dc, &rect, &lpBits, WinVidPalette);
		if( bitmap != NULL ) {
			TASK_PARAMS *params = new TASK_PARAMS;
			params->bitmap = bitmap;
			params->format = GDI_PNG;
			params->quality = 100;

			CreateDateTimeFilename(params->fileName, sizeof(params->fileName), ScreenshotPath, ".png", &lastTime, &lastIndex);
			CreateDirectories(params->fileName, true);
			if( !CreateThread(NULL, 0, &SaveImageTask, params, 0, NULL) ) {
				// if failed to create a thread, we just save the image
				GDI_SaveImageFile(params->fileName, params->format, params->quality, params->bitmap);
				DeleteObject(bitmap);
				delete params;
			}
		}
	}
	screen->ReleaseDC(dc);
#if (DIRECT3D_VERSION >= 0x900)
	if( screen != CaptureBufferSurface ) {
		screen->Release();
	}
#endif // (DIRECT3D_VERSION >= 0x900)
}
#endif // FEATURE_SCREENSHOT_IMPROVED


static TGA_HEADER ScreenShotTgaHeader = {
	0, 0,
	.dataTypeCode = 2, // Uncompressed, RGB images
	0, 0, 0, 0, 0,
	.width = 320,
	.height = 256,
	.bpp = 16,
	.imageDescriptor = 0,
};


// NOTE: This function is not presented in the original code
// but the code is taken away form ScreenShot() and extended
// to be compatible with 24/32 bit
static void __cdecl ScreenShotTGA(LPDDS screen, BYTE tgaBpp) {
	static int scrshotNumber = 0;
	DWORD i, j;
	BYTE *src, *dst;
	DDSDESC desc;
	BYTE *tgaPic = NULL;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD bytesWritten;
#if defined(FEATURE_SCREENSHOT_IMPROVED)
	char fileName[MAX_PATH];
#else // !FEATURE_SCREENSHOT_IMPROVED
	char fileName[128];
#endif // FEATURE_SCREENSHOT_IMPROVED
	DWORD width = 0;
	DWORD height = 0;

#if (DIRECT3D_VERSION >= 0x900)
	if( tgaBpp != 24 )
		return;
#else // (DIRECT3D_VERSION >= 0x900)
	if( tgaBpp != 16 && tgaBpp != 24 )
		return;
#endif // (DIRECT3D_VERSION >= 0x900)

	memset(&desc, 0, sizeof(desc));
#if (DIRECT3D_VERSION < 0x900)
	desc.dwSize = sizeof(desc);
#endif // (DIRECT3D_VERSION < 0x900)

#if defined(FEATURE_SCREENSHOT_IMPROVED)
	RECT rect = {0,0,0,0};

	// do game window screenshot, not the whole screen
	if( GetClientRect(HGameWindow, &rect) ) {
		if( CaptureBufferSurface == NULL ) {
			MapWindowPoints(HGameWindow, GetParent(HGameWindow), (LPPOINT)&rect, 2);
		}
		width = ABS(rect.right - rect.left);
		height = ABS(rect.bottom - rect.top);
	}

#if (DIRECT3D_VERSION >= 0x900)
	screen = NULL;
	DISPLAY_MODE mode;

	if( CaptureBufferSurface != NULL ) {
		screen = CaptureBufferSurface;
	} else if( !WinVidGetDisplayMode(&mode)
		|| FAILED(D3DDev->CreateOffscreenPlainSurface(mode.width, mode.height, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &screen, NULL))
		|| FAILED(D3DDev->GetFrontBufferData(0, screen)) )
	{
		goto CLEANUP;
	}

	if FAILED(screen->LockRect(&desc, &rect, D3DLOCK_READONLY)) {
		goto CLEANUP;
	}
#else // (DIRECT3D_VERSION >= 0x900)
	HRESULT rc;
	do {
		rc = screen->Lock(&rect, &desc, DDLOCK_READONLY|DDLOCK_WAIT, NULL);
	} while( rc == DDERR_WASSTILLDRAWING );

	if( rc == DDERR_SURFACELOST )
		rc = screen->Restore();
	if FAILED(rc)
		return;

	if( width == 0 || width > desc.dwWidth )
		width = desc.dwWidth;
	if( height == 0 || height > desc.dwHeight )
		height = desc.dwHeight;
#endif // (DIRECT3D_VERSION >= 0x900)

	scrshotNumber = CreateSequenceFilename(fileName, sizeof(fileName), ScreenshotPath, ".tga", "tomb", 4, scrshotNumber);
	if( scrshotNumber < 0 ) goto CLEANUP;
	++scrshotNumber;
	CreateDirectories(fileName, true); // create whole path just in case if it's not created yet
#else // !FEATURE_SCREENSHOT_IMPROVED
#if (DIRECT3D_VERSION < 0x900)
	if FAILED(WinVidBufferLock(screen, &desc, DDLOCK_WRITEONLY|DDLOCK_WAIT))
		return;

	width = desc.dwWidth;
	height = desc.dwHeight;
#endif // (DIRECT3D_VERSION < 0x900)

	wsprintf(fileName, "tomb%04d.tga", scrshotNumber++);
#endif // FEATURE_SCREENSHOT_IMPROVED

	hFile = CreateFile(fileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if( hFile == INVALID_HANDLE_VALUE )
		goto CLEANUP;

	ScreenShotTgaHeader.width = width;
	ScreenShotTgaHeader.height = height;
	ScreenShotTgaHeader.bpp = tgaBpp;
	WriteFile(hFile, &ScreenShotTgaHeader, sizeof(TGA_HEADER), &bytesWritten, NULL);

#if defined(FEATURE_SCREENSHOT_IMPROVED)
	// NOTE: There was unsafe memory usage in the original code. The game just used GameAllocMemPointer buffer!
	// No new memory allocations. On higher resolutions there was critical data overwriting and game crashed
	tgaPic = (BYTE *)GlobalAlloc(GMEM_FIXED, width*height*(tgaBpp/8));
#else // !FEATURE_SCREENSHOT_IMPROVED
	tgaPic = (BYTE *)GameAllocMemPointer;
#endif // FEATURE_SCREENSHOT_IMPROVED

	if( tgaPic == NULL )
		goto CLEANUP;

	// We need to load bitmap lines to TGA starting from the bottom line
#if (DIRECT3D_VERSION >= 0x900)
	src = (BYTE *)desc.pBits + desc.Pitch*(height - 1);
#else // (DIRECT3D_VERSION >= 0x900)
#if defined(FEATURE_SCREENSHOT_IMPROVED)
	// NOTE: There was bug in the original formula: src = lpSurface + lPitch * dwHeight
	// Height must be subtracted by 1 in this formula
	src = (BYTE *)desc.lpSurface + desc.lPitch*(height - 1);
#else // !FEATURE_SCREENSHOT_IMPROVED
	src = (BYTE *)desc.lpSurface + desc.lPitch*height;
#endif // FEATURE_SCREENSHOT_IMPROVED
#endif // (DIRECT3D_VERSION >= 0x900)

	dst = tgaPic;
#if (DIRECT3D_VERSION >= 0x900)
	for( i=0; i < height; ++i ) {
		for( j=0; j < width; ++j ) {
			((RGB888 *)dst)[j] = *(RGB888*)(src + j * 4);
		}
		src -= desc.Pitch;
		dst += sizeof(RGB888)*width;
	}
#else // (DIRECT3D_VERSION >= 0x900)
	if( tgaBpp == 16 ) {
		for( i=0; i < height; ++i ) {
			// R5G6B5 - not TGA compatible
			if( desc.ddpfPixelFormat.dwRBitMask == 0xF800 ) {
				// right shift highest 10 bits (R+G) over lowest G bit
				for( j=0; j < width; ++j ) {
					UINT16 sample = ((UINT16 *)src)[j];
					((UINT16 *)dst)[j] = ((sample & 0xFFC0) >> 1) | (sample & 0x001F);
				}
			} else {
				// X1R5G5B5 - already TGA compatible
				memcpy(dst, src, sizeof(UINT16)*width);
			}
			src -= desc.lPitch;
			dst += sizeof(UINT16)*width;
		}
	} else {
		for( i=0; i < height; ++i ) {
			if( desc.ddpfPixelFormat.dwRGBBitCount == 24 ) {
				memcpy(dst, src, sizeof(RGB888)*width);
			} else {
				for( j=0; j < width; ++j ) {
					((RGB888 *)dst)[j] = *(RGB888*)(src + j * (desc.ddpfPixelFormat.dwRGBBitCount / 8));
				}
			}
			src -= desc.lPitch;
			dst += sizeof(RGB888)*width;
		}
	}
#endif // (DIRECT3D_VERSION >= 0x900)
	WriteFile(hFile, tgaPic, width*height*(tgaBpp/8), &bytesWritten, NULL);

CLEANUP :
#if defined(FEATURE_SCREENSHOT_IMPROVED)
	if( tgaPic != NULL )
		GlobalFree((HGLOBAL)tgaPic);
#endif // FEATURE_SCREENSHOT_IMPROVED

	if( hFile != INVALID_HANDLE_VALUE )
		CloseHandle(hFile);

#if (DIRECT3D_VERSION >= 0x900)
	if( screen != NULL ) {
		screen->UnlockRect();
		if( screen != CaptureBufferSurface ) {
			screen->Release();
		}
	}
#else // (DIRECT3D_VERSION >= 0x900)
#if defined(FEATURE_SCREENSHOT_IMPROVED)
	screen->Unlock(desc.lpSurface);
#else // !FEATURE_SCREENSHOT_IMPROVED
	WinVidBufferUnlock(screen, &desc);
#endif // FEATURE_SCREENSHOT_IMPROVED
#endif // (DIRECT3D_VERSION >= 0x900)
}


void __cdecl ScreenShotPCX() {
	static int scrshotNumber = 0;
#if (DIRECT3D_VERSION < 0x900)
	HRESULT rc;
	LPDDS screen;
	DDSDESC desc;
#endif // (DIRECT3D_VERSION < 0x900)
	BYTE *pcxData = NULL;
	DWORD pcxSize;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD bytesWritten;
#if defined(FEATURE_SCREENSHOT_IMPROVED)
	char fileName[MAX_PATH];
#else // !FEATURE_SCREENSHOT_IMPROVED
	char fileName[128];
#endif // FEATURE_SCREENSHOT_IMPROVED

#if (DIRECT3D_VERSION >= 0x900)
	if( !RenderBuffer.bitmap || !RenderBuffer.width || !RenderBuffer.height ) return;
	pcxSize = CompPCX(RenderBuffer.bitmap, RenderBuffer.width, RenderBuffer.height, GamePalette8, &pcxData);
#else // (DIRECT3D_VERSION >= 0x900)
	screen = ( SavedAppSettings.RenderMode == RM_Software ) ? RenderBufferSurface : PrimaryBufferSurface;
	desc.dwSize = sizeof(desc);

	do {
		rc = screen->Lock(NULL, &desc, DDLOCK_SURFACEMEMORYPTR, NULL);
	} while( rc == DDERR_WASSTILLDRAWING );

	if( rc == DDERR_SURFACELOST )
		rc = screen->Restore();
	if FAILED(rc)
		return;

	pcxSize = CompPCX((BYTE *)desc.lpSurface, desc.dwWidth, desc.dwHeight, GamePalette8, &pcxData);
	screen->Unlock(&desc);
#endif // (DIRECT3D_VERSION >= 0x900)

	if( pcxSize == 0 || pcxData == NULL )
		return;

#if defined(FEATURE_SCREENSHOT_IMPROVED)
	scrshotNumber = CreateSequenceFilename(fileName, sizeof(fileName), ScreenshotPath, ".pcx", "tomb", 4, scrshotNumber);
	if( scrshotNumber < 0 ) return;
	++scrshotNumber;
	CreateDirectories(fileName, true); // create whole path just in case if it is not created yet
#else // !FEATURE_SCREENSHOT_IMPROVED
	if( ++scrshotNumber > 9999 ) scrshotNumber = 1;
	wsprintf(fileName, "tomb%04d.pcx", scrshotNumber);
#endif // FEATURE_SCREENSHOT_IMPROVED

	hFile = CreateFile(fileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if( hFile != INVALID_HANDLE_VALUE ) {
		WriteFile(hFile, pcxData, pcxSize, &bytesWritten, NULL);
		CloseHandle(hFile);
	}
	GlobalFree(pcxData);
}


DWORD __cdecl CompPCX(BYTE *bitmap, DWORD width, DWORD height, RGB888 *palette, BYTE **pcxData) {
	DWORD i;
	PCX_HEADER *pcxHeader;
	BYTE *picData;

	*pcxData = (BYTE *)GlobalAlloc(GMEM_FIXED, width*height*2 + sizeof(PCX_HEADER) + sizeof(RGB888)*256);
	if( *pcxData == NULL )
		return 0;

	pcxHeader = *(PCX_HEADER **)pcxData;

	pcxHeader->manufacturer = 10;
	pcxHeader->version = 5;
	pcxHeader->rle = 1;
	pcxHeader->bpp = 8;
	pcxHeader->planes = 1;

	pcxHeader->xMin = 0;
	pcxHeader->yMin = 0;
	pcxHeader->xMax = width - 1;
	pcxHeader->yMax = height - 1;
	pcxHeader->h_dpi = width;
	pcxHeader->v_dpi = height;
	pcxHeader->bytesPerLine = width;

	picData = *pcxData + sizeof(PCX_HEADER);
	for( i=0; i<height; ++i ) {
		picData += EncodeLinePCX(bitmap, width, picData);
		bitmap += width;
	}

	*(picData++) = 0x0C;
	memcpy(picData, palette, sizeof(RGB888)*256);

	return (DWORD)(picData - *pcxData + sizeof(RGB888)*256); // pcx data size
}


DWORD __cdecl EncodeLinePCX(BYTE *src, DWORD width, BYTE *dst) {
	BYTE current, add;
	DWORD total = 0;
	BYTE runCount = 1;
	BYTE last = *src;

	for( DWORD i=1; i<width; ++i ) {
		current = *(++src);
		if( current == last ) {
			++runCount;
			if( runCount == 63 ) {
				add = EncodePutPCX(last, runCount, dst);
				if( add == 0 ) {
					return 0;
				}
				total += add;
				dst += add;
				runCount = 0;
			}
		} else {
			if( runCount != 0 ) {
				add = EncodePutPCX(last, runCount, dst);
				if( add == 0 ) {
					return 0;
				}
				total += add;
				dst += add;
			}
			last = current;
			runCount = 1;
		}
	}

	if( runCount ) {
		add = EncodePutPCX(last, runCount, dst);
		if( add == 0) {
			return 0;
		}
		total += add;
		dst += add;
	}
	return total;
}


DWORD __cdecl EncodePutPCX(BYTE value, BYTE num, BYTE *buffer) {
	if( num == 0 || num > 63 ) {
		return 0;
	}

	if( num == 1 && (value & 0xC0) != 0xC0 ) {
		buffer[0] = value;
		return 1;
	}

	buffer[0] = num | 0xC0;
	buffer[1] = value;
	return 2;
}


void __cdecl ScreenShot(LPDDS screen) {
#if defined(FEATURE_SCREENSHOT_IMPROVED)
#if (DIRECT3D_VERSION < 0x900)
	if( SavedAppSettings.RenderMode == RM_Software ) {
		screen = RenderBufferSurface;
	} else if( CaptureBufferSurface != NULL ) {
		screen = CaptureBufferSurface;
	}
#endif // (DIRECT3D_VERSION < 0x900)

	if( ScreenshotFormat > 0 ) {
		ScreenShotPNG(screen);
		return;
	}
#endif // FEATURE_SCREENSHOT_IMPROVED

#if (DIRECT3D_VERSION >= 0x900)
	if( SavedAppSettings.RenderMode == RM_Software ) {
		ScreenShotPCX();
	} else {
		ScreenShotTGA(screen, 24);
	}
#else // (DIRECT3D_VERSION >= 0x900)
	DDSDESC desc;

	memset(&desc, 0, sizeof(desc));
	desc.dwSize = sizeof(desc);

	if SUCCEEDED( screen->GetSurfaceDesc(&desc)) {
		switch( desc.ddpfPixelFormat.dwRGBBitCount ) {
			case 8 :
				ScreenShotPCX();
				break;

			case 16 :
				ScreenShotTGA(screen, 16);
				break;

#if defined(FEATURE_SCREENSHOT_IMPROVED)
			case 24 :
			case 32 :
				// NOTE: the original game cannot make 24/32 bit screenshots
				ScreenShotTGA(screen, 24);
				break;
#endif // FEATURE_SCREENSHOT_IMPROVED

			default :
				break;
		}
	}
#endif // (DIRECT3D_VERSION >= 0x900)
}


/*
 * Inject function
 */
void Inject_Screenshot() {
	INJECT(0x0044E9A0, ScreenShotPCX);
	INJECT(0x0044EAB0, CompPCX);
	INJECT(0x0044EB60, EncodeLinePCX);
	INJECT(0x0044EC40, EncodePutPCX);
	INJECT(0x0044EC80, ScreenShot);
}
