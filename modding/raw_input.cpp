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
#include "modding/raw_input.h"
#include "global/vars.h"

#ifdef FEATURE_INPUT_IMPROVED
#ifdef __cplusplus
extern "C" {
#endif

#include <hidusage.h>
#include <hidpi.h>

#ifdef __cplusplus
} /* extern "C" */
#endif

#include "global/md5.h"

static HANDLE hMutex = NULL;
static HWND hRawInputWindow = NULL;
static HANDLE hRawInputDevice = INVALID_HANDLE_VALUE;
static PTCHAR lpRawInputDeviceName = NULL;

static struct {
	UINT numButtons;
	BOOL buttons[32];
	LONG rangeX,  valueX;
	LONG rangeY,  valueY;
	LONG rangeZ,  valueZ;
	LONG rangeRX, valueRX;
	LONG rangeRY, valueRY;
	LONG rangeRZ, valueRZ;
	LONG rangeDP, valueDP;
} RawState;

static bool CalculateRawGUID(LPCTSTR lpString, LPGUID lpGuid) {
	if( !lpString || !*lpString || !lpGuid ) return false;
	// use MD5 hash as RawInput Device GUID
	MD5_CTX mdContext;
	MD5Init(&mdContext);
	MD5Update(&mdContext, (unsigned char *)lpString, sizeof(TCHAR)*strlen(lpString));
	MD5Final(&mdContext);
	memcpy(lpGuid, mdContext.digest, sizeof(GUID));
	return true;
}

static LPVOID GetRawDeviceInfo(HANDLE hDevice, UINT uiCommand) {
	LPVOID pData = NULL;
	UINT ret, cbSize = 0;
	HANDLE hHeap = GetProcessHeap();
	ret = GetRawInputDeviceInfo(hDevice, uiCommand, NULL, &cbSize);
	if( (int)ret < 0 || cbSize == 0 ) return NULL;
	pData = HeapAlloc(hHeap, 0, uiCommand == RIDI_DEVICENAME ? sizeof(TCHAR)*cbSize : cbSize);
	if( pData == NULL) return NULL;
	ret = GetRawInputDeviceInfo(hDevice, uiCommand, pData, &cbSize);
	if( (int)ret < 0 ) {
		HeapFree(hHeap, 0, pData);
		pData = NULL;
	}
	return pData;
}

static bool ParseRawInputData(PRAWINPUT pRawInput) {
	bool result = false;
	PHIDP_PREPARSED_DATA pPreparsedData = NULL;
	PHIDP_BUTTON_CAPS pButtonCaps = NULL;
	PHIDP_VALUE_CAPS pValueCaps = NULL;
	HIDP_CAPS caps;
	USHORT capsLen;
	ULONG value;
	USAGE usage[128];
	HANDLE hHeap = GetProcessHeap();

	if( pRawInput == NULL ) return false;
	pPreparsedData = (PHIDP_PREPARSED_DATA)GetRawDeviceInfo(pRawInput->header.hDevice, RIDI_PREPARSEDDATA);
	if( pPreparsedData == NULL ) return false;
	if( HIDP_STATUS_SUCCESS != HidP_GetCaps(pPreparsedData, &caps) ) goto CLEANUP;

	// Button caps
	pButtonCaps = (PHIDP_BUTTON_CAPS)HeapAlloc(hHeap, 0, sizeof(HIDP_BUTTON_CAPS) * caps.NumberInputButtonCaps);
	if( pButtonCaps == NULL ) goto CLEANUP;
	capsLen = caps.NumberInputButtonCaps;
	if( HIDP_STATUS_SUCCESS != HidP_GetButtonCaps(HidP_Input, pButtonCaps, &capsLen, pPreparsedData) ) goto CLEANUP;
	RawState.numButtons = pButtonCaps->Range.UsageMax - pButtonCaps->Range.UsageMin + 1;

	// Button states
	value = RawState.numButtons;
	if( HIDP_STATUS_SUCCESS != HidP_GetUsages(HidP_Input, pButtonCaps->UsagePage, 0, usage, &value,
		 pPreparsedData,(PCHAR)pRawInput->data.hid.bRawData, pRawInput->data.hid.dwSizeHid) ) goto CLEANUP;
	memset(RawState.buttons, 0, sizeof(RawState.buttons));
	for( UINT i = 0; i < MIN(value, 32); ++i ) {
		RawState.buttons[usage[i] - pButtonCaps->Range.UsageMin] = TRUE;
	}

	// Axes and D-Pad caps
	pValueCaps = (PHIDP_VALUE_CAPS)HeapAlloc(hHeap, 0, sizeof(HIDP_VALUE_CAPS) * caps.NumberInputValueCaps);
	if( pValueCaps == NULL ) goto CLEANUP;
	capsLen = caps.NumberInputValueCaps;
	if( HIDP_STATUS_SUCCESS != HidP_GetValueCaps(HidP_Input, pValueCaps, &capsLen, pPreparsedData) ) goto CLEANUP;

	// Axes and D-Pad values
	for( UINT i = 0; i < caps.NumberInputValueCaps; ++i ) {
		if( HIDP_STATUS_SUCCESS != HidP_GetUsageValue(HidP_Input, pValueCaps[i].UsagePage,
			0, pValueCaps[i].Range.UsageMin, &value, pPreparsedData,
			(PCHAR)pRawInput->data.hid.bRawData, pRawInput->data.hid.dwSizeHid) ) continue;

		switch( pValueCaps[i].Range.UsageMin ) {
			case '0': // X-axis
				RawState.valueX = (UWORD)value - pValueCaps[i].LogicalMin;
				RawState.rangeX = 1 + (UWORD)pValueCaps[i].LogicalMax - pValueCaps[i].LogicalMin;
				break;
			case '1': // Y-axis
				RawState.valueY = (UWORD)value - pValueCaps[i].LogicalMin;
				RawState.rangeY = 1 + (UWORD)pValueCaps[i].LogicalMax - pValueCaps[i].LogicalMin;
				break;
			case '2': // Z-axis
				RawState.valueZ = (UWORD)value - pValueCaps[i].LogicalMin;
				RawState.rangeZ = 1 + (UWORD)pValueCaps[i].LogicalMax - pValueCaps[i].LogicalMin;
				break;
			case '3': // RX-axis
				RawState.valueRX = (UWORD)value - pValueCaps[i].LogicalMin;
				RawState.rangeRX = 1 + (UWORD)pValueCaps[i].LogicalMax - pValueCaps[i].LogicalMin;
				break;
			case '4': // RY-axis
				RawState.valueRY = (UWORD)value - pValueCaps[i].LogicalMin;
				RawState.rangeRY = 1 + (UWORD)pValueCaps[i].LogicalMax - pValueCaps[i].LogicalMin;
				break;
			case '5': // RZ-axis
				RawState.valueRZ = (UWORD)value - pValueCaps[i].LogicalMin;
				RawState.rangeRZ = 1 + (UWORD)pValueCaps[i].LogicalMax - pValueCaps[i].LogicalMin;
				break;
			case '9': // D-Pad
				if( value < (UWORD)pValueCaps[i].LogicalMin || value > (UWORD)pValueCaps[i].LogicalMax ) {
					RawState.valueDP = -1;
				} else {
					RawState.valueDP = (UWORD)value - pValueCaps[i].LogicalMin;
				}
				RawState.rangeDP = 1 + (UWORD)pValueCaps[i].LogicalMax - pValueCaps[i].LogicalMin;
				break;
		}
	}
	result = true;

CLEANUP:
	if( pValueCaps != NULL ) HeapFree(hHeap, 0, pValueCaps);
	if( pButtonCaps != NULL ) HeapFree(hHeap, 0, pButtonCaps);
	if( pPreparsedData != NULL ) HeapFree(hHeap, 0, pPreparsedData);

	return result;
}

bool RawInputEnumerate(BOOL(CALLBACK *callback)(HANDLE, LPGUID, PRID_DEVICE_INFO_HID, LPVOID), LPVOID lpContext) {
	PRAWINPUTDEVICELIST devList = NULL;
	UINT ret, cbSize, numDevices = 0;
	HANDLE hHeap = GetProcessHeap();

	if( callback == NULL ) return false;
	ret = GetRawInputDeviceList(NULL, &numDevices, sizeof(RAWINPUTDEVICELIST));
	if( (int)ret < 0 ) return false;
	if( numDevices == 0 ) return true;
	devList = (PRAWINPUTDEVICELIST)HeapAlloc(hHeap, 0, sizeof(RAWINPUTDEVICELIST)*numDevices);
	if( devList == NULL ) return false;

	memset(devList, 0, sizeof(RAWINPUTDEVICELIST)*numDevices);
	ret = GetRawInputDeviceList(devList, &numDevices, sizeof(RAWINPUTDEVICELIST));
	if( (int)ret < 0 ) {
		HeapFree(hHeap, 0, devList);
		return false;
	}

	for( UINT i=0; i<numDevices; ++i ) {
		if( devList[i].dwType != RIM_TYPEHID ) continue;
		RID_DEVICE_INFO info;
		memset(&info, 0, sizeof(info));
		info.cbSize = cbSize = sizeof(info);
		ret = GetRawInputDeviceInfo(devList[i].hDevice, RIDI_DEVICEINFO, &info, &cbSize);
		if( (int)ret >= 0 && info.dwType == RIM_TYPEHID && info.hid.usUsagePage == HID_USAGE_PAGE_GENERIC
			&& (info.hid.usUsage == HID_USAGE_GENERIC_JOYSTICK || info.hid.usUsage == HID_USAGE_GENERIC_GAMEPAD) )
		{
			GUID guid;
			PTCHAR deviceName = (PTCHAR)GetRawDeviceInfo(devList[i].hDevice, RIDI_DEVICENAME);
			bool ready = CalculateRawGUID(deviceName, &guid);
			if( deviceName != NULL ) HeapFree(hHeap, 0, deviceName);
			if( ready && !callback(devList[i].hDevice, &guid, &info.hid, lpContext) ) break;
		}
	}

	HeapFree(hHeap, 0, devList);
	return true;
}

bool RawInputStart(HWND hWnd, HANDLE hDevice) {
	if( hMutex == NULL ) {
		hMutex = CreateMutex(NULL, FALSE, NULL);
		if( hMutex == NULL ) return false;
	}
	WaitForSingleObject(hMutex, INFINITE);

	bool result = false;
	if( hRawInputWindow == hWnd && hRawInputDevice == hDevice ) {
		// everything is fine, no need to restart
		result = true;
		goto CLEANUP;
	} else if( hRawInputDevice != INVALID_HANDLE_VALUE ) {
		// need to call RawInputStop first
		goto CLEANUP;
	}
	if( hRawInputWindow != hWnd ) {
		RAWINPUTDEVICE rid[2] = {
			{HID_USAGE_PAGE_GENERIC, HID_USAGE_GENERIC_JOYSTICK, 0, hWnd},
			{HID_USAGE_PAGE_GENERIC, HID_USAGE_GENERIC_GAMEPAD, 0, hWnd},
		};
		if( !RegisterRawInputDevices(rid, ARRAY_SIZE(rid), sizeof(*rid)) ) goto CLEANUP;
		hRawInputWindow = hWnd;
	}

	lpRawInputDeviceName = (PTCHAR)GetRawDeviceInfo(hDevice, RIDI_DEVICENAME);
	if( lpRawInputDeviceName == NULL ) goto CLEANUP;
	hRawInputDevice = hDevice;
	memset(&RawState, 0, sizeof(RawState));
	result = true;

CLEANUP:
	ReleaseMutex(hMutex);
	return result;
}

void RawInputStop() {
	if( hMutex == NULL ) return;
	WaitForSingleObject(hMutex, INFINITE);
	HANDLE hHeap = GetProcessHeap();
	// it's better to leave hRawInputWindow unchanged (cannot unregister it anyway)
	hRawInputDevice = INVALID_HANDLE_VALUE;
	if( lpRawInputDeviceName != NULL ) {
		HeapFree(hHeap, 0, lpRawInputDeviceName);
		lpRawInputDeviceName = NULL;
	}
	ReleaseMutex(hMutex);
}

bool RawInputReceive(HWND hWnd, HRAWINPUT hRawInput) {
	if( hMutex == NULL ) return false;
	WaitForSingleObject(hMutex, INFINITE);
	HANDLE hHeap = GetProcessHeap();

	bool result = false;
	PRAWINPUT pRawInput = NULL;
	UINT ret, cbSize = 0;
	if( hWnd != hRawInputWindow ) goto CLEANUP;
	ret = GetRawInputData(hRawInput, RID_INPUT, NULL, &cbSize, sizeof(RAWINPUTHEADER));
	if( (int)ret < 0 || cbSize == 0 ) goto CLEANUP;
	pRawInput = (PRAWINPUT)HeapAlloc(hHeap, 0, cbSize);
	if( pRawInput == NULL) goto CLEANUP;
	ret = GetRawInputData(hRawInput, RID_INPUT, pRawInput, &cbSize, sizeof(RAWINPUTHEADER));
	if( (int)ret >= 0 && pRawInput->header.hDevice == hRawInputDevice ) {
		result = ParseRawInputData(pRawInput);
	}

CLEANUP:
	if( pRawInput != NULL ) HeapFree(hHeap, 0, pRawInput);
	ReleaseMutex(hMutex);
	return result;
}

bool RawInputSend(WORD leftMotor, WORD rightMotor, DWORD color) {
	if( hMutex == NULL ) return false;
	WaitForSingleObject(hMutex, INFINITE);

	bool result = false;
	HANDLE hDevice = INVALID_HANDLE_VALUE;
	DWORD bytesWritten;
	BYTE buf[32] = {
		0x05, 0xFF, 0x00, 0x00,
		(BYTE)(rightMotor>>8),
		(BYTE)(leftMotor>>8),
		(BYTE)(RGB_GETRED(color)),
		(BYTE)(RGB_GETGREEN(color)),
		(BYTE)(RGB_GETBLUE(color)),
		0, // all other bytes are zeros
	};
	if( lpRawInputDeviceName == NULL ) goto CLEANUP;
	hDevice = CreateFile(lpRawInputDeviceName, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if( hDevice == INVALID_HANDLE_VALUE ) goto CLEANUP;
	WriteFile(hDevice, buf, sizeof(buf), &bytesWritten, NULL);
	CloseHandle(hDevice);
	result = (bytesWritten == sizeof(buf));

CLEANUP:
	ReleaseMutex(hMutex);
	return result;
}

bool RawInputGetState(RINPUT_STATE *pState) {
	if( !pState || hMutex == NULL ) return false;
	WaitForSingleObject(hMutex, INFINITE);
	if( hRawInputDevice == INVALID_HANDLE_VALUE ) {
		ReleaseMutex(hMutex);
		return false;
	}

	memset(pState, 0, sizeof(RINPUT_STATE));
	if( RawState.rangeDP && RawState.valueDP >= 0 ) {
		pState->dPad = 36000 * RawState.valueDP / RawState.rangeDP;
	} else {
		pState->dPad = -1;
	}

	pState->btnSquare = RawState.buttons[0];
	pState->btnCross = RawState.buttons[1];
	pState->btnCircle = RawState.buttons[2];
	pState->btnTriangle = RawState.buttons[3];
	pState->btnL1 = RawState.buttons[4];
	pState->btnR1 = RawState.buttons[5];
	pState->btnL2 = RawState.buttons[6];
	pState->btnR2 = RawState.buttons[7];
	pState->btnShare = RawState.buttons[8];
	pState->btnOptions = RawState.buttons[9];
	pState->btnL3 = RawState.buttons[10];
	pState->btnR3 = RawState.buttons[11];
	pState->btnPS = RawState.buttons[12];

	if( RawState.rangeX  ) pState->axisLX = (float)(RawState.valueX*2 - RawState.rangeX) / (float)RawState.rangeX;
	if( RawState.rangeY  ) pState->axisLY = (float)(RawState.rangeY - RawState.valueY*2) / (float)RawState.rangeY;
	if( RawState.rangeZ  ) pState->axisRX = (float)(RawState.valueZ*2 - RawState.rangeZ) / (float)RawState.rangeZ;
	if( RawState.rangeRZ ) pState->axisRY = (float)(RawState.rangeRZ - RawState.valueRZ*2) / (float)RawState.rangeRZ;
	if( RawState.rangeRX ) pState->axisL2 = (float)RawState.valueRX / (float)RawState.rangeRX;
	if( RawState.rangeRY ) pState->axisR2 = (float)RawState.valueRY / (float)RawState.rangeRY;

	ReleaseMutex(hMutex);
	return true;
}

#endif // FEATURE_INPUT_IMPROVED
