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
#include "specific/init_input.h"
#include "3dsystem/phd_math.h"
#include "global/vars.h"

#ifdef FEATURE_INPUT_IMPROVED
#include "modding/xinput_ex.h"
#include "modding/raw_input.h"
#include "modding/joy_output.h"
#define HAS_AXIS(x) (JoyRanges[x].lMax != JoyRanges[x].lMin)

typedef enum {
	JoyX,
	JoyY,
	JoyZ,
	JoyRX,
	JoyRY,
	JoyRZ,
	JoyAxisNumber,
} JOY_AXIS_ENUM;

typedef struct {
	long lMin;
	long lMax;
} JOY_AXIS_RANGE;

static bool IsRawInput = false;
static int XInputIndex = -1;
static XINPUT_CAPABILITIES XInputCaps;
static DIDEVCAPS JoyCaps;
static JOY_AXIS_RANGE JoyRanges[JoyAxisNumber];

bool JoystickVibrationEnabled = true;
bool JoystickLedColorEnabled = true;

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

static BOOL CALLBACK RawInputCallBack(LPGUID lpGuid, LPCTSTR lpDeviceName, LPCTSTR lpProductName, WORD vid, WORD pid, LPVOID lpContext) {
	if( lpGuid == NULL || lpDeviceName == NULL || lpProductName == NULL || lpContext == NULL || IsXInputDevice(vid, pid) )
		return TRUE;

	JOYSTICK_LIST *joyList = (JOYSTICK_LIST *)lpContext;
	JOYSTICK_NODE *joyNode = new JOYSTICK_NODE;

	if( joyNode == NULL )
		return TRUE;

	joyNode->next = NULL;
	joyNode->previous = joyList->tail;

	if( !joyList->head )
		joyList->head = joyNode;

	if( joyList->tail )
		joyList->tail->next = joyNode;

	joyList->tail = joyNode;
	joyList->dwCount++;

	joyNode->body.joystickGuid = *lpGuid;
	joyNode->body.lpJoystickGuid = &joyNode->body.joystickGuid;
	FlaggedStringCreate(&joyNode->body.productName, strlen(lpProductName)+1);
	FlaggedStringCreate(&joyNode->body.instanceName, strlen(lpDeviceName)+1);
	lstrcpy(joyNode->body.productName.lpString, lpProductName);
	lstrcpy(joyNode->body.instanceName.lpString, lpDeviceName);
	joyNode->body.iface = JOY_RawInput;
	return TRUE;
}

void SetJoystickOutput(WORD leftMotor, WORD rightMotor, DWORD ledColor) {
	static DWORD leftMotorOld = ~0;
	static DWORD rightMotorOld = ~0;
	static DWORD ledColorOld = ~0;
	if( !SavedAppSettings.JoystickEnabled
		|| (leftMotor == leftMotorOld && rightMotor == rightMotorOld && ledColor == ledColorOld) )
	{
		return;
	}
	bool result = true;

	if( XInputIndex >= 0 ) {
		XINPUT_VIBRATION vibration;
		vibration.wLeftMotorSpeed  = leftMotor;
		vibration.wRightMotorSpeed = rightMotor;
		XInputEnable(TRUE);
		result = (ERROR_SUCCESS == XInputSetState(XInputIndex, &vibration));
	} else if( IsRawInput ) {
		result = RawInputSetState(leftMotor, rightMotor, ledColor);
	}

	if( result ) {
		leftMotorOld = leftMotor;
		rightMotorOld = rightMotor;
		ledColorOld = ledColor;
	}
}

JOYTYPE GetJoystickType() {
	if( SavedAppSettings.JoystickEnabled && SavedAppSettings.PreferredJoystick ) {
		if( IsRawInput ) {
			return JT_PLAYSTATION;
		}
		if( XInputIndex >= 0 ) {
			return JT_XINPUT;
		}
		if( IDID_SysJoystick != NULL ) {
			return JT_DIRECTINPUT;
		}
	}
	return JT_NONE;
}

bool IsJoyVibrationSupported() {
	if( IsRawInput ) {
		return true;
	}
	if( XInputIndex >= 0 ) {
		return( XInputCaps.Vibration.wLeftMotorSpeed || XInputCaps.Vibration.wLeftMotorSpeed );
	}
	return false;
}

bool IsJoyLedColorSupported() {
	return IsRawInput;
}

bool IsJoyVibrationEnabled() {
	return SavedAppSettings.JoystickEnabled && JoystickVibrationEnabled && IsJoyVibrationSupported();
}

bool IsJoyLedColorEnabled() {
	return SavedAppSettings.JoystickEnabled && JoystickLedColorEnabled && IsJoyLedColorSupported();
}
#endif // FEATURE_INPUT_IMPROVED

extern void __thiscall FlaggedStringDelete(STRING_FLAGGED *item);
extern bool FlaggedStringCopy(STRING_FLAGGED *dst, STRING_FLAGGED *src);

bool __cdecl DInputCreate() {
#if (DIRECTINPUT_VERSION >= 0x800)
	return SUCCEEDED(DirectInput8Create(GameModule, DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID *)&DInput, NULL));
#else // (DIRECTINPUT_VERSION >= 0x800)
	return SUCCEEDED(DirectInputCreate(GameModule, DIRECTINPUT_VERSION, &DInput, NULL));
#endif // (DIRECTINPUT_VERSION >= 0x800)
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

#ifdef FEATURE_INPUT_IMPROVED
static void PovToPos(int *xPos, int *yPos, DWORD pov) {
	int x=0, y=0;
	// Check if D-PAD is not centered
	if( LOWORD(pov) != 0xFFFF ) {
		// We need here cyclic quadrilateral with a side 32.
		// The radius of circumscribed circle is approx 23 = 16 * SQRT(2)
		x = +23 * phd_sin(pov * PHD_360 / 36000) / PHD_IONE;
		y = -23 * phd_cos(pov * PHD_360 / 36000) / PHD_IONE;
		CLAMP(x, -16, 16);
		CLAMP(y, -16, 16);
	}
	if( xPos ) *xPos = x;
	if( yPos ) *yPos = y;
}

static int SelectJoyDirection(int dp, int ls, int rs, int threshold) {
	// D-Pad has priority
	if( ABS(dp) > threshold ) return dp;
	// if sticks have opposite directions, just sum them
	if( (ls^rs) < 0 ) return ls + rs;
	// or just select the one with max value
	if( ABS(rs) > ABS(ls) ) return rs;
	return ls;
}

static DWORD XInputReadJoystick(int *xPos, int *yPos) {
	if( !xPos || !yPos || XInputIndex < 0 ) return 0;
	*xPos = *yPos = 0;
	DWORD buttonStatus = 0;
	XINPUT_STATE state;

	XInputEnable(TRUE);
	if( ERROR_SUCCESS != XInputGetState(XInputIndex, &state) ) {
		return 0;
	}

	int dx=0, dy=0, lx=0, ly=0, rx=0, ry=0;
	if( XINPUT_DPAD(XInputCaps.Gamepad.wButtons) ) {
		dx += CHK_ANY(state.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_RIGHT) ? 16 : 0;
		dx -= CHK_ANY(state.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_LEFT) ? 16 : 0;
		dy += CHK_ANY(state.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_DOWN) ? 16 : 0;
		dy -= CHK_ANY(state.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_UP) ? 16 : 0;
	}
	if( XInputCaps.Gamepad.sThumbLX && XInputCaps.Gamepad.sThumbLY ) {
		lx = 16 * state.Gamepad.sThumbLX / 0x8000;
		ly = -16 * state.Gamepad.sThumbLY / 0x8000;
	}
	if( XInputCaps.Gamepad.sThumbRX && XInputCaps.Gamepad.sThumbRY ) {
		rx = 16 * state.Gamepad.sThumbRX / 0x8000;
		ry = -16 * state.Gamepad.sThumbRY / 0x8000;
	}
	*xPos = SelectJoyDirection(dx, lx, rx, 8);
	*yPos = SelectJoyDirection(dy, ly, ry, 8);

	buttonStatus |= CHK_ANY(state.Gamepad.wButtons, XINPUT_GAMEPAD_Y) ? 0x001 : 0;
	buttonStatus |= CHK_ANY(state.Gamepad.wButtons, XINPUT_GAMEPAD_B) ? 0x002 : 0;
	buttonStatus |= CHK_ANY(state.Gamepad.wButtons, XINPUT_GAMEPAD_A) ? 0x004 : 0;
	buttonStatus |= CHK_ANY(state.Gamepad.wButtons, XINPUT_GAMEPAD_X) ? 0x008 : 0;
	buttonStatus |= CHK_ANY(state.Gamepad.wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER) ? 0x010 : 0;
	buttonStatus |= CHK_ANY(state.Gamepad.wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER) ? 0x020 : 0;
	buttonStatus |= (state.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD) ? 0x040 : 0;
	buttonStatus |= (state.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD) ? 0x080 : 0;
	buttonStatus |= CHK_ANY(state.Gamepad.wButtons, XINPUT_GAMEPAD_BACK) ? 0x100 : 0;
	buttonStatus |= CHK_ANY(state.Gamepad.wButtons, XINPUT_GAMEPAD_START) ? 0x200 : 0;
	buttonStatus |= CHK_ANY(state.Gamepad.wButtons, XINPUT_GAMEPAD_LEFT_THUMB) ? 0x400 : 0;
	buttonStatus |= CHK_ANY(state.Gamepad.wButtons, XINPUT_GAMEPAD_RIGHT_THUMB) ? 0x800 : 0;
	buttonStatus |= CHK_ANY(state.Gamepad.wButtons, XINPUT_GAMEPAD_GUIDE) ? 0x1000 : 0;
	return buttonStatus;
}

static DWORD RawInputReadJoystick(int *xPos, int *yPos) {
	if( !xPos || !yPos || !IsRawInput ) return 0;
	*xPos = *yPos = 0;
	DWORD buttonStatus = 0;
	RINPUT_STATE state;

	if( !RawInputGetState(&state) ) {
		return 0;
	}

	int dx=0, dy=0, lx=0, ly=0, rx=0, ry=0;
	PovToPos(&dx, &dy, state.dPad);
	lx = (int)(+16.0 * state.axisLX);
	ly = (int)(-16.0 * state.axisLY);
	rx = (int)(+16.0 * state.axisRX);
	ry = (int)(-16.0 * state.axisRY);
	*xPos = SelectJoyDirection(dx, lx, rx, 8);
	*yPos = SelectJoyDirection(dy, ly, ry, 8);

	buttonStatus |= state.btnTriangle ? 0x0001 : 0;
	buttonStatus |= state.btnCircle ? 0x0002 : 0;
	buttonStatus |= state.btnCross ? 0x0004 : 0;
	buttonStatus |= state.btnSquare ? 0x0008 : 0;
	buttonStatus |= state.btnL1 ? 0x0010 : 0;
	buttonStatus |= state.btnR1 ? 0x0020 : 0;
	buttonStatus |= state.btnL2 ? 0x0040 : 0;
	buttonStatus |= state.btnR2 ? 0x0080 : 0;
	buttonStatus |= state.btnShare ? 0x0100 : 0;
	buttonStatus |= state.btnOptions ? 0x0200 : 0;
	buttonStatus |= state.btnL3 ? 0x0400 : 0;
	buttonStatus |= state.btnR3 ? 0x0800 : 0;
	buttonStatus |= state.btnPS ? 0x1000 : 0;
	buttonStatus |= state.btnTouch ? 0x2000 : 0;
	return buttonStatus;
}

static DWORD DInputReadJoystick(int *xPos, int *yPos) {
	if( !xPos || !yPos || !IDID_SysJoystick ) return 0;
	*xPos = *yPos = 0;
	DWORD buttonStatus = 0;
	DIJOYSTATE joyState;

#if (DIRECTINPUT_VERSION >= 0x800)
	while( FAILED(IDID_SysJoystick->Poll()) || FAILED(IDID_SysJoystick->GetDeviceState(sizeof(joyState), &joyState)) )
#else // (DIRECTINPUT_VERSION >= 0x800)
	while( FAILED(IDID_SysJoystick->GetDeviceState(sizeof(joyState), &joyState)) )
#endif // (DIRECTINPUT_VERSION >= 0x800)
	{
		if FAILED(IDID_SysJoystick->Acquire()) return 0;
	}

	int dx=0, dy=0, lx=0, ly=0, rx=0, ry=0;
	if( JoyCaps.dwPOVs ) {
		PovToPos(&dx, &dy, joyState.rgdwPOV[0]);
	}
	if( HAS_AXIS(JoyX) && HAS_AXIS(JoyY) ) {
		lx = 32 * joyState.lX / (JoyRanges[JoyX].lMax - JoyRanges[JoyX].lMin) - 16;
		ly = 32 * joyState.lY / (JoyRanges[JoyY].lMax - JoyRanges[JoyY].lMin) - 16;
	}
	if( HAS_AXIS(JoyZ) && HAS_AXIS(JoyRZ) ) {
		rx = 32 * joyState.lZ  / (JoyRanges[JoyZ].lMax  - JoyRanges[JoyZ].lMin)  - 16;
		ry = 32 * joyState.lRz / (JoyRanges[JoyRZ].lMax - JoyRanges[JoyRZ].lMin) - 16;
	} else if( HAS_AXIS(JoyRX) && HAS_AXIS(JoyRY) ) {
		rx = 32 * joyState.lRx / (JoyRanges[JoyRX].lMax - JoyRanges[JoyRX].lMin) - 16;
		ry = 32 * joyState.lRy / (JoyRanges[JoyRY].lMax - JoyRanges[JoyRY].lMin) - 16;
	}
	*xPos = SelectJoyDirection(dx, lx, rx, 8);
	*yPos = SelectJoyDirection(dy, ly, ry, 8);

	for( DWORD i=0; i<MIN(JoyCaps.dwButtons, 32); ++i ) {
		buttonStatus |= CHK_ANY(joyState.rgbButtons[i], 0x80) ? 1<<i : 0;
	}
	return buttonStatus;
}
#endif // FEATURE_INPUT_IMPROVED

DWORD __cdecl WinInReadJoystick(int *xPos, int *yPos) {
#ifdef FEATURE_INPUT_IMPROVED
	*yPos = 0;
	*xPos = 0;
	if( !SavedAppSettings.JoystickEnabled ) return 0;
	switch( GetJoystickType() ) {
		case JT_XINPUT:
			return XInputReadJoystick(xPos, yPos);
		case JT_PLAYSTATION:
			return RawInputReadJoystick(xPos, yPos);
		case JT_DIRECTINPUT:
			return DInputReadJoystick(xPos, yPos);
		case JT_NONE:
			break;
	}
	return 0;
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
		joyNode->body.iface = JOY_XInput;
	}
	RawInputEnumerate(RawInputCallBack, (LPVOID)joystickList);
#endif // FEATURE_INPUT_IMPROVED
	return SUCCEEDED(DInput->EnumDevices(DIDEVTYPE_JOYSTICK, DInputEnumDevicesCallback, (LPVOID)joystickList, DIEDFL_ATTACHEDONLY));
}

BOOL CALLBACK DInputEnumDevicesCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef) {
	if( lpddi == NULL || pvRef == NULL ) {
		return DIENUM_CONTINUE;
	}

#ifdef FEATURE_INPUT_IMPROVED
	DWORD vid = LOWORD(lpddi->guidProduct.Data1);
	DWORD pid = HIWORD(lpddi->guidProduct.Data1);
	if( GetRawInputName(vid, pid, FALSE) || IsXInputDevice(vid, pid) ) {
		return DIENUM_CONTINUE;
	}
#endif // FEATURE_INPUT_IMPROVED

	JOYSTICK_LIST *joyList = (JOYSTICK_LIST *)pvRef;
	JOYSTICK_NODE *joyNode = new JOYSTICK_NODE;

	if( joyNode == NULL )
		return DIENUM_CONTINUE;

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
#ifdef FEATURE_INPUT_IMPROVED
	joyNode->body.iface = JOY_DirectInput;
#endif // FEATURE_INPUT_IMPROVED

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
	if FAILED(DInput->CreateDevice(GUID_SysKeyboard, &IDID_SysKeyboard, NULL))
		throw ERR_CantCreateKeyboardDevice;
	if FAILED(IDID_SysKeyboard->SetCooperativeLevel(HGameWindow, DISCL_FOREGROUND|DISCL_NONEXCLUSIVE))
		throw ERR_CantSetKBCooperativeLevel;
	if FAILED(IDID_SysKeyboard->SetDataFormat(&c_dfDIKeyboard))
		throw ERR_CantSetKBDataFormat;

	// NOTE: there is no DIERR_OTHERAPPHASPRIO check in the original code
	HRESULT res = IDID_SysKeyboard->Acquire();
	if( !SUCCEEDED(res) && res != DIERR_OTHERAPPHASPRIO )
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
	if( CurrentJoystick.iface == JOY_XInput ) {
		if( ERROR_SUCCESS != XInputGetCapabilities(guid->Data4[7], 0, &XInputCaps) ) {
			return false;
		}
		XInputIndex = guid->Data4[7];
		return true;
	}
	if( CurrentJoystick.iface == JOY_RawInput ) {
		IsRawInput = RawInputStart(CurrentJoystick.instanceName.lpString);
		return IsRawInput;
	}
	memset(JoyRanges, 0, sizeof(JoyRanges));
	memset(&JoyCaps, 0, sizeof(JoyCaps));
	JoyCaps.dwSize = sizeof(JoyCaps);
	if FAILED(DInput->CreateDevice(CurrentJoystick.joystickGuid, &IDID_SysJoystick, NULL))
		return false;
	if FAILED(IDID_SysJoystick->SetCooperativeLevel(HGameWindow, DISCL_FOREGROUND|DISCL_NONEXCLUSIVE))
		return false;
	if FAILED(IDID_SysJoystick->SetDataFormat(&c_dfDIJoystick))
		return false;
	if FAILED(IDID_SysJoystick->GetCapabilities(&JoyCaps))
		return false;
	if FAILED(IDID_SysJoystick->EnumObjects(DInputEnumJoystickAxisCallback, (LPVOID)JoyRanges, DIDFT_AXIS))
		return false;

	HRESULT res = IDID_SysJoystick->Acquire();
	if( !SUCCEEDED(res) && res != DIERR_OTHERAPPHASPRIO )
		return false;
#endif // FEATURE_INPUT_IMPROVED

	return true;
}

void __cdecl DInputJoystickRelease() {
#ifdef FEATURE_INPUT_IMPROVED
	SetJoystickOutput(0, 0, DEFAULT_JOYSTICK_LED_COLOR);
	RawInputStop();
	IsRawInput = false;
	XInputEnable(FALSE);
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
		if SUCCEEDED(DInput->CreateDevice(preferred->joystickGuid, &IDID_SysJoystick, NULL))
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
