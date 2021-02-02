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
#include "specific/init_3d.h"
#include "global/vars.h"

#if (DIRECT3D_VERSION >= 0x900)
LPDIRECT3DVERTEXBUFFER9 D3DVtx = NULL;
#endif // (DIRECT3D_VERSION >= 0x900)

#if (DIRECT3D_VERSION < 0x900)
void __cdecl Enumerate3DDevices(DISPLAY_ADAPTER *adapter) {
	if( D3DCreate() ) {
		D3D->EnumDevices(Enum3DDevicesCallback, (LPVOID)adapter);
		D3DRelease();
	}
}
#endif // (DIRECT3D_VERSION < 0x900)

bool __cdecl D3DCreate() {
#if (DIRECT3D_VERSION >= 0x900)
	D3D = Direct3DCreate9(D3D_SDK_VERSION);
	return ( D3D != NULL );
#else // (DIRECT3D_VERSION >= 0x900)
	return SUCCEEDED(DDraw->QueryInterface(IID_IDirect3D2, (LPVOID *)&D3D));
#endif // (DIRECT3D_VERSION >= 0x900)
}

void __cdecl D3DRelease() {
	if( D3D ) {
		D3D->Release();
		D3D = NULL;
	}
}

#if (DIRECT3D_VERSION < 0x900)
HRESULT CALLBACK Enum3DDevicesCallback(GUID FAR* lpGuid, LPTSTR lpDeviceDescription, LPTSTR lpDeviceName, LPD3DDEVICEDESC lpD3DHWDeviceDesc, LPD3DDEVICEDESC lpD3DHELDeviceDesc, LPVOID lpContext) {
	DISPLAY_ADAPTER *adapter = (DISPLAY_ADAPTER *)lpContext;

	if( lpD3DHWDeviceDesc && D3DIsSupported(lpD3DHWDeviceDesc) ) {
		adapter->hwRenderSupported = true;
		adapter->deviceGuid = *lpGuid;
		adapter->D3DHWDeviceDesc = *lpD3DHWDeviceDesc;

		adapter->perspectiveCorrectSupported = (lpD3DHWDeviceDesc->dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_PERSPECTIVE)? true:false;
		adapter->ditherSupported = (lpD3DHWDeviceDesc->dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_DITHER)? true:false;
		adapter->zBufferSupported = (lpD3DHWDeviceDesc->dwDeviceZBufferBitDepth)? true:false;
		adapter->linearFilterSupported = (lpD3DHWDeviceDesc->dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_LINEAR)? true:false;
		adapter->shadeRestricted = (lpD3DHWDeviceDesc->dpcTriCaps.dwShadeCaps & (D3DPSHADECAPS_ALPHAGOURAUDBLEND|D3DPSHADECAPS_ALPHAFLATBLEND))? false:true;
	}
	return D3DENUMRET_OK;
}

bool __cdecl D3DIsSupported(LPD3DDEVICEDESC desc) {
	if( (desc->dwFlags & D3DDD_COLORMODEL) &&
		(desc->dcmColorModel & D3DCOLOR_RGB) &&
		(desc->dwFlags & D3DDD_TRICAPS) &&
		(desc->dpcTriCaps.dwShadeCaps & D3DPSHADECAPS_COLORGOURAUDRGB) &&
		(desc->dpcTriCaps.dwTextureBlendCaps & D3DPTBLENDCAPS_MODULATE) )
	{
		return true;
	}
	return false;
}

bool __cdecl D3DSetViewport() {
	D3DVIEWPORT2 viewPort;

	viewPort.dwSize = sizeof(D3DVIEWPORT2);
	viewPort.dvClipX = 0.0;
	viewPort.dvClipY = 0.0;
	viewPort.dvClipWidth = (float)GameVidWidth;
	viewPort.dvClipHeight = (float)GameVidHeight;

	viewPort.dwX = 0;
	viewPort.dwY = 0;
	viewPort.dwWidth = GameVidWidth;
	viewPort.dwHeight = GameVidHeight;

	viewPort.dvMinZ = 0.0;
	viewPort.dvMaxZ = 1.0;

	if FAILED(D3DView->SetViewport2(&viewPort)) {
		D3DView->GetViewport2(&viewPort);
		return false;
	}

	return SUCCEEDED(D3DDev->SetCurrentViewport(D3DView));

}
#endif // (DIRECT3D_VERSION < 0x900)

void __cdecl D3DDeviceCreate(LPDDS lpBackBuffer) {
	if( D3D == NULL && !D3DCreate() )
		throw ERR_D3D_Create;

#if (DIRECT3D_VERSION >= 0x900)
	D3DPRESENT_PARAMETERS d3dpp;
	memset(&d3dpp, 0, sizeof(d3dpp));

	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = HGameWindow;
	d3dpp.EnableAutoDepthStencil = TRUE;

	D3DCAPS9 *caps = &SavedAppSettings.PreferredDisplayAdapter->body.caps;
	if SUCCEEDED(D3D->CheckDepthStencilMatch(caps->AdapterOrdinal, caps->DeviceType, D3DFMT_X8R8G8B8, D3DFMT_X8R8G8B8, D3DFMT_D16)) {
		d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	} else if SUCCEEDED(D3D->CheckDepthStencilMatch(caps->AdapterOrdinal, caps->DeviceType, D3DFMT_X8R8G8B8, D3DFMT_X8R8G8B8, D3DFMT_D24X8)) {
		d3dpp.AutoDepthStencilFormat = D3DFMT_D24X8;
	} else {
		throw ERR_CreateDevice;
	}

	d3dpp.Windowed = !SavedAppSettings.FullScreen;
	if( SavedAppSettings.FullScreen ) {
		d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
		d3dpp.BackBufferWidth = FullScreenWidth;
		d3dpp.BackBufferHeight = FullScreenHeight;
	}

	if( D3DDev ) {
		if( D3DVtx != NULL ) {
			D3DVtx->Release();
			D3DVtx = NULL;
		}
		HRESULT res = D3D_OK;
		do {
			res = D3DDev->TestCooperativeLevel();
		} while( res == D3DERR_DEVICELOST );
		if( (res != D3D_OK && res != D3DERR_DEVICENOTRESET) || FAILED(D3DDev->Reset(&d3dpp)) ) {
			throw ERR_CreateDevice;
		}
	} else if FAILED(D3D->CreateDevice(CurrentDisplayAdapter.index, D3DDEVTYPE_HAL, HGameWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &D3DDev)) {
		throw ERR_CreateDevice;
	}

	if( !D3DVtx && FAILED(D3DDev->CreateVertexBuffer(256*sizeof(D3DTLVERTEX), D3DUSAGE_DYNAMIC|D3DUSAGE_SOFTWAREPROCESSING, D3DFVF_TLVERTEX, D3DPOOL_DEFAULT, &D3DVtx, NULL)) )
		throw ERR_CreateDevice;

	D3DDev->SetStreamSource(0, D3DVtx, 0, sizeof(D3DTLVERTEX));
	D3DDev->SetFVF(D3DFVF_TLVERTEX);
#else // (DIRECT3D_VERSION >= 0x900)
	if FAILED(D3D->CreateDevice(IID_IDirect3DHALDevice, (LPDIRECTDRAWSURFACE)lpBackBuffer, &D3DDev))
		throw ERR_CreateDevice;

	if FAILED(D3D->CreateViewport(&D3DView, NULL))
		throw ERR_CreateViewport;

	if FAILED(D3DDev->AddViewport(D3DView))
		throw ERR_AddViewport;

	if FAILED(!D3DSetViewport())
		throw ERR_SetViewport2;

	if FAILED(D3D->CreateMaterial(&D3DMaterial, NULL))
		throw ERR_CreateViewport;

	D3DMATERIAL matData;
	D3DMATERIALHANDLE matHandle;
	memset(&matData, 0, sizeof(matData));
	matData.dwSize = sizeof(matData);

	if FAILED(D3DMaterial->SetMaterial(&matData))
		throw ERR_CreateViewport;

	if FAILED(D3DMaterial->GetHandle(D3DDev, &matHandle))
		throw ERR_CreateViewport;

	if FAILED(D3DView->SetBackground(matHandle))
		throw ERR_CreateViewport;
#endif // (DIRECT3D_VERSION >= 0x900)
}

void __cdecl Direct3DRelease() {
#if (DIRECT3D_VERSION >= 0x900)
	if( D3DVtx != NULL ) {
		D3DVtx->Release();
		D3DVtx = NULL;
	}
#else // (DIRECT3D_VERSION >= 0x900)
	if( D3DMaterial != NULL ) {
		D3DMaterial->Release();
		D3DMaterial = NULL;
	}
	if( D3DView != NULL ) {
		D3DView->Release();
		D3DView = NULL;
	}
#endif // (DIRECT3D_VERSION >= 0x900)
	if( D3DDev != NULL ) {
		D3DDev->Release();
		D3DDev = NULL;
	}
	D3DRelease();
}

bool __cdecl Direct3DInit() {
	return true;
}

/*
 * Inject function
 */
void Inject_Init3d() {
#if (DIRECT3D_VERSION < 0x900)
	INJECT(0x004445F0, Enumerate3DDevices);
#endif // (DIRECT3D_VERSION < 0x900)
	INJECT(0x00444620, D3DCreate);
	INJECT(0x00444640, D3DRelease);
#if (DIRECT3D_VERSION < 0x900)
	INJECT(0x00444660, Enum3DDevicesCallback);
	INJECT(0x00444720, D3DIsSupported);
	INJECT(0x00444760, D3DSetViewport);
#endif // (DIRECT3D_VERSION < 0x900)
	INJECT(0x00444820, D3DDeviceCreate);
	INJECT(0x004449E0, Direct3DRelease);
	INJECT(0x00444A30, Direct3DInit);
}
