/*
 * Copyright (c) 2017-2019 Michael Chaban. All rights reserved.
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
#include "specific/game.h"
#include "game/camera.h"
#include "game/control.h"
#include "game/draw.h"
#include "game/inventory.h"
#include "game/invtext.h"
#include "game/savegame.h"
#include "game/setup.h"
#include "game/text.h"
#include "specific/background.h"
#include "specific/display.h"
#include "specific/file.h"
#include "specific/frontend.h"
#include "specific/init.h"
#include "specific/input.h"
#include "specific/output.h"
#include "specific/sndpc.h"
#include "specific/winvid.h"
#include "specific/texture.h"
#include "global/vars.h"

#ifdef FEATURE_BACKGROUND_IMPROVED
#include "modding/background_new.h"
#endif // FEATURE_BACKGROUND_IMPROVED

__int16 __cdecl StartGame(int levelID, int levelType) {
	// TODO: define levelType as enum
	if( levelType == 1 || levelType == 2 || levelType == 3 )
		CurrentLevel = levelID;

	if( levelType != 2 )
		ModifyStartInfo(levelID);

	IsTitleLoaded = FALSE;

	if( levelType != 2 )
		InitialiseLevelFlags();

	if( !InitialiseLevel(levelID, levelType) ) {
		CurrentLevel = 0;
		return GF_EXIT_GAME;
	}

	int res = GameLoop(FALSE);
	switch( res ) {
		case GF_EXIT_GAME :
			CurrentLevel = 0;
			return GF_EXIT_GAME;

		case GF_START_DEMO :
		case GF_EXIT_TO_TITLE :
			return res;
	}

	if( IsLevelComplete ) {
		if( (GF_GameFlow.flags & GFF_DemoVersion) != 0 && GF_GameFlow.singleLevel )
			return GF_EXIT_TO_TITLE;

		if( CurrentLevel == 0 ) { // Assault (Lara Home)
			S_FadeToBlack();
			return GF_EXIT_TO_TITLE;
		} else { // Normal level
			S_FadeInInventory(TRUE);
			return GF_LEVEL_COMPLETE | CurrentLevel;
		}
	}

	S_FadeToBlack();
	if( !InventoryChosen )
		return GF_EXIT_TO_TITLE;

	if( InventoryExtraData[0] == 0 ) { // passport page (Load game)
		S_LoadGame(&SaveGame, sizeof(SAVEGAME_INFO), InventoryExtraData[1]);
		return GF_START_SAVEDGAME | InventoryExtraData[1]; // saveGame slot
	}

	if( InventoryExtraData[0] == 1 ) { // passport page (New game)
		if( (GF_GameFlow.flags & GFF_SelectAnyLevel) != 0 )
			return GF_START_GAME | (InventoryExtraData[1] + 1); // selected level
		else
			return GF_START_GAME | 1; // first level
	}
	return GF_EXIT_TO_TITLE;
}

int __cdecl GameLoop(BOOL demoMode) {
	int result;
	int nFrames;

	OverlayStatus = 1;
	InitialiseCamera();
	NoInputCounter = 0;

	result = ControlPhase(1, demoMode);
	while( result == 0 ) {
		nFrames = DrawPhaseGame();
		result = IsGameToExit ? GF_EXIT_GAME : ControlPhase(nFrames, demoMode);
	}

	S_SoundStopAllSamples();

#ifdef FEATURE_BACKGROUND_IMPROVED
	// this fixes issue when the final "bath" cut scene is cut off
	if( CHK_ANY(Lara_Flags, LARA_EXTRA_ANIM) && LaraItem->currentAnimState == EXTRA_FINALANIM ) {
		S_CopyScreenToBuffer();
		BGND2_ShowPicture(0, 0, 20, 70, FALSE);
	}
#endif // FEATURE_BACKGROUND_IMPROVED
	S_CDStop();
	if( MusicVolume > 0 ) {
		S_CDVolume(MusicVolume * 25 + 5);
	}
	return result;
}

int __cdecl LevelCompleteSequence() {
	return GF_EXIT_TO_TITLE;
}

int __cdecl LevelStats(int levelID) {
	int hours, minutes, seconds;
	char timeString[100] = {0};

	SaveGame.start[levelID].statistics = SaveGame.statistics;

	seconds = SaveGame.statistics.timer / 30 % 60;
	minutes = SaveGame.statistics.timer / 30 / 60 % 60;
	hours   = SaveGame.statistics.timer / 30 / 60 / 60;
	sprintf(timeString, "%02d:%02d:%02d", hours, minutes, seconds);

	S_CDPlay(GF_GameFlow.levelCompleteTrack, FALSE);

	TempVideoAdjust(HiRes, 1.0);
	FadeToPal(30, GamePalette8);
	T_InitPrint();
	S_CopyScreenToBuffer();

	while( CHK_ANY(InputStatus, IN_SELECT) )
		S_UpdateInput();

	while( !CHK_ANY(InputStatus, IN_SELECT) ) {
		S_InitialisePolyList(FALSE);
		S_CopyBufferToScreen();
		S_UpdateInput();
		// NOTE: this check is absent in the original game
		if( IsGameToExit ) {
			break;
		}
		if( IsResetFlag ) {
			InputStatus = IN_SELECT;
		}
		InputDB = GetDebouncedInput(InputStatus);
		ShowStatsText(timeString, FALSE);
		T_DrawText();
		S_OutputPolyList();
		S_DumpScreen();
	}

	++levelID;
	CreateStartInfo(levelID);
	SaveGame.currentLevel = levelID;
	SaveGame.start[CurrentLevel].flags &= ~1;
	S_FadeToBlack();
	TempVideoRemove();
	return 0;
}

int __cdecl GameStats() {
	SaveGame.start[CurrentLevel].statistics = SaveGame.statistics;

	TempVideoAdjust(HiRes, 1.0); // NOTE: this line was not in the original code
	T_InitPrint();

	while( CHK_ANY(InputStatus, IN_SELECT) )
		S_UpdateInput();

	while( !CHK_ANY(InputStatus, IN_SELECT) ) {
		S_InitialisePolyList(FALSE);
		S_CopyBufferToScreen();
		S_UpdateInput();
		// NOTE: this check is absent in the original game
		if( IsGameToExit ) {
			break;
		}
		if( IsResetFlag ) {
			InputStatus = IN_SELECT;
		}
		InputDB = GetDebouncedInput(InputStatus);
		ShowEndStatsText();
		T_DrawText();
		S_OutputPolyList();
		S_DumpScreen();
	}

	SaveGame.bonusFlag = true;
	for( int i = 1; i <= GF_GameFlow.num_Levels; ++i ) {
		ModifyStartInfo(i);
	}
	SaveGame.currentLevel = 1;
	S_DontDisplayPicture();
	TempVideoRemove(); // NOTE: this line was not in the original code
	return 0;
}

int __cdecl GetRandomControl() {
	RandomControl = RandomControl * 1103515245 + 12345;
	return (RandomControl / 0x10000) & 0x7FFF;
	// NOTE: the original game code was: return (RandomControl >> 10) & 0x7FFF;
	// instead of the correct ANSI one: return (RandomControl >> 0x10) & 0x7FFF;
}

void __cdecl SeedRandomControl(int seed) {
	RandomControl = seed;
}

int __cdecl GetRandomDraw() {
	RandomDraw = RandomDraw * 1103515245 + 12345;
	return (RandomDraw / 0x10000) & 0x7FFF;
	// NOTE: the original game code was: return (RandomDraw >> 10) & 0x7FFF;
	// instead of the correct ANSI one: return (RandomDraw >> 0x10) & 0x7FFF;
}

void __cdecl SeedRandomDraw(int seed) {
	RandomDraw = seed;
}

void __cdecl GetValidLevelsList(REQUEST_INFO *req) {
	RemoveAllReqItems(req);
	for( DWORD i = 1; i < GF_GameFlow.num_Levels; ++i )
		AddRequesterItem(req, GF_LevelNamesStringTable[i], 0, NULL, 0);
}

void __cdecl GetSavedGamesList(REQUEST_INFO *req) {
	SetPCRequesterSize(req, 10, -32);

	if( req->selected >= req->visibleCount ) {
		req->lineOffset = req->selected - req->visibleCount + 1;
	}
	memcpy(RequesterItemFlags1, SaveGameItemFlags1, sizeof(RequesterItemFlags1));
	memcpy(RequesterItemFlags2, SaveGameItemFlags2, sizeof(RequesterItemFlags2));
}

void __cdecl DisplayCredits() {
	int i;
	RGB888 palette[256];
#ifdef FEATURE_BACKGROUND_IMPROVED
	char fileName[64] = {0};
#else // !FEATURE_BACKGROUND_IMPROVED
	DWORD bytesRead;
	HANDLE hFile;
	DWORD fileSize[9];
	DWORD bitmapSize;
	BYTE *fileData[9];
	BYTE *bitmapData;
	LPCSTR fullPath;
	char fileName[64] = "data\\credit0?.pcx";
#endif // FEATURE_BACKGROUND_IMPROVED

	S_FadeToBlack(); // fade out 12 frames / 0.4 seconds (software renderer only)
	S_UnloadLevelFile();
	TempVideoAdjust(HiRes, 1.0); // NOTE: this line was not in the original code

	if( !InitialiseLevel(0, 0) ) // init title level
		return;

	memcpy(palette, GamePalette8, sizeof(GamePalette8));
	memset(GamePalette8, 0, sizeof(GamePalette8));

	IsVidModeLock = true;
	FadeToPal(0, GamePalette8); // fade in instantly
#ifdef FEATURE_BACKGROUND_IMPROVED
	S_CDPlay(52, FALSE);

	// slideshow loop
	for( i=1; i<100; ++i ) {
		snprintf(fileName, sizeof(fileName), "data\\credit%02d.pcx", i);
		if( !BGND2_LoadPicture(fileName, FALSE, FALSE) ) {
			BGND2_ShowPicture(30, 225, 10, 2, FALSE);
		}
		S_DontDisplayPicture();

		if( IsGameToExit ) {
			return;
		}
	}
	memcpy(GamePalette8, palette, sizeof(GamePalette8));
	// NOTE: background for final statistics (picture is presented but not used in the original game)
	if( !BGND2_LoadPicture("data\\end.pcx", TRUE, FALSE) ) {
		BGND2_ShowPicture(30, 0, 0, 0, FALSE);
	}
	IsVidModeLock = false;
	TempVideoRemove();

#else // !FEATURE_BACKGROUND_IMPROVED
	// credit files load loop (preload all files may be reasonable because of CDAudio issues, since PCX are on CD too)
	for( i=0; i<9; ++i ) {
		fileName[12] = '0'+i+1;
		fullPath = GetFullPath(fileName);
		hFile = CreateFile(fullPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if( hFile != INVALID_HANDLE_VALUE ) {
			fileSize[i] = GetFileSize(hFile, 0);
			fileData[i] = (BYTE *)game_malloc(fileSize[i], GBUF_LoadPiccyBuffer);
			ReadFile(hFile, fileData[i], fileSize[i], &bytesRead, 0);
			CloseHandle(hFile);
		}
	}

	bitmapSize = 640*480;
	bitmapData = (BYTE *)game_malloc(bitmapSize, GBUF_LoadPiccyBuffer);
	S_CDPlay(52, FALSE);

	// slideshow loop
	for( i=0; i<9; ++i ) {
		DecompPCX(fileData[i], fileSize[i], bitmapData, PicPalette);

		if( SavedAppSettings.RenderMode == RM_Software )
			WinVidCopyBitmapToBuffer(PictureBufferSurface, bitmapData);
		else
			BGND_Make640x480(bitmapData, PicPalette);

		CopyBitmapPalette(PicPalette, bitmapData, bitmapSize, GamePalette8);
		S_InitialisePolyList(FALSE);
		S_CopyBufferToScreen();
		S_OutputPolyList();
		S_DumpScreen();

		FadeToPal(30, GamePalette8); // fade in 30 frames / 1.0 seconds (software renderer only)
		S_Wait(225 * TICKS_PER_FRAME, FALSE); // wait 225 frames / 7.5 seconds (disable keyboard)
		S_FadeToBlack(); // fade out 12 frames / 0.4 seconds (software renderer only)
		S_DontDisplayPicture();

		if( IsGameToExit )
			break;
	}

	memcpy(GamePalette8, palette, sizeof(GamePalette8));
	S_Wait(150 * TICKS_PER_FRAME, FALSE); // wait 150 frames / 5 seconds (disable keyboard)
	FadeToPal(30, GamePalette8); // fade in 30 frames / 1.0 seconds (software renderer only)
	IsVidModeLock = false;
	TempVideoRemove(); // NOTE: this line was not in the original code

	// NOTE: here is no game_free for game_malloc. Memory will be free in S_DisplayPicture by init_game_malloc call
#endif // FEATURE_BACKGROUND_IMPROVED
}

BOOL __cdecl S_FrontEndCheck() {
	HANDLE hFile;
	DWORD bytesRead;
	DWORD saveCounter;
	char fileName[16] = {0};
	char levelName[80] = {0};
	char saveCountStr[16] = {0};

	Init_Requester(&LoadGameRequester);
	SavedGamesCount = 0;

	for( int i=0; i<16; ++i ) {
		wsprintf(fileName, "savegame.%d", i);
		hFile = CreateFile(fileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if( hFile == INVALID_HANDLE_VALUE ) {
			AddRequesterItem(&LoadGameRequester, GF_SpecificStringTable[SSI_EmptySlot], 0, NULL, 0);
			SaveSlotFlags[i] = 0;
			continue;
		}

		ReadFile(hFile, levelName, 75, &bytesRead, NULL);
		ReadFile(hFile, &saveCounter, sizeof(DWORD), &bytesRead, NULL);
		CloseHandle(hFile);

		wsprintf(saveCountStr, "%d", saveCounter);
		AddRequesterItem(&LoadGameRequester, levelName, REQFLAG_LEFT, saveCountStr, REQFLAG_RIGHT);

		if( saveCounter > SaveCounter ) {
			SaveCounter = saveCounter;
			LoadGameRequester.selected = i;
		}

		SaveSlotFlags[i] = 1;
		++SavedGamesCount;
	}
	memcpy(SaveGameItemFlags1, RequesterItemFlags1, sizeof(SaveGameItemFlags1));
	memcpy(SaveGameItemFlags2, RequesterItemFlags2, sizeof(SaveGameItemFlags2));

	++SaveCounter;
	return 1;
}

BOOL __cdecl S_SaveGame(LPCVOID saveData, DWORD saveSize, int slotNumber) {
	HANDLE hFile;
	DWORD bytesWritten;
	char fileName[16] = {0};
	char levelName[80] = {0};
	char saveCountStr[16] = {0};


	wsprintf(fileName, "savegame.%d", slotNumber);
	hFile = CreateFile(fileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if( hFile == INVALID_HANDLE_VALUE )
		return FALSE;

	wsprintf(levelName, "%s", GF_LevelNamesStringTable[SaveGame.currentLevel]);
	WriteFile(hFile, levelName, 75, &bytesWritten, NULL);
	WriteFile(hFile, &SaveCounter, sizeof(DWORD), &bytesWritten, NULL);
	WriteFile(hFile, saveData, saveSize, &bytesWritten, NULL);
	CloseHandle(hFile);

	wsprintf(saveCountStr, "%d", SaveCounter);
	ChangeRequesterItem(&LoadGameRequester, slotNumber, levelName, REQFLAG_LEFT, saveCountStr, REQFLAG_RIGHT);
	++SaveCounter;

	// NOTE: There was no such check in the original code. Save files counter incremented anyway
	if( SaveSlotFlags[slotNumber] == 0 ) {
		SaveSlotFlags[slotNumber] = 1;
		++SavedGamesCount;
	}

	return TRUE;
}

BOOL __cdecl S_LoadGame(LPVOID saveData, DWORD saveSize, int slotNumber) {
	HANDLE hFile;
	DWORD bytesRead;
	DWORD saveCounter;
	char fileName[16] = {0};
	char levelName[80] = {0};

	wsprintf(fileName, "savegame.%d", slotNumber);
	hFile = CreateFile(fileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if( hFile == INVALID_HANDLE_VALUE )
		return FALSE;

	ReadFile(hFile, levelName, 75, &bytesRead, NULL);
	ReadFile(hFile, &saveCounter, sizeof(DWORD), &bytesRead, NULL);
	ReadFile(hFile, saveData, saveSize, &bytesRead, NULL);
	CloseHandle(hFile);
	return TRUE;
}

/*
 * Inject function
 */
void Inject_Game() {
	INJECT(0x0044C550, StartGame);
	INJECT(0x0044C6A0, GameLoop);
	INJECT(0x0044C740, LevelCompleteSequence);
	INJECT(0x0044C750, LevelStats);
	INJECT(0x0044C920, GameStats);
	INJECT(0x0044CA40, GetRandomControl);
	INJECT(0x0044CA60, SeedRandomControl);
	INJECT(0x0044CA70, GetRandomDraw);
	INJECT(0x0044CA90, SeedRandomDraw);
	INJECT(0x0044CAA0, GetValidLevelsList);
	INJECT(0x0044CAF0, GetSavedGamesList);
	INJECT(0x0044CB40, DisplayCredits);
	INJECT(0x0044CD80, S_FrontEndCheck);
	INJECT(0x0044CEF0, S_SaveGame);
	INJECT(0x0044D010, S_LoadGame);
}
