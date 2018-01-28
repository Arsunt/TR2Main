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
	req->renderWidth  = GetRenderWidth();
	req->renderHeight = GetRenderHeight();
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

void __cdecl ResetTextParams(REQUEST_INFO *req, TEXT_STR_INFO *textInfo) {
	if( textInfo != NULL ) {
		textInfo->xPos = req->xPos;
		textInfo->bgndOffX = 0;
	}
}

void __cdecl GetTextParams1(REQUEST_INFO *req, TEXT_STR_INFO *textInfo) {
	DWORD scaleH;
	int bgndOffX;

	if( textInfo == NULL )
		return;

	scaleH = GetTextScaleH(textInfo->scaleH);
	bgndOffX = (req->pixWidth * scaleH / PHD_ONE) / 2 - T_GetTextWidth(textInfo) / 2 - (8 * scaleH / PHD_ONE);
	textInfo->xPos = req->xPos - bgndOffX;
	textInfo->bgndOffX = bgndOffX;
}

void __cdecl GetTextParams2(REQUEST_INFO *req, TEXT_STR_INFO *textInfo) {
	DWORD scaleH;
	int bgndOffX;

	if( textInfo == NULL )
		return;

	scaleH = GetTextScaleH(textInfo->scaleH);
	bgndOffX = (req->pixWidth * scaleH / PHD_ONE) / 2 - T_GetTextWidth(textInfo) / 2 - (8 * scaleH / PHD_ONE);
	textInfo->xPos = req->xPos + bgndOffX;
	textInfo->bgndOffX = -bgndOffX;
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
	req->visibleCount = GetRenderHeight() / 2 / 0x12;
	if( req->visibleCount > maxLines )
		req->visibleCount = maxLines;
}

/*
 * Inject function
 */
void Inject_InvText() {
	INJECT(0x00425580, Init_Requester);
	INJECT(0x00425610, Remove_Requester);
	INJECT(0x004256C0, ResetTextParams);
	INJECT(0x004256E0, GetTextParams1);
	INJECT(0x00425740, GetTextParams2);

//	INJECT(0x004257A0, Display_Requester);

	INJECT(0x00426010, SetRequesterHeading);
	INJECT(0x004260C0, RemoveAllReqItems);
	INJECT(0x004260E0, ChangeRequesterItem);
	INJECT(0x004261A0, AddRequesterItem);
	INJECT(0x00426250, SetPCRequesterSize);

//	INJECT(0x00426290, AddAssaultTime);
//	INJECT(0x00426320, ShowGymStatsText);
//	INJECT(0x00426500, ShowStatsText);
//	INJECT(0x004268A0, ShowEndStatsText);
}
