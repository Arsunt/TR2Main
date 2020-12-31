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

#ifdef FEATURE_HUD_IMPROVED
#include "modding/psx_bar.h"
#endif // FEATURE_HUD_IMPROVED

#ifdef FEATURE_VIDEOFX_IMPROVED
DWORD AlphaBlendMode = 2;

typedef struct {
	D3DBLEND src;
	D3DBLEND dst;
	BYTE alpha;
} BLEND_PARAM;

static BLEND_PARAM Blend[4] = {
	{D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, 128},
	{D3DBLEND_ONE, D3DBLEND_ONE, 255},
	{D3DBLEND_ZERO, D3DBLEND_INVSRCCOLOR, 255},
	{D3DBLEND_INVSRCALPHA, D3DBLEND_ONE, 192},
};

static void SetBlendMode(D3DTLVERTEX *vtxPtr, DWORD vtxCount, DWORD mode) {
	if( !vtxPtr || !vtxCount || mode >= 4 ) return;
	for( DWORD i = 0; i < vtxCount; ++i ) {
		vtxPtr[i].color = RGBA_SETALPHA(vtxPtr[i].color, Blend[mode].alpha);
	}
#if (DIRECT3D_VERSION >= 0x900)
	D3DDev->SetRenderState(D3DRS_SRCBLEND, Blend[mode].src);
	D3DDev->SetRenderState(D3DRS_DESTBLEND, Blend[mode].dst);
#else // (DIRECT3D_VERSION >= 0x900)
	D3DDev->SetRenderState(D3DRENDERSTATE_SRCBLEND, Blend[mode].src);
	D3DDev->SetRenderState(D3DRENDERSTATE_DESTBLEND, Blend[mode].dst);
#endif // (DIRECT3D_VERSION >= 0x900)
}

static void DrawAlphaBlended(D3DTLVERTEX *vtxPtr, DWORD vtxCount, DWORD mode) {
	// do basic blending
	SetBlendMode(vtxPtr, vtxCount, mode);
	HWR_DrawPrimitive(D3DPT_TRIANGLEFAN, vtxPtr, vtxCount, true);
	// do advanced blending
	if( AlphaBlendMode == 2 && mode == 0 ) {
		SetBlendMode(vtxPtr, vtxCount, 3); // additional quarter-additive blending pass
		HWR_DrawPrimitive(D3DPT_TRIANGLEFAN, vtxPtr, vtxCount, true);
	}
	// return render states to default values
#if (DIRECT3D_VERSION >= 0x900)
	D3DDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	D3DDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
#else // (DIRECT3D_VERSION >= 0x900)
	D3DDev->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
	D3DDev->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
#endif // (DIRECT3D_VERSION >= 0x900)
}
#endif // FEATURE_VIDEOFX_IMPROVED

// NOTE: this function is absent in the original code
HRESULT HWR_DrawPrimitive(D3DPRIMITIVETYPE primitiveType, LPVOID vertices, DWORD vertexCount, bool isNoClip) {
#if (DIRECT3D_VERSION >= 0x900)
	extern LPDIRECT3DVERTEXBUFFER9 D3DVtx;
	int primitiveCount = 0;
	switch( primitiveType ) {
		case D3DPT_POINTLIST:		primitiveCount = vertexCount;   break;
		case D3DPT_LINELIST:		primitiveCount = vertexCount/2; break;
		case D3DPT_LINESTRIP:		primitiveCount = vertexCount-1; break;
		case D3DPT_TRIANGLELIST:	primitiveCount = vertexCount/3; break;
		case D3DPT_TRIANGLESTRIP:	primitiveCount = vertexCount-2; break;
		case D3DPT_TRIANGLEFAN:		primitiveCount = vertexCount-2; break;
		default: break;
	}
	if( primitiveCount <= 0 ) {
		return D3DERR_INVALIDCALL;
	}
	LPVOID ptr = NULL;
	D3DVtx->Lock(0, 0, &ptr, 0);
	memcpy(ptr, vertices, sizeof(D3DTLVERTEX) * vertexCount);
	D3DVtx->Unlock();
	D3DDev->SetFVF(D3DFVF_TLVERTEX);
	D3DDev->SetStreamSource(0, D3DVtx, 0, sizeof(D3DTLVERTEX));
	return D3DDev->DrawPrimitive(primitiveType, 0, primitiveCount);
#else // (DIRECT3D_VERSION >= 0x900)
	return D3DDev->DrawPrimitive(primitiveType, D3DVT_TLVERTEX, vertices, vertexCount, isNoClip ? D3DDP_DONOTUPDATEEXTENTS|D3DDP_DONOTCLIP : 0);
#endif // (DIRECT3D_VERSION >= 0x900)
}

void __cdecl HWR_InitState() {
#if (DIRECT3D_VERSION >= 0x900)
	D3DDev->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	D3DDev->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	D3DDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	D3DDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	D3DDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	D3DDev->SetRenderState(D3DRS_DITHERENABLE, SavedAppSettings.Dither ? TRUE : FALSE);
	AlphaBlendEnabler = D3DRS_ALPHABLENDENABLE;

	D3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	D3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	D3DDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	D3DDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	D3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	D3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	DWORD filter = SavedAppSettings.BilinearFiltering ? D3DTEXF_LINEAR : D3DTEXF_POINT;
	D3DDev->SetSamplerState(0, D3DSAMP_MAGFILTER, filter);
	D3DDev->SetSamplerState(0, D3DSAMP_MINFILTER, filter);
	D3DDev->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	D3DDev->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
#else // (DIRECT3D_VERSION >= 0x900)
	D3DDev->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_SOLID);
	D3DDev->SetRenderState(D3DRENDERSTATE_SHADEMODE, D3DSHADE_GOURAUD);
	D3DDev->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
	D3DDev->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
	D3DDev->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);

	D3DDev->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, SavedAppSettings.PerspectiveCorrect ? TRUE : FALSE);
	D3DDev->SetRenderState(D3DRENDERSTATE_DITHERENABLE, SavedAppSettings.Dither ? TRUE : FALSE);
	AlphaBlendEnabler = CurrentDisplayAdapter.shadeRestricted ? D3DRENDERSTATE_STIPPLEDALPHA : D3DRENDERSTATE_ALPHABLENDENABLE;

	DWORD filter = SavedAppSettings.BilinearFiltering ? D3DFILTER_LINEAR : D3DFILTER_NEAREST;
	DWORD blend = (CurrentDisplayAdapter.D3DHWDeviceDesc.dpcTriCaps.dwTextureBlendCaps & D3DPTBLENDCAPS_MODULATEALPHA) ? D3DTBLEND_MODULATEALPHA : D3DTBLEND_MODULATE;
	D3DDev->SetRenderState(D3DRENDERSTATE_TEXTUREMAG, filter);
	D3DDev->SetRenderState(D3DRENDERSTATE_TEXTUREMIN, filter);
	D3DDev->SetRenderState(D3DRENDERSTATE_TEXTUREMAPBLEND, blend);
	// NOTE: the next line is absent in the original game, but it fixes a texture bleeding in some cases
	D3DDev->SetRenderState(D3DRENDERSTATE_TEXTUREADDRESS, D3DTADDRESS_CLAMP);
#endif // (DIRECT3D_VERSION >= 0x900)

	HWR_ResetTexSource();
	HWR_ResetColorKey();
	HWR_ResetZBuffer();
}

void __cdecl HWR_ResetTexSource() {
	CurrentTexSource = 0;
#if (DIRECT3D_VERSION >= 0x900)
	D3DDev->SetTexture(0, NULL);
#else // (DIRECT3D_VERSION >= 0x900)
	D3DDev->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, 0);
	D3DDev->SetRenderState(D3DRENDERSTATE_FLUSHBATCH, 0);
#endif // (DIRECT3D_VERSION >= 0x900)
}

void __cdecl HWR_ResetColorKey() {
	ColorKeyState = FALSE;
#if (DIRECT3D_VERSION >= 0x900)
	D3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
#else // (DIRECT3D_VERSION >= 0x900)
	D3DDev->SetRenderState(TexturesAlphaChannel ? D3DRENDERSTATE_ALPHABLENDENABLE : D3DRENDERSTATE_COLORKEYENABLE, FALSE);
#endif // (DIRECT3D_VERSION >= 0x900)
}

void __cdecl HWR_ResetZBuffer() {
	ZEnableState = FALSE;
	ZWriteEnableState = FALSE;
#if (DIRECT3D_VERSION >= 0x900)
	D3DDev->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	D3DDev->SetRenderState(D3DRS_ZWRITEENABLE, D3DZB_FALSE);
#else // (DIRECT3D_VERSION >= 0x900)
	if( ZBufferSurface != NULL ) {
		D3DDev->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_ALWAYS);
		D3DDev->SetRenderState(D3DRENDERSTATE_ZENABLE, SavedAppSettings.ZBuffer ? TRUE : FALSE);
	} else {
		D3DDev->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL);
		D3DDev->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE);
	}
	D3DDev->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
#endif // (DIRECT3D_VERSION >= 0x900)
}

void __cdecl HWR_TexSource(HWR_TEXHANDLE texSource) {
	if( CurrentTexSource != texSource ) {
#if (DIRECT3D_VERSION >= 0x900)
		D3DDev->SetTexture(0, texSource);
#else // (DIRECT3D_VERSION >= 0x900)
		D3DDev->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, texSource);
#endif // (DIRECT3D_VERSION >= 0x900)
		CurrentTexSource = texSource;
	}
}

void __cdecl HWR_EnableColorKey(bool state) {
	if( ColorKeyState != state ) {
#if (DIRECT3D_VERSION >= 0x900)
		D3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, state ? TRUE : FALSE);
#else // (DIRECT3D_VERSION >= 0x900)
		D3DDev->SetRenderState(TexturesAlphaChannel ? D3DRENDERSTATE_ALPHABLENDENABLE : D3DRENDERSTATE_COLORKEYENABLE, state ? TRUE : FALSE);
#endif // (DIRECT3D_VERSION >= 0x900)
		ColorKeyState = state;
	}
}

void __cdecl HWR_EnableZBuffer(bool ZWriteEnable, bool ZEnable) {
	if( !SavedAppSettings.ZBuffer )
		return;

	if( ZWriteEnableState != ZWriteEnable ) {
#if (DIRECT3D_VERSION >= 0x900)
		D3DDev->SetRenderState(D3DRS_ZWRITEENABLE, ZWriteEnable ? D3DZB_TRUE : D3DZB_FALSE);
#else // (DIRECT3D_VERSION >= 0x900)
		D3DDev->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, ZWriteEnable ? TRUE : FALSE);
#endif // (DIRECT3D_VERSION >= 0x900)
		ZWriteEnableState = ZWriteEnable;
	}

	if( ZEnableState != ZEnable ) {
#if (DIRECT3D_VERSION >= 0x900)
		D3DDev->SetRenderState(D3DRS_ZENABLE, ZEnable ? D3DZB_TRUE : D3DZB_FALSE);
#else // (DIRECT3D_VERSION >= 0x900)
		if( ZBufferSurface != NULL )
			D3DDev->SetRenderState(D3DRENDERSTATE_ZFUNC, ZEnable ? D3DCMP_LESSEQUAL : D3DCMP_ALWAYS);
		else
			D3DDev->SetRenderState(D3DRENDERSTATE_ZENABLE, ZEnable ? TRUE : FALSE);
#endif // (DIRECT3D_VERSION >= 0x900)
		ZEnableState = ZEnable;
	}
}

void __cdecl HWR_BeginScene() {
	HWR_GetPageHandles();
#if (DIRECT3D_VERSION < 0x900)
	WaitPrimaryBufferFlip();
#endif // (DIRECT3D_VERSION < 0x900)
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
			UINT16 alpha = *(bufPtr++);
			if( polyType == POLY_HWR_healthbar ) {
				PSX_DrawHealthBar(x0, y0, x1, y1, bar, pixel, alpha);
			} else {
				PSX_DrawAirBar(x0, y0, x1, y1, bar, pixel, alpha);
			}
			continue;
		}
#endif // FEATURE_HUD_IMPROVED
#ifdef FEATURE_VIDEOFX_IMPROVED
		switch( polyType ) {
			case POLY_HWR_GTmap:
			case POLY_HWR_WGTmap:
			case POLY_HWR_WGTmapHalf:
			case POLY_HWR_WGTmapAdd:
			case POLY_HWR_WGTmapSub:
			case POLY_HWR_WGTmapQrt:
				texPage = *(bufPtr++);
				break;
			default:
				texPage = 0;
				break;
		}
#else // !FEATURE_VIDEOFX_IMPROVED
		texPage = ( polyType == POLY_HWR_GTmap || polyType == POLY_HWR_WGTmap ) ? *(bufPtr++) : 0;
#endif // !FEATURE_VIDEOFX_IMPROVED
		vtxCount = *(bufPtr++);
		vtxPtr = *(D3DTLVERTEX **)bufPtr;

		switch( polyType ) {
			case POLY_HWR_GTmap: // triangle fan (texture)
			case POLY_HWR_WGTmap: // triangle fan (texture + colorkey)
#ifdef FEATURE_VIDEOFX_IMPROVED
			case POLY_HWR_WGTmapHalf: // triangle fan (texture + colorkey + PSX half blend)
			case POLY_HWR_WGTmapAdd: // triangle fan (texture + colorkey + PSX additive blend)
			case POLY_HWR_WGTmapSub: // triangle fan (texture + colorkey + PSX subtractive blend)
			case POLY_HWR_WGTmapQrt: // triangle fan (texture + colorkey + PSX quarter blend)
				HWR_TexSource(texPage == (UINT16)~0 ? GetEnvmapTextureHandle() : HWR_PageHandles[texPage]);
				HWR_EnableColorKey(polyType != POLY_HWR_GTmap);
				if( TextureFormat.bpp < 16 || AlphaBlendMode == 0 || polyType == POLY_HWR_GTmap || polyType == POLY_HWR_WGTmap ) {
					HWR_DrawPrimitive(D3DPT_TRIANGLEFAN, vtxPtr, vtxCount, true);
				} else {
					DrawAlphaBlended(vtxPtr, vtxCount, polyType-POLY_HWR_WGTmapHalf);
				}
#else // !FEATURE_VIDEOFX_IMPROVED
				HWR_TexSource(HWR_PageHandles[texPage]);
				HWR_EnableColorKey(polyType == POLY_HWR_WGTmap);
				HWR_DrawPrimitive(D3DPT_TRIANGLEFAN, vtxPtr, vtxCount, true);
#endif // !FEATURE_VIDEOFX_IMPROVED
				break;

			case POLY_HWR_gouraud: // triangle fan (color)
#ifdef FEATURE_VIDEOFX_IMPROVED
			case POLY_HWR_half: // triangle fan (color + PSX half blend)
			case POLY_HWR_add: // triangle fan (color + PSX additive blend)
			case POLY_HWR_sub: // triangle fan (color + PSX subtractive blend)
			case POLY_HWR_qrt: // triangle fan (color + PSX quarter blend)
				HWR_TexSource(0);
				HWR_EnableColorKey(polyType != POLY_HWR_gouraud);
				if( TextureFormat.bpp < 16 || AlphaBlendMode == 0 || polyType == POLY_HWR_gouraud ) {
					HWR_DrawPrimitive(D3DPT_TRIANGLEFAN, vtxPtr, vtxCount, true);
				} else {
					DrawAlphaBlended(vtxPtr, vtxCount, polyType-POLY_HWR_half);
				}
#else // !FEATURE_VIDEOFX_IMPROVED
				HWR_TexSource(0);
				HWR_EnableColorKey(false);
				HWR_DrawPrimitive(D3DPT_TRIANGLEFAN, vtxPtr, vtxCount, true);
#endif // !FEATURE_VIDEOFX_IMPROVED
				break;

			case POLY_HWR_line: // line strip (color)
				HWR_TexSource(0);
				HWR_EnableColorKey(false);
				HWR_DrawPrimitive(D3DPT_LINESTRIP, vtxPtr, vtxCount, true);
				break;

			case POLY_HWR_trans: // triangle fan (color + semitransparent)
				HWR_TexSource(0);
				D3DDev->GetRenderState(AlphaBlendEnabler, &alphaState);
				D3DDev->SetRenderState(AlphaBlendEnabler, TRUE);
				HWR_DrawPrimitive(D3DPT_TRIANGLEFAN, vtxPtr, vtxCount, true);
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

	for( DWORD i=0; i<ARRAY_SIZE(HWR_TexturePageIndexes); ++i ) {
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
	for( DWORD i=0; i<ARRAY_SIZE(HWR_TexturePageIndexes); ++i ) {
		if( HWR_TexturePageIndexes[i] < 0 )
			HWR_PageHandles[i] = 0;
		else
			HWR_PageHandles[i] = GetTexturePageHandle(HWR_TexturePageIndexes[i]);
	}
}

bool __cdecl HWR_VertexBufferFull() {
	DWORD index = ((DWORD)HWR_VertexPtr - (DWORD)HWR_VertexBuffer) / sizeof(D3DTLVERTEX);
	return (index >= ARRAY_SIZE(HWR_VertexBuffer) - 0x200);
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
