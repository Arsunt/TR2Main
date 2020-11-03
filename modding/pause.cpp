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
#include "modding/pause.h"
#include "game/health.h"
#include "game/inventory.h"
#include "game/invtext.h"
#include "game/sound.h"
#include "game/text.h"
#include "specific/display.h"
#include "specific/input.h"
#include "specific/output.h"
#include "specific/sndpc.h"
#include "global/vars.h"

#ifdef FEATURE_INPUT_IMPROVED
#include "modding/joy_output.h"
#endif // FEATURE_INPUT_IMPROVED

#ifdef FEATURE_BACKGROUND_IMPROVED
static TEXT_STR_INFO *PausedText = NULL;

static void RemovePausedText() {
	T_RemovePrint(PausedText);
	PausedText = NULL;
}

static void DisplayPausedText() {
	static int renderHeight = 0;
	if( renderHeight != GetRenderHeightDownscaled() ) {
		RemovePausedText();
		renderHeight = GetRenderHeightDownscaled();
	}
	if( PausedText == NULL ) {
		PausedText = T_Print(0, -24, 5, "Paused");
		T_CentreH(PausedText, 1);
		T_BottomAlign(PausedText, 1);
	}
}

static int DisplayPauseRequester(const char *header, const char *option1, const char *option2, UINT16 selected) {
	static bool isPauseTextReady = false;
	if( !isPauseTextReady ) {
		StatsRequester.reqFlags &= ~REQFLAG_NOCURSOR;
		SetPCRequesterSize(&StatsRequester, 2, -48);

		StatsRequester.lineHeight = 18;
		StatsRequester.itemsCount = 0;
		StatsRequester.selected = selected;
		StatsRequester.lineOffset = 0;
		StatsRequester.lineOldOffset = 0;
		StatsRequester.pixWidth = 100;
		StatsRequester.xPos = 0;
		StatsRequester.zPos = 0;
		StatsRequester.lpItemStrings1 = (char *)SaveGameStrings1;
		StatsRequester.lpItemStrings2 = (char *)SaveGameStrings2;
		StatsRequester.itemStringLen = 50;

		Init_Requester(&StatsRequester);
		SetRequesterHeading(&StatsRequester, header, 0, NULL, 0);

		memset(SaveGameStrings1, 0, sizeof(SaveGameStrings1));
		memset(SaveGameStrings2, 0, sizeof(SaveGameStrings2));
		AddRequesterItem(&StatsRequester, option1, 0, NULL, 0);
		AddRequesterItem(&StatsRequester, option2, 0, NULL, 0);

		isPauseTextReady = true;
		InputDB = 0;
		InputStatus = 0;
	}

	int select = Display_Requester(&StatsRequester, 0, 0);
	if( select > 0 ) {
		isPauseTextReady = false;
	} else {
		InputDB = 0;
		InputStatus = 0;
	}
	return select;
}

static int PauseRequester() {
	static int state = 0;
	int select = 0;

	InputDB = GetDebouncedInput(InputStatus);
	switch( state ) {
		case 0:
			if( CHK_ANY(InputDB, IN_PAUSE) ) {
				select = 1;
				break;
			}
			if( CHK_ANY(InputDB, IN_OPTION) ) {
				state = 1;
			}
			if( state != 1 ) break;
			// fall through
		case 1:
			switch( DisplayPauseRequester(CHK_ANY(GF_GameFlow.flags, GFF_DemoVersion)? "Exit Demo?" : "Exit to title?", "Continue", "Quit", 0) ) {
				case 1: select = 1; break;
				case 2: state = 2; break;
			};
			if( state != 2 ) break;
			// fall through
		case 2:
			switch( DisplayPauseRequester("Are you sure?", "Yes", "No", 1) ) {
				case 1: select = -1; break;
				case 2: select = 1; break;
			};
			break;
	}
	if( select ) state = 0;
	return select;
}

bool S_Pause() {
	InventoryMode = INV_PauseMode;
	T_RemovePrint(AmmoTextInfo);
	AmmoTextInfo = NULL;
	S_FadeInInventory(TRUE);
	SOUND_Stop();
	S_CDVolume(0);
	TempVideoAdjust(HiRes, 1.0);
	IsVidModeLock = true;
	S_SetupAboveWater(FALSE);

	int select = 0;
	do {
		S_InitialisePolyList(FALSE);
		DoInventoryBackground();
		DisplayPausedText();
		DrawModeInfo();
		T_DrawText();
		S_OutputPolyList();
		SOUND_EndScene();
		S_DumpScreen();
#ifdef FEATURE_INPUT_IMPROVED
		UpdateJoyOutput(false);
#endif // FEATURE_INPUT_IMPROVED
	} while( !S_UpdateInput() && 0 == (select=PauseRequester()) );

	Remove_Requester(&StatsRequester);
	RemovePausedText();
	IsVidModeLock = false;
	TempVideoRemove();
#ifdef FEATURE_AUDIO_IMPROVED
	if( Camera.underwater ) {
		extern double UnderwaterMusicMute;
		double volume = (1.0 - UnderwaterMusicMute) * (double)(MusicVolume * 25 + 5);
		if( volume >= 1.0 ) {
			S_CDVolume((DWORD)volume);
		} else {
			S_CDVolume(0);
		}
	} else {
		S_CDVolume(MusicVolume * 25 + 5);
	}
#else // FEATURE_AUDIO_IMPROVED
	S_CDVolume(MusicVolume * 25 + 5);
#endif // FEATURE_AUDIO_IMPROVED
	S_FadeOutInventory(TRUE);
	InventoryChosen = ID_PASSPORT_OPTION;
	InventoryExtraData[0] = 2; // set the last passport page in case if "Exit to Title" is selected
	InventoryMode = INV_GameMode;
	return (select < 0);
}
#endif // FEATURE_BACKGROUND_IMPROVED
