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
#include "game/text.h"
#include "3dsystem/scalespr.h"
#include "game/health.h"
#include "specific/frontend.h"
#include "specific/output.h"
#include "global/vars.h"

#ifdef FEATURE_HUD_IMPROVED
#include "modding/texture_utils.h"

extern DWORD InvTextBoxMode;
#endif // FEATURE_HUD_IMPROVED

#define IS_CHAR_SECRET(x)		((x) >= CHAR_SECRET1 && (x) <= CHAR_SECRET3)
#define IS_CHAR_LEGAL(x)		((x) <= 0x12 || ((x) >= 0x20 && (x) <= 0x7F) || IS_CHAR_SECRET(x))
#define IS_CHAR_DIACRITIC(x)	((x) == '(' || (x) == ')' || (x) == '$' || (x) == '~')

static const BYTE T_TextSpacing[0x6E] = {
//	A	B	C	D	E	F	G	H
	14, 11, 11, 11, 11, 11, 11, 13,
//	I	J	K	L	M	N	O	P
	 8, 11, 12, 11, 13, 13, 12, 11,
//	Q	R	S	T	U	V	W	X
	12, 12, 11, 12, 13, 13, 13, 12,
//	Y	Z	a	b	c	d	e	f
	12, 11,  9,  9,  9,  9,  9,  9,
//	g	h	i	j	k	l	m	n
	 9,  9,  5,  9,  9,  5, 12, 10,
//	o	p	q	r	s	t	u	v
	 9,  9,  9,  8,  9,  8,  9,  9,
//	w	x	y	z	0	1	2	3
	11,  9,  9,  9, 12,  8, 10, 10,
//	4	5	6	7	8	9	.	,
	10, 10, 10,  9, 10, 10,  5,  5,
//	!	?	[	di"	/	di^	di'	-
	 5, 11,  9, 10,  8,  6,  6,  7,
//	+	:	€	]	\	`	#	'
	 7,  3, 11,  8, 13, 16,  9,  4,
//	arU	arD	dg0	dg1	dg2	dg3	dg4	dg5
	12, 12,  7,  5,  7,  7,  7,  7,
//	dg6	dg7	dg8	dg9	js1	js2	js3	js4
	 7,  7,  7,  7, 16, 14, 14, 14,
//	js5	js6	js7	js8	js9	sc0	sc1	sc2
	16, 16, 16, 16, 16, 12, 14,  8,
//	sc3	sc4	dwn	up	lft	rht
	 8,  8,  8,  8,  8,  8,
};

static const BYTE T_RemapASCII[0x5F] = {
//	nop			!	"to [		#	$to di`	%to €	&to #		'
	0x00,	0x40,	0x42,	0x4E,	0x4D,	0x4A,	0x4E,	0x4F,
//	(to di^	)to di'	*to js1  	+		,		-		.		/
	0x45,	0x46,	0x5C,	0x48,	0x3F,	0x47,	0x3E,	0x44,
//		0		1		2		3		4		5		6		7
	0x34,	0x35,	0x36,	0x37,	0x38,	0x39,	0x3A,	0x3B,
//		8		9		:	;to :	<to [	=to €	>to	]		?
	0x3C,	0x3D,	0x49,	0x49,	0x42,	0x4A,	0x4B,	0x41,
//	@to	A		A		B		C		D		E		F		G
	0x00,	0x00,	0x01,	0x02,	0x03,	0x04,	0x05,	0x06,
//		H		I		J		K		L		M		N		O
	0x07,	0x08,	0x09,	0x0A,	0x0B,	0x0C,	0x0D,	0x0E,
//		P		Q		R		S		T		U		V		W
	0x0F,	0x10,	0x11,	0x12,	0x13,	0x14,	0x15,	0x16,
//		X		Y		Z	[to arU		\	]to	arD ^to js6	_to js7
	0x17,	0x18,	0x19,	0x50,	0x4C,	0x51,	0x61,	0x62,
//		`		a		b		c		d		e		f		g
	0x4D,	0x1A,	0x1B,	0x1C,	0x1D,	0x1E,	0x1F,	0x20,
//		h		i		j		k		l		m		n		o
	0x21,	0x22,	0x23,	0x24,	0x25,	0x26,	0x27,	0x28,
//		p		q		r		s		t		u		v		w
	0x29,	0x2A,	0x2B,	0x2C,	0x2D,	0x2E,	0x2F,	0x30,
//		x		y		z	{to js9	|to sc0	}to sc1	~to di"
	0x31,	0x32,	0x33,	0x64,	0x65,	0x66,	0x43,
};

void __cdecl T_InitPrint() {
	DisplayModeInfo(NULL);

	for( int i=0; i<64; ++i )
		TextInfoTable[i].flags = 0;

	TextStringCount = 0;
}

TEXT_STR_INFO *__cdecl T_Print(int x, int y, __int16 z, const char *str) {
	if( str == NULL || TextStringCount >= 64 )
		return NULL;

	for( int i=0; i<64; ++i ) {
		if( !CHK_ANY(TextInfoTable[i].flags, TIF_Active) ) {
			int stringLen = T_GetStringLen(str);
			CLAMPG(stringLen, 64); // NOTE: useless check, but decided to leave it here

			TextInfoTable[i].scaleH = PHD_ONE;
			TextInfoTable[i].scaleV = PHD_ONE;
			TextInfoTable[i].xPos = x * GetTextScaleH(PHD_ONE) / PHD_ONE;
			TextInfoTable[i].yPos = y * GetTextScaleV(PHD_ONE) / PHD_ONE;
			TextInfoTable[i].zPos = z;
			TextInfoTable[i].letterSpacing = 1;
			TextInfoTable[i].wordSpacing   = 6;

			TextInfoTable[i].textFlags = 0;
			TextInfoTable[i].outlFlags  = 0;
			TextInfoTable[i].bgndFlags = 0;
			TextInfoTable[i].bgndSizeX = 0;
			TextInfoTable[i].bgndSizeY = 0;
			TextInfoTable[i].bgndOffX  = 0;
			TextInfoTable[i].bgndOffY  = 0;
			TextInfoTable[i].bgndOffZ  = 0;

			TextInfoTable[i].flags = TIF_Active;
			TextInfoTable[i].pString = TheStrings[i].str;

			memcpy(TheStrings[i].str, str, stringLen);
			++TextStringCount;

			return &TextInfoTable[i];
		}
	}
	return NULL;
}

void __cdecl T_ChangeText(TEXT_STR_INFO *textInfo, const char *newString) {
	if( newString == NULL || textInfo == NULL || !CHK_ANY(textInfo->flags, TIF_Active) )
		return;

	// NOTE: the original code was unsafe crap. Reimplemented it a little safer
	strncpy(textInfo->pString, newString, 64);
	if( T_GetStringLen(newString) >= 64 )
		textInfo->pString[63] = 0;
}

void __cdecl T_SetScale(TEXT_STR_INFO *textInfo, int scaleH, int scaleV) {
	if( textInfo != NULL ) {
		textInfo->scaleH = scaleH;
		textInfo->scaleV = scaleV;
	}
}

void __cdecl T_FlashText(TEXT_STR_INFO *textInfo, __int16 state, __int16 rate) {
	if( textInfo == NULL)
		return;

	if( state == 0 ) {
		textInfo->flags &= ~TIF_Flash;
	} else {
		textInfo->flags |= TIF_Flash;
		textInfo->flashRate = rate;
		textInfo->flashCount = rate;
	}
}

void __cdecl T_AddBackground(TEXT_STR_INFO *textInfo, __int16 xSize, __int16 ySize, __int16 xOff, __int16 yOff, __int16 zOff, INV_COLOURS invColour, GOURAUD_FILL *gour, UINT16 flags) {
	DWORD scaleH, scaleV;

	if( textInfo == NULL )
		return;

	scaleH = GetTextScaleH(textInfo->scaleH);
	scaleV = GetTextScaleV(textInfo->scaleV);

	textInfo->flags |= TIF_Bgnd;
	textInfo->bgndSizeX = scaleH * xSize / PHD_ONE;
	textInfo->bgndSizeY = scaleV * ySize / PHD_ONE;
	textInfo->bgndOffX = scaleH * xOff / PHD_ONE;
	textInfo->bgndOffY = scaleV * yOff / PHD_ONE;
	textInfo->bgndOffZ = zOff;
	textInfo->bgndColor = invColour;
	textInfo->bgndGour = gour;
	textInfo->bgndFlags = flags;
}

void __cdecl T_RemoveBackground(TEXT_STR_INFO *textInfo) {
	if( textInfo != NULL )
		textInfo->flags &= ~TIF_Bgnd;
}

void __cdecl T_AddOutline(TEXT_STR_INFO *textInfo, BOOL state, INV_COLOURS invColour, GOURAUD_OUTLINE *gour, UINT16 flags) {
	if( textInfo != NULL ) {
		textInfo->flags |= TIF_Outline;
		textInfo->outlColour = invColour;
		textInfo->outlGour = gour;
		textInfo->outlFlags = flags;
	}
}

void __cdecl T_RemoveOutline(TEXT_STR_INFO *textInfo) {
	if( textInfo != NULL )
		textInfo->flags &= ~TIF_Outline;
}

void __cdecl T_CentreH(TEXT_STR_INFO *textInfo, UINT16 state) {
	if( textInfo != NULL ) {
		if( state )
			textInfo->flags |= TIF_CentreH;
		else
			textInfo->flags &= ~TIF_CentreH;
	}
}

void __cdecl T_CentreV(TEXT_STR_INFO *textInfo, UINT16 state) {
	if( textInfo != NULL ) {
		if( state )
			textInfo->flags |= TIF_CentreV;
		else
			textInfo->flags &= ~TIF_CentreV;
	}
}

void __cdecl T_RightAlign(TEXT_STR_INFO *textInfo, bool state) {
	if( textInfo == NULL)
		return;

	if( state ) {
		textInfo->flags |= TIF_Right;
	} else {
		textInfo->flags &= ~TIF_Right;
	}
}

void __cdecl T_BottomAlign(TEXT_STR_INFO *textInfo, bool state) {
	if( textInfo == NULL)
		return;

	if( state ) {
		textInfo->flags |= TIF_Bottom;
	} else {
		textInfo->flags &= ~TIF_Bottom;
	}
}

void __cdecl T_DrawTextBox(int sx, int sy, int z, int width, int height) {
	int x0, y0, x1, y1, offset;
	int scaleH, scaleV;
	int meshIdx = Objects[ID_TEXT_BOX].meshIndex;

#ifdef FEATURE_HUD_IMPROVED
	offset = GetRenderScale(4);
	scaleH = GetRenderScale(PHD_ONE);
	scaleV = GetRenderScale(PHD_ONE);
#else // !FEATURE_HUD_IMPROVED
	offset = 4;
	scaleH = PHD_ONE;
	scaleV = PHD_ONE;
#endif // FEATURE_HUD_IMPROVED

	x0 = sx + offset;
	y0 = sy + offset;
	x1 = sx - offset + width;
	y1 = sy - offset + height;

	width = PHD_ONE * (width  - offset * 2) / 8;
	height = PHD_ONE * (height - offset * 2) / 8;

	S_DrawScreenSprite2d(x0, y0, z, scaleH, scaleV, (meshIdx + 0), 0x1000, 0);
	S_DrawScreenSprite2d(x1, y0, z, scaleH, scaleV, (meshIdx + 1), 0x1000, 0);
	S_DrawScreenSprite2d(x1, y1, z, scaleH, scaleV, (meshIdx + 2), 0x1000, 0);
	S_DrawScreenSprite2d(x0, y1, z, scaleH, scaleV, (meshIdx + 3), 0x1000, 0);

	S_DrawScreenSprite2d(x0, y0, z, width,  scaleV, (meshIdx + 4), 0x1000, 0);
	S_DrawScreenSprite2d(x1, y0, z, scaleH, height, (meshIdx + 5), 0x1000, 0);
	S_DrawScreenSprite2d(x0, y1, z, width,  scaleV, (meshIdx + 6), 0x1000, 0);
	S_DrawScreenSprite2d(x0, y0, z, scaleH, height, (meshIdx + 7), 0x1000, 0);
}

DWORD __cdecl T_GetTextWidth(TEXT_STR_INFO *textInfo) {
	int spacing;
	DWORD width, scaleH, sprite;

	width = 0;
	scaleH = GetTextScaleH(textInfo->scaleH);

	for( BYTE *str = (BYTE *)textInfo->pString; *str != 0; str++ ) {
		if( !IS_CHAR_LEGAL(*str) || IS_CHAR_DIACRITIC(*str) ) {
			continue; // if char code is illegal or not required for width measuring, go to next char
		}

		if( *str == 0x20 ) { // Check if char is "Space"
			// "Space" uses wordSpacing value instead of sprite width
			spacing = textInfo->wordSpacing;
		}
		else if( IS_CHAR_SECRET(*str) ) { // Check if "Secret" sprite
			// "Secret" sprites have spacing=16
			spacing = 16;
#ifdef FEATURE_HUD_IMPROVED
		} else if( *str == 0x7F ) { // Check if it's the opening code of the named sprite sequence
			BYTE *ptr = (BYTE *)strchr((const char *)str+1, 0x1F);
			if( ptr == NULL ) break; // Closing code is not found, break now!
			if( !GetTextSpriteByName((const char *)str+1, ptr-str-1, &sprite, &spacing) ) {
				spacing = 0;
			}
			str = ptr; // move pointer to the sequence end
#endif // FEATURE_HUD_IMPROVED
		} else {
			if( *str < 0x0B ) { // Check if "Digit" sprite
				sprite = *str + 0x51; // We have (*str >= 0x01) here. "Digit" sprite codes start from (0x52 = 0x01 + 0x51)
			}
			else if( *str <= 0x12 ) { // Check if "Special" sprite. NOTE: original code was (*str < 0x10) but this was wrong
				// Check if normal *str or "Special" sprite
				sprite = *str + 0x5B;  // We have (*str >= 0x0B) here. "Special" sprite codes start from (0x66 = 0x0B + 0x5B)
			} else { // here (*str > 0x20)
				sprite = T_RemapASCII[*str - 0x20]; // For normal letters we have sprite code table
			}

			// Check if normal letter sprite has digit representation
			if( *str >= '0' && *str <= '9' ) { // NOTE: original code was (sprite >= '0' && sprite <= '9') but this was wrong
				// Normal letter sprites with digits have spacing=12
				spacing = 12;
			} else {
				// For "Digit", "Special" and normal letter sprites we use spacing table + letterSpacing
				spacing = T_TextSpacing[sprite];
				// NOTE: this condition was added instead of returned value recalculation (see below).
				// In the original code spacing addition was unconditional
				if( str[1] != 0 ) { // If this letter is not last, add letterSpacing
					spacing += textInfo->letterSpacing;
				}
			}
		}
		width += spacing * scaleH / PHD_ONE;
	}
	// Align width (minus last letterSpacing) to lesser even value
	// NOTE: original code was ((width - textInfo->letterSpacing) & ~1) but this was wrong, because letterSpacing is not scaled
	// And also we calculate width of any string, there may not be letterSpacing at all (i.e. digit letter sprites )
	return width & ~1;
}

BOOL __cdecl T_RemovePrint(TEXT_STR_INFO *textInfo) {
	if( textInfo == NULL || !CHK_ANY(textInfo->flags, TIF_Active) )
		return false;

	textInfo->flags &= ~TIF_Active;
	--TextStringCount;
	return true;
}

__int16 __cdecl T_GetStringLen(const char *str) {
	// Calculates string length up to 64 chars including null terminator
	for( int i=0; i<64; ++i ) {
		if( str[i] == 0 )
			return i+1;
	}
	return 64;
}

void __cdecl T_DrawText() {
	for( int i=0; i<64; ++i ) {
		if( CHK_ANY(TextInfoTable[i].flags, TIF_Active) )
			T_DrawThisText(&TextInfoTable[i]);
	}
}

void __cdecl T_DrawThisText(TEXT_STR_INFO *textInfo) {
	int x, y, z, xOff;
	int boxX, boxY, boxZ, boxW, boxH;
	DWORD textWidth, scaleH, scaleV, sprite;

	scaleH = GetTextScaleH(textInfo->scaleH);
	scaleV = GetTextScaleV(textInfo->scaleV);

#ifdef FEATURE_HUD_IMPROVED
	if( CHK_ANY(textInfo->flags, TIF_Hide) ) {
		return;
	}
#endif // FEATURE_HUD_IMPROVED

	// Do text flashing if required
	if( CHK_ANY(textInfo->flags, TIF_Flash) ) {
		textInfo->flashCount -= (__int16)Camera.numberFrames;

		if( textInfo->flashCount <= -textInfo->flashRate ) {
			textInfo->flashCount = textInfo->flashRate;
		}
		else if ( textInfo->flashCount < 0 ) {
			return;
		}
	}

	x = textInfo->xPos;
	y = textInfo->yPos;
	z = textInfo->zPos;
	textWidth = T_GetTextWidth(textInfo);

	// Horizontal alignment
	if( CHK_ANY(textInfo->flags, TIF_CentreH) ) {
		x += (GetRenderWidth() - textWidth) / 2;
	}
	else if( CHK_ANY(textInfo->flags, TIF_Right) ) {
		x += GetRenderWidth() - textWidth;
	}

	// Vertical alignment
	if( CHK_ANY(textInfo->flags, TIF_CentreV) ) {
		y += GetRenderHeight() / 2;
	}
	else if ( CHK_ANY(textInfo->flags, TIF_Bottom) ) {
		y += GetRenderHeight();
	}

	boxX = x + textInfo->bgndOffX - (2 * scaleH / PHD_ONE);
	boxY = y + textInfo->bgndOffY - (4 * scaleV / PHD_ONE) - (11 * scaleV / PHD_ONE);
	boxZ = z + textInfo->bgndOffZ + 2;

	for( BYTE *str = (BYTE *)textInfo->pString; *str != 0; str++ ) {
		// Check if char code is in illegal range
		if( !IS_CHAR_LEGAL(*str) )
			continue;

		if( *str == 0x20 ) { // Check if char is "Space"
			// "Space" uses wordSpacing value instead of sprite width
			x += textInfo->wordSpacing * scaleH / PHD_ONE;
		}
		else if( IS_CHAR_SECRET(*str) ) { // Check if "Secret" sprite
			// Draw "Secret" sprite
			S_DrawPickup(x + 10, y, 0x1BE8, Objects[ID_SECRET1 + (*str - CHAR_SECRET1)].meshIndex, 0x1000);
			// "Secret" sprites have spacing=16
			x += 16 * scaleH / PHD_ONE;
#ifdef FEATURE_HUD_IMPROVED
		} else if( *str == 0x7F ) { // Check if it's the opening code of the named sprite sequence
			BYTE *ptr = (BYTE *)strchr((const char *)str+1, 0x1F);
			if( ptr == NULL ) break; // Closing code is not found, break now!
			if( GetTextSpriteByName((const char *)str+1, ptr-str-1, &sprite, &xOff) ) {
				if( x > 0 && x < GetRenderWidth() && y > 0 && y < GetRenderHeight() ) {
					S_DrawScreenSprite2d(x, y, z, scaleH, scaleV, sprite, 0x1000, textInfo->textFlags);
				}
				xOff += textInfo->letterSpacing;
				x += xOff * scaleH / PHD_ONE;
			}
			str = ptr; // move pointer to the sequence end
#endif // FEATURE_HUD_IMPROVED
		} else {
			if( *str < 0x0B ) { // Check if "Digit" sprite
				sprite = *str + 0x51; // We have (*str >= 0x01) here. "Digit" sprite codes start from (0x52 = 0x01 + 0x51)
			}
			else if( *str <= 0x12 ) { // Check if "Special" sprite. NOTE: original code was (*str < 0x10) but this was wrong
				// Check if normal *str or "Special" sprite
				sprite = *str + 0x5B;  // We have (*str >= 0x0B) here. "Special" sprite codes start from (0x66 = 0x0B + 0x5B)
			} else { // here (*str > 0x20)
				sprite = T_RemapASCII[*str - 0x20]; // For normal letters we have sprite code table
			}

			// Check if normal letter sprite is digit representation
			// Normal letter sprites with digits have spacing=12
			// But sprite itself is center aligned in this space
			if( *str >= '0' && *str <= '9' ) {
				// !!! Here we do LEFT spacing part for digit letters !!!
				xOff = (12 - T_TextSpacing[sprite]) / 2;
				x += xOff * scaleH / PHD_ONE;
			}

			// Draw letter sprite
			if( x > 0 && x < GetRenderWidth() && y > 0 && y < GetRenderHeight() ) {
				S_DrawScreenSprite2d(x, y, z, scaleH, scaleV, (Objects[ID_ALPHABET].meshIndex + sprite), 0x1000, textInfo->textFlags);
			}

			// Check if letter is diacritic
			// Diacritics are drawn right on the next letter sprite, so there is no spacing for them
			if( IS_CHAR_DIACRITIC(*str) )
				continue;

			// Check if normal letter sprite is digit representation
			if( *str >= '0' && *str <= '9' ) {
				// !!! Here we do RIGHT spacing part for digit letters !!!
				xOff = (12 - T_TextSpacing[sprite]) / 2;
				x += (12 - xOff) * scaleH / PHD_ONE;
			} else {
				// For "Digit", "Special" and normal letter sprites we use spacing table + letterSpacing
				xOff = T_TextSpacing[sprite];
				xOff += textInfo->letterSpacing;
				x += xOff * scaleH / PHD_ONE;
			}
		}
	}

	// Draw background/outline if required
	if( CHK_ANY(textInfo->flags, TIF_Bgnd|TIF_Outline) ) {
		if( textInfo->bgndSizeX != 0 ) {
			boxX += textWidth / 2 - textInfo->bgndSizeX / 2;
			boxW = textInfo->bgndSizeX + 4;
		} else {
			boxW = textWidth + 4;
		}

		if( textInfo->bgndSizeY != 0 ) {
			boxH = textInfo->bgndSizeY;
		} else {
#ifdef FEATURE_HUD_IMPROVED
			boxH = (InvTextBoxMode ? 14 : 16) * scaleV / PHD_ONE;
#else // !FEATURE_HUD_IMPROVED
			boxH = 16 * scaleV / PHD_ONE;
#endif // FEATURE_HUD_IMPROVED
		}

		// Draw background
		if( CHK_ANY(textInfo->flags, TIF_Bgnd) ) {
			S_DrawScreenFBox(boxX, boxY, boxZ, boxW, boxH, textInfo->bgndColor, textInfo->bgndGour, textInfo->bgndFlags);
		}

		// Draw outline
		if( CHK_ANY(textInfo->flags, TIF_Outline) ) {
#ifdef FEATURE_HUD_IMPROVED
			if( SavedAppSettings.RenderMode == RM_Hardware && InvTextBoxMode ) {
				S_DrawScreenBox(boxX, boxY, boxZ, boxW, boxH, textInfo->outlColour, textInfo->outlGour, textInfo->outlFlags);
			} else {
				T_DrawTextBox(boxX, boxY, z, boxW, boxH);
			}
#else // !FEATURE_HUD_IMPROVED
			T_DrawTextBox(boxX, boxY, z, boxW, boxH);
#endif // FEATURE_HUD_IMPROVED
		}
	}
}

DWORD __cdecl GetTextScaleH(DWORD baseScale) {
#ifdef FEATURE_HUD_IMPROVED
	return GetRenderScale(baseScale);
#else // !FEATURE_HUD_IMPROVED
	DWORD renderWidth, renderScale;

	renderWidth = GetRenderWidth();
	CLAMPL(renderWidth, 640)

	renderScale = renderWidth * PHD_ONE / 640;
	return (baseScale / PHD_HALF) * (renderScale / PHD_HALF);
#endif // FEATURE_HUD_IMPROVED
}

DWORD __cdecl GetTextScaleV(DWORD baseScale) {
#ifdef FEATURE_HUD_IMPROVED
	return GetRenderScale(baseScale);
#else // !FEATURE_HUD_IMPROVED
	DWORD renderHeight, renderScale;

	renderHeight = GetRenderHeight();
	CLAMPL(renderHeight, 480)

	renderScale = renderHeight * PHD_ONE / 480;
	return (baseScale / PHD_HALF) * (renderScale / PHD_HALF);
#endif // FEATURE_HUD_IMPROVED
}

#ifdef FEATURE_HUD_IMPROVED
void T_HideText(TEXT_STR_INFO *textInfo, __int16 state) {
	if( textInfo == NULL)
		return;

	if( state == 0 ) {
		textInfo->flags &= ~TIF_Hide;
	} else {
		textInfo->flags |= TIF_Hide;
	}
}
#endif // FEATURE_HUD_IMPROVED

/*
 * Inject function
 */
void Inject_Text() {
	INJECT(0x00440500, T_InitPrint);
	INJECT(0x00440530, T_Print);
	INJECT(0x00440640, T_ChangeText);
	INJECT(0x00440680, T_SetScale);
	INJECT(0x004406A0, T_FlashText);
	INJECT(0x004406D0, T_AddBackground);
	INJECT(0x00440760, T_RemoveBackground);
	INJECT(0x00440770, T_AddOutline);
	INJECT(0x004407A0, T_RemoveOutline);
	INJECT(0x004407B0, T_CentreH);
	INJECT(0x004407D0, T_CentreV);
	INJECT(0x004407F0, T_RightAlign);
	INJECT(0x00440810, T_BottomAlign);
	INJECT(0x00440830, T_GetTextWidth);
	INJECT(0x00440940, T_RemovePrint);
	INJECT(0x00440970, T_GetStringLen);
	INJECT(0x004409A0, T_DrawText);
	INJECT(0x004409D0, T_DrawTextBox);
	INJECT(0x00440B60, T_DrawThisText);
	INJECT(0x00440F40, GetTextScaleH);
	INJECT(0x00440F80, GetTextScaleV);
}
