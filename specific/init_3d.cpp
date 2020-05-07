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
#include "specific/init_3d.h"
#include "global/vars.h"

void __cdecl Enumerate3DDevices(DISPLAY_ADAPTER *adapter) {
	if( D3DCreate() ) {
		D3D->EnumDevices(Enum3DDevicesCallback, (LPVOID)adapter);
		D3DRelease();
	}
}

bool __cdecl D3DCreate() {
	return SUCCEEDED(DDraw->QueryInterface(IID_IDirect3D2, (LPVOID *)&D3D));
}

void __cdecl D3DRelease() {
	if( D3D ) {
		D3D->Release();
		D3D = NULL;
	}
}

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
	viewPort.dwX = 0;
	viewPort.dwY = 0;
	viewPort.dwWidth = GameVidWidth;
	viewPort.dwHeight = GameVidHeight;

	viewPort.dvClipX = 0.0;
	viewPort.dvClipY = 0.0;
	viewPort.dvClipWidth = (float)GameVidWidth;
	viewPort.dvClipHeight = (float)GameVidHeight;
	viewPort.dvMinZ = 0.0;
	viewPort.dvMaxZ = 1.0;

	if FAILED(D3DView->SetViewport2(&viewPort)) {
		D3DView->GetViewport2(&viewPort);
		return false;
	}

	return SUCCEEDED(D3DDev->SetCurrentViewport(D3DView));
}

void __cdecl D3DDeviceCreate(LPDIRECTDRAWSURFACE3 lpBackBuffer) {
	D3DMATERIAL matData;
	D3DMATERIALHANDLE matHandle;

	if( !D3DCreate() )
		throw ERR_D3D_Create;

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

	memset(&matData, 0, sizeof(D3DMATERIAL));
	matData.dwSize = sizeof(D3DMATERIAL);

	if FAILED(D3DMaterial->SetMaterial(&matData))
		throw ERR_CreateViewport;

	if FAILED(D3DMaterial->GetHandle(D3DDev, &matHandle))
		throw ERR_CreateViewport;

	if FAILED(D3DView->SetBackground(matHandle))
		throw ERR_CreateViewport;
}

void __cdecl Direct3DRelease() {
	if( D3DMaterial != NULL ) {
		D3DMaterial->Release();
		D3DMaterial = NULL;
	}
	if( D3DView != NULL ) {
		D3DView->Release();
		D3DView = NULL;
	}
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
	INJECT(0x004445F0, Enumerate3DDevices);
	INJECT(0x00444620, D3DCreate);
	INJECT(0x00444640, D3DRelease);
	INJECT(0x00444660, Enum3DDevicesCallback);
	INJECT(0x00444720, D3DIsSupported);
	INJECT(0x00444760, D3DSetViewport);
	INJECT(0x00444820, D3DDeviceCreate);
	INJECT(0x004449E0, Direct3DRelease);
	INJECT(0x00444A30, Direct3DInit);
}
