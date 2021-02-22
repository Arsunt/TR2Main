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
#include "specific/winvid.h"
#include "modding/file_utils.h"
#include "modding/gdi_utils.h"
#include "modding/texture_utils.h"
#include "global/vars.h"

#ifdef FEATURE_BACKGROUND_IMPROVED
extern LPDDS CaptureBufferSurface;
extern TEXPAGE_DESC TexturePages[256];

#ifdef FEATURE_INPUT_IMPROVED
#include "modding/joy_output.h"
#endif // FEATURE_INPUT_IMPROVED

#ifdef FEATURE_GOLD
extern bool IsGold();
#endif

static int BGND_CapturePageIndexes[64];
int BGND_TexturePageIndexes[64];
HWR_TEXHANDLE BGND_PageHandles[64];

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
	extern int PatternTexPage;
	D3DTLVERTEX vtx[4];

	float tu0 = (double)txr->u / 256.0;
	float tv0 = (double)txr->v / 256.0;
	float tu1 = (double)(txr->u + txr->width)  / 256.0;
	float tv1 = (double)(txr->v + txr->height) / 256.0;
	if( PatternTexPage < 0 ) {
		double uvAdjust = (double)UvAdd / (double)(256 * GetTextureSideByHandle(txr->handle));
		CLAMPL(uvAdjust, 1.0/double(PHD_ONE));
		tu0 += uvAdjust;
		tv0 += uvAdjust;
		tu1 -= uvAdjust;
		tv1 -= uvAdjust;
	}

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
	HWR_DrawPrimitive(D3DPT_TRIANGLESTRIP, &vtx, 4, false);
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
#if defined(FEATURE_VIDEOFX_IMPROVED) && (DIRECT3D_VERSION >= 0x900)
			if( SavedAppSettings.LightingMode ) light /= 2;
#endif // defined(FEATURE_VIDEOFX_IMPROVED) && (DIRECT3D_VERSION >= 0x900)
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

static int CreateCaptureTexture(DWORD index, int side) {
	int pageIndex = BGND_CapturePageIndexes[index];
	if( pageIndex >= 0 && TexturePages[pageIndex].width != side ) {
		SafeFreeTexturePage(pageIndex);
		pageIndex = -1;
	}
	if( pageIndex < 0 || !CHK_ANY(TexturePages[pageIndex].status, 1) ) {
		DDSDESC desc;
#if (DIRECT3D_VERSION >= 0x900)
		pageIndex = CreateTexturePage(side, side, false);
		if( pageIndex < 0 ) {
			return -1;
		}
		if SUCCEEDED(TexturePages[pageIndex].texture->LockRect(0, &desc, NULL, 0)) {
			TexturePages[pageIndex].texture->UnlockRect(0);
		}
#else // (DIRECT3D_VERSION >= 0x900)
		pageIndex = CreateTexturePage(side, side, NULL);
		if SUCCEEDED(WinVidBufferLock(TexturePages[pageIndex].sysMemSurface, &desc, DDLOCK_WRITEONLY|DDLOCK_WAIT)) {
			WinVidBufferUnlock(TexturePages[pageIndex].sysMemSurface, &desc);
		}
#endif // (DIRECT3D_VERSION >= 0x900)
		if( pageIndex < 0 ) {
			return -1;
		}
		BGND_CapturePageIndexes[index] = pageIndex;
	}
	return pageIndex;
}

void BGND2_CleanupCaptureTextures() {
	for( DWORD i=0; i<ARRAY_SIZE(BGND_CapturePageIndexes); ++i ) {
		BGND_CapturePageIndexes[i] = -1;
	}
}

int BGND2_PrepareCaptureTextures() {
	static bool once = false;
	if( !once ) {
		BGND2_CleanupCaptureTextures();
		once = true;
	}
	DWORD side = MIN(2048, GetMaxTextureSize());
	DWORD nx = (GameVidWidth + side - 1) / side;
	DWORD ny = (GameVidHeight + side - 1) / side;
	for( DWORD i=0; i<ARRAY_SIZE(BGND_CapturePageIndexes); ++i ) {
		if( i < nx*ny ) {
			CreateCaptureTexture(i, side);
		} else {
			SafeFreeTexturePage(BGND_CapturePageIndexes[i]);
			BGND_CapturePageIndexes[i] = -1;
		}
	}
	return 0;
}

static int MakeBgndTextures(DWORD width, DWORD height, DWORD bpp, BYTE *bitmap, RGB888 *bmpPal) {
	DWORD side = MIN(2048, GetMaxTextureSize());
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
			int pageIndex = MakeCustomTexture(i*side, j*side, w, h, width, side, bpp, bitmap, bmpPal, BGND_PaletteIndex, NULL, false);
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

static int __cdecl BGND2_FadeToPal(int fadeValue, RGB888 *palette, int inputCheck) {
	int i, j;
	int palStartIdx = 0;
	int palEndIdx = 256;
#if (DIRECT3D_VERSION < 0x900)
	int palSize = 256;
#endif // (DIRECT3D_VERSION < 0x900)
	bool fadeFaster = false;
	PALETTEENTRY fadePal[256];

#if (DIRECT3D_VERSION >= 0x900)
	if( SavedAppSettings.RenderMode != RM_Software )
		return fadeValue;
#else // (DIRECT3D_VERSION >= 0x900)
	if( !GameVid_IsVga )
		return fadeValue;

	if( GameVid_IsWindowedVga ) {
		palStartIdx += 10;
		palEndIdx -= 10;
		palSize -= 20;
	}
#endif // (DIRECT3D_VERSION >= 0x900)

	if( fadeValue <= 1 ) {
		for( i=palStartIdx; i<palEndIdx; ++i ) {
			WinVidPalette[i].peRed   = palette[i].red;
			WinVidPalette[i].peGreen = palette[i].green;
			WinVidPalette[i].peBlue  = palette[i].blue;
		}
#if (DIRECT3D_VERSION >= 0x900)
		S_InitialisePolyList(FALSE);
		S_OutputPolyList();
#else // (DIRECT3D_VERSION >= 0x900)
		DDrawPalette->SetEntries(0, palStartIdx, palSize, &WinVidPalette[palStartIdx]);
#endif // (DIRECT3D_VERSION >= 0x900)
		return fadeValue;
	}

	for( i=palStartIdx; i<palEndIdx; ++i ) {
		fadePal[i] = WinVidPalette[i];
	}

	for( j=0; j<=fadeValue; ++j ) {
		if( S_UpdateInput() ) return fadeValue;
		if( inputCheck && InputStatus ) {
			if( inputCheck == 1 ) {
				return fadeValue - j + 1;
			} else if( inputCheck == 2 ) {
				fadeFaster = true;
			}
			if( fadeFaster && j < fadeValue ) ++j;
		}
		for( i=palStartIdx; i<palEndIdx; ++i ) {
			WinVidPalette[i].peRed   = fadePal[i].peRed   + (palette[i].red   - fadePal[i].peRed)   * j / fadeValue;
			WinVidPalette[i].peGreen = fadePal[i].peGreen + (palette[i].green - fadePal[i].peGreen) * j / fadeValue;
			WinVidPalette[i].peBlue  = fadePal[i].peBlue  + (palette[i].blue  - fadePal[i].peBlue)  * j / fadeValue;
		}
#if (DIRECT3D_VERSION >= 0x900)
		S_InitialisePolyList(FALSE);
		S_OutputPolyList();
#else // (DIRECT3D_VERSION >= 0x900)
		DDrawPalette->SetEntries(0, palStartIdx, palSize, &WinVidPalette[palStartIdx]);
#endif // (DIRECT3D_VERSION >= 0x900)
		S_DumpScreen();
	}
	return fadeFaster ? 1 : 0;
}

static void BGND2_CustomBlt(LPDDSDESC dst, DWORD dstX, DWORD dstY, LPDDSDESC src, LPRECT srcRect) {
	DWORD srcX = srcRect->left;
	DWORD srcY = srcRect->top;
	DWORD width = srcRect->right - srcRect->left;
	DWORD height = srcRect->bottom - srcRect->top;

#if (DIRECT3D_VERSION >= 0x900)
	BYTE *srcLine = (BYTE *)src->pBits + srcY * src->Pitch  + srcX * 4;
	BYTE *dstLine = (BYTE *)dst->pBits + dstY * dst->Pitch  + dstX * 4;

	for( DWORD j = 0; j < height; ++j ) {
		memcpy(dstLine, srcLine, sizeof(DWORD) * width);
		srcLine += src->Pitch;
		dstLine += dst->Pitch;
	}
#else // (DIRECT3D_VERSION >= 0x900)
	DWORD srcBpp = src->ddpfPixelFormat.dwRGBBitCount/8;
	DWORD dstBpp = dst->ddpfPixelFormat.dwRGBBitCount/8;
	COLOR_BIT_MASKS srcMask, dstMask;

	WinVidGetColorBitMasks(&srcMask, &src->ddpfPixelFormat);
	WinVidGetColorBitMasks(&dstMask, &dst->ddpfPixelFormat);

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
#endif // (DIRECT3D_VERSION >= 0x900)
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

	BGND_PictureIsReady = false;
	BGND_IsCaptured = false;

	// do game window capture, not the whole screen
	if( !GetClientRect(HGameWindow, &rect) ) {
		return -1;
	}

#if (DIRECT3D_VERSION < 0x900)
	LPDDS surface = CaptureBufferSurface ? CaptureBufferSurface : PrimaryBufferSurface;
#endif // (DIRECT3D_VERSION < 0x900)
	if( CaptureBufferSurface == NULL ) {
		MapWindowPoints(HGameWindow, GetParent(HGameWindow), (LPPOINT)&rect, 2);
	}
	width = ABS(rect.right - rect.left);
	height = ABS(rect.bottom - rect.top);

	DWORD side = MIN(2048, GetMaxTextureSize());

	DWORD nx = (width + side - 1) / side;
	DWORD ny = (height + side - 1) / side;

	if( nx*ny >= ARRAY_SIZE(BGND_CapturePageIndexes) ) {
		return -1; // It seems image is too big
	}

	int x[ARRAY_SIZE(BGND_CapturePageIndexes) + 1];
	x[nx] = rect.right - rect.left;
	for( DWORD i = 0; i < nx; ++i ) {
		x[i] = i * side * x[nx] / width;
	}

	int y[ARRAY_SIZE(BGND_CapturePageIndexes) + 1];
	y[ny] = rect.bottom - rect.top;
	for( DWORD i = 0; i < ny; ++i ) {
		y[i] = i * side * y[ny] / height;
	}

#if (DIRECT3D_VERSION >= 0x900)
	DISPLAY_MODE mode;
	LPDDS surface = NULL;
	if( CaptureBufferSurface != NULL ) {
		surface = CaptureBufferSurface;
	} else if( !WinVidGetDisplayMode(&mode)
		|| FAILED(D3DDev->CreateOffscreenPlainSurface(mode.width, mode.height, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &surface, NULL))
		|| FAILED(D3DDev->GetFrontBufferData(0, surface)) )
	{
		ret = -1;
		goto CLEANUP;
	}
#endif // (DIRECT3D_VERSION >= 0x900)

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
#if (DIRECT3D_VERSION >= 0x900)
				rc = surface->LockRect(&srcDesc, &rect, D3DLOCK_READONLY);
#else // (DIRECT3D_VERSION >= 0x900)
				srcDesc.dwSize = sizeof(srcDesc);
				do {
					rc = surface->Lock(&rect, &srcDesc, DDLOCK_READONLY|DDLOCK_WAIT, NULL);
				} while( rc == DDERR_WASSTILLDRAWING );
				if( rc == DDERR_SURFACELOST ) {
					rc = surface->Restore();
				}
#endif // (DIRECT3D_VERSION >= 0x900)
				if FAILED(rc) {
					ret = -1;
					goto CLEANUP;
				}
				isSrcLock = true;
			}
#if (DIRECT3D_VERSION >= 0x900)
			if FAILED(TexturePages[pageIndex].texture->LockRect(0, &dstDesc, NULL, 0)) {
				ret = -1;
				goto CLEANUP;
			}
			BGND2_CustomBlt(&dstDesc, 0, 0, &srcDesc, &r);
			TexturePages[pageIndex].texture->UnlockRect(0);
#else // (DIRECT3D_VERSION >= 0x900)
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
#endif // (DIRECT3D_VERSION >= 0x900)
		}
	}

	BGND_TextureSide = side;
	BGND_TextureAlpha = 255;
	BGND_PictureWidth = width;
	BGND_PictureHeight = height;
	BGND_PictureIsReady = true;
	BGND_IsCaptured = true;

CLEANUP :
#if (DIRECT3D_VERSION >= 0x900)
	if( surface != NULL ) {
		if( isSrcLock ) {
			surface->UnlockRect();
		}
		if( surface != CaptureBufferSurface ) {
			surface->Release();
		}
	}
#else // (DIRECT3D_VERSION >= 0x900)
	if( isSrcLock ) {
		surface->Unlock(srcDesc.lpSurface);
	}
#endif // (DIRECT3D_VERSION >= 0x900)
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
	DWORD width, height, bpp = 8;
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
#if (DIRECT3D_VERSION >= 0x900)
		bpp = 32;
#else // (DIRECT3D_VERSION >= 0x900)
		bpp = 16;
#endif // (DIRECT3D_VERSION >= 0x900)
		if( GDI_LoadImageFile(fullPath, &bitmapData, &width, &height, bpp) ) {
			goto FAIL;
		}
		bitmapSize = width * height * 2;
		isPCX = false;
	} else {
		goto FAIL;
	}

#if (DIRECT3D_VERSION >= 0x900)
	if( PictureBuffer.bitmap == NULL ||
		PictureBuffer.width != width ||
		PictureBuffer.height != height )
	{
		BGND_PictureWidth = width;
		BGND_PictureHeight = height;
		try {
			CreatePictureBuffer();
		} catch(...) {
			goto FAIL;
		}
	}
#else // (DIRECT3D_VERSION >= 0x900)
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
#endif // (DIRECT3D_VERSION >= 0x900)

	if( SavedAppSettings.RenderMode == RM_Software ) {
#if (DIRECT3D_VERSION >= 0x900)
		if( PictureBuffer.bitmap != NULL)
			memcpy(PictureBuffer.bitmap, bitmapData, PictureBuffer.width * PictureBuffer.height);
#else // (DIRECT3D_VERSION >= 0x900)
		WinVidCopyBitmapToBuffer(PictureBufferSurface, bitmapData);
#endif // (DIRECT3D_VERSION >= 0x900)
	} else {
		MakeBgndTextures(width, height, bpp, bitmapData, isPCX ? PicPalette : NULL);
	}

	if( !isTitle && isPCX ) {
#if (DIRECT3D_VERSION >= 0x900)
		memcpy(GamePalette8, PicPalette, sizeof(GamePalette8));
#else // (DIRECT3D_VERSION >= 0x900)
		CopyBitmapPalette(PicPalette, bitmapData, bitmapSize, GamePalette8);
#endif // (DIRECT3D_VERSION >= 0x900)
	}
	if( bitmapData != NULL ) {
		free(bitmapData);
	}
	if( fileData != NULL ) {
		free(fileData);
	}

	return 0;

FAIL :
#if (DIRECT3D_VERSION >= 0x900)
	if( PictureBuffer.bitmap != NULL ) {
		free(PictureBuffer.bitmap);
		PictureBuffer.bitmap = NULL;
	}
#else // (DIRECT3D_VERSION >= 0x900)
	if( PictureBufferSurface != NULL ) {
		PictureBufferSurface->Release();
		PictureBufferSurface = NULL;
	}
#endif // (DIRECT3D_VERSION >= 0x900)
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
		int skip = 0;
		RGB888 blackPal[256];
		memset(blackPal, 0, sizeof(blackPal));

		// output picture
		S_InitialisePolyList(FALSE);
		S_CopyBufferToScreen();
		S_OutputPolyList();

		// fade in
		if( fadeIn > 0 ) {
			memset(WinVidPalette, 0, sizeof(WinVidPalette));
			skip = BGND2_FadeToPal(fadeIn, GamePalette8, inputCheck ? 1 : 0);
		}
		if( IsGameToExit ) {
			return -1;
		}
		if( skip ) {
			inputCheck = FALSE;
			fadeOut /= 2;
			waitOut /= 2;
		} else if( waitIn > 0 ) {
			S_Wait(waitIn * TICKS_PER_FRAME, inputCheck);
		}
		if( IsGameToExit ) {
			return -1;
		}

		// fade out
		if( fadeOut > 0 ) {
			skip = BGND2_FadeToPal(fadeOut, blackPal, inputCheck ? 2 : 0);
			if( skip ) waitOut /= 2;
		}
		if( IsGameToExit ) {
			return -1;
		}
		if( fadeOut > 0 || waitOut > 0 ) {
#if (DIRECT3D_VERSION >= 0x900)
			ScreenClear(false);
			S_Wait(2 * TICKS_PER_FRAME, FALSE);
#else // (DIRECT3D_VERSION >= 0x900)
			ScreenClear(false); ScreenDump();
			ScreenClear(false); ScreenDump();
#endif // (DIRECT3D_VERSION >= 0x900)
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
						BGND_TextureAlpha = 255 * frame / (fadeIn - 1);
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
	HWR_DrawPrimitive(D3DPT_TRIANGLESTRIP, &vertex, 4, true);
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

			int *index = BGND_IsCaptured ? BGND_CapturePageIndexes : BGND_TexturePageIndexes;
			BGND2_DrawTexture(&r, GetTexturePageHandle(index[i + j*nx]), 0, 0, w, h, side, color, color, color, color);
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
