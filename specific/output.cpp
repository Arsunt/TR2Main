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
#include "specific/output.h"
#include "3dsystem/3d_gen.h"
#include "3dsystem/phd_math.h"
#include "specific/background.h"
#include "specific/display.h"
#include "specific/file.h"
#include "specific/frontend.h"
#include "specific/hwr.h"
#include "specific/init.h"
#include "specific/init_display.h"
#include "specific/texture.h"
#include "specific/utils.h"
#include "specific/winvid.h"
#include "global/vars.h"

#ifdef FEATURE_HEALTHBAR_IMPROVED
#include "modding/psx_bar.h"

bool PsxBarsEnabled;
#endif // FEATURE_HEALTHBAR_IMPROVED

#ifdef FEATURE_FOG_DISTANCE
extern int CalculateFogShade(int depth);
#endif // FEATURE_FOG_DISTANCE

int __cdecl GetRenderScale(int unit) {
	int scaleX = (PhdWinWidth > 640) ? MulDiv(PhdWinWidth, unit, 640) : unit;
	int scaleY = (PhdWinHeight > 480) ? MulDiv(PhdWinHeight, unit, 480) : unit;
	return min(scaleX, scaleY);
}

int __cdecl GetRenderHeight() {
	return PhdWinHeight;
}

int __cdecl GetRenderWidth() {
	return PhdWinWidth;
}

void __cdecl S_InitialisePolyList(BOOL clearBackBuffer) {
	DWORD flags = 0;

	if( WinVidNeedToResetBuffers ) {
		RestoreLostBuffers();
		WinVidSpinMessageLoop(false);
		if( SavedAppSettings.FullScreen ) {
			flags = CLRB_BackBuffer|CLRB_PrimaryBuffer;
			if( SavedAppSettings.RenderMode == RM_Software )
				flags |= CLRB_RenderBuffer;
			if( SavedAppSettings.TripleBuffering )
				flags |= CLRB_ThirdBuffer;
			WaitPrimaryBufferFlip();
		} else {
			flags = CLRB_WindowedPrimaryBuffer;
			if( SavedAppSettings.RenderMode == RM_Hardware )
				flags |= CLRB_BackBuffer;
			else
				flags |= CLRB_RenderBuffer;
		}
		ClearBuffers(flags, 0);
		clearBackBuffer = FALSE; // already cleared here
		WinVidNeedToResetBuffers = false;
	}

	flags = CLRB_PhdWinSize;
	if( SavedAppSettings.RenderMode == RM_Software ) {
		// Software Renderer
		flags |= CLRB_RenderBuffer;
		ClearBuffers(flags, 0);
	} else {
		// Hardware Renderer
		if( clearBackBuffer )
			flags |= CLRB_BackBuffer;
		if( SavedAppSettings.ZBuffer && ZBufferSurface != NULL )
			flags |= CLRB_ZBuffer;

		ClearBuffers(flags, 0);
		HWR_BeginScene();
		HWR_EnableZBuffer(true, true);
	}
	phd_InitPolyList();
}

DWORD __cdecl S_DumpScreen() {
	DWORD ticks = Sync();
	while( ticks < TICKS_PER_FRAME ) {
		while( !Sync() ) /* just wait for new frame */;
		ticks++;
	}
	ScreenPartialDump();
	return ticks;
}

void __cdecl S_ClearScreen() {
	ScreenClear(false);
}

void __cdecl S_InitialiseScreen(int levelType) {
	if( levelType < 0 ) {
		// No Level
		FadeToPal(0, GamePalette8);
	} else {
		if( levelType ) {
			// Not title (Title has levelType=0)
			TempVideoRemove();
		}
		// Title or Any Level
		FadeToPal(30, GamePalette8);
	}

	if( SavedAppSettings.RenderMode == RM_Hardware ) {
		HWR_InitState();
	}
}

void __cdecl S_OutputPolyList() {
	struct _DDSURFACEDESC desc;

	if( SavedAppSettings.RenderMode == RM_Software ) {
		// Software renderer
		phd_SortPolyList();
		if SUCCEEDED(WinVidBufferLock(RenderBufferSurface, &desc, DDLOCK_WRITEONLY|DDLOCK_WAIT)) {
			phd_PrintPolyList((BYTE *)desc.lpSurface);
			WinVidBufferUnlock(RenderBufferSurface, &desc);
		}
	} else {
		// Hardware renderer
		if( !SavedAppSettings.ZBuffer || !SavedAppSettings.DontSortPrimitives ) {
			phd_SortPolyList();
		}
		HWR_DrawPolyList();
		_Direct3DDevice2->EndScene();
	}
}

int __cdecl S_GetObjectBounds(__int16 *bPtr) {
	int xMin, xMax, yMin, yMax, zMin, zMax;
	int numZ, xv, yv, zv;
	PHD_VECTOR vtx[8];

	if( PhdMatrixPtr->_23 >= PhdFarZ )
		return 0; // object box is out of screen

	xMin = bPtr[0];
	xMax = bPtr[1];
	yMin = bPtr[2];
	yMax = bPtr[3];
	zMin = bPtr[4];
	zMax = bPtr[5];

	vtx[0].x = xMin;	vtx[0].y = yMin;	vtx[0].z = zMin;
	vtx[1].x = xMax;	vtx[1].y = yMin;	vtx[1].z = zMin;
	vtx[2].x = xMax;	vtx[2].y = yMax;	vtx[2].z = zMin;
	vtx[3].x = xMin;	vtx[3].y = yMax;	vtx[3].z = zMin;
	vtx[4].x = xMin;	vtx[4].y = yMin;	vtx[4].z = zMax;
	vtx[5].x = xMax;	vtx[5].y = yMin;	vtx[5].z = zMax;
	vtx[6].x = xMax;	vtx[6].y = yMax;	vtx[6].z = zMax;
	vtx[7].x = xMin;	vtx[7].y = yMax;	vtx[7].z = zMax;

	xMin = yMin = +0x3FFFFFFF;
	xMax = yMax = -0x3FFFFFFF;

	numZ = 0;

	for( int i=0; i<8; ++i ) {
		zv = PhdMatrixPtr->_20 * vtx[i].x +
			 PhdMatrixPtr->_21 * vtx[i].y +
			 PhdMatrixPtr->_22 * vtx[i].z +
			 PhdMatrixPtr->_23;

		if( zv > PhdNearZ && zv < PhdFarZ ) {
			++numZ;
			zv /= PhdPersp;

			xv = (PhdMatrixPtr->_00 * vtx[i].x +
				  PhdMatrixPtr->_01 * vtx[i].y +
				  PhdMatrixPtr->_02 * vtx[i].z +
				  PhdMatrixPtr->_03) / zv;

			if( xMin > xv )
				xMin = xv;
			if( xMax < xv )
				xMax = xv;

			yv = (PhdMatrixPtr->_10 * vtx[i].x +
				  PhdMatrixPtr->_11 * vtx[i].y +
				  PhdMatrixPtr->_12 * vtx[i].z +
				  PhdMatrixPtr->_13) / zv;

			if( yMin > yv )
				yMin = yv;
			if( yMax < yv )
				yMax = yv;
		}
	}

	xMin += PhdWinCenterX;
	xMax += PhdWinCenterX;
	yMin += PhdWinCenterY;
	yMax += PhdWinCenterY;

	if( numZ == 0 || xMin > PhdWinRight || yMin > PhdWinBottom || xMax < PhdWinLeft || yMax < PhdWinTop )
		return 0; // object box is out of screen

	if ( numZ < 8 || xMin < 0 || yMin < 0 || xMax > PhdWinMaxX || yMax > PhdWinMaxY )
		return -1; // object box is clipped

	return 1; // object box is totally on screen
}

void __cdecl S_InsertBackPolygon(int x0, int y0, int x1, int y1) {
	ins_flat_rect(PhdWinMinX + x0, PhdWinMinY + y0,
				  PhdWinMinX + x1, PhdWinMinY + y1,
				  PhdFarZ + 1, InvColours.black);
}

void __cdecl S_PrintShadow(__int16 radius, __int16 *bPtr, ITEM_INFO *item) {
	int x0, x1, z0, z1, midX, midZ, xAdd, zAdd;

	x0 = bPtr[0];
	x1 = bPtr[1];
	z0 = bPtr[4];
	z1 = bPtr[5];

	midX = (x0 + x1) / 2;
	xAdd = (x1 - x0) * radius / 0x400;
	midZ = (z0 + z1) / 2;
	zAdd = (z1 - z0) * radius / 0x400;

	// The shadow is a regular octagon
	ShadowInfo.vertex[0].x = midX - xAdd;
	ShadowInfo.vertex[0].z = midZ + zAdd * 2;
	ShadowInfo.vertex[1].x = midX + xAdd;
	ShadowInfo.vertex[1].z = midZ + zAdd * 2;
	ShadowInfo.vertex[2].x = midX + xAdd * 2;
	ShadowInfo.vertex[2].z = midZ + zAdd;
	ShadowInfo.vertex[3].x = midX + xAdd * 2;
	ShadowInfo.vertex[3].z = midZ - zAdd;
	ShadowInfo.vertex[4].x = midX + xAdd;
	ShadowInfo.vertex[4].z = midZ - zAdd * 2;
	ShadowInfo.vertex[5].x = midX - xAdd;
	ShadowInfo.vertex[5].z = midZ - zAdd * 2;
	ShadowInfo.vertex[6].x = midX - xAdd * 2;
	ShadowInfo.vertex[6].z = midZ - zAdd;
	ShadowInfo.vertex[7].x = midX - xAdd * 2;
	ShadowInfo.vertex[7].z = midZ + zAdd;

	// Update screen parameters
	FltWinLeft		= (double)(PhdWinMinX + PhdWinLeft);
	FltWinTop		= (double)(PhdWinMinY + PhdWinTop);
	FltWinRight		= (double)(PhdWinMinX + PhdWinRight+1);
	FltWinBottom	= (double)(PhdWinMinY + PhdWinBottom+1);
	FltWinCenterX	= (double)(PhdWinMinX + PhdWinCenterX);
	FltWinCenterY	= (double)(PhdWinMinY + PhdWinCenterY);

	// Transform and print the shadow
	phd_PushMatrix();
	phd_TranslateAbs(item->pos.x, item->floor, item->pos.z);
	phd_RotY(item->pos.rotY);
	if( calc_object_vertices(&ShadowInfo.polyCount) ) {
		// Here 32 is DepthQ index (shade factor).
		// 0 lightest, 15 no shade, 31 darkest (pitch black).
		// But 32 and above interpreted as 24 (which means 50% darker)
		ins_poly_trans8(PhdVBuf, 32);
	}
	phd_PopMatrix();
}

void __cdecl S_CalculateLight(int x, int y, int z, __int16 roomNumber) {
	ROOM_INFO *room;
	int i;
	int xDist, yDist, zDist, distance, radius, depth;
	int xBrightest=0, yBrightest=0, zBrightest=0;
	int brightest, adder;
	int shade, shade1, shade2;
	int falloff, falloff1, falloff2;
	int intensity, intensity1, intensity2;
	int lightShade;
	VECTOR_ANGLES angles;

	room = &RoomInfo[roomNumber];
	brightest = 0;

	// Static light calculation
	if( room->lightMode != 0 ) {
		lightShade = RoomLightShades[room->lightMode];
		for( i = 0; i < room->numLights; ++i ) {
			xDist = x - room->light[i].x;
			yDist = y - room->light[i].y;
			zDist = z - room->light[i].z;
			falloff1 = room->light[i].fallOff1;
			falloff2 = room->light[i].fallOff2;
			intensity1 = room->light[i].intensity1;
			intensity2 = room->light[i].intensity2;

			distance = (SQR(xDist) + SQR(yDist) + SQR(zDist)) >> 12;
			falloff1 = SQR(falloff1) >> 12;
			falloff2 = SQR(falloff2) >> 12;
			shade1 = falloff1 * intensity1 / (falloff1 + distance);
			shade2 = falloff2 * intensity2 / (falloff2 + distance);

			shade = shade1 + (shade2 - shade1) * lightShade / (WIBBLE_SIZE-1);
			if( shade > brightest ) {
				brightest = shade;
				xBrightest = xDist;
				yBrightest = yDist;
				zBrightest = zDist;
			}
		}
	} else {
		for( i = 0; i < room->numLights; ++i ) {
			xDist = x - room->light[i].x;
			yDist = y - room->light[i].y;
			zDist = z - room->light[i].z;
			falloff = room->light[i].fallOff1;
			intensity = room->light[i].intensity1;

			falloff = SQR(falloff) >> 12;
			distance = (SQR(xDist) + SQR(yDist) + SQR(zDist)) >> 12;

			shade = falloff * intensity / (falloff + distance);
			if( shade > brightest ) {
				brightest = shade;
				xBrightest = xDist;
				yBrightest = yDist;
				zBrightest = zDist;
			}
		}
	}
	adder = brightest;

	// Dynamic light calculation
	for( i = 0; i < DynamicLightCount; ++i ) {
		xDist = x - DynamicLights[i].x;
		yDist = y - DynamicLights[i].y;
		zDist = z - DynamicLights[i].z;
		falloff = DynamicLights[i].fallOff1;
		intensity = DynamicLights[i].intensity1;

		radius = 1 << falloff;

		if( (xDist >= -radius && xDist <= radius) &&
			(yDist >= -radius && yDist <= radius) &&
			(zDist >= -radius && zDist <= radius) )
		{
			distance = SQR(xDist) + SQR(yDist) + SQR(zDist);
			if( distance <= SQR(radius) ) {
				shade = (1 << intensity) - (distance >> (2 * falloff - intensity));
				if( shade > brightest ) {
					brightest = shade;
					xBrightest = xDist;
					yBrightest = yDist;
					zBrightest = zDist;
				}
				adder += shade;
			}
		}
	}

	// Light finalization
	adder = adder/2;
	if( adder == 0 ) {
		LsAdder = room->ambient1;
		LsDivider = 0;
	} else {
		LsAdder = room->ambient1 - adder;
		LsDivider = (1 << (W2V_SHIFT + 12)) / adder;
		phd_GetVectorAngles(xBrightest, yBrightest, zBrightest, &angles);
		phd_RotateLight(angles.pitch, angles.yaw);
	}

	// Fog calculation
	depth = PhdMatrixPtr->_23 >> W2V_SHIFT;
#ifdef FEATURE_FOG_DISTANCE
	LsAdder += CalculateFogShade(depth);
#else // !FEATURE_FOG_DISTANCE
	if( depth > DEPTHQ_START ) // fog begin
		LsAdder += depth - DEPTHQ_START;
#endif // FEATURE_FOG_DISTANCE
	if( LsAdder > 0x1FFF ) // fog end
		LsAdder = 0x1FFF;
}

void __cdecl S_CalculateStaticLight(__int16 adder) {
	int depth;

	LsAdder = adder - 16*256;
	depth = PhdMatrixPtr->_23 >> W2V_SHIFT;
#ifdef FEATURE_FOG_DISTANCE
	LsAdder += CalculateFogShade(depth);
#else // !FEATURE_FOG_DISTANCE
	if( depth > DEPTHQ_START ) // fog begin
		LsAdder += depth - DEPTHQ_START;
#endif // FEATURE_FOG_DISTANCE
	if( LsAdder > 0x1FFF ) // fog end
		LsAdder = 0x1FFF;
}

void __cdecl S_CalculateStaticMeshLight(int x, int y, int z, int shade1, int shade2, ROOM_INFO *room) {
	int adder, shade, falloff, intensity;
	int xDist, yDist, zDist, distance, radius;

	adder = shade1;
	if( room->lightMode != 0 ) {
		adder += (shade2 - shade1) * RoomLightShades[room->lightMode] / (WIBBLE_SIZE-1);
	}

	for( int i = 0; i < DynamicLightCount; ++i ) {
		xDist = x - DynamicLights[i].x;
		yDist = y - DynamicLights[i].y;
		zDist = z - DynamicLights[i].z;
		falloff = DynamicLights[i].fallOff1;
		intensity = DynamicLights[i].intensity1;

		radius = 1 << falloff;

		if( (xDist >= -radius && xDist <= radius) &&
			(yDist >= -radius && yDist <= radius) &&
			(zDist >= -radius && zDist <= radius) )
		{
			distance = SQR(xDist) + SQR(yDist) + SQR(zDist);
			if( distance <= SQR(radius) ) {
				shade = (1 << intensity) - (distance >> (2 * falloff - intensity));
				adder -= shade;
				if( adder < 0 ) {
					adder = 0;
					break;
				}
			}
		}
	}
	S_CalculateStaticLight(adder);
}

void __cdecl S_LightRoom(ROOM_INFO *room) {
	int i, j;
	int shade, falloff, intensity;
	int xPos, yPos, zPos;
	int xDist, yDist, zDist, distance, radius;
	int roomVtxCount;
	ROOM_VERTEX_INFO *roomVtx;


	if( room->lightMode != 0 ) {
		int *roomLightTable = RoomLightTables[RoomLightShades[room->lightMode]].table;
		roomVtxCount = *room->data;
		roomVtx = (ROOM_VERTEX_INFO *)(room->data + 1);
		for( i = 0; i < roomVtxCount; ++i ) {
			__int16 wibble = roomLightTable[roomVtx[i].lightTableValue % WIBBLE_SIZE];
			roomVtx[i].lightAdder = roomVtx[i].lightBase + wibble;
		}
	}
	else if( (room->flags & 0x10) != 0 ) {
		roomVtxCount = *room->data;
		roomVtx = (ROOM_VERTEX_INFO *)(room->data + 1);
		for( i = 0; i < roomVtxCount; ++i ) {
			roomVtx[i].lightAdder = roomVtx[i].lightBase;
		}
		room->flags &= ~0x10;
	}

	int xMin = 0x400;
	int zMin = 0x400;
	int xMax = 0x400 * (room->ySize - 1);
	int zMax = 0x400 * (room->xSize - 1);

	for( i = 0; i < DynamicLightCount; ++i ) {
		xPos = DynamicLights[i].x - room->x;
		yPos = DynamicLights[i].y;
		zPos = DynamicLights[i].z - room->z;
		falloff = DynamicLights[i].fallOff1;
		intensity = DynamicLights[i].intensity1;

		radius = 1 << falloff;

		if( xPos + radius >= xMin && zPos + radius >= zMin && xPos - radius <= xMax && zPos - radius <= zMax ) {
			room->flags |= 0x10;
			roomVtxCount = *room->data;
			roomVtx = (ROOM_VERTEX_INFO *)(room->data + 1);
			for( j = 0; j < roomVtxCount; ++j ) {
				if( roomVtx[j].lightAdder != 0 ) {
					xDist = roomVtx[j].x - xPos;
					yDist = roomVtx[j].y - yPos;
					zDist = roomVtx[j].z - zPos;
					if( (xDist >= -radius && xDist <= radius) &&
						(yDist >= -radius && yDist <= radius) &&
						(zDist >= -radius && zDist <= radius) )
					{
						distance = SQR(xDist) + SQR(yDist) + SQR(zDist);
						if( distance <= SQR(radius) ) {
							shade = (1 << intensity) - (distance >> (2 * falloff - intensity));
							roomVtx[j].lightAdder -= shade;
							if( roomVtx[j].lightAdder < 0 )
								roomVtx[j].lightAdder = 0;
						}
					}
				}
			}
		}
	}
}

void __cdecl S_DrawHealthBar(int percent) {
#ifdef FEATURE_HEALTHBAR_IMPROVED
	int barWidth = GetRenderScale(100);
	int barHeight = GetRenderScale(5);
	int barOffset = GetRenderScale(8);
	int pixel = GetRenderScale(1);

	int x0 = PhdWinMinX + barOffset;
	int y0 = PhdWinMinY + barOffset;
	int x1 = x0 + barWidth;
	int y1 = y0 + barHeight;

	int bar = barWidth * percent / 100;

	// Disable underwater shading
	IsShadeEffect = false;

	if( PsxBarsEnabled && SavedAppSettings.RenderMode == RM_Hardware && SavedAppSettings.ZBuffer ) {
		PSX_DrawHealthBar(x0, y0, x1, y1, bar, pixel);
		return;
	}

	// Frame
	ins_flat_rect(x0-pixel*2, y0-pixel*2, x1+pixel*2, y1+pixel*2, PhdNearZ + 50, InvColours.white);
	ins_flat_rect(x0-pixel*1, y0-pixel*1, x1+pixel*2, y1+pixel*2, PhdNearZ + 40, InvColours.gray);
	ins_flat_rect(x0-pixel*1, y0-pixel*1, x1+pixel*1, y1+pixel*1, PhdNearZ + 30, InvColours.black);

	// Health bar
	if( bar > 0 ) {
		ins_flat_rect(x0, y0+pixel*0, x0+bar, y0+barHeight,	PhdNearZ + 20, InvColours.red);
		ins_flat_rect(x0, y0+pixel*1, x0+bar, y0+pixel*2,	PhdNearZ + 10, InvColours.orange);
	}
#else // !FEATURE_HEALTHBAR_IMPROVED
	int i;

	int barWidth = 100;
	int barHeight = 5;

	int x0 = 8;
	int y0 = 8;
	int x1 = x0 + barWidth;
	int y1 = y0 + barHeight;

	int bar = barWidth * percent / 100;

	// Disable underwater shading
	IsShadeEffect = false;

	// Black background
	for( i = 0; i < (barHeight+2); ++i )
		ins_line(x0-2, y0+i-1, x1+1, y0+i-1, PhdNearZ + 50, InvColours.black);

	// Dark frame
	ins_line(x0-2, y1+1, x1+2, y1+1, PhdNearZ + 40, InvColours.gray);
	ins_line(x1+2, y0-2, x1+2, y1+1, PhdNearZ + 40, InvColours.gray);

	// Light frame
	ins_line(x0-2, y0-2, x1+2, y0-2, PhdNearZ + 30, InvColours.white);
	ins_line(x0-2, y1+1, x0-2, y0-2, PhdNearZ + 30, InvColours.white);

	// Health bar
	if( bar > 0 ) {
		for( i = 0; i < barHeight; ++i )
			ins_line(x0, y0+i, x0+bar, y0+i, PhdNearZ + 20, ( i == 1 ) ? InvColours.orange : InvColours.red);
	}
#endif // FEATURE_HEALTHBAR_IMPROVED
}

void __cdecl S_DrawAirBar(int percent) {
#ifdef FEATURE_HEALTHBAR_IMPROVED
	int barWidth = GetRenderScale(100);
	int barHeight = GetRenderScale(5);
	int barOffset = GetRenderScale(8);
	int pixel = GetRenderScale(1);

	int x1 = PhdWinMinX + DumpWidth - barOffset;
	int x0 = x1 - barWidth;
	int y0 = PhdWinMinY + barOffset;
	int y1 = y0 + barHeight;

	int bar = barWidth * percent / 100;

	// Disable underwater shading
	IsShadeEffect = false;

	if( PsxBarsEnabled && SavedAppSettings.RenderMode == RM_Hardware && SavedAppSettings.ZBuffer ) {
		PSX_DrawAirBar(x0, y0, x1, y1, bar, pixel);
		return;
	}

	// Frame
	ins_flat_rect(x0-pixel*2, y0-pixel*2, x1+pixel*2, y1+pixel*2, PhdNearZ + 50, InvColours.white);
	ins_flat_rect(x0-pixel*1, y0-pixel*1, x1+pixel*2, y1+pixel*2, PhdNearZ + 40, InvColours.gray);
	ins_flat_rect(x0-pixel*1, y0-pixel*1, x1+pixel*1, y1+pixel*1, PhdNearZ + 30, InvColours.black);

	// Air bar
	if( bar > 0 ) {
		ins_flat_rect(x0, y0+pixel*0, x0+bar, y0+barHeight,	PhdNearZ + 20, InvColours.blue);
		ins_flat_rect(x0, y0+pixel*1, x0+bar, y0+pixel*2,	PhdNearZ + 10, InvColours.white);
	}
#else // !FEATURE_HEALTHBAR_IMPROVED
	int i;

	int barWidth = 100;
	int barHeight = 5;

	int x1 = DumpWidth - 10;
	int x0 = x1 - barWidth;
	int y0 = 8;
	int y1 = y0 + barHeight;

	int bar = barWidth * percent / 100;

	// Disable underwater shading
	IsShadeEffect = false;

	// Black background
	for( i = 0; i < (barHeight+2); ++i )
		ins_line(x0-2, y0+i-1, x1+1, y0+i-1, PhdNearZ + 50, InvColours.black);

	// Dark frame
	ins_line(x0-2, y1+1, x1+2, y1+1, PhdNearZ + 40, InvColours.gray);
	ins_line(x1+2, y0-2, x1+2, y1+1, PhdNearZ + 40, InvColours.gray);

	// Light frame
	ins_line(x0-2, y0-2, x1+2, y0-2, PhdNearZ + 30, InvColours.white);
	ins_line(x0-2, y1+1, x0-2, y0-2, PhdNearZ + 30, InvColours.white);

	// Air bar
	if( bar > 0 ) {
		for( i = 0; i < barHeight; ++i )
			ins_line(x0, y0+i, x0+bar, y0+i, PhdNearZ + 20, ( i == 1 ) ? InvColours.white : InvColours.blue);
	}
#endif // FEATURE_HEALTHBAR_IMPROVED
}

void __cdecl AnimateTextures(int nFrames) {
	static int frameComp = 0;
	__int16 i, j;
	__int16 *ptr;
	PHD_TEXTURE temp;

	frameComp += nFrames;
	while( frameComp > 10 ) {
		ptr = AnimatedTextureRanges;
		i = *(ptr++);
		for( ; i>0; --i, ++ptr ) {
			j = *(ptr++);
			temp = PhdTextureInfo[*ptr];
			for ( ; j>0; --j, ++ptr ) {
				PhdTextureInfo[ptr[0]] = PhdTextureInfo[ptr[1]];
			}
			PhdTextureInfo[*ptr] = temp;
		}
		frameComp -= 10;
	}
}

void __cdecl S_SetupBelowWater(BOOL underwater) {
	if( IsWet != underwater ) {
		FadeToPal(1, underwater ? WaterPalette : GamePalette8);
		IsWet = underwater;
	}
	IsWaterEffect = true;
	IsShadeEffect = true;
	IsWibbleEffect = !underwater;
}

void __cdecl S_SetupAboveWater(BOOL underwater) {
	IsWaterEffect = false;
	IsShadeEffect = underwater;
	IsWibbleEffect = underwater;
}

void __cdecl S_AnimateTextures(int nFrames) {
	WibbleOffset = (WibbleOffset + nFrames/2) % WIBBLE_SIZE;
	RoomLightShades[1] = GetRandomDraw() & (WIBBLE_SIZE-1);
	RoomLightShades[2] = (WIBBLE_SIZE-1) * (phd_sin(WibbleOffset * PHD_360 / WIBBLE_SIZE) + PHD_IONE/2) / PHD_IONE;

	if( GF_SequenceReady ) {
		AnimFramesCounter += nFrames;
		RoomLightShades[3] = (WIBBLE_SIZE-1) * ((AnimFramesCounter < 72000) ? (AnimFramesCounter / 72000) : 1);
	}
	AnimateTextures(nFrames);
}

void __cdecl S_DisplayPicture(LPCTSTR fileName, BOOL reallocGame) {
	DWORD bytesRead;
	HANDLE hFile;
	DWORD fileSize;
	DWORD bitmapSize;
	BYTE *fileData;
	BYTE *bitmapData;
	LPCTSTR fullPath;

	fullPath = GetFullPath(fileName);
	hFile = CreateFile(fullPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if( hFile == INVALID_HANDLE_VALUE )
		return;

	if( !reallocGame )
		init_game_malloc();

	fileSize = GetFileSize(hFile, NULL);
	fileData = (BYTE *)game_malloc(fileSize, 0x28); // Load Piccy Buffer
	ReadFile(hFile, fileData, fileSize, &bytesRead, NULL);
	CloseHandle(hFile);

	bitmapSize = 640*480*1;
	bitmapData = (BYTE *)game_malloc(bitmapSize, 0x28); // Load Piccy Buffer
	DecompPCX(fileData, fileSize, bitmapData, PicPalette);

	DecompPCX(fileData, fileSize, bitmapData, PicPalette);

	if( SavedAppSettings.RenderMode == RM_Software )
		WinVidCopyBitmapToBuffer(PictureBufferSurface, bitmapData);
	else
		BGND_Make640x480(bitmapData, PicPalette);

	if( !reallocGame )
		CopyBitmapPalette(PicPalette, bitmapData, bitmapSize, GamePalette8);

	game_free(fileSize + bitmapSize);
}

void __cdecl S_SyncPictureBufferPalette() {
	DDSURFACEDESC desc;

	if( PictureBufferSurface == NULL || FAILED(WinVidBufferLock(PictureBufferSurface, &desc, DDLOCK_WRITEONLY|DDLOCK_WAIT)) )
		return;

	SyncSurfacePalettes(desc.lpSurface, 640, 480, desc.lPitch, PicPalette, desc.lpSurface, desc.lPitch, GamePalette8, TRUE);
	WinVidBufferUnlock(PictureBufferSurface, &desc);
	memcpy(PicPalette, GamePalette8, sizeof(RGB)*256);
}

void __cdecl S_DontDisplayPicture() {
	if( SavedAppSettings.RenderMode == RM_Hardware ) {
		BGND_Free();
		BGND_PictureIsReady = false;
	}
}

void __cdecl ScreenDump() {
	UpdateFrame(true, NULL);
}

void __cdecl ScreenPartialDump() {
	UpdateFrame(true, &PhdWinRect);
}

void __cdecl FadeToPal(int fadeValue, RGB *palette) {
	int i, j;
	int palStartIdx = 0;
	int palEndIdx = 256;
	int palSize = 256;
	PALETTEENTRY fadePal[256];

	if( !GameVid_IsVga )
		return;

	if( GameVid_IsWindowedVga ) {
		palStartIdx += 10;
		palEndIdx -= 10;
		palSize -= 20;
	}

	if( fadeValue <= 1 ) {
		for( i=palStartIdx; i<palEndIdx; ++i ) {
			WinVidPalette[i].peRed   = palette[i].red;
			WinVidPalette[i].peGreen = palette[i].green;
			WinVidPalette[i].peBlue  = palette[i].blue;
		}
		_DirectDrawPalette->SetEntries(0, palStartIdx, palSize, &WinVidPalette[palStartIdx]);
		return;
	}

	for( i=palStartIdx; i<palEndIdx; ++i ) {
		fadePal[i] = WinVidPalette[i];
	}

	for( j=0; j<=fadeValue; ++j ) {
		for( i=palStartIdx; i<palEndIdx; ++i ) {
			WinVidPalette[i].peRed   = fadePal[i].peRed   + (palette[i].red   - fadePal[i].peRed)   * j / fadeValue;
			WinVidPalette[i].peGreen = fadePal[i].peGreen + (palette[i].green - fadePal[i].peGreen) * j / fadeValue;
			WinVidPalette[i].peBlue  = fadePal[i].peBlue  + (palette[i].blue  - fadePal[i].peBlue)  * j / fadeValue;
		}
		_DirectDrawPalette->SetEntries(0, palStartIdx, palSize, &WinVidPalette[palStartIdx]);
		S_DumpScreen();
	}
}

void __cdecl ScreenClear(bool isPhdWinSize) {
	DWORD flags = ( SavedAppSettings.RenderMode == RM_Hardware ) ? CLRB_BackBuffer : CLRB_RenderBuffer;

	if( isPhdWinSize )
		flags |= CLRB_PhdWinSize;

	ClearBuffers(flags, 0);
}

void __cdecl S_CopyScreenToBuffer() {
	DDSURFACEDESC desc;

	if( SavedAppSettings.RenderMode != RM_Software )
		return;

	PictureBufferSurface->Blt(NULL, RenderBufferSurface, &GameVidRect, DDBLT_WAIT, NULL);

	if SUCCEEDED(WinVidBufferLock(PictureBufferSurface, &desc, DDLOCK_WRITEONLY|DDLOCK_WAIT)) {
		BYTE *surface = (BYTE *)desc.lpSurface;

		for( int i=0; i<480; ++i ) {
			for( int j=0; j<640; ++j ) {
				surface[j] = DepthQIndex[surface[j]];
			}
			surface += desc.lPitch;
		}
		WinVidBufferUnlock(PictureBufferSurface, &desc);
	}
	memcpy(PicPalette, GamePalette8, sizeof(RGB)*256);
}

void __cdecl S_CopyBufferToScreen() {
	static const int tileX[4] = {0, 256, 512, 640};
	static const int tileY[3] = {0, 256, 480};
	int i, x[4], y[3];
	DWORD color = 0xFFFFFFFF; // vertex color (ARGB white)

	if( SavedAppSettings.RenderMode == RM_Software ) {
		if( memcmp(GamePalette8, PicPalette, sizeof(RGB)*256) ) {
			S_SyncPictureBufferPalette();
		}
		RenderBufferSurface->Blt(&GameVidRect, PictureBufferSurface, NULL, DDBLT_WAIT, NULL);
	}
	else if( BGND_PictureIsReady ) {
		BGND_GetPageHandles();

		for( i = 0; i < 4; ++i )
			 x[i] = tileX[i] * PhdWinWidth / 640 + PhdWinMinX;
		for( i = 0; i < 3; ++i )
			 y[i] = tileY[i] * PhdWinHeight / 480 + PhdWinMinY;

		HWR_EnableZBuffer(false, false);
		DrawTextureTile(x[0], y[0], x[1]-x[0], y[1]-y[0], BGND_PageHandles[0],
						0, 0, 256, 256, color, color, color, color);
		DrawTextureTile(x[1], y[0], x[2]-x[1], y[1]-y[0], BGND_PageHandles[1],
						0, 0, 256, 256, color, color, color, color);
		DrawTextureTile(x[2], y[0], x[3]-x[2], y[1]-y[0], BGND_PageHandles[2],
						0, 0, 128, 256, color, color, color, color);
		DrawTextureTile(x[0], y[1], x[1]-x[0], y[2]-y[1], BGND_PageHandles[3],
						0, 0, 256, 224, color, color, color, color);
		DrawTextureTile(x[1], y[1], x[2]-x[1], y[2]-y[1], BGND_PageHandles[4],
						0, 0, 256, 224, color, color, color, color);
		DrawTextureTile(x[2], y[1], x[3]-x[2], y[2]-y[1], BGND_PageHandles[2],
						128, 0, 128, 224, color, color, color, color);
		HWR_EnableZBuffer(true, true);
	}
	else {
		BGND_DrawInGameBackground();
	}
}

BOOL __cdecl DecompPCX(BYTE *pcx, DWORD pcxSize, BYTE *pic, RGB *pal) {
	PCX_HEADER *header;
	DWORD width, height, dsz;
	BYTE *src, *dst;

	header = (PCX_HEADER *)pcx;
	width	= header->xMax - header->xMin + 1;
	height = header->yMax - header->yMin + 1;

	if( header->manufacturer != 10 ||
		header->version < 5 ||
		header->bpp != 8 ||
		header->rle != 1 ||
		header->planes != 1 ||
		width*height == 0 )
	{
		return FALSE;
	}

	src = pcx + sizeof(PCX_HEADER);
	dst = pic;
	dsz = width*height;

	while( dsz > 1 ) {
		if( (*src & 0xC0) == 0xC0 ) {
			BYTE n = (*src++) & 0x3F;
			BYTE c = *src++;
			if( n > 0 ) memset(dst, c, n);
			dst += n;
			dsz -= n;
		}
		else {
			*dst++ = *src++;
			--dsz;
		}
	}

	if( pal != NULL)
		memcpy(pal, pcx + pcxSize - sizeof(RGB)*256, sizeof(RGB)*256);

	return TRUE;
}

/*
 * Inject function
 */
void Inject_Output() {
	INJECT(0x00450BA0, GetRenderHeight);
	INJECT(0x00450BB0, GetRenderWidth);
	INJECT(0x00450BC0, S_InitialisePolyList);
	INJECT(0x00450CB0, S_DumpScreen);
	INJECT(0x00450CF0, S_ClearScreen);
	INJECT(0x00450D00, S_InitialiseScreen);
	INJECT(0x00450D40, S_OutputPolyList);
	INJECT(0x00450D80, S_GetObjectBounds);
	INJECT(0x00450FF0, S_InsertBackPolygon);
	INJECT(0x00451040, S_PrintShadow);
	INJECT(0x00451240, S_CalculateLight);
	INJECT(0x00451540, S_CalculateStaticLight);
	INJECT(0x00451580, S_CalculateStaticMeshLight);
	INJECT(0x004516B0, S_LightRoom);
	INJECT(0x004518C0, S_DrawHealthBar);
	INJECT(0x00451A90, S_DrawAirBar);
	INJECT(0x00451C90, AnimateTextures);
	INJECT(0x00451D50, S_SetupBelowWater);
	INJECT(0x00451DB0, S_SetupAboveWater);
	INJECT(0x00451DE0, S_AnimateTextures);
	INJECT(0x00451EA0, S_DisplayPicture);
	INJECT(0x00451FB0, S_SyncPictureBufferPalette);
	INJECT(0x00452030, S_DontDisplayPicture);
	INJECT(0x00452040, ScreenDump);
	INJECT(0x00452050, ScreenPartialDump);
	INJECT(0x00452060, FadeToPal);
	INJECT(0x00452230, ScreenClear);
	INJECT(0x00452260, S_CopyScreenToBuffer);
	INJECT(0x00452310, S_CopyBufferToScreen);
	INJECT(0x00452360, DecompPCX);
}
