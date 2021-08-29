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
#include "3dsystem/3dinsert.h"
#include "specific/hwr.h"
#include "global/vars.h"

#if defined(FEATURE_HUD_IMPROVED) || (DIRECT3D_VERSION >= 0x900)
#include "modding/texture_utils.h"
#endif // defined(FEATURE_HUD_IMPROVED) || (DIRECT3D_VERSION >= 0x900)

#ifdef FEATURE_VIDEOFX_IMPROVED
#include "specific/texture.h"
#include "modding/mod_utils.h"

extern DWORD ShadowMode;
extern DWORD AlphaBlendMode;

bool CustomWaterColorEnabled = true;
#endif // FEATURE_VIDEOFX_IMPROVED

static VERTEX_INFO VBuffer[40]; // NOTE: original size was 20
static D3DTLVERTEX VBufferD3D[32];
static D3DCOLOR GlobalTint = 0; // NOTE: not presented in the original code

#ifdef FEATURE_VIEW_IMPROVED
bool RoomSortEnabled = false;
#define MAKE_ZSORT(z) (RoomSortEnabled && (SavedAppSettings.RenderMode == RM_Software || !SavedAppSettings.ZBuffer) ? (((UINT64)MidSort)<<32)+(DWORD)(z) : (DWORD)(z))
#else // FEATURE_VIEW_IMPROVED
#define MAKE_ZSORT(z) ((DWORD)(z))
#endif // FEATURE_VIEW_IMPROVED

static D3DCOLOR shadeColor(DWORD red, DWORD green, DWORD blue, DWORD alpha, DWORD shade, bool isTextured) {
	CLAMPG(shade, 0x1FFF);

	if( GlobalTint ) {
		red = RGBA_GETRED(GlobalTint);
		green = RGBA_GETGREEN(GlobalTint);
		blue = RGBA_GETBLUE(GlobalTint);
		alpha = RGBA_GETALPHA(GlobalTint);
	}

#if defined(FEATURE_VIDEOFX_IMPROVED) && (DIRECT3D_VERSION >= 0x900)
	if( SavedAppSettings.LightingMode == 1 ) CLAMPL(shade, 0x800);
	if( SavedAppSettings.LightingMode && isTextured ) shade = 0x1000 + shade/2;
	if( !SavedAppSettings.LightingMode && !isTextured ) CLAMPL(shade, 0x1000);
#else // defined(FEATURE_VIDEOFX_IMPROVED) && (DIRECT3D_VERSION >= 0x900)
	// NOTE: The original game bugfix. We need to limit brightness of untextured faces for DirectX 5
	// because brightness of textured faces is limited by D3DTBLEND_MODULATEALPHA or D3DTBLEND_MODULATE
	if( !isTextured ) CLAMPL(shade, 0x1000);
#endif // defined(FEATURE_VIDEOFX_IMPROVED) && (DIRECT3D_VERSION >= 0x900)

	if( shade != 0x1000 ) {
		DWORD brightness = 0x2000 - shade;
		red   = red   * brightness >> 12;
		green = green * brightness >> 12;
		blue  = blue  * brightness >> 12;
	}

	CLAMPG(red,   0xFF);
	CLAMPG(green, 0xFF);
	CLAMPG(blue,  0xFF);
	CLAMPG(alpha, 0xFF);

	if( IsShadeEffect ) {
#if defined(FEATURE_VIDEOFX_IMPROVED) && defined(FEATURE_MOD_CONFIG)
		D3DCOLOR water = GetModWaterColor();
		if( CustomWaterColorEnabled && water ) {
			red   = red   * RGB_GETRED(water) / 256;
			green = green * RGB_GETGREEN(water) / 256;
			blue  = blue  * RGB_GETBLUE(water) / 256;
		} else {
			red   = red   * 128 / 256;
			green = green * 224 / 256;
		}
#else // defined(FEATURE_VIDEOFX_IMPROVED) && defined(FEATURE_MOD_CONFIG)
		red   = red   * 128 / 256;
		green = green * 224 / 256;
#endif // defined(FEATURE_VIDEOFX_IMPROVED) && defined(FEATURE_MOD_CONFIG)
	}
	return RGBA_MAKE(red, green, blue, alpha);
}

static double CalculatePolyZ(SORTTYPE sortType, double z0, double z1, double z2, double z3 = -1.0) {
	double zv = 0.0;

	switch( sortType ) {
		case ST_AvgZ :
			zv = ( z3 > 0.0 ) ? (z0+z1+z2+z3)/4.0 : (z0+z1+z2)/3.0;
			break;

		case ST_MaxZ :
			zv = z0;
			CLAMPL(zv, z1);
			CLAMPL(zv, z2);
			if( z3 > 0.0 ) CLAMPL(zv, z3);
			break;

		case ST_FarZ :
		default :
			zv = 4000000000.0; // the original game value was 1000000000.0
			break;
	}
	return zv;
}

#ifdef FEATURE_VIDEOFX_IMPROVED
static POLYTYPE GetPolyType(UINT16 drawtype) {
	switch( drawtype ) {
		case DRAW_Opaque:
			return POLY_HWR_GTmap;
		case DRAW_Semitrans:
			return AlphaBlendMode ? POLY_HWR_WGTmapHalf : POLY_HWR_WGTmap;
		case DRAW_ColorKey:
			return POLY_HWR_WGTmap;
	}
	return POLY_HWR_WGTmap;
}

bool InsertObjectEM(__int16 *ptrObj, int vtxCount, D3DCOLOR tint, PHD_UV *em_uv) {
	PHD_VBUF *vtx[4];
	PHD_TEXTURE texture;
	PHD_UV *uv = texture.uv;

	if( ptrObj == NULL || em_uv == NULL || vtxCount < 3 || vtxCount > 4 ) {
		return false;
	}

	texture.drawtype = DRAW_ColorKey;
	texture.tpage = (UINT16)~0;
	for( int i = 0; i < vtxCount; ++ i ) {
		vtx[i] = &PhdVBuf[ptrObj[i]];
		texture.uv[i] = em_uv[ptrObj[i]];
	}

	GlobalTint = tint;
	if( vtxCount == 4 ) {
		InsertGT4_Sorted(vtx[0], vtx[1], vtx[2], vtx[3], &texture, ST_AvgZ);
	} else {
		InsertGT3_Sorted(vtx[0], vtx[1], vtx[2], &texture, &uv[0], &uv[1], &uv[2], ST_AvgZ);
	}
	GlobalTint = 0;
	return true;
}
#endif // FEATURE_VIDEOFX_IMPROVED

// NOTE: this function is not presented in the original game
void __cdecl InsertGourQuad(int x0, int y0, int x1, int y1, int z, D3DCOLOR color0, D3DCOLOR color1, D3DCOLOR color2, D3DCOLOR color3) {
	double rhw, sz;

	Sort3dPtr->_0 = (DWORD)Info3dPtr;
	Sort3dPtr->_1 = MAKE_ZSORT(z);
	++Sort3dPtr;

	*(Info3dPtr++) = POLY_HWR_trans;
	*(Info3dPtr++) = 4; //  vertex count
	*(D3DTLVERTEX **)Info3dPtr = HWR_VertexPtr;
	Info3dPtr += sizeof(D3DTLVERTEX *)/sizeof(__int16);

	rhw = RhwFactor / (double)z;
	sz = FltResZBuf - rhw * FltResZORhw;

	HWR_VertexPtr[0].sx = (float)x1;
	HWR_VertexPtr[0].sy = (float)y0;
	HWR_VertexPtr[0].color = color1;

	HWR_VertexPtr[1].sx = (float)x1;
	HWR_VertexPtr[1].sy = (float)y1;
	HWR_VertexPtr[1].color = color2;

	HWR_VertexPtr[2].sx = (float)x0;
	HWR_VertexPtr[2].sy = (float)y1;
	HWR_VertexPtr[2].color = color3;

	HWR_VertexPtr[3].sx = (float)x0;
	HWR_VertexPtr[3].sy = (float)y0;
	HWR_VertexPtr[3].color = color0;

	for( int i=0; i<4; ++i ) {
		HWR_VertexPtr[i].sz = sz;
		HWR_VertexPtr[i].rhw = rhw;
	}

	HWR_VertexPtr += 4;
	++SurfaceCount;
}

BOOL __cdecl visible_zclip(PHD_VBUF *vtx0, PHD_VBUF *vtx1, PHD_VBUF *vtx2) {
	return ( (vtx0->yv * vtx2->zv - vtx0->zv * vtx2->yv) * vtx1->xv +
			 (vtx0->zv * vtx2->xv - vtx0->xv * vtx2->zv) * vtx1->yv +
			 (vtx0->xv * vtx2->yv - vtx0->yv * vtx2->xv) * vtx1->zv < 0.0 );
}

int __cdecl ZedClipper(int vtxCount, POINT_INFO *pts, VERTEX_INFO *vtx) {
	int i, j, diff0, diff1;
	double clip;
	POINT_INFO *pts0, *pts1;

	if( vtxCount == 0 )
		return 0;

	j = 0;
	pts0 = pts;
	pts1 = &pts[vtxCount-1];

	for( i = 0; i < vtxCount; ++i ) {
		diff0 = (int)(FltNearZ - pts0->zv);
		diff1 = (int)(FltNearZ - pts1->zv);
		if( (diff0|diff1) < 0 ) {
			if( (diff0^diff1) < 0 ) {
				clip = (FltNearZ - pts0->zv) / (pts1->zv - pts0->zv);
				vtx[j].x = ((pts1->xv - pts0->xv) * clip + pts0->xv) * FltPerspONearZ + FltWinCenterX;
				vtx[j].y = ((pts1->yv - pts0->yv) * clip + pts0->yv) * FltPerspONearZ + FltWinCenterY;
				vtx[j].rhw = FltRhwONearZ;
				vtx[j].u = ((pts1->u - pts0->u) * clip + pts0->u) * FltRhwONearZ;
				vtx[j].v = ((pts1->v - pts0->v) * clip + pts0->v) * FltRhwONearZ;
				vtx[j].g = ((pts1->g - pts0->g) * clip + pts0->g);
				++j;
			}
			if( diff0 < 0 ) {
				vtx[j].x = pts0->xs;
				vtx[j].y = pts0->ys;
				vtx[j].rhw = pts0->rhw;
				vtx[j].u = pts0->u * pts0->rhw;
				vtx[j].v = pts0->v * pts0->rhw;
				vtx[j].g = pts0->g;
				++j;
			}
		}
		pts1 = pts0++;
	}

	return ( j < 3 ) ? 0 : j;
}

static inline void clipGUV(VERTEX_INFO *buf, VERTEX_INFO *vtx1, VERTEX_INFO *vtx2, float clip) {
	buf->rhw = vtx2->rhw + (vtx1->rhw - vtx2->rhw) * clip;
	buf->u   = vtx2->u   + (vtx1->u   - vtx2->u)   * clip;
	buf->v   = vtx2->v   + (vtx1->v   - vtx2->v)   * clip;
	buf->g   = vtx2->g   + (vtx1->g   - vtx2->g)   * clip;
}

int __cdecl XYGUVClipper(int vtxCount, VERTEX_INFO *vtx) {
	VERTEX_INFO vtx_buf[8];
	VERTEX_INFO *vtx1, *vtx2;
	float clip;
	int i, j;

	if( vtxCount < 3 )
		return 0;

	// horizontal clip
	vtx2 = &vtx[vtxCount - 1];
	j = 0;
	for( i = 0; i < vtxCount; ++i ) {
		vtx1 = vtx2;
		vtx2 = &vtx[i];

		if( vtx1->x < FltWinLeft ) {
			if( vtx2->x < FltWinLeft ) {
				continue;
			}
			clip = (FltWinLeft - vtx2->x) / (vtx1->x - vtx2->x);
			vtx_buf[j].x = FltWinLeft;
			vtx_buf[j].y = vtx2->y + (vtx1->y - vtx2->y) * clip;
			clipGUV(&vtx_buf[j++], vtx1, vtx2, clip);
		}
		else if( vtx1->x > FltWinRight) {
			if( vtx2->x > FltWinRight ) {
				continue;
			}
			clip = (FltWinRight - vtx2->x) / (vtx1->x - vtx2->x);
			vtx_buf[j].x = FltWinRight;
			vtx_buf[j].y = vtx2->y + (vtx1->y - vtx2->y) * clip;
			clipGUV(&vtx_buf[j++], vtx1, vtx2, clip);
		}

		if( vtx2->x < FltWinLeft ) {
			clip = (FltWinLeft - vtx2->x) / (vtx1->x - vtx2->x);
			vtx_buf[j].x = FltWinLeft;
			vtx_buf[j].y = vtx2->y + (vtx1->y - vtx2->y) * clip;
			clipGUV(&vtx_buf[j++], vtx1, vtx2, clip);
		}
		else if( vtx2->x > FltWinRight ) {
			clip = (FltWinRight - vtx2->x) / (vtx1->x - vtx2->x);
			vtx_buf[j].x = FltWinRight;
			vtx_buf[j].y = vtx2->y + (vtx1->y - vtx2->y) * clip;
			clipGUV(&vtx_buf[j++], vtx1, vtx2, clip);
		} else {
			vtx_buf[j++] = *vtx2;
		}
	}
	vtxCount = j;

	if( vtxCount < 3 )
		return 0;

	// vertical clip
	vtx2 = &vtx_buf[vtxCount-1];
	j = 0;
	for( i = 0; i < vtxCount; ++i ) {
		vtx1 = vtx2;
		vtx2 = &vtx_buf[i];

		if( vtx1->y < FltWinTop ) {
			if( vtx2->y < FltWinTop ) {
				continue;
			}
			clip = (FltWinTop - vtx2->y) / (vtx1->y - vtx2->y);
			vtx[j].x = vtx2->x + (vtx1->x - vtx2->x) * clip;
			vtx[j].y = FltWinTop;
			clipGUV(&vtx[j++], vtx1, vtx2, clip);
		}
		else if( vtx1->y > FltWinBottom ) {
			if( vtx2->y > FltWinBottom ) {
				continue;
			}
			clip = (FltWinBottom - vtx2->y) / (vtx1->y - vtx2->y);
			vtx[j].x = vtx2->x + (vtx1->x - vtx2->x) * clip;
			vtx[j].y = FltWinBottom;
			clipGUV(&vtx[j++], vtx1, vtx2, clip);
		}

		if( vtx2->y < FltWinTop ) {
			clip = (FltWinTop - vtx2->y) / (vtx1->y - vtx2->y);
			vtx[j].x = vtx2->x + (vtx1->x - vtx2->x) * clip;
			vtx[j].y = FltWinTop;
			clipGUV(&vtx[j++], vtx1, vtx2, clip);
		}
		else if( vtx2->y > FltWinBottom ) {
			clip = (FltWinBottom - vtx2->y) / (vtx1->y - vtx2->y);
			vtx[j].x = vtx2->x + (vtx1->x - vtx2->x) * clip;
			vtx[j].y = FltWinBottom;
			clipGUV(&vtx[j++], vtx1, vtx2, clip);
		} else {
			vtx[j++] = *vtx2;
		}
	}
	return ( j < 3 ) ? 0 : j;
}

__int16 *__cdecl InsertObjectGT4(__int16 *ptrObj, int number, SORTTYPE sortType) {
	char clipOR, clipAND;
	PHD_VBUF *vtx0, *vtx1, *vtx2, *vtx3;
	int i, j, nPoints;
	float zv;
	__int16 textureIdx;
	PHD_TEXTURE *texture;
	PHD_UV *uv;
	POINT_INFO points[4];

	for( i = 0; i < number; ++i ) {
		vtx0 = &PhdVBuf[*ptrObj++];
		vtx1 = &PhdVBuf[*ptrObj++];
		vtx2 = &PhdVBuf[*ptrObj++];
		vtx3 = &PhdVBuf[*ptrObj++];
		textureIdx = *ptrObj++;
		texture = &PhdTextureInfo[textureIdx];
		uv = texture->uv;
		nPoints = 4;

		clipOR  = LOBYTE(vtx0->clip | vtx1->clip | vtx2->clip | vtx3->clip);
		clipAND = LOBYTE(vtx0->clip & vtx1->clip & vtx2->clip & vtx3->clip);

		if( clipAND != 0 )
			continue;

		if( clipOR >= 0 ) {
			if( !VBUF_VISIBLE(*vtx0, *vtx1, *vtx2) )
				continue;

			if( clipOR == 0 ) {
				zv = CalculatePolyZ(sortType, vtx0->zv, vtx1->zv, vtx2->zv, vtx3->zv);
				Sort3dPtr->_0 = (DWORD)Info3dPtr;
				Sort3dPtr->_1 = MAKE_ZSORT(zv);
				++Sort3dPtr;

				if( zv >= (double)PerspectiveDistance ) {
					*Info3dPtr++ = ( texture->drawtype == DRAW_Opaque ) ? POLY_GTmap : POLY_WGTmap;
					*Info3dPtr++ = texture->tpage;
					*Info3dPtr++ = 4;

					*Info3dPtr++ = (int)vtx0->xs;
					*Info3dPtr++ = (int)vtx0->ys;
					*Info3dPtr++ = (int)vtx0->g;
					*Info3dPtr++ = uv[0].u;
					*Info3dPtr++ = uv[0].v;

					*Info3dPtr++ = (int)vtx1->xs;
					*Info3dPtr++ = (int)vtx1->ys;
					*Info3dPtr++ = (int)vtx1->g;
					*Info3dPtr++ = uv[1].u;
					*Info3dPtr++ = uv[1].v;

					*Info3dPtr++ = (int)vtx2->xs;
					*Info3dPtr++ = (int)vtx2->ys;
					*Info3dPtr++ = (int)vtx2->g;
					*Info3dPtr++ = uv[2].u;
					*Info3dPtr++ = uv[2].v;

					*Info3dPtr++ = (int)vtx3->xs;
					*Info3dPtr++ = (int)vtx3->ys;
					*Info3dPtr++ = (int)vtx3->g;
					*Info3dPtr++ = uv[3].u;
					*Info3dPtr++ = uv[3].v;
				} else {
					*Info3dPtr++ = ( texture->drawtype == DRAW_Opaque ) ? POLY_GTmap_persp : POLY_WGTmap_persp;
					*Info3dPtr++ = texture->tpage;
					*Info3dPtr++ = 4;

					*Info3dPtr++ = (int)vtx0->xs;
					*Info3dPtr++ = (int)vtx0->ys;
					*Info3dPtr++ = (int)vtx0->g;
					*(float *)Info3dPtr = vtx0->rhw;
					Info3dPtr += sizeof(float)/sizeof(__int16);
					*(float *)Info3dPtr = (double)uv[0].u * vtx0->rhw;
					Info3dPtr += sizeof(float)/sizeof(__int16);
					*(float *)Info3dPtr = (double)uv[0].v * vtx0->rhw;
					Info3dPtr += sizeof(float)/sizeof(__int16);

					*Info3dPtr++ = (int)vtx1->xs;
					*Info3dPtr++ = (int)vtx1->ys;
					*Info3dPtr++ = (int)vtx1->g;
					*(float *)Info3dPtr = vtx1->rhw;
					Info3dPtr += sizeof(float)/sizeof(__int16);
					*(float *)Info3dPtr = (double)uv[1].u * vtx1->rhw;
					Info3dPtr += sizeof(float)/sizeof(__int16);
					*(float *)Info3dPtr = (double)uv[1].v * vtx1->rhw;
					Info3dPtr += sizeof(float)/sizeof(__int16);

					*Info3dPtr++ = (int)vtx2->xs;
					*Info3dPtr++ = (int)vtx2->ys;
					*Info3dPtr++ = (int)vtx2->g;
					*(float *)Info3dPtr = vtx2->rhw;
					Info3dPtr += sizeof(float)/sizeof(__int16);
					*(float *)Info3dPtr = (double)uv[2].u * vtx2->rhw;
					Info3dPtr += sizeof(float)/sizeof(__int16);
					*(float *)Info3dPtr = (double)uv[2].v * vtx2->rhw;
					Info3dPtr += sizeof(float)/sizeof(__int16);

					*Info3dPtr++ = (int)vtx3->xs;
					*Info3dPtr++ = (int)vtx3->ys;
					*Info3dPtr++ = (int)vtx3->g;
					*(float *)Info3dPtr = vtx3->rhw;
					Info3dPtr += sizeof(float)/sizeof(__int16);
					*(float *)Info3dPtr = (double)uv[3].u * vtx3->rhw;
					Info3dPtr += sizeof(float)/sizeof(__int16);
					*(float *)Info3dPtr = (double)uv[3].v * vtx3->rhw;
					Info3dPtr += sizeof(float)/sizeof(__int16);
				}
				++SurfaceCount;
				continue;
			}

			VBuffer[0].x = vtx0->xs;
			VBuffer[0].y = vtx0->ys;
			VBuffer[0].rhw = vtx0->rhw;
			VBuffer[0].g = (float)vtx0->g;
			VBuffer[0].u = (double)uv[0].u * vtx0->rhw;
			VBuffer[0].v = (double)uv[0].v * vtx0->rhw;

			VBuffer[1].x = vtx1->xs;
			VBuffer[1].y = vtx1->ys;
			VBuffer[1].rhw = vtx1->rhw;
			VBuffer[1].g = (float)vtx1->g;
			VBuffer[1].u = (double)uv[1].u * vtx1->rhw;
			VBuffer[1].v = (double)uv[1].v * vtx1->rhw;

			VBuffer[2].x = vtx2->xs;
			VBuffer[2].y = vtx2->ys;
			VBuffer[2].rhw = vtx2->rhw;
			VBuffer[2].g = (float)vtx2->g;
			VBuffer[2].u = (double)uv[2].u * vtx2->rhw;
			VBuffer[2].v = (double)uv[2].v * vtx2->rhw;

			VBuffer[3].x = vtx3->xs;
			VBuffer[3].y = vtx3->ys;
			VBuffer[3].rhw = vtx3->rhw;
			VBuffer[3].g = (float)vtx3->g;
			VBuffer[3].u = (double)uv[3].u * vtx3->rhw;
			VBuffer[3].v = (double)uv[3].v * vtx3->rhw;
		} else {

			if( !visible_zclip(vtx0, vtx1, vtx2) )
				continue;

			points[0].xv	= vtx0->xv;
			points[0].yv	= vtx0->yv;
			points[0].zv	= vtx0->zv;
			points[0].rhw	= vtx0->rhw;
			points[0].xs	= vtx0->xs;
			points[0].ys	= vtx0->ys;
			points[0].u		= (float)uv[0].u;
			points[0].v		= (float)uv[0].v;
			points[0].g		= (float)vtx0->g;

			points[1].yv	= vtx1->yv;
			points[1].xv	= vtx1->xv;
			points[1].zv	= vtx1->zv;
			points[1].rhw	= vtx1->rhw;
			points[1].xs	= vtx1->xs;
			points[1].ys	= vtx1->ys;
			points[1].u		= (float)uv[1].u;
			points[1].v		= (float)uv[1].v;
			points[1].g		= (float)vtx1->g;

			points[2].xv	= vtx2->xv;
			points[2].yv	= vtx2->yv;
			points[2].zv	= vtx2->zv;
			points[2].rhw	= vtx2->rhw;
			points[2].xs	= vtx2->xs;
			points[2].ys	= vtx2->ys;
			points[2].u		= (float)uv[2].u;
			points[2].v		= (float)uv[2].v;
			points[2].g		= (float)vtx2->g;

			points[3].xv	= vtx3->xv;
			points[3].yv	= vtx3->yv;
			points[3].zv	= vtx3->zv;
			points[3].rhw	= vtx3->rhw;
			points[3].xs	= vtx3->xs;
			points[3].ys	= vtx3->ys;
			points[3].u		= (float)uv[3].u;
			points[3].v		= (float)uv[3].v;
			points[3].g		= (float)vtx3->g;

			nPoints = ZedClipper(nPoints, points, VBuffer);
			if( nPoints == 0 ) continue;
		}

		nPoints = XYGUVClipper(nPoints, VBuffer);
		if( nPoints == 0 ) continue;

		zv = CalculatePolyZ(sortType, vtx0->zv, vtx1->zv, vtx2->zv, vtx3->zv);
		Sort3dPtr->_0 = (DWORD)Info3dPtr;
		Sort3dPtr->_1 = MAKE_ZSORT(zv);
		++Sort3dPtr;

		if( zv >= (double)PerspectiveDistance ) {
			*Info3dPtr++ = ( texture->drawtype == DRAW_Opaque ) ? POLY_GTmap : POLY_WGTmap;
			*Info3dPtr++ = texture->tpage;
			*Info3dPtr++ = nPoints;

			for( j = 0; j < nPoints; ++j ) {
				*Info3dPtr++ = (int)VBuffer[j].x;
				*Info3dPtr++ = (int)VBuffer[j].y;
				*Info3dPtr++ = (int)VBuffer[j].g;
				*Info3dPtr++ = (int)(VBuffer[j].u / VBuffer[j].rhw);
				*Info3dPtr++ = (int)(VBuffer[j].v / VBuffer[j].rhw);
			}
		} else {
			*Info3dPtr++ = ( texture->drawtype == DRAW_Opaque ) ? POLY_GTmap_persp : POLY_WGTmap_persp;
			*Info3dPtr++ = texture->tpage;
			*Info3dPtr++ = nPoints;

			for( j = 0; j < nPoints; ++j ) {
				*Info3dPtr++ = (int)VBuffer[j].x;
				*Info3dPtr++ = (int)VBuffer[j].y;
				*Info3dPtr++ = (int)VBuffer[j].g;
				*(float *)Info3dPtr = VBuffer[j].rhw;
				Info3dPtr += sizeof(float)/sizeof(__int16);
				*(float *)Info3dPtr = VBuffer[j].u;
				Info3dPtr += sizeof(float)/sizeof(__int16);
				*(float *)Info3dPtr = VBuffer[j].v;
				Info3dPtr += sizeof(float)/sizeof(__int16);
			}
		}
		++SurfaceCount;
	}

	return ptrObj;
}

__int16 *__cdecl InsertObjectGT3(__int16 *ptrObj, int number, SORTTYPE sortType) {
	char clipOR, clipAND;
	PHD_VBUF *vtx0, *vtx1, *vtx2;
	int i, j, nPoints;
	float zv;
	__int16 textureIdx;
	PHD_TEXTURE *texture;
	PHD_UV *uv;
	POINT_INFO points[3];

	for( i = 0; i < number; ++i ) {
		vtx0 = &PhdVBuf[*ptrObj++];
		vtx1 = &PhdVBuf[*ptrObj++];
		vtx2 = &PhdVBuf[*ptrObj++];
		textureIdx = *ptrObj++;
		texture = &PhdTextureInfo[textureIdx];
		uv = texture->uv;
		nPoints = 3;

		clipOR  = LOBYTE(vtx0->clip | vtx1->clip | vtx2->clip);
		clipAND = LOBYTE(vtx0->clip & vtx1->clip & vtx2->clip);

		if( clipAND != 0 )
			continue;

		if( clipOR >= 0 ) {
			if( !VBUF_VISIBLE(*vtx0, *vtx1, *vtx2) )
				continue;

			if( clipOR == 0 ) {
				zv = CalculatePolyZ(sortType, vtx0->zv, vtx1->zv, vtx2->zv);
				Sort3dPtr->_0 = (DWORD)Info3dPtr;
				Sort3dPtr->_1 = MAKE_ZSORT(zv);
				++Sort3dPtr;

				if( zv >= (double)PerspectiveDistance ) {
					*Info3dPtr++ = ( texture->drawtype == DRAW_Opaque ) ? POLY_GTmap : POLY_WGTmap;
					*Info3dPtr++ = texture->tpage;
					*Info3dPtr++ = 3;

					*Info3dPtr++ = (int)vtx0->xs;
					*Info3dPtr++ = (int)vtx0->ys;
					*Info3dPtr++ = (int)vtx0->g;
					*Info3dPtr++ = uv[0].u;
					*Info3dPtr++ = uv[0].v;

					*Info3dPtr++ = (int)vtx1->xs;
					*Info3dPtr++ = (int)vtx1->ys;
					*Info3dPtr++ = (int)vtx1->g;
					*Info3dPtr++ = uv[1].u;
					*Info3dPtr++ = uv[1].v;

					*Info3dPtr++ = (int)vtx2->xs;
					*Info3dPtr++ = (int)vtx2->ys;
					*Info3dPtr++ = (int)vtx2->g;
					*Info3dPtr++ = uv[2].u;
					*Info3dPtr++ = uv[2].v;
				} else {
					*Info3dPtr++ = ( texture->drawtype == DRAW_Opaque ) ? POLY_GTmap_persp : POLY_WGTmap_persp;
					*Info3dPtr++ = texture->tpage;
					*Info3dPtr++ = 3;

					*Info3dPtr++ = (int)vtx0->xs;
					*Info3dPtr++ = (int)vtx0->ys;
					*Info3dPtr++ = (int)vtx0->g;
					*(float *)Info3dPtr = vtx0->rhw;
					Info3dPtr += sizeof(float)/sizeof(__int16);
					*(float *)Info3dPtr = (double)uv[0].u * vtx0->rhw;
					Info3dPtr += sizeof(float)/sizeof(__int16);
					*(float *)Info3dPtr = (double)uv[0].v * vtx0->rhw;
					Info3dPtr += sizeof(float)/sizeof(__int16);

					*Info3dPtr++ = (int)vtx1->xs;
					*Info3dPtr++ = (int)vtx1->ys;
					*Info3dPtr++ = (int)vtx1->g;
					*(float *)Info3dPtr = vtx1->rhw;
					Info3dPtr += sizeof(float)/sizeof(__int16);
					*(float *)Info3dPtr = (double)uv[1].u * vtx1->rhw;
					Info3dPtr += sizeof(float)/sizeof(__int16);
					*(float *)Info3dPtr = (double)uv[1].v * vtx1->rhw;
					Info3dPtr += sizeof(float)/sizeof(__int16);

					*Info3dPtr++ = (int)vtx2->xs;
					*Info3dPtr++ = (int)vtx2->ys;
					*Info3dPtr++ = (int)vtx2->g;
					*(float *)Info3dPtr = vtx2->rhw;
					Info3dPtr += sizeof(float)/sizeof(__int16);
					*(float *)Info3dPtr = (double)uv[2].u * vtx2->rhw;
					Info3dPtr += sizeof(float)/sizeof(__int16);
					*(float *)Info3dPtr = (double)uv[2].v * vtx2->rhw;
					Info3dPtr += sizeof(float)/sizeof(__int16);
				}
				++SurfaceCount;
				continue;
			}

			VBuffer[0].x = vtx0->xs;
			VBuffer[0].y = vtx0->ys;
			VBuffer[0].rhw = vtx0->rhw;
			VBuffer[0].g = (float)vtx0->g;
			VBuffer[0].u = (double)uv[0].u * vtx0->rhw;
			VBuffer[0].v = (double)uv[0].v * vtx0->rhw;

			VBuffer[1].x = vtx1->xs;
			VBuffer[1].y = vtx1->ys;
			VBuffer[1].rhw = vtx1->rhw;
			VBuffer[1].g = (float)vtx1->g;
			VBuffer[1].u = (double)uv[1].u * vtx1->rhw;
			VBuffer[1].v = (double)uv[1].v * vtx1->rhw;

			VBuffer[2].x = vtx2->xs;
			VBuffer[2].y = vtx2->ys;
			VBuffer[2].rhw = vtx2->rhw;
			VBuffer[2].g = (float)vtx2->g;
			VBuffer[2].u = (double)uv[2].u * vtx2->rhw;
			VBuffer[2].v = (double)uv[2].v * vtx2->rhw;
		} else {

			if( !visible_zclip(vtx0, vtx1, vtx2) )
				continue;

			points[0].xv	= vtx0->xv;
			points[0].yv	= vtx0->yv;
			points[0].zv	= vtx0->zv;
			points[0].rhw	= vtx0->rhw;
			points[0].xs	= vtx0->xs;
			points[0].ys	= vtx0->ys;
			points[0].u		= (float)uv[0].u;
			points[0].v		= (float)uv[0].v;
			points[0].g		= (float)vtx0->g;

			points[1].yv	= vtx1->yv;
			points[1].xv	= vtx1->xv;
			points[1].zv	= vtx1->zv;
			points[1].rhw	= vtx1->rhw;
			points[1].xs	= vtx1->xs;
			points[1].ys	= vtx1->ys;
			points[1].u		= (float)uv[1].u;
			points[1].v		= (float)uv[1].v;
			points[1].g		= (float)vtx1->g;

			points[2].xv	= vtx2->xv;
			points[2].yv	= vtx2->yv;
			points[2].zv	= vtx2->zv;
			points[2].rhw	= vtx2->rhw;
			points[2].xs	= vtx2->xs;
			points[2].ys	= vtx2->ys;
			points[2].u		= (float)uv[2].u;
			points[2].v		= (float)uv[2].v;
			points[2].g		= (float)vtx2->g;

			nPoints = ZedClipper(nPoints, points, VBuffer);
			if( nPoints == 0 ) continue;
		}

		nPoints = XYGUVClipper(nPoints, VBuffer);
		if( nPoints == 0 ) continue;

		zv = CalculatePolyZ(sortType, vtx0->zv, vtx1->zv, vtx2->zv);
		Sort3dPtr->_0 = (DWORD)Info3dPtr;
		Sort3dPtr->_1 = MAKE_ZSORT(zv);
		++Sort3dPtr;

		if( zv >= (double)PerspectiveDistance ) {
			*Info3dPtr++ = ( texture->drawtype == DRAW_Opaque ) ? POLY_GTmap : POLY_WGTmap;
			*Info3dPtr++ = texture->tpage;
			*Info3dPtr++ = nPoints;

			for( j = 0; j < nPoints; ++j ) {
				*Info3dPtr++ = (int)VBuffer[j].x;
				*Info3dPtr++ = (int)VBuffer[j].y;
				*Info3dPtr++ = (int)VBuffer[j].g;
				*Info3dPtr++ = (int)(VBuffer[j].u / VBuffer[j].rhw);
				*Info3dPtr++ = (int)(VBuffer[j].v / VBuffer[j].rhw);
			}
		} else {
			*Info3dPtr++ = ( texture->drawtype == DRAW_Opaque ) ? POLY_GTmap_persp : POLY_WGTmap_persp;
			*Info3dPtr++ = texture->tpage;
			*Info3dPtr++ = nPoints;

			for( j = 0; j < nPoints; ++j ) {
				*Info3dPtr++ = (int)VBuffer[j].x;
				*Info3dPtr++ = (int)VBuffer[j].y;
				*Info3dPtr++ = (int)VBuffer[j].g;
				*(float *)Info3dPtr = VBuffer[j].rhw;
				Info3dPtr += sizeof(float)/sizeof(__int16);
				*(float *)Info3dPtr = VBuffer[j].u;
				Info3dPtr += sizeof(float)/sizeof(__int16);
				*(float *)Info3dPtr = VBuffer[j].v;
				Info3dPtr += sizeof(float)/sizeof(__int16);
			}
		}
		++SurfaceCount;
	}

	return ptrObj;
}

static inline void clipG(VERTEX_INFO *buf, VERTEX_INFO *vtx1, VERTEX_INFO *vtx2, float clip) {
	buf->rhw = vtx2->rhw + (vtx1->rhw - vtx2->rhw) * clip;
	buf->g   = vtx2->g   + (vtx1->g   - vtx2->g)   * clip;
}

int __cdecl XYGClipper(int vtxCount, VERTEX_INFO *vtx) {
	VERTEX_INFO vtx_buf[8];
	VERTEX_INFO *vtx1, *vtx2;
	float clip;
	int i, j;

	if( vtxCount < 3 )
		return 0;

	// horizontal clip
	vtx2 = &vtx[vtxCount - 1];
	j = 0;
	for( i = 0; i < vtxCount; ++i ) {
		vtx1 = vtx2;
		vtx2 = &vtx[i];

		if( vtx1->x < FltWinLeft ) {
			if( vtx2->x < FltWinLeft ) {
				continue;
			}
			clip = (FltWinLeft - vtx2->x) / (vtx1->x - vtx2->x);
			vtx_buf[j].x = FltWinLeft;
			vtx_buf[j].y = vtx2->y + (vtx1->y - vtx2->y) * clip;
			clipG(&vtx_buf[j++], vtx1, vtx2, clip);
		}
		else if( vtx1->x > FltWinRight) {
			if( vtx2->x > FltWinRight ) {
				continue;
			}
			clip = (FltWinRight - vtx2->x) / (vtx1->x - vtx2->x);
			vtx_buf[j].x = FltWinRight;
			vtx_buf[j].y = vtx2->y + (vtx1->y - vtx2->y) * clip;
			clipG(&vtx_buf[j++], vtx1, vtx2, clip);
		}

		if( vtx2->x < FltWinLeft ) {
			clip = (FltWinLeft - vtx2->x) / (vtx1->x - vtx2->x);
			vtx_buf[j].x = FltWinLeft;
			vtx_buf[j].y = vtx2->y + (vtx1->y - vtx2->y) * clip;
			clipG(&vtx_buf[j++], vtx1, vtx2, clip);
		}
		else if( vtx2->x > FltWinRight ) {
			clip = (FltWinRight - vtx2->x) / (vtx1->x - vtx2->x);
			vtx_buf[j].x = FltWinRight;
			vtx_buf[j].y = vtx2->y + (vtx1->y - vtx2->y) * clip;
			clipG(&vtx_buf[j++], vtx1, vtx2, clip);
		} else {
			vtx_buf[j++] = *vtx2;
		}
	}
	vtxCount = j;

	if( vtxCount < 3 )
		return 0;

	// vertical clip
	vtx2 = &vtx_buf[vtxCount-1];
	j = 0;
	for( i = 0; i < vtxCount; ++i ) {
		vtx1 = vtx2;
		vtx2 = &vtx_buf[i];

		if( vtx1->y < FltWinTop ) {
			if( vtx2->y < FltWinTop ) {
				continue;
			}
			clip = (FltWinTop - vtx2->y) / (vtx1->y - vtx2->y);
			vtx[j].x = vtx2->x + (vtx1->x - vtx2->x) * clip;
			vtx[j].y = FltWinTop;
			clipG(&vtx[j++], vtx1, vtx2, clip);
		}
		else if( vtx1->y > FltWinBottom ) {
			if( vtx2->y > FltWinBottom ) {
				continue;
			}
			clip = (FltWinBottom - vtx2->y) / (vtx1->y - vtx2->y);
			vtx[j].x = vtx2->x + (vtx1->x - vtx2->x) * clip;
			vtx[j].y = FltWinBottom;
			clipG(&vtx[j++], vtx1, vtx2, clip);
		}

		if( vtx2->y < FltWinTop ) {
			clip = (FltWinTop - vtx2->y) / (vtx1->y - vtx2->y);
			vtx[j].x = vtx2->x + (vtx1->x - vtx2->x) * clip;
			vtx[j].y = FltWinTop;
			clipG(&vtx[j++], vtx1, vtx2, clip);
		}
		else if( vtx2->y > FltWinBottom ) {
			clip = (FltWinBottom - vtx2->y) / (vtx1->y - vtx2->y);
			vtx[j].x = vtx2->x + (vtx1->x - vtx2->x) * clip;
			vtx[j].y = FltWinBottom;
			clipG(&vtx[j++], vtx1, vtx2, clip);
		} else {
			vtx[j++] = *vtx2;
		}
	}
	return ( j < 3 ) ? 0 : j;
}

__int16 *__cdecl InsertObjectG4(__int16 *ptrObj, int number, SORTTYPE sortType) {
	char clipOR, clipAND;
	PHD_VBUF *vtx0, *vtx1, *vtx2, *vtx3;
	int i, j, nPoints;
	float zv;
	BYTE colorIdx;
	POINT_INFO pts[4];

	for( i = 0; i < number; ++i ) {
		vtx0 = &PhdVBuf[*ptrObj++];
		vtx1 = &PhdVBuf[*ptrObj++];
		vtx2 = &PhdVBuf[*ptrObj++];
		vtx3 = &PhdVBuf[*ptrObj++];
		colorIdx = *ptrObj++;
		nPoints = 4;

		clipOR  = LOBYTE(vtx0->clip | vtx1->clip | vtx2->clip | vtx3->clip);
		clipAND = LOBYTE(vtx0->clip & vtx1->clip & vtx2->clip & vtx3->clip);

		if( clipAND != 0 )
			continue;

		if( clipOR >= 0 ) {
			if( !VBUF_VISIBLE(*vtx0, *vtx1, *vtx2) )
				continue;

			VBuffer[0].x = vtx0->xs;
			VBuffer[0].y = vtx0->ys;
			VBuffer[0].rhw = vtx0->rhw;
			VBuffer[0].g = (float)vtx0->g;

			VBuffer[1].x = vtx1->xs;
			VBuffer[1].y = vtx1->ys;
			VBuffer[1].rhw = vtx1->rhw;
			VBuffer[1].g = (float)vtx1->g;

			VBuffer[2].x = vtx2->xs;
			VBuffer[2].y = vtx2->ys;
			VBuffer[2].rhw = vtx2->rhw;
			VBuffer[2].g = (float)vtx2->g;

			VBuffer[3].x = vtx3->xs;
			VBuffer[3].y = vtx3->ys;
			VBuffer[3].rhw = vtx3->rhw;
			VBuffer[3].g = (float)vtx3->g;

			if( clipOR > 0 ) {
				nPoints = XYGClipper(nPoints, VBuffer);
			}
		} else {
			if( !visible_zclip(vtx0, vtx1, vtx2) )
				continue;

			pts[0].xv = vtx0->xv;
			pts[0].yv = vtx0->yv;
			pts[0].zv = vtx0->zv;
			pts[0].rhw = vtx0->rhw;
			pts[0].xs = vtx0->xs;
			pts[0].ys = vtx0->ys;
			pts[0].g = (float)vtx0->g;

			pts[1].xv = vtx1->xv;
			pts[1].yv = vtx1->yv;
			pts[1].zv = vtx1->zv;
			pts[1].rhw = vtx1->rhw;
			pts[1].xs = vtx1->xs;
			pts[1].ys = vtx1->ys;
			pts[1].g = (float)vtx1->g;

			pts[2].xv = vtx2->xv;
			pts[2].yv = vtx2->yv;
			pts[2].zv = vtx2->zv;
			pts[2].rhw = vtx2->rhw;
			pts[2].xs = vtx2->xs;
			pts[2].ys = vtx2->ys;
			pts[2].g = (float)vtx2->g;

			pts[3].xv = vtx3->xv;
			pts[3].yv = vtx3->yv;
			pts[3].zv = vtx3->zv;
			pts[3].rhw = vtx3->rhw;
			pts[3].xs = vtx3->xs;
			pts[3].ys = vtx3->ys;
			pts[3].g = (float)vtx3->g;

			nPoints = ZedClipper(nPoints, pts, VBuffer);
			if( nPoints == 0 ) continue;

			nPoints = XYGClipper(nPoints, VBuffer);
		}

		if( nPoints == 0 )
			continue;

		zv = CalculatePolyZ(sortType, vtx0->zv, vtx1->zv, vtx2->zv, vtx3->zv);
		Sort3dPtr->_0 = (DWORD)Info3dPtr;
		Sort3dPtr->_1 = MAKE_ZSORT(zv);
		++Sort3dPtr;

		*Info3dPtr++ = POLY_gouraud;
		*Info3dPtr++ = colorIdx;
		*Info3dPtr++ = nPoints;

		for( j = 0; j < nPoints; ++j ) {
			*Info3dPtr++ = VBuffer[j].x;
			*Info3dPtr++ = VBuffer[j].y;
			*Info3dPtr++ = VBuffer[j].g;
		}
		++SurfaceCount;
	}

	return ptrObj;
}

__int16 *__cdecl InsertObjectG3(__int16 *ptrObj, int number, SORTTYPE sortType) {
	char clipOR, clipAND;
	PHD_VBUF *vtx0, *vtx1, *vtx2;
	int i, j, nPoints;
	float zv;
	BYTE colorIdx;
	POINT_INFO pts[3];

	for( i = 0; i < number; ++i ) {
		vtx0 = &PhdVBuf[*ptrObj++];
		vtx1 = &PhdVBuf[*ptrObj++];
		vtx2 = &PhdVBuf[*ptrObj++];
		colorIdx = *ptrObj++;
		nPoints = 3;

		clipOR  = LOBYTE(vtx0->clip | vtx1->clip | vtx2->clip);
		clipAND = LOBYTE(vtx0->clip & vtx1->clip & vtx2->clip);

		if( clipAND != 0 )
			continue;

		if( clipOR >= 0 ) {
			if( !VBUF_VISIBLE(*vtx0, *vtx1, *vtx2) )
				continue;

			VBuffer[0].x = vtx0->xs;
			VBuffer[0].y = vtx0->ys;
			VBuffer[0].rhw = vtx0->rhw;
			VBuffer[0].g = (float)vtx0->g;

			VBuffer[1].x = vtx1->xs;
			VBuffer[1].y = vtx1->ys;
			VBuffer[1].rhw = vtx1->rhw;
			VBuffer[1].g = (float)vtx1->g;

			VBuffer[2].x = vtx2->xs;
			VBuffer[2].y = vtx2->ys;
			VBuffer[2].rhw = vtx2->rhw;
			VBuffer[2].g = (float)vtx2->g;

			if( clipOR > 0 ) {
				nPoints = XYGClipper(nPoints, VBuffer);
			}
		} else {
			if( !visible_zclip(vtx0, vtx1, vtx2) )
				continue;

			pts[0].xv = vtx0->xv;
			pts[0].yv = vtx0->yv;
			pts[0].zv = vtx0->zv;
			pts[0].rhw = vtx0->rhw;
			pts[0].xs = vtx0->xs;
			pts[0].ys = vtx0->ys;
			pts[0].g = (float)vtx0->g;

			pts[1].xv = vtx1->xv;
			pts[1].yv = vtx1->yv;
			pts[1].zv = vtx1->zv;
			pts[1].rhw = vtx1->rhw;
			pts[1].xs = vtx1->xs;
			pts[1].ys = vtx1->ys;
			pts[1].g = (float)vtx1->g;

			pts[2].xv = vtx2->xv;
			pts[2].yv = vtx2->yv;
			pts[2].zv = vtx2->zv;
			pts[2].rhw = vtx2->rhw;
			pts[2].xs = vtx2->xs;
			pts[2].ys = vtx2->ys;
			pts[2].g = (float)vtx2->g;

			nPoints = ZedClipper(nPoints, pts, VBuffer);
			if( nPoints == 0 ) continue;

			nPoints = XYGClipper(nPoints, VBuffer);
		}

		if( nPoints == 0 )
			continue;

		zv = CalculatePolyZ(sortType, vtx0->zv, vtx1->zv, vtx2->zv);
		Sort3dPtr->_0 = (DWORD)Info3dPtr;
		Sort3dPtr->_1 = MAKE_ZSORT(zv);
		++Sort3dPtr;

		*Info3dPtr++ = POLY_gouraud;
		*Info3dPtr++ = colorIdx;
		*Info3dPtr++ = nPoints;

		for( j = 0; j < nPoints; ++j ) {
			*Info3dPtr++ = (int)VBuffer[j].x;
			*Info3dPtr++ = (int)VBuffer[j].y;
			*Info3dPtr++ = (int)VBuffer[j].g;
		}
		++SurfaceCount;
	}

	return ptrObj;
}

int __cdecl XYClipper(int vtxCount, VERTEX_INFO *vtx) {
	// NOTE: the original function ignores rhw clipping that produces bugs for Z Buffer
	static VERTEX_INFO vtx_buf[40]; // NOTE: original size was 20
	VERTEX_INFO *vtx1, *vtx2;
	float clip;
	int i, j;

	if( vtxCount < 3 )
		return 0;

	// horizontal clip
	vtx2 = &vtx[vtxCount - 1];
	j = 0;
	for( i = 0; i < vtxCount; ++i ) {
		vtx1 = vtx2;
		vtx2 = &vtx[i];

		if( vtx1->x < FltWinLeft ) {
			if( vtx2->x < FltWinLeft ) {
				continue;
			}
			clip = (FltWinLeft - vtx2->x) / (vtx1->x - vtx2->x);
			vtx_buf[j].x = FltWinLeft;
			vtx_buf[j].y = vtx2->y + (vtx1->y - vtx2->y) * clip;
			vtx_buf[j++].rhw = vtx2->rhw + (vtx1->rhw - vtx2->rhw) * clip;
		}
		else if( vtx1->x > FltWinRight) {
			if( vtx2->x > FltWinRight ) {
				continue;
			}
			clip = (FltWinRight - vtx2->x) / (vtx1->x - vtx2->x);
			vtx_buf[j].x = FltWinRight;
			vtx_buf[j].y = vtx2->y + (vtx1->y - vtx2->y) * clip;
			vtx_buf[j++].rhw = vtx2->rhw + (vtx1->rhw - vtx2->rhw) * clip;
		}

		if( vtx2->x < FltWinLeft ) {
			clip = (FltWinLeft - vtx2->x) / (vtx1->x - vtx2->x);
			vtx_buf[j].x = FltWinLeft;
			vtx_buf[j].y = vtx2->y + (vtx1->y - vtx2->y) * clip;
			vtx_buf[j++].rhw = vtx2->rhw + (vtx1->rhw - vtx2->rhw) * clip;
		}
		else if( vtx2->x > FltWinRight ) {
			clip = (FltWinRight - vtx2->x) / (vtx1->x - vtx2->x);
			vtx_buf[j].x = FltWinRight;
			vtx_buf[j].y = vtx2->y + (vtx1->y - vtx2->y) * clip;
			vtx_buf[j++].rhw = vtx2->rhw + (vtx1->rhw - vtx2->rhw) * clip;
		} else {
			vtx_buf[j].x = vtx2->x;
			vtx_buf[j].y = vtx2->y;
			vtx_buf[j++].rhw = vtx2->rhw;
		}
	}
	vtxCount = j;

	if( vtxCount < 3 )
		return 0;

	// vertical clip
	vtx2 = &vtx_buf[vtxCount-1];
	j = 0;
	for( i = 0; i < vtxCount; ++i ) {
		vtx1 = vtx2;
		vtx2 = &vtx_buf[i];

		if( vtx1->y < FltWinTop ) {
			if( vtx2->y < FltWinTop ) {
				continue;
			}
			clip = (FltWinTop - vtx2->y) / (vtx1->y - vtx2->y);
			vtx[j].x = vtx2->x + (vtx1->x - vtx2->x) * clip;
			vtx[j].y = FltWinTop;
			vtx[j++].rhw = vtx2->rhw + (vtx1->rhw - vtx2->rhw) * clip;
		}
		else if( vtx1->y > FltWinBottom ) {
			if( vtx2->y > FltWinBottom ) {
				continue;
			}
			clip = (FltWinBottom - vtx2->y) / (vtx1->y - vtx2->y);
			vtx[j].x = vtx2->x + (vtx1->x - vtx2->x) * clip;
			vtx[j].y = FltWinBottom;
			vtx[j++].rhw = vtx2->rhw + (vtx1->rhw - vtx2->rhw) * clip;
		}

		if( vtx2->y < FltWinTop ) {
			clip = (FltWinTop - vtx2->y) / (vtx1->y - vtx2->y);
			vtx[j].x = vtx2->x + (vtx1->x - vtx2->x) * clip;
			vtx[j].y = FltWinTop;
			vtx[j++].rhw = vtx2->rhw + (vtx1->rhw - vtx2->rhw) * clip;
		}
		else if( vtx2->y > FltWinBottom ) {
			clip = (FltWinBottom - vtx2->y) / (vtx1->y - vtx2->y);
			vtx[j].x = vtx2->x + (vtx1->x - vtx2->x) * clip;
			vtx[j].y = FltWinBottom;
			vtx[j++].rhw = vtx2->rhw + (vtx1->rhw - vtx2->rhw) * clip;
		} else {
			vtx[j].x = vtx2->x;
			vtx[j].y = vtx2->y;
			vtx[j++].rhw = vtx2->rhw;
		}
	}

	return ( j < 3 ) ? 0 : j;
}

void __cdecl InsertTrans8(PHD_VBUF *vbuf, __int16 shade) {
	int i, nPoints, polyZ;
	char clipOR = 0x00;
	char clipAND = 0xFF;
#ifdef FEATURE_VIDEOFX_IMPROVED
	int nVtx = ( ShadowMode == 1 ) ? 32 : 8;
#else // FEATURE_VIDEOFX_IMPROVED
	int nVtx = 8;
#endif // FEATURE_VIDEOFX_IMPROVED

	for( i = 0; i < nVtx; ++i ) {
		clipOR  |= LOBYTE(vbuf[i].clip);
		clipAND &= LOBYTE(vbuf[i].clip);
	}

	if( (clipOR < 0) || (clipAND != 0) || !VBUF_VISIBLE(vbuf[0], vbuf[1], vbuf[2]) )
		return;

	for( i = 0; i < nVtx; ++i ) {
		VBuffer[i].x = vbuf[i].xs;
		VBuffer[i].y = vbuf[i].ys;
	}

	nPoints = nVtx;

	if( clipOR != 0 ) {
		FltWinLeft = 0.0;
		FltWinTop  = 0.0;
		FltWinRight  = (float)PhdWinMaxX;
		FltWinBottom = (float)PhdWinMaxY;

		nPoints = XYClipper(nPoints, VBuffer);
		if( nPoints == 0) return;
	}

#ifdef FEATURE_VIDEOFX_IMPROVED
	double polyZflt = 0.0;
	for( i = 0; i < nVtx; ++i ) {
		polyZflt += (double)vbuf[i].zv / (double)nVtx;
	}
	polyZ = polyZflt;
#else // FEATURE_VIDEOFX_IMPROVED
	polyZ = 0;
	for( i = 0; i < nVtx; ++i ) {
		polyZ += vbuf[i].zv;
	}
	polyZ /= nVtx;
#endif // FEATURE_VIDEOFX_IMPROVED

	Sort3dPtr->_0 = (DWORD)Info3dPtr;
	Sort3dPtr->_1 = MAKE_ZSORT(polyZ);
	++Sort3dPtr;

	*(Info3dPtr++) = POLY_trans;
	*(Info3dPtr++) = shade;
	*(Info3dPtr++) = nPoints; // number of vertices

	for( i = 0; i < nPoints; ++i ) {
		*(Info3dPtr++) = (int)VBuffer[i].x;
		*(Info3dPtr++) = (int)VBuffer[i].y;
	}

	++SurfaceCount;
}

void __cdecl InsertTransQuad(int x, int y, int width, int height, int z) {
	Sort3dPtr->_0 = (DWORD)Info3dPtr;
	Sort3dPtr->_1 = MAKE_ZSORT(PhdNearZ + 8*z);
	++Sort3dPtr;

	*(Info3dPtr++) = POLY_trans;
	// NOTE: Here 24 is DepthQ index (shade factor).
	// 0 lightest, 15 no shade, 31 darkest (pitch black).
	// But original code has value 32 supposed to be interpreted as 24 (which means 50% darker)
	// Also 32 is maximum valid value in the original code, though it is DepthQTable range violation.
	// This trick worked because DepthQIndex array was right after DepthQ array in the memory
	// (DepthQIndex is equal to &DepthQ[24].index).This allocation is not guaranteed on some systems, so it was fixed
	*(Info3dPtr++) = 24;
	*(Info3dPtr++) = 4;  // number of vertices
	*(Info3dPtr++) = x;
	*(Info3dPtr++) = y;
	*(Info3dPtr++) = x + width;
	*(Info3dPtr++) = y;
	*(Info3dPtr++) = x + width;
	*(Info3dPtr++) = y + height;
	*(Info3dPtr++) = x;
	*(Info3dPtr++) = y + height;

	++SurfaceCount;
}

void __cdecl InsertFlatRect(int x0, int y0, int x1, int y1, int z, BYTE colorIdx) {
	Sort3dPtr->_0 = (DWORD)Info3dPtr;
	Sort3dPtr->_1 = MAKE_ZSORT(z);
	++Sort3dPtr;

	*(Info3dPtr++) = POLY_flat;
	*(Info3dPtr++) = colorIdx;
	*(Info3dPtr++) = 4; // number of vertices
	*(Info3dPtr++) = x0;
	*(Info3dPtr++) = y0;
	*(Info3dPtr++) = x1;
	*(Info3dPtr++) = y0;
	*(Info3dPtr++) = x1;
	*(Info3dPtr++) = y1;
	*(Info3dPtr++) = x0;
	*(Info3dPtr++) = y1;

	++SurfaceCount;
}

void __cdecl InsertLine(int x0, int y0, int x1, int y1, int z, BYTE colorIdx) {
	Sort3dPtr->_0 = (DWORD)Info3dPtr;
	Sort3dPtr->_1 = MAKE_ZSORT(z);
	++Sort3dPtr;

	*(Info3dPtr++) = POLY_line;
	*(Info3dPtr++) = PhdWinMinX + x0;
	*(Info3dPtr++) = PhdWinMinY + y0;
	*(Info3dPtr++) = PhdWinMinX + x1;
	*(Info3dPtr++) = PhdWinMinY + y1;
	*(Info3dPtr++) = colorIdx;

	++SurfaceCount;
}

void __cdecl InsertGT3_ZBuffered(PHD_VBUF *vtx0, PHD_VBUF *vtx1, PHD_VBUF *vtx2, PHD_TEXTURE *texture, PHD_UV *uv0, PHD_UV *uv1, PHD_UV *uv2) {
	char clipOR, clipAND;
	POINT_INFO points[3];
	int nPoints = 3;

	clipOR  = LOBYTE(vtx0->clip | vtx1->clip | vtx2->clip);
	clipAND = LOBYTE(vtx0->clip & vtx1->clip & vtx2->clip);

	if( clipAND != 0 )
		return;

	if( clipOR >= 0 ) {
		if( !VBUF_VISIBLE(*vtx0, *vtx1, *vtx2) )
			return;

		if( clipOR == 0 ) {
			VBufferD3D[0].sx = vtx0->xs;
			VBufferD3D[0].sy = vtx0->ys;
			VBufferD3D[0].sz = FltResZBuf - FltResZORhw * vtx0->rhw;
			VBufferD3D[0].rhw = vtx0->rhw;
			VBufferD3D[0].color = shadeColor(0xFF, 0xFF, 0xFF, 0xFF, vtx0->g, true);
			VBufferD3D[0].tu = (double)uv0->u / (double)PHD_ONE;
			VBufferD3D[0].tv = (double)uv0->v / (double)PHD_ONE;

			VBufferD3D[1].sx = vtx1->xs;
			VBufferD3D[1].sy = vtx1->ys;
			VBufferD3D[1].sz = FltResZBuf - FltResZORhw * vtx1->rhw;
			VBufferD3D[1].rhw = vtx1->rhw;
			VBufferD3D[1].color = shadeColor(0xFF, 0xFF, 0xFF, 0xFF, vtx1->g, true);
			VBufferD3D[1].tu = (double)uv1->u / (double)PHD_ONE;
			VBufferD3D[1].tv = (double)uv1->v / (double)PHD_ONE;

			VBufferD3D[2].sx = vtx2->xs;
			VBufferD3D[2].sy = vtx2->ys;
			VBufferD3D[2].sz = FltResZBuf - FltResZORhw * vtx2->rhw;
			VBufferD3D[2].rhw = vtx2->rhw;
			VBufferD3D[2].color = shadeColor(0xFF, 0xFF, 0xFF, 0xFF, vtx2->g, true);
			VBufferD3D[2].tu = (double)uv2->u / (double)PHD_ONE;
			VBufferD3D[2].tv = (double)uv2->v / (double)PHD_ONE;

#ifdef FEATURE_VIDEOFX_IMPROVED
			HWR_TexSource(texture->tpage == (UINT16)~0 ? GetEnvmapTextureHandle() : HWR_PageHandles[texture->tpage]);
#else // !FEATURE_VIDEOFX_IMPROVED
			HWR_TexSource(HWR_PageHandles[texture->tpage]);
#endif // !FEATURE_VIDEOFX_IMPROVED
			HWR_EnableColorKey(texture->drawtype != DRAW_Opaque);

			HWR_DrawPrimitive(D3DPT_TRIANGLELIST, VBufferD3D, 3, true);
			return;
		}

		VBuffer[0].x = vtx0->xs;
		VBuffer[0].y = vtx0->ys;
		VBuffer[0].rhw = vtx0->rhw;
		VBuffer[0].g = (double)vtx0->g;
		VBuffer[0].u = (double)uv0->u * vtx0->rhw;
		VBuffer[0].v = (double)uv0->v * vtx0->rhw;

		VBuffer[1].x = vtx1->xs;
		VBuffer[1].y = vtx1->ys;
		VBuffer[1].rhw = vtx1->rhw;
		VBuffer[1].g = (double)vtx1->g;
		VBuffer[1].u = (double)uv1->u * vtx1->rhw;
		VBuffer[1].v = (double)uv1->v * vtx1->rhw;

		VBuffer[2].x = vtx2->xs;
		VBuffer[2].y = vtx2->ys;
		VBuffer[2].rhw = vtx2->rhw;
		VBuffer[2].g = (double)vtx2->g;
		VBuffer[2].u = (double)uv2->u * vtx2->rhw;
		VBuffer[2].v = (double)uv2->v * vtx2->rhw;
	} else {

		if( !visible_zclip(vtx0, vtx1, vtx2) )
			return;

		points[0].xv	= vtx0->xv;
		points[0].yv	= vtx0->yv;
		points[0].zv	= vtx0->zv;
		points[0].rhw	= vtx0->rhw;
		points[0].xs	= vtx0->xs;
		points[0].ys	= vtx0->ys;
		points[0].u		= (float)uv0->u;
		points[0].v		= (float)uv0->v;
		points[0].g		= (float)vtx0->g;

		points[1].yv	= vtx1->yv;
		points[1].xv	= vtx1->xv;
		points[1].zv	= vtx1->zv;
		points[1].rhw	= vtx1->rhw;
		points[1].xs	= vtx1->xs;
		points[1].ys	= vtx1->ys;
		points[1].u		= (float)uv1->u;
		points[1].v		= (float)uv1->v;
		points[1].g		= (float)vtx1->g;

		points[2].xv	= vtx2->xv;
		points[2].yv	= vtx2->yv;
		points[2].zv	= vtx2->zv;
		points[2].rhw	= vtx2->rhw;
		points[2].xs	= vtx2->xs;
		points[2].ys	= vtx2->ys;
		points[2].u		= (float)uv2->u;
		points[2].v		= (float)uv2->v;
		points[2].g		= (float)vtx2->g;

		nPoints = ZedClipper(nPoints, points, VBuffer);
		if( nPoints == 0 ) return;
	}

	nPoints = XYGUVClipper(nPoints, VBuffer);
	if( nPoints == 0 ) return;

#ifdef FEATURE_VIDEOFX_IMPROVED
	HWR_TexSource(texture->tpage == (UINT16)~0 ? GetEnvmapTextureHandle() : HWR_PageHandles[texture->tpage]);
#else // !FEATURE_VIDEOFX_IMPROVED
	HWR_TexSource(HWR_PageHandles[texture->tpage]);
#endif // !FEATURE_VIDEOFX_IMPROVED
	HWR_EnableColorKey(texture->drawtype != DRAW_Opaque);
	DrawClippedPoly_Textured(nPoints);
}

void __cdecl DrawClippedPoly_Textured(int vtxCount) {
	D3DCOLOR color;
	double tu, tv;

	if( vtxCount == 0 )
		return;

	for( int i = 0; i < vtxCount; ++i ) {
		color = shadeColor(0xFF, 0xFF, 0xFF, 0xFF, (DWORD)VBuffer[i].g, true);

		tu = VBuffer[i].u / VBuffer[i].rhw / (double)PHD_ONE;
		tv = VBuffer[i].v / VBuffer[i].rhw / (double)PHD_ONE;
		CLAMP(tu, 0.0, 1.0);
		CLAMP(tv, 0.0, 1.0);

		VBufferD3D[i].sx = VBuffer[i].x;
		VBufferD3D[i].sy = VBuffer[i].y;
		VBufferD3D[i].sz = FltResZBuf - FltResZORhw * VBuffer[i].rhw;
		VBufferD3D[i].rhw = VBuffer[i].rhw;
		VBufferD3D[i].color = color;
		VBufferD3D[i].tu = tu;
		VBufferD3D[i].tv = tv;
	}

	HWR_DrawPrimitive(D3DPT_TRIANGLEFAN, VBufferD3D, vtxCount, true);
}

void __cdecl InsertGT4_ZBuffered(PHD_VBUF *vtx0, PHD_VBUF *vtx1, PHD_VBUF *vtx2, PHD_VBUF *vtx3, PHD_TEXTURE *texture) {
	char clipOR, clipAND;

	clipOR  = LOBYTE(vtx0->clip | vtx1->clip | vtx2->clip | vtx3->clip);
	clipAND = LOBYTE(vtx0->clip & vtx1->clip & vtx2->clip & vtx3->clip);

	if( clipAND != 0 )
		return;

	if( clipOR == 0 && VBUF_VISIBLE(*vtx0, *vtx1, *vtx2) ) {
		VBufferD3D[0].sx = vtx0->xs;
		VBufferD3D[0].sy = vtx0->ys;
		VBufferD3D[0].sz = FltResZBuf - FltResZORhw * vtx0->rhw;
		VBufferD3D[0].rhw = vtx0->rhw;
		VBufferD3D[0].color = shadeColor(0xFF, 0xFF, 0xFF, 0xFF, vtx0->g, true);
		VBufferD3D[0].tu = (double)texture->uv[0].u / (double)PHD_ONE;
		VBufferD3D[0].tv = (double)texture->uv[0].v / (double)PHD_ONE;

		VBufferD3D[1].sx = vtx1->xs;
		VBufferD3D[1].sy = vtx1->ys;
		VBufferD3D[1].sz = FltResZBuf - FltResZORhw * vtx1->rhw;
		VBufferD3D[1].rhw = vtx1->rhw;
		VBufferD3D[1].color = shadeColor(0xFF, 0xFF, 0xFF, 0xFF, vtx1->g, true);
		VBufferD3D[1].tu = (double)texture->uv[1].u / (double)PHD_ONE;
		VBufferD3D[1].tv = (double)texture->uv[1].v / (double)PHD_ONE;

		VBufferD3D[2].sx = vtx2->xs;
		VBufferD3D[2].sy = vtx2->ys;
		VBufferD3D[2].sz = FltResZBuf - FltResZORhw * vtx2->rhw;
		VBufferD3D[2].rhw = vtx2->rhw;
		VBufferD3D[2].color = shadeColor(0xFF, 0xFF, 0xFF, 0xFF, vtx2->g, true);
		VBufferD3D[2].tu = (double)texture->uv[2].u / (double)PHD_ONE;
		VBufferD3D[2].tv = (double)texture->uv[2].v / (double)PHD_ONE;

		VBufferD3D[3].sx = vtx3->xs;
		VBufferD3D[3].sy = vtx3->ys;
		VBufferD3D[3].sz = FltResZBuf - FltResZORhw * vtx3->rhw;
		VBufferD3D[3].rhw = vtx3->rhw;
		VBufferD3D[3].color = shadeColor(0xFF, 0xFF, 0xFF, 0xFF, vtx3->g, true);
		VBufferD3D[3].tu = (double)texture->uv[3].u / (double)PHD_ONE;
		VBufferD3D[3].tv = (double)texture->uv[3].v / (double)PHD_ONE;

#ifdef FEATURE_VIDEOFX_IMPROVED
		HWR_TexSource(texture->tpage == (UINT16)~0 ? GetEnvmapTextureHandle() : HWR_PageHandles[texture->tpage]);
#else // !FEATURE_VIDEOFX_IMPROVED
		HWR_TexSource(HWR_PageHandles[texture->tpage]);
#endif // !FEATURE_VIDEOFX_IMPROVED
		HWR_EnableColorKey(texture->drawtype != DRAW_Opaque);

		HWR_DrawPrimitive(D3DPT_TRIANGLEFAN, VBufferD3D, 4, true);
	}
	else if( (clipOR < 0 && visible_zclip(vtx0, vtx1, vtx2)) ||
			 (clipOR > 0 && VBUF_VISIBLE(*vtx0, *vtx1, *vtx2)) )
	{
		InsertGT3_ZBuffered(vtx0, vtx1, vtx2, texture, texture->uv, &texture->uv[1], &texture->uv[2]);
		InsertGT3_ZBuffered(vtx0, vtx2, vtx3, texture, texture->uv, &texture->uv[2], &texture->uv[3]);
	}
}

__int16 *__cdecl InsertObjectGT4_ZBuffered(__int16 *ptrObj, int number, SORTTYPE sortType) {
	PHD_VBUF *vtx0, *vtx1, *vtx2, *vtx3;
	PHD_TEXTURE *texture;

	for( int i = 0; i < number; ++i ) {
		vtx0 = &PhdVBuf[ptrObj[0]];
		vtx1 = &PhdVBuf[ptrObj[1]];
		vtx2 = &PhdVBuf[ptrObj[2]];
		vtx3 = &PhdVBuf[ptrObj[3]];
		texture = &PhdTextureInfo[ptrObj[4]];

		if( texture->drawtype != DRAW_Opaque )
			InsertGT4_Sorted(vtx0, vtx1, vtx2, vtx3, texture, sortType);
		else
			InsertGT4_ZBuffered(vtx0, vtx1, vtx2, vtx3, texture);

		ptrObj += 5;
	}
	return ptrObj;
}

__int16 *__cdecl InsertObjectGT3_ZBuffered(__int16 *ptrObj, int number, SORTTYPE sortType) {
	PHD_VBUF *vtx0, *vtx1, *vtx2;
	PHD_TEXTURE *texture;
	PHD_UV *uv;

	for( int i = 0; i < number; ++i ) {
		vtx0 = &PhdVBuf[ptrObj[0]];
		vtx1 = &PhdVBuf[ptrObj[1]];
		vtx2 = &PhdVBuf[ptrObj[2]];
		texture = &PhdTextureInfo[ptrObj[3]];
		uv = texture->uv;

		if( texture->drawtype != DRAW_Opaque )
			InsertGT3_Sorted(vtx0, vtx1, vtx2, texture, &uv[0], &uv[1], &uv[2], sortType);
		else
			InsertGT3_ZBuffered(vtx0, vtx1, vtx2, texture, &uv[0], &uv[1], &uv[2]);

		ptrObj += 4;
	}
	return ptrObj;
}

__int16 *__cdecl InsertObjectG4_ZBuffered(__int16 *ptrObj, int number, SORTTYPE sortType) {
	char clipOR, clipAND;
	PHD_VBUF *vtx0, *vtx1, *vtx2, *vtx3;
	int i, nPoints;
	__int16 colorIdx;
	POINT_INFO pts[4];

	HWR_TexSource(0);
	HWR_EnableColorKey(false);

	for( i = 0; i < number; ++i ) {
		vtx0 = &PhdVBuf[*ptrObj++];
		vtx1 = &PhdVBuf[*ptrObj++];
		vtx2 = &PhdVBuf[*ptrObj++];
		vtx3 = &PhdVBuf[*ptrObj++];
		colorIdx = *ptrObj++;
		nPoints = 4;

		clipOR  = LOBYTE(vtx0->clip | vtx1->clip | vtx2->clip | vtx3->clip);
		clipAND = LOBYTE(vtx0->clip & vtx1->clip & vtx2->clip & vtx3->clip);

		if( clipAND != 0 )
			continue;

		if( clipOR >= 0 ) {
			if( !VBUF_VISIBLE(*vtx0, *vtx1, *vtx2) )
				continue;

			VBuffer[0].x = vtx0->xs;
			VBuffer[0].y = vtx0->ys;
			VBuffer[0].rhw = vtx0->rhw;
			VBuffer[0].g = (float)vtx0->g;

			VBuffer[1].x = vtx1->xs;
			VBuffer[1].y = vtx1->ys;
			VBuffer[1].rhw = vtx1->rhw;
			VBuffer[1].g = (float)vtx1->g;

			VBuffer[2].x = vtx2->xs;
			VBuffer[2].y = vtx2->ys;
			VBuffer[2].rhw = vtx2->rhw;
			VBuffer[2].g = (float)vtx2->g;

			VBuffer[3].x = vtx3->xs;
			VBuffer[3].y = vtx3->ys;
			VBuffer[3].rhw = vtx3->rhw;
			VBuffer[3].g = (float)vtx3->g;

			if( clipOR > 0 ) {
				nPoints = XYGClipper(nPoints, VBuffer);
			}
		} else {
			if( !visible_zclip(vtx0, vtx1, vtx2) )
				continue;

			pts[0].xv = vtx0->xv;
			pts[0].yv = vtx0->yv;
			pts[0].zv = vtx0->zv;
			pts[0].rhw = vtx0->rhw;
			pts[0].xs = vtx0->xs;
			pts[0].ys = vtx0->ys;
			pts[0].g = (float)vtx0->g;

			pts[1].xv = vtx1->xv;
			pts[1].yv = vtx1->yv;
			pts[1].zv = vtx1->zv;
			pts[1].rhw = vtx1->rhw;
			pts[1].xs = vtx1->xs;
			pts[1].ys = vtx1->ys;
			pts[1].g = (float)vtx1->g;

			pts[2].xv = vtx2->xv;
			pts[2].yv = vtx2->yv;
			pts[2].zv = vtx2->zv;
			pts[2].rhw = vtx2->rhw;
			pts[2].xs = vtx2->xs;
			pts[2].ys = vtx2->ys;
			pts[2].g = (float)vtx2->g;

			pts[3].xv = vtx3->xv;
			pts[3].yv = vtx3->yv;
			pts[3].zv = vtx3->zv;
			pts[3].rhw = vtx3->rhw;
			pts[3].xs = vtx3->xs;
			pts[3].ys = vtx3->ys;
			pts[3].g = (float)vtx3->g;

			nPoints = ZedClipper(nPoints, pts, VBuffer);
			if( nPoints == 0 ) continue;

			nPoints = XYGClipper(nPoints, VBuffer);
		}

		if( nPoints != 0 ) {
			PALETTEENTRY *color = &GamePalette16[colorIdx >> 8];
#ifdef FEATURE_VIDEOFX_IMPROVED
			if( AlphaBlendMode && color->peFlags > 0 && color->peFlags <= 4 ) {
				float zv = CalculatePolyZ(sortType, vtx0->zv, vtx1->zv, vtx2->zv, vtx3->zv);
				short blend[4] = {POLY_HWR_half, POLY_HWR_add, POLY_HWR_sub, POLY_HWR_qrt};
				InsertPoly_Gouraud(nPoints, zv, color->peRed, color->peGreen, color->peBlue, blend[color->peFlags - 1]);
			} else {
				DrawPoly_Gouraud(nPoints, color->peRed, color->peGreen, color->peBlue);
			}
#else // FEATURE_VIDEOFX_IMPROVED
			DrawPoly_Gouraud(nPoints, color->peRed, color->peGreen, color->peBlue);
#endif // FEATURE_VIDEOFX_IMPROVED
		}
	}

	return ptrObj;
}

void __cdecl DrawPoly_Gouraud(int vtxCount, int red, int green, int blue) {
	D3DCOLOR color;

	if( vtxCount == 0 )
		return;

	for( int i = 0; i < vtxCount; ++i ) {
		color = shadeColor(red, green, blue, 0xFF, (DWORD)VBuffer[i].g, false);

		VBufferD3D[i].sx = VBuffer[i].x;
		VBufferD3D[i].sy = VBuffer[i].y;
		VBufferD3D[i].sz = FltResZBuf - FltResZORhw * VBuffer[i].rhw;
		VBufferD3D[i].rhw = VBuffer[i].rhw;
		VBufferD3D[i].color = color;
	}

	HWR_DrawPrimitive(D3DPT_TRIANGLEFAN, VBufferD3D, vtxCount, true);
}

__int16 *__cdecl InsertObjectG3_ZBuffered(__int16 *ptrObj, int number, SORTTYPE sortType) {
	char clipOR, clipAND;
	PHD_VBUF *vtx0, *vtx1, *vtx2;
	int i, nPoints;
	__int16 colorIdx;
	POINT_INFO pts[3];

	HWR_TexSource(0);
	HWR_EnableColorKey(false);

	for( i = 0; i < number; ++i ) {
		vtx0 = &PhdVBuf[*ptrObj++];
		vtx1 = &PhdVBuf[*ptrObj++];
		vtx2 = &PhdVBuf[*ptrObj++];
		colorIdx = *ptrObj++;
		nPoints = 3;

		clipOR  = LOBYTE(vtx0->clip | vtx1->clip | vtx2->clip);
		clipAND = LOBYTE(vtx0->clip & vtx1->clip & vtx2->clip);

		if( clipAND != 0 )
			continue;

		if( clipOR >= 0 ) {
			if( !VBUF_VISIBLE(*vtx0, *vtx1, *vtx2) )
				continue;

			VBuffer[0].x = vtx0->xs;
			VBuffer[0].y = vtx0->ys;
			VBuffer[0].rhw = vtx0->rhw;
			VBuffer[0].g = (float)vtx0->g;

			VBuffer[1].x = vtx1->xs;
			VBuffer[1].y = vtx1->ys;
			VBuffer[1].rhw = vtx1->rhw;
			VBuffer[1].g = (float)vtx1->g;

			VBuffer[2].x = vtx2->xs;
			VBuffer[2].y = vtx2->ys;
			VBuffer[2].rhw = vtx2->rhw;
			VBuffer[2].g = (float)vtx2->g;

			if( clipOR > 0 ) {
				nPoints = XYGClipper(nPoints, VBuffer);
			}
		} else {
			if( !visible_zclip(vtx0, vtx1, vtx2) )
				continue;

			pts[0].xv = vtx0->xv;
			pts[0].yv = vtx0->yv;
			pts[0].zv = vtx0->zv;
			pts[0].rhw = vtx0->rhw;
			pts[0].xs = vtx0->xs;
			pts[0].ys = vtx0->ys;
			pts[0].g = (float)vtx0->g;

			pts[1].xv = vtx1->xv;
			pts[1].yv = vtx1->yv;
			pts[1].zv = vtx1->zv;
			pts[1].rhw = vtx1->rhw;
			pts[1].xs = vtx1->xs;
			pts[1].ys = vtx1->ys;
			pts[1].g = (float)vtx1->g;

			pts[2].xv = vtx2->xv;
			pts[2].yv = vtx2->yv;
			pts[2].zv = vtx2->zv;
			pts[2].rhw = vtx2->rhw;
			pts[2].xs = vtx2->xs;
			pts[2].ys = vtx2->ys;
			pts[2].g = (float)vtx2->g;

			nPoints = ZedClipper(nPoints, pts, VBuffer);
			if( nPoints == 0 ) continue;

			nPoints = XYGClipper(nPoints, VBuffer);
		}

		if( nPoints != 0 ) {
			PALETTEENTRY *color = &GamePalette16[colorIdx >> 8];
#ifdef FEATURE_VIDEOFX_IMPROVED
			if( AlphaBlendMode && color->peFlags > 0 && color->peFlags <= 4 ) {
				float zv = CalculatePolyZ(sortType, vtx0->zv, vtx1->zv, vtx2->zv);
				short blend[4] = {POLY_HWR_half, POLY_HWR_add, POLY_HWR_sub, POLY_HWR_qrt};
				InsertPoly_Gouraud(nPoints, zv, color->peRed, color->peGreen, color->peBlue, blend[color->peFlags - 1]);
			} else {
				DrawPoly_Gouraud(nPoints, color->peRed, color->peGreen, color->peBlue);
			}
#else // FEATURE_VIDEOFX_IMPROVED
			DrawPoly_Gouraud(nPoints, color->peRed, color->peGreen, color->peBlue);
#endif // FEATURE_VIDEOFX_IMPROVED
		}
	}

	return ptrObj;
}

void __cdecl InsertFlatRect_ZBuffered(int x0, int y0, int x1, int y1, int z, BYTE colorIdx) {
	double sz, rhw;
	D3DCOLOR color;

	if( x0 >= x1 || y0 >= y1 )
		return;

	if( x0 < PhdWinMinX )
		x0 = PhdWinMinX;
	if( y0 < PhdWinMinY )
		y0 = PhdWinMinY;
	if( x1 > PhdWinMinX + PhdWinWidth )
		x1 = PhdWinMinX + PhdWinWidth;
	if( y1 > PhdWinMinY + PhdWinHeight )
		x1 = PhdWinMinY + PhdWinHeight;

	CLAMP(z, PhdNearZ, PhdFarZ);

	color = shadeColor(GamePalette8[colorIdx].red, GamePalette8[colorIdx].green, GamePalette8[colorIdx].blue, 0xFF, 0, false);
	rhw = RhwFactor / (double)z;
	sz = FltResZBuf - rhw * FltResZORhw;

	VBufferD3D[0].sx = (float)x0;
	VBufferD3D[0].sy = (float)y0;
	VBufferD3D[1].sx = (float)x1;
	VBufferD3D[1].sy = (float)y0;
	VBufferD3D[2].sx = (float)x0;
	VBufferD3D[2].sy = (float)y1;
	VBufferD3D[3].sx = (float)x1;
	VBufferD3D[3].sy = (float)y1;

	for( int i=0; i<4; ++i ) {
		VBufferD3D[i].sz = sz;
		VBufferD3D[i].rhw = rhw;
		VBufferD3D[i].color = color;
	}

	HWR_TexSource(0);
	HWR_EnableColorKey(false);
	HWR_DrawPrimitive(D3DPT_TRIANGLESTRIP, VBufferD3D, 4, true);
}

void __cdecl InsertLine_ZBuffered(int x0, int y0, int x1, int y1, int z, BYTE colorIdx) {
	double sz, rhw;
	D3DCOLOR color;

	if( z > PhdFarZ ) {
		return;
	}
	if( z < PhdNearZ ) {
		z = PhdNearZ;
	}

	color = shadeColor(GamePalette8[colorIdx].red, GamePalette8[colorIdx].green, GamePalette8[colorIdx].blue, 0xFF, 0, false);
	rhw = RhwFactor / (double)z;
	sz = FltResZBuf - rhw * FltResZORhw;

	VBufferD3D[0].sx = (float)(PhdWinMinX + x0);
	VBufferD3D[0].sy = (float)(PhdWinMinY + y0);
	VBufferD3D[1].sx = (float)(PhdWinMinX + x1);
	VBufferD3D[1].sy = (float)(PhdWinMinY + y1);

	for( int i=0; i<2; ++i ) {
		VBufferD3D[i].sz = sz;
		VBufferD3D[i].rhw = rhw;
		VBufferD3D[i].color = color;
	}

	HWR_TexSource(0);
	HWR_EnableColorKey(false);
	HWR_DrawPrimitive(D3DPT_LINESTRIP, VBufferD3D, 2, true);
}

void __cdecl InsertGT3_Sorted(PHD_VBUF *vtx0, PHD_VBUF *vtx1, PHD_VBUF *vtx2, PHD_TEXTURE *texture, PHD_UV *uv0, PHD_UV *uv1, PHD_UV *uv2, SORTTYPE sortType) {
	char clipOR, clipAND;
	float zv;
	POINT_INFO points[3];
	int nPoints = 3;

	clipOR  = LOBYTE(vtx0->clip | vtx1->clip | vtx2->clip);
	clipAND = LOBYTE(vtx0->clip & vtx1->clip & vtx2->clip);

	if( clipAND != 0 )
		return;

	if( clipOR >= 0 ) {
		if( !VBUF_VISIBLE(*vtx0, *vtx1, *vtx2) )
			return;

		if( clipOR == 0 ) {
			zv = CalculatePolyZ(sortType, vtx0->zv, vtx1->zv, vtx2->zv);
			Sort3dPtr->_0 = (DWORD)Info3dPtr;
			Sort3dPtr->_1 = MAKE_ZSORT(zv);
			++Sort3dPtr;

#ifdef FEATURE_VIDEOFX_IMPROVED
			*Info3dPtr++ = GetPolyType(texture->drawtype);
#else // FEATURE_VIDEOFX_IMPROVED
			*Info3dPtr++ = ( texture->drawtype == DRAW_Opaque ) ? POLY_HWR_GTmap : POLY_HWR_WGTmap;
#endif // FEATURE_VIDEOFX_IMPROVED
			*Info3dPtr++ = texture->tpage;
			*Info3dPtr++ = 3;
			*(D3DTLVERTEX **)Info3dPtr = HWR_VertexPtr;
			Info3dPtr += sizeof(D3DTLVERTEX *)/sizeof(__int16);

			HWR_VertexPtr[0].sx = vtx0->xs;
			HWR_VertexPtr[0].sy = vtx0->ys;
			HWR_VertexPtr[0].sz = FltResZBuf - FltResZORhw * vtx0->rhw; // NOTE: there was bug because of uninitialized sz and rhw
			HWR_VertexPtr[0].rhw = vtx0->rhw;
			HWR_VertexPtr[0].color = shadeColor(0xFF, 0xFF, 0xFF, 0xFF, vtx0->g, true);
			HWR_VertexPtr[0].tu = (double)uv0->u / (double)PHD_ONE;
			HWR_VertexPtr[0].tv = (double)uv0->v / (double)PHD_ONE;

			HWR_VertexPtr[1].sx = vtx1->xs;
			HWR_VertexPtr[1].sy = vtx1->ys;
			HWR_VertexPtr[1].sz = FltResZBuf - FltResZORhw * vtx1->rhw; // NOTE: there was bug because of uninitialized sz and rhw
			HWR_VertexPtr[1].rhw = vtx1->rhw;
			HWR_VertexPtr[1].color = shadeColor(0xFF, 0xFF, 0xFF, 0xFF, vtx1->g, true);
			HWR_VertexPtr[1].tu = (double)uv1->u / (double)PHD_ONE;
			HWR_VertexPtr[1].tv = (double)uv1->v / (double)PHD_ONE;

			HWR_VertexPtr[2].sx = vtx2->xs;
			HWR_VertexPtr[2].sy = vtx2->ys;
			HWR_VertexPtr[2].sz = FltResZBuf - FltResZORhw * vtx2->rhw; // NOTE: there was bug because of uninitialized sz and rhw
			HWR_VertexPtr[2].rhw = vtx2->rhw;
			HWR_VertexPtr[2].color = shadeColor(0xFF, 0xFF, 0xFF, 0xFF, vtx2->g, true);
			HWR_VertexPtr[2].tu = (double)uv2->u / (double)PHD_ONE;
			HWR_VertexPtr[2].tv = (double)uv2->v / (double)PHD_ONE;

			HWR_VertexPtr += 3;
			++SurfaceCount;
			return;
		}

		VBuffer[0].x = vtx0->xs;
		VBuffer[0].y = vtx0->ys;
		VBuffer[0].rhw = vtx0->rhw;
		VBuffer[0].g = (double)vtx0->g;
		VBuffer[0].u = (double)uv0->u * vtx0->rhw;
		VBuffer[0].v = (double)uv0->v * vtx0->rhw;

		VBuffer[1].x = vtx1->xs;
		VBuffer[1].y = vtx1->ys;
		VBuffer[1].rhw = vtx1->rhw;
		VBuffer[1].g = (double)vtx1->g;
		VBuffer[1].u = (double)uv1->u * vtx1->rhw;
		VBuffer[1].v = (double)uv1->v * vtx1->rhw;

		VBuffer[2].x = vtx2->xs;
		VBuffer[2].y = vtx2->ys;
		VBuffer[2].rhw = vtx2->rhw;
		VBuffer[2].g = (double)vtx2->g;
		VBuffer[2].u = (double)uv2->u * vtx2->rhw;
		VBuffer[2].v = (double)uv2->v * vtx2->rhw;
	} else {

		if( !visible_zclip(vtx0, vtx1, vtx2) )
			return;

		points[0].xv	= vtx0->xv;
		points[0].yv	= vtx0->yv;
		points[0].zv	= vtx0->zv;
		points[0].rhw	= vtx0->rhw;
		points[0].xs	= vtx0->xs;
		points[0].ys	= vtx0->ys;
		points[0].u		= (float)uv0->u;
		points[0].v		= (float)uv0->v;
		points[0].g		= (float)vtx0->g;

		points[1].yv	= vtx1->yv;
		points[1].xv	= vtx1->xv;
		points[1].zv	= vtx1->zv;
		points[1].rhw	= vtx1->rhw;
		points[1].xs	= vtx1->xs;
		points[1].ys	= vtx1->ys;
		points[1].u		= (float)uv1->u;
		points[1].v		= (float)uv1->v;
		points[1].g		= (float)vtx1->g;

		points[2].xv	= vtx2->xv;
		points[2].yv	= vtx2->yv;
		points[2].zv	= vtx2->zv;
		points[2].rhw	= vtx2->rhw;
		points[2].xs	= vtx2->xs;
		points[2].ys	= vtx2->ys;
		points[2].u		= (float)uv2->u;
		points[2].v		= (float)uv2->v;
		points[2].g		= (float)vtx2->g;

		nPoints = ZedClipper(nPoints, points, VBuffer);
		if( nPoints == 0 ) return;
	}

	nPoints = XYGUVClipper(nPoints, VBuffer);
	if( nPoints == 0 ) return;

	zv = CalculatePolyZ(sortType, vtx0->zv, vtx1->zv, vtx2->zv);
#ifdef FEATURE_VIDEOFX_IMPROVED
	InsertClippedPoly_Textured(nPoints, zv, GetPolyType(texture->drawtype), texture->tpage);
#else // FEATURE_VIDEOFX_IMPROVED
	InsertClippedPoly_Textured(nPoints, zv, ( texture->drawtype == DRAW_Opaque ) ? POLY_HWR_GTmap : POLY_HWR_WGTmap, texture->tpage);
#endif // FEATURE_VIDEOFX_IMPROVED
}

void __cdecl InsertClippedPoly_Textured(int vtxCount, float z, __int16 polyType, __int16 texPage) {
	double tu, tv;

	Sort3dPtr->_0 = (DWORD)Info3dPtr;
	Sort3dPtr->_1 = MAKE_ZSORT(z);
	++Sort3dPtr;

	*(Info3dPtr++) = polyType;
	*(Info3dPtr++) = texPage;
	*(Info3dPtr++) = vtxCount;
	*(D3DTLVERTEX **)Info3dPtr = HWR_VertexPtr;
	Info3dPtr += sizeof(D3DTLVERTEX *)/sizeof(__int16);

	for( int i = 0; i < vtxCount; ++i ) {
		tu = VBuffer[i].u / double(PHD_ONE) / VBuffer[i].rhw;
		tv = VBuffer[i].v / double(PHD_ONE) / VBuffer[i].rhw;
		CLAMP(tu, 0.0, 1.0);
		CLAMP(tv, 0.0, 1.0);

		HWR_VertexPtr[i].sx = VBuffer[i].x;
		HWR_VertexPtr[i].sy = VBuffer[i].y;
		HWR_VertexPtr[i].sz = FltResZBuf - FltResZORhw * VBuffer[i].rhw; // NOTE: there was bug because of uninitialized sz and rhw
		HWR_VertexPtr[i].rhw = VBuffer[i].rhw;
		HWR_VertexPtr[i].color = shadeColor(0xFF, 0xFF, 0xFF, 0xFF, VBuffer[i].g, true);
		HWR_VertexPtr[i].tu = tu;
		HWR_VertexPtr[i].tv = tv;
	}

	HWR_VertexPtr += vtxCount;
	++SurfaceCount;
}

void __cdecl InsertGT4_Sorted(PHD_VBUF *vtx0, PHD_VBUF *vtx1, PHD_VBUF *vtx2, PHD_VBUF *vtx3, PHD_TEXTURE *texture, SORTTYPE sortType) {
	char clipOR, clipAND;
	float zv;

	clipOR  = LOBYTE(vtx0->clip | vtx1->clip | vtx2->clip | vtx3->clip);
	clipAND = LOBYTE(vtx0->clip & vtx1->clip & vtx2->clip & vtx3->clip);

	if( clipAND != 0 )
		return;

	if( clipOR == 0 && VBUF_VISIBLE(*vtx0, *vtx1, *vtx2) ) {
		zv = CalculatePolyZ(sortType, vtx0->zv, vtx1->zv, vtx2->zv, vtx3->zv);
		Sort3dPtr->_0 = (DWORD)Info3dPtr;
		Sort3dPtr->_1 = MAKE_ZSORT(zv);
		++Sort3dPtr;
#ifdef FEATURE_VIDEOFX_IMPROVED
		*Info3dPtr++ = GetPolyType(texture->drawtype);
#else // FEATURE_VIDEOFX_IMPROVED
		*Info3dPtr++ = ( texture->drawtype == DRAW_Opaque ) ? POLY_HWR_GTmap : POLY_HWR_WGTmap;
#endif // FEATURE_VIDEOFX_IMPROVED
		*Info3dPtr++ = texture->tpage;
		*Info3dPtr++ = 4;
		*(D3DTLVERTEX **)Info3dPtr = HWR_VertexPtr;
		Info3dPtr += sizeof(D3DTLVERTEX *)/sizeof(__int16);

		HWR_VertexPtr[0].sx = vtx0->xs;
		HWR_VertexPtr[0].sy = vtx0->ys;
		HWR_VertexPtr[0].sz = FltResZBuf - FltResZORhw * vtx0->rhw; // NOTE: there was bug because of uninitialized sz and rhw
		HWR_VertexPtr[0].rhw = vtx0->rhw;
		HWR_VertexPtr[0].color = shadeColor(0xFF, 0xFF, 0xFF, 0xFF, vtx0->g, true);
		HWR_VertexPtr[0].tu = (double)texture->uv[0].u / (double)PHD_ONE;
		HWR_VertexPtr[0].tv = (double)texture->uv[0].v / (double)PHD_ONE;

		HWR_VertexPtr[1].sx = vtx1->xs;
		HWR_VertexPtr[1].sy = vtx1->ys;
		HWR_VertexPtr[1].sz = FltResZBuf - FltResZORhw * vtx1->rhw; // NOTE: there was bug because of uninitialized sz and rhw
		HWR_VertexPtr[1].rhw = vtx1->rhw;
		HWR_VertexPtr[1].color = shadeColor(0xFF, 0xFF, 0xFF, 0xFF, vtx1->g, true);
		HWR_VertexPtr[1].tu = (double)texture->uv[1].u / (double)PHD_ONE;
		HWR_VertexPtr[1].tv = (double)texture->uv[1].v / (double)PHD_ONE;

		HWR_VertexPtr[2].sx = vtx2->xs;
		HWR_VertexPtr[2].sy = vtx2->ys;
		HWR_VertexPtr[2].sz = FltResZBuf - FltResZORhw * vtx2->rhw; // NOTE: there was bug because of uninitialized sz and rhw
		HWR_VertexPtr[2].rhw = vtx2->rhw;
		HWR_VertexPtr[2].color = shadeColor(0xFF, 0xFF, 0xFF, 0xFF, vtx2->g, true);
		HWR_VertexPtr[2].tu = (double)texture->uv[2].u / (double)PHD_ONE;
		HWR_VertexPtr[2].tv = (double)texture->uv[2].v / (double)PHD_ONE;

		HWR_VertexPtr[3].sx = vtx3->xs;
		HWR_VertexPtr[3].sy = vtx3->ys;
		HWR_VertexPtr[3].sz = FltResZBuf - FltResZORhw * vtx3->rhw; // NOTE: there was bug because of uninitialized sz and rhw
		HWR_VertexPtr[3].rhw = vtx3->rhw;
		HWR_VertexPtr[3].color = shadeColor(0xFF, 0xFF, 0xFF, 0xFF, vtx3->g, true);
		HWR_VertexPtr[3].tu = (double)texture->uv[3].u / (double)PHD_ONE;
		HWR_VertexPtr[3].tv = (double)texture->uv[3].v / (double)PHD_ONE;

		HWR_VertexPtr += 4;
		++SurfaceCount;
	}
	else if( (clipOR < 0 && visible_zclip(vtx0, vtx1, vtx2)) ||
			 (clipOR > 0 && VBUF_VISIBLE(*vtx0, *vtx1, *vtx2)) )
	{
		InsertGT3_Sorted(vtx0, vtx1, vtx2, texture, texture->uv, &texture->uv[1], &texture->uv[2], sortType);
		InsertGT3_Sorted(vtx0, vtx2, vtx3, texture, texture->uv, &texture->uv[2], &texture->uv[3], sortType);
	}
}

__int16 *__cdecl InsertObjectGT4_Sorted(__int16 *ptrObj, int number, SORTTYPE sortType) {
	PHD_VBUF *vtx0, *vtx1, *vtx2, *vtx3;
	PHD_TEXTURE *texture;

	for( int i = 0; i < number; ++i ) {
		if( HWR_VertexBufferFull() ) {
			ptrObj += (number - i) * 5;
			break;
		}

		vtx0 = &PhdVBuf[ptrObj[0]];
		vtx1 = &PhdVBuf[ptrObj[1]];
		vtx2 = &PhdVBuf[ptrObj[2]];
		vtx3 = &PhdVBuf[ptrObj[3]];
		texture = &PhdTextureInfo[ptrObj[4]];
		ptrObj += 5;

		InsertGT4_Sorted(vtx0, vtx1, vtx2, vtx3, texture, sortType);
	}
	return ptrObj;
}

__int16 *__cdecl InsertObjectGT3_Sorted(__int16 *ptrObj, int number, SORTTYPE sortType) {
	PHD_VBUF *vtx0, *vtx1, *vtx2;
	PHD_TEXTURE *texture;
	PHD_UV *uv;

	for( int i = 0; i < number; ++i ) {
		if( HWR_VertexBufferFull() ) {
			ptrObj += (number - i) * 4;
			break;
		}

		vtx0 = &PhdVBuf[ptrObj[0]];
		vtx1 = &PhdVBuf[ptrObj[1]];
		vtx2 = &PhdVBuf[ptrObj[2]];
		texture = &PhdTextureInfo[ptrObj[3]];
		uv = texture->uv;
		ptrObj += 4;

		InsertGT3_Sorted(vtx0, vtx1, vtx2, texture, &uv[0], &uv[1], &uv[2], sortType);
	}
	return ptrObj;
}

__int16 *__cdecl InsertObjectG4_Sorted(__int16 *ptrObj, int number, SORTTYPE sortType) {
	char clipOR, clipAND;
	PHD_VBUF *vtx0, *vtx1, *vtx2, *vtx3;
	int i, nPoints;
	float zv;
	__int16 colorIdx;
	PALETTEENTRY *color;
	POINT_INFO pts[4];

	for( i = 0; i < number; ++i ) {
		if( HWR_VertexBufferFull() ) {
			ptrObj += number - i;
			break;
		}

		vtx0 = &PhdVBuf[*ptrObj++];
		vtx1 = &PhdVBuf[*ptrObj++];
		vtx2 = &PhdVBuf[*ptrObj++];
		vtx3 = &PhdVBuf[*ptrObj++];
		colorIdx = *ptrObj++;
		nPoints = 4;

		clipOR  = LOBYTE(vtx0->clip | vtx1->clip | vtx2->clip | vtx3->clip);
		clipAND = LOBYTE(vtx0->clip & vtx1->clip & vtx2->clip & vtx3->clip);

		if( clipAND != 0 )
			continue;

		if( clipOR >= 0 ) {
			if( !VBUF_VISIBLE(*vtx0, *vtx1, *vtx2) )
				continue;

			VBuffer[0].x = vtx0->xs;
			VBuffer[0].y = vtx0->ys;
			VBuffer[0].rhw = vtx0->rhw;
			VBuffer[0].g = (float)vtx0->g;

			VBuffer[1].x = vtx1->xs;
			VBuffer[1].y = vtx1->ys;
			VBuffer[1].rhw = vtx1->rhw;
			VBuffer[1].g = (float)vtx1->g;

			VBuffer[2].x = vtx2->xs;
			VBuffer[2].y = vtx2->ys;
			VBuffer[2].rhw = vtx2->rhw;
			VBuffer[2].g = (float)vtx2->g;

			VBuffer[3].x = vtx3->xs;
			VBuffer[3].y = vtx3->ys;
			VBuffer[3].rhw = vtx3->rhw;
			VBuffer[3].g = (float)vtx3->g;

			if( clipOR > 0 ) {
				nPoints = XYGClipper(nPoints, VBuffer);
			}
		} else {
			if( !visible_zclip(vtx0, vtx1, vtx2) )
				continue;

			pts[0].xv = vtx0->xv;
			pts[0].yv = vtx0->yv;
			pts[0].zv = vtx0->zv;
			pts[0].rhw = vtx0->rhw;
			pts[0].xs = vtx0->xs;
			pts[0].ys = vtx0->ys;
			pts[0].g = (float)vtx0->g;

			pts[1].xv = vtx1->xv;
			pts[1].yv = vtx1->yv;
			pts[1].zv = vtx1->zv;
			pts[1].rhw = vtx1->rhw;
			pts[1].xs = vtx1->xs;
			pts[1].ys = vtx1->ys;
			pts[1].g = (float)vtx1->g;

			pts[2].xv = vtx2->xv;
			pts[2].yv = vtx2->yv;
			pts[2].zv = vtx2->zv;
			pts[2].rhw = vtx2->rhw;
			pts[2].xs = vtx2->xs;
			pts[2].ys = vtx2->ys;
			pts[2].g = (float)vtx2->g;

			pts[3].xv = vtx3->xv;
			pts[3].yv = vtx3->yv;
			pts[3].zv = vtx3->zv;
			pts[3].rhw = vtx3->rhw;
			pts[3].xs = vtx3->xs;
			pts[3].ys = vtx3->ys;
			pts[3].g = (float)vtx3->g;

			nPoints = ZedClipper(nPoints, pts, VBuffer);
			if( nPoints == 0 ) continue;

			nPoints = XYGClipper(nPoints, VBuffer);
		}

		if( nPoints == 0 )
			continue;

		color = &GamePalette16[colorIdx >> 8];
		zv = CalculatePolyZ(sortType, vtx0->zv, vtx1->zv, vtx2->zv, vtx3->zv);
#ifdef FEATURE_VIDEOFX_IMPROVED
		if( AlphaBlendMode && color->peFlags > 0 && color->peFlags <= 4 ) {
			short blend[4] = {POLY_HWR_half, POLY_HWR_add, POLY_HWR_sub, POLY_HWR_qrt};
			InsertPoly_Gouraud(nPoints, zv, color->peRed, color->peGreen, color->peBlue, blend[color->peFlags - 1]);
		} else {
			InsertPoly_Gouraud(nPoints, zv, color->peRed, color->peGreen, color->peBlue, POLY_HWR_gouraud);
		}
#else // FEATURE_VIDEOFX_IMPROVED
		InsertPoly_Gouraud(nPoints, zv, color->peRed, color->peGreen, color->peBlue, POLY_HWR_gouraud);
#endif // FEATURE_VIDEOFX_IMPROVED
	}

	return ptrObj;
}

void __cdecl InsertPoly_Gouraud(int vtxCount, float z, int red, int green, int blue, __int16 polyType) {
	BYTE alpha = ( polyType == POLY_HWR_trans ) ? 0x80 : 0xFF;

	Sort3dPtr->_0 = (DWORD)Info3dPtr;
	Sort3dPtr->_1 = MAKE_ZSORT(z);
	++Sort3dPtr;

	*(Info3dPtr++) = polyType;
	*(Info3dPtr++) = vtxCount;
	*(D3DTLVERTEX **)Info3dPtr = HWR_VertexPtr;
	Info3dPtr += sizeof(D3DTLVERTEX *)/sizeof(__int16);

	for( int i = 0; i < vtxCount; ++i ) {
		HWR_VertexPtr[i].sx = VBuffer[i].x;
		HWR_VertexPtr[i].sy = VBuffer[i].y;
		HWR_VertexPtr[i].sz = FltResZBuf - FltResZORhw * VBuffer[i].rhw; // NOTE: there was bug because of uninitialized sz and rhw
		HWR_VertexPtr[i].rhw = VBuffer[i].rhw;
		HWR_VertexPtr[i].color = shadeColor(red, green, blue, alpha, VBuffer[i].g, false);
	}

	HWR_VertexPtr += vtxCount;
	++SurfaceCount;
}

__int16 *__cdecl InsertObjectG3_Sorted(__int16 *ptrObj, int number, SORTTYPE sortType) {
	char clipOR, clipAND;
	PHD_VBUF *vtx0, *vtx1, *vtx2;
	int i, nPoints;
	float zv;
	__int16 colorIdx;
	PALETTEENTRY *color;
	POINT_INFO pts[3];

	for( i = 0; i < number; ++i ) {
		if( HWR_VertexBufferFull() ) {
			ptrObj += number - i;
			break;
		}

		vtx0 = &PhdVBuf[*ptrObj++];
		vtx1 = &PhdVBuf[*ptrObj++];
		vtx2 = &PhdVBuf[*ptrObj++];
		colorIdx = *ptrObj++;
		nPoints = 3;

		clipOR  = LOBYTE(vtx0->clip | vtx1->clip | vtx2->clip);
		clipAND = LOBYTE(vtx0->clip & vtx1->clip & vtx2->clip);

		if( clipAND != 0 )
			continue;

		if( clipOR >= 0 ) {
			if( !VBUF_VISIBLE(*vtx0, *vtx1, *vtx2) )
				continue;

			VBuffer[0].x = vtx0->xs;
			VBuffer[0].y = vtx0->ys;
			VBuffer[0].rhw = vtx0->rhw;
			VBuffer[0].g = (float)vtx0->g;

			VBuffer[1].x = vtx1->xs;
			VBuffer[1].y = vtx1->ys;
			VBuffer[1].rhw = vtx1->rhw;
			VBuffer[1].g = (float)vtx1->g;

			VBuffer[2].x = vtx2->xs;
			VBuffer[2].y = vtx2->ys;
			VBuffer[2].rhw = vtx2->rhw;
			VBuffer[2].g = (float)vtx2->g;

			if( clipOR > 0 ) {
				nPoints = XYGClipper(nPoints, VBuffer);
			}
		} else {
			if( !visible_zclip(vtx0, vtx1, vtx2) )
				continue;

			pts[0].xv = vtx0->xv;
			pts[0].yv = vtx0->yv;
			pts[0].zv = vtx0->zv;
			pts[0].rhw = vtx0->rhw;
			pts[0].xs = vtx0->xs;
			pts[0].ys = vtx0->ys;
			pts[0].g = (float)vtx0->g;

			pts[1].xv = vtx1->xv;
			pts[1].yv = vtx1->yv;
			pts[1].zv = vtx1->zv;
			pts[1].rhw = vtx1->rhw;
			pts[1].xs = vtx1->xs;
			pts[1].ys = vtx1->ys;
			pts[1].g = (float)vtx1->g;

			pts[2].xv = vtx2->xv;
			pts[2].yv = vtx2->yv;
			pts[2].zv = vtx2->zv;
			pts[2].rhw = vtx2->rhw;
			pts[2].xs = vtx2->xs;
			pts[2].ys = vtx2->ys;
			pts[2].g = (float)vtx2->g;

			nPoints = ZedClipper(nPoints, pts, VBuffer);
			if( nPoints == 0 ) continue;

			nPoints = XYGClipper(nPoints, VBuffer);
		}

		if( nPoints == 0 )
			continue;

		color = &GamePalette16[colorIdx >> 8];
		zv = CalculatePolyZ(sortType, vtx0->zv, vtx1->zv, vtx2->zv);
#ifdef FEATURE_VIDEOFX_IMPROVED
		if( AlphaBlendMode && color->peFlags > 0 && color->peFlags <= 4 ) {
			short blend[4] = {POLY_HWR_half, POLY_HWR_add, POLY_HWR_sub, POLY_HWR_qrt};
			InsertPoly_Gouraud(nPoints, zv, color->peRed, color->peGreen, color->peBlue, blend[color->peFlags - 1]);
		} else {
			InsertPoly_Gouraud(nPoints, zv, color->peRed, color->peGreen, color->peBlue, POLY_HWR_gouraud);
		}
#else // FEATURE_VIDEOFX_IMPROVED
		InsertPoly_Gouraud(nPoints, zv, color->peRed, color->peGreen, color->peBlue, POLY_HWR_gouraud);
#endif // FEATURE_VIDEOFX_IMPROVED
	}

	return ptrObj;
}

#ifdef FEATURE_VIDEOFX_IMPROVED
void __cdecl InsertSprite_Sorted(int z, int x0, int y0, int x1, int y1, int spriteIdx, __int16 shade, DWORD flags) {
	if( TextureFormat.bpp < 16 && CHK_ANY(flags, SPR_TINT) ) return; // tinted sprites are not supported for 8 bit textured mode
#else // FEATURE_VIDEOFX_IMPROVED
void __cdecl InsertSprite_Sorted(int z, int x0, int y0, int x1, int y1, int spriteIdx, __int16 shade) {
#endif // FEATURE_VIDEOFX_IMPROVED
	double rhw, u0, v0, u1, v1;
	int uOffset, vOffset, nPoints;

	if( HWR_VertexBufferFull() || x0 >= x1 || y0 >= y1 || x1 <= 0 || y1 <= 0  || x0 >= PhdWinMaxX || y0 >= PhdWinMaxY || z >= PhdFarZ )
		return;

	x0 += PhdWinMinX;
	y0 += PhdWinMinY;
	x1 += PhdWinMinX;
	y1 += PhdWinMinY;

	if( z < PhdNearZ )
		z = PhdNearZ;

	rhw = RhwFactor / (double)z;

	uOffset = LOBYTE(PhdSpriteInfo[spriteIdx].offset) * 256;
	vOffset = HIBYTE(PhdSpriteInfo[spriteIdx].offset) * 256;

	// NOTE: page side is not counted in the original game, but we need it for HD textures
	int adjustment = UvAdd * 256 / GetTextureSideByPage(PhdSpriteInfo[spriteIdx].texPage);
#if (DIRECT3D_VERSION >= 0x900)
	double forcedAdjust = GetTexPagesAdjustment();
	if( forcedAdjust > 0.0) {
		adjustment = (int)(forcedAdjust * 256.0);
	}
#endif // (DIRECT3D_VERSION >= 0x900)
#ifdef FEATURE_HUD_IMPROVED
	if( spriteIdx >= (int)ARRAY_SIZE(PhdSpriteInfo) - HUD_SPRITE_RESERVED ) {
		adjustment = 0;
	}
#endif // FEATURE_HUD_IMPROVED
	CLAMPL(adjustment, 1);

	u0 = rhw * (double)(uOffset - adjustment + PhdSpriteInfo[spriteIdx].width);
	v0 = rhw * (double)(vOffset + adjustment);

	u1 = rhw * (double)(uOffset + adjustment);
	v1 = rhw * (double)(vOffset - adjustment + PhdSpriteInfo[spriteIdx].height);

	VBuffer[0].x = (float)x0;
	VBuffer[0].y = (float)y0;
	VBuffer[0].u = u1;
	VBuffer[0].v = v0;
	VBuffer[1].x = (float)x1;
	VBuffer[1].y = (float)y0;
	VBuffer[1].u = u0;
	VBuffer[1].v = v0;
	VBuffer[2].x = (float)x1;
	VBuffer[2].y = (float)y1;
	VBuffer[2].u = u0;
	VBuffer[2].v = v1;
	VBuffer[3].x = (float)x0;
	VBuffer[3].y = (float)y1;
	VBuffer[3].u = u1;
	VBuffer[3].v = v1;

	for( int i=0; i<4; ++i ) {
		VBuffer[i].rhw = rhw;
		VBuffer[i].g = (float)shade;
	}

	nPoints = 4;

	if( x0 < PhdWinMinX || y0 < PhdWinMinY || x1 > PhdWinWidth + PhdWinMinX || y1 > PhdWinHeight + PhdWinMinY ) {
		FltWinLeft = (float)PhdWinMinX;
		FltWinTop  = (float)PhdWinMinY;
		FltWinRight  = (float)(PhdWinMinX + PhdWinWidth);
		FltWinBottom = (float)(PhdWinMinY + PhdWinHeight);
		nPoints = XYGUVClipper(nPoints, VBuffer);
		if( nPoints == 0 ) return;
	}

	bool isShadeEffectBackup = IsShadeEffect;
	IsShadeEffect = false;
#ifdef FEATURE_VIDEOFX_IMPROVED
	short polyType = POLY_HWR_WGTmap;
	if( CHK_ANY(flags, SPR_TINT) ) {
		GlobalTint = RGBA_SETALPHA(flags, 0xFF);
	}
	if( AlphaBlendMode && CHK_ANY(flags, SPR_SEMITRANS) ) {
		short blend[4] = {
			POLY_HWR_WGTmapHalf,
			POLY_HWR_WGTmapAdd,
			POLY_HWR_WGTmapSub,
			POLY_HWR_WGTmapQrt,
		};
		polyType = blend[(flags & SPR_BLEND) >> 29];
	}
	InsertClippedPoly_Textured(nPoints, (float)z, polyType, PhdSpriteInfo[spriteIdx].texPage);
	GlobalTint = 0;
#else // FEATURE_VIDEOFX_IMPROVED
	InsertClippedPoly_Textured(nPoints, (float)z, POLY_HWR_WGTmap, PhdSpriteInfo[spriteIdx].texPage);
#endif // FEATURE_VIDEOFX_IMPROVED
	IsShadeEffect = isShadeEffectBackup;
}

void __cdecl InsertFlatRect_Sorted(int x0, int y0, int x1, int y1, int z, BYTE colorIdx) {
	double rhw, sz;
	D3DCOLOR color;

	if( x0 >= x1 || y0 >= y1 )
		return;

	if( x0 < PhdWinMinX )
		x0 = PhdWinMinX;
	if( y0 < PhdWinMinY )
		y0 = PhdWinMinY;
	if( x1 > PhdWinMinX + PhdWinWidth )
		x1 = PhdWinMinX + PhdWinWidth;
	if( y1 > PhdWinMinY + PhdWinHeight )
		x1 = PhdWinMinY + PhdWinHeight;

	Sort3dPtr->_0 = (DWORD)Info3dPtr;
	Sort3dPtr->_1 = MAKE_ZSORT(z);
	++Sort3dPtr;

	*(Info3dPtr++) = POLY_HWR_gouraud;
	*(Info3dPtr++) = 4; //  vertex count
	*(D3DTLVERTEX **)Info3dPtr = HWR_VertexPtr;
	Info3dPtr += sizeof(D3DTLVERTEX *)/sizeof(__int16);

	color = shadeColor(GamePalette8[colorIdx].red, GamePalette8[colorIdx].green, GamePalette8[colorIdx].blue, 0xFF, 0, false);
	rhw = RhwFactor / (double)z;
	sz = FltResZBuf - rhw * FltResZORhw;

	HWR_VertexPtr[0].sx = (float)x0;
	HWR_VertexPtr[0].sy = (float)y0;
	HWR_VertexPtr[1].sx = (float)x1;
	HWR_VertexPtr[1].sy = (float)y0;
	HWR_VertexPtr[2].sx = (float)x1;
	HWR_VertexPtr[2].sy = (float)y1;
	HWR_VertexPtr[3].sx = (float)x0;
	HWR_VertexPtr[3].sy = (float)y1;

	for( int i=0; i<4; ++i ) {
		HWR_VertexPtr[i].color = color;
		HWR_VertexPtr[i].sz = sz; // NOTE: there was bug because of uninitialized sz and rhw
		HWR_VertexPtr[i].rhw = rhw;
	}

	HWR_VertexPtr += 4;
	++SurfaceCount;
}

void __cdecl InsertLine_Sorted(int x0, int y0, int x1, int y1, int z, BYTE colorIdx) {
	double rhw, sz;
	D3DCOLOR color;

	Sort3dPtr->_0 = (DWORD)Info3dPtr;
	Sort3dPtr->_1 = MAKE_ZSORT(z);
	++Sort3dPtr;

	*(Info3dPtr++) = POLY_HWR_line;
	*(Info3dPtr++) = 2; //  vertex count
	*(D3DTLVERTEX **)Info3dPtr = HWR_VertexPtr;
	Info3dPtr += sizeof(D3DTLVERTEX *)/sizeof(__int16);

	color = shadeColor(GamePalette8[colorIdx].red, GamePalette8[colorIdx].green, GamePalette8[colorIdx].blue, 0xFF, 0, false);
	rhw = RhwFactor / (double)z;
	sz = FltResZBuf - rhw * FltResZORhw;

	HWR_VertexPtr[0].sx = (float)(PhdWinMinX + x0);
	HWR_VertexPtr[0].sy = (float)(PhdWinMinY + y0);
	HWR_VertexPtr[1].sx = (float)(PhdWinMinX + x1);
	HWR_VertexPtr[1].sy = (float)(PhdWinMinY + y1);

	for( int i=0; i<2; ++i ) {
		HWR_VertexPtr[i].color = color;
		HWR_VertexPtr[i].sz = sz; // NOTE: there was bug because of uninitialized sz and rhw
		HWR_VertexPtr[i].rhw = rhw;
	}

	HWR_VertexPtr += 2;
	++SurfaceCount;
}

void __cdecl InsertTrans8_Sorted(PHD_VBUF *vbuf, __int16 shade) {
	int i, nPoints, polyZ;
	char clipOR = 0x00;
	char clipAND = 0xFF;
#ifdef FEATURE_VIDEOFX_IMPROVED
	int nVtx = ( ShadowMode == 1 ) ? 32 : 8;
#else // FEATURE_VIDEOFX_IMPROVED
	int nVtx = 8;
#endif // FEATURE_VIDEOFX_IMPROVED

	for( i = 0; i < nVtx; ++i ) {
		clipOR  |= LOBYTE(vbuf[i].clip);
		clipAND &= LOBYTE(vbuf[i].clip);
	}

	if( (clipOR < 0) || (clipAND != 0) || !VBUF_VISIBLE(vbuf[0], vbuf[1], vbuf[2]) )
		return;

	for( i = 0; i < nVtx; ++i ) {
		VBuffer[i].x = vbuf[i].xs;
		VBuffer[i].y = vbuf[i].ys;
		VBuffer[i].rhw = RhwFactor / (double)(vbuf[i].zv - 0x20000);
	}

	nPoints = nVtx;

	if( clipOR != 0 ) {
		FltWinLeft = 0.0;
		FltWinTop = 0.0;
		FltWinRight = (float)PhdWinMaxX;
		FltWinBottom = (float)PhdWinMaxY;

		nPoints = XYClipper(nPoints, VBuffer);
		if( nPoints == 0) return;
	}

#ifdef FEATURE_VIDEOFX_IMPROVED
	double polyZflt = 0.0;
	for( i = 0; i < nVtx; ++i ) {
		polyZflt += (double)vbuf[i].zv / (double)nVtx;
	}
	polyZ = polyZflt;
#else // FEATURE_VIDEOFX_IMPROVED
	polyZ = 0;
	for( i = 0; i < nVtx; ++i ) {
		polyZ += vbuf[i].zv;
	}
	polyZ /= nVtx;
#endif // FEATURE_VIDEOFX_IMPROVED

	InsertPoly_Gouraud(nPoints, (float)(polyZ - 0x20000), 0, 0, 0, POLY_HWR_trans);
}

void __cdecl InsertTransQuad_Sorted(int x, int y, int width, int height, int z) {
	float x0, y0, x1, y1;
	double rhw, sz;

	Sort3dPtr->_0 = (DWORD)Info3dPtr;
	Sort3dPtr->_1 = MAKE_ZSORT(z);
	++Sort3dPtr;

	*(Info3dPtr++) = POLY_HWR_trans;
	*(Info3dPtr++) = 4; //  vertex count
	*(D3DTLVERTEX **)Info3dPtr = HWR_VertexPtr;
	Info3dPtr += sizeof(D3DTLVERTEX *)/sizeof(__int16);

	x0 = (float)x;
	y0 = (float)y;
	x1 = (float)(x + width);
	y1 = (float)(y + height);

	rhw = RhwFactor / (double)z;
	sz = FltResZBuf - rhw * FltResZORhw;

	HWR_VertexPtr[0].sx = x0;
	HWR_VertexPtr[0].sy = y0;
	HWR_VertexPtr[1].sx = x1;
	HWR_VertexPtr[1].sy = y0;
	HWR_VertexPtr[2].sx = x1;
	HWR_VertexPtr[2].sy = y1;
	HWR_VertexPtr[3].sx = x0;
	HWR_VertexPtr[3].sy = y1;

	for( int i=0; i<4; ++i ) {
		HWR_VertexPtr[i].color = 0x80000000; // half transparent black
		HWR_VertexPtr[i].sz = sz; // NOTE: there was bug because of uninitialized sz and rhw
		HWR_VertexPtr[i].rhw = rhw;
	}

	HWR_VertexPtr += 4;
	++SurfaceCount;
}

#ifdef FEATURE_VIDEOFX_IMPROVED
void __cdecl InsertSprite(int z, int x0, int y0, int x1, int y1, int spriteIdx, __int16 shade, DWORD flags) {
	if( CHK_ANY(flags, SPR_TINT) ) return; // tinted sprites are not supported by software renderer yet
#else // FEATURE_VIDEOFX_IMPROVED
void __cdecl InsertSprite(int z, int x0, int y0, int x1, int y1, int spriteIdx, __int16 shade) {
#endif // FEATURE_VIDEOFX_IMPROVED
	Sort3dPtr->_0 = (DWORD)Info3dPtr;
	Sort3dPtr->_1 = MAKE_ZSORT(z);
	++Sort3dPtr;

	*(Info3dPtr++) = POLY_sprite;
	*(Info3dPtr++) = x0;
	*(Info3dPtr++) = y0;
	*(Info3dPtr++) = x1;
	*(Info3dPtr++) = y1;
	*(Info3dPtr++) = spriteIdx;
	*(Info3dPtr++) = shade;

	++SurfaceCount;
}

/*
 * Inject function
 */
void Inject_3Dinsert() {
	INJECT(0x00405840, visible_zclip);
	INJECT(0x004058B0, ZedClipper);
	INJECT(0x004059F0, XYGUVClipper);
	INJECT(0x00405F10, InsertObjectGT4);
	INJECT(0x00406970, InsertObjectGT3);
	INJECT(0x004071F0, XYGClipper);
	INJECT(0x00407620, InsertObjectG4);
	INJECT(0x00407A00, InsertObjectG3);
	INJECT(0x00407D20, XYClipper);
	INJECT(0x00407FF0, InsertTrans8);
	INJECT(0x004084A0, InsertTransQuad);
	INJECT(0x00408580, InsertFlatRect);
	INJECT(0x00408650, InsertLine);
	INJECT(0x00408710, InsertGT3_ZBuffered);
	INJECT(0x00408D60, DrawClippedPoly_Textured);
	INJECT(0x00408EA0, InsertGT4_ZBuffered);
	INJECT(0x004092E0, InsertObjectGT4_ZBuffered);
	INJECT(0x00409380, InsertObjectGT3_ZBuffered);
	INJECT(0x00409430, InsertObjectG4_ZBuffered);
	INJECT(0x004097D0, DrawPoly_Gouraud);
	INJECT(0x004098D0, InsertObjectG3_ZBuffered);
	INJECT(0x00409BB0, InsertFlatRect_ZBuffered);
	INJECT(0x00409D80, InsertLine_ZBuffered);
	INJECT(0x00409EC0, InsertGT3_Sorted);
	INJECT(0x0040A5D0, InsertClippedPoly_Textured);
	INJECT(0x0040A780, InsertGT4_Sorted);
	INJECT(0x0040AC60, InsertObjectGT4_Sorted);
	INJECT(0x0040ACF0, InsertObjectGT3_Sorted);
	INJECT(0x0040AD90, InsertObjectG4_Sorted);
	INJECT(0x0040B1D0, InsertPoly_Gouraud);
	INJECT(0x0040B350, InsertObjectG3_Sorted);
	INJECT(0x0040B6A0, InsertSprite_Sorted);
	INJECT(0x0040B9F0, InsertFlatRect_Sorted);
	INJECT(0x0040BB70, InsertLine_Sorted);
	INJECT(0x0040BCA0, InsertTrans8_Sorted);
	INJECT(0x0040BE40, InsertTransQuad_Sorted);
	INJECT(0x0040BF80, InsertSprite);
}
