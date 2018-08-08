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
	char checkFileName[256] = {0};
	int ext;

	if( fileName == NULL || !*fileName ) {
		return -1;
	}

	if( path != NULL && *path ) {
		LPCSTR fname = PathFindFileName(fileName);
		PathCombine(checkFileName, path, fname);

		ext = AutoSelectExtension(checkFileName, exts, num_exts);

		if( ext >= 0 ) {
			strncpy(fileName, checkFileName, sizeof(checkFileName));
			return 1;
		}
	}

	ext = AutoSelectExtension(fileName, exts, num_exts);
	return ( ext >= 0 ) ? 0 : -1;
}
