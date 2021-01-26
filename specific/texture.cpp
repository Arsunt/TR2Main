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
#include "specific/texture.h"
#include "3dsystem/3d_gen.h"
#include "specific/hwr.h"
#include "specific/winvid.h"
#include "global/vars.h"
#include <limits.h>

#ifdef FEATURE_BACKGROUND_IMPROVED
#include "modding/background_new.h"
#endif // FEATURE_BACKGROUND_IMPROVED

#ifdef FEATURE_EXTENDED_LIMITS
PHD_TEXTURE PhdTextureInfo[0x2000];
BYTE LabTextureUVFlags[0x2000];
BYTE *TexturePageBuffer8[128];
HWR_TEXHANDLE HWR_PageHandles[128];
int HWR_TexturePageIndexes[128];
#endif // FEATURE_EXTENDED_LIMITS

#if defined(FEATURE_EXTENDED_LIMITS) || defined(FEATURE_BACKGROUND_IMPROVED)
TEXPAGE_DESC TexturePages[256];
#else // defined(FEATURE_EXTENDED_LIMITS) || defined(FEATURE_BACKGROUND_IMPROVED)
TEXPAGE_DESC TexturePages[32];
#endif // defined(FEATURE_EXTENDED_LIMITS) || defined(FEATURE_BACKGROUND_IMPROVED)

#if (DIRECT3D_VERSION >= 0x900)
RGB888 *TexturePalettes[256];
#elif defined(FEATURE_EXTENDED_LIMITS) || defined(FEATURE_BACKGROUND_IMPROVED)
LPDIRECTDRAWPALETTE TexturePalettes[256];
#else // defined(FEATURE_EXTENDED_LIMITS) || defined(FEATURE_BACKGROUND_IMPROVED)
LPDIRECTDRAWPALETTE TexturePalettes[16];
#endif // defined(FEATURE_EXTENDED_LIMITS) || defined(FEATURE_BACKGROUND_IMPROVED)

#ifdef FEATURE_VIDEOFX_IMPROVED
DWORD ReflectionMode = 2;

#if (DIRECT3D_VERSION >= 0x900)
static LPDIRECT3DTEXTURE9 EnvmapTexture = NULL;
#else // (DIRECT3D_VERSION >= 0x900)
extern LPDDS EnvmapBufferSurface;
extern LPDDS CaptureBufferSurface;
static LPDIRECT3DTEXTURE2 EnvmapTexture = NULL;
#endif // (DIRECT3D_VERSION >= 0x900)

static HWR_TEXHANDLE EnvmapTextureHandle = 0;

static DWORD GetEnvmapSide() {
	static const DWORD mapside[] = {64, 256, 1024};
	if( ReflectionMode < 1 || ReflectionMode > 3 ) return 0;
	DWORD side = MIN(mapside[3 - ReflectionMode], GetMaxTextureSize());
	DWORD sideLimit = MIN(GameVidWidth, GameVidHeight);
	while( side > sideLimit ) side >>= 1;
	return side;
}

#if (DIRECT3D_VERSION >= 0x900)
static bool __cdecl CreateEnvmapTexture() {
	DWORD side = GetEnvmapSide();
	if( !side ) return false;
	if( EnvmapTexture ) return true;
	return SUCCEEDED(D3DDev->CreateTexture(side, side, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &EnvmapTexture, 0));
}
#else // (DIRECT3D_VERSION >= 0x900)
static bool __cdecl CreateEnvmapBufferSurface() {
	DWORD side = GetEnvmapSide();
	if( !side ) return false;
	if( EnvmapBufferSurface ) return true;

	DDSDESC dsp;
	memset(&dsp, 0, sizeof(dsp));
	dsp.dwSize = sizeof(dsp);
	dsp.dwFlags = DDSD_WIDTH|DDSD_HEIGHT|DDSD_CAPS;
	dsp.dwWidth = side;
	dsp.dwHeight = side;
	dsp.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY|DDSCAPS_TEXTURE;

	if FAILED(DDrawSurfaceCreate(&dsp, &EnvmapBufferSurface))
		return -1;

	WinVidClearBuffer(EnvmapBufferSurface, NULL, 0);
	return 0;
}
#endif // (DIRECT3D_VERSION >= 0x900)

void FreeEnvmapTexture() {
	if( EnvmapTexture ) {
		EnvmapTexture->Release();
		EnvmapTexture = NULL;
	}
	EnvmapTextureHandle = 0;
}

bool SetEnvmapTexture(LPDDS surface) {
	EnvmapTextureHandle = 0;

	// Getting centred square area of the screen
	int side = MIN(GameVidWidth, GameVidHeight);
	int x = (GameVidWidth - side) / 2;
	int y = (GameVidHeight - side) / 2;
	RECT srcRect = {
		.left	= GameVidRect.left + x,
		.top	= GameVidRect.top  + y,
		.right	= GameVidRect.left + x + side,
		.bottom	= GameVidRect.top  + y + side,
	};

#if (DIRECT3D_VERSION >= 0x900)
	if( !CreateEnvmapTexture() ) return false;
	LPDDS texSurface = NULL;

	if SUCCEEDED(EnvmapTexture->GetSurfaceLevel(0, &texSurface)) {
		if SUCCEEDED(D3DDev->StretchRect(surface, &srcRect, texSurface, NULL, D3DTEXF_LINEAR)) {
			EnvmapTextureHandle = EnvmapTexture;
		}
		texSurface->Release();
	}
#else // (DIRECT3D_VERSION >= 0x900)
	if( !CreateEnvmapBufferSurface() ) return false;
	if( EnvmapTexture ) FreeEnvmapTexture();
	EnvmapBufferSurface->Blt(NULL, surface, &srcRect, DDBLT_WAIT, NULL);
	EnvmapTexture = Create3DTexture(EnvmapBufferSurface);
	if( EnvmapTexture ) {
		EnvmapTexture->GetHandle(D3DDev, &EnvmapTextureHandle);
	}
#endif // (DIRECT3D_VERSION >= 0x900)

	return ( EnvmapTextureHandle != 0 );
}

HWR_TEXHANDLE GetEnvmapTextureHandle() {
#if (DIRECT3D_VERSION < 0x900)
	if( EnvmapTextureHandle ) return EnvmapTextureHandle;
	SetEnvmapTexture(CaptureBufferSurface ? CaptureBufferSurface : PrimaryBufferSurface);
#endif // (DIRECT3D_VERSION < 0x900)
	return EnvmapTextureHandle;
}
#endif // FEATURE_VIDEOFX_IMPROVED

DWORD GetMaxTextureSize() {
#if (DIRECT3D_VERSION >= 0x900)
	return MIN(CurrentDisplayAdapter.caps.MaxTextureWidth, CurrentDisplayAdapter.caps.MaxTextureHeight);
#else // (DIRECT3D_VERSION >= 0x900)
	return MIN(CurrentDisplayAdapter.D3DHWDeviceDesc.dwMaxTextureWidth, CurrentDisplayAdapter.D3DHWDeviceDesc.dwMaxTextureHeight);
#endif // (DIRECT3D_VERSION >= 0x900)
}

void __cdecl CopyBitmapPalette(RGB888 *srcPal, BYTE *srcBitmap, int bitmapSize, RGB888 *destPal) {
	int i, j;
#if (DIRECT3D_VERSION < 0x900)
	HDC hdc;
	PALETTEENTRY firstSysPalEntries[10];
	PALETTEENTRY lastSysPalEntries[10];
#endif // (DIRECT3D_VERSION < 0x900)

	for( i=0; i<256; ++i ) {
		SortBuffer[i]._0 = i;
		SortBuffer[i]._1 = 0;
	}

	for( i=0; i<bitmapSize; ++i ) {
		SortBuffer[srcBitmap[i]]._1++;
	}

	do_quickysorty(0, 255);

#if (DIRECT3D_VERSION >= 0x900)
	// middle palette entries
	for( j=0; j<256; ++j ) {
		destPal[j] = srcPal[SortBuffer[j]._0];
	}
#else // (DIRECT3D_VERSION >= 0x900)
	hdc = GetDC(NULL);
	GetSystemPaletteEntries(hdc, 0,   10, firstSysPalEntries);
	GetSystemPaletteEntries(hdc, 246, 10, lastSysPalEntries);
	ReleaseDC(NULL, hdc);

	// first palette entries
	for( i=0; i<8; ++i ) {
		destPal[i].red   = firstSysPalEntries[i].peRed;
		destPal[i].green = firstSysPalEntries[i].peGreen;
		destPal[i].blue  = firstSysPalEntries[i].peBlue;
	}
	memset(&destPal[8], 0, 2*sizeof(RGB888));

	// middle palette entries
	for( i=0, j=10; i<236; ++i, ++j ) {
		destPal[j] = srcPal[SortBuffer[i]._0];
	}

	// last palette entries
	memset(&destPal[246], 0, 1*sizeof(RGB888));
	for( i=1, j=247; i<10; ++i, ++j ) {
		destPal[j].red   = lastSysPalEntries[i].peRed;
		destPal[j].green = lastSysPalEntries[i].peGreen;
		destPal[j].blue  = lastSysPalEntries[i].peBlue;
	}
#endif // (DIRECT3D_VERSION >= 0x900)
}

BYTE __cdecl FindNearestPaletteEntry(RGB888 *palette, int red, int green, int blue, bool ignoreSysPalette) {
	int i;
	int diffRed, diffGreen, diffBlue, diffTotal;
	int diffMin = INT_MAX;
	int palStartIdx = 0;
	int palEndIdx = 256;
	BYTE result = 0;

#if (DIRECT3D_VERSION < 0x900)
	if( ignoreSysPalette ) {
		palStartIdx += 10;
		palEndIdx -= 10;
	}
#endif // (DIRECT3D_VERSION < 0x900)

	for( i=palStartIdx; i<palEndIdx; ++i ) {
		diffRed   = red   - palette[i].red;
		diffGreen = green - palette[i].green;
		diffBlue  = blue  - palette[i].blue;
		diffTotal = diffRed*diffRed + diffGreen*diffGreen + diffBlue*diffBlue;
		if( diffTotal < diffMin ) {
			diffMin = diffTotal;
			result = i;
		}
	}
	return result;
}

void __cdecl SyncSurfacePalettes(void *srcData, int width, int height, int srcPitch, RGB888 *srcPalette, void *dstData, int dstPitch, RGB888 *dstPalette, bool preserveSysPalette) {
	int i, j;
	BYTE *src, *dst;
	BYTE bufPalette[256];

	for( i=0; i<256; ++i ) {
		bufPalette[i] = FindNearestPaletteEntry(dstPalette, srcPalette[i].red, srcPalette[i].green, srcPalette[i].blue, preserveSysPalette);
	}

	src = (BYTE *)srcData;
	dst = (BYTE *)dstData;

	for( i=0; i<height; ++i ) {
		for( j=0; j<width; ++j ) {
			*(dst++) = bufPalette[*(src++)];
		}
		src += srcPitch - width;
		dst += dstPitch - width;
	}
}

int __cdecl CreateTexturePalette(RGB888 *pal) {
#if (DIRECT3D_VERSION >= 0x900)
	int palIndex = GetFreePaletteIndex();
	if( palIndex < 0 )
		return -1;

	TexturePalettes[palIndex] = (RGB888 *)malloc(sizeof(RGB888) * 256);
	if( TexturePalettes[palIndex] == NULL )
		return -1;

	memcpy(TexturePalettes[palIndex], pal, sizeof(RGB888) * 256);
	return palIndex;
#else // (DIRECT3D_VERSION >= 0x900)
	int palIndex;
	PALETTEENTRY palEntries[256];

	palIndex = GetFreePaletteIndex();
	if( palIndex < 0 )
		return -1;

	for( int i=0; i<256; ++i ) {
		palEntries[i].peRed   = pal[i].red;
		palEntries[i].peGreen = pal[i].green;
		palEntries[i].peBlue  = pal[i].blue;
		palEntries[i].peFlags = 0;
	}

	if FAILED(DDraw->CreatePalette(DDPCAPS_ALLOW256|DDPCAPS_8BIT, palEntries, &TexturePalettes[palIndex], NULL))
		return -1;

	return palIndex;
#endif // (DIRECT3D_VERSION >= 0x900)
}

int __cdecl GetFreePaletteIndex() {
	for( DWORD i=0; i<ARRAY_SIZE(TexturePalettes); ++i ) {
		if( TexturePalettes[i] == NULL )
			return i;
	}
	return -1;
}

void __cdecl FreePalette(int paletteIndex) {
	if( TexturePalettes[paletteIndex] != NULL ) {
#if (DIRECT3D_VERSION >= 0x900)
		free(TexturePalettes[paletteIndex]);
#else // (DIRECT3D_VERSION >= 0x900)
		TexturePalettes[paletteIndex]->Release();
#endif // (DIRECT3D_VERSION >= 0x900)
		TexturePalettes[paletteIndex] = NULL;
	}
}

void __cdecl SafeFreePalette(int paletteIndex) {
	if( paletteIndex >= 0 ) {
		FreePalette(paletteIndex);
	}
}

#if (DIRECT3D_VERSION >= 0x900)
int __cdecl CreateTexturePage(int width, int height)
#else // (DIRECT3D_VERSION >= 0x900)
int __cdecl CreateTexturePage(int width, int height, LPDIRECTDRAWPALETTE palette)
#endif // (DIRECT3D_VERSION >= 0x900)
{
	int pageIndex = GetFreeTexturePageIndex();
	if( pageIndex < 0 )
		return -1;

	memset(&TexturePages[pageIndex], 0, sizeof(TEXPAGE_DESC));
	TexturePages[pageIndex].status = 1;
	TexturePages[pageIndex].width = width;
	TexturePages[pageIndex].height = height;
#if (DIRECT3D_VERSION >= 0x900)
	if FAILED(D3DDev->CreateTexture(width, height, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &TexturePages[pageIndex].texture, 0))
		return -1;
#else // (DIRECT3D_VERSION >= 0x900)
	TexturePages[pageIndex].palette = palette;
	if( !CreateTexturePageSurface(&TexturePages[pageIndex]) )
		return -1;

	TexturePageInit(&TexturePages[pageIndex]);
#endif // (DIRECT3D_VERSION >= 0x900)
	return pageIndex;
}

#if (DIRECT3D_VERSION < 0x900)
bool __cdecl CreateTexturePageSurface(TEXPAGE_DESC *desc) {
	DDSDESC dsp;

	memset(&dsp, 0, sizeof(dsp));
	dsp.dwSize = sizeof(dsp);
	dsp.dwFlags = DDSD_PIXELFORMAT|DDSD_WIDTH|DDSD_HEIGHT|DDSD_CAPS;
	dsp.dwWidth  = desc->width;
	dsp.dwHeight = desc->height;
	dsp.ddpfPixelFormat = TextureFormat.pixelFmt;
	dsp.ddsCaps.dwCaps = DDSCAPS_TEXTURE|DDSCAPS_SYSTEMMEMORY;
	if FAILED(DDrawSurfaceCreate(&dsp, &desc->sysMemSurface))
		return false;

	return ( (desc->palette == NULL) || SUCCEEDED(desc->sysMemSurface->SetPalette(desc->palette)) );
}
#endif // (DIRECT3D_VERSION < 0x900)

int __cdecl GetFreeTexturePageIndex() {
	for( DWORD i=0; i<ARRAY_SIZE(TexturePages); ++i ) {
		if( (TexturePages[i].status & 1) == 0 )
			return i;
	}
	return -1;
}

#if (DIRECT3D_VERSION < 0x900)
bool __cdecl TexturePageInit(TEXPAGE_DESC *page) {
	DDSDESC dsp;
	DDCOLORKEY colorKey;

	memset(&dsp, 0, sizeof(dsp));
	dsp.dwSize = sizeof(dsp);
	dsp.dwFlags = DDSD_PIXELFORMAT|DDSD_WIDTH|DDSD_HEIGHT|DDSD_CAPS;
	dsp.dwWidth = page->width;;
	dsp.dwHeight = page->height;
	dsp.ddpfPixelFormat = TextureFormat.pixelFmt;
	dsp.ddsCaps.dwCaps = DDSCAPS_ALLOCONLOAD|DDSCAPS_VIDEOMEMORY|DDSCAPS_TEXTURE;

	if( FAILED(DDrawSurfaceCreate(&dsp, &page->vidMemSurface)) || page->vidMemSurface == NULL ) {
		return false;
	}

	if( page->palette ) {
		colorKey.dwColorSpaceLowValue = 0;
		colorKey.dwColorSpaceHighValue = 0;
		if( FAILED(page->vidMemSurface->SetPalette(page->palette)) ||
			FAILED(page->vidMemSurface->SetColorKey(DDCKEY_SRCBLT, &colorKey)) )
		{
			page->vidMemSurface->Release();
			page->vidMemSurface = NULL;
			return false;
		}
	}

	page->texture3d = Create3DTexture(page->vidMemSurface);
	if( page->texture3d == NULL ) {
		page->vidMemSurface->Release();
		page->vidMemSurface = NULL;
		return false;
	}

	if FAILED(page->texture3d->GetHandle(D3DDev, &page->texHandle)) {
		page->texture3d->Release();
		page->texture3d = NULL;

		page->vidMemSurface->Release();
		page->vidMemSurface = NULL;

		page->texHandle = 0;
		return false;
	}

	return true;
}

LPDIRECT3DTEXTURE2 __cdecl Create3DTexture(LPDDS surface) {
	LPDIRECT3DTEXTURE2 texture3d = NULL;
	if FAILED(surface->QueryInterface(IID_IDirect3DTexture2, (LPVOID *)&texture3d)) {
		return NULL;
	}
	return texture3d;
}
#endif // (DIRECT3D_VERSION < 0x900)

void __cdecl SafeFreeTexturePage(int pageIndex) {
	if( pageIndex >= 0 && (TexturePages[pageIndex].status & 1) != 0 ) {
		FreeTexturePage(pageIndex);
	}
}

void __cdecl FreeTexturePage(int pageIndex) {
#if (DIRECT3D_VERSION >= 0x900)
	if( TexturePages[pageIndex].texture != NULL ) {
		TexturePages[pageIndex].texture->Release();
		TexturePages[pageIndex].texture = NULL;
	}
#else // (DIRECT3D_VERSION >= 0x900)
	TexturePageReleaseVidMemSurface(&TexturePages[pageIndex]);

	if( TexturePages[pageIndex].sysMemSurface != NULL ) {
		TexturePages[pageIndex].sysMemSurface->Release();
		TexturePages[pageIndex].sysMemSurface = NULL;
	}
#endif // (DIRECT3D_VERSION >= 0x900)
	TexturePages[pageIndex].status = 0;
}

#if (DIRECT3D_VERSION < 0x900)
void __cdecl TexturePageReleaseVidMemSurface(TEXPAGE_DESC *page) {
	HWR_ResetTexSource();
	page->texHandle = 0;
	if( page->texture3d ) {
		page->texture3d->Release();
		page->texture3d = NULL;
	}
	if( page->vidMemSurface ) {
		page->vidMemSurface->Release();
		page->vidMemSurface = NULL;
	}
}

bool __cdecl ReloadTextures(bool reset) {
	bool result = true;

	for( DWORD i=0; i<ARRAY_SIZE(TexturePages); ++i ) {
		if( (TexturePages[i].status & 1) != 0 )
			result &= LoadTexturePage(i, reset);
	}
	return result;
}
#endif // (DIRECT3D_VERSION < 0x900)

void __cdecl FreeTexturePages() {
#ifdef FEATURE_VIDEOFX_IMPROVED
	FreeEnvmapTexture();
#endif // FEATURE_VIDEOFX_IMPROVED
#ifdef FEATURE_BACKGROUND_IMPROVED
	BGND2_CleanupCaptureTextures();
#endif // FEATURE_BACKGROUND_IMPROVED
	for( DWORD i=0; i<ARRAY_SIZE(TexturePages); ++i ) {
		if( (TexturePages[i].status & 1) != 0 )
			FreeTexturePage(i);
	}
}

#if (DIRECT3D_VERSION < 0x900)
bool __cdecl LoadTexturePage(int pageIndex, bool reset) {
	bool rc = false;

	if( pageIndex < 0 )
		return false;

	if( reset || TexturePages[pageIndex].vidMemSurface == NULL ) {
		rc = SUCCEEDED(DDrawSurfaceRestoreLost(TexturePages[pageIndex].vidMemSurface, NULL, false));
	}

	if( !rc ) {
		TexturePageReleaseVidMemSurface(&TexturePages[pageIndex]);
		rc = TexturePageInit(&TexturePages[pageIndex]);
	}

	if( !rc )
		return false;

	DDrawSurfaceRestoreLost(TexturePages[pageIndex].sysMemSurface, NULL, false);
	LPDIRECT3DTEXTURE2 sysMemTexture = Create3DTexture(TexturePages[pageIndex].sysMemSurface);

	if( sysMemTexture == NULL )
		return false;

	rc = SUCCEEDED(TexturePages[pageIndex].texture3d->Load(sysMemTexture));
	sysMemTexture->Release();

	return rc;
}
#endif // (DIRECT3D_VERSION < 0x900)


HWR_TEXHANDLE __cdecl GetTexturePageHandle(int pageIndex) {
	if( pageIndex < 0 )
		return 0;

#if (DIRECT3D_VERSION >= 0x900)
	return TexturePages[pageIndex].texture;
#else // (DIRECT3D_VERSION >= 0x900)
	if( TexturePages[pageIndex].vidMemSurface &&
		TexturePages[pageIndex].vidMemSurface->IsLost() == DDERR_SURFACELOST )
	{
		LoadTexturePage(pageIndex, 1);
	}

	return TexturePages[pageIndex].texHandle;
#endif // (DIRECT3D_VERSION >= 0x900)
}

int __cdecl AddTexturePage8(int width, int height, BYTE *pageBuffer, int palIndex) {
#if (DIRECT3D_VERSION >= 0x900)
	int pageIndex = CreateTexturePage(width, height);
	if( pageIndex < 0 )
		return -1;

	DDSDESC desc;
	if FAILED(TexturePages[pageIndex].texture->LockRect(0, &desc, NULL, 0)) {
		return -1;
	}
	BYTE *src = pageBuffer;
	for( int i=0; i<height; ++i ) {
		DWORD *dst = (DWORD *)((BYTE *)desc.pBits + desc.Pitch * i);
		for( int j=0; j<width; ++j ) {
			if( *src ) {
				BYTE r = TexturePalettes[palIndex][*src].red;
				BYTE g = TexturePalettes[palIndex][*src].green;
				BYTE b = TexturePalettes[palIndex][*src].blue;
				*dst++ = RGBA_MAKE(r, g, b, 0xFF);
			} else {
				*dst++ = 0;
			}
			++src;
		}
	}
	TexturePages[pageIndex].texture->UnlockRect(0);

	return pageIndex;
#else // (DIRECT3D_VERSION >= 0x900)
	int pageIndex;
	BYTE *src, *dst;
	DDSDESC desc;

	if( palIndex < 0 )
		return -1;

	pageIndex = CreateTexturePage(width, height, TexturePalettes[palIndex]);
	if( pageIndex < 0 )
		return -1;

	if FAILED(WinVidBufferLock(TexturePages[pageIndex].sysMemSurface, &desc, DDLOCK_WRITEONLY|DDLOCK_WAIT))
		return -1;

	src = pageBuffer;
	dst = (BYTE *)desc.lpSurface;
	for( int i=0; i<height; ++i ) {
		memcpy(dst, src, width);
		src += width;
		dst += desc.lPitch;
	}
	WinVidBufferUnlock(TexturePages[pageIndex].sysMemSurface, &desc);
	LoadTexturePage(pageIndex, false);

	return pageIndex;
#endif // (DIRECT3D_VERSION >= 0x900)
}

int __cdecl AddTexturePage16(int width, int height, BYTE *pageBuffer) {
#if (DIRECT3D_VERSION >= 0x900)
	int pageIndex = CreateTexturePage(width, height);
	if( pageIndex < 0 )
		return -1;

	DDSDESC desc;
	if FAILED(TexturePages[pageIndex].texture->LockRect(0, &desc, NULL, 0)) {
		return -1;
	}
	UINT16 *src = (UINT16 *)pageBuffer;
	for( int i=0; i<height; ++i ) {
		DWORD *dst = (DWORD *)((BYTE *)desc.pBits + desc.Pitch * i);
		for( int j=0; j<width; ++j ) {
			BYTE r = ((*src >> 7) & 0xF8) | ((*src >> 13) & 0x07);
			BYTE g = ((*src >> 2) & 0xF8) | ((*src >>  7) & 0x07);
			BYTE b = ((*src << 3) & 0xF8) | ((*src >>  2) & 0x07);
			BYTE a = (*src & 0x8000) ? 0xFF : 0;
			*dst++ = RGBA_MAKE(r, g, b, a);
			++src;
		}
	}
	TexturePages[pageIndex].texture->UnlockRect(0);

	return pageIndex;
#else // (DIRECT3D_VERSION >= 0x900)
	int i, j, k;
	int pageIndex, bytesPerPixel;
	BYTE *src, *dst, *subdst;
	BYTE srcRed, srcGreen, srcBlue, srcAlpha;
	DWORD compatibleColor;
	DDSDESC desc;

	pageIndex = CreateTexturePage(width, height, NULL);
	if( pageIndex < 0 )
		return -1;

	if FAILED(WinVidBufferLock(TexturePages[pageIndex].sysMemSurface, &desc, DDLOCK_WRITEONLY|DDLOCK_WAIT))
		return -1;

	if( TexturesHaveCompatibleMasks ) {
		src = (BYTE *)pageBuffer;
		dst = (BYTE *)desc.lpSurface;
		for( i=0; i<height; ++i ) {
			memcpy(dst, src, width*2);
			src += width*2;
			dst += desc.lPitch;
		}
	} else {
		src = pageBuffer;
		dst = (BYTE *)desc.lpSurface;
		bytesPerPixel = (TextureFormat.bpp + 7) / 8;
		for( i=0; i<height; ++i ) {
			subdst = dst;
			for( j=0; j<width; ++j ) {
				srcRed   = (*(UINT16 *)src >>  7) & 0xF8;
				srcGreen = (*(UINT16 *)src >>  2) & 0xF8;
				srcBlue  = (*(UINT16 *)src <<  3) & 0xF8;
				srcAlpha = (*(UINT16 *)src >> 15) & 1;
				compatibleColor = CalculateCompatibleColor(&TextureFormat.colorBitMasks, srcRed, srcGreen, srcBlue, srcAlpha);
				for( k=0; k<bytesPerPixel; ++k ) {
					*(subdst++) = compatibleColor & 0xFF;
					compatibleColor >>= 8;
				}
				src += 2;
			}
			dst += desc.lPitch;
		}
	}

	WinVidBufferUnlock(TexturePages[pageIndex].sysMemSurface, &desc);
	LoadTexturePage(pageIndex, false);

	return pageIndex;
#endif // (DIRECT3D_VERSION >= 0x900)
}

#if (DIRECT3D_VERSION < 0x900)
HRESULT CALLBACK EnumTextureFormatsCallback(LPDDSDESC lpDdsd, LPVOID lpContext) {
	LPDDPIXELFORMAT lpDDPixFmt = &lpDdsd->ddpfPixelFormat;

	if( lpDDPixFmt->dwRGBBitCount < 8 )
		return D3DENUMRET_OK;

	if( SavedAppSettings.Disable16BitTextures || lpDDPixFmt->dwRGBBitCount < 16 ) {
		if( CHK_ANY(lpDDPixFmt->dwFlags, DDPF_PALETTEINDEXED8) ) {
			TextureFormat.pixelFmt = *lpDDPixFmt;
			TextureFormat.bpp = 8;
			TexturesAlphaChannel = false;
			if( SavedAppSettings.Disable16BitTextures ) {
				TexturesHaveCompatibleMasks = false;
				return D3DENUMRET_CANCEL; // NOTE: not presented in the original code
			}
		}
	} else if( CHK_ANY(lpDDPixFmt->dwFlags, DDPF_RGB) ) {
		TextureFormat.pixelFmt = *lpDDPixFmt;
		TextureFormat.bpp = 16;
		TexturesAlphaChannel = CHK_ANY(lpDDPixFmt->dwFlags, DDPF_ALPHAPIXELS);
		WinVidGetColorBitMasks(&TextureFormat.colorBitMasks, lpDDPixFmt);
		if( TextureFormat.bpp == 16 &&
			TextureFormat.colorBitMasks.dwRGBAlphaBitDepth	== 1  &&
			TextureFormat.colorBitMasks.dwRBitDepth			== 5  &&
			TextureFormat.colorBitMasks.dwGBitDepth			== 5  &&
			TextureFormat.colorBitMasks.dwBBitDepth			== 5  &&
			TextureFormat.colorBitMasks.dwRGBAlphaBitOffset	== 15 &&
			TextureFormat.colorBitMasks.dwRBitOffset		== 10 &&
			TextureFormat.colorBitMasks.dwGBitOffset		== 5  &&
			TextureFormat.colorBitMasks.dwBBitOffset		== 0 )
		{
			TexturesHaveCompatibleMasks = true;
			return D3DENUMRET_CANCEL;
		}
	}

	TexturesHaveCompatibleMasks = false;
	return D3DENUMRET_OK;
}

HRESULT __cdecl EnumerateTextureFormats() {
	memset(&TextureFormat, 0, sizeof(TEXTURE_FORMAT));
	HRESULT ret = D3DDev->EnumTextureFormats(EnumTextureFormatsCallback, NULL);
	// NOTE: there is no such check in the original code
	if( SavedAppSettings.Disable16BitTextures && TextureFormat.bpp < 8 ) {
		SavedAppSettings.Disable16BitTextures = false;
		ret = D3DDev->EnumTextureFormats(EnumTextureFormatsCallback, NULL);
	}
	return ret;
}
#endif // (DIRECT3D_VERSION < 0x900)

void __cdecl CleanupTextures() {
	FreeTexturePages();
	for( DWORD i=0; i<ARRAY_SIZE(TexturePalettes); ++i ) {
		if( TexturePalettes[i] != NULL )
			FreePalette(i);
	}
}

bool __cdecl InitTextures() {
	memset(TexturePages,  0, sizeof(TexturePages));
	memset(TexturePalettes, 0, sizeof(TexturePalettes));
	return true;
}

/*
 * Inject function
 */
void Inject_Texture() {
	INJECT(0x00455990, CopyBitmapPalette);
	INJECT(0x00455AD0, FindNearestPaletteEntry);
	INJECT(0x00455BA0, SyncSurfacePalettes);
	INJECT(0x00455C50, CreateTexturePalette);
	INJECT(0x00455CE0, GetFreePaletteIndex);
	INJECT(0x00455D00, FreePalette);
	INJECT(0x00455D30, SafeFreePalette);
	INJECT(0x00455EB0, CreateTexturePage);
	INJECT(0x00455DF0, GetFreeTexturePageIndex);
#if (DIRECT3D_VERSION < 0x900)
	INJECT(0x00455E10, CreateTexturePageSurface);
	INJECT(0x00455EB0, TexturePageInit);
	INJECT(0x00456030, Create3DTexture);
#endif // (DIRECT3D_VERSION < 0x900)
	INJECT(0x00456060, SafeFreeTexturePage);
	INJECT(0x00456080, FreeTexturePage);
#if (DIRECT3D_VERSION < 0x900)
	INJECT(0x004560C0, TexturePageReleaseVidMemSurface);
#endif // (DIRECT3D_VERSION < 0x900)
	INJECT(0x00456100, FreeTexturePages);
#if (DIRECT3D_VERSION < 0x900)
	INJECT(0x004561E0, ReloadTextures);
	INJECT(0x00456130, LoadTexturePage);
#endif // (DIRECT3D_VERSION < 0x900)
	INJECT(0x00456220, GetTexturePageHandle);
	INJECT(0x00456260, AddTexturePage8);
	INJECT(0x00456360, AddTexturePage16);
#if (DIRECT3D_VERSION < 0x900)
	INJECT(0x00456500, EnumTextureFormatsCallback);
	INJECT(0x00456620, EnumerateTextureFormats);
#endif // (DIRECT3D_VERSION < 0x900)
	INJECT(0x00456650, CleanupTextures);
	INJECT(0x00456660, InitTextures);
}
