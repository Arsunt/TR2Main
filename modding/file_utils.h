/*
 * Copyright (c) 2017-2020 Michael Chaban. All rights reserved.
 * Original game is created by Core Design Ltd. in 1997.
 * Lara Croft and Tomb Raider are trademarks of Embracer Group AB.
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

#ifndef FILE_UTILS_H_INCLUDED
#define FILE_UTILS_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
int PathStringCombine(LPSTR destPath, DWORD destSize, LPCSTR filePath, LPCSTR fileName);

int AutoSelectExtension(LPSTR fileName, const STRING_FIXED4 *exts, DWORD num_exts);

int AutoSelectPathAndExtension(LPSTR fileName, LPCSTR path, const STRING_FIXED4 *exts, DWORD num_exts);

int CreateDirectories(LPCSTR path, bool isFileName);

int CreateSequenceFilename(LPSTR destName, DWORD destSize, LPCSTR filePath, LPCSTR fileExt, LPCSTR nameBase, int seqDigits, int seqNumber);

int CreateDateTimeFilename(LPSTR fileName, DWORD destSize, LPCSTR filePath, LPCSTR fileExt, SYSTEMTIME *lastTime, int *lastIndex);

int AddFilenameSuffix(LPSTR destName, DWORD destSize, LPCSTR fileName, LPCSTR suffix);

LPCVOID GetResourceData(LPCTSTR resName, LPDWORD resSize);

#endif // FILE_UTILS_H_INCLUDED
