/*
 * Copyright (c) 2017-2021 Michael Chaban. All rights reserved.
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
#include "game/invtext.h"
#include "game/text.h"
#include "specific/output.h"
#include "specific/smain.h"
#include "global/vars.h"

#define REQ_NEARZ		(8)
#define REQ_MIDZ		(16)
#define REQ_FARZ		(48)

#define STATS_LN_COUNT	(7)
#define STATS_WIDTH		(304)

// Y coordinates relative to the bottom of the screen
#ifdef FEATURE_HUD_IMPROVED
#define STATS_Y_POS		(-44)
#define REQ_LN_HEIGHT (15)

extern DWORD InvTextBoxMode;

static const char MoreDownString[]	= " \x0F                                        \x0F ";
static const char MoreUpString[]	= " \x10                                        \x10 ";
#else // FEATURE_HUD_IMPROVED
#define STATS_Y_POS		(-32)
#define REQ_LN_HEIGHT	(18)
#endif // FEATURE_HUD_IMPROVED

// NOTE: gouraud arrays have been taken from PlayStation version of the game.
// These arrays are not used in the original PC version of the game.

GOURAUD_FILL ReqBgndGour1 = {{
	{0x80002000, 0x80002000, 0x80006000, 0x80002000},
	{0x80002000, 0x80002000, 0x80002000, 0x80006000},
	{0x80006000, 0x80002000, 0x80002000, 0x80002000},
	{0x80002000, 0x80006000, 0x80002000, 0x80002000},
}};
GOURAUD_OUTLINE ReqBgndGour2 = {
	0xFF606060, 0xFF808080, 0xFF202020,
	0xFF000000, 0xFF000000, 0xFF202020,
	0xFF404040, 0xFF404040, 0xFF606060,
};

GOURAUD_FILL ReqMainGour1 = {{
	{0x80000000, 0x80000000, 0x80108038, 0x80000000},
	{0x80000000, 0x80000000, 0x80000000, 0x80108038},
	{0x80108038, 0x80000000, 0x80000000, 0x80000000},
	{0x80000000, 0x80108038, 0x80000000, 0x80000000},
}};
GOURAUD_OUTLINE ReqMainGour2 = {
	0xFF000000, 0xFF000000, 0xFF000000,
	0xFF000000, 0xFF000000, 0xFF000000,
	0xFF000000, 0xFF000000, 0xFF000000,
};

GOURAUD_FILL ReqSelGour1 = {{
	{0x80000000, 0x80000000, 0x8038F080, 0x80000000},
	{0x80000000, 0x80000000, 0x80000000, 0x8038F080},
	{0x8038F080, 0x80000000, 0x80000000, 0x80000000},
	{0x80000000, 0x8038F080, 0x80000000, 0x80000000},
}};
GOURAUD_OUTLINE ReqSelGour2 = {
	0xFF000000, 0xFFFFFFFF, 0xFF000000,
	0xFF38F080, 0xFF000000, 0xFFFFFFFF,
	0xFF000000, 0xFF38F080, 0xFF000000,
};

extern int GF_GetNumSecrets(DWORD levelID);

#ifdef FEATURE_ASSAULT_SAVE
extern void SaveAssault();
#endif // FEATURE_ASSAULT_SAVE

void __cdecl Init_Requester(REQUEST_INFO *req) {
	req->headingText1  = NULL;
	req->headingText2  = NULL;
	req->headingFlags1 = 0;
	req->headingFlags2 = 0;

	req->backgroundText = NULL;
	req->backgroundFlags = 1;
	req->moreupText = NULL;
	req->moreupFlags = 1;
	req->moredownText = NULL;
	req->moredownFlags = 1;

	for( int i=0; i<24; ++i ) {
		req->itemTexts1[i] = NULL;
		req->itemFlags1[i] = 0;
		req->itemTexts2[i] = NULL;
		req->itemFlags2[i] = 0;
	}
	req->itemsCount = 0;

	req->lpItemFlags1 = RequesterItemFlags1;
	req->lpItemFlags2 = RequesterItemFlags2;
#ifdef FEATURE_HUD_IMPROVED
	req->renderWidth  = GetRenderWidthDownscaled();
	req->renderHeight = GetRenderHeightDownscaled();
#else // !FEATURE_HUD_IMPROVED
	req->renderWidth  = GetRenderWidth();
	req->renderHeight = GetRenderHeight();
#endif // FEATURE_HUD_IMPROVED
}

void __cdecl Remove_Requester(REQUEST_INFO *req) {
	T_RemovePrint(req->headingText1);
	req->headingText1 = NULL;
	T_RemovePrint(req->headingText2);
	req->headingText2 = NULL;
	T_RemovePrint(req->backgroundText);
	req->backgroundText = NULL;
	T_RemovePrint(req->moreupText);
	req->moreupText = NULL;
	T_RemovePrint(req->moredownText);
	req->moredownText = NULL;

	for( int i=0; i<24; ++i ) {
		T_RemovePrint(req->itemTexts1[i]);
		req->itemTexts1[i] = NULL;
		T_RemovePrint(req->itemTexts2[i]);
		req->itemTexts2[i] = NULL;
	}
}

void __cdecl ReqItemCentreAlign(REQUEST_INFO *req, TEXT_STR_INFO *textInfo) {
	if( textInfo != NULL ) {
		textInfo->xPos = req->xPos;
		textInfo->bgndOffX = 0;
	}
}

void __cdecl ReqItemLeftAlign(REQUEST_INFO *req, TEXT_STR_INFO *textInfo) {
	int bgndOffX;

	if( textInfo == NULL )
		return;

#ifdef FEATURE_HUD_IMPROVED
	bgndOffX = (req->pixWidth - T_GetTextWidth(textInfo)) / 2 - 8;
#else // FEATURE_HUD_IMPROVED
	DWORD scaleH = GetTextScaleH(textInfo->scaleH);
	bgndOffX = (req->pixWidth * scaleH / PHD_ONE) / 2 - T_GetTextWidth(textInfo) / 2 - (8 * scaleH / PHD_ONE);
#endif // FEATURE_HUD_IMPROVED
	textInfo->xPos = req->xPos - bgndOffX;
	textInfo->bgndOffX = bgndOffX;
}

void __cdecl ReqItemRightAlign(REQUEST_INFO *req, TEXT_STR_INFO *textInfo) {
	int bgndOffX;

	if( textInfo == NULL )
		return;

#ifdef FEATURE_HUD_IMPROVED
	bgndOffX = (req->pixWidth - T_GetTextWidth(textInfo)) / 2 - 8;
#else // FEATURE_HUD_IMPROVED
	DWORD scaleH = GetTextScaleH(textInfo->scaleH);
	bgndOffX = (req->pixWidth * scaleH / PHD_ONE) / 2 - T_GetTextWidth(textInfo) / 2 - (8 * scaleH / PHD_ONE);
#endif // FEATURE_HUD_IMPROVED
	textInfo->xPos = req->xPos + bgndOffX;
	textInfo->bgndOffX = -bgndOffX;
}

int __cdecl Display_Requester(REQUEST_INFO *req, BOOL removeOnDeselect, BOOL isBackground) {
	int i, linesCount, boxHeight, boxOff, linesOff;
	DWORD renderWidth, renderHeight;

	linesCount = req->visibleCount;

#ifdef FEATURE_HUD_IMPROVED
	if( SavedAppSettings.RenderMode == RM_Hardware && InvTextBoxMode ) {
		boxHeight = req->lineHeight * linesCount + 42;
		boxOff = req->yPos - boxHeight + 2;
		linesOff = boxOff + 30;
	} else {
		boxHeight = req->lineHeight * (linesCount + 1) + 22;
		boxOff = req->yPos - boxHeight + 2;
		linesOff = boxOff + req->lineHeight + 10;
	}

	renderWidth = GetRenderWidthDownscaled();
	renderHeight = GetRenderHeightDownscaled();
#else // !FEATURE_HUD_IMPROVED
	boxHeight = req->lineHeight * (linesCount + 1) + 22;
	boxOff = req->yPos - boxHeight + 2;
	linesOff = boxOff + req->lineHeight + 10;

	renderWidth = GetRenderWidth();
	renderHeight = GetRenderHeight();
#endif // FEATURE_HUD_IMPROVED

	if( renderWidth != req->renderWidth || renderHeight != req->renderHeight ) {
		Remove_Requester(req);
		req->renderWidth = renderWidth;
		req->renderHeight = renderHeight;
	}

	req->lpItemFlags1 = RequesterItemFlags1;
	req->lpItemFlags2 = RequesterItemFlags2;

	if( req->itemsCount < req->visibleCount ) {
		linesCount = req->itemsCount;
	}

	// Heading 1
	if( CHK_ANY(req->headingFlags1, REQFLAG_ACTIVE) ) {
		if( req->headingText1 == NULL ) {
			req->headingText1 = T_Print(req->xPos, boxOff, req->zPos, req->headingString1);
			T_CentreH(req->headingText1, 1);
			T_BottomAlign(req->headingText1, 1);
			if( isBackground ) {
				T_AddBackground(req->headingText1, (req->pixWidth - 4), 0, 0, 0, REQ_NEARZ, ICLR_Black, &ReqMainGour1, 2);
				T_AddOutline(req->headingText1, TRUE, ICLR_Orange, &ReqMainGour2, 0);
			}
		}
		if( CHK_ANY(req->headingFlags1, REQFLAG_LEFT) ) {
			ReqItemLeftAlign(req, req->headingText1);
		}
		if( CHK_ANY(req->headingFlags1, REQFLAG_RIGHT) ) {
			ReqItemRightAlign(req, req->headingText1);
		}
	}

	// Heading 2
	if( CHK_ANY(req->headingFlags2, REQFLAG_ACTIVE) ) {
		if( req->headingText2 == NULL ) {
			req->headingText2 = T_Print(req->xPos, boxOff, req->zPos, req->headingString2);
			T_CentreH(req->headingText2, 1);
			T_BottomAlign(req->headingText2, 1);
			if( isBackground ) {
				T_AddBackground(req->headingText2, (req->pixWidth - 4), 0, 0, 0, REQ_NEARZ, ICLR_Black, &ReqMainGour1, 2);
				T_AddOutline(req->headingText2, TRUE, ICLR_Orange, &ReqMainGour2, 0);
			}
		}
		if( CHK_ANY(req->headingFlags2, REQFLAG_LEFT) ) {
			ReqItemLeftAlign(req, req->headingText2);
		}
		if( CHK_ANY(req->headingFlags2, REQFLAG_RIGHT) ) {
			ReqItemRightAlign(req, req->headingText2);
		}
	}

	// Background
	if( isBackground && req->backgroundText == NULL && CHK_ANY(req->backgroundFlags, REQFLAG_ACTIVE) ) {
		req->backgroundText = T_Print(req->xPos, boxOff - 2, 0, " ");
		T_CentreH(req->backgroundText, 1);
		T_BottomAlign(req->backgroundText, 1);
		T_AddBackground(req->backgroundText, req->pixWidth, boxHeight, 0, 0, REQ_FARZ, ICLR_Black, &ReqBgndGour1, 1);
		T_AddOutline(req->backgroundText, TRUE, ICLR_Blue, &ReqBgndGour2, 0);
	}

	// More up
	if( req->lineOffset == 0 ) {
		T_RemovePrint(req->moreupText);
		req->moreupText = NULL;
	}
	else if( req->moreupText == NULL && CHK_ANY(req->moreupFlags, REQFLAG_ACTIVE) ) {
#ifdef FEATURE_HUD_IMPROVED
		if( SavedAppSettings.RenderMode == RM_Hardware && InvTextBoxMode ) {
			req->moreupText = T_Print(req->xPos, boxOff + 15, 0, MoreUpString);
		}
#endif // FEATURE_HUD_IMPROVED
		T_CentreH(req->moreupText, 1);
		T_BottomAlign(req->moreupText, 1);
	}

	// More down
	if( req->itemsCount <= (req->visibleCount + req->lineOffset) ) {
		T_RemovePrint(req->moredownText);
		req->moredownText = 0;
	}
	else if( req->moredownText == NULL && CHK_ANY(req->moredownFlags, REQFLAG_ACTIVE) ) {
#ifdef FEATURE_HUD_IMPROVED
		if( SavedAppSettings.RenderMode == RM_Hardware && InvTextBoxMode ) {
			req->moredownText = T_Print(req->xPos, req->yPos - 12, 0, MoreDownString);
		}
#endif // FEATURE_HUD_IMPROVED
		T_CentreH(req->moredownText, 1);
		T_BottomAlign(req->moredownText, 1);
	}

	// Lines init
	for( i = 0; i < linesCount; ++i ) {
		if( CHK_ANY(req->lpItemFlags1[req->lineOffset + i], REQFLAG_ACTIVE) ) {
			if( req->itemTexts1[i] == NULL ) {
				// NOTE: here was 0 instead of REQ_NEARZ in the original game
				req->itemTexts1[i] = T_Print(0, (linesOff + req->lineHeight * i), REQ_NEARZ, &req->lpItemStrings1[(req->lineOffset + i) * req->itemStringLen]);
				T_CentreH(req->itemTexts1[i], 1);
				T_BottomAlign(req->itemTexts1[i], 1);
			}

			if( CHK_ANY(req->reqFlags, REQFLAG_NOCURSOR) || (req->lineOffset + i != req->selected) ) {
				req->itemTexts1[i]->zPos = REQ_NEARZ; // NOTE: this line is absent in the original game
				T_RemoveBackground(req->itemTexts1[i]);
				T_RemoveOutline(req->itemTexts1[i]);
			} else {
				req->itemTexts1[i]->zPos = 0; // NOTE: this line is absent in the original game
				T_AddBackground(req->itemTexts1[i], (req->pixWidth - 12), 0, 0, 0, REQ_MIDZ, ICLR_Black, &ReqSelGour1, 1);
				T_AddOutline(req->itemTexts1[i], TRUE, ICLR_Orange, &ReqSelGour2, 0);
			}

			if( CHK_ANY(req->lpItemFlags1[req->lineOffset + i], REQFLAG_LEFT) ) {
				ReqItemLeftAlign(req, req->itemTexts1[i]);
			}
			else if( CHK_ANY(req->lpItemFlags1[req->lineOffset + i], REQFLAG_RIGHT) ) {
				ReqItemRightAlign(req, req->itemTexts1[i]);
			}
			else {
				ReqItemCentreAlign(req, req->itemTexts1[i]);
			}
		} else {
			T_RemovePrint(req->itemTexts1[i]);
			T_RemoveBackground(req->itemTexts1[i]);
			T_RemoveOutline(req->itemTexts1[i]);
			req->itemTexts1[i] = NULL;
		}

		if( CHK_ANY(req->lpItemFlags2[req->lineOffset + i], REQFLAG_ACTIVE) ) {
			if( req->itemTexts2[i] == NULL ) {
				// NOTE: here was 0 instead of REQ_NEARZ in the original game
				req->itemTexts2[i] = T_Print(0, (linesOff + req->lineHeight * i), REQ_NEARZ, &req->lpItemStrings2[(req->lineOffset + i) * req->itemStringLen]);
				T_CentreH(req->itemTexts2[i], 1);
				T_BottomAlign(req->itemTexts2[i], 1);
			}

			// NOTE: this code block is absent in the original game
			if( CHK_ANY(req->reqFlags, REQFLAG_NOCURSOR) || (req->lineOffset + i != req->selected) ) {
				req->itemTexts2[i]->zPos = REQ_NEARZ;
			} else {
				req->itemTexts2[i]->zPos = 0;
			}

			if( CHK_ANY(req->lpItemFlags2[req->lineOffset + i], REQFLAG_LEFT) ) {
				ReqItemLeftAlign(req, req->itemTexts2[i]);
			}
			else if( CHK_ANY(req->lpItemFlags2[req->lineOffset + i], REQFLAG_RIGHT) ) {
				ReqItemRightAlign(req, req->itemTexts2[i]);
			}
			else {
				ReqItemCentreAlign(req, req->itemTexts2[i]);
			}
		} else {
			T_RemovePrint(req->itemTexts2[i]);
			T_RemoveBackground(req->itemTexts2[i]);
			T_RemoveOutline(req->itemTexts2[i]);
			req->itemTexts2[i] = NULL;
		}
	}

	// Lines change
	if( req->lineOffset != req->lineOldOffset ) {
		for( i = 0; i < linesCount; ++i ) {
			if( req->itemTexts1[i] != NULL && CHK_ANY(req->lpItemFlags1[req->lineOffset + i], REQFLAG_ACTIVE) ) {
				T_ChangeText(req->itemTexts1[i], &req->lpItemStrings1[(req->lineOffset + i) * req->itemStringLen]);
			}

			if( CHK_ANY(req->lpItemFlags1[req->lineOffset + i], REQFLAG_LEFT) ) {
				ReqItemLeftAlign(req, req->itemTexts1[i]);
			}
			else if( CHK_ANY(req->lpItemFlags1[req->lineOffset + i], REQFLAG_RIGHT) ) {
				ReqItemRightAlign(req, req->itemTexts1[i]);
			}
			else {
				ReqItemCentreAlign(req, req->itemTexts1[i]);
			}

			if( req->itemTexts2[i] != NULL && CHK_ANY(req->lpItemFlags2[req->lineOffset + i], REQFLAG_ACTIVE) ) {
				T_ChangeText(req->itemTexts2[i], &req->lpItemStrings2[(req->lineOffset + i) * req->itemStringLen]);
			}

			if( CHK_ANY(req->lpItemFlags2[req->lineOffset + i], REQFLAG_LEFT) ) {
				ReqItemLeftAlign(req, req->itemTexts2[i]);
			}
			else if( CHK_ANY(req->lpItemFlags2[req->lineOffset + i], REQFLAG_RIGHT) ) {
				ReqItemRightAlign(req, req->itemTexts2[i]);
			}
			else {
				ReqItemCentreAlign(req, req->itemTexts2[i]);
			}
		}
	}

	// Menu down
	if( CHK_ANY(InputDB, IN_BACK) ) {
		if( CHK_ANY(req->reqFlags, REQFLAG_NOCURSOR) ) { // Cursor is disabled. Move the list
			req->lineOldOffset = req->lineOffset;
			if( req->lineOffset < (req->itemsCount - req->visibleCount) ) {
				++req->lineOffset;
			}
		}
		else { // Cursor is enabled. Move the cursor
			if( req->selected < (req->itemsCount - 1) ) {
				++req->selected;
			}
			req->lineOldOffset = req->lineOffset;
			if( req->lineOffset <= (req->selected - req->visibleCount) ) {
				++req->lineOffset;
			}
		}
		return 0;
	}

	// Menu Up
	if( CHK_ANY(InputDB, IN_FORWARD) ) {
		if( CHK_ANY(req->reqFlags, REQFLAG_NOCURSOR) ) { // Cursor is disabled. Move the list
			req->lineOldOffset = req->lineOffset;
			if( req->lineOffset > 0 ) {
				--req->lineOffset;
			}
		}
		else { // Cursor is enabled. Move the cursor
			if( req->selected > 0 ) {
				--req->selected;
			}

			req->lineOldOffset = req->lineOffset;
			if( req->lineOffset > req->selected ) {
				--req->lineOffset;
			}
		}
		return 0;
	}

	// Menu Select
	if( CHK_ANY(InputDB, IN_SELECT) ) {
		// This check prevents from loading an empty saved game slot
		// NOTE: there was an ugly strcmp check in the original game
		if( req == &LoadGameRequester && InventoryExtraData[0] == 0 && !SaveSlotFlags[req->selected] ) {
			InputStatus = 0;
			return 0;
		} else {
			Remove_Requester(req);
			return (req->selected + 1);
		}
	}

	// Menu Deselect
	if( CHK_ANY(InputDB, IN_DESELECT) && removeOnDeselect ) {
		Remove_Requester(req);
		return -1;
	}

	return 0;
}

void __cdecl SetRequesterHeading(REQUEST_INFO *req, const char *string1, DWORD flags1, const char *string2, DWORD flags2) {
	T_RemovePrint(req->headingText1);
	req->headingText1 = NULL;
	T_RemovePrint(req->headingText2);
	req->headingText2 = NULL;

	if( string1 != NULL ) {
		strcpy(req->headingString1, string1);
		req->headingFlags1 = flags1 | REQFLAG_ACTIVE;
	} else {
		strcpy(req->headingString1, "u");
		req->headingFlags1 = 0;
	}

	if( string2 != NULL ) {
		strcpy(req->headingString2, string2);
		req->headingFlags2 = flags2 | REQFLAG_ACTIVE;
	} else {
		strcpy(req->headingString2, "u");
		req->headingFlags2 = 0;
	}
}

void __cdecl RemoveAllReqItems(REQUEST_INFO *req) {
	req->itemsCount = 0;
	req->lineOffset = 0;
	req->selected = 0;
}

void __cdecl ChangeRequesterItem(REQUEST_INFO *req, DWORD itemIdx, const char *string1, DWORD flags1, const char *string2, DWORD flags2) {
	T_RemovePrint(req->itemTexts1[itemIdx]);
	req->itemTexts1[itemIdx] = NULL;
	T_RemovePrint(req->itemTexts2[itemIdx]);
	req->itemTexts2[itemIdx] = NULL;

	if( string1 != NULL ) {
		strcpy(&req->lpItemStrings1[itemIdx * req->itemStringLen], string1);
		req->lpItemFlags1[itemIdx] = flags1 | REQFLAG_ACTIVE;
	} else {
		req->lpItemFlags1[itemIdx] = 0;
	}

	if( string2 != NULL ) {
		strcpy(&req->lpItemStrings2[itemIdx * req->itemStringLen], string2);
		req->lpItemFlags2[itemIdx] = flags2 | REQFLAG_ACTIVE;
	} else {
		req->lpItemFlags2[itemIdx] = 0;
	}
}

void __cdecl AddRequesterItem(REQUEST_INFO *req, const char *string1, DWORD flags1, const char *string2, DWORD flags2) {
	req->lpItemFlags1 = RequesterItemFlags1;
	req->lpItemFlags2 = RequesterItemFlags2;

	if( string1 != NULL ) {
		strcpy(&req->lpItemStrings1[req->itemsCount * req->itemStringLen], string1);
		req->lpItemFlags1[req->itemsCount] = flags1 | REQFLAG_ACTIVE;
	} else {
		req->lpItemFlags1[req->itemsCount] = 0;
	}

	if( string2 != NULL ) {
		strcpy(&req->lpItemStrings2[req->itemsCount * req->itemStringLen], string2);
		req->lpItemFlags2[req->itemsCount] = flags2 | REQFLAG_ACTIVE;
	} else {
		req->lpItemFlags2[req->itemsCount] = 0;
	}

	++req->itemsCount;
}

void __cdecl SetPCRequesterSize(REQUEST_INFO *req, int maxLines, __int16 yPos) {
	req->yPos = yPos;
#ifdef FEATURE_HUD_IMPROVED
	req->visibleCount = GetRenderHeightDownscaled() / 2 / REQ_LN_HEIGHT;
#else // !FEATURE_HUD_IMPROVED
	req->visibleCount = GetRenderHeight() / 2 / REQ_LN_HEIGHT;
#endif // FEATURE_HUD_IMPROVED
	if( req->visibleCount > maxLines )
		req->visibleCount = maxLines;
}

BOOL __cdecl AddAssaultTime(DWORD newTime) {
	int i, j;

	for( i = 0; i < 10; ++i ) {
		if( Assault.bestTime[i] == 0 || newTime < Assault.bestTime[i] ) {
			break;
		}
	}

	if( i > 9 ) { // Not best time at all
		// NOTE: finishCount is not incremented in the original code in case of bad time
		// but in my opinion, bad finish should be counted here, as well as good.
		++Assault.finishCount;
		return FALSE;
	}

	// Insertion required if the finish slot is not the last one
	for( j = 9; j > i; --j ) {
		Assault.bestTime[j] = Assault.bestTime[j - 1];
		Assault.bestFinish[j] = Assault.bestFinish[j - 1];
	}

	Assault.bestTime[i] = newTime;
	Assault.bestFinish[i] = ++Assault.finishCount;

#ifdef FEATURE_ASSAULT_SAVE
	SaveAssault();
#endif // FEATURE_ASSAULT_SAVE
	return TRUE;
}

void __cdecl ShowGymStatsText() {
	static bool isStatsTextReady = false;
	int minutes, seconds, deciseconds;
	char statStr1[32];
	char statStr2[32];

	if( !isStatsTextReady ) {
		StatsRequester.reqFlags |= REQFLAG_NOCURSOR;
		SetPCRequesterSize(&StatsRequester, STATS_LN_COUNT, STATS_Y_POS);

		StatsRequester.lineHeight = REQ_LN_HEIGHT;
		StatsRequester.itemsCount = 0;
		StatsRequester.selected = 0;
		StatsRequester.lineOffset = 0;
		StatsRequester.lineOldOffset = 0;
		StatsRequester.pixWidth = STATS_WIDTH;
		StatsRequester.xPos = 0;
		StatsRequester.zPos = 0;
		StatsRequester.lpItemStrings1 = (char *)SaveGameStrings1;
		StatsRequester.lpItemStrings2 = (char *)SaveGameStrings2;
		StatsRequester.itemStringLen = 50;

		Init_Requester(&StatsRequester);
		SetRequesterHeading(&StatsRequester, GF_GameStringTable[GSI_String_BestTimes], 0, NULL, 0);

		for( int i = 0; i < 10; ++i ) {
			if( Assault.bestTime[i] == 0 ) {
				if( i == 0 ) {
					AddRequesterItem(&StatsRequester, GF_GameStringTable[GSI_String_NoTimesSet], 0, NULL, 0);
				}
				break;
			}
			deciseconds = Assault.bestTime[i] % 30 / 3;
			seconds = Assault.bestTime[i] / 30 % 60;
			minutes = Assault.bestTime[i] / 30 / 60;
			sprintf(statStr1, "%2d: %s %d", (i + 1), GF_GameStringTable[GSI_String_Finish], (int)Assault.bestFinish[i]);
			sprintf(statStr2, "%02d:%02d.%-2d", minutes, seconds, deciseconds);
			AddRequesterItem(&StatsRequester, statStr1, REQFLAG_LEFT, statStr2, REQFLAG_RIGHT);
		}

		isStatsTextReady = true;
	}
	else if( Display_Requester(&StatsRequester, TRUE, TRUE) ) {
		isStatsTextReady = false;
	}
	else {
		InputDB = 0;
		InputStatus = 0;
	}
}

void __cdecl ShowStatsText(char *timeString, BOOL removeOnDeselect) {
	static bool isStatsTextReady = false;
	bool isSecret1, isSecret2, isSecret3;
	int bufLen, distance;
	char bufStr[32];

	if( !isStatsTextReady ) {
		StatsRequester.reqFlags |= REQFLAG_NOCURSOR;
		SetPCRequesterSize(&StatsRequester, STATS_LN_COUNT, STATS_Y_POS);

		StatsRequester.lineHeight = REQ_LN_HEIGHT;
		StatsRequester.itemsCount = 0;
		StatsRequester.selected = 0;
		StatsRequester.lineOffset = 0;
		StatsRequester.lineOldOffset = 0;
		StatsRequester.pixWidth = STATS_WIDTH;
		StatsRequester.xPos = 0;
		StatsRequester.zPos = 0;
		StatsRequester.lpItemStrings1 = (char *)SaveGameStrings1;
		StatsRequester.lpItemStrings2 = (char *)SaveGameStrings2;
		StatsRequester.itemStringLen = 50;

		Init_Requester(&StatsRequester);
		SetRequesterHeading(&StatsRequester, GF_LevelNamesStringTable[CurrentLevel], 0, NULL, 0);

		// Time taken
		AddRequesterItem(&StatsRequester, GF_GameStringTable[GSI_String_TimeTaken], REQFLAG_LEFT, timeString, REQFLAG_RIGHT);

		// Secrets found
		if( GF_NumSecrets > 0 ) {
			isSecret1 = CHK_ANY(SaveGame.statistics.secrets, 1);
			isSecret2 = CHK_ANY(SaveGame.statistics.secrets, 2);
			isSecret3 = CHK_ANY(SaveGame.statistics.secrets, 4);

			if( isSecret1 || isSecret2 || isSecret3 ) {
				bufLen = 0;

				if( isSecret1 ) {
					bufStr[bufLen++] = CHAR_SECRET1;
				} else {
					bufStr[bufLen++] = ' ';
					bufStr[bufLen++] = ' ';
					bufStr[bufLen++] = ' ';
				}

				if( isSecret2 ) {
					bufStr[bufLen++] = CHAR_SECRET2;
				} else {
					bufStr[bufLen++] = ' ';
					bufStr[bufLen++] = ' ';
					bufStr[bufLen++] = ' ';
				}

				if( isSecret3 ) {
					bufStr[bufLen++] = CHAR_SECRET3;
				} else {
					bufStr[bufLen++] = ' ';
					bufStr[bufLen++] = ' ';
					bufStr[bufLen++] = ' ';
				}

				bufStr[bufLen] = 0;
			} else {
				sprintf(bufStr, GF_GameStringTable[GSI_String_None]);
			}
			AddRequesterItem(&StatsRequester, GF_GameStringTable[GSI_String_SecretsFound], REQFLAG_LEFT, bufStr, REQFLAG_RIGHT);
		}

		// Kills
		sprintf(bufStr, "%d", (int)SaveGame.statistics.kills);
		AddRequesterItem(&StatsRequester, GF_GameStringTable[GSI_String_Kills], REQFLAG_LEFT, bufStr, REQFLAG_RIGHT);

		// Ammo used
		sprintf(bufStr, "%d", (int)SaveGame.statistics.shots);
		AddRequesterItem(&StatsRequester, GF_GameStringTable[GSI_String_AmmoUsed], REQFLAG_LEFT, bufStr, REQFLAG_RIGHT);

		// Hits
		sprintf(bufStr, "%d", (int)SaveGame.statistics.hits);
		AddRequesterItem(&StatsRequester, GF_GameStringTable[GSI_String_Hits], REQFLAG_LEFT, bufStr, REQFLAG_RIGHT);

		// HealthPacks used
		if( (SaveGame.statistics.mediPacks % 2) == 0 ) {
			sprintf(bufStr, "%d.0", (int)(SaveGame.statistics.mediPacks / 2) );
		} else {
			sprintf(bufStr, "%d.5", (int)(SaveGame.statistics.mediPacks / 2) );
		}
		AddRequesterItem(&StatsRequester, GF_GameStringTable[GSI_String_HealthPacksUsed], REQFLAG_LEFT, bufStr, REQFLAG_RIGHT);

		// Distance travelled
		distance = SaveGame.statistics.distance / 445;
		if( distance < 1000 ) {
			sprintf(bufStr, "%dm", distance);
		} else {
			sprintf(bufStr, "%d.%02dkm", (distance / 1000), (distance % 100));
		}
		AddRequesterItem(&StatsRequester, GF_GameStringTable[GSI_String_DistanceTravelled], REQFLAG_LEFT, bufStr, REQFLAG_RIGHT);

		isStatsTextReady = true;
	} else {
		ChangeRequesterItem(&StatsRequester, 0, GF_GameStringTable[GSI_String_TimeTaken], REQFLAG_LEFT, timeString, REQFLAG_RIGHT);
		if( Display_Requester(&StatsRequester, removeOnDeselect, TRUE) ) {
			isStatsTextReady = false;
		} else {
			InputDB = 0;
			InputStatus = 0;
		}
	}
}

void __cdecl ShowEndStatsText() {
	static bool isStatsTextReady = false;
	int i, numLevels;
	int total, maxTotal;
	int hours, minutes, seconds;
	char bufStr[32];

	numLevels = GF_GameFlow.num_Levels - GF_GameFlow.num_Demos;
	CLAMPG(numLevels, CurrentLevel+1); // NOTE: Fix for Gold. Don't count statistics for bonus levels!

	if( !isStatsTextReady ) {
		StatsRequester.reqFlags |= REQFLAG_NOCURSOR;
		SetPCRequesterSize(&StatsRequester, STATS_LN_COUNT, STATS_Y_POS);

		StatsRequester.lineHeight = REQ_LN_HEIGHT;
		StatsRequester.itemsCount = 0;
		StatsRequester.selected = 0;
		StatsRequester.lineOffset = 0;
		StatsRequester.lineOldOffset = 0;
		StatsRequester.pixWidth = STATS_WIDTH;
		StatsRequester.xPos = 0;
		StatsRequester.zPos = 0;
		StatsRequester.lpItemStrings1 = (char *)SaveGameStrings1;
		StatsRequester.lpItemStrings2 = (char *)SaveGameStrings2;
		StatsRequester.itemStringLen = 50;

		Init_Requester(&StatsRequester);
		SetRequesterHeading(&StatsRequester, GF_GameStringTable[GSI_String_FinalStatistics], 0, NULL, 0);

		// Time taken
		total = 0;
		for( i = 1; i < numLevels; ++i ) {
			total += SaveGame.start[i].statistics.timer;
		}
		seconds = total / 30 % 60;
		minutes = total / 30 / 60 % 60;
		hours   = total / 30 / 60 / 60;
		sprintf(bufStr, "%02d:%02d:%02d", hours, minutes, seconds);
		AddRequesterItem(&StatsRequester, GF_GameStringTable[GSI_String_TimeTaken], REQFLAG_LEFT, bufStr, REQFLAG_RIGHT);

		// Secrets found
		total = 0;
		maxTotal = 0;
		// NOTE: In the original code there was hardcode for secrets: for( i = 1; i < (numLevels - 2); ++i )
		for( i = 1; i < numLevels; ++i ) {
			total += CHK_ANY(SaveGame.start[i].statistics.secrets, 1) ? 1 : 0;
			total += CHK_ANY(SaveGame.start[i].statistics.secrets, 2) ? 1 : 0;
			total += CHK_ANY(SaveGame.start[i].statistics.secrets, 4) ? 1 : 0;
			maxTotal += GF_GetNumSecrets(i); // In the original code there is 3 instead of GF_GetNumSecrets function
		}
		sprintf(bufStr, "%d %s %d", total, GF_GameStringTable[GSI_String_Of], maxTotal);
#ifdef FEATURE_GOLD
		// NOTE: this check is presented in the "Golden Mask" only
		if( total == maxTotal ) {
			EnableLevelSelect();
		}
#endif // FEATURE_GOLD
		AddRequesterItem(&StatsRequester, GF_GameStringTable[GSI_String_SecretsFound], REQFLAG_LEFT, bufStr, REQFLAG_RIGHT);

		// Kills
		total = 0;
		for( i = 1; i < numLevels; ++i ) {
			total += SaveGame.start[i].statistics.kills;
		}
		sprintf(bufStr, "%d", total);
		AddRequesterItem(&StatsRequester, GF_GameStringTable[GSI_String_Kills], REQFLAG_LEFT, bufStr, REQFLAG_RIGHT);

		// Ammo used
		total = 0;
		for( i = 1; i < numLevels; ++i ) {
			total += SaveGame.start[i].statistics.shots;
		}
		sprintf(bufStr, "%d", total);
		AddRequesterItem(&StatsRequester, GF_GameStringTable[GSI_String_AmmoUsed], REQFLAG_LEFT, bufStr, REQFLAG_RIGHT);

		// Hits
		total = 0;
		for( i = 1; i < numLevels; ++i ) {
			total += SaveGame.start[i].statistics.hits;
		}
		sprintf(bufStr, "%d", total);
		AddRequesterItem(&StatsRequester, GF_GameStringTable[GSI_String_Hits], REQFLAG_LEFT, bufStr, REQFLAG_RIGHT);

		// HealthPacks used
		total = 0;
		for( i = 1; i < numLevels; ++i ) {
			total += SaveGame.start[i].statistics.mediPacks;
		}
		if( (total % 2) == 0 ) {
			sprintf(bufStr, "%d.0", (total / 2));
		} else {
			sprintf(bufStr, "%d.5", (total / 2));
		}
		AddRequesterItem(&StatsRequester, GF_GameStringTable[GSI_String_HealthPacksUsed], REQFLAG_LEFT, bufStr, REQFLAG_RIGHT);

		// Distance travelled
		total = 0;
		for( i = 1; i < numLevels; ++i ) {
			total += SaveGame.start[i].statistics.distance;
		}
		total /= 445;
		if( total < 1000 )
			sprintf(bufStr, "%dm", total);
		else
			sprintf(bufStr, "%d.%02dkm", total / 1000, total % 100);
		AddRequesterItem(&StatsRequester, GF_GameStringTable[GSI_String_DistanceTravelled], REQFLAG_LEFT, bufStr, REQFLAG_RIGHT);

		isStatsTextReady = true;
	}
	else if( Display_Requester(&StatsRequester, FALSE, TRUE) ) {
		isStatsTextReady = false;
	} else {
		InputDB = 0;
		InputStatus = 0;
	}
}

/*
 * Inject function
 */
void Inject_InvText() {
	INJECT(0x00425580, Init_Requester);
	INJECT(0x00425610, Remove_Requester);
	INJECT(0x004256C0, ReqItemCentreAlign);
	INJECT(0x004256E0, ReqItemLeftAlign);
	INJECT(0x00425740, ReqItemRightAlign);
	INJECT(0x004257A0, Display_Requester);
	INJECT(0x00426010, SetRequesterHeading);
	INJECT(0x004260C0, RemoveAllReqItems);
	INJECT(0x004260E0, ChangeRequesterItem);
	INJECT(0x004261A0, AddRequesterItem);
	INJECT(0x00426250, SetPCRequesterSize);
	INJECT(0x00426290, AddAssaultTime);
	INJECT(0x00426320, ShowGymStatsText);
	INJECT(0x00426500, ShowStatsText);
	INJECT(0x004268A0, ShowEndStatsText);
}
