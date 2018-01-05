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
#include "3dsystem/3dinsert.h"
#include "specific/hwr.h"
#include "global/vars.h"

static D3DCOLOR shadeColor(DWORD red, DWORD green, DWORD blue, DWORD alpha, DWORD shade) {
	CLAMPG(shade, 0x1FFF);

	if( shade > 0 ) {
		DWORD brightness = 0x1FFF - shade;
		red   = red   * brightness >> 12;
		green = green * brightness >> 12;
		blue  = blue  * brightness >> 12;
	}

	CLAMPG(red,   0xFF);
	CLAMPG(green, 0xFF);
	CLAMPG(blue,  0xFF);
	CLAMPG(alpha, 0xFF);

	if( IsShadeEffect ) {
		red   = red   * 128 / 256;
		green = green * 224 / 256;
	}
	return RGBA_MAKE(red, green, blue, alpha);
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

void __cdecl InsertTrans8(PHD_VBUF *vbuf, __int16 shade) {
	int i, nPoints, polyZ;
	char clipOR = 0x00;
	char clipAND = 0xFF;

	for( i=0; i<8; ++i ) {
		clipOR  |= LOBYTE(vbuf[i].clip);
		clipAND &= LOBYTE(vbuf[i].clip);
	}

	if( (clipOR < 0) || (clipAND != 0) || !VBUF_VISIBLE(vbuf[0], vbuf[1], vbuf[2]) )
		return;

	for( i = 0; i < 8; ++i ) {
		VBuffer[i].x = vbuf[i].xs;
		VBuffer[i].y = vbuf[i].ys;
	}

	nPoints = 8;

	if( clipOR != 0 ) {
		FltWinLeft = 0.0;
		FltWinTop  = 0.0;
		FltWinRight  = (float)PhdWinMaxX;
		FltWinBottom = (float)PhdWinMaxY;

		nPoints = XYClipper(nPoints, VBuffer);
		if( nPoints == 0) return;
	}

	polyZ = 0;
	for( i = 0; i < 8; ++i ) {
		polyZ += vbuf[i].zv;
	}
	polyZ /= 8;

	Sort3dPtr->_0 = (int)Info3dPtr;
	Sort3dPtr->_1 = polyZ;
	++Sort3dPtr;

	*(Info3dPtr++) = 7; // TODO: change to enum (polyType)
	*(Info3dPtr++) = shade;
	*(Info3dPtr++) = nPoints; // number of vertices

	for( i = 0; i < nPoints; ++i ) {
		*(Info3dPtr++) = VBuffer[i].x;
		*(Info3dPtr++) = VBuffer[i].y;
	}

	++SurfaceCount;
}

void __cdecl InsertTransQuad(int x, int y, int width, int height, int z) {
	Sort3dPtr->_0 = (int)Info3dPtr;
	Sort3dPtr->_1 = PhdNearZ + 8 * z;
	++Sort3dPtr;

	*(Info3dPtr++) = 7;  // TODO: change to enum (polyType)
	*(Info3dPtr++) = 32; // TODO: add description
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
	Sort3dPtr->_0 = (int)Info3dPtr;
	Sort3dPtr->_1 = z;
	++Sort3dPtr;

	*(Info3dPtr++) = 5; // TODO: change to enum (polyType)
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
	Sort3dPtr->_0 = (int)Info3dPtr;
	Sort3dPtr->_1 = z;
	++Sort3dPtr;

	*(Info3dPtr++) = 4; // TODO: change to enum (polyType)
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

	clipOR  = LOBYTE(vtx0->clip) | LOBYTE(vtx1->clip) | LOBYTE(vtx2->clip);
	clipAND = LOBYTE(vtx0->clip) & LOBYTE(vtx1->clip) & LOBYTE(vtx2->clip);

	if( clipAND != 0 )
		return;

	if( clipOR >= 0 ) {
		if( !VBUF_VISIBLE(*vtx0, *vtx1, *vtx2) )
			return;

		if( clipOR == 0 ) {
			VertexZBuffered[0].sx = vtx0->xs;
			VertexZBuffered[0].sy = vtx0->ys;
			VertexZBuffered[0].sz = FltResZBuf - FltResZORhw * vtx0->rhw;
			VertexZBuffered[0].rhw = vtx0->rhw;
			VertexZBuffered[0].color = shadeColor(0xFF, 0xFF, 0xFF, 0xFF, vtx0->g);
			VertexZBuffered[0].tu = (double)uv0->u / (double)PHD_ONE;
			VertexZBuffered[0].tv = (double)uv0->v / (double)PHD_ONE;

			VertexZBuffered[1].sx = vtx1->xs;
			VertexZBuffered[1].sy = vtx1->ys;
			VertexZBuffered[1].sz = FltResZBuf - FltResZORhw * vtx1->rhw;
			VertexZBuffered[1].rhw = vtx1->rhw;
			VertexZBuffered[1].color = shadeColor(0xFF, 0xFF, 0xFF, 0xFF, vtx1->g);
			VertexZBuffered[1].tu = (double)uv1->u / (double)PHD_ONE;
			VertexZBuffered[1].tv = (double)uv1->v / (double)PHD_ONE;

			VertexZBuffered[2].sx = vtx2->xs;
			VertexZBuffered[2].sy = vtx2->ys;
			VertexZBuffered[2].sz = FltResZBuf - FltResZORhw * vtx2->rhw;
			VertexZBuffered[2].rhw = vtx2->rhw;
			VertexZBuffered[2].color = shadeColor(0xFF, 0xFF, 0xFF, 0xFF, vtx2->g);
			VertexZBuffered[2].tu = (double)uv2->u / (double)PHD_ONE;
			VertexZBuffered[2].tv = (double)uv2->v / (double)PHD_ONE;

			HWR_TexSource(HWR_PageHandles[texture->tpage]);
			HWR_EnableColorKey(texture->drawtype != 0);

			_Direct3DDevice2->DrawPrimitive(D3DPT_TRIANGLELIST, D3DVT_TLVERTEX, VertexZBuffered, 3, D3DDP_DONOTUPDATEEXTENTS|D3DDP_DONOTCLIP);
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
		if( !nPoints ) return;
	}

	nPoints = XYGUVClipper(nPoints, VBuffer);
	if( !nPoints ) return;

	HWR_TexSource(HWR_PageHandles[texture->tpage]);
	HWR_EnableColorKey(texture->drawtype != 0);
	DrawClippedPoly_Textured(nPoints);
}

void __cdecl DrawClippedPoly_Textured(int vtxCount) {
	D3DCOLOR color;
	double tu, tv;

	if( vtxCount == 0 )
		return;

	for( int i = 0; i < vtxCount; ++i ) {
		color = shadeColor(0xFF, 0xFF, 0xFF, 0xFF, (DWORD)VBuffer[i].g);

		tu = VBuffer[i].u / VBuffer[i].rhw / (double)PHD_ONE;
		tv = VBuffer[i].v / VBuffer[i].rhw / (double)PHD_ONE;
		CLAMP(tu, 0.0, 1.0);
		CLAMP(tv, 0.0, 1.0);

		VertexZBuffered[i].sx = VBuffer[i].x;
		VertexZBuffered[i].sy = VBuffer[i].y;
		VertexZBuffered[i].sz = FltResZBuf - FltResZORhw * VBuffer[i].rhw;
		VertexZBuffered[i].rhw = VBuffer[i].rhw;
		VertexZBuffered[i].color = color;
		VertexZBuffered[i].tu = tu;
		VertexZBuffered[i].tv = tv;
	}

	_Direct3DDevice2->DrawPrimitive(D3DPT_TRIANGLEFAN, D3DVT_TLVERTEX, VertexZBuffered, vtxCount, D3DDP_DONOTUPDATEEXTENTS|D3DDP_DONOTCLIP);
}

void __cdecl InsertGT4_ZBuffered(PHD_VBUF *vtx0, PHD_VBUF *vtx1, PHD_VBUF *vtx2, PHD_VBUF *vtx3, PHD_TEXTURE *texture) {
	char clipOR, clipAND;

	clipOR  = LOBYTE(vtx0->clip) | LOBYTE(vtx1->clip) | LOBYTE(vtx2->clip) | LOBYTE(vtx3->clip);
	clipAND = LOBYTE(vtx0->clip) & LOBYTE(vtx1->clip) & LOBYTE(vtx2->clip) & LOBYTE(vtx3->clip);

	if( clipAND != 0 )
		return;

	if( clipOR == 0 && VBUF_VISIBLE(*vtx0, *vtx1, *vtx2) ) {
		VertexZBuffered[0].sx = vtx0->xs;
		VertexZBuffered[0].sy = vtx0->ys;
		VertexZBuffered[0].sz = FltResZBuf - FltResZORhw * vtx0->rhw;
		VertexZBuffered[0].rhw = vtx0->rhw;
		VertexZBuffered[0].color = shadeColor(0xFF, 0xFF, 0xFF, 0xFF, vtx0->g);
		VertexZBuffered[0].tu = (double)texture->uv[0].u / (double)PHD_ONE;
		VertexZBuffered[0].tv = (double)texture->uv[0].v / (double)PHD_ONE;

		VertexZBuffered[1].sx = vtx1->xs;
		VertexZBuffered[1].sy = vtx1->ys;
		VertexZBuffered[1].sz = FltResZBuf - FltResZORhw * vtx1->rhw;
		VertexZBuffered[1].rhw = vtx1->rhw;
		VertexZBuffered[1].color = shadeColor(0xFF, 0xFF, 0xFF, 0xFF, vtx1->g);
		VertexZBuffered[1].tu = (double)texture->uv[1].u / (double)PHD_ONE;
		VertexZBuffered[1].tv = (double)texture->uv[1].v / (double)PHD_ONE;

		VertexZBuffered[2].sx = vtx2->xs;
		VertexZBuffered[2].sy = vtx2->ys;
		VertexZBuffered[2].sz = FltResZBuf - FltResZORhw * vtx2->rhw;
		VertexZBuffered[2].rhw = vtx2->rhw;
		VertexZBuffered[2].color = shadeColor(0xFF, 0xFF, 0xFF, 0xFF, vtx2->g);
		VertexZBuffered[2].tu = (double)texture->uv[2].u / (double)PHD_ONE;
		VertexZBuffered[2].tv = (double)texture->uv[2].v / (double)PHD_ONE;

		VertexZBuffered[3].sx = vtx3->xs;
		VertexZBuffered[3].sy = vtx3->ys;
		VertexZBuffered[3].sz = FltResZBuf - FltResZORhw * vtx3->rhw;
		VertexZBuffered[3].rhw = vtx3->rhw;
		VertexZBuffered[3].color = shadeColor(0xFF, 0xFF, 0xFF, 0xFF, vtx3->g);
		VertexZBuffered[3].tu = (double)texture->uv[3].u / (double)PHD_ONE;
		VertexZBuffered[3].tv = (double)texture->uv[3].v / (double)PHD_ONE;

		HWR_TexSource(HWR_PageHandles[texture->tpage]);
		HWR_EnableColorKey(texture->drawtype != 0);

		_Direct3DDevice2->DrawPrimitive(D3DPT_TRIANGLEFAN, D3DVT_TLVERTEX, VertexZBuffered, 4, D3DDP_DONOTUPDATEEXTENTS|D3DDP_DONOTCLIP);
	}
	else if( (clipOR < 0 && visible_zclip(vtx0, vtx1, vtx2)) ||
			 (clipOR > 0 && VBUF_VISIBLE(*vtx0, *vtx1, *vtx2)) )
	{
		InsertGT3_ZBuffered(vtx0, vtx1, vtx2, texture, texture->uv, &texture->uv[1], &texture->uv[2]);
		InsertGT3_ZBuffered(vtx0, vtx2, vtx3, texture, texture->uv, &texture->uv[2], &texture->uv[3]);
	}
}

void __cdecl DrawPoly_Gouraud(int vtxCount, int red, int green, int blue) {
	D3DCOLOR color;

	if( vtxCount == 0 )
		return;

	for( int i = 0; i < vtxCount; ++i ) {
		color = shadeColor(red, green, blue, 0xFF, (DWORD)VBuffer[i].g);

		VertexZBuffered[i].sx = VBuffer[i].x;
		VertexZBuffered[i].sy = VBuffer[i].y;
		VertexZBuffered[i].sz = FltResZBuf - FltResZORhw * VBuffer[i].rhw;
		VertexZBuffered[i].rhw = VBuffer[i].rhw;
		VertexZBuffered[i].color = color;
	}

	_Direct3DDevice2->DrawPrimitive(D3DPT_TRIANGLEFAN, D3DVT_TLVERTEX, VertexZBuffered, vtxCount, D3DDP_DONOTUPDATEEXTENTS|D3DDP_DONOTCLIP);
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

	color = shadeColor(GamePalette8[colorIdx].red, GamePalette8[colorIdx].green, GamePalette8[colorIdx].blue, 0xFF, 0);
	rhw = RhwFactor / (double)z;
	sz = FltResZBuf - rhw * FltResZORhw;

	VertexZBuffered[0].sx = (float)x0;
	VertexZBuffered[0].sy = (float)y0;
	VertexZBuffered[1].sx = (float)x1;
	VertexZBuffered[1].sy = (float)y0;
	VertexZBuffered[2].sx = (float)x0;
	VertexZBuffered[2].sy = (float)y1;
	VertexZBuffered[3].sx = (float)x1;
	VertexZBuffered[3].sy = (float)y1;

	for( int i=0; i<4; ++i ) {
		VertexZBuffered[i].sz = sz;
		VertexZBuffered[i].rhw = rhw;
		VertexZBuffered[i].color = color;
	}

	HWR_TexSource(0);
	HWR_EnableColorKey(false);
	_Direct3DDevice2->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DVT_TLVERTEX, VertexZBuffered, 4, D3DDP_DONOTUPDATEEXTENTS|D3DDP_DONOTCLIP);
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

	color = shadeColor(GamePalette8[colorIdx].red, GamePalette8[colorIdx].green, GamePalette8[colorIdx].blue, 0xFF, 0);
	rhw = RhwFactor / (double)z;
	sz = FltResZBuf - rhw * FltResZORhw;

	VertexZBuffered[0].sx = (float)(PhdWinMinX + x0);
	VertexZBuffered[0].sy = (float)(PhdWinMinY + y0);
	VertexZBuffered[1].sx = (float)(PhdWinMinX + x1);
	VertexZBuffered[1].sy = (float)(PhdWinMinY + y1);

	for( int i=0; i<2; ++i ) {
		VertexZBuffered[i].sz = sz;
		VertexZBuffered[i].rhw = rhw;
		VertexZBuffered[i].color = color;
	}

	HWR_TexSource(0);
	HWR_EnableColorKey(false);
	_Direct3DDevice2->DrawPrimitive(D3DPT_LINESTRIP, D3DVT_TLVERTEX, VertexZBuffered, 2, D3DDP_DONOTUPDATEEXTENTS|D3DDP_DONOTCLIP);
}

void __cdecl InsertSprite_Sorted(int z, int x0, int y0, int x1, int y1, int spriteIdx, __int16 shade) {
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

	u0 = rhw * (double)(uOffset - UvAdd + PhdSpriteInfo[spriteIdx].width);
	v0 = rhw * (double)(vOffset + UvAdd);

	u1 = rhw * (double)(uOffset + UvAdd);
	v1 = rhw * (double)(vOffset - UvAdd + PhdSpriteInfo[spriteIdx].height);

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

	IsShadeEffect = 0;
	InsertClippedPoly_Textured(nPoints, (float)z, 10, PhdSpriteInfo[spriteIdx].texPage); // TODO: change to enum (polyType)
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

	Sort3dPtr->_0 = (int)Info3dPtr;
	Sort3dPtr->_1 = z;
	++Sort3dPtr;

	*(Info3dPtr++) = 11; // TODO: change to enum (polyType)
	*(Info3dPtr++) = 4; //  vertex count
	*(D3DTLVERTEX **)Info3dPtr = HWR_VertexPtr;
	Info3dPtr += sizeof(D3DTLVERTEX *)/sizeof(__int16);

	color = shadeColor(GamePalette8[colorIdx].red, GamePalette8[colorIdx].green, GamePalette8[colorIdx].blue, 0xFF, 0);
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

	Sort3dPtr->_0 = (int)Info3dPtr;
	Sort3dPtr->_1 = z;
	++Sort3dPtr;

	*(Info3dPtr++) = 12; // TODO: change to enum (polyType)
	*(Info3dPtr++) = 2; //  vertex count
	*(D3DTLVERTEX **)Info3dPtr = HWR_VertexPtr;
	Info3dPtr += sizeof(D3DTLVERTEX *)/sizeof(__int16);

	color = shadeColor(GamePalette8[colorIdx].red, GamePalette8[colorIdx].green, GamePalette8[colorIdx].blue, 0xFF, 0);
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

	for( i=0; i<8; ++i ) {
		clipOR  |= LOBYTE(vbuf[i].clip);
		clipAND &= LOBYTE(vbuf[i].clip);
	}

	if( (clipOR < 0) || (clipAND != 0) || !VBUF_VISIBLE(vbuf[0], vbuf[1], vbuf[2]) )
		return;

	for( i=0; i<8; ++i ) {
		VBuffer[i].x = vbuf[i].xs;
		VBuffer[i].y = vbuf[i].ys;
		VBuffer[i].rhw = RhwFactor / (double)(vbuf[i].zv - 0x20000);
	}

	nPoints = 8;

	if( clipOR != 0 ) {
		FltWinLeft = 0.0;
		FltWinTop = 0.0;
		FltWinRight = (float)PhdWinMaxX;
		FltWinBottom = (float)PhdWinMaxY;

		nPoints = XYClipper(nPoints, VBuffer);
		if( nPoints == 0) return;
	}

	polyZ = 0;
	for( i = 0; i < 8; ++i ) {
		polyZ += vbuf[i].zv;
	}
	polyZ /= 8;

	InsertPoly_Gouraud(nPoints, (float)(polyZ - 0x20000), 0, 0, 0, 13); // TODO: change to enum (polyType)
}

void __cdecl InsertTransQuad_Sorted(int x, int y, int width, int height, int z) {
	float x0, y0, x1, y1;
	double rhw, sz;

	Sort3dPtr->_0 = (int)Info3dPtr;
	Sort3dPtr->_1 = z;
	++Sort3dPtr;

	*(Info3dPtr++) = 13; // TODO: change to enum (polyType)
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

void __cdecl InsertSprite(int z, int x0, int y0, int x1, int y1, int spriteIdx, __int16 shade) {
	Sort3dPtr->_0 = (int)Info3dPtr;
	Sort3dPtr->_1 = z;
	++Sort3dPtr;

	*(Info3dPtr++) = 8; // TODO: change to enum (polyType)
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

	INJECT(0x00407FF0, InsertTrans8);
	INJECT(0x004084A0, InsertTransQuad);
	INJECT(0x00408580, InsertFlatRect);
	INJECT(0x00408650, InsertLine);
	INJECT(0x00408710, InsertGT3_ZBuffered);
	INJECT(0x00408D60, DrawClippedPoly_Textured);
	INJECT(0x00408EA0, InsertGT4_ZBuffered);

	INJECT(0x004097D0, DrawPoly_Gouraud);

	INJECT(0x00409BB0, InsertFlatRect_ZBuffered);
	INJECT(0x00409D80, InsertLine_ZBuffered);

	INJECT(0x0040B6A0, InsertSprite_Sorted);
	INJECT(0x0040B9F0, InsertFlatRect_Sorted);
	INJECT(0x0040BB70, InsertLine_Sorted);
	INJECT(0x0040BCA0, InsertTrans8_Sorted);
	INJECT(0x0040BE40, InsertTransQuad_Sorted);
	INJECT(0x0040BF80, InsertSprite);
}
