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
#include "modding/background_new.h"
#include "3dsystem/phd_math.h"
#include "specific/background.h"
#include "specific/file.h"
#include "specific/hwr.h"
#include "specific/init_display.h"
#include "specific/output.h"
#include "specific/texture.h"
#include "specific/utils.h"
#include "specific/winvid.h"
#include "global/vars.h"

#ifdef FEATURE_BACKGROUND_IMPROVED

DWORD BGND_PictureWidth  = 640;
DWORD BGND_PictureHeight = 480;
DWORD BGND_TextureSide  = 1024;

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
	D3DTEXTUREHANDLE handle; ///< Handle of texture
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
	_Direct3DDevice2->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DVT_TLVERTEX, &vtx, 4, D3DDP_DONOTUPDATEEXTENTS|D3DDP_DONOTCLIP);
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
void PSX_Background(D3DTEXTUREHANDLE texSource, int tu, int tv, int t_width, int t_height, int halfRowCount,
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

static int GetPcxResolution(void *pcx, DWORD pcxSize, DWORD *width, DWORD *height) {
	PCX_HEADER *header;

	if( pcx == NULL || pcxSize <= sizeof(PCX_HEADER) || width == NULL || height == NULL ) {
		return -1;
	}

	header  = (PCX_HEADER *)pcx;
	*width  = header->xMax - header->xMin + 1;
	*height = header->yMax - header->yMin + 1;

	if( header->manufacturer != 10 ||
		header->version < 5 ||
		header->bpp != 8 ||
		header->rle != 1 ||
		header->planes != 1 ||
		*width == 0 ||
		*height == 0 )
	{
		return -1;
	}

	return 0;
}

static int MakeBgndTexture(DWORD width, DWORD height, BYTE *bitmap, RGB888 *bmpPal) {
	DWORD side = 1;
	int pageIndex;

	while( side < width || side < height ) {
		side <<= 1;
		if( side > MAX_SURFACE_SIZE ) {
			return -1;
		}
	}

	BGND_PaletteIndex = (TextureFormat.bpp < 16) ? CreateTexturePalette(bmpPal) : -1;

	if( BGND_PaletteIndex < 0 ) {
		UINT16 *tmpBmp = (UINT16 *)calloc(2, SQR(side));
		UINT16 *bmpDst = tmpBmp;
		BYTE *bmpSrc = bitmap;

		// Translating bitmap data from 8 bit bitmap to 16 bit bitmap
		for( DWORD j = 0; j < height; ++j ) {
			for( DWORD i = 0; i < width; ++i ) {
				RGB888 *color = &bmpPal[bmpSrc[i]]; // get RGB color from palette
				bmpDst[i] = (1 << 15) // convert RGB to 16 bit
						| (((UINT16)color->red   >> 3) << 10)
						| (((UINT16)color->green >> 3) << 5)
						| (((UINT16)color->blue  >> 3));
			}
			bmpSrc += width;
			bmpDst += side;
		}
		pageIndex = AddTexturePage16(side, side, (BYTE *)tmpBmp);
		free(tmpBmp);
	} else {
		BYTE *tmpBmp = (BYTE *)calloc(1, SQR(side));
		UT_MemBlt(tmpBmp, 0, 0, width, height, side, bitmap, 0, 0, width);
		pageIndex = AddTexturePage8(side, side, tmpBmp, BGND_PaletteIndex);
		free(tmpBmp);
	}

	if( pageIndex < 0) {
		return -1;
	}

	BGND_PictureWidth = width;
	BGND_PictureHeight = height;
	BGND_TextureSide = side;

	BGND_TexturePageIndexes[0] = pageIndex;
	BGND_GetPageHandles();
	BGND_PictureIsReady = true;
	return 0;
}

int __cdecl BGND2_LoadPicture(LPCTSTR fileName, BOOL isTitle) {
	DWORD bytesRead;
	HANDLE hFile;
	DWORD fileSize, bitmapSize;
	BYTE *fileData = NULL;
	BYTE *bitmapData = NULL;
	DWORD width, height;
	LPCTSTR fullPath;

	if( fileName == NULL || *fileName == 0 ) {
		goto FAIL;
	}

	fullPath = GetFullPath(fileName);
	if( INVALID_FILE_ATTRIBUTES == GetFileAttributes(fullPath) ) {
		goto FAIL;
	}
	hFile = CreateFile(fullPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if( hFile == INVALID_HANDLE_VALUE ) {
		goto FAIL;
	}
	fileSize = GetFileSize(hFile, NULL);
	fileData = (BYTE *)malloc(fileSize);
	ReadFile(hFile, fileData, fileSize, &bytesRead, NULL);
	CloseHandle(hFile);

	if( GetPcxResolution(fileData, fileSize, &width, &height) ||
		width > MAX_SURFACE_SIZE || height > MAX_SURFACE_SIZE )
	{
		goto FAIL;
	}

	bitmapSize = width * height;
	bitmapData = (BYTE *)malloc(bitmapSize);
	DecompPCX(fileData, fileSize, bitmapData, PicPalette);

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
		MakeBgndTexture(width, height, bitmapData, PicPalette);

	if( !isTitle )
		CopyBitmapPalette(PicPalette, bitmapData, bitmapSize, GamePalette8);

	free(bitmapData);
	free(fileData);

	return 0;

FAIL:
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
	return -1;
}


void __cdecl BGND2_DrawTexture(int sx, int sy, int width, int height, D3DTEXTUREHANDLE texSource,
							   int tu, int tv, int t_width, int t_height, int t_side,
							   D3DCOLOR color0, D3DCOLOR color1, D3DCOLOR color2, D3DCOLOR color3)
{
	float sx0, sy0, sx1, sy1;
	float tu0, tv0, tu1, tv1;
	double uvAdjust;
	D3DTLVERTEX vertex[4];

	sx0 = (double)sx;
	sy0 = (double)sy;
	sx1 = (double)(sx + width);
	sy1 = (double)(sy + height);

	uvAdjust = ((double)UvAdd / (double)(PHD_ONE)) * ((double)t_side / 256.0);
	tu0 = (double)tu / (double)t_side + uvAdjust;
	tv0 = (double)tv / (double)t_side + uvAdjust;
	tu1 = (double)(tu + t_width)  / (double)t_side - uvAdjust;
	tv1 = (double)(tv + t_height) / (double)t_side - uvAdjust;

	vertex[0].sx = sx0;
	vertex[0].sy = sy0;
	vertex[0].color = color0;
	vertex[0].tu = tu0;
	vertex[0].tv = tv0;

	vertex[1].sx = sx1;
	vertex[1].sy = sy0;
	vertex[1].color = color1;
	vertex[1].tu = tu1;
	vertex[1].tv = tv0;

	vertex[2].sx = sx0;
	vertex[2].sy = sy1;
	vertex[2].color = color2;
	vertex[2].tu = tu0;
	vertex[2].tv = tv1;

	vertex[3].sx = sx1;
	vertex[3].sy = sy1;
	vertex[3].color = color3;
	vertex[3].tu = tu1;
	vertex[3].tv = tv1;

	for( int i=0; i<4; ++i ) {
		vertex[i].sz = 0.995;
		vertex[i].rhw = RhwFactor / FltFarZ;
		vertex[i].specular = 0;
	}

	HWR_TexSource(texSource);
	HWR_EnableColorKey(false);
	_Direct3DDevice2->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DVT_TLVERTEX, &vertex, 4, D3DDP_DONOTUPDATEEXTENTS|D3DDP_DONOTCLIP);
}

#endif // FEATURE_BACKGROUND_IMPROVED
