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
#include "game/invtext.h"
#include "game/text.h"
#include "specific/output.h"
#include "global/vars.h"

#define REQ_NEARZ		(8)
#define REQ_MIDZ		(16)
#define REQ_FARZ		(48)

#define REQ_LN_HEIGHT	(18)
#define STATS_LN_COUNT	(7)
// Y coordinates relative to the bottom of the screen
#define STATS_Y_POS		(-32)
#define STATS_WIDTH		(304)

// These gouraud arrays are not used in the game (apparently were not ready for release)
static __int16 ReqBgndGour1[16] = {
	0x1E00, 0x1E00, 0x1A00, 0x1E00,
	0x1E00, 0x1E00, 0x1E00, 0x1A00,
	0x1A00, 0x1E00, 0x1E00, 0x1E00,
	0x1E00, 0x1A00, 0x1E00, 0x1E00,
};
static __int16 ReqBgndGour2[9] = {
	0x1A00, 0x1800, 0x1E00,
	0x2000, 0x2000, 0x1E00,
	0x1C00, 0x1C00, 0x1A00,
};

static __int16 ReqMainGour1[16] = {
	0x2000, 0x2000, 0x1800, 0x2000,
	0x2000, 0x2000, 0x2000, 0x1800,
	0x1800, 0x2000, 0x2000, 0x2000,
	0x2000, 0x1800, 0x2000, 0x2000,
};
static __int16 ReqMainGour2[9] = {
	0x2000, 0x2000, 0x2000,
	0x2000, 0x2000, 0x2000,
	0x2000, 0x2000, 0x2000,
};

static __int16 ReqSelGour1[16] = {
	0x2000, 0x2000, 0x1A00, 0x2000,
	0x2000, 0x2000, 0x2000, 0x1A00,
	0x1A00, 0x2000, 0x2000, 0x2000,
	0x2000, 0x1A00, 0x2000, 0x2000,
};
static __int16 ReqSelGour2[9] = {
	0x2000, 0x1010, 0x2000,
	0x1400, 0x2000, 0x1010,
	0x2000, 0x1400, 0x2000,
};

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
#ifdef FEATURE_FOV_FIX
	req->renderWidth  = GetRenderWidthDownscaled();
	req->renderHeight = GetRenderHeightDownscaled();
#else // !FEATURE_FOV_FIX
	req->renderWidth  = GetRenderWidth();
	req->renderHeight = GetRenderHeight();
#endif // FEATURE_FOV_FIX
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
	DWORD scaleH;
	int bgndOffX;

	if( textInfo == NULL )
		return;

	scaleH = GetTextScaleH(textInfo->scaleH);
	bgndOffX = (req->pixWidth * scaleH / PHD_ONE) / 2 - T_GetTextWidth(textInfo) / 2 - (8 * scaleH / PHD_ONE);
	textInfo->xPos = req->xPos - bgndOffX;
	textInfo->bgndOffX = bgndOffX;
}

void __cdecl ReqItemRightAlign(REQUEST_INFO *req, TEXT_STR_INFO *textInfo) {
	DWORD scaleH;
	int bgndOffX;

	if( textInfo == NULL )
		return;

	scaleH = GetTextScaleH(textInfo->scaleH);
	bgndOffX = (req->pixWidth * scaleH / PHD_ONE) / 2 - T_GetTextWidth(textInfo) / 2 - (8 * scaleH / PHD_ONE);
	textInfo->xPos = req->xPos + bgndOffX;
	textInfo->bgndOffX = -bgndOffX;
}

int __cdecl Display_Requester(REQUEST_INFO *req, BOOL removeOnDeselect, BOOL isBackground) {
	int i, linesCount, linesHeight, linesOff;
	DWORD renderWidth, renderHeight;

	linesCount = req->visibleCount;
	linesHeight = req->lineHeight * linesCount + 10;
	linesOff = req->yPos - linesHeight;

#ifdef FEATURE_FOV_FIX
	renderWidth = GetRenderWidthDownscaled();
	renderHeight = GetRenderHeightDownscaled();
#else // !FEATURE_FOV_FIX
	renderWidth = GetRenderWidth();
	renderHeight = GetRenderHeight();
#endif // FEATURE_FOV_FIX

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
	if( CHK_ANY(req->headingFlags1, 1) ) {
		if( req->headingText1 == NULL ) {
			req->headingText1 = T_Print(req->xPos, (linesOff - req->lineHeight - 10), req->zPos, req->headingString1);
			T_CentreH(req->headingText1, 1);
			T_BottomAlign(req->headingText1, 1);
			if( isBackground ) {
				T_AddBackground(req->headingText1, (req->pixWidth - 4), 0, 0, 0, REQ_NEARZ, ICLR_Black, ReqMainGour1, 2);
				T_AddOutline(req->headingText1, TRUE, ICLR_Orange, ReqMainGour2, 0);
			}
		}
		if( CHK_ANY(req->headingFlags1, 2) ) {
			ReqItemLeftAlign(req, req->headingText1);
		}
		if( CHK_ANY(req->headingFlags1, 4) ) {
			ReqItemRightAlign(req, req->headingText1);
		}
	}

	// Heading 2
	if( CHK_ANY(req->headingFlags2, 1) ) {
		if( req->headingText2 == NULL ) {
			req->headingText2 = T_Print(req->xPos, (linesOff - req->lineHeight - 10), req->zPos, req->headingString2);
			T_CentreH(req->headingText2, 1);
			T_BottomAlign(req->headingText2, 1);
			if( isBackground ) {
				T_AddBackground(req->headingText2, (req->pixWidth - 4), 0, 0, 0, REQ_NEARZ, ICLR_Black, ReqMainGour1, 2);
				T_AddOutline(req->headingText2, TRUE, ICLR_Orange, ReqMainGour2, 0);
			}
		}
		if( CHK_ANY(req->headingFlags2, 2) ) {
			ReqItemLeftAlign(req, req->headingText2);
		}
		if( CHK_ANY(req->headingFlags2, 4) ) {
			ReqItemRightAlign(req, req->headingText2);
		}
	}

	// Background
	if( isBackground && req->backgroundText == NULL && CHK_ANY(req->backgroundFlags, 1) ) {
		req->backgroundText = T_Print(req->xPos, (linesOff - req->lineHeight - 12), 0, " ");
		T_CentreH(req->backgroundText, 1);
		T_BottomAlign(req->backgroundText, 1);
		T_AddBackground(req->backgroundText, req->pixWidth, (req->lineHeight + linesHeight + 12), 0, 0, REQ_FARZ, ICLR_Black, ReqBgndGour1, 1);
		T_AddOutline(req->backgroundText, TRUE, ICLR_Blue, ReqBgndGour2, 0);
	}

	// More up
	if( req->lineOffset == 0 ) {
		T_RemovePrint(req->moreupText);
		req->moreupText = NULL;
	}
	else if( req->moreupText == NULL && CHK_ANY(req->moreupFlags, 1) ) {
		T_CentreH(req->moreupText, 1);
		T_BottomAlign(req->moreupText, 1);
	}

	// More down
	if( req->itemsCount <= (req->visibleCount + req->lineOffset) ) {
		T_RemovePrint(req->moredownText);
		req->moredownText = 0;
	}
	else if( req->moredownText == NULL && CHK_ANY(req->moredownFlags, 1) ) {
		T_CentreH(req->moredownText, 1);
		T_BottomAlign(req->moredownText, 1);
	}

	// Lines init
	for( i = 0; i < linesCount; ++i ) {
		if( CHK_ANY(req->lpItemFlags1[req->lineOffset + i], 1) ) {
			if( req->itemTexts1[i] == NULL ) {
				req->itemTexts1[i] = T_Print(0, (linesOff + req->lineHeight * i), 0, &req->lpItemStrings1[(req->lineOffset + i) * req->itemStringLen]);
				T_CentreH(req->itemTexts1[i], 1);
				T_BottomAlign(req->itemTexts1[i], 1);
			}

			if( CHK_ANY(req->reqFlags, 1) || (req->lineOffset + i != req->selected) ) {
				T_RemoveBackground(req->itemTexts1[i]);
				T_RemoveOutline(req->itemTexts1[i]);
			} else {
				T_AddBackground(req->itemTexts1[i], (req->pixWidth - 12), 0, 0, 0, REQ_MIDZ, ICLR_Black, ReqSelGour1, 1);
				T_AddOutline(req->itemTexts1[i], TRUE, ICLR_Orange, ReqSelGour2, 0);
			}

			if( CHK_ANY(req->lpItemFlags1[req->lineOffset + i], 2) ) {
				ReqItemLeftAlign(req, req->itemTexts1[i]);
			}
			else if( CHK_ANY(req->lpItemFlags1[req->lineOffset + i], 4) ) {
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

		if( CHK_ANY(req->lpItemFlags2[req->lineOffset + i], 1) ) {
			if( req->itemTexts2[i] == NULL ) {
				req->itemTexts2[i] = T_Print(0, (linesOff + req->lineHeight * i), 0, &req->lpItemStrings2[(req->lineOffset + i) * req->itemStringLen]);
				T_CentreH(req->itemTexts2[i], 1);
				T_BottomAlign(req->itemTexts2[i], 1);
			}

			if( CHK_ANY(req->lpItemFlags2[req->lineOffset + i], 2) ) {
				ReqItemLeftAlign(req, req->itemTexts2[i]);
			}
			else if( CHK_ANY(req->lpItemFlags2[req->lineOffset + i], 4) ) {
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
			if( req->itemTexts1[i] != NULL && CHK_ANY(req->lpItemFlags1[req->lineOffset + i], 1) ) {
				T_ChangeText(req->itemTexts1[i], &req->lpItemStrings1[(req->lineOffset + i) * req->itemStringLen]);
			}

			if( CHK_ANY(req->lpItemFlags1[req->lineOffset + i], 2) ) {
				ReqItemLeftAlign(req, req->itemTexts1[i]);
			}
			else if( CHK_ANY(req->lpItemFlags1[req->lineOffset + i], 4) ) {
				ReqItemRightAlign(req, req->itemTexts1[i]);
			}
			else {
				ReqItemCentreAlign(req, req->itemTexts1[i]);
			}

			if( req->itemTexts2[i] != NULL && CHK_ANY(req->lpItemFlags2[req->lineOffset + i], 1) ) {
				T_ChangeText(req->itemTexts2[i], &req->lpItemStrings2[(req->lineOffset + i) * req->itemStringLen]);
			}

			if( CHK_ANY(req->lpItemFlags2[req->lineOffset + i], 2) ) {
				ReqItemLeftAlign(req, req->itemTexts2[i]);
			}
			else if( CHK_ANY(req->lpItemFlags2[req->lineOffset + i], 4) ) {
				ReqItemRightAlign(req, req->itemTexts2[i]);
			}
			else {
				ReqItemCentreAlign(req, req->itemTexts2[i]);
			}
		}
	}

	// Menu down
	if( CHK_ANY(InputDB, IN_BACK) ) {
		if( CHK_ANY(req->reqFlags, 1) ) { // Cursor is disabled. Move the list
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
		if( CHK_ANY(req->reqFlags, 1) ) { // Cursor is disabled. Move the list
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
		if( strncmp(req->itemTexts1[req->selected - req->lineOffset]->pString, GF_SpecificStringTable[SSI_EmptySlot], 12) ||
			strcmp(PassportTextInfo->pString, GF_GameStringTable[GSI_Passport_LoadGame]) )
		{
			Remove_Requester(req);
			return (req->selected + 1);
		} else {
			InputStatus = 0;
			return 0;
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
		req->headingFlags1 = flags1 | 1;
	} else {
		strcpy(req->headingString1, "u");
		req->headingFlags1 = 0;
	}

	if( string2 != NULL ) {
		strcpy(req->headingString2, string2);
		req->headingFlags2 = flags2 | 1;
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
		req->lpItemFlags1[itemIdx] = flags1 | 1;
	} else {
		req->lpItemFlags1[itemIdx] = 0;
	}

	if( string2 != NULL ) {
		strcpy(&req->lpItemStrings2[itemIdx * req->itemStringLen], string2);
		req->lpItemFlags2[itemIdx] = flags2 | 1;
	} else {
		req->lpItemFlags2[itemIdx] = 0;
	}
}

void __cdecl AddRequesterItem(REQUEST_INFO *req, const char *string1, DWORD flags1, const char *string2, DWORD flags2) {
	req->lpItemFlags1 = RequesterItemFlags1;
	req->lpItemFlags2 = RequesterItemFlags2;

	if( string1 != NULL ) {
		strcpy(&req->lpItemStrings1[req->itemsCount * req->itemStringLen], string1);
		req->lpItemFlags1[req->itemsCount] = flags1 | 1;
	} else {
		RequesterItemFlags1[req->itemsCount] = 0;
	}

	if( string2 != NULL ) {
		strcpy(&req->lpItemStrings2[req->itemsCount * req->itemStringLen], string2);
		req->lpItemFlags2[req->itemsCount] = flags2 | 1;
	} else {
		req->lpItemFlags2[req->itemsCount] = 0;
	}

	++req->itemsCount;
}

void __cdecl SetPCRequesterSize(REQUEST_INFO *req, int maxLines, __int16 yPos) {
	req->yPos = yPos;
#ifdef FEATURE_FOV_FIX
	req->visibleCount = GetRenderHeightDownscaled() / 2 / REQ_LN_HEIGHT;
#else // !FEATURE_FOV_FIX
	req->visibleCount = GetRenderHeight() / 2 / REQ_LN_HEIGHT;
#endif // FEATURE_FOV_FIX
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

	return TRUE;
}

void __cdecl ShowGymStatsText() {
	static bool isStatsTextReady = false;
	int minutes, seconds, deciseconds;
	char statStr1[32];
	char statStr2[32];

	if( !isStatsTextReady ) {
		StatsRequester.reqFlags |= 1;
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
			AddRequesterItem(&StatsRequester, statStr1, 2, statStr2, 4);
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

//	INJECT(0x00426500, ShowStatsText);
//	INJECT(0x004268A0, ShowEndStatsText);
}
