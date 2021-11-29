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
#include "modding/raw_input.h"
#include "global/vars.h"

#ifdef FEATURE_INPUT_IMPROVED
#ifdef __cplusplus
extern "C" {
#endif

#include <hidusage.h>
#include <hidpi.h>
#include <hidsdi.h>
#include <setupapi.h>

#ifdef __cplusplus
} /* extern "C" */
#endif

#include "global/md5.h"

#define SONY_BT_REPORT_SIZE (78)

#define VID_SONY (0x054C)
#define PID_DUALSHOCK4_1 (0x05C4)
#define PID_DUALSHOCK4_2 (0x09CC)
#define PID_DUALSHOCK4_W (0x0BA0)
#define PID_DUALSENSE (0x0CE6)

#define VIA_USB " - USB"
#define VIA_BLUETOOTH " - Bluetooth"

#define NAME_DUALSHOCK4_1 "Sony DualShock 4 (1st gen)"
#define NAME_DUALSHOCK4_2 "Sony DualShock 4 (2nd gen)"
#define NAME_DUALSHOCK4_W "Sony DualShock 4 (adaptor)"
#define NAME_DUALSENSE "Sony DualSense"

typedef struct {
	UINT numButtons;
	bool buttons[32];
	LONG rangeX,  valueX;
	LONG rangeY,  valueY;
	LONG rangeZ,  valueZ;
	LONG rangeRX, valueRX;
	LONG rangeRY, valueRY;
	LONG rangeRZ, valueRZ;
	LONG rangeDP, valueDP;
} RAW_STATE;

typedef struct {
	WORD leftMotor;
	WORD rightMotor;
	DWORD color;
	bool updated;
} RAW_REPORT;

static DWORD SonyBluetoothCRC(LPCBYTE buf, DWORD len, DWORD crc) {
	static const DWORD lut[] = {
		0xD202EF8D, 0xA505DF1B, 0x3C0C8EA1, 0x4B0BBE37, 0xD56F2B94, 0xA2681B02, 0x3B614AB8, 0x4C667A2E,
		0xDCD967BF, 0xABDE5729, 0x32D70693, 0x45D03605, 0xDBB4A3A6, 0xACB39330, 0x35BAC28A, 0x42BDF21C,
		0xCFB5FFE9, 0xB8B2CF7F, 0x21BB9EC5, 0x56BCAE53, 0xC8D83BF0, 0xBFDF0B66, 0x26D65ADC, 0x51D16A4A,
		0xC16E77DB, 0xB669474D, 0x2F6016F7, 0x58672661, 0xC603B3C2, 0xB1048354, 0x280DD2EE, 0x5F0AE278,
		0xE96CCF45, 0x9E6BFFD3, 0x0762AE69, 0x70659EFF, 0xEE010B5C, 0x99063BCA, 0x000F6A70, 0x77085AE6,
		0xE7B74777, 0x90B077E1, 0x09B9265B, 0x7EBE16CD, 0xE0DA836E, 0x97DDB3F8, 0x0ED4E242, 0x79D3D2D4,
		0xF4DBDF21, 0x83DCEFB7, 0x1AD5BE0D, 0x6DD28E9B, 0xF3B61B38, 0x84B12BAE, 0x1DB87A14, 0x6ABF4A82,
		0xFA005713, 0x8D076785, 0x140E363F, 0x630906A9, 0xFD6D930A, 0x8A6AA39C, 0x1363F226, 0x6464C2B0,
		0xA4DEAE1D, 0xD3D99E8B, 0x4AD0CF31, 0x3DD7FFA7, 0xA3B36A04, 0xD4B45A92, 0x4DBD0B28, 0x3ABA3BBE,
		0xAA05262F, 0xDD0216B9, 0x440B4703, 0x330C7795, 0xAD68E236, 0xDA6FD2A0, 0x4366831A, 0x3461B38C,
		0xB969BE79, 0xCE6E8EEF, 0x5767DF55, 0x2060EFC3, 0xBE047A60, 0xC9034AF6, 0x500A1B4C, 0x270D2BDA,
		0xB7B2364B, 0xC0B506DD, 0x59BC5767, 0x2EBB67F1, 0xB0DFF252, 0xC7D8C2C4, 0x5ED1937E, 0x29D6A3E8,
		0x9FB08ED5, 0xE8B7BE43, 0x71BEEFF9, 0x06B9DF6F, 0x98DD4ACC, 0xEFDA7A5A, 0x76D32BE0, 0x01D41B76,
		0x916B06E7, 0xE66C3671, 0x7F6567CB, 0x0862575D, 0x9606C2FE, 0xE101F268, 0x7808A3D2, 0x0F0F9344,
		0x82079EB1, 0xF500AE27, 0x6C09FF9D, 0x1B0ECF0B, 0x856A5AA8, 0xF26D6A3E, 0x6B643B84, 0x1C630B12,
		0x8CDC1683, 0xFBDB2615, 0x62D277AF, 0x15D54739, 0x8BB1D29A, 0xFCB6E20C, 0x65BFB3B6, 0x12B88320,
		0x3FBA6CAD, 0x48BD5C3B, 0xD1B40D81, 0xA6B33D17, 0x38D7A8B4, 0x4FD09822, 0xD6D9C998, 0xA1DEF90E,
		0x3161E49F, 0x4666D409, 0xDF6F85B3, 0xA868B525, 0x360C2086, 0x410B1010, 0xD80241AA, 0xAF05713C,
		0x220D7CC9, 0x550A4C5F, 0xCC031DE5, 0xBB042D73, 0x2560B8D0, 0x52678846, 0xCB6ED9FC, 0xBC69E96A,
		0x2CD6F4FB, 0x5BD1C46D, 0xC2D895D7, 0xB5DFA541, 0x2BBB30E2, 0x5CBC0074, 0xC5B551CE, 0xB2B26158,
		0x04D44C65, 0x73D37CF3, 0xEADA2D49, 0x9DDD1DDF, 0x03B9887C, 0x74BEB8EA, 0xEDB7E950, 0x9AB0D9C6,
		0x0A0FC457, 0x7D08F4C1, 0xE401A57B, 0x930695ED, 0x0D62004E, 0x7A6530D8, 0xE36C6162, 0x946B51F4,
		0x19635C01, 0x6E646C97, 0xF76D3D2D, 0x806A0DBB, 0x1E0E9818, 0x6909A88E, 0xF000F934, 0x8707C9A2,
		0x17B8D433, 0x60BFE4A5, 0xF9B6B51F, 0x8EB18589, 0x10D5102A, 0x67D220BC, 0xFEDB7106, 0x89DC4190,
		0x49662D3D, 0x3E611DAB, 0xA7684C11, 0xD06F7C87, 0x4E0BE924, 0x390CD9B2, 0xA0058808, 0xD702B89E,
		0x47BDA50F, 0x30BA9599, 0xA9B3C423, 0xDEB4F4B5, 0x40D06116, 0x37D75180, 0xAEDE003A, 0xD9D930AC,
		0x54D13D59, 0x23D60DCF, 0xBADF5C75, 0xCDD86CE3, 0x53BCF940, 0x24BBC9D6, 0xBDB2986C, 0xCAB5A8FA,
		0x5A0AB56B, 0x2D0D85FD, 0xB404D447, 0xC303E4D1, 0x5D677172, 0x2A6041E4, 0xB369105E, 0xC46E20C8,
		0x72080DF5, 0x050F3D63, 0x9C066CD9, 0xEB015C4F, 0x7565C9EC, 0x0262F97A, 0x9B6BA8C0, 0xEC6C9856,
		0x7CD385C7, 0x0BD4B551, 0x92DDE4EB, 0xE5DAD47D, 0x7BBE41DE, 0x0CB97148, 0x95B020F2, 0xE2B71064,
		0x6FBF1D91, 0x18B82D07, 0x81B17CBD, 0xF6B64C2B, 0x68D2D988, 0x1FD5E91E, 0x86DCB8A4, 0xF1DB8832,
		0x616495A3, 0x1663A535, 0x8F6AF48F, 0xF86DC419, 0x660951BA, 0x110E612C, 0x88073096, 0xFF000000,
	};
	for( DWORD i = 0; i < len; ++i ) {
		crc = lut[((BYTE)crc) ^ ((BYTE)buf[i])] ^ (crc >> 8);
	}
	return crc;
}

static bool IsRawBluetooth(PHIDD_ATTRIBUTES pAttr, PHIDP_CAPS pCaps) {
	if( !pAttr || !pCaps ) return false;
	switch( pAttr->VendorID ) {
	case VID_SONY:
		switch( pAttr->ProductID ) {
		case PID_DUALSHOCK4_1:
		case PID_DUALSHOCK4_2:
		case PID_DUALSHOCK4_W:
		case PID_DUALSENSE:
			if( pCaps->InputReportByteLength >= SONY_BT_REPORT_SIZE &&
				pCaps->OutputReportByteLength >= SONY_BT_REPORT_SIZE )
			{
				return true;
			}
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	return false;
}

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

LPCTSTR GetRawInputName(WORD vid, WORD pid, BOOL bt) {
	switch( vid ) {
	case VID_SONY:
		switch( pid ) {
		case PID_DUALSHOCK4_1:
			return bt ? NAME_DUALSHOCK4_1 VIA_BLUETOOTH : NAME_DUALSHOCK4_1 VIA_USB;
		case PID_DUALSHOCK4_2:
			return bt ? NAME_DUALSHOCK4_2 VIA_BLUETOOTH : NAME_DUALSHOCK4_2 VIA_USB;
		case PID_DUALSHOCK4_W:
			return bt ? NAME_DUALSHOCK4_W VIA_BLUETOOTH : NAME_DUALSHOCK4_W VIA_USB;
		case PID_DUALSENSE:
			return bt ? NAME_DUALSENSE VIA_BLUETOOTH : NAME_DUALSENSE VIA_USB;
		default:
			break;
		}
		break;
	default:
		break;
	}
	return NULL;
}

bool RawInputEnumerate(BOOL(CALLBACK *callback)(LPGUID, LPCTSTR, LPCTSTR, WORD, WORD, LPVOID), LPVOID lpContext) {
	if( callback == NULL ) return false;
	GUID hidGuid;
	HidD_GetHidGuid(&hidGuid);
	HANDLE hDevs = SetupDiGetClassDevs(&hidGuid, NULL, NULL, DIGCF_PRESENT|DIGCF_DEVICEINTERFACE);
	if( hDevs == INVALID_HANDLE_VALUE ) return false;
	DWORD devIndex = 0;
	SP_DEVINFO_DATA devInfo;
	devInfo.cbSize = sizeof(SP_DEVINFO_DATA);
	while( SetupDiEnumDeviceInfo(hDevs, devIndex++, &devInfo) ) {
		DWORD ifaceIndex = 0;
		SP_DEVICE_INTERFACE_DATA ifaceInfo;
		ifaceInfo.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		while( SetupDiEnumDeviceInterfaces(hDevs, &devInfo, &hidGuid, ifaceIndex++, &ifaceInfo) ) {
			DWORD detailSize = 0;
			SetupDiGetDeviceInterfaceDetail(hDevs, &ifaceInfo, NULL, 0, &detailSize, NULL);
			PSP_DEVICE_INTERFACE_DETAIL_DATA pDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(detailSize);
			pDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
			SetupDiGetDeviceInterfaceDetail(hDevs, &ifaceInfo, pDetail, detailSize, NULL, NULL);
			HANDLE hDevice = CreateFile(pDetail->DevicePath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
			if( hDevice != INVALID_HANDLE_VALUE ) {
				PHIDP_PREPARSED_DATA pPreparsed = NULL;
				HIDD_ATTRIBUTES attr;
				HIDP_CAPS caps;
				bool fetched = (HidD_GetAttributes(hDevice, &attr)
					&& HidD_GetPreparsedData(hDevice, &pPreparsed)
					&& HIDP_STATUS_SUCCESS == HidP_GetCaps(pPreparsed, &caps));
				if( pPreparsed != NULL ) HidD_FreePreparsedData(pPreparsed);
				CloseHandle(hDevice);
				if( fetched && caps.UsagePage == HID_USAGE_PAGE_GENERIC
					&& (caps.Usage == HID_USAGE_GENERIC_JOYSTICK || caps.Usage == HID_USAGE_GENERIC_GAMEPAD) )
				{
					GUID deviceGuid;
					LPCTSTR productName = GetRawInputName(attr.VendorID, attr.ProductID, IsRawBluetooth(&attr, &caps));
					if( productName != NULL && CalculateRawGUID(pDetail->DevicePath, &deviceGuid) ) {
						callback(&deviceGuid, pDetail->DevicePath, productName, attr.VendorID, attr.ProductID, lpContext);
					}
				}
			}
			free(pDetail);
		}
	}
	SetupDiDestroyDeviceInfoList(hDevs);
	return true;
}

// --------------------
// RAW HID Device class
// --------------------

class RawHidDevice {
	private:
		// fields
		bool isStop = false;
		bool isInit = false;
		bool isConnected = false;
		bool isBluetooth = false;
		HANDLE hMutex = NULL;
		HANDLE hThread = NULL;
		LPTSTR lpDeviceName = NULL;
		HANDLE hDeviceFile = INVALID_HANDLE_VALUE;
		PHIDP_PREPARSED_DATA pPreparsedData = NULL;
		PHIDP_BUTTON_CAPS pButtonCaps = NULL;
		PHIDP_VALUE_CAPS pValueCaps = NULL;
		HIDD_ATTRIBUTES HidAttr;
		HIDP_CAPS HidCaps;
		LPBYTE pRawInput = NULL;
		RAW_STATE RawState;
		RAW_REPORT RawReport;
		// methods
		static DWORD WINAPI StaticTask(CONST LPVOID lpParam) {
			RawHidDevice *This = (RawHidDevice *)lpParam;
			return This->Task();
		}
		DWORD Task();
		bool Connect();
		void Disconnect(bool release=false);
		bool Reconnect();
		bool Send(LPCVOID lpBuffer, DWORD nSize);
		bool Receive(LPVOID lpBuffer, DWORD nSize);
		bool Calibrate();
		bool SonyControllerReport(LPBYTE buf, DWORD bufLen, DWORD productId, RAW_REPORT *pReport);
		bool ParseRawInputStandard(LPBYTE buf, DWORD bufLen, RAW_STATE *pState);
		bool ParseRawInputSonyDualShock4(LPBYTE buf, DWORD bufLen, RAW_STATE *pState);
		bool ParseRawInputSonyDualSense(LPBYTE buf, DWORD bufLen, RAW_STATE *pState);
		bool SendRawReport(bool clean=false);
		bool ReceiveRawInput();
	public:
		bool IsRunning();
		bool Start(LPCTSTR lpName);
		void Stop();
		bool SetState(WORD leftMotor, WORD rightMotor, DWORD colors);
		bool GetState(RINPUT_STATE *pState);
};

DWORD RawHidDevice::Task() {
	Connect();
	while( !isStop ) {
		if( isConnected ) {
			ReceiveRawInput();
			SendRawReport();
		} else if( !Reconnect() )  {
			Sleep(100);
		}
	}
	Disconnect(true);
	return 0;
}

bool RawHidDevice::Connect() {
	if( hDeviceFile == INVALID_HANDLE_VALUE ) {
		hDeviceFile = CreateFile(lpDeviceName, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	}
	isConnected = (hDeviceFile != INVALID_HANDLE_VALUE);
	if( isConnected && !isInit ) {
		HANDLE hHeap = GetProcessHeap();
		if( !HidD_GetAttributes(hDeviceFile, &HidAttr) ||
			!HidD_GetPreparsedData(hDeviceFile, &pPreparsedData) ||
			HIDP_STATUS_SUCCESS != HidP_GetCaps(pPreparsedData, &HidCaps) )
		{
			Disconnect(true);
			return false;
		}
		isBluetooth = IsRawBluetooth(&HidAttr, &HidCaps);
		pRawInput = (LPBYTE)HeapAlloc(hHeap, 0, HidCaps.InputReportByteLength);
		pButtonCaps = (PHIDP_BUTTON_CAPS)HeapAlloc(hHeap, 0, sizeof(HIDP_BUTTON_CAPS) * HidCaps.NumberInputButtonCaps);
		pValueCaps = (PHIDP_VALUE_CAPS)HeapAlloc(hHeap, 0, sizeof(HIDP_VALUE_CAPS) * HidCaps.NumberInputValueCaps);
		isInit = true;
	}
	if( !isConnected ) {
		return false;
	}
	Calibrate();
	if( isBluetooth ) {
		SendRawReport(true);
	}
	return true;
}

void RawHidDevice::Disconnect(bool release) {
	if( release ) {
		HANDLE hHeap = GetProcessHeap();
		if( pPreparsedData != NULL ) {
			HidD_FreePreparsedData(pPreparsedData);
			pPreparsedData = NULL;
		}
		if( pButtonCaps != NULL ) {
			HeapFree(hHeap, 0, pButtonCaps);
			pButtonCaps = NULL;
		}
		if( pValueCaps != NULL ) {
			HeapFree(hHeap, 0, pValueCaps);
			pValueCaps = NULL;
		}
		if( pRawInput != NULL ) {
			HeapFree(hHeap, 0, pRawInput);
			pRawInput = NULL;
		}
		memset(&HidAttr, 0, sizeof(HidAttr));
		memset(&HidCaps, 0, sizeof(HidCaps));
		isInit = false;
	}
	if( hDeviceFile != INVALID_HANDLE_VALUE ) {
		CloseHandle(hDeviceFile);
		hDeviceFile = INVALID_HANDLE_VALUE;
	}
	isConnected = false;
}

bool RawHidDevice::Reconnect() {
	Calibrate(); // kick OS to refresh lost device state
	Disconnect();
	return Connect();
}

bool RawHidDevice::Send(LPCVOID lpBuffer, DWORD nSize) {
	OVERLAPPED overlapped;
	memset(&overlapped, 0, sizeof(overlapped));
	WriteFile(hDeviceFile, lpBuffer, nSize, NULL, &overlapped);
	for( int i=0; i<50; ++i ) {
		Sleep(1);
		DWORD bytesWritten = 0;
		BOOL res = GetOverlappedResult(hDeviceFile, &overlapped, &bytesWritten, FALSE);
		if( res && bytesWritten >= nSize ) {
			return true;
		}
	}
	isConnected = false;
	return false;
}

bool RawHidDevice::Receive(LPVOID lpBuffer, DWORD nSize) {
	OVERLAPPED overlapped;
	memset(&overlapped, 0, sizeof(overlapped));
	ReadFile(hDeviceFile, lpBuffer, nSize, NULL, &overlapped);
	for( int i=0; i<50; ++i ) {
		Sleep(1);
		DWORD bytesRead = 0;
		BOOL res = GetOverlappedResult(hDeviceFile, &overlapped, &bytesRead, FALSE);
		if( res && bytesRead >= nSize ) {
			return true;
		}
	}
	isConnected = false;
	return false;
}

bool RawHidDevice::Calibrate() {
	if( !isInit || hDeviceFile == INVALID_HANDLE_VALUE ) return false;
	// We don't need the feature data itself, we need this to unlock advanced controls
	bool result = true;
	switch( HidAttr.VendorID ) {
	case VID_SONY:
		switch( HidAttr.ProductID ) {
		case PID_DUALSHOCK4_1:
		case PID_DUALSHOCK4_2:
		case PID_DUALSHOCK4_W:
			{
				BYTE buf[41] = {0x02};
				result = HidD_GetFeature(hDeviceFile, buf, sizeof(buf));
			}
			break;
		case PID_DUALSENSE:
			{
				BYTE buf[41] = {0x05};
				result = HidD_GetFeature(hDeviceFile, buf, sizeof(buf));
			}
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	return result;
}

bool RawHidDevice::SonyControllerReport(LPBYTE buf, DWORD bufLen, DWORD productId, RAW_REPORT *pReport) {
	LPBYTE data = NULL;
	memset(buf, 0, bufLen);
	switch( productId ) {
	case PID_DUALSHOCK4_1:
	case PID_DUALSHOCK4_2:
	case PID_DUALSHOCK4_W:
		if( isBluetooth ) {
			buf[0] = 0x11;
			buf[1] = 0xC0;
			buf[2] = 0xA0;
			data = buf+3;
		} else if( bufLen == 32 ) {
			buf[0] = 0x05;
			data = buf+1;
		} else {
			return false;
		}
		if( pReport == NULL ) {
			data[0] = 0xF4;
		} else {
			data[0] = 0xF7;
			data[3] = (BYTE)(pReport->rightMotor>>8);
			data[4] = (BYTE)(pReport->leftMotor>>8);
			data[5] = (BYTE)(RGB_GETRED(pReport->color));
			data[6] = (BYTE)(RGB_GETGREEN(pReport->color));
			data[7] = (BYTE)(RGB_GETBLUE(pReport->color));
		}
		break;
	case PID_DUALSENSE:
		if( isBluetooth ) {
			buf[0] = 0x31;
			buf[1] = 0x02;
			data = buf+2;
		} else if( bufLen == 48 ) {
			buf[0] = 0x02;
			data = buf+1;
		} else {
			return false;
		}
		if( pReport == NULL ) {
			data[1] = 0x15;
		} else {
			data[0] = 0x0F;
			data[1] = 0x55;
			data[2] = (BYTE)(pReport->rightMotor>>8);
			data[3] = (BYTE)(pReport->leftMotor>>8);
			data[8] = 0x00; // Mute LED: [Off]=0, [On]=1, [Pulse]=2
			// 10 - 19: right trigger feedback
			// 21 - 30: left trigger feedback
			data[36] = 0x02; // Haptic intensity: [High]=0, [Medium]=2, [Low]=5
			data[38] = 0x02; // LED mode: [LED static brightness in 42]=1,  [Blue LED fade in 41]=2
			data[41] = 0x02; // LED fade: [Fade in]=1, [Fade out]=2
			data[42] = 0x02; // LED brightness: [High]=0, [Medium]=1, [Low]=2
			data[43] = 0x00; // Player LED mask
			data[44] = (BYTE)(RGB_GETRED(pReport->color));
			data[45] = (BYTE)(RGB_GETGREEN(pReport->color));
			data[46] = (BYTE)(RGB_GETBLUE(pReport->color));
		}
		break;
	default:
		return false;
	}
	if( isBluetooth ) {
		DWORD crc = 0;
		BYTE btheader[] = {0xA2}; // (format 4:2:2) transaction_type=0xA (data), parameters=0x0 (none), report_type=0x2 (output)
		crc = SonyBluetoothCRC(btheader, sizeof(btheader), crc);
		crc = SonyBluetoothCRC(buf, SONY_BT_REPORT_SIZE-4, crc);
		for( int i=4; i>=1; --i ) {
			buf[SONY_BT_REPORT_SIZE - i] = (BYTE)crc;
			crc >>= 8;
		}
	}
	return true;
}

bool RawHidDevice::ParseRawInputStandard(LPBYTE buf, DWORD bufLen, RAW_STATE *pState) {
	WORD capsLen;
	DWORD value;
	USAGE usage[128];

	if( pState == NULL || pPreparsedData == NULL || pButtonCaps == NULL || pValueCaps == NULL ) return false;

	// Button caps
	capsLen = HidCaps.NumberInputButtonCaps;
	if( HIDP_STATUS_SUCCESS != HidP_GetButtonCaps(HidP_Input, pButtonCaps, &capsLen, pPreparsedData) ) return false;
	pState->numButtons = pButtonCaps->Range.UsageMax - pButtonCaps->Range.UsageMin + 1;

	// Button states
	value = pState->numButtons;
	if( HIDP_STATUS_SUCCESS != HidP_GetUsages(HidP_Input, pButtonCaps->UsagePage,
		0, usage, &value, pPreparsedData, (PCHAR)buf, bufLen) ) return false;
	memset(pState->buttons, 0, sizeof(pState->buttons));
	for( UINT i = 0; i < MIN(value, 32); ++i ) {
		pState->buttons[usage[i] - pButtonCaps->Range.UsageMin] = true;
	}

	// Axes and D-Pad caps
	capsLen = HidCaps.NumberInputValueCaps;
	if( HIDP_STATUS_SUCCESS != HidP_GetValueCaps(HidP_Input, pValueCaps, &capsLen, pPreparsedData) ) return false;

	// Axes and D-Pad values
	for( UINT i = 0; i < HidCaps.NumberInputValueCaps; ++i ) {
		if( HIDP_STATUS_SUCCESS != HidP_GetUsageValue(HidP_Input, pValueCaps[i].UsagePage,
			0, pValueCaps[i].Range.UsageMin, &value, pPreparsedData, (PCHAR)buf, bufLen) ) continue;

		switch( pValueCaps[i].Range.UsageMin ) {
		case '0': // X-axis
			pState->valueX = (UWORD)value - pValueCaps[i].LogicalMin;
			pState->rangeX = 1 + (UWORD)pValueCaps[i].LogicalMax - pValueCaps[i].LogicalMin;
			break;
		case '1': // Y-axis
			pState->valueY = (UWORD)value - pValueCaps[i].LogicalMin;
			pState->rangeY = 1 + (UWORD)pValueCaps[i].LogicalMax - pValueCaps[i].LogicalMin;
			break;
		case '2': // Z-axis
			pState->valueZ = (UWORD)value - pValueCaps[i].LogicalMin;
			pState->rangeZ = 1 + (UWORD)pValueCaps[i].LogicalMax - pValueCaps[i].LogicalMin;
			break;
		case '3': // RX-axis
			pState->valueRX = (UWORD)value - pValueCaps[i].LogicalMin;
			pState->rangeRX = 1 + (UWORD)pValueCaps[i].LogicalMax - pValueCaps[i].LogicalMin;
			break;
		case '4': // RY-axis
			pState->valueRY = (UWORD)value - pValueCaps[i].LogicalMin;
			pState->rangeRY = 1 + (UWORD)pValueCaps[i].LogicalMax - pValueCaps[i].LogicalMin;
			break;
		case '5': // RZ-axis
			pState->valueRZ = (UWORD)value - pValueCaps[i].LogicalMin;
			pState->rangeRZ = 1 + (UWORD)pValueCaps[i].LogicalMax - pValueCaps[i].LogicalMin;
			break;
		case '9': // D-Pad
			if( value < (UWORD)pValueCaps[i].LogicalMin || value > (UWORD)pValueCaps[i].LogicalMax ) {
				pState->valueDP = -1;
			} else {
				pState->valueDP = (UWORD)value - pValueCaps[i].LogicalMin;
			}
			pState->rangeDP = 1 + (UWORD)pValueCaps[i].LogicalMax - pValueCaps[i].LogicalMin;
			break;
		}
	}
	return true;
}

bool RawHidDevice::ParseRawInputSonyDualShock4(LPBYTE buf, DWORD bufLen, RAW_STATE *pState) {
	if( buf == NULL || bufLen < 16 || pState == NULL ) return false;
	memset(pState, 0, sizeof(RAW_STATE));

	// Axes
	pState->rangeX = pState->rangeY = pState->rangeZ = 0x100;
	pState->rangeRX = pState->rangeRY = pState->rangeRZ = 0x100;
	pState->valueX = buf[1];
	pState->valueY = buf[2];
	pState->valueZ = buf[3];
	pState->valueRZ = buf[4];
	pState->valueRX = buf[8];
	pState->valueRY = buf[9];

	// D-Pad
	pState->rangeDP = 8;
	pState->valueDP = (buf[5] & 0x0F);
	if( pState->valueDP >= pState->rangeDP ) {
		pState->valueDP = -1;
	}

	// Buttons
	pState->numButtons = 14;
	WORD mask = ((WORD)buf[5] >> 4) | ((WORD)buf[6] << 4) | ((WORD)buf[7] << 12);
	for( UINT i = 0; i < pState->numButtons; ++i ) {
		pState->buttons[i] = ((mask & (1<<i)) != 0);
	}
	return true;
}

bool RawHidDevice::ParseRawInputSonyDualSense(LPBYTE buf, DWORD bufLen, RAW_STATE *pState) {
	if( buf == NULL || bufLen < 16 || pState == NULL ) return false;
	memset(pState, 0, sizeof(RAW_STATE));

	// Axes
	pState->rangeX = pState->rangeY = pState->rangeZ = 0x100;
	pState->rangeRX = pState->rangeRY = pState->rangeRZ = 0x100;
	pState->valueX = buf[1];
	pState->valueY = buf[2];
	pState->valueZ = buf[3];
	pState->valueRZ = buf[4];
	pState->valueRX = buf[5];
	pState->valueRY = buf[6];

	// D-Pad
	pState->rangeDP = 8;
	pState->valueDP = (buf[8] & 0x0F);
	if( pState->valueDP >= pState->rangeDP ) {
		pState->valueDP = -1;
	}

	// Buttons
	pState->numButtons = 14;
	WORD mask = ((WORD)buf[8] >> 4) | ((WORD)buf[9] << 4) | ((WORD)buf[10] << 12);
	for( UINT i = 0; i < pState->numButtons; ++i ) {
		pState->buttons[i] = ((mask & (1<<i)) != 0);
	}
	return true;
}

bool RawHidDevice::SendRawReport(bool clean) {
	if( !isConnected ) return false;
	bool result = false;
	BYTE buf[HidCaps.OutputReportByteLength];
	switch( HidAttr.VendorID ) {
	case VID_SONY:
		WaitForSingleObject(hMutex, INFINITE);
		if( clean ) {
			result = SonyControllerReport(buf, sizeof(buf), HidAttr.ProductID, NULL);
			RawReport.updated = true;
		} else if( RawReport.updated ) {
			result = SonyControllerReport(buf, sizeof(buf), HidAttr.ProductID, &RawReport);
			RawReport.updated = false;
		}
		ReleaseMutex(hMutex);
		break;
	default:
		break;
	}
	if( !result || !Send(buf, sizeof(buf)) ) {
		if( !isConnected ) {
			WaitForSingleObject(hMutex, INFINITE);
			memset(&RawState, 0, sizeof(RawState));
			RawReport.updated = true;
			ReleaseMutex(hMutex);
		}
		return false;
	}
	return true;
}

bool RawHidDevice::ReceiveRawInput() {
	if( !isConnected || !Receive(pRawInput, HidCaps.InputReportByteLength) ) {
		if( !isConnected ) {
			WaitForSingleObject(hMutex, INFINITE);
			memset(&RawState, 0, sizeof(RawState));
			RawReport.updated = true;
			ReleaseMutex(hMutex);
		}
		return false;
	}
	if( isBluetooth && HidAttr.VendorID == VID_SONY ) {
		DWORD readCRC = 0;
		DWORD calcCRC = 0;
		if( pRawInput[0] == ((HidAttr.ProductID == PID_DUALSENSE) ? 0x31 : 0x11) ) {
			for( int i=1; i<=4; ++i ) {
				readCRC <<= 8;
				readCRC |= pRawInput[SONY_BT_REPORT_SIZE - i];
			}
			BYTE btheader[] = {0xA1}; // (format 4:2:2) transaction_type=0xA (data), parameters=0x0 (none), report_type=0x1 (input)
			calcCRC = SonyBluetoothCRC(btheader, sizeof(btheader), calcCRC);
			calcCRC = SonyBluetoothCRC(pRawInput, SONY_BT_REPORT_SIZE-4, calcCRC);
		}
		if( calcCRC != readCRC ) {
			// just skip this packet
			return false;
		}
	}
	bool result = false;
	RAW_STATE state;
	memset(&state, 0, sizeof(state));
	switch( HidAttr.VendorID ) {
	case VID_SONY:
		switch( HidAttr.ProductID ) {
		case PID_DUALSHOCK4_1:
		case PID_DUALSHOCK4_2:
		case PID_DUALSHOCK4_W:
			if( isBluetooth ) {
				DWORD offset = (pRawInput[0] == 0x11) ? 2 : 0;
				result = ParseRawInputSonyDualShock4(pRawInput + offset, HidCaps.InputReportByteLength, &state);
			}
			break;
		case PID_DUALSENSE:
			if( isBluetooth ) {
				DWORD offset = (pRawInput[0] == 0x31) ? 1 : 0;
				result = ParseRawInputSonyDualSense(pRawInput + offset, HidCaps.InputReportByteLength-1, &state);
			}
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	if( !result ) {
		result = ParseRawInputStandard(pRawInput, HidCaps.InputReportByteLength, &state);
	}
	WaitForSingleObject(hMutex, INFINITE);
	RawState = state;
	ReleaseMutex(hMutex);
	return result;
}

bool RawHidDevice::IsRunning() {
	DWORD status;
	return ((hThread != NULL) && GetExitCodeThread(hThread, &status) && (status == STILL_ACTIVE));
}

bool RawHidDevice::Start(LPCTSTR lpName) {
	if( !lpName || !*lpName ) {
		return false;
	}
	if( IsRunning() ) {
		return !strcasecmp(lpDeviceName, lpName);
	}
	hMutex = CreateMutex(NULL, FALSE, NULL);
	if( hMutex != NULL ) {
		lpDeviceName = strdup(lpName);
		if( lpDeviceName != NULL ) {
			memset(&RawReport, 0, sizeof(RawReport));
			isStop = false;
			hThread = CreateThread(NULL, 0, StaticTask, this, 0, NULL);
			if( hThread != NULL ) {
				return true;
			}
		}
	}
	Stop();
	return false;
}

void RawHidDevice::Stop() {
	if( IsRunning() ) {
		isStop = true;
		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);
		hThread = NULL;
	}
	if( lpDeviceName != NULL ) {
		free(lpDeviceName);
		lpDeviceName = NULL;
	}
	if( hMutex != NULL ) {
		CloseHandle(hMutex);
		hMutex = NULL;
	}
}

bool RawHidDevice::SetState(WORD leftMotor, WORD rightMotor, DWORD color) {
	if( !IsRunning() ) return false;
	WaitForSingleObject(hMutex, INFINITE);
	if( leftMotor != RawReport.leftMotor ||
		rightMotor != RawReport.rightMotor ||
		color != RawReport.color )
	{
		RawReport.leftMotor = leftMotor;
		RawReport.rightMotor = rightMotor;
		RawReport.color = color;
		RawReport.updated = true;
	}
	ReleaseMutex(hMutex);
	return true;
}

bool RawHidDevice::GetState(RINPUT_STATE *pState) {
	if( !pState || !IsRunning() ) return false;

	WaitForSingleObject(hMutex, INFINITE);
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
	pState->btnTouch = RawState.buttons[13];

	if( RawState.rangeX  ) pState->axisLX = (float)(RawState.valueX*2 - RawState.rangeX) / (float)RawState.rangeX;
	if( RawState.rangeY  ) pState->axisLY = (float)(RawState.rangeY - RawState.valueY*2) / (float)RawState.rangeY;
	if( RawState.rangeZ  ) pState->axisRX = (float)(RawState.valueZ*2 - RawState.rangeZ) / (float)RawState.rangeZ;
	if( RawState.rangeRZ ) pState->axisRY = (float)(RawState.rangeRZ - RawState.valueRZ*2) / (float)RawState.rangeRZ;
	if( RawState.rangeRX ) pState->axisL2 = (float)RawState.valueRX / (float)RawState.rangeRX;
	if( RawState.rangeRY ) pState->axisR2 = (float)RawState.valueRY / (float)RawState.rangeRY;

	ReleaseMutex(hMutex);
	return true;
}

static RawHidDevice RawInput;

bool RawInputStart(LPCTSTR lpName) {
	return RawInput.Start(lpName);
}

void RawInputStop() {
	RawInput.Stop();
}

bool RawInputSetState(WORD leftMotor, WORD rightMotor, DWORD color) {
	return RawInput.SetState(leftMotor, rightMotor, color);
}

bool RawInputGetState(RINPUT_STATE *pState) {
	return RawInput.GetState(pState);
}

#endif // FEATURE_INPUT_IMPROVED
