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
#include "modding/file_utils.h"
#include "global/vars.h"

int PathStringCombine(LPSTR destPath, DWORD destSize, LPCSTR filePath, LPCSTR fileName) {
	if( destPath == NULL || destSize == 0 || fileName == NULL || *fileName == 0 ) {
		return -1;
	}
	char *tmpPath = new char[destSize];

	if( filePath == NULL || *filePath == 0 ) {
		strncpy(tmpPath, ".", destSize);
	} else {
		strncpy(tmpPath, filePath, destSize);
		PathRemoveBackslash(tmpPath);
	}
	snprintf(destPath, destSize, "%s\\%s", tmpPath, fileName);

	delete[] tmpPath;
	return 0;
}

int AutoSelectExtension(LPSTR fileName, const STRING_FIXED4 *exts, DWORD num_exts) {
	if( fileName == NULL ) {
		return -1;
	}

	if( exts != NULL && num_exts > 0 ) {
		char extFileName[256] = {0};
		char *extension;

		strncpy(extFileName, fileName, sizeof(extFileName));
		extension = PathFindExtension(extFileName);

		if( extension == NULL ) {
			extension = strchr(extFileName, 0);
			*extension = '.';
		}

		for( DWORD i = 0; i < num_exts; ++i ) {
			memcpy(extension + 1, &exts[i], sizeof(STRING_FIXED4));
			if( PathFileExists(extFileName) ) {
				strncpy(fileName, extFileName, sizeof(extFileName));
				return (i + 1);
			}
		}
	}

	return ( PathFileExists(fileName) ) ? 0 : -1;
}

int AutoSelectPathAndExtension(LPSTR fileName, LPCSTR path, const STRING_FIXED4 *exts, DWORD num_exts) {
	char checkFileName[MAX_PATH] = {0};
	int ext;

	if( fileName == NULL || !*fileName ) {
		return -1;
	}

	if( path != NULL && *path ) {
		LPCSTR fname = PathFindFileName(fileName);
		PathStringCombine(checkFileName, sizeof(checkFileName), path, fname);

		ext = AutoSelectExtension(checkFileName, exts, num_exts);

		if( ext >= 0 ) {
			strncpy(fileName, checkFileName, sizeof(checkFileName));
			return 1;
		}
	}

	ext = AutoSelectExtension(fileName, exts, num_exts);
	return ( ext >= 0 ) ? 0 : -1;
}

int CreateDirectories(LPCSTR path, bool isFileName) {
	char shortPath[MAX_PATH];
	char fullPath[MAX_PATH];

	strncpy(shortPath, path, sizeof(shortPath));
	if( isFileName ) {
		PathRemoveFileSpec(shortPath);
	}
	GetFullPathName(shortPath, MAX_PATH, fullPath, NULL);

	switch( SHCreateDirectoryExA(NULL, fullPath, NULL) ) {
		case ERROR_ALREADY_EXISTS :
		case ERROR_FILE_EXISTS :
		case ERROR_SUCCESS :
			return 0;
		default :
			break;
	}
	return -1;
}

int CreateSequenceFilename(LPSTR destName, DWORD destSize, LPCSTR filePath, LPCSTR fileExt, LPCSTR nameBase, int seqDigits, int seqNumber) {
	char shortName[32] = {0};
	int i;

	if( destName == NULL || seqNumber < 0 || seqDigits <= 0 ) {
		return -1;
	}
	if( filePath == NULL ) filePath = "";
	if( nameBase == NULL ) nameBase = "";
	if( fileExt  == NULL ) fileExt  = "";

	int seqMax = 1;
	for( i = 0; i < seqDigits; ++i ) {
		seqMax *= 10;
	}

	for( i = seqNumber; i < seqMax; ++i ) { // search first free screenshot slot
		snprintf(shortName, sizeof(shortName), "%s%0*d%s", nameBase, seqDigits, i, fileExt);
		PathStringCombine(destName, destSize, filePath, shortName);
		if( INVALID_FILE_ATTRIBUTES == GetFileAttributes(destName) ) {
			return i; // no file with such name - we have found the free slot
		}
	}
	return -2;
}

int CreateDateTimeFilename(LPSTR destName, DWORD destSize, LPCSTR filePath, LPCSTR fileExt, SYSTEMTIME *lastTime, int *lastIndex) {
	char shortName[32] = {0};
	char fileName[32] = {0};
	SYSTEMTIME sysTime;

	if( destName == NULL ) {
		return -1;
	}
	if( filePath == NULL ) filePath = "";
	if( fileExt  == NULL ) fileExt  = "";

	GetLocalTime(&sysTime);
	sysTime.wMilliseconds = 0; // we don't use milliseconds

	snprintf(shortName, sizeof(shortName), "%04d%02d%02d_%02d%02d%02d",
			sysTime.wYear, sysTime.wMonth,  sysTime.wDay,
			sysTime.wHour, sysTime.wMinute, sysTime.wSecond);

	if( lastTime != NULL && lastIndex != NULL && !memcmp(&sysTime, lastTime, sizeof(SYSTEMTIME)) ) {
		snprintf(fileName, sizeof(fileName), "%s_%d%s", shortName, ++*lastIndex, fileExt);
	} else {
		if( lastTime != NULL && lastIndex != NULL ) {
			*lastIndex = 0;
			*lastTime = sysTime;
		}
		snprintf(fileName, sizeof(fileName), "%s%s", shortName, fileExt);
	}

	PathStringCombine(destName, destSize, filePath, fileName);
	return 0;
}
