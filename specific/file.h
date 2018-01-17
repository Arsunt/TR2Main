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

#ifndef FILE_H_INCLUDED
#define FILE_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
BOOL __cdecl ReadFileSync(HANDLE hFile, LPVOID lpBuffer, DWORD nBytesToRead, LPDWORD lpnBytesRead, LPOVERLAPPED lpOverlapped); // 0x00449980
BOOL __cdecl LoadTexturePages(HANDLE hFile); // 0x004499D0
BOOL __cdecl LoadRooms(HANDLE hFile); // 0x00449B60
void __cdecl AdjustTextureUVs(bool resetUvFix); // 0x00449F00
BOOL __cdecl LoadObjects(HANDLE hFile); // 0x00449FA0
BOOL __cdecl LoadSprites(HANDLE hFile); // 0x0044A520
BOOL __cdecl LoadItems(HANDLE hFile); // 0x0044A660
BOOL __cdecl LoadDepthQ(HANDLE hFile); // 0x0044A840
BOOL __cdecl LoadPalettes(HANDLE hFile); // 0x0044A9D0
BOOL __cdecl LoadCameras(HANDLE hFile); // 0x0044AA50
BOOL __cdecl LoadSoundEffects(HANDLE hFile); // 0x0044AAB0
BOOL __cdecl LoadBoxes(HANDLE hFile); // 0x0044AB10
BOOL __cdecl LoadAnimatedTextures(HANDLE hFile); // 0x0044AD40
BOOL __cdecl LoadCinematic(HANDLE hFile); // 0x0044ADA0
BOOL __cdecl LoadDemo(HANDLE hFile); // 0x0044AE20
void __cdecl LoadDemoExternal(LPCSTR levelName); // 0x0044AEB0
BOOL __cdecl LoadSamples(HANDLE hFile); // 0x0044AF50
void __cdecl ChangeFileNameExtension(char *fileName, const char *fileExt); // 0x0044B1C0
LPCTSTR __cdecl GetFullPath(LPCTSTR path); // 0x0044B200
BOOL __cdecl SelectDrive(); // 0x0044B230
BOOL __cdecl LoadLevel(LPCTSTR fileName, int levelID); // 0x0044B310
BOOL __cdecl S_LoadLevelFile(LPCTSTR fileName, int levelID); // 0x0044B560
void __cdecl S_UnloadLevelFile(); // 0x0044B580
void __cdecl S_AdjustTexelCoordinates(); // 0x0044B5B0
BOOL __cdecl S_ReloadLevelGraphics(BOOL reloadPalettes, BOOL reloadTexPages); // 0x0044B5D0
BOOL __cdecl Read_Strings(DWORD dwCount, char **stringTable, char **stringBuffer, LPDWORD lpBufferSize, HANDLE hFile); // 0x0044B6A0
BOOL __cdecl S_LoadGameFlow(LPCTSTR fileName); // 0x0044B770

#endif // FILE_H_INCLUDED
