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
#include "specific/init.h"
#include "3dsystem/phd_math.h"
#include "specific/game.h"
#include "specific/winmain.h"
#include "global/vars.h"
#include <time.h>

// related to GAMEALLOC_BUFFER enum
static LPCTSTR BufferNames[] = {
	"Temp Alloc",
	"Texture Pages",
	"Mesh Pointers",
	"Meshes",
	"Anims",
	"Structs",
	"Ranges",
	"Commands",
	"Bones",
	"Frames",
	"Room Textures",
	"Room Infos",
	"Room Mesh",
	"Room Door",
	"Room Floor",
	"Room Lights",
	"Room Static Mesh Infos",
	"Floor Data",
	"ITEMS!!",
	"Cameras",
	"Sound FX",
	"Boxes",
	"Overlaps",
	"GroundZone",
	"FlyZone",
	"Animating Texture Ranges",
	"Cinematic Frames",
	"LoadDemo Buffer",
	"SaveDemo Buffer",
	"Cinematic Effects",
	"Mummy Head Turn",
	"Extra Door stuff",
	"Effects_Array",
	"Creature Data",
	"Creature LOT",
	"Sample Infos",
	"Samples",
	"Sample Offsets",
	"Rolling Ball Stuff",
	"Skidoo Stuff",
	"Load Piccy Buffer",
	"FMV Buffers",
	"Polygon Buffers",
	"Order Tables",
	"CLUTs",
	"Texture Infos",
	"Sprite Infos",
};

BOOL __cdecl S_InitialiseSystem() {
	S_SeedRandom();
	DumpX = 0;
	DumpY = 0;
	DumpWidth = GameVidWidth;
	DumpHeight = GameVidHeight;
	CalculateWibbleTable();
	GameMemorySize = 0x380000; // 3.5 MB
	return TRUE;
}

void __cdecl ShutdownGame() {
	if( GameMemoryPointer != NULL ) {
		GlobalFree(GameMemoryPointer);
		GameMemoryPointer = NULL;
	}
}

void __cdecl init_game_malloc() {
	GameAllocMemPointer = GameMemoryPointer;
	GameAllocMemFree = GameMemorySize;
	GameAllocMemUsed = 0;
}

void *__cdecl game_malloc(DWORD allocSize, DWORD bufIndex) {
	DWORD alignedSize = (allocSize + 3) & ~3;
	if( alignedSize > GameAllocMemFree ) {
		wsprintf(StringToShow, "game_malloc(): OUT OF MEMORY %s %d", BufferNames[bufIndex], alignedSize);
		S_ExitSystem(StringToShow);
		return NULL; // the app is terminated here
	}

	void *result = GameAllocMemPointer;
	GameAllocMemFree -= alignedSize;
	GameAllocMemUsed += alignedSize;
	GameAllocMemPointer += alignedSize;
	return result;
}

void __cdecl game_free(DWORD freeSize) {
	DWORD alignedSize = (freeSize + 3) & ~3;

	GameAllocMemPointer -= alignedSize;
	GameAllocMemFree += alignedSize;
	GameAllocMemUsed -= alignedSize;
}

void __cdecl CalculateWibbleTable() {
	// This function calculates water effect tables
	for( int i=0; i < WIBBLE_SIZE; ++i ) {
		int sine = phd_sin(i * PHD_360 / WIBBLE_SIZE); // 360 degrees divided by wibble number
		WibbleTable[i] = sine * MAX_WIBBLE >> W2V_SHIFT;
		ShadesTable[i] = sine * MAX_SHADE  >> W2V_SHIFT;
		RandomTable[i] = (GetRandomDraw() >> 5) - 0x01FF;
		for( int j=0; j < WIBBLE_SIZE; ++j )
			RoomLightTables[i].table[j] = (j - (WIBBLE_SIZE/2)) * i * MAX_ROOMLIGHT_UNIT / (WIBBLE_SIZE-1);
	}
}

void __cdecl S_SeedRandom() {
	time_t t = time(NULL);
	struct tm *ltm = localtime(&t);

	SeedRandomControl(ltm->tm_sec + 57 * ltm->tm_min + 3543 * ltm->tm_hour);
	SeedRandomDraw(ltm->tm_sec + 43 * ltm->tm_min + 3477 * ltm->tm_hour);
}

/*
 * Inject function
 */
void Inject_Init() {
	INJECT(0x0044D6E0, S_InitialiseSystem);
	INJECT(0x0044D730, ShutdownGame);
	INJECT(0x0044D750, init_game_malloc);
	INJECT(0x0044D780, game_malloc);
	INJECT(0x0044D800, game_free);
	INJECT(0x0044D840, CalculateWibbleTable);
	INJECT(0x0044D930, S_SeedRandom);
}
