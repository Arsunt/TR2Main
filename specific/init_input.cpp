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
#include "global/vars.h"

extern void __thiscall FlaggedStringDelete(STRING_FLAGGED *item);
extern bool FlaggedStringCopy(STRING_FLAGGED *dst, STRING_FLAGGED *src);

bool __cdecl DInputCreate() {
	return SUCCEEDED(DirectInputCreate(GameModule, DIRECTINPUT_VERSION, &DInput, NULL));
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
	return SUCCEEDED(DInput->EnumDevices(DIDEVTYPE_JOYSTICK, DInputEnumDevicesCallback, (LPVOID)joystickList, DIEDFL_ATTACHEDONLY));
}

BOOL CALLBACK DInputEnumDevicesCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef) {
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

	if( lpddi == NULL ) {
		memset(&joyNode->body.joystickGuid, 0, sizeof(GUID));
		joyNode->body.lpJoystickGuid = NULL;
	} else {
		joyNode->body.joystickGuid = lpddi->guidInstance;
		joyNode->body.lpJoystickGuid = &joyNode->body.joystickGuid;
	}

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
	if FAILED(DInput->CreateDevice(GUID_SysKeyboard, &IDID_SysKeyboard, NULL))
		throw ERR_CantCreateKeyboardDevice;
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

bool __cdecl DInputJoystickSelect() {
	if( SavedAppSettings.PreferredJoystick == NULL )
		return false;

	JOYSTICK *preferred = &SavedAppSettings.PreferredJoystick->body;
	CurrentJoystick = *preferred;

	FlaggedStringCopy(&CurrentJoystick.productName, &preferred->productName);
	FlaggedStringCopy(&CurrentJoystick.instanceName, &preferred->instanceName);

	return true;
}

void __cdecl WinInStart() {
	if( !DInputCreate() )
		throw ERR_CantCreateDirectInput;

	DInputKeyboardCreate();
	DInputJoystickSelect();
}

void __cdecl WinInFinish() {
	DInputKeyboardRelease();
	DInputRelease();
}

void __cdecl WinInRunControlPanel(HWND hWnd) {
	if( DInput != NULL )
		DInput->RunControlPanel(hWnd, 0);
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
	INJECT(0x00447770, DInputJoystickSelect);
	INJECT(0x00447860, WinInStart);
	INJECT(0x00447890, WinInFinish);
	INJECT(0x004478A0, WinInRunControlPanel);
}
