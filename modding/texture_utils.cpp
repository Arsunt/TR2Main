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
#include "modding/texture_utils.h"
#include "specific/output.h"
#include "specific/texture.h"
#include "specific/utils.h"
#include "modding/file_utils.h"
#include "modding/json_utils.h"
#include "global/vars.h"

#ifdef FEATURE_HUD_IMPROVED
#define BTN_SPR_IDX (ARRAY_SIZE(PhdSpriteInfo) - 256)

int JoyHintMode = 0;

typedef enum {
	kbd_esc,
	kbd_tab,
	kbd_1,
	kbd_2,
	kbd_3,
	kbd_4,
	kbd_5,
	kbd_6,
	kbd_7,
	kbd_8,
	kbd_9,
	kbd_0,
	kbd_minus,
	kbd_equals,
	kbd_backspace,
	kbd_capslock,
	kbd_q,
	kbd_w,
	kbd_e,
	kbd_r,
	kbd_t,
	kbd_y,
	kbd_u,
	kbd_i,
	kbd_o,
	kbd_p,
	kbd_lbracket,
	kbd_rbracket,
	kbd_return,
	kbd_ctrl,
	kbd_a,
	kbd_s,
	kbd_d,
	kbd_f,
	kbd_g,
	kbd_h,
	kbd_j,
	kbd_k,
	kbd_l,
	kbd_semicolon,
	kbd_apostrophe,
	kbd_grave,
	kbd_shift,
	kbd_alt,
	kbd_backslash,
	kbd_z,
	kbd_x,
	kbd_c,
	kbd_v,
	kbd_b,
	kbd_n,
	kbd_m,
	kbd_comma,
	kbd_period,
	kbd_slash,
	kbd_oem102,
	kbd_space,
	kbd_insert,
	kbd_home,
	kbd_pageup,
	kbd_delete,
	kbd_end,
	kbd_pagedown,
	kbd_left,
	kbd_right,
	kbd_up,
	kbd_down,
	kbd_numlock,
	kbd_pad7,
	kbd_pad8,
	kbd_pad9,
	kbd_divide,
	kbd_pad4,
	kbd_pad5,
	kbd_pad6,
	kbd_multiply,
	kbd_pad1,
	kbd_pad2,
	kbd_pad3,
	kbd_subtract,
	kbd_pad0,
	kbd_decimal,
	kbd_enter,
	kbd_add,

	joy_di1,  joy_di2,  joy_di3,  joy_di4,  joy_di5,  joy_di6,  joy_di7,  joy_di8,
	joy_di9,  joy_di10, joy_di11, joy_di12, joy_di13, joy_di14, joy_di15, joy_di16,

	joy_ps1,  joy_ps2,  joy_ps3,  joy_ps4,  joy_ps5,  joy_ps6,  joy_ps7,  joy_ps8,
	joy_ps9,  joy_ps10, joy_ps11, joy_ps12, joy_ps13, joy_ps14, joy_ps15, joy_ps16,

	joy_xi1,  joy_xi2,  joy_xi3,  joy_xi4,  joy_xi5,  joy_xi6,  joy_xi7,  joy_xi8,
	joy_xi9,  joy_xi10, joy_xi11, joy_xi12, joy_xi13, joy_xi14, joy_xi15, joy_xi16,

	button_sprites_number,
} SPR_BUTTONS;

typedef struct {
	int id;
	const char *name;
} MAP;

static MAP btnMap[] = {
	{kbd_esc, "esc"},
	{kbd_tab, "tab"},
	{kbd_capslock, "capslock"},
	{kbd_shift, "shift"},
	{kbd_ctrl, "ctrl"},
	{kbd_alt, "alt"},
	{kbd_space, "space"},
	{kbd_backspace, "backspace"},
	{kbd_return, "return"},
	{kbd_1, "1"},
	{kbd_2, "2"},
	{kbd_3, "3"},
	{kbd_4, "4"},
	{kbd_5, "5"},
	{kbd_6, "6"},
	{kbd_7, "7"},
	{kbd_8, "8"},
	{kbd_9, "9"},
	{kbd_0, "0"},
	{kbd_minus, "-"},
	{kbd_equals, "="},
	{kbd_q, "q"},
	{kbd_w, "w"},
	{kbd_e, "e"},
	{kbd_r, "r"},
	{kbd_t, "t"},
	{kbd_y, "y"},
	{kbd_u, "u"},
	{kbd_i, "i"},
	{kbd_o, "o"},
	{kbd_p, "p"},
	{kbd_lbracket, "["},
	{kbd_rbracket, "]"},
	{kbd_a, "a"},
	{kbd_s, "s"},
	{kbd_d, "d"},
	{kbd_f, "f"},
	{kbd_g, "g"},
	{kbd_h, "h"},
	{kbd_j, "j"},
	{kbd_k, "k"},
	{kbd_l, "l"},
	{kbd_semicolon, ";"},
	{kbd_apostrophe, "'"},
	{kbd_grave, "`"},
	{kbd_backslash, "\\"},
	{kbd_z, "z"},
	{kbd_x, "x"},
	{kbd_c, "c"},
	{kbd_v, "v"},
	{kbd_b, "b"},
	{kbd_n, "n"},
	{kbd_m, "m"},
	{kbd_comma, ","},
	{kbd_period, "."},
	{kbd_slash, "/"},
	{kbd_oem102, "<"},
	{kbd_insert, "insert"},
	{kbd_home, "home"},
	{kbd_pageup, "pageup"},
	{kbd_delete, "delete"},
	{kbd_end, "end"},
	{kbd_pagedown, "pagedown"},
	{kbd_left, "left"},
	{kbd_right, "right"},
	{kbd_up, "up"},
	{kbd_down, "down"},
	{kbd_numlock, "numlock"},
	{kbd_pad7, "pad7"},
	{kbd_pad8, "pad8"},
	{kbd_pad9, "pad9"},
	{kbd_divide, "pad/"},
	{kbd_pad4, "pad4"},
	{kbd_pad5, "pad5"},
	{kbd_pad6, "pad6"},
	{kbd_multiply, "pad*"},
	{kbd_pad1, "pad1"},
	{kbd_pad2, "pad2"},
	{kbd_pad3, "pad3"},
	{kbd_subtract, "pad-"},
	{kbd_pad0, "pad0"},
	{kbd_decimal, "pad."},
	{kbd_enter, "enter"},
	{kbd_add, "pad+"},

	{joy_di1,  "di1"},  {joy_di2,  "di2"},  {joy_di3,  "di3"},  {joy_di4,  "di4"},
	{joy_di5,  "di5"},  {joy_di6,  "di6"},  {joy_di7,  "di7"},  {joy_di8,  "di8"},
	{joy_di9,  "di9"},  {joy_di10, "di10"}, {joy_di11, "di11"}, {joy_di12, "di12"},
	{joy_di13, "di13"}, {joy_di14, "di14"}, {joy_di15, "di15"}, {joy_di16, "di16"},

	{joy_ps1,  "ps1"},  {joy_ps2,  "ps2"},  {joy_ps3,  "ps3"},  {joy_ps4,  "ps4"},
	{joy_ps5,  "ps5"},  {joy_ps6,  "ps6"},  {joy_ps7,  "ps7"},  {joy_ps8,  "ps8"},
	{joy_ps9,  "ps9"},  {joy_ps10, "ps10"}, {joy_ps11, "ps11"}, {joy_ps12, "ps12"},
	{joy_ps13, "ps13"}, {joy_ps14, "ps14"}, {joy_ps15, "ps15"}, {joy_ps16, "ps16"},

	{joy_xi1,  "xi1"},  {joy_xi2,  "xi2"},  {joy_xi3,  "xi3"},  {joy_xi4,  "xi4"},
	{joy_xi5,  "xi5"},  {joy_xi6,  "xi6"},  {joy_xi7,  "xi7"},  {joy_xi8,  "xi8"},
	{joy_xi9,  "xi9"},  {joy_xi10, "xi10"}, {joy_xi11, "xi11"}, {joy_xi12, "xi12"},
	{joy_xi13, "xi13"}, {joy_xi14, "xi14"}, {joy_xi15, "xi15"}, {joy_xi16, "xi16"},
};

static MAP joyMapDI[] = {
	{joy_di1,  "joy1"},  {joy_di2,  "joy2"},  {joy_di3,  "joy3"},  {joy_di4,  "joy4"},
	{joy_di5,  "joy5"},  {joy_di6,  "joy6"},  {joy_di7,  "joy7"},  {joy_di8,  "joy8"},
	{joy_di9,  "joy9"},  {joy_di10, "joy10"}, {joy_di11, "joy11"}, {joy_di12, "joy12"},
	{joy_di13, "joy13"}, {joy_di14, "joy14"}, {joy_di15, "joy15"}, {joy_di16, "joy16"},
};

static MAP joyMapPS[] = {
	{joy_ps1,  "joy1"},  {joy_ps2,  "joy2"},  {joy_ps3,  "joy3"},  {joy_ps4,  "joy4"},
	{joy_ps5,  "joy5"},  {joy_ps6,  "joy6"},  {joy_ps7,  "joy7"},  {joy_ps8,  "joy8"},
	{joy_ps9,  "joy9"},  {joy_ps10, "joy10"}, {joy_ps11, "joy11"}, {joy_ps12, "joy12"},
	{joy_ps13, "joy13"}, {joy_ps14, "joy14"}, {joy_ps15, "joy15"}, {joy_ps16, "joy16"},
};

static MAP joyMapXI[] = {
	{joy_xi1,  "joy1"},  {joy_xi2,  "joy2"},  {joy_xi3,  "joy3"},  {joy_xi4,  "joy4"},
	{joy_xi5,  "joy5"},  {joy_xi6,  "joy6"},  {joy_xi7,  "joy7"},  {joy_xi8,  "joy8"},
	{joy_xi9,  "joy9"},  {joy_xi10, "joy10"}, {joy_xi11, "joy11"}, {joy_xi12, "joy12"},
	{joy_xi13, "joy13"}, {joy_xi14, "joy14"}, {joy_xi15, "joy15"}, {joy_xi16, "joy16"},
};

static bool ButtonSpriteLoaded = false;
static BYTE ButtonSpriteSpacing[256] = {0};
static BYTE ButtonSpriteSWR[256*256] = {0};

static int compareMap(const void *a, const void *b) {
	return strcmp(((const MAP *)a)->name, ((const MAP *)b)->name);
}

static void sortMaps() {
	static bool once = false;
	if( !once ) {
		qsort(btnMap,    ARRAY_SIZE(btnMap),  sizeof(MAP), compareMap);
		qsort(joyMapDI, ARRAY_SIZE(joyMapDI), sizeof(MAP), compareMap);
		qsort(joyMapPS, ARRAY_SIZE(joyMapPS), sizeof(MAP), compareMap);
		qsort(joyMapXI, ARRAY_SIZE(joyMapXI), sizeof(MAP), compareMap);
		once = true;
	}
}

static int searchMap(const char *name, MAP *mapArray, DWORD mapCount) {
	if( !name || !*name ) return -1;
	MAP keyItem = {0, name};
	MAP *foundItem = (MAP *)bsearch(&keyItem, mapArray, mapCount, sizeof(MAP), compareMap);
	return foundItem ? foundItem->id : -1;
}

static bool ParseSpriteInfo(json_value *root, int id) {
	if( root == NULL || root->type != json_object || id < 0 || id >= button_sprites_number ) {
		return false;
	}
	PHD_SPRITE *info = &PhdSpriteInfo[BTN_SPR_IDX + id];

	BYTE u = GetJsonIntegerFieldValue(root, "u");
	BYTE v = GetJsonIntegerFieldValue(root, "v");
	int x = GetJsonIntegerFieldValue(root, "x");
	int y = GetJsonIntegerFieldValue(root, "y");
	int width = GetJsonIntegerFieldValue(root, "width");
	int height = GetJsonIntegerFieldValue(root, "height");
	ButtonSpriteSpacing[id] = GetJsonIntegerFieldValue(root, "spacing");;

	info->offset = (v << 8) | u;
	info->width = ABS(width)*256;
	info->height = ABS(height)*256;
	info->x1 = x;
	info->y1 = y;
	info->x2 = x + width;
	info->y2 = y + height;

	return true;
}

static bool ParseButtonSprites(json_value *root) {
	if( root == NULL || root->type != json_object ) {
		return false;
	}
	sortMaps();
	for( DWORD i = 0; i < root->u.object.length; ++i ) {
		if( root->u.object.values[i].value->type != json_object ) continue;
		int id = searchMap(root->u.object.values[i].name, btnMap, ARRAY_SIZE(btnMap));
		ParseSpriteInfo(root->u.object.values[i].value, id);
	}
	return true;
}

static void AdaptToPalette(void *srcData, int width, int height, int srcPitch, RGB888 *srcPalette, void *dstData, int dstPitch, RGB888 *dstPalette) {
	int i, j;
	BYTE *src, *dst;
	BYTE bufPalette[256] = {0};

	// skip index 0 as it is reserved as semitransparent
	for( i=1; i<256; ++i ) {
		bufPalette[i] = FindNearestPaletteEntry(dstPalette, srcPalette[i].red, srcPalette[i].green, srcPalette[i].blue, false);
	}

	src = (BYTE *)srcData;
	dst = (BYTE *)dstData;
	for( i=0; i<height; ++i ) {
		for( j=0; j<width; ++j ) {
			*(dst++) = bufPalette[*(src++)];
		}
		src += srcPitch - width;
		dst += dstPitch - width;
	}
}

static int LoadButtonSpriteTexturePage() {
	int pageIndex = -1;
	DWORD width, height, pcxSize = 0;

	LPCBYTE pcxData = (LPCBYTE)GetResourceData("BUTTONS.PCX", &pcxSize);
	if( !pcxData || !pcxSize || GetPcxResolution(pcxData, pcxSize, &width, &height)
		|| width != height || (SavedAppSettings.RenderMode != RM_Hardware && width != 256) )
	{
		return -1;
	}

	RGB888 bmpPal[256] = {{0,0,0}};
	BYTE *bitmap = (BYTE *)malloc(width * height);
	if( bitmap != NULL ) {
		if( DecompPCX(pcxData, pcxSize, bitmap, bmpPal) ) {
			if( SavedAppSettings.RenderMode != RM_Hardware || TextureFormat.bpp < 16 ) {
				AdaptToPalette(bitmap, width, height, width, bmpPal, bitmap, width, GamePalette8);
				memcpy(bmpPal, GamePalette8, sizeof(bmpPal));
			}
			pageIndex = MakeCustomTexture(0, 0, width, height, width, width, bitmap, bmpPal, PaletteIndex, ButtonSpriteSWR, true);
			if( pageIndex >= 0 && SavedAppSettings.RenderMode == RM_Hardware ) {
				HWR_TexturePageIndexes[HwrTexturePagesCount] = pageIndex;
				HWR_PageHandles[HwrTexturePagesCount] = GetTexturePageHandle(pageIndex);
				pageIndex = HwrTexturePagesCount++;
			}
		}
		free(bitmap);
	}

	return pageIndex;
}

bool LoadButtonSprites() {
	ButtonSpriteLoaded = false;
	memset(&PhdSpriteInfo[BTN_SPR_IDX], 0, sizeof(PHD_SPRITE) * button_sprites_number);
	int pageIndex = LoadButtonSpriteTexturePage();
	if( pageIndex >= 0 ) {
		for( int i = 0; i < button_sprites_number; ++i ) {
			PhdSpriteInfo[BTN_SPR_IDX + i].texPage = pageIndex;
		}
		DWORD jsonSize = 0;
		LPCVOID jsonData = GetResourceData("BUTTONS.JSON", &jsonSize);
		if( jsonData && jsonSize ) {
			json_value* json = json_parse((const json_char *)jsonData, jsonSize);
			if( json != NULL ) {
				ButtonSpriteLoaded = ParseButtonSprites(json);
				json_value_free(json);
			}
		}
	}
	return ButtonSpriteLoaded;
}

bool GetTextSpriteByName(const char *name, int nameLen, DWORD *sprite, int *spacing) {
	int id = -1;
	char mapName[64] = {0};
	if( !ButtonSpriteLoaded || name == NULL || nameLen <= 0 || (DWORD)nameLen >= sizeof(mapName) ) {
		return false;
	}
	memcpy(mapName, name, nameLen);
	switch( JoyHintMode ) {
		case 0:
			id = searchMap(mapName, joyMapDI, ARRAY_SIZE(joyMapDI));
			break;
		case 1:
			id = searchMap(mapName, joyMapPS, ARRAY_SIZE(joyMapPS));
			break;
		case 2:
			id = searchMap(mapName, joyMapXI, ARRAY_SIZE(joyMapXI));
			break;
	}
	if( id < 0 || id >= button_sprites_number ) {
		id = searchMap(mapName, btnMap, ARRAY_SIZE(btnMap));
	}
	if( id < 0 || id >= button_sprites_number ) {
		return false;
	}
	if( sprite ) *sprite = BTN_SPR_IDX + id;
	if( spacing ) *spacing = ButtonSpriteSpacing[id];
	return true;
}
#endif // FEATURE_HUD_IMPROVED

// This prevents texture bleeding instead of UV adjustment
static int FillEdgePadding(DWORD width, DWORD height, DWORD side, BYTE *bitmap, DWORD bpp) {
	if( !width || !height || width > side || height > side || bitmap == NULL ) {
		return -1;
	}
	switch( bpp ) {
		case  8 :
		case 16 :
		case 32 :
			break;
		default :
			return -1;
	}

	DWORD i;
	DWORD padRight = side - width;
	DWORD padBottom = side - height;

	if( padRight > 0 ) {
		switch( bpp ) {
			case  8 : {
				BYTE *p = (BYTE *)bitmap;
				for( i = 0; i < height ; ++i ) {
					p += width;
					p[0] = p[-1];
					p += padRight;
				}
				break;
			}
			case 16 : {
				UINT16 *p = (UINT16 *)bitmap;
				for( i = 0; i < height ; ++i ) {
					p += width;
					p[0] = p[-1];
					p += padRight;
				}
				break;
			}
			case 32 : {
				DWORD *p = (DWORD *)bitmap;
				for( i = 0; i < height ; ++i ) {
					p += width;
					p[0] = p[-1];
					p += padRight;
				}
				break;
			}
			default :
				break;
		}
	}

	if( padBottom > 0 ) {
		DWORD pitch = (width + padRight?1:0) * (bpp/8);
		BYTE *p = bitmap + height * pitch;
		memcpy(p, p - pitch, pitch);
		p += pitch;
	}

	return 0;
}

int MakeCustomTexture(DWORD x, DWORD y, DWORD width, DWORD height, DWORD pitch, DWORD side, BYTE *bitmap, RGB888 *bmpPal, int hwrPal, BYTE *swrBuf, bool keyColor) {
	int pageIndex = -1;
	if( bmpPal == NULL ) { // source bitmap is not indexed
		if( SavedAppSettings.RenderMode != RM_Hardware || TextureFormat.bpp < 16 ) { // texture cannot be indexed in this case
			return -1;
		}
		UINT16 *tmpBmp = (UINT16 *)calloc(2, SQR(side));
		UINT16 *bmpDst = tmpBmp;
		UINT16 *bmpSrc = (UINT16 *)bitmap + x + y * pitch;

		for( DWORD j = 0; j < height; ++j ) {
			for( DWORD i = 0; i < width; ++i ) {
				bmpDst[i] = bmpSrc[i];
			}
			bmpSrc += pitch;
			bmpDst += side;
		}
		FillEdgePadding(width, height, side, (BYTE *)tmpBmp, 16);
		pageIndex = AddTexturePage16(side, side, (BYTE *)tmpBmp);
		free(tmpBmp);
	} else if( SavedAppSettings.RenderMode == RM_Hardware && TextureFormat.bpp >= 16 ) {
		UINT16 *tmpBmp = (UINT16 *)calloc(2, SQR(side));
		UINT16 *bmpDst = tmpBmp;
		BYTE *bmpSrc = bitmap + x + y * pitch;

		// Translating bitmap data from 8 bit bitmap to 16 bit bitmap
		for( DWORD j = 0; j < height; ++j ) {
			for( DWORD i = 0; i < width; ++i ) {
				if( !keyColor || bmpSrc[i] ) {
					RGB888 *color = &bmpPal[bmpSrc[i]]; // get RGB color from palette
					bmpDst[i] = (1 << 15) // convert RGB to 16 bit
							| (((UINT16)color->red   >> 3) << 10)
							| (((UINT16)color->green >> 3) << 5)
							| (((UINT16)color->blue  >> 3));
				} else {
					bmpDst[i] = 0;
				}
			}
			bmpSrc += pitch;
			bmpDst += side;
		}
		FillEdgePadding(width, height, side, (BYTE *)tmpBmp, 16);
		pageIndex = AddTexturePage16(side, side, (BYTE *)tmpBmp);
		free(tmpBmp);
	} else if( SavedAppSettings.RenderMode == RM_Hardware ) {
		BYTE *tmpBmp = (BYTE *)calloc(1, SQR(side));
		UT_MemBlt(tmpBmp, 0, 0, width, height, side, bitmap, x, y, pitch);
		FillEdgePadding(width, height, side, tmpBmp, 8);
		pageIndex = AddTexturePage8(side, side, tmpBmp, hwrPal);
		free(tmpBmp);
	} else if( swrBuf != NULL && width == 256 && height == 256 && side == 256 ) {
		for( DWORD i=0; i<ARRAY_SIZE(TexturePageBuffer8); ++i ) {
			if( TexturePageBuffer8[i] == NULL || TexturePageBuffer8[i] == swrBuf ) {
				UT_MemBlt(swrBuf, 0, 0, width, height, side, bitmap, x, y, pitch);
				TexturePageBuffer8[i] = swrBuf;
				pageIndex = i;
				break;
			}
		}
	}
	return pageIndex;
}

