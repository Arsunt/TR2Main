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
#include "modding/psx_background.h"
#include "3dsystem/phd_math.h"
#include "specific/hwr.h"
#include "global/vars.h"

#ifdef FEATURE_BACKGROUND_IMPROVED

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

#endif // FEATURE_BACKGROUND_IMPROVED
