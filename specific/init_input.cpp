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
#include "specific/init_input.h"
#include "3dsystem/phd_math.h"
#include "global/vars.h"

#ifdef FEATURE_INPUT_IMPROVED
#include <XInput.h>
#define XINPUT_DPAD(x) (CHK_ALL((x), XINPUT_GAMEPAD_DPAD_UP|XINPUT_GAMEPAD_DPAD_DOWN|XINPUT_GAMEPAD_DPAD_LEFT|XINPUT_GAMEPAD_DPAD_RIGHT))
#define HAS_AXIS(x) (JoyRanges[x].lMax != JoyRanges[x].lMin)

typedef enum {
	JoyX,
	JoyY,
	JoyZ,
	JoyRX,
	JoyRY,
	JoyRZ,
	JoyS0,
	JoyS1,
	JoyAxisNumber,
} JOY_AXIS_ENUM;

typedef struct {
	long lMin;
	long lMax;
} JOY_AXIS_RANGE;

static int XInputIndex = -1;
static XINPUT_CAPABILITIES XInputCaps;
static DIDEVCAPS JoyCaps;
static JOY_AXIS_RANGE JoyRanges[JoyAxisNumber];

DWORD JoystickMovement = 0;

BOOL CALLBACK DInputEnumJoystickAxisCallback(LPCDIDEVICEOBJECTINSTANCE pdidoi, LPVOID pContext) {
	if( !pdidoi || !pContext ) {
		return DIENUM_CONTINUE;
	}

	DIPROPRANGE prop;
	prop.diph.dwSize = sizeof(prop);
	prop.diph.dwHeaderSize = sizeof(prop.diph);
	prop.diph.dwHow = DIPH_BYOFFSET;
	prop.diph.dwObj = pdidoi->dwOfs;
	if( FAILED( IDID_SysJoystick->GetProperty(DIPROP_RANGE, &prop.diph) ) ) {
		return DIENUM_CONTINUE;
	}

	DWORD axisOfs[JoyAxisNumber] = {
		DIJOFS_X, DIJOFS_Y, DIJOFS_Z,
		DIJOFS_RX, DIJOFS_RY, DIJOFS_RZ,
		DIJOFS_SLIDER(0), DIJOFS_SLIDER(1),
	};

	JOY_AXIS_RANGE *axisRanges = (JOY_AXIS_RANGE *)pContext;
	for( int i=0; i<JoyAxisNumber; ++i ) {
		if( pdidoi->dwOfs == axisOfs[i] ) {
			axisRanges[i].lMin = prop.lMin;
			axisRanges[i].lMax = prop.lMax;
			break;
		}
	}
	return DIENUM_CONTINUE;
}

//-----------------------------------------------------------------------------
// Enum each PNP device using WMI and check each device ID to see if it contains
// "IG_" (ex. "VID_045E&PID_028E&IG_00").  If it does, then it's an XInput device
// Unfortunately this information can not be found by just using DirectInput
//-----------------------------------------------------------------------------
#include <wbemidl.h>
#include <oleauto.h>

BOOL IsXInputDevice( const GUID* pGuidProductFromDirectInput ) {
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
					DWORD dwVidPid = MAKELONG( dwVid, dwPid );
					if( dwVidPid == pGuidProductFromDirectInput->Data1 )
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

extern void __thiscall FlaggedStringDelete(STRING_FLAGGED *item);
extern bool FlaggedStringCopy(STRING_FLAGGED *dst, STRING_FLAGGED *src);

bool __cdecl DInputCreate() {
#if (DIRECTINPUT_VERSION >= 0x700)
	return SUCCEEDED(DirectInputCreateEx(GameModule, DIRECTINPUT_VERSION, IID_IDirectInput7, (LPVOID *)&DInput, NULL));
#else // (DIRECTINPUT_VERSION >= 0x700)
	return SUCCEEDED(DirectInputCreate(GameModule, DIRECTINPUT_VERSION, &DInput, NULL));
#endif // (DIRECTINPUT_VERSION >= 0x700)
}

void __cdecl DInputRelease() {
	if( DInput != NULL ) {
		DInput->Release();
		DInput = NULL;
	}
}

void __cdecl WinInReadKeyboard(LPVOID lpInputData) {
	// NOTE: the original code did check only DIERR_INPUTLOST. Any FAILED state must be check to call Acquire
	while FAILED(IDID_SysKeyboard->GetDeviceState(256, lpInputData)) {
		if FAILED(IDID_SysKeyboard->Acquire()) {
			memset(lpInputData, 0, 256);
			break;
		}
	}
}

DWORD __cdecl WinInReadJoystick(int *xPos, int *yPos) {
#ifdef FEATURE_INPUT_IMPROVED
	*yPos = 0;
	*xPos = 0;
	DWORD buttonStatus = 0;
	if( !SavedAppSettings.JoystickEnabled ) return 0;

	if( XInputIndex >= 0 ) {
		XInputEnable(TRUE);
		XINPUT_STATE state;
		if( ERROR_SUCCESS != XInputGetState(XInputIndex, &state) ) {
			return 0;
		}
		if( (JoystickMovement || !XINPUT_DPAD(XInputCaps.Gamepad.wButtons)) && XInputCaps.Gamepad.sThumbLX && XInputCaps.Gamepad.sThumbLY ) {
			*xPos += 32 * state.Gamepad.sThumbLX / PHD_ONE;
			*yPos -= 32 * state.Gamepad.sThumbLY / PHD_ONE;
		} else if( XINPUT_DPAD(XInputCaps.Gamepad.wButtons) ) {
			*xPos += CHK_ANY(state.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_RIGHT) ? 16 : 0;
			*xPos -= CHK_ANY(state.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_LEFT) ? 16 : 0;
			*yPos += CHK_ANY(state.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_DOWN) ? 16 : 0;
			*yPos -= CHK_ANY(state.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_UP) ? 16 : 0;
		}
		buttonStatus |= CHK_ANY(state.Gamepad.wButtons, XINPUT_GAMEPAD_A) ? 0x001 : 0;
		buttonStatus |= CHK_ANY(state.Gamepad.wButtons, XINPUT_GAMEPAD_B) ? 0x002 : 0;
		buttonStatus |= CHK_ANY(state.Gamepad.wButtons, XINPUT_GAMEPAD_X) ? 0x004 : 0;
		buttonStatus |= CHK_ANY(state.Gamepad.wButtons, XINPUT_GAMEPAD_Y) ? 0x008 : 0;
		buttonStatus |= CHK_ANY(state.Gamepad.wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER) ? 0x010 : 0;
		buttonStatus |= CHK_ANY(state.Gamepad.wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER) ? 0x020 : 0;
		buttonStatus |= (state.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD) ? 0x040 : 0;
		buttonStatus |= (state.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD) ? 0x080 : 0;
		buttonStatus |= CHK_ANY(state.Gamepad.wButtons, XINPUT_GAMEPAD_BACK) ? 0x100 : 0;
		buttonStatus |= CHK_ANY(state.Gamepad.wButtons, XINPUT_GAMEPAD_START) ? 0x200 : 0;
		buttonStatus |= CHK_ANY(state.Gamepad.wButtons, XINPUT_GAMEPAD_LEFT_THUMB) ? 0x400 : 0;
		buttonStatus |= CHK_ANY(state.Gamepad.wButtons, XINPUT_GAMEPAD_RIGHT_THUMB) ? 0x800 : 0;
		return buttonStatus;
	}

	if( !IDID_SysJoystick ) return 0;
	DIJOYSTATE joyState;
#if (DIRECTINPUT_VERSION >= 0x700)
	while( FAILED(IDID_SysJoystick->Poll()) || FAILED(IDID_SysJoystick->GetDeviceState(sizeof(joyState), &joyState)) )
#else // (DIRECTINPUT_VERSION >= 0x700)
	while( FAILED(IDID_SysJoystick->GetDeviceState(sizeof(joyState), &joyState)) )
#endif // (DIRECTINPUT_VERSION >= 0x700)
	{
		if FAILED(IDID_SysJoystick->Acquire()) return 0;
	}
	if( (JoystickMovement || !JoyCaps.dwPOVs) && JoyCaps.dwAxes && HAS_AXIS(JoyX) && HAS_AXIS(JoyY) ) {
		*xPos = 32 * joyState.lX / (JoyRanges[JoyX].lMax - JoyRanges[JoyX].lMin) - 16;
		*yPos = 32 * joyState.lY / (JoyRanges[JoyY].lMax - JoyRanges[JoyY].lMin) - 16;
	} else if( JoyCaps.dwPOVs ) {
		DWORD pov = joyState.rgdwPOV[0];
		// Check if D-PAD is not centered
		if( LOWORD(pov) != 0xFFFF ) {
			*xPos = +16 * phd_sin(pov * PHD_360 / 36000) / PHD_IONE;
			*yPos = -16 * phd_cos(pov * PHD_360 / 36000) / PHD_IONE;
		}
	}
	for( DWORD i=0; i<MIN(JoyCaps.dwButtons, 32); ++i ) {
		buttonStatus |= CHK_ANY(joyState.rgbButtons[i], 0x80) ? 1<<i : 0;
	}
	return buttonStatus;
#else // FEATURE_INPUT_IMPROVED
	static bool joyNeedCaps = true;
	static JOYCAPS joyCaps;

	if( SavedAppSettings.JoystickEnabled ) {
		if( joyNeedCaps && JOYERR_NOERROR == joyGetDevCaps(0, &joyCaps, sizeof(JOYCAPS)) ) {
			joyNeedCaps = false;
		}

		if( !joyNeedCaps ) {
			JOYINFOEX joyInfo;

			joyInfo.dwSize = sizeof(JOYINFOEX);
			joyInfo.dwFlags = JOY_RETURNBUTTONS|JOY_RETURNX|JOY_RETURNY;

			if( JOYERR_NOERROR == joyGetPosEx(0, &joyInfo) ) {
				*xPos = 32 * joyInfo.dwXpos / (joyCaps.wXmax - joyCaps.wXmin) - 16;
				*yPos = 32 * joyInfo.dwYpos / (joyCaps.wYmax - joyCaps.wYmin) - 16;
				return joyInfo.dwButtons;
			}

			joyNeedCaps = true;
		}
	}
	*yPos = 0;
	*xPos = 0;
#endif // FEATURE_INPUT_IMPROVED
	return 0;
}

bool __cdecl WinInputInit() {
	JOYSTICK_NODE *node, *nextNode;
	bool result;

	for( node = JoystickList.head; node; node = nextNode ) {
		nextNode = node->next;
		FlaggedStringDelete(&node->body.productName);
		FlaggedStringDelete(&node->body.instanceName);
		delete(node);
	}

	JoystickList.head = NULL;
	JoystickList.tail = NULL;
	JoystickList.dwCount = 0;

	if( !DInputCreate() )
		return false;

	result = DInputEnumDevices(&JoystickList);
	DInputRelease();

	return result;
}

bool __cdecl DInputEnumDevices(JOYSTICK_LIST *joystickList) {
#ifdef FEATURE_INPUT_IMPROVED
	for( DWORD i = 0; i < XUSER_MAX_COUNT; ++i ) {
		XINPUT_STATE state;
		memset(&state, 0, sizeof(state));
		if( ERROR_SUCCESS != XInputGetState(i, &state) ) {
			continue;
		}
		JOYSTICK_NODE *joyNode = new JOYSTICK_NODE;
		if( joyNode == NULL ) {
			continue;
		}
		joyNode->next = NULL;
		joyNode->previous = joystickList->tail;
		if( !joystickList->head ) {
			joystickList->head = joyNode;
		}
		if( joystickList->tail ) {
			joystickList->tail->next = joyNode;
		}
		joystickList->tail = joyNode;
		joystickList->dwCount++;
		memset(&joyNode->body.joystickGuid, 0, sizeof(GUID));
		joyNode->body.joystickGuid.Data4[7] = i;
		joyNode->body.lpJoystickGuid = &joyNode->body.joystickGuid;
		FlaggedStringCreate(&joyNode->body.productName, 256);
		FlaggedStringCreate(&joyNode->body.instanceName, 256);
		snprintf(joyNode->body.productName.lpString, 256, "XInput Controller %lu", i+1);
	}
#endif // FEATURE_INPUT_IMPROVED
	return SUCCEEDED(DInput->EnumDevices(DIDEVTYPE_JOYSTICK, DInputEnumDevicesCallback, (LPVOID)joystickList, DIEDFL_ATTACHEDONLY));
}

BOOL CALLBACK DInputEnumDevicesCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef) {
	JOYSTICK_LIST *joyList = (JOYSTICK_LIST *)pvRef;
	JOYSTICK_NODE *joyNode = new JOYSTICK_NODE;

	if( joyNode == NULL || lpddi == NULL )
		return DIENUM_CONTINUE;

#ifdef FEATURE_INPUT_IMPROVED
	if( IsXInputDevice(&lpddi->guidProduct) ) {
		return DIENUM_CONTINUE;
	}
#endif // FEATURE_INPUT_IMPROVED

	joyNode->next = NULL;
	joyNode->previous = joyList->tail;

	if( !joyList->head )
		joyList->head = joyNode;

	if( joyList->tail )
		joyList->tail->next = joyNode;

	joyList->tail = joyNode;
	joyList->dwCount++;

	joyNode->body.joystickGuid = lpddi->guidInstance;
	joyNode->body.lpJoystickGuid = &joyNode->body.joystickGuid;
	FlaggedStringCreate(&joyNode->body.productName, 256);
	FlaggedStringCreate(&joyNode->body.instanceName, 256);
	lstrcpy(joyNode->body.productName.lpString, lpddi->tszProductName);
	lstrcpy(joyNode->body.instanceName.lpString, lpddi->tszInstanceName);

	return DIENUM_CONTINUE;
}

void __thiscall FlaggedStringCreate(STRING_FLAGGED *item, DWORD dwSize) {
	item->lpString = new char[dwSize];

	if( item->lpString != NULL ) {
		*item->lpString = 0;
		item->isPresented = true;
	}
}

JOYSTICK_NODE *__cdecl GetJoystick(GUID *lpGuid) {
	JOYSTICK_NODE *joystick;

	if( JoystickList.dwCount == 0 )
		return NULL;

	if( lpGuid != NULL ) {
		for( joystick = JoystickList.head; joystick; joystick = joystick->next ) {
			if( !memcmp(&joystick->body.joystickGuid, lpGuid, sizeof(GUID)) )
				return joystick;
		}
	}
	return JoystickList.head;
}

void __cdecl DInputKeyboardCreate() {
#if (DIRECTINPUT_VERSION >= 0x700)
	if FAILED(DInput->CreateDeviceEx(GUID_SysKeyboard, IID_IDirectInputDevice7, (LPVOID *)&IDID_SysKeyboard, NULL))
		throw ERR_CantCreateKeyboardDevice;
#else // (DIRECTINPUT_VERSION >= 0x700)
	if FAILED(DInput->CreateDevice(GUID_SysKeyboard, &IDID_SysKeyboard, NULL))
		throw ERR_CantCreateKeyboardDevice;
#endif // (DIRECTINPUT_VERSION >= 0x700)
	if FAILED(IDID_SysKeyboard->SetCooperativeLevel(HGameWindow, DISCL_FOREGROUND|DISCL_NONEXCLUSIVE))
		throw ERR_CantSetKBCooperativeLevel;
	if FAILED(IDID_SysKeyboard->SetDataFormat(&c_dfDIKeyboard))
		throw ERR_CantSetKBDataFormat;
	if FAILED(IDID_SysKeyboard->Acquire())
		throw ERR_CantAcquireKeyboard;
}

void __cdecl DInputKeyboardRelease() {
	if( IDID_SysKeyboard != NULL ) {
		IDID_SysKeyboard->Unacquire();
		IDID_SysKeyboard->Release();
		IDID_SysKeyboard = NULL;
	}
}

bool __cdecl DInputJoystickCreate() {
	if( SavedAppSettings.PreferredJoystick == NULL )
		return true;

	JOYSTICK *preferred = &SavedAppSettings.PreferredJoystick->body;
	CurrentJoystick = *preferred;

	FlaggedStringCopy(&CurrentJoystick.productName, &preferred->productName);
	FlaggedStringCopy(&CurrentJoystick.instanceName, &preferred->instanceName);
#ifdef FEATURE_INPUT_IMPROVED
	XInputIndex = -1;
	memset(&XInputCaps, 0, sizeof(XInputCaps));
	GUID *guid = &CurrentJoystick.joystickGuid;
	if( !guid->Data1 && !guid->Data2 && !guid->Data3 ) {
		if( ERROR_SUCCESS != XInputGetCapabilities(guid->Data4[7], 0, &XInputCaps) ) {
			return false;
		}
		XInputIndex = guid->Data4[7];
		return true;
	}

	memset(JoyRanges, 0, sizeof(JoyRanges));
	memset(&JoyCaps, 0, sizeof(JoyCaps));
	JoyCaps.dwSize = sizeof(JoyCaps);
#if (DIRECTINPUT_VERSION >= 0x700)
	if FAILED(DInput->CreateDeviceEx(CurrentJoystick.joystickGuid, IID_IDirectInputDevice7, (LPVOID *)&IDID_SysJoystick, NULL))
		return false;
#else // (DIRECTINPUT_VERSION >= 0x700)
	if FAILED(DInput->CreateDevice(CurrentJoystick.joystickGuid, &IDID_SysJoystick, NULL))
		return false;
#endif // (DIRECTINPUT_VERSION >= 0x700)
	if FAILED(IDID_SysJoystick->SetCooperativeLevel(HGameWindow, DISCL_FOREGROUND|DISCL_NONEXCLUSIVE))
		return false;
	if FAILED(IDID_SysJoystick->SetDataFormat(&c_dfDIJoystick))
		return false;
	if FAILED(IDID_SysJoystick->GetCapabilities(&JoyCaps))
		return false;
	if FAILED(IDID_SysJoystick->EnumObjects(DInputEnumJoystickAxisCallback, (LPVOID)JoyRanges, DIDFT_AXIS))
		return false;
	if FAILED(IDID_SysJoystick->Acquire())
		return false;
#endif // FEATURE_INPUT_IMPROVED

	return true;
}

void __cdecl DInputJoystickRelease() {
#ifdef FEATURE_INPUT_IMPROVED
	XInputIndex = -1;
#endif // FEATURE_INPUT_IMPROVED
	if( IDID_SysJoystick != NULL ) {
		IDID_SysJoystick->Unacquire();
		IDID_SysJoystick->Release();
		IDID_SysJoystick = NULL;
	}
}

void __cdecl WinInStart() {
	if( !DInputCreate() )
		throw ERR_CantCreateDirectInput;

	DInputKeyboardCreate();
	DInputJoystickCreate();
}

void __cdecl WinInFinish() {
	DInputJoystickRelease();
	DInputKeyboardRelease();
	DInputRelease();
}

void __cdecl WinInRunControlPanel(HWND hWnd) {
	if( DInput != NULL ) {
#ifdef FEATURE_INPUT_IMPROVED
		JOYSTICK *preferred = &ChangedAppSettings.PreferredJoystick->body;
#if (DIRECTINPUT_VERSION >= 0x700)
		if SUCCEEDED(DInput->CreateDeviceEx(preferred->joystickGuid, IID_IDirectInputDevice7, (LPVOID *)&IDID_SysJoystick, NULL))
#else // (DIRECTINPUT_VERSION >= 0x700)
		if SUCCEEDED(DInput->CreateDevice(preferred->joystickGuid, &IDID_SysJoystick, NULL))
#endif // (DIRECTINPUT_VERSION >= 0x700)
		{
			IDID_SysJoystick->RunControlPanel(hWnd, 0);
			IDID_SysJoystick->Release();
			IDID_SysJoystick = NULL;
			return;
		}
#endif // FEATURE_INPUT_IMPROVED
		DInput->RunControlPanel(hWnd, 0);
	}
}

/*
 * Inject function
 */
void Inject_InitInput() {
	INJECT(0x004472A0, DInputCreate);
	INJECT(0x004472D0, DInputRelease);
	INJECT(0x004472F0, WinInReadKeyboard);
	INJECT(0x00447350, WinInReadJoystick);
	INJECT(0x00447460, WinInputInit);
	INJECT(0x004474E0, DInputEnumDevices);
	INJECT(0x00447510, DInputEnumDevicesCallback);
	INJECT(0x00447600, FlaggedStringCreate);
	INJECT(0x00447620, GetJoystick);
	INJECT(0x00447670, DInputKeyboardCreate);
	INJECT(0x00447740, DInputKeyboardRelease);
	INJECT(0x00447770, DInputJoystickCreate);
//	INJECT(----------, DInputJoystickRelease);
	INJECT(0x00447860, WinInStart);
	INJECT(0x00447890, WinInFinish);
	INJECT(0x004478A0, WinInRunControlPanel);
}
