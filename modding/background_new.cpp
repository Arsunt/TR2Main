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
#include "modding/background_new.h"
#include "3dsystem/phd_math.h"
#include "specific/background.h"
#include "specific/file.h"
#include "specific/frontend.h"
#include "specific/hwr.h"
#include "specific/init_display.h"
#include "specific/input.h"
#include "specific/output.h"
#include "specific/texture.h"
#include "specific/utils.h"
#include "specific/winvid.h"
#include "modding/file_utils.h"
#include "modding/gdi_utils.h"
#include "global/vars.h"

#ifdef FEATURE_BACKGROUND_IMPROVED
extern LPDDS CaptureBufferSurface;

#ifdef FEATURE_INPUT_IMPROVED
#include "modding/joy_output.h"
#endif // FEATURE_INPUT_IMPROVED

#ifdef FEATURE_GOLD
extern bool IsGold();
#endif

int BGND_TexturePageIndexes[128];
HWR_TEXHANDLE BGND_PageHandles[128];

DWORD BGND_PictureWidth  = 640;
DWORD BGND_PictureHeight = 480;
bool BGND_IsCaptured = false;

DWORD PictureStretchLimit = 10;
bool RemasteredPixEnabled = true;

static DWORD BGND_TextureSide  = 1024;
static DWORD BGND_TextureAlpha = 255;

/// Short wave horizontal pattern step
#define SHORT_WAVE_X_STEP	(0x3000)
/// Short wave vertical pattern step
#define SHORT_WAVE_Y_STEP	(0x33E7)
/// Short wave horizontal pattern offset
#define SHORT_WAVE_X_OFFSET	(SHORT_WAVE_X_STEP * 2)
/// Short wave vertical pattern offset
#define SHORT_WAVE_Y_OFFSET	(SHORT_WAVE_Y_STEP * 1)
/// Long wave horizontal pattern step
#define LONG_WAVE_X_STEP	(0x1822)
/// Long wave vertical pattern step
#define LONG_WAVE_Y_STEP	(0x1422)
/// Long wave horizontal pattern offset
#define LONG_WAVE_X_OFFSET	(LONG_WAVE_X_STEP * 2)
/// Long wave vertical pattern offset
#define LONG_WAVE_Y_OFFSET	(LONG_WAVE_Y_STEP * 1)

/// Pixel accuracy factor (for more exact integer computations)
#define PIXEL_ACCURACY	(4)
/// Animated pattern detail level (Increases the smoothness of the curve)
#define PATTERN_DETAIL	(2)

/// 2D vertex (no Z coordinate) structure
typedef struct {
	D3DVALUE x; ///< Vertex X coordinate (pixels)
	D3DVALUE y; ///< Vertex Y coordinate (pixels)
	D3DCOLOR color; ///< Vertex color (ARGB)
} VERTEX2D;

/// Texture data structure
typedef struct {
	HWR_TEXHANDLE handle; ///< Handle of texture
	int u; ///< Texture U coordinate (pixels)
	int v; ///< Texture V coordinate (pixels)
	int width;	///< Texture width (pixels)
	int height;	///< Texture height (pixels)
} TEXTURE;

/**
 * Draws flat textured quad polygon (two triangles) at far Z coordinate
 * @param[in] vtx0,vtx1,vtx2,vtx3 Pointers to the Vertex structures
 * @param[in] txr Pointer to the Texture structure
 */
void RenderTexturedFarQuad(VERTEX2D *vtx0, VERTEX2D *vtx1, VERTEX2D *vtx2, VERTEX2D *vtx3, TEXTURE *txr) {
	D3DTLVERTEX vtx[4];
	double uvAdjust = (double)UvAdd / (double)(PHD_ONE);

	float tu0 = (double)txr->u / 256.0 + uvAdjust;
	float tv0 = (double)txr->v / 256.0 + uvAdjust;
	float tu1 = (double)(txr->u + txr->width)  / 256.0 - uvAdjust;
	float tv1 = (double)(txr->v + txr->height) / 256.0 - uvAdjust;

	float rhw = RhwFactor / FltFarZ;

	vtx[0].sx = vtx0->x;
	vtx[0].sy = vtx0->y;
	vtx[0].color = vtx0->color;
	vtx[0].tu = tu0;
	vtx[0].tv = tv0;

	vtx[1].sx = vtx1->x;
	vtx[1].sy = vtx1->y;
	vtx[1].color = vtx1->color;
	vtx[1].tu = tu1;
	vtx[1].tv = tv0;

	vtx[2].sx = vtx2->x;
	vtx[2].sy = vtx2->y;
	vtx[2].color = vtx2->color;
	vtx[2].tu = tu0;
	vtx[2].tv = tv1;

	vtx[3].sx = vtx3->x;
	vtx[3].sy = vtx3->y;
	vtx[3].color = vtx3->color;
	vtx[3].tu = tu1;
	vtx[3].tv = tv1;

	for( int i=0; i<4; ++i ) {
		vtx[i].sz = 0.995;
		vtx[i].rhw = rhw;
		vtx[i].specular = 0;
	}

	HWR_TexSource(txr->handle);
	HWR_EnableColorKey(false);
#if (DIRECT3D_VERSION >= 0x700)
	D3DDev->SetRenderState(D3DRENDERSTATE_CLIPPING, TRUE);
	D3DDev->SetRenderState(D3DRENDERSTATE_EXTENTS, TRUE);
#endif // (DIRECT3D_VERSION >= 0x700)
	D3DDev->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3D_TLVERTEX, &vtx, 4, 0);
#if (DIRECT3D_VERSION >= 0x700)
	D3DDev->SetRenderState(D3DRENDERSTATE_CLIPPING, FALSE);
	D3DDev->SetRenderState(D3DRENDERSTATE_EXTENTS, FALSE);
#endif // (DIRECT3D_VERSION >= 0x700)
}

/**
 * Draws animated pattern wallpaper (TR2 PlayStation inventory style)
 * @param[in] texSource Handle of texture
 * @param[in] tu Texture U coordinate (pixels)
 * @param[in] tv Texture V coordinate (pixels)
 * @param[in] t_width Texture width (pixels)
 * @param[in] t_height Texture height (pixels)
 * @param[in] halfRowCount Half number of vertical rows of the wallpaper pattern
 * @param[in] amplitude Percent value of the deformation amplitude (vertex rotation radius)
 * @param[in] deformWavePhase Deformation wave phase in Integer representation
 * @param[in] shortWavePhase Lighting short wave phase in Integer representation
 * @param[in] longWavePhase Lighting long wave phase in Integer representation
 */
void PSX_Background(HWR_TEXHANDLE texSource, int tu, int tv, int t_width, int t_height, int halfRowCount,
					__int16 amplitude, __int16 deformWavePhase, __int16 shortWavePhase, __int16 longWavePhase)
{
	int halfColCount = MulDiv(halfRowCount, PhdWinWidth*3, PhdWinHeight*4) + 1;

	halfRowCount *= PATTERN_DETAIL;
	halfColCount *= PATTERN_DETAIL;

	int light;
	int countY = halfRowCount*2+1;
	int countX = halfColCount*2+1;
	int tileSize = MulDiv(PhdWinHeight, PIXEL_ACCURACY*2, halfRowCount*3);
	int tileRadius = MulDiv(tileSize, amplitude*PATTERN_DETAIL, 100);
	int baseY = PhdWinHeight * PIXEL_ACCURACY/2 - halfRowCount*tileSize;
	int baseX = PhdWinWidth  * PIXEL_ACCURACY/2  - halfColCount*tileSize;
	VERTEX2D *vertices = (VERTEX2D *)malloc(sizeof(VERTEX2D)*countX*countY);
	TEXTURE subTxr;

	deformWavePhase += SHORT_WAVE_X_OFFSET;
	shortWavePhase  += SHORT_WAVE_X_OFFSET;
	longWavePhase   += LONG_WAVE_X_OFFSET;

	for( int i=0; i<countX; ++i ) {
		short deformWaveRowPhase = deformWavePhase + SHORT_WAVE_Y_OFFSET;
		short shortWaveRowPhase  = shortWavePhase  + SHORT_WAVE_Y_OFFSET;
		short longWaveRowPhase   = longWavePhase   + LONG_WAVE_Y_OFFSET;

		for( int j=0; j<countY; ++j ) {
			VERTEX2D *vtx = &vertices[i*countY+j];
			int shortWave = phd_sin(shortWaveRowPhase)*32/0x4000;
			int longWave = phd_sin(longWaveRowPhase)*32/0x4000;
			light = 128 + shortWave + longWave;
			vtx->color = RGBA_MAKE(light, light, light, 0xFFu);
			vtx->y = ((float)(baseY + tileSize*j + phd_sin(deformWaveRowPhase)*tileRadius/0x4000)) / PIXEL_ACCURACY;
			vtx->x = ((float)(baseX + tileSize*i + phd_cos(deformWaveRowPhase)*tileRadius/0x4000)) / PIXEL_ACCURACY;

			deformWaveRowPhase += SHORT_WAVE_Y_STEP / PATTERN_DETAIL;
			shortWaveRowPhase  += SHORT_WAVE_Y_STEP / PATTERN_DETAIL;
			longWaveRowPhase   += LONG_WAVE_Y_STEP  / PATTERN_DETAIL;
		}
		deformWavePhase += SHORT_WAVE_X_STEP / PATTERN_DETAIL;
		shortWavePhase  += SHORT_WAVE_X_STEP / PATTERN_DETAIL;
		longWavePhase   += LONG_WAVE_X_STEP  / PATTERN_DETAIL;
	}

	subTxr.handle = texSource;
	subTxr.width  = t_width  / PATTERN_DETAIL;
	subTxr.height = t_height / PATTERN_DETAIL;

	for( int i=0; i<halfColCount*2; ++i ) {
		for( int j=0; j<halfRowCount*2; ++j ) {
			VERTEX2D *vtx0 = &vertices[(i+0)*countY+(j+0)];
			VERTEX2D *vtx1 = &vertices[(i+1)*countY+(j+0)];
			VERTEX2D *vtx2 = &vertices[(i+0)*countY+(j+1)];
			VERTEX2D *vtx3 = &vertices[(i+1)*countY+(j+1)];
			subTxr.u = tu + (i%PATTERN_DETAIL)*subTxr.width;
			subTxr.v = tv + (j%PATTERN_DETAIL)*subTxr.height;
			RenderTexturedFarQuad(vtx0, vtx1, vtx2, vtx3, &subTxr);
		}
	}
	free(vertices);
}

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


static int CreateCaptureTexture(DWORD index, DWORD side) {
	int pageIndex = BGND_TexturePageIndexes[index];
	if( pageIndex < 0 || !TexturePages[pageIndex].status ) {
		pageIndex = CreateTexturePage(side, side, NULL);
		if( pageIndex < 0 ) {
			return -1;
		}
		BGND_TexturePageIndexes[index] = pageIndex;
	}
	return pageIndex;
}

static int MakeBgndTexture(DWORD x, DWORD y, DWORD width, DWORD height, DWORD pitch, DWORD side, BYTE *bitmap, RGB888 *bmpPal) {
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
	} else if( BGND_PaletteIndex < 0 ) {
		UINT16 *tmpBmp = (UINT16 *)calloc(2, SQR(side));
		UINT16 *bmpDst = tmpBmp;
		BYTE *bmpSrc = bitmap + x + y * pitch;

		// Translating bitmap data from 8 bit bitmap to 16 bit bitmap
		for( DWORD j = 0; j < height; ++j ) {
			for( DWORD i = 0; i < width; ++i ) {
				RGB888 *color = &bmpPal[bmpSrc[i]]; // get RGB color from palette
				bmpDst[i] = (1 << 15) // convert RGB to 16 bit
						| (((UINT16)color->red   >> 3) << 10)
						| (((UINT16)color->green >> 3) << 5)
						| (((UINT16)color->blue  >> 3));
			}
			bmpSrc += pitch;
			bmpDst += side;
		}
		FillEdgePadding(width, height, side, (BYTE *)tmpBmp, 16);
		pageIndex = AddTexturePage16(side, side, (BYTE *)tmpBmp);
		free(tmpBmp);
	} else {
		BYTE *tmpBmp = (BYTE *)calloc(1, SQR(side));
		UT_MemBlt(tmpBmp, 0, 0, width, height, side, bitmap, x, y, pitch);
		FillEdgePadding(width, height, side, tmpBmp, 8);
		pageIndex = AddTexturePage8(side, side, tmpBmp, BGND_PaletteIndex);
		free(tmpBmp);
	}
	return pageIndex;
}

static int MakeBgndTextures(DWORD width, DWORD height, BYTE *bitmap, RGB888 *bmpPal) {
	DWORD side = GetMaxTextureSize();
	S_DontDisplayPicture(); // clean up previous textures

	if( bmpPal != NULL && (SavedAppSettings.RenderMode != RM_Hardware || TextureFormat.bpp < 16) ) {
		BGND_PaletteIndex = CreateTexturePalette(bmpPal);
	} else {
		BGND_PaletteIndex = -1;
	}

	DWORD nx = (width + side - 1) / side;
	DWORD ny = (height + side - 1) / side;

	if( nx*ny >= ARRAY_SIZE(BGND_TexturePageIndexes) ) {
		return -1; // It seems image is too big
	}

	for( DWORD j = 0; j < ny; ++j ) {
		for( DWORD i = 0; i < nx; ++i ) {
			DWORD w = side;
			DWORD h = side;
			if( i == nx - 1 && width % side ) w = width % side;
			if( j == ny - 1 && height % side ) h = height % side;
			int pageIndex = MakeBgndTexture(i*side, j*side, w, h, width, side, bitmap, bmpPal);
			if( pageIndex < 0) {
				return -1;
			}
			BGND_TexturePageIndexes[i + j*nx] = pageIndex;
		}
	}

	BGND_PictureWidth = width;
	BGND_PictureHeight = height;
	BGND_TextureSide = side;
	BGND_GetPageHandles();
	BGND_PictureIsReady = true;
	return 0;
}

static int PickBestPictureFile(LPTSTR fileName, LPCTSTR modDir) {
	static const STRING_FIXED4 exts[] = {"PNG", "JPG", "BMP"};
	static const BYTE numAspects = 7;
	static const BYTE aspects[numAspects][2] = {
		{5,4}, {4,3}, {3,2}, {16,10}, {16,9}, {21,9}, {32,9} // common display aspect ratios
	};
	if( fileName == NULL || !*fileName || modDir == NULL || !*modDir ) {
		return -1;
	}

	DWORD i, j;
	float winAspect = (float)PhdWinWidth / (float)PhdWinHeight;
	float stretch[numAspects];
	BYTE tmp, idx[numAspects];
	for( i = 0; i < numAspects; ++i ) {
		float imgAspect = (float)aspects[i][0] / (float)aspects[i][1];
		idx[i] = i;
		stretch[i] = (imgAspect > winAspect) ? (imgAspect / winAspect) : (winAspect / imgAspect);
	}

	for( i = 0; i < (numAspects - 1); ++i ) {
		for( j = (i + 1); j < numAspects; ++j ) {
			if( stretch[idx[i]] > stretch[idx[j]] ) {
				SWAP(idx[i], idx[j], tmp); // sort stretch values
			}
		}
	}

	bool isRemasterEnabled = RemasteredPixEnabled && SavedAppSettings.RenderMode == RM_Hardware && TextureFormat.bpp >= 16;
	char altPath[256];
	for( i = 0; i < numAspects; ++i ) {
		snprintf(altPath, sizeof(altPath), ".\\%s\\%dx%d", modDir, aspects[idx[i]][0], aspects[idx[i]][1]);
		if( 0 < AutoSelectPathAndExtension(fileName, altPath, exts, isRemasterEnabled ? ARRAY_SIZE(exts) : 0) ) {
			return 2;
		}
	}

	snprintf(altPath, sizeof(altPath), ".\\%s", modDir);
	return AutoSelectPathAndExtension(fileName, altPath, exts, isRemasterEnabled ? ARRAY_SIZE(exts) : 0);
}


int __cdecl BGND2_FadeTo(int target, int delta) {
	int current = BGND_TextureAlpha;

	if( target > current && delta > 0 )	{
		current += delta;
		CLAMPG(current, target);
	} else if( target < current && delta < 0 ) {
		current += delta;
		CLAMPL(current, target);
	} else {
		return current;
	}

	CLAMP(current, 0, 255);
	BGND_TextureAlpha = current;
	return current;
}

static void BGND2_CustomBlt(LPDDSDESC dst, DWORD dstX, DWORD dstY, LPDDSDESC src, LPRECT srcRect) {
	DWORD srcBpp = src->ddpfPixelFormat.dwRGBBitCount/8;
	DWORD dstBpp = dst->ddpfPixelFormat.dwRGBBitCount/8;
	COLOR_BIT_MASKS srcMask, dstMask;

	WinVidGetColorBitMasks(&srcMask, &src->ddpfPixelFormat);
	WinVidGetColorBitMasks(&dstMask, &dst->ddpfPixelFormat);

	DWORD srcX = srcRect->left;
	DWORD srcY = srcRect->top;
	DWORD width = srcRect->right - srcRect->left;
	DWORD height = srcRect->bottom - srcRect->top;

	BYTE *srcLine = (BYTE *)src->lpSurface + srcY * src->lPitch  + srcX * srcBpp;
	BYTE *dstLine = (BYTE *)dst->lpSurface + dstY * dst->lPitch  + dstX * dstBpp;
	for( DWORD j = 0; j < height; ++j ) {
		BYTE *srcPtr = srcLine;
		BYTE *dstPtr = dstLine;
		for( DWORD i = 0; i < width; ++i ) {
			DWORD color = 0;
			memcpy(&color, srcPtr, srcBpp);
			DWORD red   = ((color & srcMask.dwRBitMask) >> srcMask.dwRBitOffset);
			DWORD green = ((color & srcMask.dwGBitMask) >> srcMask.dwGBitOffset);
			DWORD blue  = ((color & srcMask.dwBBitMask) >> srcMask.dwBBitOffset);
			if( srcMask.dwRBitDepth < dstMask.dwRBitDepth ) {
				DWORD high = dstMask.dwRBitDepth - srcMask.dwRBitDepth;
				DWORD low = (srcMask.dwRBitDepth > high) ? srcMask.dwRBitDepth - high : 0;
				red = (red << high) | (red >> low);
			} else if( srcMask.dwRBitDepth > dstMask.dwRBitDepth ) {
				red >>= srcMask.dwRBitDepth - dstMask.dwRBitDepth;
			}
			if( srcMask.dwGBitDepth < dstMask.dwGBitDepth ) {
				DWORD high = dstMask.dwGBitDepth - srcMask.dwGBitDepth;
				DWORD low = (srcMask.dwGBitDepth > high) ? srcMask.dwGBitDepth - high : 0;
				green = (green << high) | (green >> low);
			} else if( srcMask.dwGBitDepth > dstMask.dwGBitDepth ) {
				green >>= srcMask.dwGBitDepth - dstMask.dwGBitDepth;
			}
			if( srcMask.dwBBitDepth < dstMask.dwBBitDepth ) {
				DWORD high = dstMask.dwBBitDepth - srcMask.dwBBitDepth;
				DWORD low = (srcMask.dwBBitDepth > high) ? srcMask.dwBBitDepth - high : 0;
				blue = (blue << high) | (blue >> low);
			} else if( srcMask.dwBBitDepth > dstMask.dwBBitDepth ) {
				blue >>= srcMask.dwBBitDepth - dstMask.dwBBitDepth;
			}
			color = dst->ddpfPixelFormat.dwRGBAlphaBitMask; // destination is opaque
			color |= red   << dstMask.dwRBitOffset;
			color |= green << dstMask.dwGBitOffset;
			color |= blue  << dstMask.dwBBitOffset;
			memcpy(dstPtr, &color, dstBpp);
			srcPtr += srcBpp;
			dstPtr += dstBpp;
		}
		srcLine += src->lPitch;
		dstLine += dst->lPitch;
	}
}

int __cdecl BGND2_CapturePicture() {
	bool isSrcLock = false;
	int ret = 0;
	DDSDESC srcDesc, dstDesc;
	DWORD width = 0;
	DWORD height = 0;
	RECT rect = {0, 0, 0, 0};

	if( SavedAppSettings.RenderMode != RM_Hardware || TextureFormat.bpp < 16 ) {
		return -1;
	}

	LPDDS surface = CaptureBufferSurface ? CaptureBufferSurface : PrimaryBufferSurface;

	BGND_PictureIsReady = false;
	BGND_IsCaptured = false;

	// do game window capture, not the whole screen
	if( !GetClientRect(HGameWindow, &rect) ) {
		return -1;
	}

	if( surface == PrimaryBufferSurface ) {
		MapWindowPoints(HGameWindow, GetParent(HGameWindow), (LPPOINT)&rect, 2);
	}
	width = ABS(rect.right - rect.left);
	height = ABS(rect.bottom - rect.top);

	DWORD side = GetMaxTextureSize();

	DWORD nx = (width + side - 1) / side;
	DWORD ny = (height + side - 1) / side;

	if( nx*ny >= ARRAY_SIZE(BGND_TexturePageIndexes) ) {
		return -1; // It seems image is too big
	}

	int x[ARRAY_SIZE(BGND_TexturePageIndexes) + 1];
	for( DWORD i = 0; i < nx; ++i ) {
		x[i] = i * side * (rect.right - rect.left) / width + rect.left;
	}
	x[nx] = rect.right;

	int y[ARRAY_SIZE(BGND_TexturePageIndexes) + 1];
	for( DWORD i = 0; i < ny; ++i ) {
		y[i] = i * side * (rect.bottom - rect.top) / height + rect.top;
	}
	y[ny] = rect.bottom;

	for( DWORD j = 0; j < ny; ++j ) {
		for( DWORD i = 0; i < nx; ++i ) {
			RECT r = {x[i], y[j], x[i+1], y[j+1]};
			int pageIndex = CreateCaptureTexture(i + j*nx, side);
			if( pageIndex < 0 ) {
				ret = -1;
				goto CLEANUP;
			}

			// NOTE: On some system default Blt/BltFast is unsupported here but returns no error, so failsafe custom blitter is used instead
			if( !isSrcLock ) {
				HRESULT rc;
				memset(&srcDesc, 0, sizeof(srcDesc));
				srcDesc.dwSize = sizeof(srcDesc);
				do {
					rc = surface->Lock(&rect, &srcDesc, DDLOCK_READONLY|DDLOCK_WAIT, NULL);
				} while( rc == DDERR_WASSTILLDRAWING );
				if( rc == DDERR_SURFACELOST ) {
					rc = surface->Restore();
				}
				if FAILED(rc) {
					ret = -1;
					goto CLEANUP;
				}
				isSrcLock = true;
			}
			if FAILED(WinVidBufferLock(TexturePages[pageIndex].sysMemSurface, &dstDesc, DDLOCK_WRITEONLY|DDLOCK_WAIT)) {
				ret = -1;
				goto CLEANUP;
			}
			BGND2_CustomBlt(&dstDesc, 0, 0, &srcDesc, &r);
			WinVidBufferUnlock(TexturePages[pageIndex].sysMemSurface, &dstDesc);

			if( !LoadTexturePage(pageIndex, false) ) {
				ret = -1;
				goto CLEANUP;
			}
		}
	}

	BGND_TextureSide = side;
	BGND_TextureAlpha = 255;
	BGND_PictureWidth = width;
	BGND_PictureHeight = height;
	BGND_PictureIsReady = true;
	BGND_IsCaptured = true;

CLEANUP :
	if( isSrcLock ) {
#if (DIRECT3D_VERSION >= 0x700)
		surface->Unlock(&rect);
#else // (DIRECT3D_VERSION >= 0x700)
		surface->Unlock(srcDesc.lpSurface);
#endif // (DIRECT3D_VERSION >= 0x700)
	}
	return ret;
}


int __cdecl BGND2_LoadPicture(LPCTSTR fileName, BOOL isTitle, BOOL isReload) {
	static char lastFileName[256] = {0};
	static char lastFullPath[256] = {0};
	static int lastWinWidth = 0;
	static int lastWinHeight = 0;
	static BOOL lastTitleState = 0;
	DWORD bytesRead;
	HANDLE hFile;
	DWORD fileSize, bitmapSize;
	BYTE *fileData = NULL;
	BYTE *bitmapData = NULL;
	DWORD width, height;
	char fullPath[256] = {0};
	bool isPCX;
	int pickResult = -1;

	BGND_IsCaptured = false; // captured screen is not valid since we want picture file now

	if( isReload ) {
		if( IsGameWindowChanging || IsGameWindowUpdating ||
			(lastWinWidth == PhdWinWidth && lastWinHeight == PhdWinHeight) )
		{
			return 0; // same dimensions - no need to reload picture
		}
		fileName = lastFileName; // assign last fileName pointer as parameter
		isTitle = lastTitleState; // copy last isTitle state
	} else {
		BGND_TextureAlpha = 255;
		if( fileName == NULL || *fileName == 0 ) {
			goto FAIL;
		}
		strncpy(lastFileName, fileName, sizeof(lastFileName)-1); // backup filename string
		lastTitleState = isTitle; // backup isTitle state
	}

	lastWinWidth = PhdWinWidth;
	lastWinHeight = PhdWinHeight;

#ifdef FEATURE_GOLD
	if( IsGold() ) {
		AddFilenameSuffix(fullPath, sizeof(fullPath), GetFullPath(fileName), "g");
		pickResult = PickBestPictureFile(fullPath, "pix");
	}
	if( !IsGold() || pickResult < 0 ) {
		strncpy(fullPath, GetFullPath(fileName), sizeof(fullPath)-1);
		pickResult = PickBestPictureFile(fullPath, "pix");
	}
#else // !FEATURE_GOLD
	strncpy(fullPath, GetFullPath(fileName), sizeof(fullPath));
	pickResult = PickBestPictureFile(fullPath, "pix");
#endif // FEATURE_GOLD

	if( pickResult < 0 ) {
		if( isReload ) {
			if( !strncmp(lastFullPath, fullPath, sizeof(lastFullPath)) ) {
				return 0; // same filepath - no need to reload picture
			}
			strncpy(lastFullPath, fullPath, sizeof(lastFullPath));
		} else {
			goto FAIL;
		}
	}

	if( INVALID_FILE_ATTRIBUTES == GetFileAttributes(fullPath) ) {
		goto FAIL;
	}

	if( !stricmp(PathFindExtension(fullPath), ".pcx") ) {
		hFile = CreateFile(fullPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if( hFile == INVALID_HANDLE_VALUE ) {
			goto FAIL;
		}
		fileSize = GetFileSize(hFile, NULL);
		fileData = (BYTE *)malloc(fileSize);
		ReadFile(hFile, fileData, fileSize, &bytesRead, NULL);
		CloseHandle(hFile);

		if( GetPcxResolution(fileData, fileSize, &width, &height) ) {
			goto FAIL;
		}
		bitmapSize = width * height;
		bitmapData = (BYTE *)malloc(bitmapSize);
		DecompPCX(fileData, fileSize, bitmapData, PicPalette);
		isPCX = true;
	} else if( SavedAppSettings.RenderMode == RM_Hardware && TextureFormat.bpp >= 16 ) {
		if( GDI_LoadImageFile(fullPath, &bitmapData, &width, &height, 16) ) {
			goto FAIL;
		}
		bitmapSize = width * height * 2;
		isPCX = false;
	} else {
		goto FAIL;
	}

	if( PictureBufferSurface != NULL &&
		(BGND_PictureWidth != width || BGND_PictureHeight != height) )
	{
		BGND_PictureWidth = width;
		BGND_PictureHeight = height;
		PictureBufferSurface->Release();
		PictureBufferSurface = NULL;
	}
	if( PictureBufferSurface == NULL ) {
		try {
			CreatePictureBuffer();
		} catch(...) {
			goto FAIL;
		}
	}

	if( SavedAppSettings.RenderMode == RM_Software )
		WinVidCopyBitmapToBuffer(PictureBufferSurface, bitmapData);
	else
		MakeBgndTextures(width, height, bitmapData, isPCX ? PicPalette : NULL);

	if( !isTitle && isPCX ) {
		CopyBitmapPalette(PicPalette, bitmapData, bitmapSize, GamePalette8);
	}
	if( bitmapData != NULL ) {
		free(bitmapData);
	}
	if( fileData != NULL ) {
		free(fileData);
	}

	return 0;

FAIL :
	if( PictureBufferSurface != NULL ) {
		PictureBufferSurface->Release();
		PictureBufferSurface = NULL;
	}
	if( bitmapData != NULL ) {
		free(bitmapData);
	}
	if( fileData != NULL ) {
		free(fileData);
	}

	S_DontDisplayPicture();
	return -1;
}

int __cdecl BGND2_ShowPicture(DWORD fadeIn, DWORD waitIn, DWORD fadeOut, DWORD waitOut, BOOL inputCheck) {
	if( SavedAppSettings.RenderMode == RM_Software ) {
		RGB888 blackPal[256];
		memset(blackPal, 0, sizeof(blackPal));

		// output picture
		S_InitialisePolyList(FALSE);
		S_CopyBufferToScreen();
		S_OutputPolyList();
		S_DumpScreen();

		// fade in
		if( fadeIn > 0 ) {
			memset(WinVidPalette, 0, sizeof(WinVidPalette));
			FadeToPal(fadeIn, GamePalette8);
		}
		if( waitIn > 0) {
			S_Wait(waitIn * TICKS_PER_FRAME, inputCheck);
		}
		if( IsGameToExit ) {
			return -1;
		}

		// fade out
		if( fadeOut > 0 ) {
			FadeToPal(fadeOut, blackPal);
		}
		if( fadeOut > 0 || waitOut > 0 ) {
			ScreenClear(false); ScreenDump();
			ScreenClear(false); ScreenDump();
		}
		if( waitOut > 2 ) {
			S_Wait((waitOut - 2) * TICKS_PER_FRAME, inputCheck);
		}
		if( IsGameToExit ) {
			return -1;
		}
	} else {
		DWORD i = fadeIn + waitIn + fadeOut + waitOut;
		DWORD phase = 0;
		DWORD frame = 0;
		BGND_TextureAlpha = 255;
		while( i-- ) {
			switch( phase ) {
				case 0 :
					if( frame < fadeIn ) {
						BGND_TextureAlpha = 255 * frame / fadeIn;
						break;
					}
					++phase;
					frame = 0;
					// fall through
				case 1 :
					if( frame < waitIn ) {
						BGND_TextureAlpha = 255;
						break;
					}
					++phase;
					frame = 0;
					// fall through
				case 2 :
					if( frame < fadeOut ) {
						BGND_TextureAlpha = 255 * (fadeOut - frame - 1) / fadeOut;
						break;
					}
					++phase;
					frame = 0;
					// fall through
				case 3 :
					if( frame < waitOut ) {
						BGND_TextureAlpha = 0;
						break;
					}
					++phase;
					frame = 0;
					// fall through
				default :
					return 0;
			}
			S_InitialisePolyList(FALSE);
			S_CopyBufferToScreen();
			S_OutputPolyList();
			S_DumpScreen();
			S_UpdateInput();
			if( IsGameToExit ) {
				return -1;
			}
			if( inputCheck && InputStatus != 0 ) {
				// fade out faster if key pressed
				inputCheck = FALSE;
				fadeOut /= 2;
				waitOut /= 2;
				phase = 2;
				for( frame = 0; frame < fadeOut; ++frame ) {
					if( BGND_TextureAlpha > 255 * (fadeOut - frame - 1) / fadeOut ) {
						break;
					}
				}
			}
			++frame;
#ifdef FEATURE_INPUT_IMPROVED
			UpdateJoyOutput(false);
#endif // FEATURE_INPUT_IMPROVED
		}
	}
	return 0;
}

static void __cdecl BGND2_DrawTexture(RECT *rect, HWR_TEXHANDLE texSource,
									  int tu, int tv, int t_width, int t_height, int t_side,
									  D3DCOLOR color0, D3DCOLOR color1, D3DCOLOR color2, D3DCOLOR color3)
{
	float sx0, sy0, sx1, sy1;
	float tu0, tv0, tu1, tv1;
	DWORD alphaState;
	D3DTLVERTEX vertex[4];

	if( rect == NULL ) {
		return;
	}

	sx0 = (double)rect->left;
	sy0 = (double)rect->top;
	sx1 = (double)rect->right;
	sy1 = (double)rect->bottom;

	tu0 = (double)tu / (double)t_side;
	tv0 = (double)tv / (double)t_side;
	tu1 = (double)(tu + t_width)  / (double)t_side;
	tv1 = (double)(tv + t_height) / (double)t_side;

	vertex[0].sx = sx0;
	vertex[0].sy = sy0;
	vertex[0].color = RGBA_SETALPHA(color0, BGND_TextureAlpha);
	vertex[0].tu = tu0;
	vertex[0].tv = tv0;

	vertex[1].sx = sx1;
	vertex[1].sy = sy0;
	vertex[1].color = RGBA_SETALPHA(color1, BGND_TextureAlpha);
	vertex[1].tu = tu1;
	vertex[1].tv = tv0;

	vertex[2].sx = sx0;
	vertex[2].sy = sy1;
	vertex[2].color = RGBA_SETALPHA(color2, BGND_TextureAlpha);
	vertex[2].tu = tu0;
	vertex[2].tv = tv1;

	vertex[3].sx = sx1;
	vertex[3].sy = sy1;
	vertex[3].color = RGBA_SETALPHA(color3, BGND_TextureAlpha);
	vertex[3].tu = tu1;
	vertex[3].tv = tv1;

	for( int i=0; i<4; ++i ) {
		vertex[i].sz = 0.995;
		vertex[i].rhw = RhwFactor / FltFarZ;
		vertex[i].specular = 0;
	}

	HWR_TexSource(texSource);
	HWR_EnableColorKey(false);
	D3DDev->GetRenderState(AlphaBlendEnabler, &alphaState);
	D3DDev->SetRenderState(AlphaBlendEnabler, TRUE);
	D3DDev->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3D_TLVERTEX, &vertex, 4, D3D_DRAWFLAGS);
	D3DDev->SetRenderState(AlphaBlendEnabler, alphaState);
}

void __cdecl BGND2_DrawTextures(RECT *rect, D3DCOLOR color) {
	if( !BGND_PictureIsReady || !BGND_TextureSide ) {
		return;
	}

	DWORD width = BGND_PictureWidth;
	DWORD height = BGND_PictureHeight;
	DWORD side = BGND_TextureSide;
	DWORD nx = (width + side - 1) / side;
	DWORD ny = (height + side - 1) / side;

	if( nx*ny >= ARRAY_SIZE(BGND_TexturePageIndexes) ) {
		return; // It seems image is too big
	}

	BGND_GetPageHandles();

	int x[ARRAY_SIZE(BGND_TexturePageIndexes) + 1];
	for( DWORD i = 0; i < nx; ++i ) {
		x[i] = i * side * (rect->right - rect->left) / width + rect->left;
	}
	x[nx] = rect->right;

	int y[ARRAY_SIZE(BGND_TexturePageIndexes) + 1];
	for( DWORD i = 0; i < ny; ++i ) {
		y[i] = i * side * (rect->bottom - rect->top) / height + rect->top;
	}
	y[ny] = rect->bottom;

	for( DWORD j = 0; j < ny; ++j ) {
		for( DWORD i = 0; i < nx; ++i ) {
			DWORD w = side;
			DWORD h = side;
			if( i == nx - 1 && width % side ) w = width % side;
			if( j == ny - 1 && height % side ) h = height % side;
			RECT r = {x[i], y[j], x[i+1], y[j+1]};
			BGND2_DrawTexture(&r, BGND_PageHandles[i + j*nx], 0, 0, w, h, side, color, color, color, color);
		}
	}
}

int __cdecl BGND2_CalculatePictureRect(RECT *rect) {
	if( rect == NULL || !PhdWinWidth || !PhdWinHeight || !BGND_PictureWidth || !BGND_PictureHeight )
		return -1;

	if( PictureStretchLimit >= 100 ) {
		*rect = PhdWinRect;
		return 1;
	}

	DWORD stretch;
	int x, y, w, h;
	double windowAspect = (double)PhdWinWidth / (double)PhdWinHeight;
	double pictureAspect = (double)BGND_PictureWidth / (double)BGND_PictureHeight;

	if( windowAspect > pictureAspect ) {
		w = (double)PhdWinHeight * pictureAspect;
		h = PhdWinHeight;
		x = (PhdWinWidth - w) / 2;
		y = 0;
		stretch = 100 * PhdWinWidth / w - 100;
	} else {
		w = PhdWinWidth;
		h = (double)PhdWinWidth / pictureAspect;
		x = 0;
		y = (PhdWinHeight - h) / 2;
		stretch = 100 * PhdWinHeight / h - 100;
	}

	if( stretch <= PictureStretchLimit ) {
		*rect = PhdWinRect;
		return 1;
	}

	rect->left = x;
	rect->top = y;
	rect->right = x + w;
	rect->bottom = y + h;
	return 0;
}

#endif // FEATURE_BACKGROUND_IMPROVED
