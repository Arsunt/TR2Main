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
extern DWORD StatsBackgroundMode;
#endif // FEATURE_BACKGROUND_IMPROVED

#ifdef FEATURE_INPUT_IMPROVED
#include "modding/joy_output.h"
#endif // FEATURE_INPUT_IMPROVED

#ifdef FEATURE_GOLD
extern bool IsGold();
#endif // FEATURE_GOLD

#ifdef FEATURE_VIDEOFX_IMPROVED
extern void ResetGoldenLaraAlpha();
#endif // FEATURE_VIDEOFX_IMPROVED

#ifdef FEATURE_HUD_IMPROVED
extern bool GF_IsFinalLevel(DWORD levelID);
extern void RemoveJoystickHintText(bool isSelect, bool isContinue, bool isDeselect);
extern void DisplayJoystickHintText(bool isSelect, bool isContinue, bool isDeselect);
#endif // FEATURE_HUD_IMPROVED

#ifdef FEATURE_SUBFOLDERS
#include "modding/file_utils.h"

static int GetSaveFileName(LPSTR destName, DWORD destSize, int slotNumber) {
	if( destName == NULL || destSize == 0 || slotNumber < 0 ) {
		return -1;
	}
#ifdef FEATURE_GOLD
	snprintf(destName, destSize, ".\\saves%s\\savegame.%d", IsGold()?"Gold":"", slotNumber);
#else // !FEATURE_GOLD
	snprintf(destName, destSize, ".\\saves\\savegame.%d", slotNumber);
#endif // !FEATURE_GOLD
	return 0;
}
#endif // FEATURE_SUBFOLDERS

DWORD SavegameSlots = 16;

__int16 __cdecl StartGame(int levelID, GF_LEVEL_TYPE levelType) {
	if( levelType == GFL_NORMAL || levelType == GFL_SAVED || levelType == GFL_DEMO )
		CurrentLevel = levelID;

	if( levelType != GFL_SAVED )
		ModifyStartInfo(levelID);

	IsTitleLoaded = FALSE;

	if( levelType != GFL_SAVED )
		InitialiseLevelFlags();

	if( !InitialiseLevel(levelID, levelType) ) {
		CurrentLevel = 0;
		return GF_EXIT_GAME;
	}

#ifdef FEATURE_VIDEOFX_IMPROVED
	ResetGoldenLaraAlpha();
#endif // FEATURE_VIDEOFX_IMPROVED

	int res = GameLoop(FALSE);
#ifdef FEATURE_INPUT_IMPROVED
	JoyOutputReset();
#endif // FEATURE_INPUT_IMPROVED
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

	if( InventoryExtraData[0] == 1 ) { // passport page (New game | Restart Level)
#ifdef FEATURE_HUD_IMPROVED
		if( InventoryMode == INV_DeathMode ) {
			// check if previous level is not Assault and not a final one (before bonus level)
			if( CurrentLevel > 1 && !GF_IsFinalLevel(CurrentLevel-1) ) {
				SaveGame.start[CurrentLevel] = SaveGame.start[CurrentLevel-1];
			} else {
				ModifyStartInfo(CurrentLevel);
			}
			return GF_START_GAME | CurrentLevel;
		}
#endif // FEATURE_HUD_IMPROVED
		if( (GF_GameFlow.flags & GFF_SelectAnyLevel) != 0 )
			return GF_START_GAME | (InventoryExtraData[1] + 1); // selected level
		else
			return GF_START_GAME | 1; // first level
	}
	return GF_EXIT_TO_TITLE;
}

int __cdecl GameLoop(BOOL demoMode) {
	int result;
	int nTicks;

	OverlayStatus = 1;
	InitialiseCamera();
	NoInputCounter = 0;

	result = ControlPhase(1, demoMode);
	while( result == 0 ) {
		nTicks = DrawPhaseGame();
		result = IsGameToExit ? GF_EXIT_GAME : ControlPhase(nTicks, demoMode);
	}

	S_SoundStopAllSamples();

#ifdef FEATURE_BACKGROUND_IMPROVED
	// this fixes issue when the final "bath" cut scene is cut off
	if( Lara.extra_anim && LaraItem->currentAnimState == EXTRA_FINALANIM ) {
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

	CreateStartInfo(levelID); // NOTE: this line is absent in the original code, but it's required for "Restart Level" feature
	SaveGame.start[levelID].statistics = SaveGame.statistics;

	seconds = SaveGame.statistics.timer / 30 % 60;
	minutes = SaveGame.statistics.timer / 30 / 60 % 60;
	hours   = SaveGame.statistics.timer / 30 / 60 / 60;
	sprintf(timeString, "%02d:%02d:%02d", hours, minutes, seconds);

#ifdef FEATURE_BACKGROUND_IMPROVED
	TempVideoAdjust(HiRes, 1.0);
	if( SavedAppSettings.RenderMode == RM_Software ) {
		S_CopyScreenToBuffer();
		RGB888 gamePal[256];
		memcpy(gamePal, GamePalette8, sizeof(gamePal));
		for( int i = 0; i<256; ++i ) {
			GamePalette8[i] = gamePal[DepthQIndex[i]];
		}
		FadeToPal(10, GamePalette8);
		memcpy(GamePalette8, gamePal, sizeof(gamePal));
		FadeToPal(0, GamePalette8);
	} else if( !StatsBackgroundMode ) {
		S_CopyScreenToBuffer();
		while( !IsGameToExit && BGND2_FadeTo(128, 0) > 128 ) {
			S_InitialisePolyList(FALSE);
			S_CopyBufferToScreen();
			if( S_UpdateInput() || IsResetFlag ) {
				break;
			}
			S_OutputPolyList();
			S_DumpScreen();
		}
	} else {
		S_FadeToBlack();
	}
	T_InitPrint();

	S_CDPlay(GF_GameFlow.levelCompleteTrack, FALSE);
#else // // FEATURE_BACKGROUND_IMPROVED
	S_CDPlay(GF_GameFlow.levelCompleteTrack, FALSE);

	TempVideoAdjust(HiRes, 1.0);
	FadeToPal(30, GamePalette8);
	T_InitPrint();
	S_CopyScreenToBuffer();
#endif // FEATURE_BACKGROUND_IMPROVED

	while( CHK_ANY(InputStatus, IN_SELECT) )
		S_UpdateInput();

#ifdef FEATURE_HUD_IMPROVED
	DisplayJoystickHintText(false, true, false);
#endif // FEATURE_HUD_IMPROVED
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
#ifdef FEATURE_HUD_IMPROVED
	RemoveJoystickHintText(false, true, false);
#endif // FEATURE_HUD_IMPROVED

	// NOTE: This LevelStats bonusFlag activation is not presented in the original game.
	// If the level is final, but there no GFE_GAMECOMPLETE in the script, just activate Game+ here.
	if( levelID == GF_GameFlow.num_Levels-GF_GameFlow.num_Demos-1 ) {
		SaveGame.bonusFlag = true;
	}

	++levelID;
	CreateStartInfo(levelID);
	SaveGame.currentLevel = levelID;
	SaveGame.start[CurrentLevel].available = 0; // make level not available
	S_FadeToBlack();
	TempVideoRemove();
	return 0;
}

int __cdecl GameStats(int levelID) {
	SaveGame.start[CurrentLevel].statistics = SaveGame.statistics;

	TempVideoAdjust(HiRes, 1.0); // NOTE: this line was not in the original code
	T_InitPrint();

	while( CHK_ANY(InputStatus, IN_SELECT) )
		S_UpdateInput();

#ifdef FEATURE_HUD_IMPROVED
	DisplayJoystickHintText(false, true, false);
#endif // FEATURE_HUD_IMPROVED
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
#ifdef FEATURE_HUD_IMPROVED
	RemoveJoystickHintText(false, true, false);
#endif // FEATURE_HUD_IMPROVED

	// NOTE: in the original game, there is slightly different bonusFlag activation.
	// Here removed bonuses initialization, and added the check that the level is final
	if( CurrentLevel == GF_GameFlow.num_Levels-GF_GameFlow.num_Demos-1 ) {
		SaveGame.bonusFlag = true;
	}

#ifdef FEATURE_BACKGROUND_IMPROVED
	BGND2_ShowPicture(0, 0, 10, 2, FALSE);
#endif // FEATURE_BACKGROUND_IMPROVED
	S_DontDisplayPicture();
	TempVideoRemove(); // NOTE: this line was not in the original code
	return 0;
}

int __cdecl GetRandomControl() {
	RandomControl = RandomControl * 1103515245 + 12345;
	return (RandomControl >> 10) & 0x7FFF;
	// NOTE: the shift value should be 0x10, but the original game has 10,
	// it left "as is" to save consistency with the original game.
}

void __cdecl SeedRandomControl(int seed) {
	RandomControl = seed;
}

int __cdecl GetRandomDraw() {
	RandomDraw = RandomDraw * 1103515245 + 12345;
	return (RandomDraw >> 10) & 0x7FFF;
	// NOTE: the shift value should be 0x10, but the original game has 10,
	// it left "as is" to save consistency with the original game.
}

void __cdecl SeedRandomDraw(int seed) {
	RandomDraw = seed;
}

void __cdecl GetValidLevelsList(REQUEST_INFO *req) {
	RemoveAllReqItems(req);

	// NOTE: this check fixes original game bug.
	// Now demo levels are excluded from the *"New Game"* menu,
	// if the *"Select Level"* option is active.
	DWORD numLevels = GF_GameFlow.num_Levels - GF_GameFlow.num_Demos;

	for( DWORD i = 1; i < numLevels; ++i )
		AddRequesterItem(req, GF_LevelNamesStringTable[i], 0, NULL, 0);
}

void __cdecl GetSavedGamesList(REQUEST_INFO *req) {
	extern void SetPassportRequesterSize(REQUEST_INFO *req);
	SetPassportRequesterSize(req);

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
#ifdef FEATURE_GOLD
	DWORD fileSize[10];
	BYTE *fileData[10];
#else // !FEATURE_GOLD
	DWORD fileSize[9];
	BYTE *fileData[9];
#endif // !FEATURE_GOLD
	DWORD bitmapSize;
	BYTE *bitmapData;
	LPCSTR fullPath;
	char fileName[64] = "data\\credit0?.pcx";
#endif // FEATURE_BACKGROUND_IMPROVED

	S_FadeToBlack(); // fade out 12 frames / 0.4 seconds
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
#ifdef FEATURE_GOLD
	for( i=IsGold()?0:1; i<100; ++i ) {
#else // !FEATURE_GOLD
	for( i=1; i<100; ++i ) {
#endif // !FEATURE_GOLD
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
#ifdef FEATURE_GOLD
	for( i=0; i<(IsGold()?10:9); ++i ) {
		fileName[12] = '0'+i+(IsGold()?0:1);
#else // !FEATURE_GOLD
	for( i=0; i<9; ++i ) {
		fileName[12] = '0'+i+1;
#endif // !FEATURE_GOLD
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
#ifdef FEATURE_GOLD
	for( i=0; i<(IsGold()?10:9); ++i ) {
#else // !FEATURE_GOLD
	for( i=0; i<9; ++i ) {
#endif // !FEATURE_GOLD
		DecompPCX(fileData[i], fileSize[i], bitmapData, PicPalette);

		if( SavedAppSettings.RenderMode == RM_Software ) {
#if (DIRECT3D_VERSION >= 0x900)
			if( PictureBuffer.bitmap != NULL)
				memcpy(PictureBuffer.bitmap, bitmapData, PictureBuffer.width * PictureBuffer.height);
#else // (DIRECT3D_VERSION >= 0x900)
			WinVidCopyBitmapToBuffer(PictureBufferSurface, bitmapData);
#endif // (DIRECT3D_VERSION >= 0x900)
		} else {
			BGND_Make640x480(bitmapData, PicPalette);
		}

#if (DIRECT3D_VERSION >= 0x900)
		memcpy(GamePalette8, PicPalette, sizeof(GamePalette8));
#else // (DIRECT3D_VERSION >= 0x900)
		CopyBitmapPalette(PicPalette, bitmapData, bitmapSize, GamePalette8);
#endif // (DIRECT3D_VERSION >= 0x900)
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
	char levelName[80] = {0};
	char saveCountStr[16] = {0};
#ifdef FEATURE_SUBFOLDERS
	char fileName[256] = {0};
#else // !FEATURE_SUBFOLDERS
	char fileName[16] = {0};
#endif // !FEATURE_SUBFOLDERS

	Init_Requester(&LoadGameRequester);
	SavedGamesCount = 0;

	for( DWORD i=0; i<SavegameSlots; ++i ) {
#ifdef FEATURE_SUBFOLDERS
		GetSaveFileName(fileName, sizeof(fileName), i);
#else // !FEATURE_SUBFOLDERS
		wsprintf(fileName, "savegame.%d", i);
#endif // !FEATURE_SUBFOLDERS
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
	char levelName[80] = {0};
	char saveCountStr[16] = {0};

#ifdef FEATURE_SUBFOLDERS
	char fileName[256] = {0};
	GetSaveFileName(fileName, sizeof(fileName), slotNumber);
	if( CreateDirectories(fileName, true) ) {
		return FALSE;
	}
#else // !FEATURE_SUBFOLDERS
	char fileName[16] = {0};
	wsprintf(fileName, "savegame.%d", slotNumber);
#endif // !FEATURE_SUBFOLDERS

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
	// NOTE: the next two lines fix a bug in the original game:
	// When player saves the game to EMPTY SLOT, save counter won't appear until the game relaunch
	SaveGameItemFlags1[slotNumber] = RequesterItemFlags1[slotNumber];
	SaveGameItemFlags2[slotNumber] = RequesterItemFlags2[slotNumber];
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
	char levelName[80] = {0};

#ifdef FEATURE_SUBFOLDERS
	char fileName[256] = {0};
	GetSaveFileName(fileName, sizeof(fileName), slotNumber);
#else // !FEATURE_SUBFOLDERS
	char fileName[16] = {0};
	wsprintf(fileName, "savegame.%d", slotNumber);
#endif // !FEATURE_SUBFOLDERS

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
