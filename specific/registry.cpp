/*
 * Copyright (c) 2017-2018 Michael Chaban. All rights reserved.
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
#include "specific/registry.h"
#include "global/vars.h"

static const BYTE GuidMask[] = { '{',3,2,1,0,'-',5,4,'-',7,6,'-',8,9,10,11,12,13,14,15,'}' };
static const char HexChars[] = "0123456789ABCDEF";

static inline BYTE HexChar(char c) {
	if( c>='0' && c<='9' )
		return c-'0';
	if( c>='A' && c<='F' )
		return c-'A'+10;
	if( c>='a' && c<='f' )
		return c-'a'+10;
	return 0xFF;
}

LPCTSTR __cdecl GuidBinaryToString(GUID *guid) {
	static char guidStringBuffer[GUID_STRING_SIZE];
	LPTSTR strPtr = guidStringBuffer;

	for( DWORD i=0; i<sizeof(GuidMask); ++i ) {
		if( GuidMask[i] >= 16 ) {
			*(strPtr++) = GuidMask[i];
		} else {
			BYTE code = ((BYTE *)guid)[GuidMask[i]];
			*(strPtr++) = HexChars[code >> 4];
			*(strPtr++) = HexChars[code & 0xF];
		}
	}
	*strPtr = 0;
	return guidStringBuffer;
}

bool __cdecl GuidStringToBinary(LPCTSTR lpString, GUID *guid) {
	for( DWORD i=0; i<sizeof(GuidMask); ++i ) {
		if( GuidMask[i] >= 16 ) {
			if( *(lpString++) != GuidMask[i] )
				return false;
		} else {
			BYTE hex1 = HexChar(*(lpString++));
			BYTE hex2 = HexChar(*(lpString++));
			if( hex1 >= 16 || hex2 >= 16 )
				return false;

			((BYTE *)guid)[GuidMask[i]] = (hex1 << 4)|(hex2 & 0xF);
		}
	}
	return ( *lpString == 0 ); // check if the string ends with zero
}

BOOL __cdecl OpenRegistryKey(LPCTSTR lpSubKey) {
	return ( ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, lpSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &HKey, &RegKeyDisposition) );
}

bool __cdecl IsNewRegistryKeyCreated() {
	return ( RegKeyDisposition == REG_CREATED_NEW_KEY );
}

LONG __cdecl CloseRegistryKey() {
	return RegCloseKey(HKey);
}

LONG __cdecl SetRegistryDwordValue(LPCTSTR lpValueName, DWORD value) {
	return RegSetValueEx(HKey, lpValueName, 0, REG_DWORD, (const BYTE *)&value, sizeof(DWORD));
}

LONG __cdecl SetRegistryBoolValue(LPCTSTR lpValueName, bool value) {
	DWORD dwValue = value;
	return RegSetValueEx(HKey, lpValueName, 0, REG_DWORD, (const BYTE *)&dwValue, sizeof(DWORD));
}

LONG __cdecl SetRegistryFloatValue(LPCTSTR lpValueName, double value) {
	int stringLen;
	char stringBuf[64];

	stringLen = sprintf(stringBuf, "%.5f", value);
	return SetRegistryStringValue(lpValueName, stringBuf, stringLen);
}

LONG __cdecl SetRegistryBinaryValue(LPCTSTR lpValueName, LPBYTE value, DWORD valueSize) {
	if( value != NULL )
		return RegSetValueEx(HKey, lpValueName, 0, REG_BINARY, value, valueSize);
	else
		return RegDeleteValue(HKey, lpValueName);
}

LONG __cdecl SetRegistryStringValue(LPCTSTR lpValueName, LPCTSTR value, int length) {
	if( value == NULL )
		return RegDeleteValue(HKey, lpValueName);

	if( length < 0 )
		length = lstrlen(value);

	return RegSetValueEx(HKey, lpValueName, 0, REG_SZ, (const BYTE *)value, length+1);
}

LONG __cdecl DeleteRegistryValue(LPCTSTR lpValueName) {
	return RegDeleteValue(HKey, lpValueName);
}

bool __cdecl GetRegistryDwordValue(LPCTSTR lpValueName, DWORD *pValue, DWORD defaultValue) {
	DWORD dwType;
	DWORD dwSize = sizeof(DWORD);

	if( ERROR_SUCCESS == RegQueryValueEx(HKey, lpValueName, 0, &dwType, (LPBYTE)pValue, &dwSize) &&
		dwType == REG_DWORD && dwSize == sizeof(DWORD) )
	{
		return true;
	}

	SetRegistryDwordValue(lpValueName, defaultValue);
	*pValue = defaultValue;
	return false;
}

bool __cdecl GetRegistryBoolValue(LPCTSTR lpValueName, bool *pValue, bool defaultValue) {
	DWORD dwType, dwValue;
	DWORD dwSize = sizeof(DWORD);

	if( ERROR_SUCCESS == RegQueryValueEx(HKey, lpValueName, 0, &dwType, (LPBYTE)&dwValue, &dwSize) &&
		dwType == REG_DWORD && dwSize == sizeof(DWORD) )
	{
		*pValue = dwValue ? true : false;
		return true;
	}

	SetRegistryBoolValue(lpValueName, defaultValue);
	*pValue = defaultValue;
	return false;
}

bool __cdecl GetRegistryFloatValue(LPCTSTR lpValueName, double *value, double defaultValue) {
	char stringBuf[64];

	if( GetRegistryStringValue(lpValueName, stringBuf, sizeof(stringBuf), NULL) ) {
		*value = atof(stringBuf);
		return true;
	}

	SetRegistryFloatValue(lpValueName, defaultValue);
	*value = defaultValue;
	return false;
}

bool __cdecl GetRegistryBinaryValue(LPCTSTR lpValueName, LPBYTE value, DWORD valueSize, LPBYTE defaultValue) {
	DWORD dwType;
	DWORD dwSize = valueSize;

	if( ERROR_SUCCESS == RegQueryValueEx(HKey, lpValueName, 0, &dwType, value, &dwSize) &&
		dwType == REG_BINARY && dwSize == valueSize )
	{
		return true;
	}

	if( defaultValue != NULL )
		SetRegistryBinaryValue(lpValueName, defaultValue, valueSize);
	else
		RegDeleteValue(HKey, lpValueName);

	return false;
}

bool __cdecl GetRegistryStringValue(LPCTSTR lpValueName, LPTSTR value, DWORD maxSize, LPCTSTR defaultValue) {
	DWORD dwType;
	DWORD dwSize = maxSize;

	if( ERROR_SUCCESS == RegQueryValueEx(HKey, lpValueName, 0, &dwType, (LPBYTE)value, &dwSize) &&
		dwType == REG_SZ )
	{
		return true;
	}

	if( defaultValue != NULL ) {
		SetRegistryStringValue(lpValueName, defaultValue, -1);
		dwSize = lstrlen(defaultValue) + 1;
		if( dwSize > maxSize ) {
			dwSize = maxSize-1;
			value[dwSize] = 0;
		}
		memcpy(value, defaultValue, dwSize);
	} else {
		RegDeleteValue(HKey, lpValueName);
	}
	return false;
}

bool __cdecl GetRegistryGuidValue(LPCTSTR lpValueName, GUID *value, GUID *defaultValue) {
	LPCTSTR guidString;
	char guidStringBuffer[GUID_STRING_SIZE];

	if( GetRegistryStringValue(lpValueName, guidStringBuffer, GUID_STRING_SIZE, NULL) &&
		GuidStringToBinary(guidStringBuffer, value) )
	{
		return true;
	}

	if( defaultValue != NULL ) {
		guidString = GuidBinaryToString(defaultValue);
		SetRegistryStringValue(lpValueName, guidString, GUID_STRING_SIZE-1);
		*value = *defaultValue;
	} else {
		RegDeleteValue(HKey, lpValueName);
	}

	return false;
}

/*
 * Inject function
 */
void Inject_Registry() {
	INJECT(0x00456A20, GuidBinaryToString);
	INJECT(0x00456A80, GuidStringToBinary);
	INJECT(0x00456B30, OpenRegistryKey);
	INJECT(0x00456B60, IsNewRegistryKeyCreated);
	INJECT(0x00456B70, CloseRegistryKey);
	INJECT(0x00456B80, SetRegistryDwordValue);
	INJECT(0x00456BA0, SetRegistryBoolValue);
	INJECT(0x00456BD0, SetRegistryFloatValue);
	INJECT(0x00456C10, SetRegistryBinaryValue);
	INJECT(0x00456C50, SetRegistryStringValue);
	INJECT(0x00456CA0, DeleteRegistryValue);
	INJECT(0x00456CC0, GetRegistryDwordValue);
	INJECT(0x00456D20, GetRegistryBoolValue);
	INJECT(0x00456DA0, GetRegistryFloatValue);
	INJECT(0x00456E00, GetRegistryBinaryValue);
	INJECT(0x00456E80, GetRegistryStringValue);
	INJECT(0x00456F20, GetRegistryGuidValue);
}
