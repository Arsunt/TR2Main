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
#include "modding/xinput_ex.h"
#include "global/vars.h"

#ifdef FEATURE_INPUT_IMPROVED

// Imports from xinput*.dll
static HMODULE hXInput = NULL;
static void (WINAPI *_XInputEnable)(WINBOOL) = NULL;
static DWORD (WINAPI *_XInputGetCapabilities)(DWORD, DWORD, XINPUT_CAPABILITIES*) = NULL;
static DWORD (WINAPI *_XInputSetState)(DWORD, XINPUT_VIBRATION*) = NULL;
static DWORD (WINAPI *_XInputGetState)(DWORD, XINPUT_STATE*) = NULL;

static void XInputLibUnlink() {
	_XInputEnable = NULL;
	_XInputSetState = NULL;
	_XInputGetState = NULL;
	_XInputGetCapabilities = NULL;
	if( hXInput != NULL ) {
		FreeLibrary(hXInput);
		hXInput = NULL;
	}
}

static bool XInputLibLink(LPCSTR lpLibFileName, bool advanced) {
	HMODULE hXInput = LoadLibrary(lpLibFileName);
	if( hXInput == NULL ) {XInputLibUnlink(); return false;}

	// XInputEnable is optional, so don't check if it's NULL
	*(FARPROC *)&_XInputEnable = GetProcAddress(hXInput, "XInputEnable");

	*(FARPROC *)&_XInputGetCapabilities = GetProcAddress(hXInput, "XInputGetCapabilities");
	if( _XInputGetCapabilities == NULL ) {XInputLibUnlink(); return false;}

	*(FARPROC *)&_XInputSetState = GetProcAddress(hXInput, "XInputSetState");
	if( _XInputSetState == NULL ) {XInputLibUnlink(); return false;}

	if( advanced ) {
		// The hack to get XInput Guide button state, works for some xinput*.dll versions
		*(FARPROC *)&_XInputGetState = GetProcAddress(hXInput, MAKEINTRESOURCE(100));
	}
	if( _XInputGetState == NULL ) {
		*(FARPROC *)&_XInputGetState = GetProcAddress(hXInput, "XInputGetState");
	}
	if( _XInputGetState == NULL ) {XInputLibUnlink(); return false;}

	return true;
}

static bool XInputLibInit() {
	static const struct {
		LPCSTR name; bool advanced;
	} libs[] = {
		{"xinput1_4.dll", true},
		{"xinput1_3.dll", true},
		{"xinput9_1_0.dll", false},
		{"xinput1_2.dll", false},
		{"xinput1_1.dll", false},
	};
	static bool failed = false;
	if( hXInput != NULL ) {
		return true;
	} else if( failed ) {
		return false;
	}
	for( DWORD i=0; i<ARRAY_SIZE(libs); ++i ) {
		if( XInputLibLink(libs[i].name, libs[i].advanced) ) {
			return true;
		}
	}
	failed = true;
	return false;
}

void WINAPI XInputEnable(BOOL enable) {
	if( !XInputLibInit() || _XInputEnable == NULL ) {
		return;
	}
	_XInputEnable(enable);
}

DWORD WINAPI XInputGetCapabilities(DWORD dwUserIndex, DWORD dwFlags, XINPUT_CAPABILITIES *pCapabilities) {
	if( !XInputLibInit() || _XInputGetCapabilities == NULL ) {
		return ERROR_DEVICE_NOT_CONNECTED;
	}
	return _XInputGetCapabilities(dwUserIndex, dwFlags, pCapabilities);
}

DWORD WINAPI XInputSetState(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration) {
	if( !XInputLibInit() || _XInputSetState == NULL ) {
		return ERROR_DEVICE_NOT_CONNECTED;
	}
	return _XInputSetState(dwUserIndex, pVibration);
}

DWORD WINAPI XInputGetState(DWORD dwUserIndex, XINPUT_STATE *pState) {
	if( !XInputLibInit() || _XInputGetState == NULL ) {
		return ERROR_DEVICE_NOT_CONNECTED;
	}
	return _XInputGetState(dwUserIndex, pState);
}

// ----------------------- The ugly function by Microsoft -----------------------
// Enum each PNP device using WMI and check each device ID to see if it contains
// "IG_" (ex. "VID_045E&PID_028E&IG_00").  If it does, then it's an XInput device
// Unfortunately this information can not be found by just using DirectInput
// ------------------------------------------------------------------------------
#include <wbemidl.h>
#include <oleauto.h>

BOOL IsXInputDevice(DWORD dwVendorId, DWORD dwProductId) {
	IWbemLocator* pIWbemLocator = NULL;
	IEnumWbemClassObject* pEnumDevices = NULL;
	IWbemClassObject* pDevices[20] = {0};
	IWbemServices* pIWbemServices = NULL;
	BSTR bstrNamespace = NULL;
	BSTR bstrDeviceID = NULL;
	BSTR bstrClassName = NULL;
	DWORD uReturned = 0;
	bool bIsXinputDevice = false;
	UINT iDevice = 0;
	VARIANT var;
	HRESULT hr;

	// CoInit if needed
	hr = CoInitialize(NULL);
	bool bCleanupCOM = SUCCEEDED(hr);

	// Create WMI
	hr = CoCreateInstance(__uuidof(WbemLocator), NULL, CLSCTX_INPROC_SERVER,
							__uuidof(IWbemLocator), (LPVOID*) &pIWbemLocator);
	if( FAILED(hr) || pIWbemLocator == NULL ) goto LCleanup;

	bstrNamespace = SysAllocString(L"\\\\.\\root\\cimv2");
	if( bstrNamespace == NULL ) goto LCleanup;

	bstrClassName = SysAllocString(L"Win32_PNPEntity");
	if( bstrClassName == NULL ) goto LCleanup;

	bstrDeviceID = SysAllocString(L"DeviceID");
	if( bstrDeviceID == NULL ) goto LCleanup;

	// Connect to WMI
	hr = pIWbemLocator->ConnectServer(bstrNamespace, NULL, NULL, 0L, 0L, NULL, NULL, &pIWbemServices);
	if( FAILED(hr) || pIWbemServices == NULL ) goto LCleanup;

	// Switch security level to IMPERSONATE.
	CoSetProxyBlanket(pIWbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
						RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

	hr = pIWbemServices->CreateInstanceEnum(bstrClassName, 0, NULL, &pEnumDevices);
	if( FAILED(hr) || pEnumDevices == NULL ) goto LCleanup;

	// Loop over all devices
	for( ;; ) {
		// Get 20 at a time
		hr = pEnumDevices->Next(10000, 20, pDevices, &uReturned);
		if( FAILED(hr) ) goto LCleanup;
		if( uReturned == 0 ) break;

		for( iDevice=0; iDevice<uReturned; iDevice++ ) {
			// For each device, get its device ID
			hr = pDevices[iDevice]->Get(bstrDeviceID, 0L, &var, NULL, NULL);
			if( SUCCEEDED(hr) && var.vt == VT_BSTR && var.bstrVal != NULL ) {
				// Check if the device ID contains "IG_".  If it does, then it's an XInput device
				if( wcsstr(var.bstrVal, L"IG_") ) {
					// If it does, then get the VID/PID from var.bstrVal
					DWORD dwPid = 0, dwVid = 0;
					WCHAR* strVid = wcsstr( var.bstrVal, L"VID_" );
					if( strVid && swscanf( strVid, L"VID_%4X", &dwVid ) != 1 )
						dwVid = 0;
					WCHAR* strPid = wcsstr( var.bstrVal, L"PID_" );
					if( strPid && swscanf( strPid, L"PID_%4X", &dwPid ) != 1 )
						dwPid = 0;

					// Compare the VID/PID to the DInput device
					if( dwVendorId == dwVid && dwProductId == dwPid )
					{
						bIsXinputDevice = true;
						goto LCleanup;
					}
				}
			}
			if( pDevices[iDevice] ) {
				pDevices[iDevice]->Release();
				pDevices[iDevice] = NULL;
			}
		}
	}

LCleanup:
	if( bstrNamespace ) SysFreeString(bstrNamespace);
	if( bstrDeviceID ) SysFreeString(bstrDeviceID);
	if( bstrClassName ) SysFreeString(bstrClassName);
	for( iDevice=0; iDevice<20; iDevice++ ) {
		if( pDevices[iDevice] ) pDevices[iDevice]->Release();
	}
	if( pEnumDevices ) pEnumDevices->Release();
	if( pIWbemLocator ) pIWbemLocator->Release();
	if( pIWbemServices ) pIWbemServices->Release();
	if( bCleanupCOM ) CoUninitialize();
	return bIsXinputDevice;
}

#endif // FEATURE_INPUT_IMPROVED
