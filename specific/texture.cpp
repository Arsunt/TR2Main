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
#include "specific/texture.h"
#include "3dsystem/3d_gen.h"
#include "specific/hwr.h"
#include "specific/winvid.h"
#include "global/vars.h"
#include <limits.h>

void __cdecl CopyBitmapPalette(RGB888 *srcPal, BYTE *srcBitmap, int bitmapSize, RGB888 *destPal) {
	int i, j;
	HDC hdc;
	PALETTEENTRY firstSysPalEntries[10];
	PALETTEENTRY lastSysPalEntries[10];

	for( i=0; i<256; ++i ) {
		SortBuffer[i]._0 = i;
		SortBuffer[i]._1 = 0;
	}

	for( i=0; i<bitmapSize; ++i ) {
		SortBuffer[srcBitmap[i]]._1++;
	}

	do_quickysorty(0, 255);

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
}

BYTE __cdecl FindNearestPaletteEntry(RGB888 *palette, int red, int green, int blue, bool ignoreSysPalette) {
	int i;
	int diffRed, diffGreen, diffBlue, diffTotal;
	int diffMin = INT_MAX;
	int palStartIdx = 0;
	int palEndIdx = 256;
	int palSize = 256;
	BYTE result = 0;

	if( ignoreSysPalette ) {
		palStartIdx += 10;
		palEndIdx -= 10;
		palSize -= 20;
	}

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
	int palIndex;
	PALETTEENTRY palEntries[256];

	palIndex = GetFreePaletteIndex();
	if( palIndex < 0 )
		return -1;

	for( int i=0; i<256; ++i ) {
		palEntries[i].peRed	 = pal[i].red;
		palEntries[i].peGreen = pal[i].green;
		palEntries[i].peBlue	= pal[i].blue;
		palEntries[i].peFlags = 0;
	}

	if FAILED(_DirectDraw2->CreatePalette(DDPCAPS_ALLOW256|DDPCAPS_8BIT, palEntries, &DDrawPalettes[palIndex], NULL))
		return -1;

	return palIndex;
}

int __cdecl GetFreePaletteIndex() {
	for( int i=0; i<16; ++i ) {
		if( DDrawPalettes[i] == NULL )
			return i;
	}
	return -1;
}

void __cdecl FreePalette(int paletteIndex) {
	if( DDrawPalettes[paletteIndex] != NULL ) {
		DDrawPalettes[paletteIndex]->Release();
		DDrawPalettes[paletteIndex] = NULL;
	}
}

void __cdecl SafeFreePalette(int paletteIndex) {
	if( paletteIndex >= 0 ) {
		FreePalette(paletteIndex);
	}
}

int __cdecl CreateTexturePage(int width, int height, LPDIRECTDRAWPALETTE palette) {

	int pageIndex = GetFreeTexturePageIndex();
	if( pageIndex < 0 )
		return -1;

	memset(&TexturePages[pageIndex], 0, sizeof(TEXPAGE_DESC));
	TexturePages[pageIndex].status = 1;
	TexturePages[pageIndex].width = width;
	TexturePages[pageIndex].height = height;
	TexturePages[pageIndex].palette = palette;
	if( !CreateTexturePageSurface(&TexturePages[pageIndex]) )
		return -1;

	TexturePageInit(&TexturePages[pageIndex]);
	return pageIndex;
}

bool __cdecl CreateTexturePageSurface(TEXPAGE_DESC *desc) {
	DDSURFACEDESC dsp;

	memset(&dsp, 0, sizeof(DDSURFACEDESC));
	dsp.dwSize = sizeof(DDSURFACEDESC);
	dsp.dwFlags = DDSD_PIXELFORMAT|DDSD_WIDTH|DDSD_HEIGHT|DDSD_CAPS;
	dsp.dwWidth  = desc->width;
	dsp.dwHeight = desc->height;
	dsp.ddpfPixelFormat = TextureFormat.pixelFmt;
	dsp.ddsCaps.dwCaps = DDSCAPS_TEXTURE|DDSCAPS_SYSTEMMEMORY;
	if FAILED(DDrawSurfaceCreate(&dsp, &desc->sysMemSurface))
		return false;

	return ( (desc->palette == NULL) || SUCCEEDED(desc->sysMemSurface->SetPalette(desc->palette)) );
}

int __cdecl GetFreeTexturePageIndex() {
	for( int i=0; i<32; ++i ) {
		if( (TexturePages[i].status & 1) == 0 )
			return i;
	}
	return -1;
}

bool __cdecl TexturePageInit(TEXPAGE_DESC *page) {
	DDSURFACEDESC dsp;
	DDCOLORKEY colorKey;

	memset(&dsp, 0, sizeof(DDSURFACEDESC));
	dsp.dwSize = sizeof(DDSURFACEDESC);
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

	if FAILED(page->texture3d->GetHandle(_Direct3DDevice2, &page->texHandle)) {
		page->texture3d->Release();
		page->texture3d = NULL;

		page->vidMemSurface->Release();
		page->vidMemSurface = NULL;

		page->texHandle = 0;
		return false;
	}

	return true;
}

LPDIRECT3DTEXTURE2 __cdecl Create3DTexture(LPDIRECTDRAWSURFACE3 surface) {
	LPDIRECT3DTEXTURE2 texture3d = NULL;
	if FAILED(surface->QueryInterface(IID_IDirect3DTexture2, (LPVOID *)&texture3d)) {
		return NULL;
	}
	return texture3d;
}

void __cdecl SafeFreeTexturePage(int pageIndex) {
	if( pageIndex >= 0 && (TexturePages[pageIndex].status & 1) != 0 ) {
		FreeTexturePage(pageIndex);
	}
}

void __cdecl FreeTexturePage(int pageIndex) {
	TexturePageReleaseVidMemSurface(&TexturePages[pageIndex]);

	if( TexturePages[pageIndex].sysMemSurface != NULL ) {
		TexturePages[pageIndex].sysMemSurface->Release();
		TexturePages[pageIndex].sysMemSurface = NULL;
	}
	TexturePages[pageIndex].status = 0;
}

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

	for( int i=0; i<32; ++i ) {
		if( (TexturePages[i].status & 1) != 0 )
			result &= LoadTexturePage(i, reset);
	}
	return result;
}

void __cdecl FreeTexturePages() {
	for( int i=0; i<32; ++i ) {
		if( (TexturePages[i].status & 1) != 0 )
			FreeTexturePage(i);
	}
}

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


D3DTEXTUREHANDLE __cdecl GetTexturePageHandle(int pageIndex) {
	if( pageIndex < 0 )
		return 0;

	if( TexturePages[pageIndex].vidMemSurface &&
		TexturePages[pageIndex].vidMemSurface->IsLost() == DDERR_SURFACELOST )
	{
		LoadTexturePage(pageIndex, 1);
	}

	return TexturePages[pageIndex].texHandle;
}

int __cdecl AddTexturePage8(int width, int height, BYTE *pageBuffer, int palIndex) {
	int pageIndex;
	BYTE *src, *dst;
	DDSURFACEDESC desc;

	if( palIndex < 0 )
		return -1;

	pageIndex = CreateTexturePage(width, height, DDrawPalettes[palIndex]);
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
}

int __cdecl AddTexturePage16(int width, int height, BYTE *pageBuffer) {
	int i, j, k;
	int pageIndex, bytesPerPixel;
	BYTE *src, *dst, *subdst;
	BYTE srcRed, srcGreen, srcBlue, srcAlpha;
	DWORD compatibleColor;
	DDSURFACEDESC desc;

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
}

HRESULT CALLBACK EnumTextureFormatsCallback(LPDDSURFACEDESC lpDdsd, LPVOID lpContext) {
	DDPIXELFORMAT pixelFormat = lpDdsd->ddpfPixelFormat;

	if( pixelFormat.dwRGBBitCount < 8 )
		return D3DENUMRET_OK;

	if( SavedAppSettings.Disable16BitTextures || pixelFormat.dwRGBBitCount < 16 ) {
		if( pixelFormat.dwFlags & DDPF_PALETTEINDEXED8 ) {
			TextureFormat.pixelFmt = lpDdsd->ddpfPixelFormat;
			TextureFormat.bpp = 8;
			TexturesAlphaChannel = false;
		}
		TexturesHaveCompatibleMasks = false;
		return D3DENUMRET_OK;
	}

	if( pixelFormat.dwFlags & DDPF_RGB ) {
		TextureFormat.pixelFmt = lpDdsd->ddpfPixelFormat;
		TextureFormat.bpp = 16;
		TexturesAlphaChannel = ( (pixelFormat.dwFlags & DDPF_ALPHAPIXELS) != 0 );
		WinVidGetColorBitMasks(&TextureFormat.colorBitMasks, &pixelFormat);
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
	return _Direct3DDevice2->EnumTextureFormats(EnumTextureFormatsCallback, NULL);
}

void __cdecl CleanupTextures() {
	FreeTexturePages();
	for( int i=0; i<16; ++i ) {
		if( DDrawPalettes[i] != NULL )
			FreePalette(i);
	}
}

bool __cdecl InitTextures() {
	memset(TexturePages,  0, sizeof(TexturePages));
	memset(DDrawPalettes, 0, sizeof(DDrawPalettes));
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
	INJECT(0x00455E10, CreateTexturePageSurface);
	INJECT(0x00455EB0, TexturePageInit);
	INJECT(0x00456030, Create3DTexture);
	INJECT(0x00456060, SafeFreeTexturePage);
	INJECT(0x00456080, FreeTexturePage);
	INJECT(0x004560C0, TexturePageReleaseVidMemSurface);
	INJECT(0x00456100, FreeTexturePages);
	INJECT(0x004561E0, ReloadTextures);
	INJECT(0x00456130, LoadTexturePage);
	INJECT(0x00456220, GetTexturePageHandle);
	INJECT(0x00456260, AddTexturePage8);
	INJECT(0x00456360, AddTexturePage16);
	INJECT(0x00456500, EnumTextureFormatsCallback);
	INJECT(0x00456620, EnumerateTextureFormats);
	INJECT(0x00456650, CleanupTextures);
	INJECT(0x00456660, InitTextures);
}
