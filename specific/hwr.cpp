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
#include "specific/hwr.h"
#include "specific/init_display.h"
#include "specific/texture.h"
#include "global/vars.h"

#ifdef FEATURE_VIDEOFX_IMPROVED
extern HWR_TEXHANDLE GetEnvmapTextureHandle();
#endif // FEATURE_VIDEOFX_IMPROVED

#ifdef FEATURE_HUD_IMPROVED
#include "modding/psx_bar.h"
#endif // FEATURE_HUD_IMPROVED

void __cdecl HWR_InitState() {
	DWORD filter, blend;

	D3DDev->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_SOLID);
	D3DDev->SetRenderState(D3DRENDERSTATE_SHADEMODE, D3DSHADE_GOURAUD);
	D3DDev->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
	D3DDev->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
	D3DDev->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);

	filter = SavedAppSettings.BilinearFiltering ? D3DFILTER_LINEAR : D3DFILTER_NEAREST;
	blend = (CurrentDisplayAdapter.D3DHWDeviceDesc.dpcTriCaps.dwTextureBlendCaps & D3DPTBLENDCAPS_MODULATEALPHA) ? D3DTBLEND_MODULATEALPHA : D3DTBLEND_MODULATE;

	D3DDev->SetRenderState(D3DRENDERSTATE_TEXTUREMAG, filter);
	D3DDev->SetRenderState(D3DRENDERSTATE_TEXTUREMIN, filter);
	D3DDev->SetRenderState(D3DRENDERSTATE_TEXTUREMAPBLEND, blend);
	D3DDev->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, SavedAppSettings.PerspectiveCorrect ? TRUE : FALSE);
	D3DDev->SetRenderState(D3DRENDERSTATE_DITHERENABLE, SavedAppSettings.Dither ? TRUE : FALSE);
	AlphaBlendEnabler = CurrentDisplayAdapter.shadeRestricted ? D3DRENDERSTATE_STIPPLEDALPHA : D3DRENDERSTATE_ALPHABLENDENABLE;

	// NOTE: the next line is absent in the original game, but it fixes a texture bleeding in some cases
	D3DDev->SetRenderState(D3DRENDERSTATE_TEXTUREADDRESS, D3DTADDRESS_CLAMP);

	HWR_ResetTexSource();
	HWR_ResetColorKey();
	HWR_ResetZBuffer();
}

void __cdecl HWR_ResetTexSource() {
	CurrentTexSource = 0;
	D3DDev->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, 0);
	D3DDev->SetRenderState(D3DRENDERSTATE_FLUSHBATCH, 0);
}

void __cdecl HWR_ResetColorKey() {
	ColorKeyState = FALSE;
	D3DDev->SetRenderState(TexturesAlphaChannel ? D3DRENDERSTATE_ALPHABLENDENABLE : D3DRENDERSTATE_COLORKEYENABLE, FALSE);
}

void __cdecl HWR_ResetZBuffer() {
	ZEnableState = FALSE;
	ZWriteEnableState = FALSE;
	if( ZBufferSurface != NULL ) {
		D3DDev->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_ALWAYS);
		D3DDev->SetRenderState(D3DRENDERSTATE_ZENABLE, SavedAppSettings.ZBuffer ? TRUE : FALSE);
	} else {
		D3DDev->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL);
		D3DDev->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE);
	}
	D3DDev->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
}

void __cdecl HWR_TexSource(HWR_TEXHANDLE texSource) {
	if( CurrentTexSource != texSource ) {
		D3DDev->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, texSource);
		CurrentTexSource = texSource;
	}
}

void __cdecl HWR_EnableColorKey(bool state) {
	if( ColorKeyState != state ) {
		if( TexturesAlphaChannel )
			D3DDev->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, state ? TRUE : FALSE);
		else
			D3DDev->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, state ? TRUE : FALSE);
		ColorKeyState = state;
	}
}

void __cdecl HWR_EnableZBuffer(bool ZWriteEnable, bool ZEnable) {
	if( !SavedAppSettings.ZBuffer )
		return;

	if( ZWriteEnableState != ZWriteEnable ) {
		D3DDev->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, ZWriteEnable ? TRUE : FALSE);
		ZWriteEnableState = ZWriteEnable;
	}

	if( ZEnableState != ZEnable ) {
		if( ZBufferSurface != NULL )
			D3DDev->SetRenderState(D3DRENDERSTATE_ZFUNC, ZEnable ? D3DCMP_LESSEQUAL : D3DCMP_ALWAYS);
		else
			D3DDev->SetRenderState(D3DRENDERSTATE_ZENABLE, ZEnable ? TRUE : FALSE);
		ZEnableState = ZEnable;
	}
}

void __cdecl HWR_BeginScene() {
	HWR_GetPageHandles();
	WaitPrimaryBufferFlip();
	D3DDev->BeginScene();
}

void __cdecl HWR_DrawPolyList() {
	DWORD alphaState;
	UINT16 *bufPtr;
	UINT16 polyType, texPage, vtxCount;
	D3DTLVERTEX *vtxPtr;

	HWR_EnableZBuffer(false, true);
	for( DWORD i=0; i<SurfaceCount; ++i ) {
		bufPtr = (UINT16 *)SortBuffer[i]._0;

		polyType = *(bufPtr++);
#ifdef FEATURE_HUD_IMPROVED
		if( polyType == POLY_HWR_healthbar || polyType == POLY_HWR_airbar ) {
			UINT16 x0 = *(bufPtr++);
			UINT16 y0 = *(bufPtr++);
			UINT16 x1 = *(bufPtr++);
			UINT16 y1 = *(bufPtr++);
			UINT16 bar = *(bufPtr++);
			UINT16 pixel = *(bufPtr++);
			if( polyType == POLY_HWR_healthbar ) {
				PSX_DrawHealthBar(x0, y0, x1, y1, bar, pixel);
			} else {
				PSX_DrawAirBar(x0, y0, x1, y1, bar, pixel);
			}
			continue;
		}
#endif // FEATURE_HUD_IMPROVED
		texPage = ( polyType == POLY_HWR_GTmap || polyType == POLY_HWR_WGTmap ) ? *(bufPtr++) : 0;
		vtxCount = *(bufPtr++);
		vtxPtr = *(D3DTLVERTEX **)bufPtr;

		switch( polyType ) {
			case POLY_HWR_GTmap: // triangle fan (texture)
			case POLY_HWR_WGTmap: // triangle fan (texture + colorkey)
#ifdef FEATURE_VIDEOFX_IMPROVED
				HWR_TexSource(texPage == (UINT16)~0 ? GetEnvmapTextureHandle() : HWR_PageHandles[texPage]);
#else // !FEATURE_VIDEOFX_IMPROVED
				HWR_TexSource(HWR_PageHandles[texPage]);
#endif // !FEATURE_VIDEOFX_IMPROVED
				HWR_EnableColorKey(polyType == POLY_HWR_WGTmap);
				D3DDev->DrawPrimitive(D3DPT_TRIANGLEFAN, D3D_TLVERTEX, vtxPtr, vtxCount, D3DDP_DONOTUPDATEEXTENTS|D3DDP_DONOTCLIP);
				break;

			case POLY_HWR_gouraud: // triangle fan (color)
				HWR_TexSource(0);
				HWR_EnableColorKey(false);
				D3DDev->DrawPrimitive(D3DPT_TRIANGLEFAN, D3D_TLVERTEX, vtxPtr, vtxCount, D3DDP_DONOTUPDATEEXTENTS|D3DDP_DONOTCLIP);
				break;

			case POLY_HWR_line: // line strip (color)
				HWR_TexSource(0);
				HWR_EnableColorKey(false);
				D3DDev->DrawPrimitive(D3DPT_LINESTRIP, D3D_TLVERTEX, vtxPtr, vtxCount, D3DDP_DONOTUPDATEEXTENTS|D3DDP_DONOTCLIP);
				break;

			case POLY_HWR_trans: // triangle fan (color + semitransparent)
				HWR_TexSource(0);
				D3DDev->GetRenderState(AlphaBlendEnabler, &alphaState);
				D3DDev->SetRenderState(AlphaBlendEnabler, TRUE);
				D3DDev->DrawPrimitive(D3DPT_TRIANGLEFAN, D3D_TLVERTEX, vtxPtr, vtxCount, D3DDP_DONOTUPDATEEXTENTS|D3DDP_DONOTCLIP);
				D3DDev->SetRenderState(AlphaBlendEnabler, alphaState);
				break;
		}
	}
}

void __cdecl HWR_LoadTexturePages(int pagesCount, LPVOID pagesBuffer, RGB888 *palette) {
	int pageIndex = -1;
	BYTE *bufferPtr = (BYTE *)pagesBuffer;

	HWR_FreeTexturePages();

	if( palette != NULL )
		PaletteIndex = CreateTexturePalette(palette);

	for( int i=0; i<pagesCount; ++i ) {
		if( palette != NULL ) {
			pageIndex = AddTexturePage8(256, 256, bufferPtr, PaletteIndex);
			bufferPtr += 256*256*1;
		} else {
			pageIndex = AddTexturePage16(256, 256, bufferPtr);
			bufferPtr += 256*256*2;
		}
		HWR_TexturePageIndexes[i] = (pageIndex < 0) ? -1 : pageIndex;
	}
	HWR_GetPageHandles();
}

void __cdecl HWR_FreeTexturePages() {

	for( int i=0; i<32; ++i ) {
		if( HWR_TexturePageIndexes[i] >= 0 ) {
			SafeFreeTexturePage(HWR_TexturePageIndexes[i]);
			HWR_TexturePageIndexes[i] = -1;
		}
		HWR_PageHandles[i] = 0;
	}
	if( PaletteIndex >= 0 ) {
		SafeFreePalette(PaletteIndex);
	}
}

void __cdecl HWR_GetPageHandles() {
	for( int i=0; i<32; ++i ) {
		if( HWR_TexturePageIndexes[i] < 0 )
			HWR_PageHandles[i] = 0;
		else
			HWR_PageHandles[i] = GetTexturePageHandle(HWR_TexturePageIndexes[i]);
	}
}

bool __cdecl HWR_VertexBufferFull() {
	DWORD index = ((DWORD)HWR_VertexPtr - (DWORD)HWR_VertexBuffer) / sizeof(D3DTLVERTEX);
	return (index >= 0x2000 - 0x200);
}

bool __cdecl HWR_Init() {
	memset(HWR_VertexBuffer, 0, sizeof(HWR_VertexBuffer));
	memset(HWR_TexturePageIndexes, 0xFF, sizeof(HWR_TexturePageIndexes)); // fill indexes by -1
	return true;
}

/*
 * Inject function
 */
void Inject_HWR() {
	INJECT(0x0044D0B0, HWR_InitState);
	INJECT(0x0044D1E0, HWR_ResetTexSource);
	INJECT(0x0044D210, HWR_ResetColorKey);
	INJECT(0x0044D240, HWR_ResetZBuffer);
	INJECT(0x0044D2A0, HWR_TexSource);
	INJECT(0x0044D2D0, HWR_EnableColorKey);
	INJECT(0x0044D320, HWR_EnableZBuffer);
	INJECT(0x0044D3B0, HWR_BeginScene);
	INJECT(0x0044D3E0, HWR_DrawPolyList);
	INJECT(0x0044D560, HWR_LoadTexturePages);
	INJECT(0x0044D5F0, HWR_FreeTexturePages);
	INJECT(0x0044D640, HWR_GetPageHandles);
	INJECT(0x0044D680, HWR_VertexBufferFull);
	INJECT(0x0044D6B0, HWR_Init);
}
