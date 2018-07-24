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

#ifndef WINMAIN_H_INCLUDED
#define WINMAIN_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */

int __cdecl RenderErrorBox(int errorCode); // 0x0044E5A0
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nShowCmd); // 0x0044E5E0
int __cdecl Init(bool skipCDInit); // 0x0044E7B0
void __cdecl WinCleanup(); // 0x0044E830
int __cdecl WinGameStart(); // 0x0044E860
void __cdecl WinGameFinish(); // 0x0044E8E0
void __cdecl S_ExitSystem(LPCTSTR message); // 0x0044E950

#endif // WINMAIN_H_INCLUDED
